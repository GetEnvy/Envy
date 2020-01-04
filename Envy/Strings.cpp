//
// Strings.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2020
// Portions copyright Shareaza 2010 and PeerProject 2010-2016
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

#include "StdAfx.h"
#include "Strings.h"

#ifndef XPSUPPORT	// inet_ntoa deprecated, use InetNtop Vista+
#include <Ws2tcpip.h>
#endif

bool IsCharacter(const WCHAR nChar)
{
	WORD nCharType = 0;

	if ( GetStringTypeW( CT_CTYPE3, &nChar, 1, &nCharType ) )
		return ( nCharType & C3_ALPHA
			|| ( ( nCharType & ( C3_KATAKANA | C3_HIRAGANA ) ) && ( nCharType & C3_DIACRITIC ) )
			|| iswdigit( nChar ) );

	return false;
}

bool IsHiragana(const WCHAR nChar)
{
	WORD nCharType = 0;

	if ( GetStringTypeW( CT_CTYPE3, &nChar, 1, &nCharType ) )
		return ( nCharType & C3_HIRAGANA ) != 0;

	return false;
}

bool IsKatakana(const WCHAR nChar)
{
	WORD nCharType = 0;

	if ( GetStringTypeW( CT_CTYPE3, &nChar, 1, &nCharType ) )
		return ( nCharType & C3_KATAKANA ) != 0;

	return false;
}

bool IsKanji(const WCHAR nChar)
{
	WORD nCharType = 0;

	if ( GetStringTypeW( CT_CTYPE3, &nChar, 1, &nCharType ) )
		return ( nCharType & C3_IDEOGRAPH ) != 0;

	return false;
}

bool IsNumber(LPCTSTR pszString, size_t nStart /*0*/, size_t nLength /*0*/)
{
	if ( ! nLength ) nLength--;
	for ( pszString += nStart; *pszString && nLength; pszString++, nLength-- )
	{
		if ( ! _istdigit( *pszString ) )
			return false;
	}
	return true;
}

bool IsWord(LPCTSTR pszString, size_t nStart, size_t nLength)
{
	for ( pszString += nStart; *pszString && nLength; ++pszString, --nLength )
	{
		if ( _istdigit( *pszString ) )
			return false;
	}
	return true;
}

void IsType(LPCTSTR pszString, size_t nStart, size_t nLength, bool& bWord, bool& bDigit, bool& bMix)
{
	bWord = false;
	bDigit = false;
	for ( pszString += nStart; *pszString && nLength; ++pszString, --nLength )
	{
		if ( _istdigit( *pszString ) )
			bDigit = true;
		else if ( IsCharacter( *pszString ) )
			bWord = true;
	}

	bMix = bWord && bDigit;
	if ( bMix )
	{
		bWord = false;
		bDigit = false;
	}
}

const CLowerCaseTable ToLower;

CLowerCaseTable::CLowerCaseTable()
{
	for ( size_t i = 0; i < 65536; ++i )	cTable[ i ] = TCHAR( i );
	CharLowerBuff( cTable, 65536 );

	cTable[ 0x0130 ] = 0x0069;	// Turkish Capital I with dot to "i"			(304 to 105)

	cTable[ 0x03A3 ] = 0x03C3;	// Greek Capital Sigma to Greek Small Sigma		(931 to 963)
	cTable[ 0x03C2 ] = 0x03C3;	// Greek Final Sigma to Greek Small Sigma		(962 to 963)

	cTable[ 0x0401 ] = 0x0435;	// Russian Capital Io to Russian Small Ie		(1025 to 1077)
	cTable[ 0x0451 ] = 0x0435;	// Russian Small Io to Russian Small Ie			(1105 to 1077)
	cTable[ 0x0419 ] = 0x0438;	// Russian Capital Short I to Russian Small I	(1049 to 1080)
	cTable[ 0x0439 ] = 0x0438;	// Russian Small Short I to Russian Small I 	(1081 to 1080)

	// Convert fullwidth latin characters to halfwidth
	for ( size_t i = 65281; i < 65313; ++i )	cTable[ i ] = TCHAR( i - 65248 );
	for ( size_t i = 65313; i < 65339; ++i )	cTable[ i ] = TCHAR( i - 65216 );
	for ( size_t i = 65339; i < 65375; ++i )	cTable[ i ] = TCHAR( i - 65248 );

	// Convert circled katakana to ordinary katakana
	for ( size_t i = 13008; i < 13028; ++i )	cTable[ i ] = TCHAR( 2 * i - 13566 );
	for ( size_t i = 13028; i < 13033; ++i )	cTable[ i ] = TCHAR( i - 538 );
	for ( size_t i = 13033; i < 13038; ++i )	cTable[ i ] = TCHAR( 3 * i - 26604 );
	for ( size_t i = 13038; i < 13043; ++i )	cTable[ i ] = TCHAR( i - 528 );
	for ( size_t i = 13043; i < 13046; ++i )	cTable[ i ] = TCHAR( 2 * i - 13571 );
	for ( size_t i = 13046; i < 13051; ++i )	cTable[ i ] = TCHAR( i - 525 );
	cTable[ 13051 ] = TCHAR( 12527 );
	for ( size_t i = 13052; i < 13055; ++i )	cTable[ i ] = TCHAR( i - 524 );

	// Map Katakana middle dot to space, since no API identifies it as a punctuation
	cTable[ 12539 ] = cTable[ 65381 ] = L' ';

	// Map CJK Fullwidth space to halfwidth space
	cTable[ 12288 ] = L' ';

	// Convert japanese halfwidth sound marks to fullwidth
	// all forms should be mapped; we need NFKD here
	cTable[ 65392 ] = TCHAR( 12540 );
	cTable[ 65438 ] = TCHAR( 12443 );
	cTable[ 65439 ] = TCHAR( 12444 );
}

TCHAR CLowerCaseTable::operator()(TCHAR cLookup) const
{
	if ( cLookup <= 127 )
	{
		// A..Z -> a..z
		if ( cLookup >= L'A' && cLookup <= L'Z' )
			return (TCHAR)( cLookup + 32 );

		return cLookup;
	}

	return cTable[ cLookup ];
}

