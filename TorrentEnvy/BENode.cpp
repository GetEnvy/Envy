//
// BENode.cpp
//
// This file is part of Torrent Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2008,2014 and Shareaza 2007
//
// Envy is free software; you can redistribute it
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
// Note: Mirrors ..\Envy\BENode.cpp file

#include "StdAfx.h"
#include "TorrentEnvy.h"
#include "BENode.h"
#include "Buffer.h"

#ifdef _PORTABLE
#include "Portable\SHA1.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

static bool Decode(UINT nCodePage, LPCSTR szFrom, CString& strTo)
{
	const int nLength = MultiByteToWideChar( nCodePage, MB_ERR_INVALID_CHARS, szFrom, -1, NULL, 0 );
	if ( nLength < 1 ) return false;

	MultiByteToWideChar( nCodePage, 0, szFrom, -1, strTo.GetBuffer( nLength ), nLength );
	strTo.ReleaseBuffer();

	return true;
}

UINT CBENode::m_nDefaultCP = CP_ACP;

//////////////////////////////////////////////////////////////////////
// CBENode construction/destruction

CBENode::CBENode()
	: m_nType		( beNull )
	, m_pValue		( NULL )
	, m_nValue		( 0 )
{
}

CBENode::~CBENode()
{
	if ( m_pValue != NULL ) Clear();
}

//////////////////////////////////////////////////////////////////////
// CBENode clear

void CBENode::Clear()
{
	if ( m_pValue != NULL )
	{
		if ( m_nType == beString )
		{
			delete [] (LPSTR)m_pValue;
		}
		else if ( m_nType == beList )
		{
			CBENode** pNode = (CBENode**)m_pValue;
			for ( ; m_nValue--; pNode++ ) delete *pNode;
			delete [] (CBENode**)m_pValue;
		}
		else if ( m_nType == beDict )
		{
			CBENode** pNode = (CBENode**)m_pValue;
			for ( ; m_nValue--; pNode++ )
			{
				delete *pNode++;
				delete [] (LPBYTE)*pNode;
			}
			delete [] (CBENode**)m_pValue;
		}
	}

	m_nType  = beNull;
	m_pValue = NULL;
	m_nValue = 0;
}

//////////////////////////////////////////////////////////////////////
// CBENode add a child node

CBENode* CBENode::Add(const LPBYTE pKey, size_t nKey)
{
	switch ( m_nType )
	{
	case beNull:
		m_nType  = ( pKey != NULL && nKey > 0 ) ? beDict : beList;
		m_pValue = NULL;
		m_nValue = 0;
		break;
	case beList:
		ASSERT( pKey == NULL && nKey == 0 );
		break;
	case beDict:
		ASSERT( pKey != NULL && nKey > 0 );
		break;
	default:
		ASSERT( FALSE );
		break;
	}

//	auto_ptr< CBENode > pNew( new CBENode );
//	CBENode* pNew_ = pNew.get();

	CAutoPtr< CBENode > pNew( new CBENode );
	if ( ! pNew )
		return NULL;	// Out of memory

	CBENode* pNew_ = pNew;

	// Overflow check
	ASSERT ( m_nValue <= SIZE_T_MAX );
	size_t nValue = static_cast< size_t >( m_nValue );

	if ( m_nType == beList )
	{
		// Overflow check
		ASSERT( nValue + 1 <= SIZE_T_MAX );
	//	auto_array< CBENode* > pList( new CBENode*[ nValue + 1 ] );
		CAutoVectorPtr< CBENode* > pList( new CBENode*[ nValue + 1 ] );
		if ( ! pList )
			return NULL;	// Out of memory

		if ( m_pValue )
		{
			// Overflow check
			ASSERT( nValue * sizeof( CBENode* ) <= SIZE_T_MAX );
			memcpy( pList, m_pValue, nValue * sizeof( CBENode* ) );

			delete [] (CBENode**)m_pValue;
		}

		pList[ nValue ] = pNew.Detach();

		m_pValue = pList.Detach();
		++m_nValue;
	}
	else
	{
		const size_t nValueDoubled = nValue * 2;

		// Overflow check
		ASSERT( nValueDoubled + 2 <= SIZE_T_MAX );
		CAutoVectorPtr< CBENode* > pList( new CBENode*[ nValueDoubled + 2 ] );
		if ( ! pList )
			return NULL;	// Out of memory

		if ( m_pValue )
		{
			// Overflow check
			ASSERT( nValueDoubled * sizeof( CBENode* ) <= SIZE_T_MAX );
			memcpy( pList, m_pValue, nValueDoubled * sizeof( CBENode* ) );

			delete [] (CBENode**)m_pValue;
		}

		CAutoVectorPtr< BYTE > pxKey( new BYTE[ nKey + 1 ] );
		if ( ! pxKey )
			return NULL;	// Out of memory

		memcpy( pxKey, pKey, nKey );
		pxKey[ nKey ] = 0;

		pList[ nValueDoubled ] = pNew.Detach();
		pList[ nValueDoubled + 1 ] = (CBENode*)pxKey.Detach();

		m_pValue = pList.Detach();
		++m_nValue;
	}

	return pNew_;
}

