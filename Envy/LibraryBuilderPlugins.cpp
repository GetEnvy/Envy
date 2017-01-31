//
// LibraryBuilderPlugins.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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
#include "LibraryBuilderPlugins.h"

#include "LibraryBuilder.h"
#include "Plugins.h"
#include "XMLCOM.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins construction

HRESULT CLibraryBuilderPlugins::SafeProcess(ILibraryBuilderPlugin* pPlugin, BSTR szPath, ISXMLElement* pElement)
{
	__try
	{
		return pPlugin->Process( szPath, pElement );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return RPC_E_SERVERFAULT;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins extract

bool CLibraryBuilderPlugins::ExtractPluginMetadata(DWORD nIndex, const CString& strPath)
{
	CString strType = PathFindExtension( strPath );
	strType.MakeLower();

	for ( int i = 0 ; i < 2 ; ++i )
	{
		CComPtr< ILibraryBuilderPlugin > pPlugin( LoadPlugin( strType ) );
		if ( ! pPlugin )
			break;

		unique_ptr< CXMLElement > pXML( new CXMLElement() );
		CComPtr< ISXMLElement > pISXMLElement;
		pISXMLElement.Attach( (ISXMLElement*)CXMLCOM::Wrap( pXML.get(), IID_ISXMLElement ) );
		HRESULT hr = SafeProcess( pPlugin, CComBSTR( strPath ), pISXMLElement );
		if ( SUCCEEDED( hr ) )
		{
			if ( CXMLElement* pOuter = pXML->GetFirstElement() )
			{
				CXMLElement* pInner  = pOuter->GetFirstElement();
				CString strSchemaURI = pOuter->GetAttributeValue( CXMLAttribute::schemaName );

				if ( pInner && ! strSchemaURI.IsEmpty() )
				{
					pInner = pInner->Detach();
					return LibraryBuilder.SubmitMetadata( nIndex, strSchemaURI, pInner ) != 0;
				}
			}
		}
		else if ( hr == E_UNEXPECTED )
		{
			return LibraryBuilder.SubmitCorrupted( nIndex );
		}
		else if ( SERVERLOST( hr ) )
		{
			Plugins.ReloadPlugin( L"LibraryBuilder", strType );

			pPlugin.Release();

			continue;	// Try again
		}

		//break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins load plugin

ILibraryBuilderPlugin* CLibraryBuilderPlugins::LoadPlugin(LPCTSTR pszType)
{
	CComQIPtr< ILibraryBuilderPlugin > pPlugin(
		Plugins.GetPlugin( L"LibraryBuilder", pszType ) );

	return pPlugin.Detach();
}
