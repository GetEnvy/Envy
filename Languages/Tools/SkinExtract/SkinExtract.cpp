//
// SkinExtract.cpp
//

#include "StdAfx.h"

#define _VERSION_ _T("1.0")

enum States
{
	stFile,
	stGuidelines, stGuidelinesContent, stGuidelinesDialog, stGuidelinesDialogContent,
	stStringTable, stContent, stString,
	stError
};

typedef CAtlMap< CStringA, UINT > CSUMap;
class CSSPair
{
public:
	CSSPair() {}
	CSSPair(const CStringA& f, const CStringA& s) : sFirst( f ), sSecond( s ) {}
	CSSPair(const CSSPair& p) : sFirst( p.sFirst ), sSecond( p.sSecond ) {}
	CSSPair& operator= (const CSSPair& p) { sFirst = p.sFirst; sSecond = p.sSecond; return *this; }
	CStringA sFirst;
	CStringA sSecond;
};
typedef CAtlMap< UINT, CSSPair > CUSMap;

CSUMap g_oGuidelines;	// GUIDELINES resources
CSUMap g_oDialogs;		// DIALOG or DIALOGEX resources
CSUMap g_oIcons;		// ICON resources
CSUMap g_oBitmaps;		// JPEG, PNG or BITMAP resources
CSUMap g_oHtmls;		// HTML or GZIP resources
CUSMap g_oStrings;		// STRINGTABLE resources
CSUMap g_oIDs;

BOOL ProcessString(CStringA sID, CStringA sString)
{
	const int len = sString.GetLength();
	if ( sString.GetAt( 0 ) != '\"' || sString.GetAt( len - 1 ) != '\"' )
	{
		_tprintf( _T("Error: Invalid string format \"%hs\"\n"), sString );
		return FALSE;
	}
	sString = sString.Mid( 1, len - 2 );

	UINT nID = 0;
	if ( ! g_oIDs.Lookup( sID, nID ) )
	{
		_tprintf( _T("Warning: Unknown ID %hs \"%hs\"\n"), sID, sString );
		return TRUE;
	}

	CSSPair sFoo;
	if ( g_oStrings.Lookup( nID, sFoo ) )
	{
		_tprintf( _T("Error: Duplicate ID %hs \"%hs\"\n"), sID, sString );
		return FALSE;
	}

	g_oStrings.SetAt( nID, CSSPair( sString, sID ) );

	return TRUE;
}

BOOL LoadIDs(LPCTSTR szFilename)
{
	BOOL bSuccess = FALSE;

	FILE* pFile = NULL;
	if ( _tfopen_s( &pFile, szFilename, _T("rb") ) == 0 )
	{
		for (;;)
		{
			CStringA sLine;
			CHAR* res = fgets( sLine.GetBuffer( 4096 ), 4096, pFile );
			sLine.ReleaseBuffer();
			if ( ! res )
				break;		// End of file
			sLine.Trim( " \t\r\n" );
			if ( sLine.IsEmpty() || sLine.GetAt( 0 ) != '#' )
				continue;		// Skip empty lines
			int nPos = sLine.FindOneOf( " \t" );
			if ( nPos == -1 || sLine.Left( nPos ) != "#define" )
				continue;		// Skip unknown line
			sLine = sLine.Mid( nPos + 1 ).TrimLeft( " \t" );
			nPos = sLine.FindOneOf( " \t" );
			if ( nPos == -1 )
				continue;		// Skip unknown line
			CStringA sID = sLine.Left( nPos );
			sLine = sLine.Mid( nPos + 1 ).TrimLeft( " \t" );
			UINT nID = 0;
			if ( sLine.Left( 2 ).CompareNoCase( "0x" ) == 0 )
			{
				if ( sscanf_s( sLine, "%x", &nID ) != 1 )
					continue;	// Skip unknown line
			}
			else
			{
				if ( sscanf_s( sLine, "%u", &nID ) != 1 )
					continue;	// Skip unknown line
			}
			if ( g_oIDs.Lookup( sID, nID ) )
			{
				_tprintf( _T("Error: Duplicate ID %hs\n"), sID );
				continue;
			}
			g_oIDs.SetAt( sID, nID );
			bSuccess = TRUE;
		}
		fclose( pFile );
	}

	return bSuccess;
}

