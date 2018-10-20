//
// Class.cpp : Implementation of CClass
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008-2015
//
// Envy is free software; you can redistribute it and/or
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

#include "StdAfx.h"
#include "Class.h"

class ATL_NO_VTABLE CZipHandler
{
public:
	inline CZipHandler( LPCTSTR szFilename ) throw()
	{
		hArchive = unzOpen( CW2A( szFilename ) );
		if ( ! hArchive )
		{
			TCHAR szFileShort[ MAX_PATH ];
			if ( GetShortPathName( szFilename, szFileShort, MAX_PATH ) )
				hArchive = unzOpen( CW2A( szFileShort ) );
		}
	}

	inline ~CZipHandler() throw()
	{
		if ( hArchive )
		{
			unzClose( hArchive );
			hArchive = NULL;
		}
	}

	inline operator unzFile() const throw()
	{
		return hArchive;
	}

protected:
	unzFile hArchive;
};

STDMETHODIMP CSkinScan::Process(
	/*[in]*/ BSTR sFile,
	/*[in]*/ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	CZipHandler pFile( sFile );
	if ( ! pFile )
		return E_FAIL;

	unz_global_info pDir = {};
	if ( unzGetGlobalInfo( pFile, &pDir ) != UNZ_OK )
		return E_UNEXPECTED;			// Bad format. Call CLibraryBuilder::SubmitCorrupted()

	for ( UINT nFile = 0; nFile < pDir.number_entry; nFile++ )
	{
		unz_file_info pInfo = {};
		CHAR szFile[ MAX_PATH ];

		if ( nFile && unzGoToNextFile( pFile ) != UNZ_OK )
			return E_UNEXPECTED;		// Bad format. Call CLibraryBuilder::SubmitCorrupted()

		if ( unzGetCurrentFileInfo( pFile, &pInfo, szFile, MAX_PATH, NULL, 0, NULL, 0 ) != UNZ_OK )
			return E_UNEXPECTED;		// Bad format. Call CLibraryBuilder::SubmitCorrupted()

		if ( lstrcmpiA( szFile + lstrlenA( szFile ) - 4, ".xml" ) == 0 )
		{
			if ( unzOpenCurrentFile( pFile ) != UNZ_OK )
				return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()

			LPSTR pszXML = new CHAR[ pInfo.uncompressed_size + 1 ];
			if ( ! pszXML )
			{
				unzCloseCurrentFile( pFile );
				return E_OUTOFMEMORY;
			}
			ZeroMemory( pszXML, pInfo.uncompressed_size + 1 );

			if ( unzReadCurrentFile( pFile, pszXML, pInfo.uncompressed_size ) < 0 )
			{
				delete [] pszXML;
				unzCloseCurrentFile( pFile );
				return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()
			}

			pszXML[ pInfo.uncompressed_size ] = 0;

			if ( ScanFile( pszXML, pXML ) )
			{
				delete [] pszXML;
				unzCloseCurrentFile( pFile );
				return S_OK;
			}

			delete [] pszXML;
			unzCloseCurrentFile( pFile );
		}
	}

	return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
//
// ScanFile() is a helper function which accepts an XML string, decodes it,
// checks if it is a Envy skin file, and copies <manifest> metadata to the output.
//
// pszXML  : The XML string
// pOutput : An empty XML element to build metadata in
//

BOOL CSkinScan::ScanFile(LPCSTR pszXML, ISXMLElement* pOutput)
{
	// Put the XML string in a BSTR
	BOOL bBOMPresent = FALSE;
	if ( lstrlenA( pszXML ) > 3  && (UCHAR)pszXML[0] == 0xEF &&
		(UCHAR)pszXML[1] == 0xBB && (UCHAR)pszXML[2] == 0xBF )
	{
		bBOMPresent = TRUE;
		pszXML += 3;
	}
	int nLength = MultiByteToWideChar(CP_UTF8, 0, pszXML, lstrlenA(pszXML), NULL, 0);
	WCHAR* pszUnicode = new WCHAR[ nLength + 1 ];
	MultiByteToWideChar(CP_UTF8, 0, pszXML, lstrlenA(pszXML), pszUnicode, nLength);
	pszUnicode[ nLength ] = 0;
	CComBSTR sUnicode( pszUnicode );
	delete [] pszUnicode;
	if ( bBOMPresent ) pszXML -= 3;

	// Use the FromString() method in ISXMLElement to decode an XML document from the XML string, output in pFile.
	CComPtr< ISXMLElement > pFile;
	if ( FAILED( pOutput->FromString( sUnicode, &pFile ) ) || pFile == NULL )
	{
		return FALSE;
	}

	//BOOL bSkin = TRUE;

	// Test if the root element of the document is called "skin" (or "package")
	VARIANT_BOOL bNamed = VARIANT_FALSE;
	pFile->IsNamed( CComBSTR( L"skin" ), &bNamed );
	if ( ! bNamed )
	{
		pFile->IsNamed( CComBSTR( L"package" ), &bNamed );
		if ( ! bNamed )
		{
			pFile->Delete();
			return FALSE;
		}
	//	bSkin = FALSE;
	}

	// Get the Elements collection from the XML document
	CComPtr< ISXMLElements > pElements;
	if ( FAILED( pFile->get_Elements( &pElements ) ) || pElements == NULL )
	{
		pFile->Delete();
		return FALSE;
	}

	// Find the <manifest> element
	CComPtr< ISXMLElement > pManifest;
	if ( FAILED( pElements->get_ByName( CComBSTR( L"manifest" ), &pManifest ) ) || pManifest == NULL )
	{
		pFile->Delete();
		return FALSE;
	}

	// Add the plural <EnvySkins> element
	CComPtr< ISXMLElement > pPlural;
	{
		CComPtr< ISXMLElements > pOutputElements;
		if ( FAILED( pOutput->get_Elements( &pOutputElements ) ) || pOutputElements == NULL )
		{
			pFile->Delete();
			return FALSE;
		}
		pOutputElements->Create( CComBSTR( L"EnvySkins" ), &pPlural );	// "EnvyPackages" requires schema xml
	}

	// Add xsi:noNamespaceSchemaLocation="http://schemas.getenvy.com/Skin.xsd"
	// to the Attributes collection of <EnvySkins>
	{
		CComPtr< ISXMLAttributes > pAttrs;
		pPlural->get_Attributes( &pAttrs );
		pAttrs->Add( CComBSTR( L"xsi:noNamespaceSchemaLocation" ),
			CComBSTR( L"http://schemas.getenvy.com/Skin.xsd" ) );	// "Package.xsd" requires schema xml
	}

	// Change <manifest> to <EnvySkin>
	pManifest->put_Name( CComBSTR( L"EnvySkin" ) );				// "EnvyPackage" requires schema xml

	// Detach <manifest> from the file document and add it to the output XML document
	pManifest->Detach();
	{
		CComPtr< ISXMLElements > pPluralElements;
		pPlural->get_Elements( &pPluralElements );
		pPluralElements->Attach( pManifest );
	}

	pFile->Delete();

	return TRUE;
}
