//
// StdAfx.cpp : Empty workaround stub file
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2014 and Nikolay Raspopov 2014
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


CString LoadString( UINT nID )
{
	CString str;
	str.LoadString( nID );
	return str;
}

CString GetURLs()
{
	CString strURLs;
	DWORD nType = 0, nSize = MAX_PATH * sizeof( TCHAR ) * 100;
	LPTSTR szData = strURLs.GetBuffer( nSize );
	if ( SHRegGetUSValue( LoadString( IDS_KEY ), L"URLs", &nType, szData, &nSize, FALSE, NULL, 0 ) != ERROR_SUCCESS || nType != REG_SZ )
		nSize = 0;
	szData[ nSize / sizeof( TCHAR ) ] = L'\0';
	strURLs.ReleaseBuffer();
	strURLs.Trim();
	return ( strURLs.IsEmpty() ? LoadString( IDS_URL ) : strURLs );
}

BOOL SaveURLs( const CString& sURLs )
{
	return ( SHRegSetUSValue( LoadString( IDS_KEY ), L"URLs", REG_SZ, (LPCTSTR)sURLs, sURLs.GetLength() * sizeof( TCHAR ), SHREGSET_FORCE_HKCU ) == ERROR_SUCCESS );
}
