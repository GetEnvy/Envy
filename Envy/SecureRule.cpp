//
// SecureRule.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2012-2015
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
//#include "Settings.h"
#include "Envy.h"
#include "SecureRule.h"
#include "Security.h"
#include "WndSecurity.h"	// Column enum
#include "EnvyFile.h"
#include "QuerySearch.h"
#include "LiveList.h"
#include "RegExp.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CSecureRule construction

CSecureRule::CSecureRule(BOOL bCreate)
{
	m_nType		= srAddress;
	m_nAction	= BYTE( Security.m_bDenyPolicy ? srAccept : srDeny );
	m_nExpire	= srIndefinite;
	m_nToday	= 0;
	m_nEver		= 0;

	m_nIP[0]	= m_nIP[1] = m_nIP[2] = m_nIP[3] = 0;
	m_nMask[0]	= m_nMask[1] = m_nMask[2] = m_nMask[3] = 255;
	m_pContent	= NULL;
	m_nContentLength = 0;

	if ( bCreate ) CoCreateGuid( &m_pGUID );
}

CSecureRule::CSecureRule(const CSecureRule& pRule)
{
	m_nContentLength = 0;
	m_pContent = NULL;
	*this = pRule;
}

CSecureRule& CSecureRule::operator=(const CSecureRule& pRule)
{
	m_nType		= pRule.m_nType;
	m_nAction	= pRule.m_nAction;
	m_sComment	= pRule.m_sComment;
	m_pGUID		= pRule.m_pGUID;
	m_nExpire	= pRule.m_nExpire;
	m_nToday	= pRule.m_nToday;
	m_nEver		= pRule.m_nEver;
	m_nIP[0]	= pRule.m_nIP[0];
	m_nIP[1]	= pRule.m_nIP[1];
	m_nIP[2]	= pRule.m_nIP[2];
	m_nIP[3]	= pRule.m_nIP[3];
	m_nMask[0]	= pRule.m_nMask[0];
	m_nMask[1]	= pRule.m_nMask[1];
	m_nMask[2]	= pRule.m_nMask[2];
	m_nMask[3]	= pRule.m_nMask[3];

	delete [] m_pContent;
	m_pContent	= pRule.m_nContentLength ? new TCHAR[ pRule.m_nContentLength ] : NULL;
	m_nContentLength = pRule.m_nContentLength;
	CopyMemory( m_pContent, pRule.m_pContent, m_nContentLength * sizeof( TCHAR ) );

	return *this;
}

