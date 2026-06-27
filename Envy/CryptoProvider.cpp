//
// CryptoProvider.cpp
//
// RSA cryptography implementation for CryptLayer and SecureID
//
// This file is part of Envy (getenvy.com) © 2016-2026
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//

#include "StdAfx.h"
#include "CryptoProvider.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CCryptoProvider::CCryptoProvider()
	: m_pPublicKey(NULL)
	, m_nPublicKeyLen(0)
	, m_pPrivateKey(NULL)
	, m_nPrivateKeyLen(0)
	, m_hCryptProv(NULL)
	, m_hKeyPair(NULL)
{
}

CCryptoProvider::~CCryptoProvider()
{
	Cleanup();
}

void CCryptoProvider::Cleanup()
{
	if (m_hKeyPair)
	{
		CryptDestroyKey(m_hKeyPair);
		m_hKeyPair = NULL;
	}

	if (m_hCryptProv)
	{
		CryptReleaseContext(m_hCryptProv, 0);
		m_hCryptProv = NULL;
	}

	if (m_pPublicKey)
	{
		delete[] m_pPublicKey;
		m_pPublicKey = NULL;
		m_nPublicKeyLen = 0;
	}

	if (m_pPrivateKey)
	{
		delete[] m_pPrivateKey;
		m_pPrivateKey = NULL;
		m_nPrivateKeyLen = 0;
	}
}

bool CCryptoProvider::IsCryptoAvailable()
{
	HCRYPTPROV hProv = NULL;
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		CryptReleaseContext(hProv, 0);
		return true;
	}
	return false;
}