BOOL LoadResources(LPCTSTR szFilename)
{
	FILE* pFile = NULL;
	if ( _tfopen_s( &pFile, szFilename, _T("rb") ) == 0 )
	{
		CStringA sID;
		for ( States nState = stFile ; nState != stError ; )
		{
			CStringA sLine;
			CHAR* res = fgets( sLine.GetBuffer( 4096 ), 4096, pFile );
			sLine.ReleaseBuffer();
			if ( ! res )
			{
				if ( nState != stFile )
					_tprintf( _T("Error: Unexpected end of file\n") );

				break;		// End of file
			}
			sLine.Trim( ", \t\r\n" );
			if ( sLine.IsEmpty() ||
				 sLine.GetAt( 0 ) == '/' ||
				 sLine.GetAt( 0 ) == '#' )
				continue;	// Skip empty line, comment and pragma

			switch ( nState )
			{
			case stFile:
				if ( sLine == "STRINGTABLE" )
					nState = stStringTable;
				else if ( sLine == "GUIDELINES DESIGNINFO" )
					nState = stGuidelines;
				else
				{
					int nPos = sLine.Find( " DIALOG" );		// DIALOG or DIALOGEX
					if ( nPos != -1 )
					{
						CStringA sID = sLine.SpanExcluding( " " );
						UINT nID;
						if ( ! g_oIDs.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Unknown ID \"%hs\" inside DIALOG\n"), sID );
							return 2;
						}
						if ( g_oDialogs.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Duplicate ID \"%hs\" inside DIALOG\n"), sID );
							return 2;
						}
						g_oDialogs.SetAt( sID, nID );
					}

					nPos = sLine.Find( " ICON " );
					if ( nPos != -1 )
					{
						CStringA sID = sLine.SpanExcluding( " " );
						UINT nID;
						if ( ! g_oIDs.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Unknown ID \"%hs\" of ICON\n"), sID );
							return 2;
						}
						if ( g_oIcons.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Duplicate ID \"%hs\" of ICON\n"), sID );
							return 2;
						}
						g_oIcons.SetAt( sID, nID );
					}

					nPos = sLine.Find( " HTML " );
					if ( nPos == -1 )
						nPos = sLine.Find( " GZIP " );
					if ( nPos != -1 )
					{
						CStringA sID = sLine.SpanExcluding( " " );
						UINT nID;
						if ( ! g_oIDs.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Unknown ID \"%hs\" of HTML\n"), sID );
							return 2;
						}
						if ( g_oHtmls.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Duplicate ID \"%hs\" of HTML\n"), sID );
							return 2;
						}
						g_oHtmls.SetAt( sID, nID );
					}

					nPos = sLine.Find( " BITMAP " );
					if ( nPos == -1 )
						nPos = sLine.Find( " JPEG " );
					if ( nPos == -1 )
						nPos = sLine.Find( " PNG " );
					if ( nPos != -1 )
					{
						CStringA sID = sLine.SpanExcluding( " " );
						UINT nID;
						if ( ! g_oIDs.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Unknown ID \"%hs\" of BITMAP\n"), sID );
							return 2;
						}
						if ( g_oBitmaps.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Duplicate ID \"%hs\" of BITMAP\n"), sID );
							return 2;
						}
						g_oBitmaps.SetAt( sID, nID );
					}
				}
				break;

			case stGuidelines:
				if ( sLine == "BEGIN" )
					nState = stGuidelinesContent;
				else
				{
					_tprintf( _T("Error: BEGIN not found after GUIDELINES DESIGNINFO\n") );
					return 2;
				}
				break;

			case stGuidelinesContent:
				if ( sLine == "END" )
					nState = stFile;
				else if ( sLine.Right( 6 ) == "DIALOG" )
				{
					nState = stGuidelinesDialog;
					CStringA sID = sLine.SpanExcluding( "," );
					UINT nID;
					if ( ! g_oIDs.Lookup( sID, nID ) )
					{
						_tprintf( _T("Error: Unknown dialog ID \"%hs\" inside GUIDELINES\n"), sID );
						return 2;
					}
					if ( g_oGuidelines.Lookup( sID, nID ) )
					{
						_tprintf( _T("Error: Duplicate dialog ID \"%hs\" inside GUIDELINES\n"), sID );
						return 2;
					}
					if ( ! g_oDialogs.Lookup( sID, nID ) )
					{
						_tprintf( _T("Error: Orphan dialog ID \"%hs\" inside GUIDELINES\n"), sID );
						return 2;
					}
					g_oGuidelines.SetAt( sID, nID );
				}
				else
				{
					_tprintf( _T("Error: Unknown line \"%hs\" inside GUIDELINES\n"), sLine );
					return 2;
				}
				break;

			case stGuidelinesDialog:
				if ( sLine == "END" )
					nState = stGuidelinesContent;
				break;

			case stStringTable:
				if ( sLine == "BEGIN" )
					nState = stContent;
				else
				{
					_tprintf( _T("Error: BEGIN not found after STRINGTABLE\n") );
					return 2;
				}
				break;

			case stContent:
				if ( sLine == "END" )
					nState = stFile;
				else
				{
					int nPos = sLine.FindOneOf( ", \t" );
					if ( nPos != -1 )
					{
						if ( ProcessString( sLine.Left( nPos ),
							sLine.Mid( nPos + 1 ).TrimLeft( ", \t" ) ) )
							nState = stContent;
						else
							nState = stError;
					}
					else
					{
						sID = sLine;
						nState =  stString;
					}
				}
				break;

			case stString:
				if ( ProcessString( sID, sLine ) )
					nState = stContent;
				else
					nState = stError;
				sID.Empty();
				break;
			}
		}

		fclose( pFile );
	}

	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc < 4 )
	{
		_tprintf( _T("SkinUpdate ") _VERSION_
			_T("\nUsage: SkinUpdate.exe input.h input.rc output.xml\n") );
		return 1;
	}

	LPCTSTR szOutput = NULL;

	for ( int i = 1 ; i < argc ; i++ )
	{
		LPCTSTR szFilename = PathFindFileName( argv[ i ] );
		LPCTSTR szExt = PathFindExtension( szFilename );
		if ( _tcscmp( szExt, _T(".h") ) == 0 )
		{
			if ( ! LoadIDs( argv[ i ] ) )
			{
				_tprintf( _T("Error: Filed to load IDs from: %s\n"), szFilename );
				return 1;
			}
			_tprintf(
				_T("Loaded from %s:\n")
				_T("  %d IDs\n"),
				szFilename, g_oIDs.GetCount() );
		}
		else if ( _tcscmp( szExt, _T(".rc") ) == 0 )
		{
			if ( ! LoadResources( argv[ i ] ) )
			{
				_tprintf( _T("Error: Filed to load strings from: %s\n"), szFilename );
				return 1;
			}
			_tprintf(
				_T("Loaded from %s:\n")
				_T("  %d strings\n")
				_T("  %d guidelines\n")
				_T("  %d dialogs\n")
				_T("  %d icons\n")
				_T("  %d bitmaps\n")
				_T("  %d htmls\n"),
				szFilename,
				g_oStrings.GetCount(),
				g_oGuidelines.GetCount(),
				g_oDialogs.GetCount(),
				g_oIcons.GetCount(),
				g_oBitmaps.GetCount(),
				g_oHtmls.GetCount() );
		}
		else if ( _tcscmp( szExt, _T(".xml") ) == 0 )
		{
			szOutput = argv[ i ];
		}
		else
		{
			_tprintf( _T("Error: Unknown file extension: %s\n"), szExt );
			return 1;
		}
	}

	for ( POSITION pos = g_oDialogs.GetStartPosition() ; pos ; )
	{
		CStringA sID;
		UINT nID;
		g_oDialogs.GetNextAssoc( pos, sID, nID );

		if ( ! g_oGuidelines.Lookup( sID, nID ) )
			_tprintf( _T("Warning: Found dialog \"%hs\" without guideline\n"), sID );
	}

	for ( POSITION pos = g_oIDs.GetStartPosition() ; pos ; )
	{
		CStringA sID;
		UINT nID;
		g_oIDs.GetNextAssoc( pos, sID, nID );

		if ( _strnicmp( sID, "IDI_", 4 ) == 0 )
		{
			UINT nFoo;
			if ( ! g_oIcons.Lookup( sID, nFoo ) )
				_tprintf( _T("Warning: Found orphan icon ID \"%hs\"\n"), sID );
		}
		else if ( _strnicmp( sID, "IDR_", 4 ) == 0 )
		{
			UINT nFoo;
			if ( ! g_oBitmaps.Lookup( sID, nFoo ) &&
				 ! g_oIcons.Lookup( sID, nFoo ) &&
				 ! g_oHtmls.Lookup( sID, nFoo ) )
			{
				_tprintf( _T("Warning: Found orphan bitmap/icon/html ID \"%hs\"\n"), sID );
			}
		}
		else if ( _strnicmp( sID, "IDB_", 4 ) == 0 )
		{
			UINT nFoo;
			if ( ! g_oBitmaps.Lookup( sID, nFoo ) )
				_tprintf( _T("Warning: Found orphan bitmap ID \"%hs\"\n"), sID );
		}
		else if ( _strnicmp( sID, "IDD_", 4 ) == 0 )
		{
			UINT nFoo;
			if ( ! g_oDialogs.Lookup( sID, nFoo ) )
				_tprintf( _T("Warning: Found orphan dialog ID \"%hs\"\n"), sID );
		}
		else if ( _strnicmp( sID, "IDS_", 4 ) == 0 )
		{
			CSSPair oFoo;
			if ( ! g_oStrings.Lookup( nID, oFoo ) )
				_tprintf( _T("Warning: Found orphan string ID \"%hs\"\n"), sID );
		}
		//else if ( _strnicmp( sID, "ID_", 3 ) == 0 )
		//{
		//	CSSPair oFoo;
		//	if ( ! g_oStrings.Lookup( nID, oFoo ) )
		//		_tprintf( _T("Warning: Found orphan string ID \"%hs\"\n"), sID );
		//}
	}

	// Sort by ID
	std::list< CStringA > indexTips;
	std::list< UINT > indexStrings;
	for ( POSITION pos = g_oStrings.GetStartPosition() ; pos ; )
	{
		UINT nID;
		CSSPair oPair;
		g_oStrings.GetNextAssoc( pos, nID, oPair );
		if ( nID < 30000 )	// Strings
			indexStrings.push_back( nID );
		else	// Tips
			indexTips.push_back( oPair.sSecond );
	}
	indexStrings.sort();
	indexTips.sort();

	if ( ! szOutput )
		szOutput = _T("default-en.xml");

	FILE* pFile = NULL;
	if ( _tfopen_s( &pFile, szOutput, _T("wt") ) != 0 )
	{
		_tprintf( _T("Error: Can't create output XML-file: %s\n"), szOutput );
		return 1;
	}

	// File header
	_ftprintf( pFile,
		_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n")
		_T("<skin xmlns=\"http://schemas.getenvy.com/Skin.xsd\" version=\"1.0\">\n\n") );

	// Manifest
	_ftprintf( pFile,
		_T("<!-- Manifest -->\n")
		_T("<manifest\tname=\"(translation language: English)\"\n")
		_T("\t\tauthor=\"(translation author)\"\n")
		_T("\t\tupdatedBy=\"(translation updaters)\"\n")
		_T("\t\tdescription=\"(translation description: Envy English Skin File)\"\n")
		_T("\t\tlink=\"(translation URL: http://getenvy.com)\"\n")
		_T("\t\temail=\"(author e-mail)\"\n")
		_T("\t\tversion=\"(Envy version)\"\n")
		_T("\t\ttype=\"Language\"\n")
		_T("\t\tlanguage=\"(translation language code: en)\"\n")
		_T("\t\tprompt=\"(translation prompt: Click here to select English as your natural language.)\"\n")
		_T("\t\tdir=\"(translation language direction: ltr or rtl)\"\n")
		_T("/>\n\n") );

	// Toolbars
	_ftprintf( pFile,
		_T("\t<!-- Toolbar Definitions -->\n\t<toolbars>\n")
		_T("\t</toolbars>\n\n") );

	// Menus
	_ftprintf( pFile,
		_T("\t<!-- Menu Definitions -->\n\t<menus>\n")
		_T("\t</menus>\n\n") );

	// Documents
	_ftprintf( pFile,
		_T("\t<!-- Documents -->\n\t<documents>\n")
		_T("\t</documents>\n\n") );

	// Command Tips
	_ftprintf( pFile,
		_T("\t<!-- Localised Command Tip Text. The \"message\" is displayed in the status bar, while the \"tip\" is shown in a tooltip -->\n\t<commandTips>\n") );
	for ( std::list< CStringA >::iterator i = indexTips.begin() ; i != indexTips.end() ; ++i )
	{
		UINT nID;
		ATLVERIFY( g_oIDs.Lookup( (*i), nID ) );
		CSSPair oPair;
		ATLVERIFY( g_oStrings.Lookup( nID, oPair ) );
		ATLASSERT( oPair.sSecond == (*i) );
		oPair.sFirst.Replace( "&", "&amp;" );
		oPair.sFirst.Replace( " ", "&#160;" );	// Not a space
		oPair.sFirst.Replace( "\"\"", "&quot;" );
		oPair.sFirst.Replace( "\\r\\n", "\\n" );
		int nPos = oPair.sFirst.Find( "\\n" );
		if ( nPos == -1 )
			_ftprintf( pFile, _T("\t\t<tip id=\"%hs\" message=\"%hs\"/>\n"), oPair.sSecond, oPair.sFirst );
		else
			_ftprintf( pFile, _T("\t\t<tip id=\"%hs\" message=\"%hs\" tip=\"%hs\"/>\n"), oPair.sSecond, oPair.sFirst.Left( nPos ), oPair.sFirst.Mid( nPos + 2 ) );
	}
	_ftprintf( pFile,
		_T("\t</commandTips>\n\n") );

	// Control Tips
	_ftprintf( pFile,
		_T("\t<!-- Tips displayed when mouse is moved over controls in the dialogs -->\n\t<controlTips>\n")
		_T("\t</controlTips>\n\n") );

	// Strings
	_ftprintf( pFile,
		_T("\t<!-- Localised Strings -->\n\t<strings>\n") );
	for ( std::list< UINT >::iterator i = indexStrings.begin() ; i != indexStrings.end() ; ++i )
	{
		CSSPair oPair;
		ATLVERIFY( g_oStrings.Lookup( (*i), oPair ) );
		oPair.sFirst.Replace( "&", "&amp;" );
		oPair.sFirst.Replace( " ", "&#160;" );	// Not a space
		oPair.sFirst.Replace( "\"\"", "&quot;" );
		oPair.sFirst.Replace( "\\r\\n", "\\n" );
		if ( (*i) < 7000 )	// Frames
			_ftprintf( pFile, _T("\t\t<string id=\"WINDOW_%hs\" value=\"%hs\"/>\n"), oPair.sSecond.Left( oPair.sSecond.GetLength() - 5 ).Mid( 4 ), oPair.sFirst );
		else
			_ftprintf( pFile, _T("\t\t<string id=\"%u\" value=\"%hs\"/>\n"), (*i), oPair.sFirst );
	}
	_ftprintf( pFile,
		_T("\t</strings>\n\n") );

	// Dialogs
	_ftprintf( pFile,
		_T("\t<!-- Localised Dialog Text, the \"cookie\" verifies that the dialog matches the skin -->\n")
		_T("\t<!-- Use DialogScan feature, do not edit dialogs manually -->\n\t<dialogs>\n")
		_T("\t</dialogs>\n\n") );

	// Columns
	_ftprintf( pFile,
		_T("\t<!-- Columns Definitions -->\n\t<listColumns>\n")
		_T("\t</listColumns>\n\n") );

	// EOF
	_ftprintf( pFile,
		_T("</skin>\n") );

	fclose( pFile );

	_tprintf( _T("Saved output XML-file: %s\n"), szOutput );

	return 0;
}
