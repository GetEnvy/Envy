//
// BTCrypto.cpp
//
// BitTorrent MSE/PE (Message Stream Encryption / Protocol Encryption)
// Implements BEP-10 compatible encryption using DH key exchange + RC4
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
// Envy is free software; AGPLv3
//

#include "StdAfx.h"
#include "BTCrypto.h"
#include "Buffer.h"
#include "Envy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// MSE DH prime (RFC 2409 Group 1 / MODP 768-bit)
static const BYTE MSE_DH_PRIME[MSE_DH_KEY_LEN] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1, 0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22, 0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B, 0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45, 0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x3A,0x36,0x21, 0x00,0x00,0x00,0x00,0x00,0x09,0x05,0x63
};

static const BYTE MSE_VC[MSE_VC_LEN] = { 0,0,0,0,0,0,0,0 };

//////////////////////////////////////////////////////////////////////
// CBigInt768 - Simple 768-bit big integer for DH

CBigInt768::CBigInt768() {
	SetZero();
}

CBigInt768::CBigInt768(const BYTE* pBytes, size_t nLen) {
	SetFromBytes(pBytes, nLen);
}

void CBigInt768::SetZero() {
	memset(data, 0, sizeof(data));
}

void CBigInt768::SetFromBytes(const BYTE* pBytes, size_t nLen) {
	SetZero();
	// Big-endian bytes to big-endian DWORDs
	size_t offset = (nLen < MSE_DH_KEY_LEN) ? (MSE_DH_KEY_LEN - nLen) : 0;
	size_t start = (nLen > MSE_DH_KEY_LEN) ? (nLen - MSE_DH_KEY_LEN) : 0;

	for (size_t i = start; i < nLen; i++) {
		size_t pos = offset + (i - start);
		size_t wordIdx = pos / 4;
		size_t byteIdx = 3 - (pos % 4);
		if (wordIdx < 24)
			data[wordIdx] |= ((DWORD)pBytes[i]) << (byteIdx * 8);
	}
}

void CBigInt768::ToBytes(BYTE* pBytes, size_t nLen) const {
	memset(pBytes, 0, nLen);
	size_t offset = (nLen > MSE_DH_KEY_LEN) ? (nLen - MSE_DH_KEY_LEN) : 0;
	size_t limit = min(nLen, (size_t)MSE_DH_KEY_LEN);

	for (size_t i = 0; i < limit; i++) {
		size_t wordIdx = i / 4;
		size_t byteIdx = 3 - (i % 4);
		pBytes[offset + i] = (BYTE)(data[wordIdx] >> (byteIdx * 8));
	}
}

bool CBigInt768::IsZero() const {
	for (int i = 0; i < 24; i++)
		if (data[i]) return false;
	return true;
}

int CBigInt768::Compare(const CBigInt768& other) const {
	for (int i = 0; i < 24; i++) {
		if (data[i] > other.data[i]) return 1;
		if (data[i] < other.data[i]) return -1;
	}
	return 0;
}

void CBigInt768::Add(const CBigInt768& other) {
	QWORD carry = 0;
	for (int i = 23; i >= 0; i--) {
		QWORD sum = (QWORD)data[i] + (QWORD)other.data[i] + carry;
		data[i] = (DWORD)sum;
		carry = sum >> 32;
	}
}

void CBigInt768::Sub(const CBigInt768& other) {
	QWORD borrow = 0;
	for (int i = 23; i >= 0; i--) {
		QWORD diff = (QWORD)data[i] - (QWORD)other.data[i] - borrow;
		data[i] = (DWORD)diff;
		borrow = (diff >> 63) & 1;
	}
}

