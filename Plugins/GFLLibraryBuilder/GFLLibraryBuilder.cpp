//
// GFLLibraryBuilder.cpp : Implementation of DLL Exports.
//
// This file is part of Envy (getenvy.com) © 2016-2018
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
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "Resource.h"
#include "GFLLibraryBuilder.h"

#define REG_LIBRARYBUILDER_KEY L"Software\\Envy\\Envy\\Plugins\\LibraryBuilder"

class CGFLLibraryBuilderModule : public CAtlDllModuleT< CGFLLibraryBuilderModule >
{
public :
	DECLARE_LIBID(LIBID_GFLLibraryBuilderLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_GFLLIBRARYBUILDER, "{F74AD137-A43F-46FD-A1FE-6532C3FC3E88}")
};

CGFLLibraryBuilderModule _AtlModule;
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
		GFL_ERROR err = gflGetFormatInformationByIndex(i, &info);
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
	return _AtlModule.DllGetClassObject( rclsid, riid, ppv );
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
		if ( ext == "pdf" || ext == "ps" || ext == "eps" || ext == "vst" )
			continue;
		ext.Insert( 0, '.' );
		ATLTRACE( "Add %s\n", ext );
		SHSetValue( HKEY_CURRENT_USER, REG_LIBRARYBUILDER_KEY, CA2T( ext ), REG_SZ,
			L"{C937FE9E-FC47-49F8-A115-1925D95E1FE5}",
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
		if ( ext == "pdf" || ext == "ps" || ext == "eps" || ext == "vst" )
			continue;
		ext.Insert( 0, '.' );
		ATLTRACE( "Remove %s\n", ext );
		SHDeleteValue( HKEY_CURRENT_USER, REG_LIBRARYBUILDER_KEY, CA2T( ext ) );
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