CString& CLowerCaseTable::operator()(CString& strSource) const
{
	register const int nLength = strSource.GetLength();
	register LPTSTR str = strSource.GetBuffer();
	for ( int i = 0; i < nLength; ++i, ++str )
	{
		// A...Z -> a...z
		register TCHAR l = *str;
		register TCHAR r = ToLower( l );
		if ( l != r ) *str = r;
	}
	strSource.ReleaseBuffer( nLength );

	return strSource;
}

CString& CLowerCaseTable::Clean(CString& strSource) const
{
	register const int nLength = strSource.GetLength();
	register const int nExt = strSource.ReverseFind( L'.' );
	register LPTSTR str = strSource.GetBuffer();
	for ( int i = 0; i < nLength; ++i, ++str )
	{
		register TCHAR l = *str;
		switch ( l )
		{
		case L'_':
		case L'+':
			*str = L' ';
			break;

		case L'.':
			if ( i < nExt )
				*str = L' ';
			break;

		case L'[':
		case L'{':
			*str = L'(';
			break;

		case L']':
		case L'}':
			*str = L')';
			break;

		case L' ':
		case L'(':
		case L')':
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'-':
			break;

		default:
			register TCHAR r = ToLower( l );
			if ( l != r ) *str = r;
		}
	}
	strSource.ReleaseBuffer( nLength );
	return strSource;
}

CStringA UTF8Encode(__in const CStringW& strInput)
{
	return UTF8Encode( strInput, strInput.GetLength() );
}

CStringA UTF8Encode(__in_bcount(nInput) LPCWSTR psInput, __in int nInput)
{
	CStringA sUTF8;
	int nUTF8 = ::WideCharToMultiByte( CP_UTF8, 0, psInput, nInput,
		sUTF8.GetBuffer( nInput * 4 + 1 ), nInput * 4 + 1, NULL, NULL );

	if ( nUTF8 == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER )
	{
		nUTF8 = ::WideCharToMultiByte( CP_UTF8, 0, psInput, nInput, NULL, 0, NULL, NULL );

		nUTF8 = ::WideCharToMultiByte( CP_UTF8, 0, psInput, nInput, sUTF8.GetBuffer( nUTF8 ), nUTF8, NULL, NULL );
	}
	sUTF8.ReleaseBuffer( nUTF8 );

	return sUTF8;
}

CStringW UTF8Decode(__in const CStringA& strInput)
{
	return UTF8Decode( strInput, strInput.GetLength() );
}

CStringW UTF8Decode(__in_bcount(nInput) LPCSTR psInput, __in int nInput)
{
	CStringW strWide;
	int nWide = 0;

	// Reverted Test:
	nWide = ::MultiByteToWideChar( CP_UTF8, 0, psInput, nInput, strWide.GetBuffer( nInput + 1 ), nInput + 1 );
	if ( nWide == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER )
	{
		nWide = ::MultiByteToWideChar( CP_UTF8, 0, psInput, nInput, NULL, 0 );
		nWide = ::MultiByteToWideChar( CP_UTF8, 0, psInput, nInput, strWide.GetBuffer( nWide ), nWide );
	}
	strWide.ReleaseBuffer( nWide );
	return strWide;

//	// Try UTF-8
//	nWide = ::MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, psInput, nInput, NULL, 0 );
//	if ( nWide > 0 )
//	{
//		nWide = ::MultiByteToWideChar( CP_UTF8, 0, psInput, nInput, strWide.GetBuffer( nWide ), nWide );
//		strWide.ReleaseBuffer( nWide );
//		return strWide;
//	}
//	// Try ANSI
//	nWide = ::MultiByteToWideChar( CP_ACP, MB_ERR_INVALID_CHARS, psInput, nInput, NULL, 0 );
//	if ( nWide > 0 )
//	{
//		nWide = ::MultiByteToWideChar( CP_ACP, 0, psInput, nInput, strWide.GetBuffer( nWide ), nWide );
//		strWide.ReleaseBuffer( nWide );
//		return strWide;
//	}
//	// As-is
//	return CString( psInput, nInput );
}

// Encodes unsafe characters in a string, turning text "hello world" into string "hello%20world", for instance
CString URLEncode(LPCTSTR pszInputT)
{
	// Setup two strings, one with all the hexidecimal digits, the other with all the characters to find and encode
	static LPCTSTR pszHex	= L"0123456789ABCDEF";		// A string with all the hexidecimal digits
	static LPCSTR pszUnsafe	= "<>#%{}|/\\\"^~,[]+?&@=:$";	// A string with all the characters unsafe for a URL

	// The output string starts blank
	CString strOutput;

	// If the input character pointer points to null or points to the null terminator, just return the blank output string
	if ( pszInputT == NULL || *pszInputT == 0 ) return strOutput;

	// Map the wide character string to a new character set
	int nUTF8 = WideCharToMultiByte(
		CP_UTF8,	// Translate using UTF-8, the default encoding for Unicode
		0,			// Must be 0 for UTF-8 encoding
		pszInputT,	// Points to the wide character string to be converted
		-1,			// The string is null terminated
		NULL,		// We just want to find out how long the buffer for the output string needs to be
		0,
		NULL,		// Both must be NULL for UTF-8 encoding
		NULL );

	// If the converted text would take less than 2 bytes, which is 1 character, just return blank
	if ( nUTF8 < 2 ) return strOutput;

	// Make a new array of CHARs which is nUTF8 bytes long
	//LPSTR pszUTF8 = new CHAR[ static_cast< UINT>( nUTF8 ) ];	// Obsolete
	CAutoVectorPtr< CHAR >pszUTF8( new CHAR[ static_cast< UINT>( nUTF8 ) ] );
	if ( ! pszUTF8 )
		return strOutput;	// Out of memory

	// Call WideCharToMultiByte again, this time it has the output buffer and will actually do the conversion
	WideCharToMultiByte( CP_UTF8, 0, pszInputT, -1, pszUTF8, nUTF8, NULL, NULL );

	// Set the null terminator in pszUTF8 to right where you think it should be, and point a new character pointer at it
	pszUTF8[ nUTF8 - 1 ] = 0;
	const CHAR* __restrict pszInput = pszUTF8;

	// Get the character buffer inside the output string, specifying how much larger to make it
	TCHAR* __restrict pszOutput = strOutput.GetBuffer( static_cast< int >( ( nUTF8 - 1 ) * 3 + 1 ) );	// Times 3 in case every character gets encoded

	// Loop for each character of input text
	for ( ; *pszInput; pszInput++ )
	{
		// If the character code is 32 or less, or in the unsafe list
		if ( *pszInput <= 32 || strchr( pszUnsafe, *pszInput ) != NULL )
		{
			// Write a three letter code for it like %20 in the output text
			*pszOutput++ = L'%';
			*pszOutput++ = pszHex[ ( *pszInput >> 4 ) & 0x0F ];
			*pszOutput++ = pszHex[ *pszInput & 0x0F ];
		}
		else	// The character doesn't need to be encoded
		{
			// Just copy it across
			*pszOutput++ = (TCHAR)*pszInput;
		}
	}

	// Null terminate the output text, and then close our direct manipulation of the string
	*pszOutput = 0;
	strOutput.ReleaseBuffer();			// Closes the string so Windows can start managing its memory for us again

	// Return the URL-encoded, %20-filled text
	return strOutput;
}