bool CCryptoProvider::GenerateRSAKeyPair()
{
	Cleanup();

	// Acquire cryptographic context
	if (!CryptAcquireContext(&m_hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		return false;
	}

	// Generate RSA key pair (1024-bit for eMule compatibility)
	if (!CryptGenKey(m_hCryptProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &m_hKeyPair))
	{
		Cleanup();
		return false;
	}

	// Export public key
	DWORD dwKeyLen = 0;
	if (!CryptExportKey(m_hKeyPair, NULL, PUBLICKEYBLOB, 0, NULL, &dwKeyLen))
	{
		Cleanup();
		return false;
	}

	m_pPublicKey = new BYTE[dwKeyLen];
	m_nPublicKeyLen = dwKeyLen;

	if (!CryptExportKey(m_hKeyPair, NULL, PUBLICKEYBLOB, 0, m_pPublicKey, &dwKeyLen))
	{
		Cleanup();
		return false;
	}

	// Export private key
	if (!CryptExportKey(m_hKeyPair, NULL, PRIVATEKEYBLOB, 0, NULL, &dwKeyLen))
	{
		Cleanup();
		return false;
	}

	m_pPrivateKey = new BYTE[dwKeyLen];
	m_nPrivateKeyLen = dwKeyLen;

	if (!CryptExportKey(m_hKeyPair, NULL, PRIVATEKEYBLOB, 0, m_pPrivateKey, &dwKeyLen))
	{
		Cleanup();
		return false;
	}

	return true;
}

const BYTE* CCryptoProvider::GetPublicKey() const
{
	return m_pPublicKey;
}

size_t CCryptoProvider::GetPublicKeyLen() const
{
	return m_nPublicKeyLen;
}

bool CCryptoProvider::SignData(const BYTE* pData, size_t nDataLen, BYTE* pSignature, size_t& nSignatureLen)
{
	if (!m_hCryptProv || !m_hKeyPair || !pData || !pSignature)
		return false;

	// Create hash of the data
	HCRYPTHASH hHash = NULL;
	if (!CryptCreateHash(m_hCryptProv, CALG_SHA1, 0, 0, &hHash))
		return false;

	if (!CryptHashData(hHash, pData, (DWORD)nDataLen, 0))
	{
		CryptDestroyHash(hHash);
		return false;
	}

	// Sign the hash
	DWORD dwSigLen = nSignatureLen;
	if (!CryptSignHash(hHash, AT_KEYEXCHANGE, NULL, 0, pSignature, &dwSigLen))
	{
		CryptDestroyHash(hHash);
		return false;
	}

	nSignatureLen = dwSigLen;
	CryptDestroyHash(hHash);

	return true;
}

bool CCryptoProvider::VerifySignature(const BYTE* pData, size_t nDataLen, const BYTE* pSignature, size_t nSignatureLen,
									  const BYTE* pPublicKey, size_t nPublicKeyLen)
{
	if (!pData || !pSignature || !pPublicKey)
		return false;

	// Import the public key
	HCRYPTPROV hProv = NULL;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		return false;

	HCRYPTKEY hPubKey = NULL;
	if (!CryptImportKey(hProv, pPublicKey, (DWORD)nPublicKeyLen, NULL, 0, &hPubKey))
	{
		CryptReleaseContext(hProv, 0);
		return false;
	}

	// Create hash of the data
	HCRYPTHASH hHash = NULL;
	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
	{
		CryptDestroyKey(hPubKey);
		CryptReleaseContext(hProv, 0);
		return false;
	}

	if (!CryptHashData(hHash, pData, (DWORD)nDataLen, 0))
	{
		CryptDestroyHash(hHash);
		CryptDestroyKey(hPubKey);
		CryptReleaseContext(hProv, 0);
		return false;
	}

	// Verify the signature
	BOOL bResult = CryptVerifySignature(hHash, pSignature, (DWORD)nSignatureLen, hPubKey, NULL, 0);

	CryptDestroyHash(hHash);
	CryptDestroyKey(hPubKey);
	CryptReleaseContext(hProv, 0);

	return (bResult != FALSE);
}

bool CCryptoProvider::EncryptWithPublicKey(const BYTE* pData, size_t nDataLen, BYTE* pEncrypted, size_t& nEncryptedLen, const BYTE* pPublicKey, size_t nPublicKeyLen)
{
	if (!pData || !pEncrypted || !pPublicKey || nDataLen == 0)
		return false;

	// Import the public key for encryption
	HCRYPTPROV hProv = NULL;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		return false;

	HCRYPTKEY hPubKey = NULL;
	if (!CryptImportKey(hProv, pPublicKey, (DWORD)nPublicKeyLen, NULL, 0, &hPubKey))
	{
		CryptReleaseContext(hProv, 0);
		return false;
	}

	// Encrypt the data
	DWORD dwEncryptedLen = nEncryptedLen;
	if (!CryptEncrypt(hPubKey, NULL, TRUE, 0, pEncrypted, &dwEncryptedLen, (DWORD)nEncryptedLen))
	{
		CryptDestroyKey(hPubKey);
		CryptReleaseContext(hProv, 0);
		return false;
	}

	nEncryptedLen = dwEncryptedLen;

	CryptDestroyKey(hPubKey);
	CryptReleaseContext(hProv, 0);

	return true;
}

bool CCryptoProvider::DecryptWithPrivateKey(const BYTE* pEncrypted, size_t nEncryptedLen, BYTE* pDecrypted, size_t& nDecryptedLen)
{
	if (!m_hCryptProv || !m_hKeyPair || !pEncrypted || !pDecrypted)
		return false;

	// Copy encrypted data to avoid modifying the input
	BYTE* pTemp = new BYTE[nEncryptedLen];
	memcpy(pTemp, pEncrypted, nEncryptedLen);

	// Decrypt the data
	DWORD dwDecryptedLen = nDecryptedLen;
	if (!CryptDecrypt(m_hKeyPair, NULL, TRUE, 0, pTemp, &dwDecryptedLen))
	{
		delete[] pTemp;
		return false;
	}

	if (dwDecryptedLen > nDecryptedLen)
	{
		delete[] pTemp;
		return false; // Output buffer too small
	}

	memcpy(pDecrypted, pTemp, dwDecryptedLen);
	nDecryptedLen = dwDecryptedLen;

	delete[] pTemp;
	return true;
}
