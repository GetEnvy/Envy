//
// Text.cpp
//
// This file is part of Envy (getenvy.com) © 2010
// TextViewer plugin is released under the Persistent Public Domain license.
//
// This code may be treated as Public Domain, provided:
// the work in all its forms and attendant uses shall remain available as
// persistently "Public Domain" until it naturally enters the public domain.
// History remains immutable:  Authors do not disclaim copyright, but do disclaim
// all rights beyond asserting the reach and duration and spirit of this license.

// This file contains a utility class CText, for managing texts.

// Unimplemented placeholder only!

#include "StdAfx.h"
#include "TextViewer.h"
#include "Text.h"


//////////////////////////////////////////////////////////////////////
// CText construction

CText::CText()
{
	m_pText		= NULL;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_nComponents	= 0;
	m_bPartial		= FALSE;
}

CText::~CText()
{
	if ( m_pText ) delete [] m_pText;
}

//////////////////////////////////////////////////////////////////////
// CText clear

void CText::Clear()
{
	if ( m_pText ) delete [] m_pText;

	m_pText		= NULL;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_nComponents	= 0;
	m_bPartial		= FALSE;
}

//////////////////////////////////////////////////////////////////////
// CText ensure RGB

BOOL CText::EnsureRGB(COLORREF crFill)
{
	// REMOVE
}

//////////////////////////////////////////////////////////////////////
// CText resample to a specific size

HBITMAP CText::Resample(int nNewWidth, int nNewHeight)
{
	// REMOVE
}

//////////////////////////////////////////////////////////////////////
// CText load a text file

BOOL CText::Load(LPCTSTR pszPath)
{
	// ToDo: REMOVE Placeholder image code, obviously

	// Clear any previous file
	Clear();

	// Open the file
	HANDLE hFile = CreateFile( pszPath, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	// Make sure it worked
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;

	// Get the file length
	DWORD nLength = GetFileSize( hFile, NULL );

	// Try to load the Envy ImageService for this file type
	IImageServicePlugin* pService = LoadService( pszPath );

	// Make sure it worked
	if ( pService == NULL )
	{
		CloseHandle( hFile );
		return FALSE;
	}

	// Setup the IMAGESERVICEDATA structure
	IMAGESERVICEDATA pParams;
	ZeroMemory( &pParams, sizeof(pParams) );
	pParams.cbSize		= sizeof(pParams);
	pParams.nFlags		= IMAGESERVICE_PARTIAL_IN;	// Partial texts okay

	// Ask the ImageService to load from file handle
	SAFEARRAY* pArray = NULL;
	BSTR sFile = SysAllocString (CT2CW (pszPath));
	HRESULT hr = pService->LoadFromFile( sFile, &pParams, &pArray );
	SysFreeString (sFile);

	// Check the result
	if ( hr == E_NOTIMPL )
	{
		// ImageService does not support loading from file.  This is allowed, but inconvenient for us.
		// It must support loading from memory, so we'll do that instead.

		// If an output was created, get rid of it (technically it shouldn't have happened, but anyway)
		if ( pArray != NULL ) SafeArrayDestroy( pArray );
		pArray = NULL;

		// Create a file mapping for the file
		HANDLE hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );

		if ( hMap != INVALID_HANDLE_VALUE )
		{
			DWORD nPosition = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );

			// Map a view of the whole file
			if ( LPCVOID pBuffer = MapViewOfFile( hMap, FILE_MAP_READ, 0, nPosition, nLength ) )
			{
				SAFEARRAY* pInput;

				// Create a safearray of the appropriate size
				if ( SUCCEEDED( SafeArrayAllocDescriptor( 1, &pInput ) ) && pInput != NULL )
				{
					pInput->cbElements = 1;
					pInput->rgsabound[ 0 ].lLbound = 0;
					pInput->rgsabound[ 0 ].cElements = nLength;
					SafeArrayAllocData( pInput );

					// Load the data directly into the safearray
					LPBYTE pTarget;
					if ( SUCCEEDED( SafeArrayAccessData( pInput, (void HUGEP* FAR*)&pTarget ) ) )
					{
						CopyMemory( pTarget, pBuffer, nLength );
						SafeArrayUnaccessData( pInput );
					}

					// Ask the ImageService to load from memory
					LPCTSTR pszType = _tcsrchr( pszPath, '.' );
					if ( pszType == NULL ) return FALSE;

					BSTR bstrType = SysAllocString ( CT2CW( pszType ) );
					hr = pService->LoadFromMemory( bstrType, pInput, &pParams, &pArray );
					SysFreeString( bstrType );
					SafeArrayDestroy( pInput );
				}

				UnmapViewOfFile( pBuffer );
			}

			CloseHandle( hMap );
		}
	}

	// Release the ImageService and close the file
	pService->Release();
	CloseHandle( hFile );

	// Make sure the load (whichever one was used), worked
	if ( FAILED(hr) )
	{
		// Get rid of the output if there was one
		if ( pArray != NULL ) SafeArrayDestroy( pArray );
		return FALSE;
	}

	// Retreive attributes from the IMAGESERVICELOAD structure
	m_nWidth		= pParams.nWidth;
	m_nHeight		= pParams.nHeight;
	m_nComponents	= pParams.nComponents;
	m_bPartial		= 0 != ( pParams.nFlags & IMAGESERVICE_PARTIAL_OUT );

	// Get the size of the output data
	LONG nArray = 0;
	SafeArrayGetUBound( pArray, 1, &nArray );
	nArray++;

	// Calculate the expected size (rows must be 32-bit aligned)
	LONG nFullSize = pParams.nWidth * pParams.nComponents;
	while ( nFullSize & 3 ) nFullSize++;
	nFullSize *= pParams.nHeight;

	// Make sure the size is what we expected
	if ( nArray != nFullSize )
	{
		SafeArrayDestroy( pArray );
		return FALSE;
	}

	// Allocate memory for the text
	m_pText = new BYTE[ nArray ];

	// Copy to our memory and destroy the safearray
	LPBYTE pData;
	SafeArrayAccessData( pArray, (VOID**)&pData );
	CopyMemory( m_pText, pData, nArray );
	SafeArrayUnaccessData( pArray );
	SafeArrayDestroy( pArray );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CText load an ImageService (REMOVE)

IImageServicePlugin* CText::LoadService(LPCTSTR pszFile)
{
	LPCTSTR pszType = _tcsrchr( pszFile, '.' );
	if ( ! pszType ) return NULL;

	ULONG dwCLSID = 128;
	TCHAR szCLSID[128];

	CRegKey pKey;

	if ( pKey.Open( HKEY_CURRENT_USER,
		_T("SOFTWARE\\Envy\\Envy\\Plugins\\ImageService") ) != ERROR_SUCCESS )
		return NULL;

	bool bPartial = lstrcmpi( pszType, _T(".partial") ) == 0;
	if ( pKey.QueryStringValue( ( bPartial ? _T(".jpg") : pszType ), szCLSID, &dwCLSID ) != ERROR_SUCCESS )
		return NULL;

	pKey.Close();

	CLSID pCLSID = {};
	if ( FAILED( CLSIDFromString( szCLSID, &pCLSID ) ) )
		return NULL;

	IImageServicePlugin* pService = NULL;
	HRESULT hResult = CoCreateInstance( pCLSID, NULL, CLSCTX_ALL,
		IID_IImageServicePlugin, (void**)&pService );

	return SUCCEEDED(hResult) ? pService : NULL;
}
