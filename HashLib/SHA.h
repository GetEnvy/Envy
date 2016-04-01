//
// SHA.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2008
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

class HASHLIB_API CSHA
{
public:
	CSHA();
	~CSHA() {}

public:
	void Reset();
	void Finish();
	void Add(const void* pData, size_t nLength);

	struct HASHLIB_API Digest // 160 bit
	{
		uint32& operator[](size_t i) { return data[ i ]; }
		const uint32& operator[](size_t i) const { return data[ i ]; }
		uint32 data[ 5 ];
	};

	void GetHash(__in_bcount(20) uchar* pHash) const;

#ifndef HASHLIB_USE_ASM
	struct TransformArray
	{
		__forceinline TransformArray(const uint32* const buffer);
		__forceinline uint32 operator[](uint32 index) const
		{
			return m_buffer[ index ];
		}
		uint32 m_buffer[ 80 ];
	};
#endif

#if !defined(WIN64) && defined(_MSC_VER) && (_MSC_VER >= 1700)		// VS2012 Targeting workaround
public:
#else
private:
#endif
	struct SHA1State
	{
		static const size_t	blockSize = 64;
		uint64	m_nCount;
		uint32	m_nState[ 5 ];
		uchar	m_oBuffer[ blockSize ];
	};
	SHA1State m_State;

#ifndef HASHLIB_USE_ASM
	__forceinline void Transform(const TransformArray w);
#endif
};
