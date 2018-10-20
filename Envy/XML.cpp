//
// XML.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
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

#include "StdAfx.h"
#include "Strings.h"
#include "XML.h"

#ifdef DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CXMLNode construction

CXMLNode::CXMLNode(CXMLElement* pParent, LPCTSTR pszName)
	: m_nNode	( xmlNode )
	, m_pParent	( pParent )
{
	if ( pszName )
	{
		ASSERT( *pszName );
		m_sName = pszName;
	}
}

CXMLNode::~CXMLNode()
{
}

void CXMLNode::Delete()
{
	if ( this == NULL ) return;

	if ( m_pParent != NULL )
	{
		if ( m_nNode == xmlElement )
			m_pParent->RemoveElement( (CXMLElement*)this );
		else if ( m_nNode == xmlAttribute )
			m_pParent->RemoveAttribute( (CXMLAttribute*)this );
	}

	delete this;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode parsing

BOOL CXMLNode::ParseMatch(LPCTSTR& pszBase, LPCTSTR pszToken)
{
	LPCTSTR pszXML = pszBase;
	int nParse = 0;

	for ( ; IsSpace( *pszXML ); pszXML++, nParse++ );
	if ( ! *pszXML ) return FALSE;

	for ( ; *pszXML && *pszToken; pszXML++, pszToken++, nParse++ )
	{
		if ( *pszXML != *pszToken ) return FALSE;
	}

	pszBase += nParse;

	return TRUE;
}

BOOL CXMLNode::ParseIdentifier(LPCTSTR& pszBase, CString& strIdentifier)
{
	LPCTSTR pszXML = pszBase;
	int nParse = 0;

	while ( IsSpace( *pszXML ) )
	{
		pszXML++;
		nParse++;
	}
	if ( ! *pszXML ) return FALSE;

	int nIdentifier = 0;
	while ( *pszXML && ( _istalnum( *pszXML ) || *pszXML == ':' || *pszXML == '_' || *pszXML == '-' ) )
	{
		pszXML++;
		nIdentifier++;
	}
	if ( ! nIdentifier ) return FALSE;

	pszBase += nParse;
	_tcsncpy_s( strIdentifier.GetBuffer( nIdentifier + 1 ), nIdentifier + 1, pszBase, nIdentifier );
	strIdentifier.ReleaseBuffer( nIdentifier );
	pszBase += nIdentifier;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode serialize

void CXMLNode::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_sValue;
	}
	else // Loading
	{
		ar >> m_sName;
		ar >> m_sValue;
	}
}

//////////////////////////////////////////////////////////////////////
// CXMLNode string helper