// Decodes unsafe characters in a string, turning text "hello%20world" into string "hello world", for instance
CString URLDecode(LPCTSTR pszInput)
{
	// Check each character of input text
	LPCTSTR pszLoop( pszInput );
	for ( ; *pszLoop; pszLoop++ )
	{
		// This URI is not properly encoded, and has unicode characters in it. URL-decode only
		if ( *pszLoop > 255 )
			return URLDecodeUnicode( pszInput );
	}

	// This is a correctly formatted URI, which must be url-decoded, then UTF-8 decoded.
	return URLDecodeANSI( pszInput );
}

CString URLDecode(__in const CStringA& strInput)
{
	return URLDecode( (LPCTSTR)UTF8Decode( strInput ) );
}

CString URLDecode(__in_bcount(nInput) LPCSTR psInput, __in int nInput)
{
	return URLDecode( (LPCTSTR)UTF8Decode( psInput, nInput ) );
}

// Decodes a properly formatted URI, then UTF-8 decodes it
CString URLDecodeANSI(const TCHAR* __restrict pszInput)
{
	TCHAR szHex[3] = { 0, 0, 0 };		// A 3 character long array filled with 3 null terminators
	CStringA sOutput;					// The output string, which starts out blank
	int nHex;							// The hex code of the character we found

	int nLength = (int)_tcslen( pszInput );
	CHAR* __restrict pszOutput = sOutput.GetBuffer( nLength + 1 );

	// Loop for each character of input text
	for ( ; *pszInput; pszInput++ )
	{
		if ( *pszInput == '%' )			// Encountered the start of something like %20
		{
			// Copy characters like "20" into szHex, making sure neither are null
			if ( ( szHex[0] = pszInput[1] ) != 0 &&
				 ( szHex[1] = pszInput[2] ) != 0 &&
				 _stscanf_s( szHex, L"%x", &nHex ) == 1 &&
				 nHex > 0 )
			{
				*pszOutput++ = (CHAR)nHex;
				pszInput += 2;	// "20"
			}
			else
			{
				*pszOutput++ = '%';
			}
		}
		else if ( *pszInput == '+' )	// Encountered shorthand for a space
		{
			// Add a space to the output text, and move the pointer forward
			*pszOutput++ = ' ';
		}
		else if ( *pszInput == '&' )
		{
			if ( pszInput[ 1 ] == '#' )
			{
				if ( _stscanf_s( pszInput + 2, L"%i;", &nHex ) == 1 && nHex > 0 )
				{
					*pszOutput++ = (CHAR)nHex;
					while ( *pszInput && *pszInput != ';' )
						++pszInput;
				}
				else
				{
					*pszOutput++ = '&';
					*pszOutput++ = '#';
				}
			}
			else if ( _tcsnicmp( pszInput + 1, L"quot;", 5 ) == 0 )
			{
				*pszOutput++ = '\"';
				pszInput += 5;
			}
			else if ( _tcsnicmp( pszInput + 1, L"apos;", 5 ) == 0 )
			{
				*pszOutput++ = '\'';
				pszInput += 5;
			}
			else if ( _tcsnicmp( pszInput + 1, L"lt;", 3 ) == 0 )
			{
				*pszOutput++ = '<';
				pszInput += 3;
			}
			else if ( _tcsnicmp( pszInput + 1, L"gt;", 3 ) == 0 )
			{
				*pszOutput++ = '>';
				pszInput += 3;
			}
			else if ( _tcsnicmp( pszInput + 1, L"nbsp;", 5 ) == 0 )
			{
				*pszOutput++ = ' ';
				pszInput += 5;
			}
			else if ( _tcsnicmp( pszInput + 1, L"amp;", 4 ) == 0 )
			{
				*pszOutput++ = '&';
				pszInput += 4;
			}
			else
			{
				*pszOutput++ = '&';
			}
		}
		else	// Normal character
		{
			// Copy it across
			*pszOutput++ = (CHAR)*pszInput;
		}
	}

	*pszOutput = 0;		// Null terminator

	sOutput.ReleaseBuffer();
	return UTF8Decode( sOutput );
}

