//
// MinMax.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
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
// Replaces legacy MinMax.hpp (with Boost dependency)
// Note: Simply using standard min/max templates/macros introduces runtime bugs.
// Maintaining these overloaded functions appears to be a safe compromise.

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifndef DWORD
#define NOTYPES
typedef unsigned char BYTE;
typedef unsigned int  UINT;
typedef unsigned long DWORD;	// ul
typedef unsigned __int64 QWORD;	// ull
#endif

// Min():

inline __int64 min(__int64 l, __int64 r)	// ll (long long)
{
	return l < r ? l : r;
}

inline int min(int l, int r)
{
	return l < r ? l : r;
}

inline UINT min(UINT l, UINT r)
{
	return l < r ? l : r;
}

inline BYTE min(BYTE l, BYTE r)
{
	return l < r ? l : r;
}

inline DWORD min(DWORD l, DWORD r)			// ul
{
	return l < r ? l : r;
}

inline QWORD min(QWORD l, QWORD r)			// ull
{
	return l < r ? l : r;
}

inline DWORD min(DWORD l, int r)			// Constant
{
	ASSERT( r >= 0 );
	return l < (DWORD)r ? l : (DWORD)r;
}

inline float min(float l, float r)
{
	return l < r ? l : r;
}

inline double min(double l, double r)
{
	return l < r ? l : r;
}


// Max():

inline __int64 max(__int64 l, __int64 r)	// ll (long long)
{
	return l > r ? l : r;
}

inline int max(int l, int r)
{
	return l > r ? l : r;
}

inline UINT max(UINT l, UINT r)
{
	return l > r ? l : r;
}

inline BYTE max(BYTE l, BYTE r)
{
	return l > r ? l : r;
}

inline DWORD max(DWORD l, DWORD r)			// ul
{
	return l > r ? l : r;
}

inline QWORD max(QWORD l, QWORD r)			// ull
{
	return l > r ? l : r;
}

inline DWORD max(DWORD l, int r)			// Constant
{
	ASSERT( r >= 0 );
	return l > (DWORD)r ? l : (DWORD)r;
}

inline float max(float l, float r)
{
	return l > r ? l : r;
}

inline double max(double l, double r)
{
	return l > r ? l : r;
}


#ifdef NOTYPES
#undef NOTYPES
#undef BYTE
#undef UINT
#undef DWORD
#undef QWORD
#endif
