//
// QueryKeys.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#include "StdAfx.h"
#include "Envy.h"
#include "QueryKeys.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CQueryKeys construction

CQueryKeys::CQueryKeys()
	: m_nBits	( 0 )
	, m_pTable	( NULL )
	, m_nTable	( 0 )
	, m_pMap	( NULL )
{
}

void CQueryKeys::Alloc()
{
	m_nBits		= 12;
	m_nTable	= 1u << m_nBits;
	m_pTable	= new DWORD[ m_nTable ];
	m_pMap		= new DWORD[ m_nBits * 2 ];

	DWORD* pMap = m_pMap;

	for ( DWORD nCount = m_nBits ; nCount ; nCount-- )
	{
		*pMap++ = 1 << GetRandomNum( 0, 31 );
		*pMap++ = 1 << GetRandomNum( 0, 31 );
	}

	BYTE* pFill = (BYTE*)m_pTable;

	for ( DWORD nCount = m_nTable ; nCount ; nCount-- )
	{
		*pFill++ = GetRandomNum( 0ui8, _UI8_MAX );
		*pFill++ = GetRandomNum( 0ui8, _UI8_MAX );
		*pFill++ = GetRandomNum( 0ui8, _UI8_MAX );
		*pFill++ = GetRandomNum( 0ui8, _UI8_MAX );
	}

	// ToDo: Add check for invalid (for Shareaza/Envy) zero keys
}

CQueryKeys::~CQueryKeys()
{
	delete [] m_pMap;
	delete [] m_pTable;
}

//////////////////////////////////////////////////////////////////////
// CQueryKeys create or lookup

DWORD CQueryKeys::Create(DWORD nAddress)
{
	if ( ! m_pTable )
		Alloc();

	const DWORD* pMap = m_pMap;
	DWORD nHash = 0;

	for ( DWORD nCount = m_nBits, nBit = 1 ; nCount ; nCount--, nBit <<= 1 )
	{
		BOOL bOne = ( nAddress & (*pMap++) ) != 0;
		BOOL bTwo = ( nAddress & (*pMap++) ) != 0;
		if ( bOne ^ bTwo ) nHash |= nBit;
	}

	return m_pTable[ nHash & ( m_nTable - 1 ) ];
}

//////////////////////////////////////////////////////////////////////
// CQueryKeys check

BOOL CQueryKeys::Check(DWORD nAddress, DWORD nKey)
{
	return nKey == Create( nAddress );
}