// Decodes encoded characters in a unicode string
CString URLDecodeUnicode(const TCHAR* __restrict pszInput)
{
	TCHAR szHex[3] = { 0, 0, 0 };		// A 3 character long array filled with 3 null terminators
	CString strOutput;					// The output string, which starts out blank
	int nHex;							// The hex code of the character we found

	int nLength = (int)_tcslen( pszInput );
	TCHAR* __restrict pszOutput = strOutput.GetBuffer( nLength + 1 );

	// Loop for each character of input text
	for ( ; *pszInput; ++pszInput )
	{
		if ( *pszInput == '%' )			// Encounterd the start of something like %20
		{
			if ( ( szHex[0] = pszInput[1] ) != 0 &&
				( szHex[1] = pszInput[2] ) != 0 &&
				_stscanf_s( szHex, L"%x", &nHex ) == 1 &&
				nHex > 0 )
			{
				*pszOutput++ = (TCHAR)nHex;
				pszInput += 2;	// Skip "20"
			}
			else
			{
				*pszOutput++ = '%';
			}
		}
		else if ( *pszInput == '+' )	// Encountered shorthand for a space
		{
			// Add a space to the output text, and move the pointer forward
			*pszOutput++ = ' ';
		}
		else if ( *pszInput == '&' )
		{
			if ( pszInput[ 1 ] == '#' )
			{
				if ( _stscanf_s( pszInput + 2, L"%i;", &nHex ) == 1 && nHex > 0 )
				{
					*pszOutput++ = (CHAR)nHex;
					while ( *pszInput && *pszInput != ';' )
						++pszInput;
				}
				else
				{
					*pszOutput++ = '&';
					*pszOutput++ = '#';
				}
			}
			else if ( _tcsnicmp( pszInput + 1, L"quot;", 5 ) == 0 )
			{
				*pszOutput++ = '\"';
				pszInput += 5;
			}
			else if ( _tcsnicmp( pszInput + 1, L"apos;", 5 ) == 0 )
			{
				*pszOutput++ = '\'';
				pszInput += 5;
			}
			else if ( _tcsnicmp( pszInput + 1, L"lt;", 3 ) == 0 )
			{
				*pszOutput++ = '<';
				pszInput += 3;
			}
			else if ( _tcsnicmp( pszInput + 1, L"gt;", 3 ) == 0 )
			{
				*pszOutput++ = '>';
				pszInput += 3;
			}
			else if ( _tcsnicmp( pszInput + 1, L"nbsp;", 5 ) == 0 )
			{
				*pszOutput++ = ' ';
				pszInput += 5;
			}
			else if ( _tcsnicmp( pszInput + 1, L"amp;", 4 ) == 0 )
			{
				*pszOutput++ = '&';
				pszInput += 4;
			}
			else
			{
				*pszOutput++ = '&';
			}
		}
		else	// Normal character
		{
			// Copy it across
			*pszOutput++ = (TCHAR)*pszInput;
		}
	}

	*pszOutput = 0;					// Null terminator

	strOutput.ReleaseBuffer();		// Release direct access to the buffer of CString object

	return strOutput;
}

LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszSubString)
{
	// Return null if string or substring is empty
	if ( !*pszString || !*pszSubString )
		return NULL;

	// Return if string is too small to hold the substring
	size_t nString( _tcslen( pszString ) );
	size_t nSubString( _tcslen( pszSubString ) );
	if ( nString < nSubString )
		return NULL;

	// Get the first character from the substring and lowercase it
	const TCHAR cFirstPatternChar = ToLower( *pszSubString );

	// Loop over the part of the string that the substring could fit into
	LPCTSTR pszCutOff = pszString + nString - nSubString;
	while ( pszString <= pszCutOff )
	{
		// Search for the start of the substring
		while ( pszString <= pszCutOff && ToLower( *pszString ) != cFirstPatternChar )
		{
			++pszString;
		}

		// Exit loop if no match found
		if ( pszString > pszCutOff )
			break;

		// Check the rest of the substring
		size_t nChar( 1 );
		while ( pszSubString[nChar] && ToLower( pszString[nChar] ) == ToLower( pszSubString[nChar] ) )
		{
			++nChar;
		}

		// If the substring matched return a pointer to the start of the match
		if ( ! pszSubString[nChar] )
			return pszString;

		// Move on to the next character and continue search
		++pszString;
	}

	// No match found, return a null pointer
	return NULL;
}

LPCTSTR _tcsnistr(LPCTSTR pszString, LPCTSTR pszSubString, size_t nlen)
{
	if ( ! *pszString || ! *pszSubString || ! nlen ) return NULL;

	const TCHAR cFirstPatternChar = ToLower( *pszSubString );

	for ( ; ; ++pszString )
	{
		while ( *pszString && ToLower( *pszSubString )
			!= cFirstPatternChar ) ++pszString;

		if ( !*pszString )
			return NULL;

		DWORD i = 0;
		while ( ++i < nlen )
		{
			if ( const TCHAR cStringChar = ToLower( pszString[ i ] ) )
			{
				if ( cStringChar != ToLower( pszSubString[ i ] ) )
					break;
			}
			else
			{
				return NULL;
			}
		}

		if ( i == nlen )
			return pszString;
	}
}

bool atoin(__in_bcount(nLen) const char* pszString, __in size_t nLen, __int64& nNum)
{
	bool bNeg = false;
	nNum = 0;
	for ( size_t i = 0; i < nLen; ++i )
	{
		if ( pszString[ i ] >= '0' && pszString[ i ] <= '9' )
			nNum = nNum * 10 + ( pszString[ i ] - '0' );
		else if ( i == 0 && nLen > 1 && pszString[ i ] == '-' )
			bNeg = true;
		else
			return false;
	}
	if ( bNeg )
		nNum = - nNum;
	return true;
}

#ifdef __AFXCOLL_H__
void Split(const CString& strSource, TCHAR cDelimiter, CStringArray& pAddIt, BOOL bAddFirstEmpty)
{
	for ( LPCTSTR start = strSource; *start; start++ )
	{
		LPCTSTR c = _tcschr( start, cDelimiter );
		const int len = c ? (int) ( c - start ) : (int) _tcslen( start );
		if ( len > 0 )
			pAddIt.Add( CString( start, len ) );
		else if ( bAddFirstEmpty && ( start == strSource ) )
			pAddIt.Add( CString() );
		if ( ! c )
			break;
		start = c;
	}
}
#endif	// __AFXCOLL_H__

int CountOf(const CString& strInput, LPCTSTR pszFind, int nSkip /*0*/)
{
	const int nLen = (int)_tcslen( pszFind );
	int nLimit = strInput.GetLength() - nLen;
	if (nLimit - nSkip < 0) return 0;
	int nCount = 0;
	for (int i = nSkip; i <= nLimit; i++)
	{
		if ( strInput[i] != *pszFind )
			continue;
		if (nLen == 1)
		{
			nCount++;
			continue;
		}
		bool bFound = true;
		for (int j = 1; j < nLen; j++)
		{
			pszFind++;
			if ( strInput[i + j] == (TCHAR)pszFind[j] )
				continue;
			bFound = false;
			break;
		}
		if (!bFound)
			continue;

		nCount++;
		i += nLen - 1;
	}
	return nCount;
}

BOOL StartsWith(const CString& strInput, LPCTSTR pszText, int nLen /*0*/)
{
	// Fast case-insensitive first char
	if ( strInput[0] != *pszText && //( *pszText < L'A' ||		// Note extra check causes runtime errors
	   ( strInput[0] & ~0x20 ) != *pszText && strInput[0] != ( *pszText & ~0x20 ) )
		return FALSE;

	if ( ! nLen )
		nLen = (int)_tcslen( pszText );

//	return strInput.GetLength() >= nLen &&
//		_tcsnicmp( (LPCTSTR)strInput, pszText, nLen ) == 0;

	if ( nLen > strInput.GetLength() )
		return FALSE;

	pszText++;
	for ( int n = 1; n < nLen; n++ && pszText++ )
	{
		if ( strInput[n] != *pszText && ToLower( strInput[n] ) != ToLower( *pszText ) )
			return FALSE;
	}

	return TRUE;
}

