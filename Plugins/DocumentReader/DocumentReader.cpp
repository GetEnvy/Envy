//
// DocumentReader.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2005
// Originally Created by:	Rolandas Rudomanskis
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
#include "DocumentReader.h"

////////////////////////////////////////////////////////////////////////
// Globals for this module.
//
HINSTANCE         v_hModule;				// DLL module handle
ULONG             v_cLocks;					// Count of server locks
CRITICAL_SECTION  v_csSynch;				// Critical Section
HANDLE            v_hPrivateHeap;			// Private Heap for Component
PFN_STGOPENSTGEX  v_pfnStgOpenStorageEx;	// StgOpenStorageEx

class CDocumentReaderModule : public CAtlDllModuleT< CDocumentReaderModule >
{
public :
	DECLARE_LIBID(LIBID_DocumentReaderLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_DOCUMENTREADER, "{BEC42E3F-4B6B-49A3-A099-EB3D6752AA02}")
};

CDocumentReaderModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch ( dwReason )
	{
	case DLL_PROCESS_ATTACH:
		v_cLocks = 0;
		v_hModule = hInstance;
		v_hPrivateHeap = HeapCreate( 0, 0x1000, 0 );
		v_pfnStgOpenStorageEx = (PFN_STGOPENSTGEX)GetProcAddress( GetModuleHandle( L"OLE32" ), "StgOpenStorageEx" );
		InitializeCriticalSection( &v_csSynch );
		DisableThreadLibraryCalls( hInstance );
		break;

	case DLL_PROCESS_DETACH:
		if ( v_hPrivateHeap ) HeapDestroy( v_hPrivateHeap );
		DeleteCriticalSection( &v_csSynch );
		break;
	}

	return _AtlModule.DllMain( dwReason, lpReserved );
}

STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject( rclsid, riid, ppv );
}

// Add entries to system registry
STDAPI DllRegisterServer(void)
{
	LPWSTR  pwszModule;

	// If we can't find the path to the DLL, we can't register...
	if ( ! FGetModuleFileName( v_hModule, &pwszModule ) )
		return E_UNEXPECTED;

	return _AtlModule.DllRegisterServer();
}

// Remove entries from system registry
STDAPI DllUnregisterServer(void)
{
	LPWSTR  pwszModule;

	// If we can't find the path to the DLL, we can't unregister...
	if ( ! FGetModuleFileName( v_hModule, &pwszModule) )
		return E_UNEXPECTED;

	return _AtlModule.DllUnregisterServer();
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
