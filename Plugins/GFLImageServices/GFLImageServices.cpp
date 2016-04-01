//
// GFLImageServices.cpp : Implementation of DLL Exports.
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Nikolay Raspopov 2005
//
// GFL Library, GFL SDK and XnView
// Copyright (c) 1991-2004 Pierre-E Gougelet
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "Resource.h"
#include "GFLImageServices.h"	// Generated

#define REG_IMAGESERVICE_KEY L"Software\\Envy\\Envy\\Plugins\\ImageService"

class CGFLImageServicesModule : public CAtlDllModuleT< CGFLImageServicesModule >
{
public :
	DECLARE_LIBID (LIBID_GFLImageServicesLib)
	DECLARE_REGISTRY_APPID_RESOURCEID (IDR_GFLIMAGESERVICES, "{4DD7500D-4ACC-4833-AB1D-887C59199DC5}")
};

CGFLImageServicesModule	_AtlModule;
HINSTANCE				_hModuleInstance = NULL;
typedef ATL::CAtlMap < ATL::CStringA, GFL_INT32 > CAtlStrStrMap;
CAtlStrStrMap			_ExtMap;

inline void FillExtMap()
{
	_ExtMap.RemoveAll();

	GFL_INT32 count = gflGetNumberOfFormat();
	ATLTRACE( "Total %d formats:\n", count );
	for ( GFL_INT32 i = 0; i < count; ++i )
	{
		GFL_FORMAT_INFORMATION info = {};
		GFL_ERROR err = gflGetFormatInformationByIndex (i, &info);
		if ( err == GFL_NO_ERROR && (info.Status & GFL_READ) )
		{
			ATLTRACE( "%3d. %7s %32s #%d :", i, info.Name, info.Description, info.Index );
			for ( GFL_UINT32 j = 0; j < info.NumberOfExtension; ++j )
			{
				CStringA ext( info.Extension[j] );
				ext.MakeLower();

				// GFL bugfix for short extensions
				if ( ext.GetLength() > 6 )
				{
					if ( ext == "pspimag" )			// PaintShopPro Image
						ext = "pspimage";
					else if ( ext == "pspbrus" )	// PaintShopPro Brush
						ext = "pspbrush";
					else if ( ext == "pspfram" )	// PaintShopPro Frame
						ext = "pspframe";
				}

				ATLTRACE( " .%s", ext );
				GFL_INT32 index;
				if ( ! _ExtMap.Lookup( ext, index ) )
					_ExtMap.SetAt( ext, info.Index );
			}
			ATLTRACE( "\n" );
		}
	}
}

BOOL SafeGFLInit() throw()
{
	__try
	{
		// Library initialization
		if ( gflLibraryInit() != GFL_NO_ERROR )
		{
			ATLTRACE( "gflLibraryInit failed\n" );
			return FALSE;
		}
		gflEnableLZW( GFL_TRUE );
		FillExtMap();
		return TRUE;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE( "Exception in DLL_PROCESS_ATTACH\n" );
		return FALSE;
	}
}

void SafeGFLExit() throw()
{
	__try
	{
		gflLibraryExit();
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE( "Exception in DLL_PROCESS_DETACH\n" );
	}
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (_AtlModule.DllMain( dwReason, lpReserved ) )
	{
		switch (dwReason)
		{
		case DLL_PROCESS_ATTACH:
			_hModuleInstance = hInstance;
			return SafeGFLInit();

		case DLL_PROCESS_DETACH:
			SafeGFLExit();
			break;
		}
		return TRUE;
	}
	else
	{
		ATLTRACE( "FALSE in _AtlModule.DllMain() call\n" );
		return FALSE;
	}
}

STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
	HRESULT hr = _AtlModule.DllRegisterServer();

	// Registering extensions using GFL
	for ( POSITION pos = _ExtMap.GetStartPosition (); pos; )
	{
		CStringA ext;
		GFL_INT32 index;
		_ExtMap.GetNextAssoc( pos, ext, index );
		if ( ext == "vst" ) continue;
		ext.Insert( 0, '.' );
		ATLTRACE( "Add %s\n", ext );
		SHSetValue( HKEY_CURRENT_USER, REG_IMAGESERVICE_KEY, CA2T( ext ), REG_SZ,
			L"{C9314782-CB91-40B8-B375-F631FF30C1C8}",
			38 * sizeof(TCHAR) );
	}

	return hr;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();

	// Unregistering extensions using GFL
	for ( POSITION pos = _ExtMap.GetStartPosition (); pos; )
	{
		CStringA ext;
		GFL_INT32 index;
		_ExtMap.GetNextAssoc( pos, ext, index );
		if ( ext == "vst" ) continue;
		ext.Insert( 0, '.' );
		ATLTRACE( "Remove %s\n", ext );
		SHDeleteValue( HKEY_CURRENT_USER, REG_IMAGESERVICE_KEY, CA2T( ext ) );
	}

	return hr;
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	static const wchar_t szUserSwitch[] = L"user";

	if ( pszCmdLine && _wcsnicmp( pszCmdLine, szUserSwitch, _countof(szUserSwitch) ) == 0 )
		AtlSetPerUserRegistration( true );	// VS2008+

	HRESULT hr = bInstall ?
		DllRegisterServer() :
		DllUnregisterServer();
	if ( bInstall && FAILED( hr ) )
		DllUnregisterServer();

	return hr;
}

HRESULT SAFEgflLoadBitmap(LPCWSTR filename, GFL_BITMAP **bitmap, const GFL_LOAD_PARAMS *params, GFL_FILE_INFORMATION *info) throw()
{
	HRESULT hr = E_FAIL;
	__try
	{
		GFL_ERROR err = gflLoadBitmapW(filename, bitmap, params, info);
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
			ATLTRACE( "gflLoadBitmap() error : %s\n", gflGetErrorString(err) );
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE( "gflLoadBitmap() exception\n" );
	}
	return hr;
}

HRESULT SAFEgflLoadBitmapFromMemory(const GFL_UINT8 *data, GFL_UINT32 data_length, GFL_BITMAP **bitmap, const GFL_LOAD_PARAMS *params, GFL_FILE_INFORMATION *info) throw()
{
	HRESULT hr = E_FAIL;
	__try
	{
		GFL_ERROR err = gflLoadBitmapFromMemory(data, data_length, bitmap, params, info);
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
			ATLTRACE( "gflLoadBitmapFromMemory() error : %s\n", gflGetErrorString(err) );
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE( "gflLoadBitmapFromMemory() exception\n" );
	}
	return hr;
}

HRESULT SAFEgflSaveBitmapIntoMemory(GFL_UINT8 **data, GFL_UINT32 *data_length, const GFL_BITMAP *bitmap, const GFL_SAVE_PARAMS *params) throw()
{
	HRESULT hr = E_FAIL;
	__try
	{
		GFL_ERROR err = gflSaveBitmapIntoMemory(data, data_length, bitmap, params);
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
			ATLTRACE( "gflSaveBitmapIntoMemory() error : %s\n", gflGetErrorString(err) );
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE( "gflSaveBitmapIntoMemory() exception\n" );
	}
	return hr;
}

HRESULT SAFEgflSaveBitmap(LPCWSTR filename, const GFL_BITMAP *bitmap, const GFL_SAVE_PARAMS *params) throw()
{
	HRESULT hr = E_FAIL;
	__try
	{
		GFL_ERROR err = gflSaveBitmapW( (wchar_t*)filename, bitmap, params );
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
			ATLTRACE( "gflSaveBitmap() error : %s\n", gflGetErrorString(err) );
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE( "gflSaveBitmap() exception\n" );
	}
	return hr;
}

int GetFormatIndexByExt(LPCSTR ext)
{
	GFL_INT32 index;
	if ( _ExtMap.Lookup( ext, index ) )
		return index;	// gflGetFormatIndexByName( CT2CA(name) );

	return -1;
}