BOOL EndsWith(const CString& strInput, LPCTSTR pszText, int nLen /*0*/)
{
	if ( ! nLen )
		nLen = (int)_tcslen( pszText );

	if ( strInput.GetLength() < nLen )
		return FALSE;

	CString strTrim = strInput.Right( nLen );	// Not inline below
	LPCTSTR pszTest = (LPCTSTR)strTrim;

	// Fast case-insensitive first char
	//if ( *pszTest != *pszText && //( *pszText < L'A' ||		// Note extra check causes runtime errors
	//	( ( *pszTest & ~0x20 ) != *pszText && *pszTest != ( *pszText & ~0x20 ) ) )
	//	return FALSE;
	//
	//return _tcsnicmp( pszTest, pszText, nLen ) == 0;

	for ( int n = 0; n < nLen; n++ && pszTest++ && pszText++ )
	{
		if ( *pszTest != *pszText && ToLower( *pszTest ) != ToLower( *pszText ) )
			return FALSE;
	}

	return TRUE;
}

BOOL IsText(const CString& strInput, LPCTSTR pszText, int nLen /*0*/)
{
	// Fast case-insensitive first char
	if ( strInput[0] != *pszText && //( *pszText < L'A' ||	// Note extra check causes runtime errors
	   ( strInput[0] & ~0x20 ) != *pszText && strInput[0] != ( *pszText & ~0x20 ) )
		return FALSE;

	if ( ! nLen )
		nLen = (int)_tcslen( pszText );

	if ( strInput.GetLength() != nLen )
		return FALSE;

	//return _tcsnicmp( (LPCTSTR)strInput, pszText, nLen ) == 0;

	pszText++;
	for ( int n = 1; n < nLen; n++ && pszText++ )
	{
		if ( strInput[ n ] != *pszText && ToLower( strInput[ n ] ) != ToLower( *pszText ) )
			return FALSE;
	}

	return TRUE;
}

#ifdef __AFX_H__
CString LoadFile(LPCTSTR pszPath)
{
	CString strXML;

	CFile pFile;
	if ( ! pFile.Open( pszPath, CFile::modeRead ) )
		return strXML;	// File open error

	DWORD nByte = (DWORD)pFile.GetLength();
	if ( nByte > 4096 * 1024 )
		return strXML;	// File too big (>4MB)

	//BYTE* pBuf = new BYTE[ nByte ];	// Obsolete
	CAutoVectorPtr< BYTE >pBuf( new BYTE[ nByte ] );
	if ( ! pBuf )
		return strXML;	// Out of memory

	try
	{
		pFile.Read( pBuf, nByte );
	}
	catch ( CException* pException )
	{
		// File read error
		pFile.Abort();
		pException->Delete();
		//delete [] pBuf;	// Obsolete
		return strXML;
	}

	pFile.Close();

	BYTE* pByte = pBuf;
	if ( nByte >= 2 &&
		( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) ||
		  ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
	{
		nByte = nByte / 2 - 1;
		if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
		{
			pByte += 2;
			for ( DWORD nSwap = 0; nSwap < nByte; nSwap ++ )
			{
				register BYTE nTemp = pByte[ ( nSwap << 1 ) + 0 ];
				pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
				pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
			}
		}
		else
		{
			pByte += 2;
		}

		CopyMemory( strXML.GetBuffer( nByte ), pByte, nByte * sizeof( TCHAR ) );
		strXML.ReleaseBuffer( nByte );
	}
	else
	{
		if ( nByte >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF )
		{
			pByte += 3;
			nByte -= 3;
		}

		strXML = UTF8Decode( (LPCSTR)pByte, nByte );
	}
	//delete [] pBuf;	// Obsolete

	return strXML;
}
#endif	// __AFX_H__

BOOL ReplaceNoCase(CString& sInStr, LPCTSTR pszOldStr, LPCTSTR pszNewStr)
{
	BOOL bModified = FALSE;
	DWORD nInLength = sInStr.GetLength();
	LPCTSTR pszInStr = sInStr;

	CString result;
	result.Preallocate( nInLength );

	TCHAR nOldChar = pszOldStr[ 0 ];
	for ( DWORD nPos = 0; nPos < nInLength; )
	{
		TCHAR nChar = pszInStr[ nPos ];
		if ( ToLower( nChar ) == nOldChar )
		{
			DWORD nOffset = 0;
			while ( TCHAR nChar2 = pszOldStr[ ++nOffset ] )
			{
				if ( nChar2 != ToLower( pszInStr[ nPos + nOffset ] ) )
					goto fail;
			}
			nPos += nOffset;
			result.Append( pszNewStr );
			bModified = TRUE;
		}
		else
		{
fail:
			result.AppendChar( nChar );
			++nPos;
		}
	}

	sInStr = result;

	return bModified;
}

CString MakeKeywords(const CString& strPhrase, bool bExpression)
{
	if ( strPhrase.IsEmpty() )
		return CString();

	CString str( L" " );
	LPCTSTR pszPtr = strPhrase;
	ScriptType boundary[ 2 ] = { sNone, sNone };
	int nPos = 0;
	int nPrevWord = 0;
	BOOL bNegative = FALSE;

	for ( ; *pszPtr; nPos++, pszPtr++ )
	{
		// boundary[ 0 ] -- previous character;
		// boundary[ 1 ] -- current character;
		boundary[ 0 ] = boundary[ 1 ];
		boundary[ 1 ] = sNone;

		if ( IsKanji( *pszPtr ) )
			boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sKanji );
		if ( IsKatakana( *pszPtr ) )
			boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sKatakana );
		if ( IsHiragana( *pszPtr ) )
			boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sHiragana );
		if ( IsCharacter( *pszPtr ) )
			boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sRegular );
		// For now, disable Numeric Detection in order not to split strings like "Envy2" to "Envy 2"
		//if ( _istdigit( *pszPtr ) )
		//	boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sNumeric );

		if ( ( boundary[ 1 ] & ( sHiragana | sKatakana ) ) == ( sHiragana | sKatakana ) &&
			 ( boundary[ 0 ] & ( sHiragana | sKatakana ) ) )
		{
			boundary[ 1 ] = boundary[ 0 ];
		}

		bool bCharacter = ( boundary[ 1 ] & sRegular ) ||
			bExpression && ( *pszPtr == L'-' || *pszPtr == L'"' );

		if ( !( boundary[ 0 ] & sRegular ) && *pszPtr == L'-' )
			bNegative = TRUE;
		else if ( *pszPtr == L' ' )
			bNegative = FALSE;

		int nDistance = ! bCharacter ? 1 : 0;

		if ( ! bCharacter || boundary[ 0 ] != boundary[ 1 ] && nPos )
		{
			if ( nPos > nPrevWord )
			{
				int len = str.GetLength();
				TCHAR last1 = str.GetAt( len - 1 );
				TCHAR last2 = ( len > 1 ) ? str.GetAt( len - 2 ) : L'\0';
				TCHAR last3 = ( len > 2 ) ? str.GetAt( len - 3 ) : L'\0';
				if ( boundary[ 0 ] &&
					 ( last2 == L' ' || last2 == L'-' || last2 == L'"' ) &&
					 ! _istdigit( ( nPos < 3 ) ? last1 : last3 ) )
				{
					// Join two phrases if the previous was a single characters word.
					// idea of joining single characters breaks GDF compatibility completely,
					// but because Shareaza 2.2 and above (hence Envy) are not really following GDF
					// about word length limit for ASIAN chars, merging is necessary to be done.
				}
				else if ( last1 != L' ' && bCharacter )
				{
					if ( ( last1 == L'-' || last1 == L'"' || *pszPtr == L'"' ) &&
						 ( ! bNegative || ! ( boundary[ 0 ] & ( sHiragana | sKatakana | sKanji ) ) ) )
						str += L' ';
				}
				if ( strPhrase.GetAt( nPos - 1 ) == L'-' && nPos > 1 )
				{
					if ( *pszPtr != L' ' && strPhrase.GetAt( nPos - 2 ) != L' ' )
					{
						nPrevWord += nDistance + 1;
						continue;
					}

					str += strPhrase.Mid( nPrevWord, nPos - nDistance - nPrevWord );
				}
				else
				{
					str += strPhrase.Mid( nPrevWord, nPos - nPrevWord );
					if ( boundary[ 1 ] == sNone && ! bCharacter || *pszPtr == L' ' || ! bExpression ||
						 ( ( boundary[ 0 ] & ( sHiragana | sKatakana | sKanji ) ) && ! bNegative ) )
						str += L' ';
					else if ( ! bNegative && ( ( boundary[ 0 ] & ( sHiragana | sKatakana | sKanji ) ) ||
						 ( boundary[ 0 ] & ( sHiragana | sKatakana | sKanji ) ) !=
						 ( boundary[ 1 ] & ( sHiragana | sKatakana | sKanji ) ) ) )
						str += L' ';
				}
			}
			nPrevWord = nPos + nDistance;
		}
	}

	int len = str.GetLength();
	TCHAR last1 = str.GetAt( len - 1 );
	TCHAR last2 = ( len > 1 ) ? str.GetAt( len - 2 ) : L'\0';
	if ( boundary[ 0 ] && boundary[ 1 ] &&
		( last2 == L' ' || last2 == L'-' || last2 == L'"' ) )
	{
		// Join two phrases if the previous was a single characters word.
		// idea of joining single characters breaks GDF compatibility completely,
		// but because Shareaza 2.2 and above (hence Envy) are not really following GDF
		// about word length limit for ASIAN chars, merging is necessary to be done.
	}
	else if ( boundary[ 1 ] && last1 != L' ' )
	{
		if ( ( last1 == L'-' || last1 == L'"' ) && ! bNegative )
			str += L' ';
	}
	str += strPhrase.Mid( nPrevWord, nPos - nPrevWord );

	return str.TrimLeft().TrimRight( L" -" );
}

