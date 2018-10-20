//
// ImageServices.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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
#include "Envy.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "Plugins.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CImageServices ImageServices;

IMPLEMENT_DYNCREATE(CImageServices, CComObject)

BEGIN_INTERFACE_MAP(CImageServices, CComObject)
	INTERFACE_PART(CImageServices, IID_IImageServicePlugin, ImageService)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageServices load operations

BOOL CImageServices::LoadFromMemory(CImageFile* pFile, LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly, BOOL bPartialOk)
{
	CComQIPtr< IImageServicePlugin > pService(
		Plugins.GetPlugin( L"ImageService", pszType ) );
	if ( ! pService )
		return FALSE;

	HINSTANCE hRes = AfxGetResourceHandle();
	BOOL bSuccess = FALSE;

	SAFEARRAY* pInput;
	if ( SUCCEEDED( SafeArrayAllocDescriptor( 1, &pInput ) ) && pInput )
	{
		pInput->cbElements = 1;
		pInput->rgsabound[ 0 ].lLbound = 0;
		pInput->rgsabound[ 0 ].cElements = nLength;
		if ( SUCCEEDED( SafeArrayAllocData( pInput ) ) )
		{
			LPBYTE pTarget;
			if ( SUCCEEDED( SafeArrayAccessData( pInput, (void HUGEP* FAR*)&pTarget ) ) )
			{
				CopyMemory( pTarget, pData, nLength );
				VERIFY( SUCCEEDED( SafeArrayUnaccessData( pInput ) ) );

				CComBSTR bstrType( pszType );

				SAFEARRAY* pArray = NULL;
				IMAGESERVICEDATA pParams = {};
				pParams.cbSize = sizeof( pParams );
				if ( bScanOnly ) pParams.nFlags |= IMAGESERVICE_SCANONLY;
				if ( bPartialOk ) pParams.nFlags |= IMAGESERVICE_PARTIAL_IN;
				HRESULT hr = pService->LoadFromMemory( bstrType, pInput, &pParams, &pArray );
				if ( SUCCEEDED( hr ) )
					bSuccess = PostLoad( pFile, &pParams, pArray );
				else if ( SERVERLOST( hr ) )
					Plugins.ReloadPlugin( L"ImageService", pszType );	// Plugin died, reload.

				if ( pArray )
					VERIFY( SUCCEEDED( SafeArrayDestroy( pArray ) ) );
			}
			VERIFY( SUCCEEDED( SafeArrayDestroyData( pInput ) ) );
		}
		VERIFY( SUCCEEDED( SafeArrayDestroyDescriptor( pInput ) ) );
	}

	AfxSetResourceHandle( hRes );

	return bSuccess;
}

