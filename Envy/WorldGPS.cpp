//
// WorldGPS.cpp
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
#include "Settings.h"
#include "Envy.h"
#include "WorldGPS.h"
#include "Buffer.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CWorldGPS construction

CWorldGPS::CWorldGPS()
	: m_pCountry ( NULL )
	, m_nCountry ( 0 )
{
}

CWorldGPS::~CWorldGPS()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CWorldGPS load

BOOL CWorldGPS::Load()
{
	Clear();

	CFile pFile;
	CString strFile = Settings.General.DataPath + L"WorldGPS.dat";

	// Registry use is obsolete (auto-fallback)
	//bool bImport = theApp.GetProfileInt( L"", L"ImportWorldGPS", FALSE ) != 0;
	//strFile.Append( bImport ? L".xml" : L".dat" );

	if ( pFile.Open( (LPCTSTR)strFile.GetBuffer(), CFile::modeRead ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load );
			Serialize( ar );
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}

		return TRUE;
	}

	strFile = Settings.General.DataPath + L"WorldGPS.xml";
	if ( ! pFile.Open( (LPCTSTR)strFile.GetBuffer(), CFile::modeRead ) )
		return FALSE;

	// Need to generate .dat file from .xml

	CBuffer pBuffer;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();

	CXMLElement* pXML = CXMLElement::FromBytes( pBuffer.m_pBuffer, pBuffer.m_nLength );
	if ( NULL == pXML ) return FALSE;

	BOOL bSuccess = LoadFrom( pXML );
	delete pXML;

	if ( ! bSuccess ) return FALSE;

	strFile = Settings.General.DataPath + L"WorldGPS.dat";
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
		return FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::store );	// 4 KB buffer
		try
		{
			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	return TRUE;
}

void CWorldGPS::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar.WriteCount( m_nCountry );
	}
	else // Loading
	{
		m_nCountry = static_cast< DWORD >( ar.ReadCount() );
		m_pCountry = new CWorldCountry[ m_nCountry ];
	}

	for ( DWORD nCountry = 0 ; nCountry < m_nCountry ; nCountry++ )
	{
		m_pCountry[ nCountry ].Serialize( ar );
	}
}

BOOL CWorldGPS::LoadFrom(CXMLElement* pRoot)
{
	if ( ! pRoot->IsNamed( L"world" ) ) return FALSE;

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( L"country" ) ) m_nCountry++;
	}

	m_pCountry = new CWorldCountry[ m_nCountry ];
	m_nCountry = 0;

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( L"country" ) )
			m_pCountry[ m_nCountry++ ].LoadFrom( pElement );
	}

	return m_nCountry > 0;
}

//////////////////////////////////////////////////////////////////////
// CWorldGPS clear

void CWorldGPS::Clear()
{
	if ( m_pCountry ) delete [] m_pCountry;
	m_pCountry = NULL;
	m_nCountry = 0;
}


//////////////////////////////////////////////////////////////////////
// CWorldCountry construction

CWorldCountry::CWorldCountry()
{
	m_pCity = NULL;
	m_nCity = 0;
}

CWorldCountry::~CWorldCountry()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CWorldCountry serialize

void CWorldCountry::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar << m_szID[0];
		ar << m_szID[1];
		ar << m_sName;

		ar.WriteCount( m_nCity );
	}
	else // Loading
	{
		ar >> m_szID[0];
		ar >> m_szID[1];
		ar >> m_sName;

		m_nCity = static_cast< DWORD >( ar.ReadCount() );
		m_pCity = new CWorldCity[ m_nCity ];
	}

	for ( DWORD nCity = 0 ; nCity < m_nCity ; nCity++ )
	{
		m_pCity[ nCity ].Serialize( ar );
	}
}

BOOL CWorldCountry::LoadFrom(CXMLElement* pRoot)
{
	if ( ! pRoot->IsNamed( L"country" ) ) return FALSE;

	m_sName = pRoot->GetAttributeValue( L"name" );
	CString strID = pRoot->GetAttributeValue( L"code" );

	if ( strID.GetLength() != 2 ) return FALSE;
	m_szID[0] = (CHAR)strID.GetAt( 0 );
	m_szID[1] = (CHAR)strID.GetAt( 1 );

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( L"city" ) ) m_nCity++;
	}

	m_pCity = new CWorldCity[ m_nCity ];
	m_nCity = 0;

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( L"city" ) )
			m_pCity[ m_nCity++ ].LoadFrom( pElement );
	}

	return m_nCity > 0;
}

//////////////////////////////////////////////////////////////////////
// CWorldCountry clear

void CWorldCountry::Clear()
{
	if ( m_pCity ) delete [] m_pCity;
	m_pCity = NULL;
	m_nCity = 0;
}


//////////////////////////////////////////////////////////////////////
// CWorldCity load

void CWorldCity::Serialize(CArchive& ar)
{
	WORD nValue;

	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_sState;

		nValue = (WORD)(signed short)( m_nLatitude / 90.0f * 32700.0f );
		ar << nValue;
		nValue = (WORD)(signed short)( m_nLongitude / 180.0f * 32700.0f );
		ar << nValue;
	}
	else // Loading
	{
		ar >> m_sName;
		ar >> m_sState;

		ar >> nValue;
		m_nLatitude = (float)(signed short)nValue / 32700.0f * 90.0f;

		ar >> nValue;
		m_nLongitude = (float)(signed short)nValue / 32700.0f * 180.0f;
	}
}

BOOL CWorldCity::LoadFrom(CXMLElement* pRoot)
{
	if ( ! pRoot->IsNamed( L"city" ) ) return FALSE;

	m_sName  = pRoot->GetAttributeValue( L"name" );
	m_sState = pRoot->GetAttributeValue( L"state" );

	CString strValue = pRoot->GetAttributeValue( L"latitude" );
	if ( _stscanf( strValue, L"%f", &m_nLatitude ) != 1 ) return FALSE;

	strValue = pRoot->GetAttributeValue( L"longitude" );
	if ( _stscanf( strValue, L"%f", &m_nLongitude ) != 1 ) return FALSE;

	return TRUE;
}