void BuildWordTable(LPCTSTR pszWord, WordTable& oWords, WordTable& oNegWords)
{
	// Clear word tables.
	oWords.clear();
	oNegWords.clear();

	LPCTSTR pszPtr	= pszWord;
	BOOL bQuote		= FALSE;
	BOOL bNegate	= FALSE;
	BOOL bSpace		= TRUE;

	for ( ; *pszPtr; pszPtr++ )
	{
		if ( IsCharacter( *pszPtr ) )
		{
			bSpace = FALSE;
		}
		else
		{
			if ( pszWord < pszPtr )
			{
				if ( bNegate )
				{
					oNegWords.insert( std::make_pair( pszWord, pszPtr - pszWord ) );
				}
				else
				{
					bool bWord = false, bDigit = false, bMix = false;
					IsType( pszWord, 0, pszPtr - pszWord, bWord, bDigit, bMix );
					if ( ( bWord || bMix ) || ( bDigit && pszPtr - pszWord > 3 ) )
						oWords.insert( std::make_pair( pszWord, pszPtr - pszWord ) );
				}
			}

			pszWord = pszPtr + 1;

			if ( *pszPtr == '\"' )
			{
				bQuote = ! bQuote;
				bSpace = TRUE;
			}
			else if ( *pszPtr == '-' && pszPtr[1] != ' ' && bSpace && ! bQuote )
			{
				bNegate = TRUE;
				bSpace = FALSE;
			}
			else
			{
				bSpace = ( *pszPtr == ' ' );
			}

			if ( bNegate && ! bQuote && *pszPtr != '-' )
				bNegate = FALSE;
		}
	}

	if ( pszWord < pszPtr )
	{
		if ( bNegate )
		{
			oNegWords.insert( std::make_pair( pszWord, pszPtr - pszWord ) );
		}
		else
		{
			bool bWord = false, bDigit = false, bMix = false;
			IsType( pszWord, 0, pszPtr - pszWord, bWord, bDigit, bMix );
			if ( bWord || bMix || ( bDigit && pszPtr - pszWord > 3 ) )
				oWords.insert( std::make_pair( pszWord, pszPtr - pszWord ) );
		}
	}
}

LPCTSTR SkipSlashes(LPCTSTR pszURL, int nAdd)
{
	for ( ; nAdd && *pszURL; --nAdd, ++pszURL );
	while ( *pszURL == L'/' ) pszURL++;
	return pszURL;
}

