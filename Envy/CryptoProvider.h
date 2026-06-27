//
 // CryptoProvider.h
//
// This file is part of Envy (getenvy.com) � 2016-2020
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2016
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

class CCryptoProvider
{
public:
	CCryptoProvider();
	virtual ~CCryptoProvider();

	// Check if crypto is available
	static bool IsCryptoAvailable();

	// Generate RSA key pair
	bool GenerateRSAKeyPair();

	// Get public key
	const BYTE* GetPublicKey() const;
	size_t GetPublicKeyLen() const;

	void Cleanup();

	// Sign data with private key
	bool SignData(const BYTE* pData, size_t nDataLen, BYTE* pSignature, size_t& nSignatureLen);

	// Verify signature with public key
	bool VerifySignature(const BYTE* pData, size_t nDataLen, const BYTE* pSignature, size_t nSignatureLen, const BYTE* pPublicKey, size_t nPublicKeyLen);

	// Encrypt data with public key
	bool EncryptWithPublicKey(const BYTE* pData, size_t nDataLen, BYTE* pEncrypted, size_t& nEncryptedLen, const BYTE* pPublicKey, size_t nPublicKeyLen);

	// Decrypt data with private key
	bool DecryptWithPrivateKey(const BYTE* pEncrypted, size_t nEncryptedLen, BYTE* pDecrypted, size_t& nDecryptedLen);

private:
	BYTE* m_pPublicKey;
	size_t m_nPublicKeyLen;
	BYTE* m_pPrivateKey;
	size_t m_nPrivateKeyLen;
	HCRYPTPROV m_hCryptProv;
	HCRYPTKEY m_hKeyPair;
};