BOOL CImageServices::LoadFromFile(CImageFile* pFile, LPCTSTR szFilename, BOOL bScanOnly, BOOL bPartialOk)
{
	// Get file extension
	CString strType( PathFindExtension( szFilename ) ); // ".ext"
	strType.MakeLower();

	CComQIPtr< IImageServicePlugin > pService( Plugins.GetPlugin( L"ImageService", strType ) );
	if ( pService )
	{
		if ( LoadFromFileHelper( pService, pFile, szFilename, bScanOnly, bPartialOk ) )
			return TRUE;
	}

	service_list oList;
	if ( LookupUniversalPlugins( oList ) )
	{
		for ( service_list::const_iterator i = oList.begin(); i != oList.end(); ++i )
		{
			if ( LoadFromFileHelper( (*i).second.m_T, pFile, szFilename, bScanOnly, bPartialOk ) )
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CImageServices::LoadFromFileHelper(IImageServicePlugin* pService, CImageFile* pFile, LPCTSTR szFilename, BOOL bScanOnly, BOOL bPartialOk)
{
	// Get file extension
	CString strType( PathFindExtension( szFilename ) ); // ".ext"
	strType.MakeLower();

	HINSTANCE hRes = AfxGetResourceHandle();	// Save resource handle for old plugins
	BOOL bSuccess = FALSE;

	// First chance - load directly from file
	SAFEARRAY* pArray = NULL;
	IMAGESERVICEDATA pParams = {};
	pParams.cbSize = sizeof( pParams );
	if ( bScanOnly )  pParams.nFlags |= IMAGESERVICE_SCANONLY;
	if ( bPartialOk ) pParams.nFlags |= IMAGESERVICE_PARTIAL_IN;

	HRESULT hr = pService->LoadFromFile( CComBSTR( szFilename ), &pParams, &pArray );
	if ( SUCCEEDED( hr ) )
		bSuccess = PostLoad( pFile, &pParams, pArray );
	else if ( SERVERLOST( hr ) )
		Plugins.ReloadPlugin( L"ImageService", strType );	// Plugin died
	if ( pArray )
		VERIFY( SUCCEEDED( SafeArrayDestroy( pArray ) ) );

	// Second chance - load from memory
	if ( ! bSuccess && ( hr == E_NOTIMPL ) )
	{
		HANDLE hFile = CreateFile( szFilename, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		VERIFY_FILE_ACCESS( hFile, szFilename )
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			if ( GetFileSize( hFile, NULL ) < 10*1024*1024 )	// Max size 10 MB
			{
				if ( HANDLE hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL ) )
				{
					if ( LPCVOID pBuffer = MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0 ) )
					{
						bSuccess = LoadFromMemory( pFile, strType, pBuffer,
							GetFileSize( hFile, NULL ), bScanOnly, bPartialOk );

						VERIFY( UnmapViewOfFile( pBuffer ) );
					}
					VERIFY( CloseHandle( hMap ) );
				}
			}
			VERIFY( CloseHandle( hFile ) );
		}
	}

	AfxSetResourceHandle( hRes );

	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices post load

BOOL CImageServices::PostLoad(CImageFile* pFile, const IMAGESERVICEDATA* pParams, SAFEARRAY* pArray)
{
	pFile->m_bScanned		= TRUE;
	pFile->m_nWidth			= pParams->nWidth;
	pFile->m_nHeight		= pParams->nHeight;
	pFile->m_nComponents	= pParams->nComponents;
	if ( pArray == NULL )
		return TRUE;		// Scanned only

	LONG nArray = 0;
	if ( SUCCEEDED( SafeArrayGetUBound( pArray, 1, &nArray ) ) )
	{
		nArray++;
		LONG nFullSize = ( ( pParams->nWidth * pParams->nComponents + 3 ) & ~3 ) * pParams->nHeight;
		if ( nArray == nFullSize )
		{
			pFile->m_pImage = new BYTE[ nArray ];
			if ( pFile->m_pImage )
			{
				LPBYTE pData;
				if ( SUCCEEDED( SafeArrayAccessData( pArray, (VOID**)&pData ) ) )
				{
					CopyMemory( pFile->m_pImage, pData, nArray );
					pFile->m_bLoaded = TRUE;

					VERIFY( SUCCEEDED( SafeArrayUnaccessData( pArray ) ) );
				}
			}
		}
	}

	return pFile->m_bLoaded;
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices save operations

BOOL CImageServices::SaveToMemory(CImageFile* pFile, LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength)
{
	*ppBuffer = NULL;
	*pnLength = 0;

	CComQIPtr< IImageServicePlugin > pService( Plugins.GetPlugin( L"ImageService", pszType ) );
	if ( ! pService )
		return FALSE;

	SAFEARRAY* pSource = ImageToArray( pFile );
	if ( pSource == NULL )
		return FALSE;

	IMAGESERVICEDATA pParams = {};
	pParams.cbSize		= sizeof( pParams );
	pParams.nWidth		= pFile->m_nWidth;
	pParams.nHeight		= pFile->m_nHeight;
	pParams.nComponents	= pFile->m_nComponents;
	pParams.nQuality	= nQuality;

	SAFEARRAY* pOutput = NULL;
	HINSTANCE hRes = AfxGetResourceHandle();
	BOOL bSuccess = SUCCEEDED( pService->SaveToMemory( CComBSTR( pszType ), &pOutput, &pParams, pSource ) );
	AfxSetResourceHandle( hRes );

	SafeArrayDestroy( pSource );

	if ( pOutput == NULL )
		return FALSE;

	SafeArrayGetUBound( pOutput, 1, (PLONG)pnLength );
	(*pnLength)++;

	LPBYTE pEncoded;
	SafeArrayAccessData( pOutput, (VOID**)&pEncoded );

	*ppBuffer = new BYTE[ *pnLength ];
	CopyMemory( *ppBuffer, pEncoded, *pnLength );

	SafeArrayUnaccessData( pOutput );
	SafeArrayDestroy( pOutput );

	return bSuccess;
}

BOOL CImageServices::SaveToFile(CImageFile* pFile, LPCTSTR szFilename, int nQuality, DWORD* pnLength)
{
	if ( pnLength )
		*pnLength = 0;

	// Get file extension
	CString strType( PathFindExtension( szFilename ) ); // ".ext"
	strType.MakeLower();

	CComQIPtr< IImageServicePlugin > pService(
		Plugins.GetPlugin( L"ImageService", strType ) );
	if ( ! pService )
		return FALSE;

	SAFEARRAY* pSource = ImageToArray( pFile );
	if ( pSource == NULL )
		return FALSE;

	IMAGESERVICEDATA pParams = {};
	pParams.cbSize		= sizeof( pParams );
	pParams.nWidth		= pFile->m_nWidth;
	pParams.nHeight		= pFile->m_nHeight;
	pParams.nComponents	= pFile->m_nComponents;
	pParams.nQuality	= nQuality;

	HINSTANCE hRes = AfxGetResourceHandle();
	BOOL bSuccess = SUCCEEDED( pService->SaveToFile( CComBSTR( szFilename ), &pParams, pSource ) );
	AfxSetResourceHandle( hRes );

	SafeArrayDestroy( pSource );

	if ( pnLength )
		*pnLength = (DWORD)GetFileSize( szFilename );

	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices pre save utility

SAFEARRAY* CImageServices::ImageToArray(CImageFile* pFile)
{
	SAFEARRAY* pOutput;

	if ( FAILED( SafeArrayAllocDescriptor( 1, &pOutput ) ) || pOutput == NULL ) return NULL;

	DWORD nLength = ( ( pFile->m_nWidth * pFile->m_nComponents + 3 ) & ~3 ) * pFile->m_nHeight;

	pOutput->cbElements = 1;
	pOutput->rgsabound[ 0 ].lLbound = 0;
	pOutput->rgsabound[ 0 ].cElements = nLength;

	if ( FAILED( SafeArrayAllocData( pOutput ) ) ) return NULL;

	LPBYTE pTarget;
	if ( FAILED( SafeArrayAccessData( pOutput, (void HUGEP* FAR*)&pTarget ) ) ) return NULL;

	CopyMemory( pTarget, pFile->m_pImage, nLength );

	SafeArrayUnaccessData( pOutput );

	return pOutput;
}

BOOL CImageServices::IsFileViewable(LPCTSTR pszPath)
{
	// Get file extension
	CString strType( PathFindExtension( pszPath ) );
	if ( strType.IsEmpty() ) return FALSE;
	strType.MakeLower();

	CLSID oCLSID;
	return Plugins.LookupCLSID( L"ImageService", strType, oCLSID );
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices service discovery and control

BOOL CImageServices::LookupUniversalPlugins(service_list& oList)
{
	HUSKEY hKey;
	if ( SHRegOpenUSKey( REGISTRY_KEY L"\\Plugins\\ImageService",
		KEY_READ, NULL, &hKey, FALSE ) == ERROR_SUCCESS )
	{
		for ( DWORD nKey = 0; ; nKey++ )
		{
			TCHAR szType[ 128 ] = {}, szCLSID[ 64 ] = {};
			DWORD dwType, dwTypeLen = 128, dwCLSIDLen = 64 * sizeof( TCHAR ) - 1;

			if ( SHRegEnumUSValue( hKey, nKey, szType, &dwTypeLen, &dwType,
				(LPBYTE)szCLSID, &dwCLSIDLen, SHREGENUM_DEFAULT ) != ERROR_SUCCESS )
				break;

			if ( dwType != REG_SZ || *szType == L'.' )
				continue;

			CLSID pCLSID;
			if ( ! Hashes::fromGuid( szCLSID, &pCLSID ) )
				continue;

			CComQIPtr< IImageServicePlugin > pService( Plugins.GetPlugin( L"ImageService", szType ) );
			if ( pService )
				oList.push_back( service_list::value_type( pCLSID, pService ) );
		}

		SHRegCloseUSKey( hKey );
	}

	return ! oList.empty();
}


///////////////////////////////////////////////
// IImageServicePlugin
//

IMPLEMENT_UNKNOWN(CImageServices, ImageService)

STDMETHODIMP CImageServices::XImageService::LoadFromFile( __in BSTR sFile, __inout IMAGESERVICEDATA* pParams, __out SAFEARRAY** ppImage )
{
	METHOD_PROLOGUE(CImageServices, ImageService)

	// Get file extension
	CString strType( PathFindExtension( sFile ) );	// ".ext"
	strType.MakeLower();

	CComQIPtr< IImageServicePlugin > pService( Plugins.GetPlugin( L"ImageService", strType ) );
	if ( pService )
	{
		if ( SUCCEEDED( pService->LoadFromFile( sFile, pParams, ppImage ) ) )
			return S_OK;
	}

	service_list oList;
	if ( ImageServices.LookupUniversalPlugins( oList ) )
	{
		for ( service_list::const_iterator i = oList.begin(); i != oList.end(); ++i )
		{
			if ( SUCCEEDED( (*i).second.m_T->LoadFromFile( sFile, pParams, ppImage ) ) )
				return S_OK;
		}
	}

	return E_FAIL;
}

STDMETHODIMP CImageServices::XImageService::LoadFromMemory( __in BSTR sType, __in SAFEARRAY* pMemory, __inout IMAGESERVICEDATA* pParams, __out SAFEARRAY** ppImage )
{
	METHOD_PROLOGUE(CImageServices, ImageService)

	CComQIPtr< IImageServicePlugin > pService( Plugins.GetPlugin( L"ImageService", sType ) );
	if ( pService )
		return pService->LoadFromMemory( sType, pMemory, pParams, ppImage );

	return E_FAIL;
}

STDMETHODIMP CImageServices::XImageService::SaveToFile( __in BSTR sFile, __inout IMAGESERVICEDATA* pParams, __in SAFEARRAY* pImage)
{
	METHOD_PROLOGUE(CImageServices, ImageService)

	// Get file extension
	CString strType( PathFindExtension( sFile ) ); // ".ext"
	strType.MakeLower();

	CComQIPtr< IImageServicePlugin > pService( Plugins.GetPlugin( L"ImageService", strType ) );
	if ( pService )
		return pService->SaveToFile( sFile, pParams, pImage );

	return E_FAIL;
}

STDMETHODIMP CImageServices::XImageService::SaveToMemory( __in BSTR sType, __out SAFEARRAY** ppMemory, __inout IMAGESERVICEDATA* pParams, __in SAFEARRAY* pImage)
{
	METHOD_PROLOGUE(CImageServices, ImageService)

	CComQIPtr< IImageServicePlugin > pService( Plugins.GetPlugin( L"ImageService", sType ) );
	if ( pService )
		return pService->SaveToMemory( sType, ppMemory, pParams, pImage );

	return E_FAIL;
}
