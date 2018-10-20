//
// Class.cpp : Implementation of CClass (7z)
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
#include <InitGuid.h>
#include "7zip/IArchive.h"

#define MAX_SIZE_FILES		80
#define MAX_SIZE_FOLDERS	80
#define MAX_SIZE_COMMENTS	256

// {23170F69-40C1-278A-1000-000110070000}
DEFINE_GUID( CLSID_CFormat7z,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00 );

class CComPropVariant : public PROPVARIANT
{
public:
	inline CComPropVariant() throw()
	{
		::PropVariantInit( this );
	}

	inline ~CComPropVariant() throw()
	{
		Clear();
	}

	inline HRESULT Clear() throw()
	{
		return ::PropVariantClear( this );
	}
};

class CInStream :
	public IInStream,
	public IStreamGetSize
{
public:
	CInStream() throw() :
		pFile( INVALID_HANDLE_VALUE )
	{
	}

	~CInStream() throw()
	{
		Close();
	}

	inline bool Open(LPCTSTR szFilename) throw()
	{
		if ( pFile == INVALID_HANDLE_VALUE )
			pFile = CreateFile( szFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		return ( pFile != INVALID_HANDLE_VALUE );
	}

	inline void Close() throw()
	{
		if ( pFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( pFile );
			pFile = NULL;
		}
	}

// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv) throw()
	{
		if ( riid == IID_IUnknown )
			*ppv = static_cast< IUnknown* >( static_cast< IInStream* >( this ) );
		else if (riid == IID_IInStream )
			*ppv = static_cast< IInStream* >( this );
		else if (riid == IID_ISequentialInStream )
			*ppv = static_cast< ISequentialInStream* >( this );
		else if (riid == IID_IStreamGetSize )
			*ppv = static_cast< IStreamGetSize* >( this );
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		return S_OK;
	}

	STDMETHODIMP_(ULONG) AddRef(void) throw()
	{
		return 2;
	}

	STDMETHODIMP_(ULONG) Release(void) throw()
	{
		return 1;
	}

// IInStream
	STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) throw()
	{
		if ( pFile == INVALID_HANDLE_VALUE )
			return E_UNEXPECTED;

		LARGE_INTEGER value;
		value.QuadPart = offset;
		value.LowPart = SetFilePointer( pFile, value.LowPart,
			&value.HighPart, seekOrigin );
		if ( value.LowPart == 0xFFFFFFFF )
			if( GetLastError() != NO_ERROR )
				return E_FAIL;
		if ( newPosition )
			*newPosition = value.QuadPart;
		return S_OK;
	}

// ISequentialInStream
	STDMETHODIMP Read(void *data, UInt32 size, UInt32 *processedSize) throw()
	{
		if ( pFile == INVALID_HANDLE_VALUE )
			return E_UNEXPECTED;

		return ReadFile( pFile, data, size, (LPDWORD)processedSize, NULL ) ? S_OK : E_FAIL;
	}

// IStreamGetSize
	STDMETHODIMP GetSize(UInt64 *size) throw()
	{
		if ( pFile == INVALID_HANDLE_VALUE )
			return E_UNEXPECTED;

		DWORD sizeHigh = 0;
		DWORD sizeLow = ::GetFileSize( pFile, &sizeHigh);
		if ( sizeLow == 0xFFFFFFFF )
			if ( GetLastError() != NO_ERROR )
				return E_FAIL;
		if ( size )
			*size = ( ( (UInt64)sizeHigh ) << 32 ) + sizeLow;
		return S_OK;
	}

protected:
	HANDLE pFile;
};