CSecureRule::~CSecureRule()
{
	if ( m_pContent ) delete [] m_pContent;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule remove and reset

void CSecureRule::Remove()
{
	Security.Remove( this );
}

void CSecureRule::Reset()
{
	m_nToday = m_nEver = 0;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule expiry check

BOOL CSecureRule::IsExpired(DWORD nNow, BOOL bSession) const
{
	if ( m_nExpire == srIndefinite ) return FALSE;
	if ( m_nExpire == srSession ) return bSession;
	return m_nExpire < nNow;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule match

BOOL CSecureRule::Match(const IN_ADDR* pAddress) const
{
	return m_nType == srAddress && pAddress &&
		( pAddress->s_addr & *(DWORD*)m_nMask ) == *(DWORD*)m_nIP;
}

BOOL CSecureRule::Match(LPCTSTR pszContent) const
{
	if ( m_nType == srAddress || m_nType == srContentRegExp || m_nType == srExternal || ! pszContent || ! m_pContent )
		return FALSE;

	if ( m_nType == srContentHash )	// urn:
		return pszContent[3] == L':' && _tcsistr( pszContent, (LPCTSTR)m_pContent ) != NULL;

	if ( m_nType == srSizeType )	// size:
		return pszContent[4] == L':' && _tcsistr( pszContent, (LPCTSTR)m_pContent ) != NULL;

	for ( LPCTSTR pszFilter = m_pContent; *pszFilter; )
	{
		if ( _tcsistr( pszContent, pszFilter ) != NULL )
		{
			if ( m_nType == srContentAny )
				return TRUE;
		}
		else // Not found
		{
			if ( m_nType == srContentAll )
				return FALSE;
		}

		pszFilter += _tcslen( pszFilter ) + 1;
	}

	return m_nType == srContentAll;
}

BOOL CSecureRule::Match(const CEnvyFile* pFile) const
{
	if ( m_nType == srAddress || m_nType == srContentRegExp || m_nType == srExternal || ! ( pFile && m_pContent ) )
		return FALSE;

	if ( m_nType == srSizeType )
	{
		if ( pFile->m_nSize == 0 || pFile->m_nSize == SIZE_UNKNOWN )
			return FALSE;

		LPCTSTR pszExt = PathFindExtension( (LPCTSTR)pFile->m_sName );
		if ( *pszExt != L'.' )
			return FALSE;
		pszExt++;

		CString strFilter = (LPCTSTR)m_pContent;
		strFilter = strFilter.Mid( 5 );		// "size:"
		if ( ! StartsWith( strFilter, pszExt ) )
			return FALSE;

		strFilter = strFilter.Mid( strFilter.Find( L':' ) + 1 );

		if ( strFilter.Find( L':' ) > 0 )
		{
			QWORD nLower, nUpper, nSize = pFile->m_nSize;
			_stscanf( (LPCTSTR)strFilter, L"%I64i:%I64i", &nLower, &nUpper );
			return nSize >= nLower && nSize <= nUpper;
		}
		if ( strFilter.Find( L'-' ) > 0 )
		{
			QWORD nLower, nUpper, nSize = pFile->m_nSize;
			_stscanf( (LPCTSTR)strFilter, L"%I64i-%I64i", &nLower, &nUpper );
			return nSize >= nLower && nSize <= nUpper;
		}

		CString strCompare;
		strCompare.Format( L"size:%s:%I64i", pszExt, pFile->m_nSize );
		return strCompare == (CString)m_pContent;
	}

	if ( m_nType == srContentHash )
	{
		LPCTSTR pszHash = m_pContent;
		if ( m_nContentLength < 30 || _tcsnicmp( pszHash, L"urn:", 4 ) != 0 )
			return FALSE;

		return
			( pFile->m_oSHA1  && pFile->m_oSHA1.toUrn() == pszHash ) ||		// Not Match( pFile->m_oSHA1.toUrn() )
			( pFile->m_oTiger && pFile->m_oTiger.toUrn() == pszHash ) ||
			( pFile->m_oED2K  && pFile->m_oED2K.toUrn() == pszHash ) ||
			( pFile->m_oBTH   && pFile->m_oBTH.toUrn() == pszHash ) ||
			( pFile->m_oMD5   && pFile->m_oMD5.toUrn() == pszHash );
	}

	return Match( pFile->m_sName );
}

BOOL CSecureRule::Match(const CQuerySearch* pQuery, const CString& strContent) const
{
	if ( m_nType != srContentRegExp || ! m_pContent )
		return FALSE;

	CString strFilter = pQuery->BuildRegExp( m_pContent );

	// Moved to QuerySearch:
	// Build a regular expression filter from the search query words
	// Returns an empty string if not applied or if the filter was invalid

	// Substitutes (Special-case tags):
	// <%>, <$>, <_>	Insert all query keywords.
	// <1>...<9>		Insert specific query keyword, 1-9.
	// <>				Insert next query keyword. (?)

	// Example Search Conversion:
	// Filter:			.*(<2><1>)|(<%>).*
	// "Word1 Word2":	.*(word2\s*word1\s*)|(word1\s*word2\s*).*

	//CString strFilter;
	//int nTotal = 0;
	//for ( LPCTSTR pszPattern = m_pContent; *pszPattern; pszPattern++ )
	//{
	//	if ( *pszPattern == '<' )
	//	{
	//		pszPattern++;
	//		bool bEnds = false;
	//		bool bNumber = false;
	//		bool bAll = ( *pszPattern == '%' || *pszPattern == '$' || *pszPattern == '_' || *pszPattern == '>' );
	//		if ( ! bAll ) bNumber = ( *pszPattern == '1' || *pszPattern == '2' || *pszPattern == '3' || *pszPattern == '4' ||
	//			*pszPattern == '5' || *pszPattern == '6' || *pszPattern == '7' || *pszPattern == '8' || *pszPattern == '9' );
	//		for ( ; *pszPattern; pszPattern++ )
	//		{
	//			if ( *pszPattern == '>' )
	//			{
	//				bEnds = true;
	//				break;
	//			}
	//		}
	//		if ( bEnds )	// Closed '>'
	//		{
	//			if ( bAll )	// <%>,<$>,<_>,<>
	//			{
	//				// Add all keywords at the "< >" position
	//				for ( CQuerySearch::const_iterator i = pQuery->begin(); i != pQuery->end(); ++i )
	//				{
	//					strFilter.AppendFormat( L"%s\\s*",
	//						CString( i->first, (int)( i->second ) ) );
	//				}
	//			}
	//			else if ( bNumber )	// <1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>
	//			{
	//				pszPattern--;	// Go back
	//				int nNumber = 0, nWord = 1;
	//
	//				// Numbers from 1 to 9, no more
	//				if ( _stscanf( &pszPattern[0], L"%i", &nNumber ) != 1 )
	//					nNumber = ++nTotal;
	//
	//				// Add specified keyword at the "< >" position
	//				for ( CQuerySearch::const_iterator i = pQuery->begin(); i != pQuery->end(); ++i, ++nWord )
	//				{
	//					if ( nWord == nNumber )
	//					{
	//						strFilter.AppendFormat( L"%s\\s*",
	//							CString( i->first, (int)( i->second ) ) );
	//						break;
	//					}
	//				}
	//				pszPattern++;	// Return to the last position
	//			}
	//		}
	//		else // No closing '>'
	//		{
	//			strFilter.Empty();
	//			break;
	//		}
	//	}
	//	else // No replacing
	//	{
	//		strFilter += *pszPattern;
	//	}
	//}

	if ( strFilter.IsEmpty() )
	{
		theApp.Message( MSG_DEBUG, L"Invalid RegExp filter:  %s", m_pContent );
		return FALSE;
	}

	return RegExp::Match( strFilter, strContent );
}

//////////////////////////////////////////////////////////////////////
// CSecureRule content list helpers

void CSecureRule::SetContentWords(const CString& strContent)
{
	if ( m_nType == srContentRegExp || m_nType == srExternal )
	{
		delete [] m_pContent;
		m_nContentLength = strContent.GetLength() + 2;
		LPTSTR pszContent = new TCHAR[ m_nContentLength ];
		_tcscpy_s( pszContent, m_nContentLength, strContent );
		m_pContent = pszContent;
		pszContent += strContent.GetLength();
		*pszContent++ = 0;
		*pszContent++ = 0;
		return;
	}

	LPTSTR pszContent = (LPTSTR)(LPCTSTR)strContent;
	int nTotalLength  = 3;
	CList< CString > pWords;

	int nStart = 0, nPos = 0;
	for ( ; *pszContent; nPos++, pszContent++ )
	{
		if ( *pszContent == ' ' || *pszContent == '\t' )
		{
			if ( nStart < nPos )
			{
				pWords.AddTail( strContent.Mid( nStart, nPos - nStart ) );
				nTotalLength += nPos - nStart + 1;
			}
			nStart = nPos + 1;
		}
	}

	if ( nStart < nPos )
	{
		pWords.AddTail( strContent.Mid( nStart, nPos - nStart ) );
		nTotalLength += nPos - nStart + 1;
	}

	if ( m_pContent )
	{
		delete [] m_pContent;
		m_pContent = NULL;
		m_nContentLength = 0;
	}

	if ( pWords.IsEmpty() ) return;

	m_pContent	= new TCHAR[ m_nContentLength = nTotalLength ];
	pszContent	= m_pContent;

	for ( POSITION pos = pWords.GetHeadPosition(); pos; )
	{
		CString strWord = pWords.GetNext( pos );
		CopyMemory( pszContent, (LPCTSTR)strWord, ( strWord.GetLength() + 1 ) * sizeof( TCHAR ) );
		pszContent += strWord.GetLength() + 1;
	}

	*pszContent++ = 0;
	*pszContent++ = 0;
}

CString CSecureRule::GetContentWords() const
{
	if ( m_pContent == NULL )
		return CString();

	if ( m_nType == srContentRegExp || m_nType == srExternal )
		return CString( m_pContent );

	ASSERT( m_nType != srAddress );

	CString strWords;
	for ( LPCTSTR pszFilter = m_pContent; *pszFilter; )
	{
		if ( ! strWords.IsEmpty() )
			strWords += L' ';
		strWords += pszFilter;

		pszFilter += _tcslen( pszFilter ) + 1;
	}

	return strWords;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule serialize

void CSecureRule::Serialize(CArchive& ar, int /*nVersion*/)
{
	if ( ar.IsStoring() )
	{
		ar << (int)m_nType;
		ar << m_nAction;
		ar << m_sComment;

		ar.Write( &m_pGUID, sizeof( GUID ) );

		ar << m_nExpire;
		ar << m_nEver;

		if ( m_nType == srAddress )
		{
			ar.Write( m_nIP, 4 );
			ar.Write( m_nMask, 4 );
		}
		else
		{
			ar << GetContentWords();
		}
	}
	else // Loading
	{
		int nType;
		ar >> nType;
		m_nType = (RuleType)nType;

		ar >> m_nAction;
		ar >> m_sComment;

		//if ( nVersion < 4 )
		//	CoCreateGuid( &m_pGUID );
		//else
			ReadArchive( ar, &m_pGUID, sizeof( GUID ) );

		ar >> m_nExpire;
		ar >> m_nEver;

		if ( m_nType == srAddress )
		{
			ReadArchive( ar, m_nIP, 4 );
			ReadArchive( ar, m_nMask, 4 );
			MaskFix();					// Make sure old rules are updated to new format (obsolete?)
		}
		else
		{
			//if ( nVersion < 5 )		// Map RuleType enum changes
			//{
			//	BYTE foo;
			//	ar >> foo;
			//	switch ( foo )
			//	{
			//	case 1:
			//		m_nType = srContentAll;
			//		break;
			//	case 2:
			//		m_nType = srContentRegExp;
			//		break;
			//	}
			//
			//	if ( nVersion < 3 )
			//	{
			//		for ( DWORD_PTR nCount = ar.ReadCount(); nCount > 0; nCount-- )
			//		{
			//			CString strWord;
			//			ar >> strWord;
			//			strTemp += L' ' + strWord;
			//		}
			//	}
			//}

			CString str;
			ar >> str;
			SetContentWords( str );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSecureRule XML

CXMLElement* CSecureRule::ToXML()
{
	CXMLElement* pXML = new CXMLElement( NULL, L"rule" );
	CString strValue;

	// Note: Insertion order is maintained in XML with a workaround for indeterminate CMap

	wchar_t szGUID[39];
	szGUID[ StringFromGUID2( *(GUID*)&m_pGUID, szGUID, 39 ) - 2 ] = 0;
	pXML->AddAttribute( L"guid", (CString)&szGUID[1] );

	if ( m_nType == srAddress )
	{
		pXML->AddAttribute( L"type", L"address" );

		strValue.Format( L"%lu.%lu.%lu.%lu",
			m_nIP[0], m_nIP[1], m_nIP[2], m_nIP[3] );
		pXML->AddAttribute( L"address", strValue );

		if ( *(DWORD*)m_nMask != 0xFFFFFFFF )
		{
			strValue.Format( L"%lu.%lu.%lu.%lu",
				m_nMask[0], m_nMask[1], m_nMask[2], m_nMask[3] );
			pXML->AddAttribute( L"mask", strValue );
		}
	}
	else if ( m_nType == srExternal )
	{
		pXML->AddAttribute( L"type", L"list" );
		pXML->AddAttribute( L"path", GetContentWords() );
	}
	else
	{
		switch ( m_nType )
		{
		//case srAddress:
		case srContentAny:
			strValue = L"any";
			break;
		case srContentAll:
			strValue = L"all";
			break;
		case srContentRegExp:
			strValue = L"regexp";
			break;
		case srContentHash:
			strValue = L"hash";
			break;
		case srSizeType:
			strValue = L"size";
			break;
		//case srExternal:
		//	strValue = L"list";
		//	break;
		default:
			strValue = L"null";
		}

		pXML->AddAttribute( L"type", L"content" );
		pXML->AddAttribute( L"content", GetContentWords() );
		pXML->AddAttribute( L"match", strValue );
	}

	if ( m_nExpire > srSession )
	{
		strValue.Format( L"%lu", m_nExpire );
		pXML->AddAttribute( L"expire", strValue );
	}
	else if ( m_nExpire == srSession )
	{
		pXML->AddAttribute( L"expire", L"session" );
	}

	pXML->AddAttribute( L"action",
		( m_nAction == srDeny ? L"deny" : m_nAction == srAccept ? L"accept" : L"null" ) ); 	// srNull?

	if ( ! m_sComment.IsEmpty() )
		pXML->AddAttribute( L"comment", m_sComment );

	return pXML;
}

BOOL CSecureRule::FromXML(const CXMLElement* pXML)
{
	m_sComment = pXML->GetAttributeValue( L"comment" );

	CString strValue, strType = pXML->GetAttributeValue( L"type" );

	if ( strType.CompareNoCase( L"address" ) == 0 )
	{
		int x[4] = {};

		m_nType = srAddress;

		strValue = pXML->GetAttributeValue( L"address" );
		if ( _stscanf( strValue, L"%i.%i.%i.%i", &x[0], &x[1], &x[2], &x[3] ) == 4 )
		{
			m_nIP[0] = (BYTE)x[0];
			m_nIP[1] = (BYTE)x[1];
			m_nIP[2] = (BYTE)x[2];
			m_nIP[3] = (BYTE)x[3];
		}

		strValue = pXML->GetAttributeValue( L"mask" );
		if ( _stscanf( strValue, L"%i.%i.%i.%i", &x[0], &x[1], &x[2], &x[3] ) == 4 )
		{
			m_nMask[0] = (BYTE)x[0];
			m_nMask[1] = (BYTE)x[1];
			m_nMask[2] = (BYTE)x[2];
			m_nMask[3] = (BYTE)x[3];
		}
	}
	else if ( strType.CompareNoCase( L"content" ) == 0 )
	{
		strValue = pXML->GetAttributeValue( L"match" ).MakeLower();
		if ( strValue == L"all" )
			m_nType = srContentAll;
		else if ( strValue == L"regexp" )
			m_nType = srContentRegExp;
		else if ( strValue == L"hash" )
			m_nType = srContentHash;
		else if ( strValue == L"size" )
			m_nType = srSizeType;
		else if ( strValue == L"list" || strValue == L"external" )
			m_nType = srExternal;
		else	// L"any"
			m_nType = srContentAny;

		SetContentWords( pXML->GetAttributeValue( L"content" ) );
		if ( m_pContent == NULL ) return FALSE;
	}
	else if ( strType.CompareNoCase( L"list" ) == 0 || strType.CompareNoCase( L"external" ) == 0 )
	{
		m_nType = srExternal;
		SetContentWords( pXML->GetAttributeValue( L"path" ) );
		if ( m_pContent == NULL ) return FALSE;
	}
	else
	{
		return FALSE;
	}

	strValue = pXML->GetAttributeValue( L"action" );

	if ( strValue.CompareNoCase( L"deny" ) == 0 || strValue.IsEmpty() )
		m_nAction = srDeny;
	else if ( strValue.CompareNoCase( L"accept" ) == 0 )
		m_nAction = srAccept;
	else if ( strValue.CompareNoCase( L"null" ) == 0 || strValue.CompareNoCase( L"none" ) == 0 )
		m_nAction = srNull;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( L"expire" );

	if ( strValue.CompareNoCase( L"indefinite" ) == 0 || strValue.CompareNoCase( L"none" ) == 0 )
		m_nExpire = srIndefinite;
	else if ( strValue.CompareNoCase( L"session" ) == 0 )
		m_nExpire = srSession;
	else
		_stscanf( strValue, L"%lu", &m_nExpire );

	MaskFix();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule Gnucelus strings (.net file export)

CString CSecureRule::ToGnucleusString() const
{
	CString strRule;

	if ( m_nType != srAddress ) return strRule;
	if ( m_nAction != srDeny ) return strRule;

	if ( *(DWORD*)m_nMask == 0xFFFFFFFF )
	{
		strRule.Format( L"%lu.%lu.%lu.%lu",
			m_nIP[0], m_nIP[1], m_nIP[2], m_nIP[3] );
	}
	else
	{
		BYTE nFrom[4] = {}, nTo[4] = {};

		for ( int nByte = 0; nByte < 4; nByte++ )
		{
			nFrom[ nByte ]	= m_nIP[ nByte ] & m_nMask[ nByte ];
			nTo[ nByte ]	= m_nIP[ nByte ] | ( ~m_nMask[ nByte ] );
		}

		strRule.Format( L"%lu.%lu.%lu.%lu-%lu.%lu.%lu.%lu",
			nFrom[0], nFrom[1], nFrom[2], nFrom[3],
			nTo[0], nTo[1], nTo[2], nTo[3] );
	}

	strRule += L':';
	strRule += m_sComment;
	strRule += L':';

	return strRule;
}

BOOL CSecureRule::FromGnucleusString(CString& str)
{
	int x[4] = {};

	int nPos = str.Find( L':' );
	if ( nPos < 1 ) return FALSE;

	CString strAddress = str.Left( nPos );
	str = str.Mid( nPos + 1 );

	if ( _stscanf( strAddress, L"%i.%i.%i.%i", &x[0], &x[1], &x[2], &x[3] ) != 4 )
		return FALSE;

	m_nIP[0] = (BYTE)x[0]; m_nIP[1] = (BYTE)x[1];
	m_nIP[2] = (BYTE)x[2]; m_nIP[3] = (BYTE)x[3];

	nPos = strAddress.Find( L'-' );

	if ( nPos >= 0 )
	{
		strAddress = strAddress.Mid( nPos + 1 );

		if ( _stscanf( strAddress, L"%i.%i.%i.%i", &x[0], &x[1], &x[2], &x[3] ) != 4 )
			return FALSE;

		for ( int nByte = 0; nByte < 4; nByte++ )
		{
			BYTE nTop = (BYTE)x[ nByte ], nBase = (BYTE)x[ nByte ];

			for ( BYTE nValue = m_nIP[ nByte ]; nValue < nTop; nValue++ )
			{
				m_nMask[ nByte ] &= ~( nValue ^ nBase );
			}
		}
	}

	m_nType		= srAddress;
	m_nAction	= srDeny;
	m_nExpire	= srIndefinite;
	m_sComment	= str.SpanExcluding( L":" );

	MaskFix();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule Netmask Fix

void CSecureRule::MaskFix()
{
	DWORD nNetwork = 0, nOldMask = 0, nNewMask = 0;

	for ( int nByte = 0; nByte < 4; nByte++ )		// Convert the byte arrays to dwords
	{
		BYTE nMaskByte = 0;
		BYTE nNetByte = 0;
		nNetByte = m_nIP[ nByte ];
		nMaskByte = m_nMask[ nByte ];
		for ( int nBits = 0; nBits < 8; nBits++ )
		{
			nNetwork <<= 1;
			if ( nNetByte & 0x80 )
				nNetwork |= 1;

			nNetByte <<= 1;

			nOldMask <<= 1;
			if ( nMaskByte & 0x80 )
				nOldMask |= 1;

			nMaskByte <<= 1;
		}
	}

	DWORD nTempMask = nOldMask;

	for ( int nBits = 0; nBits < 32; nBits++ )	// Get upper contiguous bits from subnet mask
	{
		if ( nTempMask & 0x80000000 )				// Check the high bit
		{
			nNewMask >>= 1;							// Shift mask down
			nNewMask |= 0x80000000;					// Put the bit on
		}
		else
		{
			break;									// Found a 0 so ignore the rest
		}
		nTempMask <<= 1;
	}

	if ( nNewMask != nOldMask )						// Set rule to expire if mask is invalid
	{
		m_nExpire = srSession;
		return;
	}

	nNetwork &= nNewMask;							// Do the & now so we don't have to each time there's a match

	for ( int nByte = 0; nByte < 4; nByte++ )		// Convert the dwords back to byte arrays
	{
		BYTE nNetByte = 0;
		for ( int nBits = 0; nBits < 8; nBits++ )
		{
			nNetByte <<= 1;
			if ( nNetwork & 0x80000000 )
				nNetByte |= 1;

			nNetwork <<= 1;
		}
		m_nIP[ nByte ] = nNetByte;
	}
}

void CSecureRule::ToList(CLiveList* pLiveList, int nCount, DWORD tNow) const
{
	// Was CSecurityWnd::Update()
	CLiveItem* pItem = pLiveList->Add( (LPVOID)this );

	pItem->SetImage( m_nAction );

	if ( m_nType == CSecureRule::srAddress )
	{
		if ( *(DWORD*)m_nMask == 0xFFFFFFFF )
			pItem->Format( COL_SECURITY_CONTENT, L"%u.%u.%u.%u",
				m_nIP[0], m_nIP[1], m_nIP[2], m_nIP[3] );
		else
			pItem->Format( COL_SECURITY_CONTENT, L"%u.%u.%u.%u/%u.%u.%u.%u",
				m_nIP[0], m_nIP[1], m_nIP[2], m_nIP[3],
				m_nMask[0], m_nMask[1], m_nMask[2], m_nMask[3] );
	}
	else
	{
		pItem->Set( COL_SECURITY_CONTENT, GetContentWords() );
	}

	switch ( m_nAction )
	{
	case CSecureRule::srNull:
		pItem->Set( COL_SECURITY_ACTION, LoadString( IDS_TIP_NA ) );
		break;
	case CSecureRule::srAccept:
		pItem->Set( COL_SECURITY_ACTION, LoadString( IDS_SECURITY_ACCEPT ) );
		break;
	case CSecureRule::srDeny:
		pItem->Set( COL_SECURITY_ACTION, LoadString( IDS_SECURITY_DENY ) );
		break;
	}

	switch ( m_nType )
	{
	case CSecureRule::srAddress:
		pItem->Set( COL_SECURITY_TYPE, L"IP" );
		break;
	case CSecureRule::srContentAny:
		pItem->Set( COL_SECURITY_TYPE, LoadString( IDS_SECURITY_ANY ) );
		break;
	case CSecureRule::srContentAll:
		pItem->Set( COL_SECURITY_TYPE, LoadString( IDS_SECURITY_ALL ) );
		break;
	case CSecureRule::srContentRegExp:
		pItem->Set( COL_SECURITY_TYPE, L"RegExp" );
		break;
	case CSecureRule::srContentHash:
		pItem->Set( COL_SECURITY_TYPE, LoadString( IDS_SECURITY_HASH ) );
		break;
	case CSecureRule::srSizeType:
		pItem->Set( COL_SECURITY_TYPE, LoadString( IDS_TIP_SIZE ) );
		break;
	case CSecureRule::srExternal:
		pItem->Set( COL_SECURITY_TYPE, LoadString( IDS_SECURITY_LIST ) );
		break;
	}

	if ( m_nExpire == CSecureRule::srIndefinite )
	{
		pItem->Set( COL_SECURITY_EXPIRES, LoadString( IDS_SECURITY_NOEXPIRE ) );		// "Never"
	}
	else if ( m_nExpire == CSecureRule::srSession )
	{
		pItem->Set( COL_SECURITY_EXPIRES, LoadString( IDS_SECURITY_SESSION ) );
	}
	else if ( m_nExpire >= tNow )
	{
		const DWORD nTime = ( m_nExpire - tNow );
		pItem->Format( COL_SECURITY_EXPIRES, L"%ud %uh %um",
			nTime / 86400u, (nTime % 86400u) / 3600u, ( nTime % 3600u ) / 60u );
		//pItem->Format( COL_EXPIRES, L"%i:%.2i:%.2i", nTime / 3600, ( nTime % 3600 ) / 60, nTime % 60 );
	}

	pItem->Format( COL_SECURITY_HITS, L"%u (%u)", m_nToday, m_nEver );
	pItem->Format( COL_SECURITY_NUM, L"%i", nCount );
	pItem->Set( COL_SECURITY_COMMENT, m_sComment );
}
