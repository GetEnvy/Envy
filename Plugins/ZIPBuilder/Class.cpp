//
// Class.cpp : Implementation of CClass (Zip)
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008-2014
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

#define MAX_SIZE_FILES		128
#define MAX_SIZE_FOLDERS	128
#define MAX_SIZE_COMMENTS	256

class ATL_NO_VTABLE CZipHandler
{
public:
	inline CZipHandler( LPCTSTR szFilename ) throw()
	{
		hArchive = unzOpen( CT2CA( szFilename ) );
		if ( ! hArchive )
		{
			TCHAR szFileShort[ MAX_PATH ];
			if ( GetShortPathName( szFilename, szFileShort, MAX_PATH ) )
				hArchive = unzOpen( CT2CA( szFileShort ) );
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

STDMETHODIMP CZIPBuilder::Process(
	/*[in]*/ BSTR sFile,
	/*[in]*/ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements( &pISXMLRootElements );
	if ( FAILED( hr ) ) return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create( CComBSTR( L"archives" ), &pXMLRootElement );
	if ( FAILED( hr ) ) return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes( &pISXMLRootAttributes );
	if ( FAILED( hr ) ) return hr;
	pISXMLRootAttributes->Add( CComBSTR( L"xmlns:xsi" ),
		CComBSTR( L"http://www.w3.org/2001/XMLSchema-instance" ) );
	pISXMLRootAttributes->Add( CComBSTR( L"xsi:noNamespaceSchemaLocation" ),
		CComBSTR( L"http://schemas.getenvy.com/Archive.xsd" ) );

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements(&pISXMLElements);
	if ( FAILED( hr ) ) return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create( CComBSTR( L"archive" ), &pXMLElement);
	if ( FAILED( hr ) ) return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes(&pISXMLAttributes);
	if ( FAILED( hr ) ) return hr;

	CString strFiles;				// Plain list of archive files
	bool bMoreFiles = false;		// More files than listed in strFiles
	CString strFolders;				// Plain list of archive folders
	bool bMoreFolders = false;		// More folders than listed in strFolders
	CString strComment;				// Archive comments
	bool bEncrypted = false;		// Archive itself or selective files are encrypted
	ULONGLONG nUnpackedSize = 0;	// Total size of unpacked files
	int nFileCount = 0;				// Total number of contained files

	USES_CONVERSION;
	CZipHandler pFile( OLE2CT( sFile ) );
	if ( ! pFile )
		return E_FAIL;

	unz_global_info pDir = {};
	if ( unzGetGlobalInfo( pFile, &pDir ) != UNZ_OK )
		return E_UNEXPECTED;		// Bad format. Call CLibraryBuilder::SubmitCorrupted()

	if ( pDir.size_comment )
	{
		char szCmtBuf[ MAX_SIZE_COMMENTS ];
		int nResult = unzGetGlobalComment( pFile, szCmtBuf, MAX_SIZE_COMMENTS );
		if ( nResult < 0 )
			return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()

		szCmtBuf[ MAX_SIZE_COMMENTS - 1 ] = L'\0';
		strComment = szCmtBuf;
		strComment.Replace( L'\r', L' ' );
		strComment.Replace( L'\n', L' ' );
		strComment.Replace( L"  ", L" " );
	}

	for ( UINT nFile = 0; nFile < pDir.number_entry; nFile++ )
	{
		if ( nFile && unzGoToNextFile( pFile ) != UNZ_OK )
			return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()

		unz_file_info pInfo = {};
		CHAR szFile[ MAX_PATH ] = {};
		if ( unzGetCurrentFileInfo( pFile, &pInfo, szFile, MAX_PATH, NULL, 0, NULL, 0 ) != UNZ_OK )
			return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()

		OemToCharA( szFile, szFile );

		if ( ( pInfo.flag & 0x01 ) )
			bEncrypted = true;

		bool bFolder = false;

		CString strName( szFile );
		int n = strName.ReverseFind( L'/' );
		if ( n == strName.GetLength() - 1 )
		{
			bFolder = true;
			strName = strName.Left( n );
			n = strName.ReverseFind( L'/' );
		}
		if ( n >= 0 )
			strName = strName.Mid( n + 1 );

		if ( ( pInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY ) )
			bFolder = true;

		if ( bFolder )
		{
			if ( strFolders.GetLength() + strName.GetLength() <= MAX_SIZE_FOLDERS - 5 )
			{
				if ( strFolders.GetLength() )
					strFolders += L", ";
				strFolders += strName;
			}
			else
				bMoreFolders = true;
		}
		else	// File
		{
			nFileCount++;

			if ( strFiles.GetLength() + strName.GetLength() <= MAX_SIZE_FILES - 5 )
			{
				if ( ! strFiles.IsEmpty() )
					strFiles += L", ";
				strFiles += strName;
			}
			else
				bMoreFiles = true;

			nUnpackedSize += pInfo.uncompressed_size;
		}
	}

	if ( ! strFiles.IsEmpty() )
	{
		if ( bMoreFiles )
			strFiles += L", ...";
		pISXMLAttributes->Add( CComBSTR( L"files" ), CComBSTR( strFiles ) );
	}

	if ( ! strFolders.IsEmpty() )
	{
		if ( bMoreFolders )
			strFolders += L", ...";
		pISXMLAttributes->Add( CComBSTR( L"folders" ), CComBSTR( strFolders ) );
	}

	if ( ! strComment.IsEmpty() )
		pISXMLAttributes->Add( CComBSTR( L"comments" ), CComBSTR( strComment ) );

	if ( bEncrypted )
		pISXMLAttributes->Add( CComBSTR( L"encrypted" ), CComBSTR( L"true" ) );

	if ( nUnpackedSize )
	{
		CString strFormat;
		strFormat.Format( L"%I64u", nUnpackedSize );
		pISXMLAttributes->Add( CComBSTR( L"unpackedsize" ), CComBSTR( strFormat ) );
	}

	if ( nFileCount > 0 )
	{
		CString strFormat;
		strFormat.Format( L"%i", nFileCount );
		pISXMLAttributes->Add( CComBSTR( L"filecount" ), CComBSTR( strFormat ) );
	}
	else
	{
		CString strFormat;
		strFormat.Format( L"%I32u", pDir.number_entry );
		pISXMLAttributes->Add( CComBSTR( L"filecount" ), CComBSTR( strFormat ) );
	}

	// Special case .CBZ - Common filename metadata
	CString strName( sFile );
	strName = strName.Mid( strName.ReverseFind( L'/' ) );
	if ( strName.GetLength() > 8 && strName.Right( 4 ) == L".cbz" )
	{
		//pISXMLAttributes->Add( CComBSTR( L"cbz" ), CComBSTR( L"cbz" ) );	// Other metadata?
		int nFind = strName.Find( L"20" );
		if ( nFind < 0 ) nFind = strName.Find( L"19" );
		if ( nFind >= 0 )
		{
			CString strYear;
			for ( int i = 2022; i > 1940; i-- )
			{
				strYear.Format( L"%i", i );
				int nFound = strName.Find( strYear, nFind );
				if ( nFound >= 0 )
				{
					// Verify number is not substring
					if ( ( ! nFound || strName[ nFound - 1 ] > L'9' || strName[ nFound - 1 ] < L'0' ) &&
						 ( strName[ nFound + 4 ] > L'9' || strName[ nFound + 4 ] < L'0' ) )
					{
						pISXMLAttributes->Add( CComBSTR( L"year" ), CComBSTR( strYear ) );
						break;
					}
				}
			}
		}
	}

	return S_OK;
}
