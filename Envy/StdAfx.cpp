//
// StdAfx.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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

#include "StdAfx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

__int64 GetMicroCount()
{
	static __int64 Freq = 0;
	static __int64 FirstCount = 0;
	if ( Freq < 0 )
		return GetTickCount() * 1000;

	if ( Freq == 0 )
	{
		if ( ! QueryPerformanceFrequency( (LARGE_INTEGER*)&Freq ) )
		{
			Freq = -1;
			return GetMicroCount();
		}
		QueryPerformanceCounter( (LARGE_INTEGER*)&FirstCount );
	}
	__int64 Count = 0;
	QueryPerformanceCounter( (LARGE_INTEGER*)&Count );
	return ( 1000000 * ( Count - FirstCount ) ) / Freq;
}

class InitGetMicroCount
{
public:
	inline InitGetMicroCount() throw() { GetMicroCount(); }
};

InitGetMicroCount initGetMicroCount;

static const UINT primes[] =
{
	11,			13,			17,			19,			23,			29,
	31,			61,			127,		251,		347,		509,
	631,		761,		887,		1021,		1531,		2039,
	3067,		4093,		5119,		6143,		7159,		8191,
	9209,		10223,		11261,		12227,		13309,		14327,
	16381,		20479,		24571,		28669,		32749,		49139,
	65521,		98299,		131071,		196597,		262139,		327673
};

UINT GetBestHashTableSize(UINT nCount)
{
	const UINT* last = primes + ( sizeof( primes ) / sizeof( primes[ 0 ] ) - 1 );
	const UINT value = ( nCount + nCount / 5 );
	return * std::lower_bound( primes, last, value, std::less< UINT >() );	// + 20%
}

// Disable exceptions if memory allocation fails

class NoThrowNew
{
public:
	NoThrowNew() throw()
	{
		std::set_new_handler( &NoThrowNew::OutOfMemoryHandlerStd );
		_set_new_handler( &NoThrowNew::OutOfMemoryHandler );
		AfxSetNewHandler( &NoThrowNew::OutOfMemoryHandlerAfx );
	}

private:
	static void __cdecl OutOfMemoryHandlerStd() throw()
	{
	}

	static int __cdecl OutOfMemoryHandler(size_t /* nSize */) throw()
	{
		return 0;
	}

	static int __cdecl OutOfMemoryHandlerAfx(size_t /* nSize */) throw()
	{
		return 0;
	}
};

NoThrowNew initNoThrowNew;
