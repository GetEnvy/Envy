//
// SchemaChild.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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
#include "Schema.h"
#include "SchemaChild.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CSchemaChild construction

CSchemaChild::CSchemaChild(CSchemaPtr pSchema)
	: m_pSchema	( pSchema )
	, m_nType	( CSchema::typeFile )
{
}

CSchemaChild::~CSchemaChild()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSchemaChild load

BOOL CSchemaChild::Load(const CXMLElement* pXML)
{
	m_sURI = pXML->GetAttributeValue( L"location" );
	if ( m_sURI.IsEmpty() ) return FALSE;

	CString strType = pXML->GetAttributeValue( L"type" );

	if ( strType == L"folder" )
		m_nType = CSchema::typeFolder;
	else if ( strType == L"file" )
		m_nType = CSchema::typeFile;
	else
		return FALSE;

	for ( POSITION pos = pXML->GetElementIterator(); pos; )
	{
		const CXMLElement* pElement = pXML->GetNextElement( pos );

		if ( pElement->IsNamed( L"identity" ) ||
			 pElement->IsNamed( L"shared" ) )
		{
			CSchemaChildMap* pMap = new CSchemaChildMap();

			if ( pMap->Load( pElement ) )
			{
				m_pMap.AddTail( pMap );
			}
			else
			{
				delete pMap;
				return FALSE;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSchemaChild clear

void CSchemaChild::Clear()
{
	for ( POSITION pos = m_pMap.GetHeadPosition(); pos; )
	{
		delete m_pMap.GetNext( pos );
	}
	m_pMap.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CSchemaChild member copy

BOOL CSchemaChild::MemberCopy(CXMLElement* pLocal, CXMLElement* pRemote, BOOL bToRemote, BOOL bAggressive) const
{
	if ( ! pLocal || ! pRemote ) return FALSE;

	BOOL bChanged = FALSE;

	for ( POSITION pos = m_pMap.GetHeadPosition(); pos; )
	{
		CSchemaChildMapPtr pMap = m_pMap.GetNext( pos );
		const CXMLAttribute* pAttribute1 = NULL;
		const CXMLAttribute* pAttribute2 = NULL;

		if ( bToRemote )
		{
			pAttribute1 = pLocal->GetAttribute( pMap->m_sLocal );
			pAttribute2 = pRemote->GetAttribute( pMap->m_sRemote );
		}
		else
		{
			pAttribute1 = pRemote->GetAttribute( pMap->m_sRemote );
			pAttribute2 = pLocal->GetAttribute( pMap->m_sLocal );
		}

		if ( pAttribute1 && ( ! pAttribute2 || bAggressive ) )
		{
			CString strValue( pAttribute1->GetValue() );

			if ( pMap->m_bIdentity )
				CXMLNode::UniformString( strValue );

			if ( bToRemote )
				pRemote->AddAttribute( pMap->m_sRemote, strValue );
			else
				pLocal->AddAttribute( pMap->m_sLocal, strValue );

			bChanged = TRUE;
		}
	}

	return bChanged;
}


//////////////////////////////////////////////////////////////////////
// CSchemaChildMap construction

CSchemaChildMap::CSchemaChildMap()
	: m_bIdentity ( FALSE )
{
}

//CSchemaChildMap::~CSchemaChildMap()
//{
//}

//////////////////////////////////////////////////////////////////////
// CSchemaChildMap operation

BOOL CSchemaChildMap::Load(const CXMLElement* pXML)
{
	if ( pXML->IsNamed( L"identity" ) )
		m_bIdentity = TRUE;
	else if ( pXML->IsNamed( L"shared" ) )
		m_bIdentity = FALSE;
	else
		return FALSE;

	m_sLocal  = pXML->GetAttributeValue( L"local" );
	m_sRemote = pXML->GetAttributeValue( L"remote" );

	if ( m_sLocal.IsEmpty() || m_sRemote.IsEmpty() )
		return FALSE;

	return TRUE;
}