void SafeString(CString& strInput)
{
	strInput.Trim();

	for ( int nIndex = strInput.GetLength() - 1; nIndex >= 0; nIndex-- )
	{
		if ( strInput.GetAt( nIndex ) < 32 )	// TCHAR
			strInput.SetAt( nIndex, '_' );
	}
}

CString Escape(const CString& strValue)
{
	bool bChanged = false;

	CString strXML;
	LPTSTR pszXML = strXML.GetBuffer( strValue.GetLength() * 8 + 1 );

	for ( LPCTSTR pszValue = strValue; *pszValue; ++pszValue )
	{
		switch ( *pszValue )
		{
		case L'\"':
			_tcscpy( pszXML, L"&quot;" );
			 pszXML += 6;
			bChanged = true;
			break;
		case L'\'':
			_tcscpy( pszXML, L"&apos;" );
			 pszXML += 6;
			bChanged = true;
			break;
		case L'<':
			_tcscpy( pszXML, L"&lt;" );
			pszXML += 4;
			bChanged = true;
			break;
		case L'>':
			_tcscpy( pszXML, L"&gt;" );
			pszXML += 4;
			bChanged = true;
			break;
		case L'&':
			_tcscpy( pszXML, L"&amp;" );
			pszXML += 5;
			bChanged = true;
			break;
		default:
			if ( *pszValue < 32 || *pszValue > 127 )
			{
				pszXML += _stprintf_s( pszXML, 9, L"&#%i;", *pszValue );
				bChanged = true;
				break;
			}

			*pszXML++ = *pszValue;
			break;
		}
	}

	*pszXML = 0;

	strXML.ReleaseBuffer();

	return bChanged ? strXML : strValue;
}

CString Unescape(const TCHAR* __restrict pszXML, int nLength)
{
	CString strValue;

	if ( ! nLength || ! *pszXML )
		return strValue;

	if ( nLength < 0 )
		nLength = (int)_tcslen( pszXML );

	TCHAR* __restrict pszValue = strValue.GetBuffer( nLength + 4 );
	TCHAR* __restrict pszOut = pszValue;
	const TCHAR* __restrict pszNull = pszXML + nLength;

	while ( pszXML < pszNull && *pszXML )
	{
		if ( IsSpace( *pszXML ) && *pszXML != 0xa0 )	// Keep non-breaking space
		{
			if ( pszValue != pszOut ) *pszOut++ = ' ';
			pszXML++;
			while ( *pszXML && IsSpace( *pszXML ) && *pszXML != 0xa0 ) pszXML++;
			if ( pszXML >= pszNull || ! *pszXML ) break;
		}

		if ( *pszXML == '&' )
		{
			pszXML++;
			if ( pszXML >= pszNull || ! *pszXML ) break;

			// http://en.wikipedia.org/wiki/List_of_XML_and_HTML_character_entity_references

			if ( *pszXML == '#' )
			{
				pszXML++;
				if ( pszXML >= pszNull || ! *pszXML || ! _istdigit( *pszXML ) ) break;

				int nChar;
				if ( _stscanf_s( pszXML, L"%i;", &nChar ) == 1 )
				{
					*pszOut++ = (TCHAR)nChar;
					while ( *pszXML && *pszXML != ';' ) pszXML++;
					if ( pszXML >= pszNull || ! *pszXML ) break;
					pszXML++;
				}
			}
			else if ( _tcsnicmp( pszXML, L"quot;", 5 ) == 0 )	// Most common
			{
				*pszOut++ = '\"';
				pszXML += 5;
			}
			else if ( _tcsnicmp( pszXML, L"apos;", 5 ) == 0 )
			{
				*pszOut++ = '\'';
				pszXML += 5;
			}
			else if ( _tcsnicmp( pszXML, L"lt;", 3 ) == 0 )
			{
				*pszOut++ = '<';
				pszXML += 3;
			}
			else if ( _tcsnicmp( pszXML, L"gt;", 3 ) == 0 )
			{
				*pszOut++ = '>';
				pszXML += 3;
			}
			else if ( _tcsnicmp( pszXML, L"nbsp;", 5 ) == 0 )	// Not XML spec, but common from HTML
			{
				*pszOut++ = ' ';
				pszXML += 5;
			}
			else if ( _tcsnicmp( pszXML, L"amp;", 4 ) == 0 )
			{
				*pszOut++ = '&';
				pszXML += 4;
			}
			else
			{
				*pszOut++ = '&';
			}
		}
		else
		{
			*pszOut++ = *pszXML++;
		}
	}

	strValue.ReleaseBuffer( (int)( pszOut - pszValue ) );

	return strValue;
}

CString Str(QWORD num, BOOL commas /*False*/)
{
//#if defined(_MSC_VER) && (_MSC_VER >= 1700)
//	return CString( std::to_wstring( num ).c_str() );
//#endif
	CString str;
	//str.Format( L"%llu", num );
	_ui64tow_s( num, str.GetBufferSetLength( 20 ), 20, 10 );
	str.ReleaseBuffer();
	if ( ! commas || num < 1000 )
		return str;
	for ( int npos = str.GetLength() - 3; npos > 0; npos -= 3 )
	{
		str.Insert( npos, L"," );
	}
	return str;
}

CString Str(DWORD num, BOOL commas /*False*/)
{
//#if defined(_MSC_VER) && (_MSC_VER >= 1700)
//	return CString( std::to_wstring( num ).c_str() );
//#endif
	CString str;
	//str.Format( L"%lu", num );
	_ultow_s( num, str.GetBufferSetLength( 12 ), 12, 10 );
	str.ReleaseBuffer();
	if ( ! commas || num < 1000 )
		return str;
	for ( int npos = str.GetLength() - 3; npos > 0; npos -= 3 )
	{
		str.Insert( npos, L"," );
	}
	return str;
}

CString Str(WORD num, BOOL commas /*False*/)
{
	CString str;
	str.Format( L"%hu", num );
	if ( ! commas || str.GetLength() < 4 )
		return str;
	str.ReleaseBuffer();
	for ( int npos = str.GetLength() - 3; npos > 0; npos -= 3 )
	{
		str.Insert( npos, L"," );
	}
	return str;
}

CString Str(BYTE num)
{
	CString str;
	str.Format( L"%hhu", num );
	return str;
}