void CXMLNode::UniformString(CString& str)
{
	// Non-alphanumeric characters which will not be ignored
	static LPCTSTR pszOK = L"'-&/,;#()";

	str.Trim();
	BOOL bSpace = TRUE;

	for ( int nPos = 0; nPos < str.GetLength(); nPos++ )
	{
		int nChar = (int)(unsigned short)str.GetAt( nPos );

		if ( nChar <= 32 )
		{
			if ( bSpace )
			{
				str = str.Left( nPos ) + str.Mid( nPos + 1 );
				nPos--;
			}
			else
			{
				if ( nChar != 32 ) str.SetAt( nPos, 32 );
				bSpace = TRUE;
			}
		}
		else if ( ! _istalnum( TCHAR( nChar ) ) && nChar < 0xC0 && _tcschr( pszOK, TCHAR( nChar ) ) == NULL )
		{
			if ( nPos == 0 || str.GetAt( nPos - 1 ) == ' ' )
				str = str.Left( nPos ) + str.Mid( nPos + 1 );
			else
			{
				LPTSTR pszTemp = _tcsninc( str, nPos );
				pszTemp[ 0 ] = ' ';
				bSpace = TRUE;
			}
		}
		else
		{
			bSpace = FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CXMLElement construction

CXMLElement::CXMLElement(CXMLElement* pParent, LPCTSTR pszName) : CXMLNode( pParent, pszName )
{
	m_nNode = xmlElement;
	m_bOrdered = TRUE;		// Retain Attribute inserton order (workaround)
}

CXMLElement::~CXMLElement()
{
	DeleteAllElements();
	DeleteAllAttributes();
}

CXMLElement* CXMLElement::AddElement(LPCTSTR pszName)
{
	CXMLElement* pElement = new CXMLElement( this, pszName );
	if ( ! pElement ) return NULL;			// Out of memory
	m_pElements.AddTail( pElement );
	return pElement;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement clone

CXMLElement* CXMLElement::Clone(CXMLElement* pParent) const
{
	CXMLElement* pClone = new CXMLElement( pParent, m_sName );
	if ( ! pClone ) return NULL;			// Out of memory

	for ( POSITION pos = GetAttributeIterator(); pos; )
	{
		CXMLAttribute* pAttribute = GetNextAttribute( pos )->Clone( pClone );
		if ( ! pAttribute ) return NULL;	// Out of memory

		CString strNameLower( pAttribute->m_sName );
		strNameLower.MakeLower();

		// Delete the old attribute if one exists
		CXMLAttribute* pExisting;
		if ( pClone->m_pAttributes.Lookup( strNameLower, pExisting ) )
			delete pExisting;

		pClone->m_pAttributes.SetAt( strNameLower, pAttribute );

		if ( m_bOrdered && ! pClone->m_pAttributesInsertion.Find( strNameLower ) )
			pClone->m_pAttributesInsertion.AddTail( strNameLower );		// Track output order workaround
	}

	for ( POSITION pos = GetElementIterator(); pos; )
	{
		const CXMLElement* pElement = GetNextElement( pos );
		pClone->m_pElements.AddTail( pElement->Clone( pClone ) );
	}

	ASSERT( ! pClone->m_sName.IsEmpty() );
	pClone->m_sValue = m_sValue;

	return pClone;
}

CXMLElement* CXMLElement::Prefix(const CString& sPrefix, CXMLElement* pParent) const
{
	CXMLElement* pCloned = Clone( pParent );
	if ( pCloned )
	{
		pCloned->SetName( sPrefix + pCloned->GetName() );

		for ( POSITION pos = pCloned->GetElementIterator(); pos; )
		{
			CXMLElement* pNode = pCloned->GetNextElement( pos );
			pNode->SetName( sPrefix + pNode->GetName() );
		}

		for ( POSITION pos = pCloned->GetAttributeIterator(); pos; )
		{
			CXMLAttribute* pNode = pCloned->GetNextAttribute( pos );
			pNode->SetName( sPrefix + pNode->GetName() );
		}
	}
	return pCloned;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement delete

void CXMLElement::DeleteAllElements()
{
	for ( POSITION pos = m_pElements.GetHeadPosition(); pos; )
	{
		delete m_pElements.GetNext( pos );
	}
	m_pElements.RemoveAll();
}

void CXMLElement::DeleteAllAttributes()
{
	for ( POSITION pos = m_pAttributes.GetStartPosition(); pos; )
	{
		CXMLAttribute* pAttribute;
		CString strName;

		m_pAttributes.GetNextAssoc( pos, strName, pAttribute );
		delete pAttribute;
	}
	m_pAttributes.RemoveAll();
	m_pAttributesInsertion.RemoveAll();		// Track output order workaround
}

//////////////////////////////////////////////////////////////////////
// CXMLElement to string

CString CXMLElement::ToString(BOOL bHeader, BOOL bNewline, BOOL bEncoding, TRISTATE bStandalone) const
{
	CString strXML;
	strXML.Preallocate( 256 );

	if ( bHeader )
	{
		strXML = L"<?xml version=\"1.0\"";

		if ( bEncoding )
			strXML.Append( _P( L" encoding=\"utf-8\"" ) );

		if ( bStandalone == TRI_TRUE )
			strXML.Append( _P( L" standalone=\"yes\"" ) );
		else if ( bStandalone == TRI_FALSE )
			strXML.Append( _P( L" standalone=\"no\"" ) );

		strXML.Append( _P( L"?>" ) );

		if ( bNewline )
			strXML.Append( _P( L"\r\n" ) );
	}

	ToString( strXML, bNewline );
//	ASSERT( strXML.GetLength() == int( _tcslen(strXML) ) );

	return strXML;
}

void CXMLElement::ToString(CString& strXML, BOOL bNewline) const
{
	// strXML += '<' + m_sName; Optimized:
	strXML.AppendChar( L'<' );
	strXML.Append( m_sName );

	POSITION pos = GetAttributeIterator();
	for ( ; pos; )
	{
		strXML.AppendChar( L' ' );
		const CXMLAttribute* pAttribute = GetNextAttribute( pos );
		pAttribute->ToString( strXML );
	}

	pos = GetElementIterator();

	if ( pos == NULL && m_sValue.IsEmpty() )
	{
		strXML.Append( _P( L"/>" ) );
		if ( bNewline )
			strXML.Append( _P( L"\r\n" ) );
		return;
	}

	strXML.AppendChar( L'>' );
	if ( bNewline && pos )
		strXML.Append( _P( L"\r\n" ) );

	while ( pos )
	{
		const CXMLElement* pElement = GetNextElement( pos );
		pElement->ToString( strXML, bNewline );
	}

	strXML += Escape( m_sValue );

	strXML.Append( _P( L"</" ) );
	strXML.Append( m_sName );
	strXML.AppendChar( L'>' );
	if ( bNewline )
		strXML.Append( _P( L"\r\n" ) );
}

//////////////////////////////////////////////////////////////////////
// CXMLElement from string

CXMLElement* CXMLElement::FromString(LPCTSTR pszXML, BOOL bHeader, CString* pEncoding)
{
	CXMLElement* pElement = NULL;
	LPCTSTR pszElement = NULL;

	try
	{
		if ( ParseMatch( pszXML, L"<?xml version=\"" ) )
		{
			pszElement = _tcsstr( pszXML, L"?>" );
			if ( ! pszElement )
				return FALSE;
			if ( pEncoding )
			{
				LPCTSTR pszEncoding = _tcsstr( pszXML, L"encoding=\"" );
				if ( pszEncoding && pszEncoding < pszElement )
				{
					pszEncoding += 10;
					LPCTSTR pszEncodingEnd = _tcschr( pszEncoding, L'\"' );
					if ( pszEncodingEnd && pszEncodingEnd < pszElement )
						pEncoding->Append( pszEncoding, (int)( pszEncodingEnd - pszEncoding ) );
				}
			}
			pszXML = pszElement + 2;
		}
		else if ( bHeader )
			return NULL;

		while ( ParseMatch( pszXML, L"<!--" ) )
		{
			pszElement = _tcsstr( pszXML, L"-->" );
			if ( ! pszElement || *pszElement != '-' )
				return FALSE;
			pszXML = pszElement + 3;
		}

		while ( ParseMatch( pszXML, L"<?xml" ) )
		{
			pszElement = _tcsstr( pszXML, L"?>" );
			if ( ! pszElement )
				return FALSE;
			pszXML = pszElement + 2;
		}

		if ( ParseMatch( pszXML, L"<!DOCTYPE" ) )
		{
			pszElement = _tcsstr( pszXML, L">" );
			if ( ! pszElement )
				return FALSE;
			pszXML = pszElement + 1;
		}

		while ( ParseMatch( pszXML, L"<!--" ) )
		{
			pszElement = _tcsstr( pszXML, L"-->" );
			if ( ! pszElement || *pszElement != '-' )
				return FALSE;
			pszXML = pszElement + 3;
		}

		pElement = new CXMLElement();

		if ( ! pElement->ParseString( pszXML ) )
		{
			delete pElement;
			pElement = NULL;
		}
	}
	catch ( CException* pException )
	{
		pException->Delete();
		delete pElement;
		pElement = NULL;
	}

	return pElement;
}

BOOL CXMLElement::ParseString(LPCTSTR& strXML)
{
	if ( ! ParseMatch( strXML, L"<" ) )
		return FALSE;

	if ( ! ParseIdentifier( strXML, m_sName ) )
		return FALSE;

	while ( ! ParseMatch( strXML, L">" ) )
	{
		if ( ParseMatch( strXML, L"/" ) )
			return ParseMatch( strXML, L">" );

		if ( ! *strXML )
			return FALSE;

		CXMLAttribute* pAttribute = new CXMLAttribute( this );

		if ( ! pAttribute || ! pAttribute->ParseString( strXML ) )
		{
			delete pAttribute;
			return FALSE;
		}

		CString strNameLower( pAttribute->m_sName );
		strNameLower.MakeLower();

		// Delete the old attribute if one exists
		CXMLAttribute* pExisting;
		if ( m_pAttributes.Lookup( strNameLower, pExisting ) )
			delete pExisting;

		m_pAttributes.SetAt( strNameLower, pAttribute );

		if ( m_bOrdered && ! m_pAttributesInsertion.Find( strNameLower ) )
			m_pAttributesInsertion.AddTail( strNameLower );		// Track output order workaround
	}

	CString strClose = L"</";
	strClose += m_sName + '>';

	for ( ;; )
	{
		if ( ! *strXML )
			return FALSE;

		LPCTSTR pszElement = _tcschr( strXML, '<' );
		if ( ! pszElement || *pszElement != '<' )
			return FALSE;

		if ( ParseMatch( strXML, L"<![CDATA[" ) )
		{
			pszElement = _tcsstr( strXML, L"]]>" );
			if ( ! pszElement || *pszElement != ']' )
				return FALSE;
			if ( ! m_sValue.IsEmpty() && m_sValue.Right( 1 ) != ' ' )
				m_sValue += ' ';
			m_sValue += Unescape( strXML, (int)( pszElement - strXML ) );
			pszElement += 3;
			strXML = pszElement;
		}

		if ( pszElement > strXML )
		{
			if ( ! m_sValue.IsEmpty() && m_sValue.Right( 1 ) != ' ' )
				m_sValue += ' ';
			m_sValue += Unescape( strXML, (int)( pszElement - strXML ) );
			strXML = pszElement;
		}

		if ( ParseMatch( strXML, strClose ) )
		{
			break;
		}
		else if ( ParseMatch( strXML, L"<!--" ) )
		{
			pszElement = _tcsstr( strXML, L"-->" );
			if ( ! pszElement || *pszElement != '-' )
				return FALSE;
			strXML = pszElement + 3;
		}
		else
		{
			CXMLElement* pElement = new CXMLElement( this );

			if ( pElement->ParseString( strXML ) )
			{
				m_pElements.AddTail( pElement );
			}
			else
			{
				delete pElement;
				return FALSE;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement from bytes

CXMLElement* CXMLElement::FromBytes(BYTE* pByte, DWORD nByte, BOOL bHeader)
{
	CString strXML;

	if ( nByte >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
	{
		nByte = nByte / 2 - 1;

		if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
		{
			pByte += 2;

			for ( DWORD nSwap = 0; nSwap < nByte; nSwap ++ )
			{
				register CHAR nTemp = pByte[ ( nSwap << 1 ) + 0 ];
				pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
				pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
			}
		}
		else
		{
			pByte += 2;
		}

		CopyMemory( strXML.GetBuffer( nByte ), pByte, nByte * sizeof( TCHAR ) );
		strXML.ReleaseBuffer( nByte );
	}
	else
	{
		if ( nByte >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF )
		{
			pByte += 3;
			nByte -= 3;
		}

		strXML = UTF8Decode( (LPCSTR)pByte, nByte );
	}

	return FromString( strXML, bHeader );
}

//////////////////////////////////////////////////////////////////////
// CXMLElement from file

CXMLElement* CXMLElement::FromFile(LPCTSTR pszPath, BOOL bHeader)
{
	HANDLE hFile = CreateFile( pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hFile == INVALID_HANDLE_VALUE ) return NULL;

	CXMLElement* pXML = FromFile( hFile, bHeader );

	CloseHandle( hFile );

	return pXML;
}

CXMLElement* CXMLElement::FromFile(HANDLE hFile, BOOL bHeader)
{
	DWORD nByte = GetFileSize( hFile, NULL );
	if ( nByte > 4096*1024 ) return FALSE;

	auto_array< BYTE > pByte( new BYTE[ nByte ] );

	if ( ! ReadFile( hFile, pByte.get(), nByte, &nByte, NULL ) ) return FALSE;

	return FromBytes( pByte.get(), nByte, bHeader );
}

//////////////////////////////////////////////////////////////////////
// CXMLElement equality

BOOL CXMLElement::Equals(CXMLElement* pXML) const
{
	if ( this == NULL || pXML == NULL ) return FALSE;
	if ( pXML == this ) return TRUE;

	if ( m_sName.CompareNoCase( pXML->m_sName ) != 0 ) return FALSE;
	if ( m_sValue != pXML->m_sValue ) return FALSE;

	if ( GetAttributeCount() != pXML->GetAttributeCount() ) return FALSE;
	if ( GetElementCount() != pXML->GetElementCount() ) return FALSE;

	for ( POSITION pos = GetAttributeIterator(); pos; )
	{
		CXMLAttribute* pAttribute1 = GetNextAttribute( pos );
		CXMLAttribute* pAttribute2 = pXML->GetAttribute( pAttribute1->m_sName );
		if ( pAttribute2 == NULL ) return FALSE;
		if ( ! pAttribute1->Equals( pAttribute2 ) ) return FALSE;
	}

	POSITION pos1 = GetElementIterator();
	POSITION pos2 = pXML->GetElementIterator();

	for ( ; pos1 && pos2; )
	{
		CXMLElement* pElement1 = GetNextElement( pos1 );
		CXMLElement* pElement2 = pXML->GetNextElement( pos2 );
		if ( pElement1 == NULL || pElement2 == NULL ) return FALSE;
		if ( ! pElement1->Equals( pElement2 ) ) return FALSE;
	}

	if ( pos1 != NULL || pos2 != NULL ) return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement Metadata merge

BOOL CXMLElement::Merge(const CXMLElement* pInput, BOOL bOverwrite)
{
	if ( ! this || ! pInput ) return FALSE;
	if ( this == pInput ) return TRUE;

	TRACE( "Merging   XML: %s\n", (LPCSTR)CT2A( ToString( FALSE, FALSE ) ) );
	TRACE( "      and XML: %s\n", (LPCSTR)CT2A( pInput->ToString( FALSE, FALSE ) ) );

	if ( m_sName.CompareNoCase( pInput->m_sName ) != 0 )
	{
		TRACE( "Failed to merge XML due different schemes \"%s\" and \"%s\".\n", (LPCSTR)CT2A( m_sName ), (LPCSTR)CT2A( pInput->m_sName ) );
		return FALSE;
	}

	BOOL bChanged = FALSE;

	for ( POSITION pos = pInput->GetElementIterator(); pos; )
	{
		const CXMLElement* pElement = pInput->GetNextElement( pos );
		CXMLElement* pTarget = GetElementByName( pElement->m_sName );

		if ( pTarget == NULL )
		{
			AddElement( pElement->Clone() );
			bChanged = TRUE;
		}
		else if ( pTarget->Merge( pElement, bOverwrite ) )
		{
			bChanged = TRUE;
		}
	}

	for ( POSITION pos = pInput->GetAttributeIterator(); pos; )
	{
		CXMLAttribute* pAttribute = pInput->GetNextAttribute( pos );
		CXMLAttribute* pTarget = GetAttribute( pAttribute->m_sName );

		if ( pTarget == NULL )
		{
			AddAttribute( pAttribute->Clone() );
			bChanged = TRUE;
		}
		else if ( bOverwrite && ! pTarget->Equals( pAttribute ) )
		{
			pTarget->SetValue( pAttribute->GetValue() );
			bChanged = TRUE;
		}
	}

	if ( bChanged )
		TRACE( "resulting XML: %s\n", (LPCSTR)CT2A( ToString( FALSE, FALSE ) ) );
	else
		TRACE( "resulting XML unchanged.\n" );

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement recursive word accumulation

CString CXMLElement::GetRecursiveWords() const
{
	CString strWords;

	AddRecursiveWords( strWords );
	strWords.Trim();

	return strWords;
}

void CXMLElement::AddRecursiveWords(CString& strWords) const
{
	for ( POSITION pos = GetAttributeIterator(); pos; )
	{
		CXMLAttribute* pAttribute = GetNextAttribute( pos );
		CString strText = pAttribute->GetName();

		if ( strText.Find( L':' ) >= 0 ) continue;
		if ( strText.CompareNoCase( L"SHA1" ) == 0 ) continue;	// NOTE: Envy/Shareaza Specific

		if ( ! strWords.IsEmpty() ) strWords += ' ';
		strWords += pAttribute->GetValue();
	}

	for ( POSITION pos = GetElementIterator(); pos; )
	{
		GetNextElement( pos )->AddRecursiveWords( strWords );
	}

	if ( ! m_sValue.IsEmpty() )
	{
		if ( ! strWords.IsEmpty() )
			strWords += L' ';
		strWords += m_sValue;
	}
}

//////////////////////////////////////////////////////////////////////
// CXMLElement serialize

void CXMLElement::Serialize(CArchive& ar)
{
	CXMLNode::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( GetAttributeCount() );

		for ( POSITION pos = GetAttributeIterator(); pos; )
		{
			GetNextAttribute( pos )->Serialize( ar );
		}

		ar.WriteCount( GetElementCount() );

		for ( POSITION pos = GetElementIterator(); pos; )
		{
			GetNextElement( pos )->Serialize( ar );
		}
	}
	else // Loading
	{
		for ( int nCount = (int)ar.ReadCount(); nCount > 0; nCount-- )
		{
			CXMLAttribute* pAttribute = new CXMLAttribute( this );
			pAttribute->Serialize( ar );

			// Skip attribute if name is missing
			if ( pAttribute->m_sName.IsEmpty() )
			{
				delete pAttribute;
				continue;
			}

			CString strNameLower( pAttribute->m_sName );
			strNameLower.MakeLower();

			// Delete the old attribute if one exists
			CXMLAttribute* pExisting;
			if ( m_pAttributes.Lookup( strNameLower, pExisting ) )
				delete pExisting;

			m_pAttributes.SetAt( strNameLower, pAttribute );

			if ( m_bOrdered && ! m_pAttributesInsertion.Find( strNameLower ) )
				m_pAttributesInsertion.AddTail( strNameLower );		// Track output order workaround
		}

		for ( int nCount = (int)ar.ReadCount(); nCount > 0; nCount-- )
		{
			CXMLElement* pElement = new CXMLElement( this );
			pElement->Serialize( ar );
			m_pElements.AddTail( pElement );
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CXMLAttribute construction

LPCTSTR CXMLAttribute::xmlnsSchema		= L"http://www.w3.org/2001/XMLSchema";
LPCTSTR CXMLAttribute::xmlnsInstance	= L"http://www.w3.org/2001/XMLSchema-instance";
LPCTSTR CXMLAttribute::schemaName		= L"xsi:noNamespaceSchemaLocation";

CXMLAttribute::CXMLAttribute(CXMLElement* pParent, LPCTSTR pszName) : CXMLNode( pParent, pszName )
{
	m_nNode = xmlAttribute;
}

CXMLAttribute::~CXMLAttribute()
{
}

CXMLAttribute* CXMLElement::AddAttribute(LPCTSTR pszName, LPCTSTR pszValue)
{
	ASSERT( pszName && *pszName );

	CXMLAttribute* pAttribute = GetAttribute( pszName );

	if ( ! pAttribute )
	{
		pAttribute = new CXMLAttribute( this, pszName );
		if ( ! pAttribute )
			return NULL;

		CString strNameLower( pszName );
		strNameLower.MakeLower();

		// Delete the old attribute if one exists
		CXMLAttribute* pExisting;
		if ( m_pAttributes.Lookup( strNameLower, pExisting ) )
			delete pExisting;

		m_pAttributes.SetAt( strNameLower, pAttribute );

		if ( m_bOrdered && ! m_pAttributesInsertion.Find( strNameLower ) )
			m_pAttributesInsertion.AddTail( strNameLower );		// Track output order workaround
	}

	if ( pszValue )
		pAttribute->SetValue( pszValue );

	return pAttribute;
}

CXMLAttribute* CXMLElement::AddAttribute(LPCTSTR pszName, __int64 nValue)
{
	CString strValue;
	strValue.Format( L"%I64d", nValue );
	return AddAttribute( pszName, strValue );
}

CXMLAttribute* CXMLElement::AddAttribute(CXMLAttribute* pAttribute)
{
	if ( pAttribute->m_pParent ) return NULL;
	CString strNameLower( pAttribute->m_sName );
	strNameLower.MakeLower();

	// Delete the old attribute if one exists
	CXMLAttribute* pExisting;
	if ( m_pAttributes.Lookup( strNameLower, pExisting ) )
		delete pExisting;

	if ( m_bOrdered && ! m_pAttributesInsertion.Find( strNameLower ) )
		m_pAttributesInsertion.AddTail( strNameLower );		// Track output order workaround

	m_pAttributes.SetAt( strNameLower, pAttribute );
	pAttribute->m_pParent = this;
	return pAttribute;
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute clone

CXMLAttribute* CXMLAttribute::Clone(CXMLElement* pParent) const
{
	CXMLAttribute* pClone = new CXMLAttribute( pParent, m_sName );
	if ( ! pClone ) return NULL;	// Out of memory
	ASSERT( ! pClone->m_sName.IsEmpty() );
	pClone->m_sValue = m_sValue;
	return pClone;
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute to string

void CXMLAttribute::ToString(CString& strXML) const
{
	strXML += m_sName + L"=\"" + Escape( m_sValue ) + L'\"';
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute from string

BOOL CXMLAttribute::ParseString(LPCTSTR& strXML)
{
	if ( ! ParseIdentifier( strXML, m_sName ) )
		return FALSE;
	if ( ! ParseMatch( strXML, L"=" ) )
		return FALSE;

	if ( ParseMatch( strXML, L"\"" ) )
	{
		LPCTSTR pszQuote = _tcschr( strXML, '\"' );
		if ( ! pszQuote || *pszQuote != '\"' )
			return FALSE;

		m_sValue = Unescape( strXML, (int)( pszQuote - strXML ) );
		strXML = pszQuote;

		return ParseMatch( strXML, L"\"" );
	}

	if ( ParseMatch( strXML, L"'" ) )
	{
		LPCTSTR pszQuote = _tcschr( strXML, '\'' );
		if ( ! pszQuote || *pszQuote != '\'' )
			return FALSE;

		m_sValue = Unescape( strXML, (int)( pszQuote - strXML ) );
		strXML = pszQuote;

		return ParseMatch( strXML, L"\'" );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute equality

BOOL CXMLAttribute::Equals(CXMLAttribute* pXML) const
{
	if ( this == NULL || pXML == NULL ) return FALSE;
	if ( pXML == this ) return TRUE;

	if ( m_sName.CompareNoCase( pXML->m_sName ) != 0 ) return FALSE;
	if ( m_sValue != pXML->m_sValue ) return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute serialize

void CXMLAttribute::Serialize(CArchive& ar)
{
	CXMLNode::Serialize( ar );
}
