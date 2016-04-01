//
// Registry.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//


#include "StdAfx.h"
#include "Registry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CRegistry read a string value

CString CRegistry::GetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszDefault, LPCTSTR pszSubKey, BOOL bIgnoreHKCU)
{
	CString strSection( pszSubKey ? pszSubKey : L"Software\\Envy\\Envy" );
	if ( pszSection && *pszSection )
	{
		if ( pszSection[0] != L'\\' )
			strSection += L"\\";
		strSection += pszSection;
	}

	// Read from HKCU then from HKLM
	DWORD nType = 0, nSize = 0;
	LONG nErrorCode = SHRegGetUSValue( (LPCTSTR)strSection, pszName, &nType,
		NULL, &nSize, bIgnoreHKCU, NULL, 0 );
	if ( nErrorCode == ERROR_SUCCESS && nType == REG_SZ && nSize >= sizeof( TCHAR ) && ( nSize & 1 ) == 0 )
	{
		CString strValue;
		nErrorCode = SHRegGetUSValue( (LPCTSTR)strSection, pszName, &nType,
			strValue.GetBuffer( nSize / sizeof( TCHAR ) ), &nSize, bIgnoreHKCU, NULL, 0 );
		strValue.ReleaseBuffer( nSize / sizeof( TCHAR ) - 1 );
		if ( nErrorCode == ERROR_SUCCESS )
			return strValue;
	}

	return pszDefault ? CString( pszDefault ) : CString();
}

//////////////////////////////////////////////////////////////////////
// CRegistry read an integer value

DWORD CRegistry::GetDword(LPCTSTR pszSection, LPCTSTR pszName, DWORD nDefault, LPCTSTR pszSubKey)
{
	CString strSection( pszSubKey ? pszSubKey : L"Software\\Envy\\Envy" );
	if ( pszSection && *pszSection )
	{
		if ( pszSection[0] != L'\\' )
			strSection += L"\\";
		strSection += pszSection;
	}

	// Read from HKCU then from HKLM
	DWORD nValue = 0;
	DWORD nType = 0, nSize = sizeof( nValue );
	LONG nErrorCode = SHRegGetUSValue( (LPCTSTR)strSection, pszName, &nType,
		(PBYTE)&nValue, &nSize, FALSE, NULL, 0 );
	if ( nErrorCode == ERROR_SUCCESS && nType == REG_DWORD && nSize == sizeof( nValue ) )
		return nValue;

	return nDefault;
}

//////////////////////////////////////////////////////////////////////
// CRegistry write a string value

BOOL CRegistry::SetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszValue, LPCTSTR pszSubKey)
{
	CString strSection( pszSubKey ? pszSubKey : L"Software\\Envy\\Envy" );
	if ( pszSection && *pszSection )
	{
		if ( pszSection[0] != L'\\' )
			strSection += L"\\";
		strSection += pszSection;
	}

	LONG nErrorCode = SHRegSetUSValue( (LPCTSTR)strSection, pszName, REG_SZ,
		(LPCVOID)pszValue, ( lstrlen( pszValue ) + 1 ) * sizeof( TCHAR ), SHREGSET_FORCE_HKCU );
	return ( nErrorCode == ERROR_SUCCESS );
}

//////////////////////////////////////////////////////////////////////
// CRegistry write an int value

BOOL CRegistry::SetDword(LPCTSTR pszSection, LPCTSTR pszName, DWORD nValue, LPCTSTR pszSubKey)
{
	CString strSection( pszSubKey ? pszSubKey : L"Software\\Envy\\Envy" );
	if ( pszSection && *pszSection )
	{
		if ( pszSection[0] != L'\\' )
			strSection += L"\\";
		strSection += pszSection;
	}

	LONG nErrorCode = SHRegSetUSValue( (LPCTSTR)strSection, pszName, REG_DWORD,
		(LPCVOID)&nValue, sizeof( nValue ), SHREGSET_FORCE_HKCU );
	return ( nErrorCode == ERROR_SUCCESS );
}

//////////////////////////////////////////////////////////////////////
// CRegistry recursively delete key

void CRegistry::DeleteKey(HKEY hParent, LPCTSTR pszKey)
{
	HKEY hKey;
	if ( RegOpenKeyEx( hParent, pszKey, 0, KEY_ALL_ACCESS, &hKey ) ) return;

	CArray< CString > pList;

	for ( DWORD nIndex = 0 ; ; nIndex++ )
	{
		DWORD dwName = 64;	// Input parameter in TCHARs
		TCHAR szName[64];

		LRESULT lResult = RegEnumKeyEx( hKey, nIndex, szName, &dwName, NULL, NULL, 0, NULL );
		if ( lResult != ERROR_SUCCESS ) break;

		szName[ dwName ] = 0;
		pList.Add( szName );
		DeleteKey( hKey, szName );
	}

	for ( int nItem = pList.GetSize() - 1 ; nItem >= 0 ; nItem-- )
	{
		RegDeleteKey( hKey, pList.GetAt( nItem ) );
	}

	if ( lstrlen( pszKey ) > 25 )	// Handle likely initial non-recursive value
		RegDeleteKey( HKEY_CURRENT_USER, pszKey );

	RegCloseKey( hKey );
}