//////////////////////////////////////////////////////////////////////
// CBENode find a child node

CBENode* CBENode::GetNode(LPCSTR pszKey) const
{
	if ( m_nType != beDict ) return NULL;

	CBENode** pNode = (CBENode**)m_pValue;

	for ( DWORD nNode = (DWORD)m_nValue; nNode; nNode--, pNode += 2 )
	{
		if ( strcmp( pszKey, (LPCSTR)pNode[1] ) == 0 ) return *pNode;
	}

	return NULL;
}

CBENode* CBENode::GetNode(const LPBYTE pKey, int nKey) const
{
	if ( m_nType != beDict ) return NULL;

	CBENode** pNode = (CBENode**)m_pValue;

	for ( DWORD nNode = (DWORD)m_nValue; nNode; nNode--, pNode += 2 )
	{
		if ( memcmp( pKey, (LPBYTE)pNode[1], nKey ) == 0 ) return *pNode;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CBENode SHA1 computation

#ifdef _PORTABLE
CHashSHA1 CBENode::GetSHA1() const
{
	ASSERT( this != NULL );

	CBuffer pBuffer;
	Encode( &pBuffer );

	CSHA1 pDigest;
	pDigest.Add( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pDigest.Finish();
	return pDigest;
}
#else // Use HashLib
CSHA CBENode::GetSHA1() const
{
	ASSERT( this != NULL );

	CBuffer pBuffer;
	Encode( &pBuffer );

	CSHA pSHA;
	pSHA.Add( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pSHA.Finish();
	return pSHA;
}
#endif

//////////////////////////////////////////////////////////////////////
// CBENode encoding

void CBENode::Encode(CBuffer* pBuffer) const
{
	CHAR szBuffer[64];

	ASSERT( this != NULL );
	ASSERT( pBuffer != NULL );
	CString str;

	if ( m_nType == beString )
	{
		sprintf( szBuffer, "%u:", (DWORD)m_nValue );
		pBuffer->Print( szBuffer );
		pBuffer->Add( m_pValue, (DWORD)m_nValue );
	}
	else if ( m_nType == beInt )
	{
		sprintf( szBuffer, "i%I64ie", m_nValue );
		pBuffer->Print( szBuffer );
	}
	else if ( m_nType == beList )
	{
		CBENode** pNode = (CBENode**)m_pValue;

		pBuffer->Print( "l" );

		for ( DWORD nItem = 0; nItem < (DWORD)m_nValue; nItem++, pNode++ )
		{
			(*pNode)->Encode( pBuffer );
		}

		pBuffer->Print( "e" );
	}
	else if ( m_nType == beDict )
	{
		CBENode** pNode = (CBENode**)m_pValue;

		pBuffer->Print( "d" );

		for ( DWORD nItem = 0; nItem < m_nValue; nItem++, pNode += 2 )
		{
			LPCSTR pszKey = (LPCSTR)pNode[1];
			size_t nKeyLength = strlen( pszKey );
			sprintf( szBuffer, "%zu:", nKeyLength );
			pBuffer->Print( szBuffer );
			pBuffer->Print( pszKey );
			(*pNode)->Encode( pBuffer );
		}

		pBuffer->Print( "e" );
	}
	else
	{
		ASSERT( FALSE );
	}
}

CString CBENode::GetString() const
{
	if ( m_nType != beString )
		return CString();

	LPCSTR szValue = (LPCSTR)m_pValue;

	// Decode from UTF-8
	CString str;
	if ( ::Decode( CP_UTF8, szValue, str ) )
		return str;

	// Use as is
	return CString( szValue );
}