STDMETHODIMP C7ZipBuilder::Process(
	/*[in]*/ BSTR sFile,
	/*[in]*/ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	if ( ! fnCreateObject )
		return E_NOTIMPL;	// 7zxa.dll not loaded

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements(&pISXMLRootElements);
	if ( FAILED( hr ) ) return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create(CComBSTR("archives"), &pXMLRootElement);
	if ( FAILED( hr ) ) return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes(&pISXMLRootAttributes);
	if ( FAILED( hr ) ) return hr;
	pISXMLRootAttributes->Add(CComBSTR("xmlns:xsi"),
		CComBSTR("http://www.w3.org/2001/XMLSchema-instance"));
	pISXMLRootAttributes->Add(CComBSTR("xsi:noNamespaceSchemaLocation"),
		CComBSTR("http://schemas.getenvy.com/Archive.xsd"));

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements(&pISXMLElements);
	if ( FAILED( hr ) ) return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create(CComBSTR("archive"), &pXMLElement);
	if ( FAILED( hr ) ) return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes(&pISXMLAttributes);
	if ( FAILED( hr ) ) return hr;

	CString strFiles;				// Plain list of archive files
	bool bMoreFiles = false;		// More files than listed in sFiles
	CString strFolders;				// Plain list of archive folders
	bool bMoreFolders = false;		// More folders than listed in sFolders
	CString strComment;				// Archive comments
	bool bEncrypted = false;		// Archive itself or selective files are encrypted
	ULONGLONG nUnpackedSize = 0;	// Total size of unpacked files
	int nFileCount = 0;				// Total number of contained files

	USES_CONVERSION;
	CInStream oInStream;
	if ( ! oInStream.Open( OLE2CT( sFile ) ) )
		return E_FAIL;				// Cannot open file

	CComPtr< IInArchive > pIInArchive;
	hr = fnCreateObject( &CLSID_CFormat7z, &IID_IInArchive, (void**)&pIInArchive );
	if ( FAILED( hr ) )
		return E_NOTIMPL;			// Bad 7zxa.dll version?

	hr = pIInArchive->Open( static_cast< IInStream* >( &oInStream ), NULL, NULL );
	if ( hr != S_OK )				// S_FALSE - unknown format
		return E_UNEXPECTED;		// Bad format. Call CLibraryBuilder::SubmitCorrupted()

	UInt32 numProperties = 0;
	hr = pIInArchive->GetNumberOfArchiveProperties( &numProperties );
	if ( FAILED( hr ) )
		return E_UNEXPECTED;		// Bad format. Call CLibraryBuilder::SubmitCorrupted()

	for ( UInt32 p = 0; p < numProperties; p++ )
	{
		BSTR name = NULL;
		PROPID propID = 0;
		VARTYPE varType = 0;
		hr = pIInArchive->GetArchivePropertyInfo( p, &name, &propID, &varType);
		if ( FAILED( hr ) )
			return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()
		SysFreeString( name );
	}

	// List command
	UInt32 numItems = 0;
	hr = pIInArchive->GetNumberOfItems( &numItems );
	if ( FAILED( hr ) )
		return E_UNEXPECTED;		// Bad format. Call CLibraryBuilder::SubmitCorrupted()

	for ( UInt32 i = 0; i < numItems; i++ )
	{
		// Get name of file (VT_BSTR)
		CComPropVariant propPath;
		hr = pIInArchive->GetProperty( i, kpidPath, &propPath );
		if ( FAILED( hr ) || propPath.vt != VT_BSTR )
			return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()

		CString strName( propPath.bstrVal );
		int nBackSlashPos = strName.ReverseFind( L'\\' );
		if ( nBackSlashPos >= 0 )
			strName = strName.Mid( nBackSlashPos + 1 );

		// Get folder/file flag (VT_BOOL)
		CComPropVariant propIsFolder;
		hr = pIInArchive->GetProperty( i, kpidIsDir, &propIsFolder );
		if ( FAILED( hr ) || propIsFolder.vt != VT_BOOL )
			return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()

		if ( propIsFolder.boolVal == VARIANT_TRUE )	// Folder
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
				if ( strFiles.GetLength() )
					strFiles += L", ";
				strFiles += strName;
			}
			else
				bMoreFiles = true;

			// Get uncompressed size of file (VT_UI8)
			CComPropVariant propSize;
			hr = pIInArchive->GetProperty( i, kpidSize, &propSize );
			if ( FAILED( hr ) || propSize.vt != VT_UI8 )
				return E_UNEXPECTED;	// Bad format. Call CLibraryBuilder::SubmitCorrupted()

			nUnpackedSize += propSize.uhVal.QuadPart;
		}
	}

	pIInArchive->Close();

	if ( ! strFiles.IsEmpty() )
	{
		if ( bMoreFiles )
			strFiles += L", ...";
		pISXMLAttributes->Add( CComBSTR( "files" ), CComBSTR( strFiles ) );
	}

	if ( ! strFolders.IsEmpty() )
	{
		if ( bMoreFolders )
			strFolders += L", ...";
		pISXMLAttributes->Add( CComBSTR( "folders" ), CComBSTR( strFolders ) );
	}

	if ( ! strComment.IsEmpty() )
		pISXMLAttributes->Add( CComBSTR( "comments" ), CComBSTR( strComment ) );

	if ( bEncrypted )
		pISXMLAttributes->Add( CComBSTR( "encrypted" ), CComBSTR( "true" ) );

	if ( nUnpackedSize )
	{
		CString strUnpackedSize;
		strUnpackedSize.Format( L"%I64u", nUnpackedSize );
		pISXMLAttributes->Add( CComBSTR( "unpackedsize" ), CComBSTR( strUnpackedSize ) );
	}

	if ( nFileCount > 0 )
	{
		CString strFileCount;
		strFileCount.Format( L"%i", nFileCount );
		pISXMLAttributes->Add( CComBSTR( "filecount" ), CComBSTR( strFileCount ) );
	}
	else
	{
		CString strFileCount;
		strFileCount.Format( L"%I32u", numItems );
		pISXMLAttributes->Add( CComBSTR( "filecount" ), CComBSTR( strFileCount ) );
	}

	return S_OK;
}