// result = (this * other) mod mod
void CBigInt768::MulMod(const CBigInt768& other, const CBigInt768& mod, CBigInt768& result) const {
	// 1536-bit product
	DWORD product[48];
	memset(product, 0, sizeof(product));

	// Schoolbook multiplication
	for (int i = 23; i >= 0; i--) {
		QWORD carry = 0;
		for (int j = 23; j >= 0; j--) {
			QWORD p = (QWORD)data[i] * (QWORD)other.data[j]
					+ (QWORD)product[i + j + 1] + carry;
			product[i + j + 1] = (DWORD)p;
			carry = p >> 32;
		}
		product[i] += (DWORD)carry;
	}

	// Modular reduction by repeated subtraction (simple but correct)
	// Shift mod left until it's >= product, then subtract
	// For 768-bit mod and 1536-bit product, we need at most 768 steps

	// Copy product to result-sized temp
	DWORD temp[49];
	memset(temp, 0, sizeof(temp));
	memcpy(&temp[1], product, 48 * sizeof(DWORD));

	// Build shifted modulus array (48 DWORDs + 1 for overflow)
	DWORD shifted_mod[49];
	memset(shifted_mod, 0, sizeof(shifted_mod));
	memcpy(&shifted_mod[49 - 24], mod.data, 24 * sizeof(DWORD));

	// Find how many bit positions to shift mod left
	int shift = 0;
	{
		// Find MSB of temp
		int tempMsb = -1;
		for (int i = 0; i < 49; i++) {
			if (temp[i]) {
				DWORD v = temp[i];
				int bit = (48 - i) * 32;
				while (v) { bit++; v >>= 1; }
				tempMsb = bit - 1;
				break;
			}
		}

		// Find MSB of mod
		int modMsb = -1;
		for (int i = 0; i < 24; i++) {
			if (mod.data[i]) {
				DWORD v = mod.data[i];
				int bit = (23 - i) * 32;
				while (v) { bit++; v >>= 1; }
				modMsb = bit - 1;
				break;
			}
		}

		shift = (tempMsb >= modMsb) ? (tempMsb - modMsb) : 0;
	}

	// Shift mod left by 'shift' bits within the 49-word array
	memset(shifted_mod, 0, sizeof(shifted_mod));
	{
		int wordShift = shift / 32;
		int bitShift = shift % 32;
		for (int i = 0; i < 24; i++) {
			int dstIdx = 49 - 24 + i - wordShift;
			if (dstIdx >= 0 && dstIdx < 49) {
				shifted_mod[dstIdx] |= mod.data[i] >> bitShift;
				if (bitShift > 0 && dstIdx - 1 >= 0)
					shifted_mod[dstIdx - 1] |= mod.data[i] << (32 - bitShift);
			}
		}
	}

	// Repeated trial subtraction
	for (int s = shift; s >= 0; s--) {
		// Compare temp >= shifted_mod
		bool ge = true;
		for (int i = 0; i < 49; i++) {
			if (temp[i] > shifted_mod[i]) break;
			if (temp[i] < shifted_mod[i]) { ge = false; break; }
		}

		if (ge) {
			// Subtract
			QWORD borrow = 0;
			for (int i = 48; i >= 0; i--) {
				QWORD diff = (QWORD)temp[i] - (QWORD)shifted_mod[i] - borrow;
				temp[i] = (DWORD)diff;
				borrow = (diff >> 63) & 1;
			}
		}

		// Shift shifted_mod right by 1 bit
		for (int i = 48; i >= 0; i--) {
			shifted_mod[i] = shifted_mod[i] >> 1;
			if (i > 0)
				shifted_mod[i] |= shifted_mod[i - 1] << 31;
		}
	}

	// Copy result (last 24 DWORDs)
	memcpy(result.data, &temp[49 - 24], 24 * sizeof(DWORD));
}

