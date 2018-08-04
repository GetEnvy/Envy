//
// RegExp.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2010-2014 and Shareaza 2008-2010
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

// Abstracted std::regex Regular Expression support  (2 case-insensitive functions:)
//
// RegExp::Match	Returns TRUE if szContent finds szRegExp regular expression
// RegExp::Split	Divides szContent according szRegExp regular expression
//	Returns number of strings in function allocated pszResult (array of strings)
//	pszResult must be freed by GlobalFree() function


#include "StdAfx.h"
#include "RegExp.h"

#include <atlconv.h>
#include <tchar.h>
//#include <string>	// In StdAfx.h

// VS2008 SP1 for _HAS_TR1, VS2012+ for std
#include <regex>

#if !defined(_MSC_VER) || (_MSC_VER < 1700)		// VS2010~
#define std::regex std::tr1::regex
#define std::wregex std::tr1::wregex
#define std::wsmatch std::tr1::wsmatch
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


namespace RegExp
{

BOOL Match(LPCTSTR szRegExp, LPCTSTR szContent)
{
	try
	{
		const std::wstring sContent( (LPCWSTR)CT2CW( szContent ) );
		const std::wstring sRegExp( (LPCWSTR)CT2CW( szRegExp ) );
		const std::wregex regExpPattern( sRegExp, std::regex_constants::ECMAScript | std::regex_constants::icase );
		if ( std::regex_search( sContent, regExpPattern ) )
			return TRUE;
	}
	catch (...)
	{
	}
	return FALSE;
}

size_t Split(LPCTSTR szRegExp, LPCTSTR szContent, LPTSTR* pszResult)
{
	try
	{
		const std::wstring sContent( (LPCWSTR)CT2CW( szContent ) );
		const std::wstring sRegExp( (LPCWSTR)CT2CW( szRegExp ) );
		const std::wregex regExpPattern( sRegExp, std::regex_constants::ECMAScript | std::regex_constants::icase );
		std::wsmatch results;

		if ( std::regex_search( sContent, results, regExpPattern ) )
		{
			const size_t nCount = results.size();
			size_t len = 0;
			for ( size_t i = 0 ; i < nCount ; ++i )
			{
				len += results.str( i ).size() + 1;
			}

			if ( LPTSTR p = (LPTSTR)GlobalAlloc( GPTR, len * sizeof( wchar_t ) ) )
			{
				*pszResult = p;
				for ( size_t i = 0 ; i < nCount ; ++i )
				{
					wcscpy_s( p, len - ( p - *pszResult ), results.str( i ).c_str() );
					p += results.str( i ).size() + 1;
				}
				return nCount;
			}
		}
	}
	catch (...)
	{
	}
	*pszResult = NULL;
	return 0;
}

};
