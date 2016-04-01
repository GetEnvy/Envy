//
// RARBuilder.cpp : Implementation of DLL Exports.
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2007
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
#include "RARBuilder.h"

class CModule : public CAtlDllModuleT< CModule >
{
public:
	CModule();
	virtual ~CModule();
	DECLARE_LIBID( LIBID_RARBuilderLib )
	DECLARE_REGISTRY_APPID_RESOURCEID( IDR_APP, "{C2F61D04-BE35-4AE3-84A8-BE140613E26D}" )

protected:
	HMODULE	m_hUnrar;

	bool LoadUnrar();
	void UnloadUnrar();
};

CModule::CModule() :
	m_hUnrar( NULL )
{
	if ( ! LoadUnrar() )
		UnloadUnrar();
}

CModule::~CModule()
{
	UnloadUnrar();
}

bool CModule::LoadUnrar()
{
#ifdef _WIN64
	LPCTSTR szUnRAR = L"Unrar64.dll";
#else
	LPCTSTR szUnRAR = L"Unrar.dll";
#endif
	m_hUnrar = LoadLibrary( szUnRAR );
	if ( ! m_hUnrar )
	{
		TCHAR szPath[ MAX_PATH ] = {};
		GetModuleFileName( _AtlBaseModule.GetModuleInstance(), szPath, MAX_PATH );
		LPTSTR c = _tcsrchr( szPath, L'\\' );
		if ( ! c )
			return false;
		lstrcpy( c + 1, szUnRAR );
		m_hUnrar = LoadLibrary( szPath );
		if ( ! m_hUnrar )
		{
			*c = L'\0';
			c = _tcsrchr( szPath, L'\\' );
			if ( ! c )
				return false;
			lstrcpy( c + 1, szUnRAR );
			m_hUnrar = LoadLibrary( szPath );
			if ( ! m_hUnrar )
				return false;
		}
	}
	fnRAROpenArchiveEx = (tRAROpenArchiveEx)GetProcAddress( m_hUnrar, "RAROpenArchiveEx");
	fnRARCloseArchive = (tRARCloseArchive)GetProcAddress( m_hUnrar, "RARCloseArchive");
	fnRARReadHeaderEx = (tRARReadHeaderEx)GetProcAddress( m_hUnrar, "RARReadHeaderEx");
	fnRARProcessFileW = (tRARProcessFileW)GetProcAddress( m_hUnrar, "RARProcessFileW");
	return ( fnRAROpenArchiveEx && fnRARCloseArchive && fnRARReadHeaderEx && fnRARProcessFileW );
}

void CModule::UnloadUnrar()
{
	if ( m_hUnrar )
	{
		fnRAROpenArchiveEx = NULL;
		fnRARCloseArchive = NULL;
		fnRARReadHeaderEx = NULL;
		fnRARProcessFileW = NULL;
		FreeLibrary( m_hUnrar );
		m_hUnrar = NULL;
	}
}

CModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved)
{
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
