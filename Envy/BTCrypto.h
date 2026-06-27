//
// BTCrypto.h
//
// BitTorrent MSE/PE (Message Stream Encryption / Protocol Encryption)
// Implements BEP-10 compatible encryption using DH key exchange + RC4
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
// Envy is free software; AGPLv3
//

#pragma once

#include "RC4.h"
#include "../HashLib/HashLib.h"

class CBuffer;

#define MSE_DH_KEY_LEN      96      // 768-bit DH key
#define MSE_PAD_MAX_LEN     512     // Maximum random padding
#define MSE_VC_LEN          8       // Verification constant length
#define MSE_CRYPTO_PLAIN    0x01    // Plaintext after handshake
#define MSE_CRYPTO_RC4      0x02    // RC4 full stream encryption
#define MSE_HASH_LEN        20      // SHA-1 hash length

// MSE handshake state
enum MseState {
	MSE_IDLE = 0,
	MSE_SENT_YA,           // Initiator: sent DH public key
	MSE_AWAITING_YB,       // Initiator: waiting for responder DH key
	MSE_SENT_CRYPTO,       // Initiator: sent crypto_provide + IA
	MSE_AWAITING_CRYPTO,   // Initiator: waiting for crypto_select

	MSE_AWAITING_YA,       // Responder: waiting for initiator DH key
	MSE_SENT_YB,           // Responder: sent DH public key
	MSE_AWAITING_HASH,     // Responder: waiting for hash verification
	MSE_SENT_SELECT,       // Responder: sent crypto_select

	MSE_ACTIVE,            // Encryption active
	MSE_PLAINTEXT,         // Fell back to plaintext
	MSE_FAILED
};

// 768-bit big integer for DH key exchange
class CBigInt768 {
public:
	DWORD data[24];  // 768 bits = 24 x 32-bit words, big-endian (data[0] = MSW)

	CBigInt768();
	CBigInt768(const BYTE* pBytes, size_t nLen);

	void SetZero();
	void SetFromBytes(const BYTE* pBytes, size_t nLen);
	void ToBytes(BYTE* pBytes, size_t nLen) const;
	bool IsZero() const;

	int Compare(const CBigInt768& other) const;
	void Add(const CBigInt768& other);
	void Sub(const CBigInt768& other);
	void MulMod(const CBigInt768& other, const CBigInt768& mod, CBigInt768& result) const;
	static void ModPow(const CBigInt768& base, const BYTE* exp, size_t expLen, const CBigInt768& mod, CBigInt768& result);
};


class CBTCrypto {
public:
	CBTCrypto();
	~CBTCrypto();

	// Begin MSE handshake as initiator (outgoing connection)
	bool InitiateHandshake(const Hashes::BtHash& infoHash, CBuffer* pOutput);

	// Process incoming data during handshake; returns TRUE when handshake complete
	// Consumes bytes from pInput, may write response to pOutput
	bool ProcessHandshake(CBuffer* pInput, CBuffer* pOutput);

	// Try to detect MSE handshake from first bytes (for incoming connections)
	bool DetectMSE(CBuffer* pInput) const;

	// Begin MSE handshake as responder (incoming connection)
	bool AcceptHandshake(CBuffer* pInput, CBuffer* pOutput);

	// Encrypt outgoing data in-place
	void Encrypt(BYTE* pData, size_t nLength);

	// Decrypt incoming data in-place
	void Decrypt(BYTE* pData, size_t nLength);

	// State queries
	MseState GetState() const { return m_nState; }
	bool IsActive() const { return m_nState == MSE_ACTIVE; }
	bool IsPlaintext() const { return m_nState == MSE_PLAINTEXT; }
	bool IsHandshaking() const { return m_nState > MSE_IDLE && m_nState < MSE_ACTIVE; }
	DWORD GetCryptoMethod() const { return m_nCryptoMethod; }

	// Set the info hash (needed for responder mode)
	void SetInfoHash(const Hashes::BtHash& infoHash);

private:
	MseState    m_nState;
	DWORD       m_nCryptoMethod;
	bool        m_bInitiator;

	Hashes::BtHash m_oInfoHash;
	bool        m_bHasInfoHash;

	// DH key exchange
	BYTE        m_privateKey[MSE_DH_KEY_LEN];
	BYTE        m_publicKey[MSE_DH_KEY_LEN];
	BYTE        m_sharedSecret[MSE_DH_KEY_LEN];

	// RC4 contexts
	CRC4        m_rc4Encrypt;
	CRC4        m_rc4Decrypt;

	// Scratch buffer for handshake processing
	size_t      m_nPadALen;

	void GenerateDHKeyPair();
	void ComputeSharedSecret(const BYTE* pPeerKey);
	void DeriveRC4Keys();
	void HashSHA1(const BYTE* pData, size_t nLen, BYTE* pHash);
	void HashSHA1Two(const BYTE* p1, size_t n1, const BYTE* p2, size_t n2, BYTE* pHash);
};
