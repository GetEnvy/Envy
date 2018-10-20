//
// VendorCache.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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
#include "Settings.h"
#include "Envy.h"
#include "VendorCache.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CVendorCache VendorCache;


//////////////////////////////////////////////////////////////////////
// CVendorCache construction

CVendorCache::CVendorCache() :
	m_pNull( new CVendor() )
{
	// Experimental values
	m_pCodeMap.InitHashTable( 83 );
	m_pNameMap.InitHashTable( 83 );
}

CVendorCache::~CVendorCache()
{
	delete m_pNull;
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CVendorCache lookup

CVendor* CVendorCache::LookupByName(LPCTSTR pszName) const
{
	ASSERT( pszName );

	if ( ! pszName || ! *pszName )
		return NULL;

	CString strName( pszName );
	int n = strName.FindOneOf( L"/ \t\r\n\\" );
	if ( n > 0 )
		strName = strName.Left( n );
	strName.MakeLower();

	CVendor* pVendor;
	if ( m_pNameMap.Lookup( strName, pVendor ) )
		return pVendor;

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CVendorCache clear

void CVendorCache::Clear()
{
	CVendor* pItem;
	CString strCode;
	for ( POSITION pos = m_pCodeMap.GetStartPosition(); pos; )
	{
		m_pCodeMap.GetNextAssoc( pos, strCode, pItem );
		delete pItem;
	}
	m_pCodeMap.RemoveAll();
	m_pNameMap.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CVendorCache load

BOOL CVendorCache::Load()
{
	const CString strPath = Settings.General.DataPath + L"Vendors.xml";

	CXMLElement* pXML = CXMLElement::FromFile( strPath, TRUE );
	BOOL bSuccess = FALSE;

	if ( pXML != NULL )
	{
		bSuccess = LoadFrom( pXML );
		delete pXML;
		if ( ! bSuccess )
			theApp.Message( MSG_ERROR, L"Invalid Vendors.xml file" );
	}
	else
		theApp.Message( MSG_ERROR, L"Missing Vendors.xml file" );

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CVendorCache load internal

BOOL CVendorCache::LoadFrom(CXMLElement* pXML)
{
	if ( ! pXML->IsNamed( L"vendorCache" ) ) return FALSE;

	CVendor* pFoo;
	for ( POSITION pos = pXML->GetElementIterator(); pos; )
	{
		CXMLElement* pKey = pXML->GetNextElement( pos );

		if ( pKey->IsNamed( L"vendor" ) )
		{
			CVendor* pVendor = new CVendor();

			if ( pVendor->LoadFrom( pKey ) )
			{
				if ( m_pCodeMap.Lookup( pVendor->m_sCode, pFoo ) )
				{
					theApp.Message( MSG_ERROR, L"Duplicate Vendors.xml code for \"%s\"",
						(LPCTSTR)pVendor->m_sCode );
					delete pVendor;
				}
				else
				{
					m_pCodeMap.SetAt( pVendor->m_sCode, pVendor );
					m_pNameMap.SetAt( CString( pVendor->m_sName ).MakeLower(), pVendor );
				}
			}
			else
			{
				theApp.Message( MSG_ERROR, L"Invalid Vendors.xml entry" );
				delete pVendor;
			}
		}
	}

	return m_pCodeMap.GetCount() > 0;
}

bool CVendorCache::IsExtended(LPCTSTR pszCode) const
{
	ASSERT( pszCode );

	if ( ! *pszCode || *pszCode == L'µ' || _tcsicmp( pszCode, L"BitTorrent" ) == 0 )
		return false;

	// Find by product name (Server or User-Agent HTTP-headers)
	CVendor* pVendor = LookupByName( pszCode );
	if ( ! pVendor )
		pVendor = Lookup( pszCode );	// Find by vendor code

	if ( pVendor )
		return pVendor->m_bExtended;

	// Unknown vendor code
	return false;
}


//////////////////////////////////////////////////////////////////////
// CVendor construciton

CVendor::CVendor()
	: m_bChatFlag	( false )
	, m_bBrowseFlag	( false )
	, m_bExtended	( false )
{
}

CVendor::CVendor(LPCTSTR pszCode)
	: m_sCode		( pszCode )
	, m_sName		( pszCode )
	, m_bChatFlag	( false )
	, m_bBrowseFlag	( false )
	, m_bExtended	( false )
{
	if ( m_sCode.GetLength() > 4 )
		m_sCode = m_sCode.Left( 4 );
	else
		while ( m_sCode.GetLength() < 4 )
			m_sCode += L' ';
}

CVendor::~CVendor()
{
}

//////////////////////////////////////////////////////////////////////
// CVendor load

BOOL CVendor::LoadFrom(CXMLElement* pXML)
{
	m_sCode = pXML->GetAttributeValue( L"code" );
	if ( m_sCode.GetLength() != 4 ) return FALSE;

	for ( POSITION pos = pXML->GetElementIterator(); pos; )
	{
		CXMLElement* pKey = pXML->GetNextElement( pos );

		if ( pKey->IsNamed( L"title" ) )
		{
			if ( ! m_sName.IsEmpty() ) return FALSE;
			m_sName = pKey->GetValue();
		}
		else if ( pKey->IsNamed( L"link" ) )
		{
			if ( ! m_sLink.IsEmpty() ) return FALSE;
			m_sLink = pKey->GetValue();
		}
		else if ( pKey->IsNamed( L"capability" ) )
		{
			const CString strCap = pKey->GetAttributeValue( L"name" ).MakeLower();

			if ( strCap == L"chatflag" )
				m_bChatFlag = true;
			else if ( strCap == L"htmlhostbrowse" || strCap == L"browseflag" )
				m_bBrowseFlag = true;
			else if ( strCap == L"extended" )
				m_bExtended = true;
			// ToDo: Other flags? g2,g1,ed2k,dc,bt,etc.
		}
	}

	return ! m_sName.IsEmpty();
}