CString Str(UINT num, BOOL commas /*False*/)
{
	CString str;
	str.Format( L"%u", num );
	if ( ! commas || num < 1000 )
		return str;
	str.ReleaseBuffer();
	for ( int npos = str.GetLength() - 3; npos > 0; npos -= 3 )
	{
		str.Insert( npos, L"," );
	}
	return str;
}

CString Str(int num, BOOL commas /*False*/)
{
	CString str;
	//str.Format( L"%i", num );
	_itow_s( num, str.GetBufferSetLength( 12 ), 12, 10 );
	str.ReleaseBuffer();
	if ( ! commas || num < 1000 )
		return str;
	for ( int npos = str.GetLength() - 3; npos > 0; npos -= 3 )
	{
		str.Insert( npos, L"," );
	}
	return str;
}

CString Str(double num)
{
	CString str;
	str.Format( L"%f", num );
	return str;
}

#ifdef __ATLTIME_H__
CString Str(CTime time)
{
	CString strFull = time.Format( L"%Y.%m.%d  %#I:%M%p" );
	const int nLen = strFull.GetLength() - 2;
	return strFull[ nLen ] == L'P' ?
		strFull.Left( nLen ) + L'p' :
		strFull[ nLen ] == L'A' ?
		strFull.Left( nLen ) + L'a' :
		strFull.Left( nLen + 1 );
}
#endif

BOOL IsValidExtension(LPCTSTR pszName)
{
//	const int nDot = sName.ReverseFind( L'.' );
//	return ( nDot > 0 && nDot > sName.GetLength() - 10 && sName.FindOneOf( L" -)]" ) < nDot );

	BOOL bNum = TRUE;
	const UINT nLen = (UINT)_tcslen( pszName );
	for ( UINT nTest = 0; nTest < 12 && nTest <= nLen; nTest++ )
	{
		TCHAR cChar = *(pszName + ( nLen - nTest ));
		if ( bNum && cChar == L'\0' ) continue;
		if ( cChar == L'.' ) return ! bNum;
		if ( cChar < L'0' || cChar == L']' ) break;
		bNum = bNum && cChar <= L'9';		//_istdigit( cChar );
	}

	return FALSE;
}

//BOOL IsValidIP(const CString& sInput)
//{
//	const int nLength = sInput.GetLength();
//	if ( nLength > 21 || nLength < 8 )
//		return FALSE;
//
//	//int nIP[5] = { 0 };
//	//if ( _stscanf( sInput, L"%i.%i.%i.%i:%i", &nIP[0], &nIP[1], &nIP[2], &nIP[3], &nIP[4] ) == 5 ||
//	//	   _stscanf( sInput, L"%i.%i.%i.%i", &nIP[0], &nIP[1], &nIP[2], &nIP[3] ) == 4 )
//	//	return nIP[0] < 256 && nIP[1] < 256 && nIP[2] < 256 && nIP[3] < 256 && nIP[4] < 65000;
//	//return FALSE;
//
//	CString strIP;
//	for ( int i = 0, d = 0; i < nLength; i++ )
//	{
//		TCHAR Ch = sInput.GetAt( i );
//		if ( _istdigit( Ch ) )
//		{
//			strIP.AppendChar( Ch );
//			if ( d == 4 )
//			{
//				if ( strIP.GetLength() > 5 || strIP.GetLength() == 5 && _tstoi( strIP ) > 65000 )
//					return FALSE;
//				continue;
//			}
//			if ( strIP.GetLength() > 3 || strIP.GetLength() == 3 && _tstoi( strIP ) > 255 )
//				return FALSE;
//			continue;
//		}
//		if ( Ch == L'.' )
//		{
//			if ( d++ > 3 || strIP.IsEmpty() )
//				return FALSE;
//			strIP.Empty();
//			continue;
//		}
//		if ( Ch == L':' )
//		{
//			if ( d != 3 || strIP.IsEmpty() )
//				return FALSE;
//			d = 4;
//			strIP.Empty();
//			continue;
//		}
//
//		return FALSE;
//	}
//
//	return TRUE;
//}

DWORD IPStringToDWORD(LPCTSTR pszIP, BOOL bReverse)
{
	DWORD nIP = 0;
	UINT nCurrent = 0;
	UINT nSet = 0;

	for ( UINT i = 16; i; i--, pszIP++ )
	{
		if ( *pszIP == '.' || *pszIP == 0 || *pszIP == ':' || i == 1 )
		{
			nIP <<= 8;
			nIP += nCurrent;
			nCurrent = 0;
			nSet++;

			if ( *pszIP == '.' )
				continue;

			break;
		}

		if ( *pszIP < '0' || *pszIP > '9' )
			return 0;

		nCurrent *= 10;
		nCurrent += (*pszIP - '0');
		if ( nCurrent > 255 )
			return 0;
	}

	if ( nSet != 4 )
		return 0;

	if ( bReverse )
		nIP = htonl( nIP );		// Reverse byte order ( Little-Endian/Big-Endian host/network )

	return nIP;
}

#ifdef _WINSOCKAPI_
CString HostToString(const SOCKADDR_IN* pHost)
{
	CString strHost;
#ifndef XPSUPPORT
	WCHAR ipbuf[ INET_ADDRSTRLEN ];
	IN_ADDR addr = pHost->sin_addr;
	InetNtop( AF_INET, &addr, ipbuf, sizeof(ipbuf) );
	strHost.Format( L"%s:%hu", (LPCTSTR)ipbuf, ntohs( pHost->sin_port ) );
#else // XP (inet_ntoa deprecated Vista+)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
	strHost.Format( L"%s:%hu", (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), ntohs( pHost->sin_port ) );
#endif
	return strHost;
}
#endif	// _WINSOCKAPI_

LPCTSTR SafePath(const CString& sPath)
{
	return ( sPath.GetLength() < MAX_PATH - 4 || sPath[2] == L'?' ) ?
		(LPCTSTR)sPath : CString( L"\\\\?\\" ) + sPath;
}

BOOL MakeSafePath(CString& sPath)
{
	if ( sPath.GetLength() < MAX_PATH - 4 )
		return FALSE;

//	ASSERT( StartsWith( sPath, _P( L"\\\\?\\" ) ) );
	if ( sPath[2] != L'?' )
		sPath = CString( L"\\\\?\\" ) + sPath;
	return TRUE;
}
