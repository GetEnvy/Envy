//
 // StreamCrypto.h
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

class CStreamCrypto
{
public:
	CStreamCrypto();
	virtual ~CStreamCrypto();

	// Initialize crypto for a connection
	bool Init(bool bOutgoing, const BYTE* pKey, size_t nKeyLen);

	// Encrypt/decrypt data
	bool Encrypt(const BYTE* pInput, BYTE* pOutput, size_t nLength);
	bool Decrypt(const BYTE* pInput, BYTE* pOutput, size_t nLength);

	// Check if crypto is available
	bool IsAvailable() const;

private:
	bool m_bAvailable;
	BYTE m_pKey[16]; // RC4 key
	size_t m_nKeyLen;
};
