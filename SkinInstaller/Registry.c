//
// Registry.c
//
// This file is part of Envy (getenvy.com) © 2016
//
// Portions of this page have been previously released into the public domain.
// You are free to redistribute and modify it without any restrictions
// with the exception of the following notice:
//
// The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
// The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.

#include "Skin.h"

static LSTATUS CreateHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass, LPTSTR lpData);
static LSTATUS DeleteHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass);

LSTATUS CreateSkinKeys()
{
	LSTATUS rtn = 0;
	TCHAR filename[MAX_PATH];
	TCHAR fullpath[MAX_PATH];
	TCHAR imagpath[MAX_PATH];

	GetModuleFileName( NULL, filename, MAX_PATH );
	_snwprintf( fullpath, MAX_PATH, L"\"%s\" \"%%1\"", filename );
	_snwprintf( imagpath, MAX_PATH, L"%s,0", filename );

	rtn = CreateHKCRKey( L"SOFTWARE\\Classes\\.sks", L"", L"Envy.SkinFile" );
	rtn = CreateHKCRKey( L"SOFTWARE\\Classes\\.psk", L"", L"Envy.SkinFile" );
	rtn = CreateHKCRKey( L"SOFTWARE\\Classes\\.envy", L"", L"Envy.SkinFile" );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey( L"Envy.SkinFile", L"", L"Envy Skin File" );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\DefaultIcon", L"", imagpath );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell", L"", L"open" );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell\\open\\command", L"", fullpath );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell\\skininstall", L"", L"Install Envy Skin" );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell\\skininstall\\command", L"", fullpath );
	if ( rtn != ERROR_SUCCESS ) return rtn;

	return ERROR_SUCCESS;
}

LSTATUS DeleteSkinKeys()
{
	DeleteHKCRKey( L"SOFTWARE\\Classes\\.envy", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\.psk", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\.sks", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell\\open\\command", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell\\open", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell\\skininstall\\command", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell\\skininstall", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\shell", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile\\DefaultIcon", L"" );
	DeleteHKCRKey( L"SOFTWARE\\Classes\\Envy.SkinFile", L"" );

	return ERROR_SUCCESS;
}

static LSTATUS CreateHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass, LPTSTR lpData)
{
	HKEY keyHandle;
	DWORD aLen;

	LSTATUS rtn = RegCreateKeyEx( HKEY_CURRENT_USER, lpSubKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &keyHandle, NULL );
	if ( rtn != ERROR_SUCCESS ) return rtn;

	aLen = (DWORD)wcslen(lpData) * sizeof(TCHAR) + 1;
	RegSetValueEx( keyHandle, lpClass, 0, REG_SZ, (LPBYTE)lpData, aLen );
	rtn = RegCloseKey( keyHandle );

	return rtn;
}

static LSTATUS DeleteHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass)
{
	HKEY keyHandle;

	LSTATUS rtn = RegOpenKeyEx( HKEY_CURRENT_USER, lpSubKey, 0, KEY_ALL_ACCESS, &keyHandle );
	if ( rtn != ERROR_SUCCESS ) return rtn;

	RegDeleteKey( keyHandle, lpClass );
	rtn = RegCloseKey( keyHandle );

	return rtn;
}
