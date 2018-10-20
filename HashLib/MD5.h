//
// MD5.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2006 and PeerProject 2008-2012
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

class HASHLIB_API CMD5
{
public:
	CMD5();
	~CMD5() {}

public:
	void Reset();
	void Finish();
	void Add(const void* pData, size_t nLength);

	struct HASHLIB_API Digest // 128 bit
	{
		uint32& operator[](size_t i) { return data[ i ]; }
		const uint32& operator[](size_t i) const { return data[ i ]; }
		uint32 data[ 4 ];
	};

	void GetHash(__in_bcount(16) uchar* pHash) const;

// Note VS2012 Win32 (not x64) must be public:
public:
	struct MD5State
	{
		static const size_t blockSize = 64;
		uint64	m_nCount;
		uint32	m_nState[ 4 ];
		uchar	m_oBuffer[ blockSize ];
	};
private:
	MD5State m_State;

#ifndef HASHLIB_USE_ASM
	__forceinline void Transform(const uint32* data);
#endif
};
