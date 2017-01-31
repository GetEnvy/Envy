//
// BENode.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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
#include "BENode.h"
#include "Buffer.h"

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
	, m_nSize		( 0 )
	, m_nPosition	( 0 )
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
			for ( ; m_nValue-- ; pNode++ )
				delete *pNode;
			delete [] (CBENode**)m_pValue;
		}
		else if ( m_nType == beDict )
		{
			CBENode** pNode = (CBENode**)m_pValue;
			for ( ; m_nValue-- ; pNode++ )
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

CBENode* CBENode::Add(LPCBYTE pKey, size_t nKey)
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
		ASSERT( pKey != NULL );		//&& nKey > 0
		break;
	default:
		ASSERT( FALSE );
		break;
	}

//	unique_ptr< CBENode > pNew( new CBENode );
//	CBENode* pNew_ = pNew.get();

	CAutoPtr< CBENode > pNew( new CBENode );
	if ( ! pNew )
		return NULL;	// Out of memory

	CBENode* pNew_ = pNew;

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

	for ( DWORD nNode = (DWORD)m_nValue ; nNode ; nNode--, pNode += 2 )
	{
		if ( strcmp( pszKey, (LPCSTR)pNode[1] ) == 0 )
			return *pNode;
	}

	return NULL;
}

CBENode* CBENode::GetNode(const LPBYTE pKey, int nKey) const
{
	if ( m_nType != beDict ) return NULL;

	CBENode** pNode = (CBENode**)m_pValue;

	for ( DWORD nNode = (DWORD)m_nValue ; nNode ; nNode--, pNode += 2 )
	{
		if ( memcmp( pKey, (LPBYTE)pNode[1], nKey ) == 0 )
			return *pNode;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////
// CBENode Extract a string from a node under this one. (Checks both normal and .utf-8)

CString CBENode::GetStringFromSubNode(LPCSTR pszKey, UINT nEncoding) const
{
	CString strValue;

	// Open the supplied node + .utf-8
	const CBENode* pSubNodeUTF8 = GetNode( CStringA( pszKey ) + ".utf-8" );
	if ( pSubNodeUTF8 && pSubNodeUTF8->m_nType == CBENode::beString )
	{
		strValue = pSubNodeUTF8->GetString();
	}
	else
	{
		// Open the supplied sub-node
		const CBENode* pSubNode = GetNode( pszKey );
		if ( pSubNode && pSubNode->m_nType == CBENode::beString )
		{
			strValue = pSubNode->GetString();
			if ( strValue.IsEmpty() )
				strValue = pSubNode->DecodeString( nEncoding );
		}
	}

	return strValue;
}

// CBENode Extract a string from a list/dictionary

CString CBENode::GetStringFromSubNode(int nItem, UINT nEncoding) const
{
	CString strValue;

	// Check we are a list / dictionary type
	if ( m_nType == beList || m_nType == beDict )
	{
		// Open the supplied list/dictionary item
		const CBENode* pSubNode = GetNode( nItem );
		if ( pSubNode && pSubNode->m_nType == CBENode::beString )
		{
			strValue = pSubNode->GetString();
			if ( strValue.IsEmpty() )
				strValue = pSubNode->DecodeString( nEncoding );
		}
	}

	return strValue;
}

//////////////////////////////////////////////////////////////////////
// CBENode SHA1 computation

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

//////////////////////////////////////////////////////////////////////
// CBENode encoding

void CBENode::Encode(CBuffer* pBuffer) const
{
	CHAR szBuffer[64];

	switch ( m_nType )
	{
	case beString:
		pBuffer->Print( szBuffer, sprintf_s( szBuffer, _countof( szBuffer ), "%I64i:", m_nValue ) );
		pBuffer->Add( m_pValue, (DWORD)m_nValue );
		break;

	case beInt:
		pBuffer->Print( szBuffer, sprintf_s( szBuffer, _countof( szBuffer ), "i%I64ie", m_nValue ) );
		break;

	case beList:
		pBuffer->Print( _P("l") );

		{
			CBENode** pNode = (CBENode**)m_pValue;

			for ( DWORD nItem = 0 ; nItem < (DWORD)m_nValue ; nItem++, pNode++ )
			{
				(*pNode)->Encode( pBuffer );
			}
		}

		pBuffer->Print( _P("e") );
		break;

	case beDict:
		pBuffer->Print( _P("d") );

		{
			CBENode** pNode = (CBENode**)m_pValue;

			for ( DWORD nItem = 0 ; nItem < (DWORD)m_nValue ; nItem++, pNode += 2 )
			{
				LPCSTR pszKey = (LPCSTR)pNode[1];
				size_t nKeyLength = strlen( pszKey );
				pBuffer->Print( szBuffer, sprintf_s( szBuffer, _countof( szBuffer ), "%u:", (DWORD)nKeyLength ) );
				pBuffer->Print( pszKey, nKeyLength );
				(*pNode)->Encode( pBuffer );
			}
		}

		pBuffer->Print( _P("e") );
		break;

	default:
		ASSERT( FALSE );
	}
}

const CString CBENode::Encode() const
{
	CString strOutput;

	switch ( m_nType )
	{
	case beNull:
		strOutput = L"(null)";
		break;

	case beString:
		strOutput = L'\"';
		{
			const DWORD nLen = (DWORD)min( m_nValue, 100ll );
			for ( DWORD n = 0 ; n < nLen ; n++ )
			{
				strOutput += ( ( ( (LPSTR)m_pValue )[ n ] < ' ' ) ? '.' : ( (LPSTR)m_pValue )[ n ] );	// Not L''
			}
		}
		strOutput += L'\"';
		strOutput.AppendFormat( L"[%I64i]", m_nValue );
		break;

	case beInt:
		strOutput.Format( L"%I64i", m_nValue );
		break;

	case beList:
		strOutput = L"{ ";
		{
			CBENode** pNode = (CBENode**)m_pValue;
			for ( DWORD n = 0 ; n < (DWORD)m_nValue ; n++, pNode++ )
			{
				if ( n ) strOutput += L", ";
				strOutput += (*pNode)->Encode();
			}
		}
		strOutput += L" }";
		break;

	case beDict:
		strOutput = L"{ ";
		{
			CBENode** pNode = (CBENode**)m_pValue;
			for ( __int64 n = 0 ; n < m_nValue ; n++, pNode += 2 )
			{
				if ( n ) strOutput += L", ";
				strOutput.AppendFormat( L"%hs=", (LPCSTR)pNode[ 1 ] );
				strOutput += (*pNode)->Encode();
			}
		}
		strOutput += L" }";
		break;

	//default:
	//	ASSERT( FALSE );
	}

	return strOutput;
}

#define INC(x) { pInput += (x); nInput -= (x); }

//////////////////////////////////////////////////////////////////////
// CBENode decoding

CBENode* CBENode::Decode(const CBuffer* pBuffer, DWORD *pnReaden)
{
	return Decode( pBuffer->m_pBuffer, pBuffer->m_nLength, pnReaden );
}

CBENode* CBENode::Decode(LPCBYTE pBuffer, DWORD nLength, DWORD *pnReaden)
{
	if ( pnReaden )
		*pnReaden = 0;

	try
	{
		CAutoPtr< CBENode > pNode( new CBENode() );
		if ( ! pNode )
			return NULL;	// Out of memory

		LPCBYTE pInput = pBuffer;
		DWORD nInput = nLength;

		// IIS based trackers may insert unneeded EOL at the beginning
		// of the torrent files or scrape responses, due to IIS bug.  Skip it.
		if ( nInput > 1 && pInput[0] == '\r' && pInput[1] == '\n' )
			INC( 2 );

		pNode->Decode( pInput, nInput, nInput );

		if ( pnReaden )
			*pnReaden = nLength - nInput;

		return pNode.Detach();
	}
	catch ( CException* pException )
	{
		pException->Delete();
		return NULL;
	}
}

void CBENode::Decode(LPCBYTE& pInput, DWORD& nInput, DWORD nSize)
{
	ASSERT( m_nType == beNull );

	if ( nInput < 1 )
		AfxThrowUserException();

	m_nPosition = nSize - nInput;

	if ( *pInput == 'i' )
	{
		INC( 1 );

		DWORD nSeek = 1;
		for ( ; nSeek < 40 ; nSeek++ )
		{
			if ( nSeek >= nInput )
				AfxThrowUserException();
			if ( pInput[nSeek] == 'e' )
				break;
		}

		if ( nSeek >= 40 ) AfxThrowUserException();

		if ( ! atoin( (LPCSTR)pInput, nSeek, m_nValue ) )
			AfxThrowUserException();

		INC( nSeek + 1 );
		m_nType = beInt;
	}
	else if ( *pInput == 'l' )
	{
		m_nType = beList;
		INC( 1 );

		for ( ;; )
		{
			if ( nInput < 1 )
				AfxThrowUserException();
			if ( *pInput == 'e' )
				break;
			Add()->Decode( pInput, nInput, nSize );
		}

		INC( 1 );
	}
	else if ( *pInput == 'd' )
	{
		m_nType = beDict;
		INC( 1 );

		for ( ;; )
		{
			if ( nInput < 1 )
				AfxThrowUserException();
			if ( *pInput == 'e' )
				break;

			int nLen = DecodeLen( pInput, nInput );
			LPCBYTE pKey = pInput;
			INC( nLen );
			Add( pKey, nLen )->Decode( pInput, nInput, nSize );
		}

		INC( 1 );
	}
	else if ( *pInput >= '0' && *pInput <= '9' )
	{
		m_nType  = beString;
		m_nValue = DecodeLen( pInput, nInput );
		m_pValue = new CHAR[ (DWORD)m_nValue + 1 ];
		CopyMemory( m_pValue, pInput, (DWORD)m_nValue );
		((LPBYTE)m_pValue)[ (DWORD)m_nValue ] = 0;

		INC( (DWORD)m_nValue );
	}
	else
	{
		AfxThrowUserException();
	}

	m_nSize = nSize - nInput - m_nPosition;
}

int CBENode::DecodeLen(LPCBYTE& pInput, DWORD& nInput)
{
	DWORD nSeek = 1;
	for ( ; nSeek < 32 ; nSeek++ )
	{
		if ( nSeek >= nInput )
			AfxThrowUserException();
		if ( pInput[ nSeek ] == ':' )
			break;
	}

	if ( nSeek >= 32 )
		AfxThrowUserException();

	__int64 nLen;
	if ( ! atoin( (LPCSTR)pInput, nSeek, nLen ) || nLen < 0 )
		AfxThrowUserException();
	INC( nSeek + 1 );

	if ( nInput < (DWORD)nLen )
		AfxThrowUserException();

	return (int)nLen;
}

CString CBENode::GetString() const
{
	if ( m_nType != beString )
		return CString();

	CString str;
	LPCSTR szValue = (LPCSTR)m_pValue;

	// Decode from UTF-8
	if ( ::Decode( CP_UTF8, szValue, str ) )
		return str;

	// Use as is
	return CString( szValue );
}

//#ifdef HASHES_HPP_INCLUDED

bool CBENode::GetString(Hashes::BtGuid& oGUID) const
{
	if ( m_nType != beString || m_nValue != oGUID.byteCount )
		return false;

	CopyMemory( &oGUID[0], m_pValue, oGUID.byteCount );
	oGUID.validate();

	return true;
}
//#endif // HASHES_HPP_INCLUDED

CString CBENode::DecodeString(UINT nCodePage) const
{
	if ( m_nType != beString )
		return CString();

	CString str;
	LPCSTR szValue = (LPCSTR)m_pValue;

	// Use the torrent code page (if present)
	if ( nCodePage != CP_ACP )
	{
		if ( ::Decode( nCodePage, szValue, str ) )
			return str;
	}

	// Try the user-specified code page if it's set (previously Settings.BitTorrent.TorrentCodePage)
	if ( m_nDefaultCP != CP_ACP && m_nDefaultCP != nCodePage )
	{
		if ( ::Decode( m_nDefaultCP, szValue, str ) )
			return str;
	}

	// Try OEM
	UINT nOEMCodePage = GetOEMCP();
	if ( nOEMCodePage != nCodePage && nOEMCodePage != m_nDefaultCP )
	{
		if ( ::Decode( nOEMCodePage, szValue, str ) )
			return str;
	}

	// Try ACP. (Should convert anything, but badly)
	if ( ::Decode( CP_ACP, szValue, str ) )
		return str;

	// Use as is
	return CString( szValue );
}