// result = base^exp mod mod (square-and-multiply)
void CBigInt768::ModPow(const CBigInt768& base, const BYTE* exp, size_t expLen,
						const CBigInt768& mod, CBigInt768& result)
{
	// Start with result = 1
	result.SetZero();
	result.data[23] = 1;

	CBigInt768 b = base;
	CBigInt768 temp;

	// Process exponent bits from MSB to LSB
	bool started = false;
	for (size_t i = 0; i < expLen; i++) {
		for (int bit = 7; bit >= 0; bit--) {
			if (started) {
				// Square
				result.MulMod(result, mod, temp);
				result = temp;
			}

			if (exp[i] & (1 << bit)) {
				if (!started) {
					result = b;
					started = true;
				} else {
					// Multiply
					result.MulMod(b, mod, temp);
					result = temp;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CBTCrypto

CBTCrypto::CBTCrypto() :
	m_nState(MSE_IDLE),
	m_nCryptoMethod(0),
	m_bInitiator(false),
	m_bHasInfoHash(false),
	m_nPadALen(0)
{
	memset(m_privateKey, 0, sizeof(m_privateKey));
	memset(m_publicKey, 0, sizeof(m_publicKey));
	memset(m_sharedSecret, 0, sizeof(m_sharedSecret));
}

CBTCrypto::~CBTCrypto() {
	SecureZeroMemory(m_privateKey, sizeof(m_privateKey));
	SecureZeroMemory(m_sharedSecret, sizeof(m_sharedSecret));
}

void CBTCrypto::SetInfoHash(const Hashes::BtHash& infoHash) {
	m_oInfoHash = infoHash;
	m_bHasInfoHash = true;
}

void CBTCrypto::HashSHA1(const BYTE* pData, size_t nLen, BYTE* pHash) {
	CSHA sha;
	sha.Add(pData, nLen);
	sha.Finish();
	sha.GetHash((uchar*)pHash);
}

void CBTCrypto::HashSHA1Two(const BYTE* p1, size_t n1, const BYTE* p2, size_t n2, BYTE* pHash) {
	CSHA sha;
	sha.Add(p1, n1);
	sha.Add(p2, n2);
	sha.Finish();
	sha.GetHash((uchar*)pHash);
}

void CBTCrypto::GenerateDHKeyPair() {
	// Generate 160-bit random private key (MSE spec)
	BYTE privKeyShort[20];
	if (!GenerateCryptographicBytes(privKeyShort, 20)) {
		theApp.Message(MSG_ERROR, L"[BT-MSE] Failed to generate DH private key");
		return;
	}

	// Compute public key: Ya = g^Xa mod p where g=2
	CBigInt768 prime(MSE_DH_PRIME, MSE_DH_KEY_LEN);
	CBigInt768 generator;
	generator.SetZero();
	generator.data[23] = 2;  // g = 2

	CBigInt768 pubKey;
	CBigInt768::ModPow(generator, privKeyShort, 20, prime, pubKey);

	// Store
	memset(m_privateKey, 0, MSE_DH_KEY_LEN);
	memcpy(&m_privateKey[MSE_DH_KEY_LEN - 20], privKeyShort, 20);
	pubKey.ToBytes(m_publicKey, MSE_DH_KEY_LEN);

	SecureZeroMemory(privKeyShort, sizeof(privKeyShort));
}

void CBTCrypto::ComputeSharedSecret(const BYTE* pPeerKey) {
	// S = Yb^Xa mod p (or Ya^Xb mod p)
	CBigInt768 prime(MSE_DH_PRIME, MSE_DH_KEY_LEN);
	CBigInt768 peerKey(pPeerKey, MSE_DH_KEY_LEN);
	CBigInt768 result;

	// Private key is stored in last 20 bytes
	CBigInt768::ModPow(peerKey, &m_privateKey[MSE_DH_KEY_LEN - 20], 20, prime, result);
	result.ToBytes(m_sharedSecret, MSE_DH_KEY_LEN);
}

void CBTCrypto::DeriveRC4Keys() {
	if (!m_bHasInfoHash) return;

	// Derive encryption keys from shared secret + info hash
	// Key A = SHA1("keyA" + S + SKEY) — used by initiator to encrypt
	// Key B = SHA1("keyB" + S + SKEY) — used by responder to encrypt
	const BYTE* pSKEY = (const BYTE*)&m_oInfoHash[0];

	BYTE keyA[MSE_HASH_LEN], keyB[MSE_HASH_LEN];
	{
		CSHA sha;
		sha.Add((const BYTE*)"keyA", 4);
		sha.Add(m_sharedSecret, MSE_DH_KEY_LEN);
		sha.Add(pSKEY, Hashes::BtHash::byteCount);
		sha.Finish();
		sha.GetHash((uchar*)keyA);
	}
	{
		CSHA sha;
		sha.Add((const BYTE*)"keyB", 4);
		sha.Add(m_sharedSecret, MSE_DH_KEY_LEN);
		sha.Add(pSKEY, Hashes::BtHash::byteCount);
		sha.Finish();
		sha.GetHash((uchar*)keyB);
	}

	if (m_bInitiator) {
		m_rc4Encrypt.Init(keyA, MSE_HASH_LEN);
		m_rc4Decrypt.Init(keyB, MSE_HASH_LEN);
	} else {
		m_rc4Encrypt.Init(keyB, MSE_HASH_LEN);
		m_rc4Decrypt.Init(keyA, MSE_HASH_LEN);
	}

	// Discard first 1024 bytes of RC4 output (MSE spec)
	BYTE discard[1024];
	memset(discard, 0, sizeof(discard));
	m_rc4Encrypt.Process(discard, 1024);
	memset(discard, 0, sizeof(discard));
	m_rc4Decrypt.Process(discard, 1024);

	SecureZeroMemory(keyA, sizeof(keyA));
	SecureZeroMemory(keyB, sizeof(keyB));
}

bool CBTCrypto::InitiateHandshake(const Hashes::BtHash& infoHash, CBuffer* pOutput) {
	SetInfoHash(infoHash);
	m_bInitiator = true;

	// Generate DH key pair
	GenerateDHKeyPair();

	// Send: Ya (96 bytes) + Pad_A (random 0-512 bytes)
	pOutput->Add(m_publicKey, MSE_DH_KEY_LEN);

	// Random padding (0-512 bytes)
	BYTE padLen;
	if (GenerateCryptographicBytes(&padLen, 1)) {
		m_nPadALen = padLen % (MSE_PAD_MAX_LEN + 1);
	} else {
		m_nPadALen = 0;
	}

	if (m_nPadALen > 0) {
		BYTE pad[MSE_PAD_MAX_LEN];
		if (GenerateCryptographicBytes(pad, m_nPadALen))
			pOutput->Add(pad, m_nPadALen);
	}

	m_nState = MSE_SENT_YA;
	return true;
}

bool CBTCrypto::DetectMSE(CBuffer* pInput) const {
	// MSE connections don't start with the BT protocol header
	// If the first bytes are NOT "\023BitTorrent protocol", it might be MSE
	if (pInput->m_nLength < 20)
		return false;

	return (memcmp(pInput->m_pBuffer, "\023BitTorrent protocol", 20) != 0);
}

bool CBTCrypto::AcceptHandshake(CBuffer* pInput, CBuffer* pOutput) {
	m_bInitiator = false;
	m_nState = MSE_AWAITING_YA;

	// Generate our DH key pair
	GenerateDHKeyPair();

	return ProcessHandshake(pInput, pOutput);
}

bool CBTCrypto::ProcessHandshake(CBuffer* pInput, CBuffer* pOutput) {
	switch (m_nState) {
	case MSE_SENT_YA:
	case MSE_AWAITING_YB:
	{
		// Initiator: waiting for Yb (96 bytes + 0-512 padding)
		if (pInput->m_nLength < MSE_DH_KEY_LEN)
			return true;  // Need more data

		BYTE peerKey[MSE_DH_KEY_LEN];
		if (!pInput->Read(peerKey, MSE_DH_KEY_LEN))
			return true;

		// Compute shared secret
		ComputeSharedSecret(peerKey);
		DeriveRC4Keys();

		// Now send: HASH('req1', S) + HASH('req2', SKEY) XOR HASH('req3', S)
		const BYTE* pSKEY = (const BYTE*)&m_oInfoHash[0];

		BYTE hash1[MSE_HASH_LEN];
		HashSHA1Two((const BYTE*)"req1", 4, m_sharedSecret, MSE_DH_KEY_LEN, hash1);
		pOutput->Add(hash1, MSE_HASH_LEN);

		BYTE hash2[MSE_HASH_LEN], hash3[MSE_HASH_LEN];
		HashSHA1Two((const BYTE*)"req2", 4, pSKEY, Hashes::BtHash::byteCount, hash2);
		HashSHA1Two((const BYTE*)"req3", 4, m_sharedSecret, MSE_DH_KEY_LEN, hash3);

		BYTE xored[MSE_HASH_LEN];
		for (int i = 0; i < MSE_HASH_LEN; i++)
			xored[i] = hash2[i] ^ hash3[i];
		pOutput->Add(xored, MSE_HASH_LEN);

		// Send encrypted: VC(8) + crypto_provide(4) + len(Pad_C)(2) + Pad_C + len(IA)(2) + IA
		BYTE block[16];

		// VC (8 zero bytes, encrypted)
		memcpy(block, MSE_VC, MSE_VC_LEN);
		m_rc4Encrypt.Process(block, MSE_VC_LEN);
		pOutput->Add(block, MSE_VC_LEN);

		// crypto_provide (4 bytes: we support RC4 and plaintext)
		DWORD provide = MSE_CRYPTO_RC4 | MSE_CRYPTO_PLAIN;
		memcpy(block, &provide, 4);
		m_rc4Encrypt.Process(block, 4);
		pOutput->Add(block, 4);

		// len(Pad_C) = 0
		WORD padCLen = 0;
		memcpy(block, &padCLen, 2);
		m_rc4Encrypt.Process(block, 2);
		pOutput->Add(block, 2);

		// len(IA) = 0 (no initial payload)
		WORD iaLen = 0;
		memcpy(block, &iaLen, 2);
		m_rc4Encrypt.Process(block, 2);
		pOutput->Add(block, 2);

		m_nState = MSE_SENT_CRYPTO;
		return true;
	}

	case MSE_SENT_CRYPTO:
	case MSE_AWAITING_CRYPTO:
	{
		// Initiator: waiting for responder's VC + crypto_select + Pad_D
		if (pInput->m_nLength < MSE_VC_LEN + 4 + 2)
			return true;  // Need more data

		// Read and decrypt VC
		BYTE vc[MSE_VC_LEN];
		if (!pInput->Read(vc, MSE_VC_LEN)) return true;
		m_rc4Decrypt.Process(vc, MSE_VC_LEN);

		// Verify VC is all zeros
		if (memcmp(vc, MSE_VC, MSE_VC_LEN) != 0) {
			theApp.Message(MSG_WARNING, L"[BT-MSE] Verification constant mismatch");
			m_nState = MSE_FAILED;
			return false;
		}

		// Read crypto_select
		BYTE selectBuf[4];
		if (!pInput->Read(selectBuf, 4)) return true;
		m_rc4Decrypt.Process(selectBuf, 4);
		memcpy(&m_nCryptoMethod, selectBuf, 4);

		// Read len(Pad_D)
		BYTE padDLenBuf[2];
		if (!pInput->Read(padDLenBuf, 2)) return true;
		m_rc4Decrypt.Process(padDLenBuf, 2);
		WORD padDLen;
		memcpy(&padDLen, padDLenBuf, 2);

		// Skip Pad_D
		if (pInput->m_nLength < padDLen)
			return true;

		if (padDLen > 0) {
			BYTE* padD = new BYTE[padDLen];
			if (!pInput->Read(padD, padDLen)) { delete[] padD; return true; }
			m_rc4Decrypt.Process(padD, padDLen);
			delete[] padD;
		}

		// Handshake complete
		if (m_nCryptoMethod & MSE_CRYPTO_RC4) {
			m_nState = MSE_ACTIVE;
			theApp.Message(MSG_DEBUG, L"[BT-MSE] Handshake complete (RC4 encryption)");
		} else if (m_nCryptoMethod & MSE_CRYPTO_PLAIN) {
			m_nState = MSE_PLAINTEXT;
			theApp.Message(MSG_DEBUG, L"[BT-MSE] Handshake complete (plaintext)");
		} else {
			theApp.Message(MSG_WARNING, L"[BT-MSE] No acceptable crypto method");
			m_nState = MSE_FAILED;
			return false;
		}

		return true;
	}

	case MSE_AWAITING_YA:
	{
		// Responder: waiting for Ya (96 bytes)
		if (pInput->m_nLength < MSE_DH_KEY_LEN)
			return true;

		BYTE peerKey[MSE_DH_KEY_LEN];
		if (!pInput->Read(peerKey, MSE_DH_KEY_LEN))
			return true;

		ComputeSharedSecret(peerKey);

		// Send Yb + Pad_B
		pOutput->Add(m_publicKey, MSE_DH_KEY_LEN);

		m_nState = MSE_SENT_YB;
		return true;
	}

	case MSE_SENT_YB:
	case MSE_AWAITING_HASH:
	{
		// Responder: waiting for HASH('req1', S)(20) + HASH('req2',SKEY)^HASH('req3',S)(20)
		// + encrypted [VC(8) + crypto_provide(4) + len(Pad_C)(2) + Pad_C + len(IA)(2) + IA]
		if (pInput->m_nLength < 2 * MSE_HASH_LEN)
			return true;

		// Read and verify hash1 = HASH('req1', S)
		BYTE recvHash1[MSE_HASH_LEN];
		if (!pInput->Read(recvHash1, MSE_HASH_LEN)) return true;

		BYTE expectedHash1[MSE_HASH_LEN];
		HashSHA1Two((const BYTE*)"req1", 4, m_sharedSecret, MSE_DH_KEY_LEN, expectedHash1);

		if (memcmp(recvHash1, expectedHash1, MSE_HASH_LEN) != 0) {
			theApp.Message(MSG_WARNING, L"[BT-MSE] req1 hash mismatch (responder)");
			m_nState = MSE_FAILED;
			return false;
		}

		// Read HASH('req2',SKEY)^HASH('req3',S) and verify using our info hash
		BYTE recvXored[MSE_HASH_LEN];
		if (!pInput->Read(recvXored, MSE_HASH_LEN)) return true;

		if (m_bHasInfoHash) {
			const BYTE* pSKEY = (const BYTE*)&m_oInfoHash[0];
			BYTE hash2[MSE_HASH_LEN], hash3[MSE_HASH_LEN];
			HashSHA1Two((const BYTE*)"req2", 4, pSKEY, Hashes::BtHash::byteCount, hash2);
			HashSHA1Two((const BYTE*)"req3", 4, m_sharedSecret, MSE_DH_KEY_LEN, hash3);

			BYTE expectedXored[MSE_HASH_LEN];
			for (int i = 0; i < MSE_HASH_LEN; i++)
				expectedXored[i] = hash2[i] ^ hash3[i];

			if (memcmp(recvXored, expectedXored, MSE_HASH_LEN) != 0) {
				theApp.Message(MSG_WARNING, L"[BT-MSE] SKEY hash mismatch (responder)");
				m_nState = MSE_FAILED;
				return false;
			}
		}

		// Derive RC4 keys
		DeriveRC4Keys();

		// Read encrypted: VC(8) + crypto_provide(4) + len(Pad_C)(2) + Pad_C + len(IA)(2) + IA
		if (pInput->m_nLength < MSE_VC_LEN + 4 + 2 + 2)
			return true;

		BYTE vc[MSE_VC_LEN];
		pInput->Read(vc, MSE_VC_LEN);
		m_rc4Decrypt.Process(vc, MSE_VC_LEN);
		pInput->Remove(MSE_VC_LEN);

		BYTE provideBuf[4];
		pInput->Read(provideBuf, 4);
		m_rc4Decrypt.Process(provideBuf, 4);
		pInput->Remove(4);
		DWORD cryptoProvide;
		memcpy(&cryptoProvide, provideBuf, 4);

		BYTE padCLenBuf[2];
		pInput->Read(padCLenBuf, 2);
		m_rc4Decrypt.Process(padCLenBuf, 2);
		pInput->Remove(2);
		WORD padCLen;
		memcpy(&padCLen, padCLenBuf, 2);

		if (padCLen > 0) {
			if (pInput->m_nLength < padCLen + 2u)
				return true;
			BYTE* padC = new BYTE[padCLen];
			if (!pInput->Read(padC, padCLen)) { delete[] padC; return true; }
			m_rc4Decrypt.Process(padC, padCLen);
			delete[] padC;
		}

		// Read len(IA)
		BYTE iaLenBuf[2];
		if (pInput->m_nLength < 2)
			return true;
		if (!pInput->Read(iaLenBuf, 2)) return true;
		m_rc4Decrypt.Process(iaLenBuf, 2);
		WORD iaLen;
		memcpy(&iaLen, iaLenBuf, 2);

		// IA (initial payload) stays in the buffer for BT handshake processing
		if (iaLen > 0 && pInput->m_nLength >= iaLen) {
			// Decrypt IA in-place
			m_rc4Decrypt.Process(pInput->m_pBuffer, iaLen);
		}

		// Select crypto method (prefer RC4)
		if (cryptoProvide & MSE_CRYPTO_RC4)
			m_nCryptoMethod = MSE_CRYPTO_RC4;
		else if (cryptoProvide & MSE_CRYPTO_PLAIN)
			m_nCryptoMethod = MSE_CRYPTO_PLAIN;
		else {
			m_nState = MSE_FAILED;
			return false;
		}

		// Send response: encrypted [VC(8) + crypto_select(4) + len(Pad_D)(2) + Pad_D]
		BYTE block[16];

		memcpy(block, MSE_VC, MSE_VC_LEN);
		m_rc4Encrypt.Process(block, MSE_VC_LEN);
		pOutput->Add(block, MSE_VC_LEN);

		memcpy(block, &m_nCryptoMethod, 4);
		m_rc4Encrypt.Process(block, 4);
		pOutput->Add(block, 4);

		WORD padDLen = 0;
		memcpy(block, &padDLen, 2);
		m_rc4Encrypt.Process(block, 2);
		pOutput->Add(block, 2);

		if (m_nCryptoMethod == MSE_CRYPTO_RC4) {
			m_nState = MSE_ACTIVE;
			theApp.Message(MSG_DEBUG, L"[BT-MSE] Responder handshake complete (RC4)");
		} else {
			m_nState = MSE_PLAINTEXT;
			theApp.Message(MSG_DEBUG, L"[BT-MSE] Responder handshake complete (plaintext)");
		}

		return true;
	}

	default:
		return true;
	}
}

void CBTCrypto::Encrypt(BYTE* pData, size_t nLength) {
	if (m_nState == MSE_ACTIVE && m_nCryptoMethod == MSE_CRYPTO_RC4)
		m_rc4Encrypt.Process(pData, nLength);
}

void CBTCrypto::Decrypt(BYTE* pData, size_t nLength) {
	if (m_nState == MSE_ACTIVE && m_nCryptoMethod == MSE_CRYPTO_RC4)
		m_rc4Decrypt.Process(pData, nLength);
}
