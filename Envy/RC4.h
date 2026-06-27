//
// RC4.h
//
// RC4 encryption implementation for CryptLayer
//
// This file is part of Envy (getenvy.com) © 2016-2026
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//

#pragma once

#include <windows.h>

class CRC4
{
public:
	CRC4();
	virtual ~CRC4();

	// Initialize with key
	void Init(const BYTE* pKey, size_t nKeyLen);

	// Encrypt/decrypt data (RC4 is symmetric)
	void Process(BYTE* pData, size_t nDataLen);

	// Check if initialized
	bool IsInitialized() const { return m_bInitialized; }

private:
	bool m_bInitialized;
	BYTE m_S[256];    // S-box
	int m_i, m_j;     // Indices

	void Swap(BYTE& a, BYTE& b);
};
