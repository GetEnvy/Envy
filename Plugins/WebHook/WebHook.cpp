//
// WebHook.cpp : Implementation of DLL Exports.
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2009 and PeerProject 2009-2014
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
#include "WebHook_i.h"

class CWebHookModule : public CAtlDllModuleT< CWebHookModule >
{
public :
	DECLARE_LIBID(LIBID_WebHookLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WEBHOOK, "{07D1F248-BCA2-4257-8863-C9ACCFBAD83D}")
};

CWebHookModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved)
{
	// Black-listed processes
	if ( dwReason == DLL_PROCESS_ATTACH )
	{
		TCHAR szName[ MAX_PATH ] = {};
		DWORD dwLength = GetModuleFileName( NULL, szName, _countof( szName ) );

		// Windows Explorer
		if ( lstrcmpi( szName + dwLength - 13, L"\\explorer.exe" ) == 0 )
			return FALSE;
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

STDAPI DllRegisterServer(void)
{
	return _AtlModule.DllRegisterServer();
}

STDAPI DllUnregisterServer(void)
{
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
