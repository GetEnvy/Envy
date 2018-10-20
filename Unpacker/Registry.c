//
// Registry.c
//
// This file is part of Envy (getenvy.com) © 2016-2018
//
// Portions of this page have been previously released into the public domain.
// You are free to redistribute and modify it without any restrictions.

#include "Unpacker.h"

static BOOL CheckKey(LPCTSTR lpSubKey);
static LSTATUS CreateKey(LPCTSTR lpSubKey, LPTSTR lpClass, LPTSTR lpData);
static LSTATUS DeleteKey(LPCTSTR lpSubKey, LPTSTR lpClass);

LSTATUS CreateSkinKeys()
{
	LSTATUS rtn = 0;
	TCHAR filename[MAX_PATH];
	TCHAR fullpath[MAX_PATH];
	TCHAR imagpath[MAX_PATH];

	GetModuleFileName( NULL, filename, MAX_PATH );
	_snwprintf( fullpath, MAX_PATH, L"\"%s\" \"%%1\"", filename );
	_snwprintf( imagpath, MAX_PATH, L"%s,0", filename );

	if ( ! CheckKey( L"SOFTWARE\\Classes\\.sks" ) )		// Shareaza
		rtn = CreateKey( L"SOFTWARE\\Classes\\.sks", L"", L"Envy.Package" );
	rtn = CreateKey( L"SOFTWARE\\Classes\\.psk", L"", L"Envy.Package" );
	rtn = CreateKey( L"SOFTWARE\\Classes\\.env", L"", L"Envy.Package" );
	rtn = CreateKey( L"SOFTWARE\\Classes\\.envy", L"", L"Envy.Package" );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateKey( L"SOFTWARE\\Classes\\Envy.Package", L"", L"Envy Package" );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateKey( L"SOFTWARE\\Classes\\Envy.Package\\DefaultIcon", L"", imagpath );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateKey( L"SOFTWARE\\Classes\\Envy.Package\\shell", L"", L"open" );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateKey( L"SOFTWARE\\Classes\\Envy.Package\\shell\\open\\command", L"", fullpath );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateKey( L"SOFTWARE\\Classes\\Envy.Package\\shell\\skininstall", L"", L"Install Envy Package" );
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateKey( L"SOFTWARE\\Classes\\Envy.Package\\shell\\skininstall\\command", L"", fullpath );
	if ( rtn != ERROR_SUCCESS ) return rtn;

	return ERROR_SUCCESS;
}

LSTATUS DeleteSkinKeys()
{
	if ( ! CheckKey( L"SOFTWARE\\Classes\\Shareaza.Package" ) )
		DeleteKey( L"SOFTWARE\\Classes\\.sks", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\.psk", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\.env", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\.envy", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\Envy.Package\\shell\\open\\command", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\Envy.Package\\shell\\open", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\Envy.Package\\shell\\skininstall\\command", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\Envy.Package\\shell\\skininstall", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\Envy.Package\\shell", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\Envy.Package\\DefaultIcon", L"" );
	DeleteKey( L"SOFTWARE\\Classes\\Envy.Package", L"" );

	return ERROR_SUCCESS;
}

static BOOL CheckKey(LPCTSTR lpSubKey)
{
	HKEY keyHandle;

	LSTATUS rtn = RegOpenKeyEx( HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, &keyHandle );
	if ( rtn != ERROR_SUCCESS ) return FALSE;

	RegCloseKey( keyHandle );
	return TRUE;
}

static LSTATUS CreateKey(LPCTSTR lpSubKey, LPTSTR lpClass, LPTSTR lpData)
{
	HKEY keyHandle;

	LSTATUS rtn = RegCreateKeyEx( HKEY_CURRENT_USER, lpSubKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &keyHandle, NULL );
	if ( rtn != ERROR_SUCCESS ) return rtn;

	DWORD aLen = (DWORD)wcslen(lpData) * sizeof(TCHAR) + 1;
	RegSetValueEx( keyHandle, lpClass, 0, REG_SZ, (LPBYTE)lpData, aLen );

	rtn = RegCloseKey( keyHandle );
	return rtn;
}

static LSTATUS DeleteKey(LPCTSTR lpSubKey, LPTSTR lpClass)
{
	HKEY keyHandle;

	LSTATUS rtn = RegOpenKeyEx( HKEY_CURRENT_USER, lpSubKey, 0, KEY_ALL_ACCESS, &keyHandle );
	if ( rtn != ERROR_SUCCESS ) return rtn;

	RegDeleteKey( keyHandle, lpClass );

	rtn = RegCloseKey( keyHandle );
	return rtn;
}
