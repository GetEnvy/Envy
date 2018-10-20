//
// Registry.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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

#pragma once


struct CRegistry
{
	// Recursively remove registry key
	static void DeleteKey(HKEY hParent, LPCTSTR pszKey);

	static CString GetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszDefault = NULL, LPCTSTR pszSubKey = NULL, BOOL bIgnoreHKCU = FALSE);

	static DWORD GetDword(LPCTSTR pszSection, LPCTSTR pszName, DWORD dwDefault = 0, LPCTSTR pszSubKey = NULL);

	inline static bool GetBool(LPCTSTR pszSection, LPCTSTR pszName, bool bDefault = false, LPCTSTR pszSubKey = NULL)
	{
		return ( GetDword( pszSection, pszName, ( bDefault ? 1 : 0 ), pszSubKey ) != 0 );
	}

	inline static int GetInt(LPCTSTR pszSection, LPCTSTR pszName, int nDefault = 0, LPCTSTR pszSubKey = NULL)
	{
		return (int)GetDword( pszSection, pszName, (DWORD)nDefault, pszSubKey );
	}

	inline static double GetFloat(LPCTSTR pszSection, LPCTSTR pszName, double fDefault = 0.0f, LPCTSTR pszSubKey = NULL)
	{
		CString buf;
		buf.Format( L"%lg", fDefault );
		return _tstof( GetString( pszSection, pszName, buf, pszSubKey ) );
	}

	static BOOL	SetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszValue, LPCTSTR pszSubKey = NULL);

	static BOOL	SetDword(LPCTSTR pszSection, LPCTSTR pszName, DWORD nValue, LPCTSTR pszSubKey = NULL);

	inline static BOOL SetBool(LPCTSTR pszSection, LPCTSTR pszName, bool bValue, LPCTSTR pszSubKey = NULL)
	{
		return SetDword( pszSection, pszName, ( bValue ? 1 : 0 ), pszSubKey );
	}

	inline static BOOL SetInt(LPCTSTR pszSection, LPCTSTR pszName, int nValue, LPCTSTR pszSubKey = NULL)
	{
		return SetDword( pszSection, pszName, (DWORD)nValue, pszSubKey );
	}

	inline static BOOL SetFloat(LPCTSTR pszSection, LPCTSTR pszName, double fValue, LPCTSTR pszSubKey = NULL)
	{
		CString buf;
		buf.Format( L"%lg", fValue );
		return SetString( pszSection, pszName, buf, pszSubKey );
	}
};
