//
// Remote.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2007
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
#include "Settings.h"
#include "Envy.h"
#include "Remote.h"

#include "Network.h"
#include "MatchObjects.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "VendorCache.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadGroups.h"
#include "DownloadGroup.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "Uploads.h"
#include "UploadQueues.h"
#include "UploadQueue.h"
#include "UploadFile.h"
#include "UploadTransfer.h"
#include "UploadTransferBT.h"
#include "Neighbours.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "GProfile.h"
#include "EnvyURL.h"
#include "Skin.h"

#include "WndMain.h"
#include "WndSearch.h"
#include "CtrlDownloads.h"
#include "CtrlUploads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CList<int> CRemote::m_pCookies;

/////////////////////////////////////////////////////////////////////////////
// CRemote construction

CRemote::CRemote(CConnection* pConnection)
{
	CTransfer::AttachTo( pConnection );
	m_mInput.pLimit = m_mOutput.pLimit = NULL;
	OnRead();
}

CRemote::~CRemote()
{
}

/////////////////////////////////////////////////////////////////////////////
// CRemote run event

BOOL CRemote::OnRun()
{
	const DWORD tNow = GetTickCount();

	// 3 minute timeout
	if ( tNow > m_mOutput.tLast + ( 3 * 60 * 1000 ) || ! Network.IsConnected() )
	{
		Close();
		delete this;
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote dropped event

void CRemote::OnDropped()
{
	Close();
	delete this;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote read event

BOOL CRemote::OnRead()
{
	if ( ! CTransfer::OnRead() ) return FALSE;

	if ( m_sHandshake.IsEmpty() )
	{
		if ( GetInputLength() > 4096 || ! Settings.Remote.Enable )
		{
			Close();
			return FALSE;
		}

		Read( m_sHandshake );
	}

	if ( ! m_sHandshake.IsEmpty() )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"%s >> REMOTE REQUEST: %s", (LPCTSTR)m_sAddress, (LPCTSTR)m_sHandshake );

		return ReadHeaders();
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote headers complete event

BOOL CRemote::OnHeadersComplete()
{
	if ( m_sHandshake.Find( L"GET /" ) != 0 )
	{
		Close();
		delete this;
		return FALSE;
	}

	m_sHandshake = m_sHandshake.Mid( 4 ).SpanExcluding( L" \t" );

	CString strPath = m_sHandshake.SpanExcluding( L"?&" );
	ToLower( strPath );

	m_sRedirect.Empty();
	m_sHeader.Empty();
	m_sResponse.Empty();
	m_pResponse.Clear();

	PageSwitch( strPath );

	if ( ! m_sRedirect.IsEmpty() )
	{
		Write( _P("HTTP/1.1 302 Found\r\n") );
		Write( _P("Content-Length: 0\r\n") );
		if ( ! m_sHeader.IsEmpty() )
			Write( m_sHeader );
		Write( _P("Location: ") );
		Write( m_sRedirect );
		Write( _P("\r\n") );
	}
	else if ( ! m_sResponse.IsEmpty() )
	{
		CString strLength;
		Prepare();
		Output( L"tail" );
		int nBytes = WideCharToMultiByte( CP_UTF8, 0, m_sResponse, m_sResponse.GetLength(), NULL, 0, NULL, NULL );
		strLength.Format( L"Content-Length: %i\r\n", nBytes );
		Write( _P("HTTP/1.1 200 OK\r\n") );
		Write( _P("Content-Type: text/html; charset=UTF-8\r\n") );
		Write( strLength );
		if ( ! m_sHeader.IsEmpty() )
			Write( m_sHeader );
	}
	else if ( m_pResponse.m_nLength > 0 )
	{
		Write( _P("HTTP/1.1 200 OK\r\n") );
		CString strLength;
		strLength.Format( L"Content-Length: %u\r\n", m_pResponse.m_nLength );
		Write( strLength );
		if ( ! m_sHeader.IsEmpty() ) Write( m_sHeader );
	}
	else
	{
		Write( _P("HTTP/1.1 404 Not Found\r\n") );
		Write( _P("Content-Length: 0\r\n") );
		Write( _P("Content-Type: text/html\r\n") );
	}

	LogOutgoing();

	Write( _P("\r\n") );
	if ( ! m_sResponse.IsEmpty() )
	{
		Write( m_sResponse, CP_UTF8 );
		m_sResponse.Empty();
	}
	else if ( m_pResponse.m_nLength > 0 )
	{
		Write( &m_pResponse );
	}

	m_sHandshake.Empty();
	ClearHeaders();
	OnWrite();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote get a query string key

CString CRemote::GetKey(LPCTSTR pszName)
{
	int nStart = 0;
	CString strPair = m_sHandshake.Tokenize( L"&?", nStart );

	while ( ! strPair.IsEmpty() )
	{
		CString strName = strPair.SpanExcluding( L"=" );

		if ( strName.CompareNoCase( pszName ) == 0 )
		{
			strName = strPair.Mid( strName.GetLength() + 1 );
			return URLDecode( strName );
		}

		strPair = m_sHandshake.Tokenize( L"&?", nStart );
	}

	return CString();
}

/////////////////////////////////////////////////////////////////////////////
// CRemote check access cookie helper

BOOL CRemote::CheckCookie()
{
	const CString strToken = L"envyremote=";
	const int nToken = strToken.GetLength();

	for ( INT_PTR nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader ++ )
	{
		if ( m_pHeaderName.GetAt( nHeader ).CompareNoCase( L"Cookie" ) == 0 )
		{
			CString strValue( m_pHeaderValue.GetAt( nHeader ) );
			ToLower( strValue );

			int nPos = strValue.Find( strToken );

			if ( nPos >= 0 )
			{
				int nCookie = 0;
				_stscanf( strValue.Mid( nPos + nToken ), L"%i", &nCookie );
				if ( m_pCookies.Find( nCookie ) != NULL ) return FALSE;
			}
		}
	}

	m_sRedirect = L"/remote/";
	return TRUE;
}

// Determines what session ID is currently being used by the logged in user and removes it from the cookie list.
BOOL CRemote::RemoveCookie()
{
	const CString strToken = L"envyremote=";
	const int nToken = strToken.GetLength();

	for ( INT_PTR nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader ++ )
	{
		if ( m_pHeaderName.GetAt( nHeader ).CompareNoCase( L"Cookie" ) == 0 )
		{
			CString strValue( m_pHeaderValue.GetAt( nHeader ) );
			ToLower( strValue );

			int nPos = strValue.Find( strToken );

			if ( nPos >= 0 )
			{
				int nCookie = 0;
				_stscanf( strValue.Mid( nPos + nToken ), L"%i", &nCookie );		// APP_LENGTH LETTERCOUNT + 7
				POSITION pos = m_pCookies.Find( nCookie );
				if ( pos != NULL )
				{
					m_pCookies.RemoveAt( pos );
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote prepare to output a HTML segment

void CRemote::Prepare(LPCTSTR pszPrefix)
{
	if ( pszPrefix )
	{
		for ( POSITION pos = m_pKeys.GetStartPosition() ; pos != NULL ; )
		{
			CString strKey, strValue;
			m_pKeys.GetNextAssoc( pos, strKey, strValue );
			if ( strKey.Find( pszPrefix ) == 0 )
				m_pKeys.RemoveKey( strKey );
		}
		return;
	}

	m_pKeys.RemoveAll();
	if ( ! m_sResponse.IsEmpty() )
		return;

	// Header
	AddText( L"text_metatitle", L"Envy Remote Access" );
	AddText( L"text_envy", L"Envy" );
	AddText( L"text_logout", L"Log out" );
	AddText( L"text_tabhome", L"Home" );
	AddText( L"text_tabdownloads", L"Downloads" );
	AddText( L"text_tabuploads", L"Uploads" );
	AddText( L"text_tabnetworks", L"Network" );
	AddText( L"text_tabsearch", L"Search" );
	Add( L"tabstyle_home", m_nTab == tabHome ? L"active" : L"inactive" );
	Add( L"tabstyle_downloads", m_nTab == tabDownloads ? L"active" : L"inactive" );
	Add( L"tabstyle_uploads", m_nTab == tabUploads ? L"active" : L"inactive" );
	Add( L"tabstyle_networks", m_nTab == tabNetwork ? L"active" : L"inactive" );
	Add( L"tabstyle_search", m_nTab == tabSearch ? L"active" : L"inactive" );
	Add( L"tablink_home", m_nTab == tabNone ? L"#" : L"home" );
	Add( L"tablink_downloads", m_nTab == tabNone ? L"#" : L"downloads" );
	Add( L"tablink_uploads", m_nTab == tabNone ? L"#" : L"uploads" );
	Add( L"tablink_networks", m_nTab == tabNone ? L"#" : L"network" );
	Add( L"tablink_search", m_nTab == tabNone ? L"#" : L"search" );
	Output( L"head" );
	m_pKeys.RemoveAll();

	// Set translatable text
	switch ( m_nTab )
	{
	case tabNone:
		AddText( L"text_welcome", L"Welcome to the <b>Envy Web Interface</b>." );
		AddText( L"text_logininfo", L"If this is your Envy client's address, please enter your username and password, and click the <b>Login</b> button." );
		AddText( L"text_loginfail", L"Username or password was incorrect, please try again." );
		AddText( L"text_loginuser", L"Username:" );
		AddText( L"text_loginpass", L"Password:" );
		AddText( L"text_loginsubmit", L"Login" );
		break;
	case tabHome:
		AddText( L"text_welcome", L"Welcome to the <b>Envy Web Interface</b>." );
		AddText( L"text_homeinfo", L"This web interface allows you to control searches, downloads, uploads and network connections." );
		AddText( L"text_homestart", L"Start downloading:" );
		AddText( L"text_homequick", L"Quick search:" );
		AddText( L"text_anyfile", L"Any File" );
		AddText( L"text_submit", L"Go" );
		AddText( L"text_directdownload", L"Magnet/Torrent download:" );
		AddText( L"text_homelinks", L"Useful links:" );
		AddText( L"text_customlink" );
		AddText( L"text_typetorrent", L"BitTorrent" );
		AddText( L"text_typeapp", L"Application" );
		AddText( L"text_typearchive", L"Archive" );
		AddText( L"text_typeaudio", L"Audio" );
		AddText( L"text_typevideo", L"Video" );
		AddText( L"text_typeimage", L"Image" );
		AddText( L"text_typedoc", L"Text Document" );
		AddText( L"text_typerom", L"ROM Image" );
		AddText( L"text_typeskin", L"Skin" );
		AddText( L"text_typecode", L"Source Code" );
		AddText( L"text_typespreadsheet", L"Spreadsheet" );
		AddText( L"text_typesubtitle", L"Subtitle" );
		AddText( L"text_typecollection", L"Collection" );
		break;
	case tabDownloads:
		AddText( L"text_refreshpage", L"Refresh page" );
		AddText( L"text_downloadcleartip", L"Clear the download from the list (this will not delete your file)" );
		AddText( L"text_downloadclear", L"Clear" );
		AddText( L"text_downloadcancel", L"Cancel" );
		AddText( L"text_downloadpause", L"Pause" );
		AddText( L"text_downloadresume", L"Resume" );
		AddText( L"text_downloadmoresources", L"More Sources" );
		AddText( L"text_directdownload", L"Magnet/Torrent download:" );
		AddText( L"text_sourceaccess", L"Access" );
		AddText( L"text_sourceforget", L"Forget" );
		AddText( L"text_filetype", L"File Type:" );
		AddText( L"text_filename", L"Filename" );	// Skin.GetHeaderTranslation( L"CMatchCtrl", L"File" )
		AddText( L"text_filesize", L"Size" );
		AddText( L"text_fileprogress", L"Progress" );
		AddText( L"text_filespeed", L"Speed" );
		AddText( L"text_filesources", L"Sources" );
		AddText( L"text_filestatus", L"Status" );
		AddText( L"text_fileoptions", L"Options" );
		AddText( L"text_filter", L"Filter:" );
		AddText( L"text_filteractive", L"Active" );
		AddText( L"text_filterqueued", L"Queued" );
		AddText( L"text_filternosources", L"No Sources" );
		AddText( L"text_filterpaused", L"Paused" );
		AddText( L"text_filtershowall", L"Show all sources" );
		AddText( L"text_filtersubmit", L"Filter" );
		AddText( L"text_submit", L"Go" );
		break;
	case tabUploads:
		AddText( L"text_refreshpage", L"Refresh page" );
		AddText( L"text_file", L"File" );
		AddText( L"text_filesize", L"Size" );
		AddText( L"text_filespeed", L"Speed" );
		AddText( L"text_useragent", L"User-Agent" );
		AddText( L"text_user", L"User" );
		break;
	case tabNetwork:
		AddText( L"text_refreshpage", L"Refresh page" );
		AddText( L"text_networkdisabled", L"This network is <b>not</b> enabled" );
		AddText( L"text_networkenabled", L"This network is enabled" );
		AddText( L"text_networkconnect", L"Connect to" );
		AddText( L"text_networkdisconnect", L"Disable" );
		AddText( L"text_networkaddress", L"Address" );
		AddText( L"text_networktime", L"Time" );
		AddText( L"text_networkpackets", L"Packets" );
		AddText( L"text_networkbandwidth", L"Bandwidth" );
		AddText( L"text_networktotal", L"Total" );
		AddText( L"text_networkmode", L"Mode" );
		AddText( L"text_networkleaves", L"Leaves" );
		AddText( L"text_useragent", L"User-Agent" );
		break;
	case tabSearch:
		AddText( L"text_searchinfo", L"To start a new search, type your search term in the box below and click <strong>Start search</strong>. If you want to restrict your search to a particular file type, select it from the list (recommended)." );
		AddText( L"text_searchstart", L"Start Search" );
		AddText( L"text_searchnew", L"New Search" );
		AddText( L"text_searchfor", L"Search for:" );
		AddText( L"text_searchstop", L"Stop Searching" );
		AddText( L"text_searchclose", L"Close Search" );
		AddText( L"text_anyfile", L"Any File" );
		AddText( L"text_filter", L"Filter:" );
		break;
	default:
		;
	}

}

/////////////////////////////////////////////////////////////////////////////
// CRemote add a substitution key for the next HTML segment

void CRemote::Add(LPCTSTR pszKey, LPCTSTR pszValue)
{
	CString strKey( pszKey );
	ToLower( strKey );

	m_pKeys.SetAt( strKey, pszValue );
}

void CRemote::AddText(LPCTSTR pszKey, LPCTSTR pszDefault /*NULL*/)
{
	CString strKey( pszKey ), str;
	ToLower( strKey );

	if ( Skin.LoadRemoteText( str, strKey ) )
		m_pKeys.SetAt( strKey, str );
	else if ( pszDefault )
		m_pKeys.SetAt( strKey, pszDefault );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote output a HTML segment

void CRemote::Output(LPCTSTR pszName)
{
	if ( _tcsstr( pszName, L".." ) || _tcschr( pszName, L'/' ) ) return;

	CString strValue = Settings.General.Path + L"\\Remote\\" + pszName + L".html";

	CFile hFile;
	if ( ! hFile.Open( strValue, CFile::modeRead ) ) return;

	int nBytes = (int)hFile.GetLength();
	CAutoVectorPtr< BYTE > pBytes ( new BYTE[ nBytes ] );
	hFile.Read( pBytes, nBytes );
	hFile.Close();
	LPCSTR pBody = (LPCSTR)(BYTE*)pBytes;
	if ( nBytes > 3 && pBytes[0] == 0xEF && pBytes[1] == 0xBB && pBytes[2] == 0xBF )
	{
		// Skip BOM
		pBody  += 3;
		nBytes -= 3;
	}

	CString strBody = UTF8Decode( pBody, nBytes );

	CList<BOOL> pDisplayStack;

	for ( BOOL bDisplay = TRUE ; ; )
	{
		int nStart = strBody.Find( L"<%" );

		if ( nStart < 0 )
		{
			if ( bDisplay )
				m_sResponse += strBody;
			break;
		}
		else if ( nStart >= 0 )
		{
			if ( bDisplay && nStart > 0 )
				m_sResponse += strBody.Left( nStart );
			strBody = strBody.Mid( nStart + 2 );
		}

		int nEnd = strBody.Find( L"%>" );
		if ( nEnd < 0 ) break;

		CString strKey = strBody.Left( nEnd );
		strBody = strBody.Mid( nEnd + 2 );

		strKey.Trim();
		ToLower( strKey );

		if ( strKey.IsEmpty() )
		{
			// Nothing
		}
		else if ( strKey.GetAt( 0 ) == L'=' && bDisplay )
		{
			strKey = strKey.Mid( 1 );
			strKey.Trim();
			if ( m_pKeys.Lookup( strKey, strValue ) )
				m_sResponse += strValue;
		}
		else if ( strKey.GetAt( 0 ) == L'?' )
		{
			strKey = strKey.Mid( 1 );
			strKey.Trim();

			if ( strKey.IsEmpty() )
			{
				if ( ! pDisplayStack.IsEmpty() )
					bDisplay = pDisplayStack.RemoveTail();
			}
			else
			{
				if ( strKey.GetAt( 0 ) == L'!' )
				{
					strKey = strKey.Mid( 1 );
					strKey.Trim();
					if ( ! m_pKeys.Lookup( strKey, strValue ) ) strValue.Empty();
					pDisplayStack.AddTail( bDisplay );
					bDisplay = bDisplay && strValue.IsEmpty();
				}
				else
				{
					if ( ! m_pKeys.Lookup( strKey, strValue ) ) strValue.Empty();
					pDisplayStack.AddTail( bDisplay );
					bDisplay = bDisplay && ! strValue.IsEmpty();
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page switch

void CRemote::PageSwitch(CString& strPath)
{
	if ( strPath == L"/" || strPath == L"/remote" )
		m_sRedirect = L"/remote/";
	else if ( strPath == L"/remote/" )
		PageLogin();
	else if ( strPath == L"/remote/logout" )
		PageLogout();
	else if ( strPath == L"/remote/home" )
		PageHome();
	else if ( strPath == L"/remote/search" )
		PageSearch();
	else if ( strPath == L"/remote/newsearch" )
		PageNewSearch();
	else if ( strPath == L"/remote/downloads" )
		PageDownloads();
	else if ( strPath == L"/remote/newdownload" )
		PageNewDownload();
	else if ( strPath == L"/remote/uploads" )
		PageUploads();
	else if ( strPath == L"/remote/network" )
		PageNetwork();
	else if ( strPath.Find( L"/remote/resources/" ) == 0 )
		PageImage( strPath );
	else
		PageBanner( strPath );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : login

void CRemote::PageLogin()
{
	if ( ! Settings.Remote.Username.IsEmpty() &&
		 ! Settings.Remote.Password.IsEmpty() )
	{
		CString strUsername = GetKey( L"username" );
		CString strPassword = GetKey( L"password" );

		if ( strUsername.IsEmpty() )
			strUsername = GetKey( L"name" );
		if ( strPassword.IsEmpty() )
			strPassword = GetKey( L"pass" );

		if ( strUsername == Settings.Remote.Username && ! strPassword.IsEmpty() )
		{
			CSHA pSHA1;
			pSHA1.Add( (LPCTSTR)strPassword, strPassword.GetLength() * sizeof(TCHAR) );
			pSHA1.Finish();
			Hashes::Sha1Hash tmp;
			pSHA1.GetHash( &tmp[ 0 ] );
			tmp.validate();
			strPassword = tmp.toString();

			if ( strPassword == Settings.Remote.Password )
			{
				// Success:
				__int32 nCookie = GetRandomNum( 0i32, _I32_MAX );
				m_pCookies.AddTail( nCookie );
				m_sHeader.Format( L"Set-Cookie: EnvyRemote=%i; path=/remote\r\n", nCookie );
				m_sRedirect.Format( L"/remote/home?%i", GetRandomNum( 0i32, _I32_MAX ) );
				return;
			}
		}
	}

	// Pre-fill or Failure:
	m_nTab = tabNone;
	Prepare();		// Header
	if ( ! GetKey( L"submit" ).IsEmpty() )
		Add( L"failure", L"true" );
	else if ( GetKey( L"username" ) == Settings.Remote.Username )
		Add( L"user_name", Settings.Remote.Username );
	Output( L"login" );
}

void CRemote::PageLogout()
{
	// Clear server-side session cookie
	RemoveCookie();

	// Clear client-side session cookie
	m_sHeader.Format( L"Set-Cookie: EnvyRemote=0; path=/remote; Max-Age=0\r\n" );
	m_sRedirect.Format( L"/remote/" );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : home

void CRemote::PageHome()
{
	if ( CheckCookie() ) return;
	m_nTab = tabHome;

	Prepare();		// Header
	Output( L"home" );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : search

void CRemote::PageSearch()
{
	if ( CheckCookie() ) return;
	m_nTab = tabSearch;

	CMainWnd* pMainWnd = static_cast< CMainWnd* >( theApp.m_pMainWnd );
	if ( pMainWnd == NULL || ! pMainWnd->IsKindOf( RUNTIME_CLASS(CMainWnd) ) ) return;

	CSingleLock pLock( &theApp.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	INT_PTR nSearchID = NULL;
	INT_PTR nCloseID = NULL;
	CSearchWnd* pSearchWnd = NULL;
	CString str;

	_stscanf( GetKey( L"id" ), L"%Ii", &nSearchID );
	_stscanf( GetKey( L"close" ), L"%Ii", &nCloseID );

	Prepare();		// Header
	Output( L"searchHeader" );

	for ( CSearchWnd* pFindWnd = NULL ; ( pFindWnd = static_cast< CSearchWnd* >( pMainWnd->m_pWindows.Find( RUNTIME_CLASS(CSearchWnd), pFindWnd ) ) ) != NULL ; )
	{
		Prepare();
		INT_PTR nFindWnd = reinterpret_cast< INT_PTR >( pFindWnd );
		if ( nCloseID == nFindWnd )
		{
			pFindWnd->PostMessage( WM_CLOSE );
			continue;
		}
		else if ( nSearchID == nFindWnd )
		{
			pSearchWnd = pFindWnd;
			Add( L"search_selected", L"true" );
		}

		str.Format( L"%Ii", nFindWnd );
		Add( L"search_id", str );
		str = pFindWnd->GetCaption();
		if ( str.Find( L"Search : " ) == 0 ) str = str.Mid( 9 ).SpanExcluding( L"[" );
		Add( L"search_caption", str );
		Output( L"searchTab" );
	}

	if ( pSearchWnd == NULL )
	{
		str.Empty();

		for ( POSITION pos = SchemaCache.GetIterator() ; pos != NULL ; )
		{
			CSchemaPtr pSchema = SchemaCache.GetNext( pos );
			if ( ! pSchema->m_bPrivate && pSchema->m_nType == CSchema::stFile )
			{
				str += L"<option value=\"" + pSchema->GetURI();
				str += L"\">" + pSchema->m_sTitle;
				str += L"</option>\r\n";
			}
		}

		Prepare();		// Header
		Add( L"schema_option_list", str );
		Output( L"searchNew" );
		Output( L"searchFooter" );
		return;
	}

	if ( ! GetKey( L"stop" ).IsEmpty() )
	{
		pSearchWnd->PostMessage( WM_COMMAND, ID_SEARCH_STOP );
		Sleep( 500 );
	}

	CLockedMatchList pMatches( pSearchWnd->GetMatches() );

	str = GetKey( L"sort" );
	if ( ! str.IsEmpty() )
	{
		int nColumn = 0;
		_stscanf( str, L"%i", &nColumn );

		if ( pMatches->m_bSortDir != 1 && pMatches->m_nSortColumn == nColumn )
			pMatches->SetSortColumn( nColumn, FALSE );
		else
			pMatches->SetSortColumn( nColumn, TRUE );

		pSearchWnd->PostMessage( WM_TIMER, 7 );
	}

	str = GetKey( L"expcol" );
	if ( ! str.IsEmpty() )
	{
		CMatchFile** pLoop = pMatches->m_pFiles;
		for ( DWORD nCount = 0 ; nCount < pMatches->m_nFiles ; nCount++, pLoop++ )
		{
			if ( (*pLoop)->GetURN() == str )
			{
				(*pLoop)->Expand( GetKey( L"collapse" ).IsEmpty() );
				pSearchWnd->PostMessage( WM_TIMER, 7 );
				break;
			}
		}
	}

	str = GetKey( L"download" );
	if ( ! str.IsEmpty() )
	{
		CMatchFile** pLoop = pMatches->m_pFiles;
		for ( DWORD nCount = 0 ; nCount < pMatches->m_nFiles ; nCount++, pLoop++ )
		{
			if ( (*pLoop)->GetURN() == str )
			{
				Downloads.Add( *pLoop );
				pSearchWnd->PostMessage( WM_TIMER, 7 );
				m_sResponse.Empty();
				m_sRedirect = L"downloads?group_reveal=all";
				return;
			}
		}
	}

	if ( ! GetKey( L"setfilter" ).IsEmpty() )
	{
		pMatches->m_sFilter = GetKey( L"filter" );
		pMatches->Filter();
		pSearchWnd->PostMessage( WM_TIMER, 7 );
	}

	Prepare();		// Header
	str.Format( L"%Ii", nSearchID );
	Add( L"search_id", str );
	str.Format( L"%i", GetRandomNum( 0i32, _I32_MAX ) );
	Add( L"random", str );
	if ( ! pSearchWnd->IsPaused() )
		Add( L"searching", L"true" );
	Add( L"search_filter", pMatches->m_sFilter );
	Output( L"searchTop" );

	PageSearchHeaderColumn( MATCH_COL_NAME, Skin.GetHeaderTranslation( L"CMatchCtrl", L"File" ), L"left" );
	PageSearchHeaderColumn( MATCH_COL_SIZE, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Size" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_RATING, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Rating" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_STATUS, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Status" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_COUNT, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Host/Count" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_SPEED, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Speed" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_CLIENT, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Client" ), L"center" );

	Output( L"searchMiddle" );

	CMatchFile** pLoop = pMatches->m_pFiles;

	for ( DWORD nCount = 0 ; nCount < pMatches->m_nFiles ; nCount++, pLoop++ )
	{
		CMatchFile* pFile = *pLoop;
		if ( pFile->GetFilteredCount() == 0 ) continue;

		Add( L"row_urn", pFile->GetURN() );
		Add( L"row_filename", pFile->m_sName );
		if ( pFile->GetFilteredCount() <= 1 )
			Add( L"row_single", L"true" );
		else if ( pFile->m_bExpanded )
			Add( L"row_expanded", L"true" );
		else
			Add( L"row_collapsed", L"true" );

		Output( L"searchRowStart" );

		PageSearchRowColumn( MATCH_COL_SIZE, pFile, Settings.SmartVolume( pFile->m_nSize ) );

		str.Empty();
		for ( INT_PTR nStar = pFile->m_nRating / max( 1, pFile->m_nRated ) ; nStar > 1 ; nStar -- ) str += L'*';
		PageSearchRowColumn( MATCH_COL_RATING, pFile, str );

		str.Empty();
		str += pFile->m_bBusy == TRI_TRUE ? L'B' : L'-';
		str += pFile->m_bPush == TRI_TRUE ? L'F' : L'-';
		str += pFile->m_bStable == TRI_FALSE ? L'U' : L'-';
		PageSearchRowColumn( MATCH_COL_STATUS, pFile, str );

		str.Empty();
		if ( pFile->GetFilteredCount() > 1 )
			str.Format(L"(%u sources)", pFile->GetFilteredCount());
		else
			str = (CString)inet_ntoa( pFile->GetBestAddress() );
		PageSearchRowColumn( MATCH_COL_COUNT, pFile, str );


		PageSearchRowColumn( MATCH_COL_SPEED, pFile, pFile->m_sSpeed );
		PageSearchRowColumn( MATCH_COL_CLIENT, pFile, pFile->GetFilteredCount() == 1 ? pFile->GetBestVendorName() : L"" );

		Output( L"searchRowEnd" );
		Prepare( L"column_" );
		Prepare( L"row_" );

		if ( pFile->m_bExpanded )
		{
			for ( CQueryHit* pHit = pFile->GetHits() ; pHit != NULL ; pHit = pHit->m_pNext )
			{
				if ( ! pHit->m_bFiltered ) continue;

				Add( L"row_urn", pFile->GetURN() );
				Add( L"row_filename", pHit->m_sName );
				Add( L"row_source", L"true" );
				Output( L"searchRowStart" );

				PageSearchRowColumn( MATCH_COL_SIZE, pFile, Settings.SmartVolume( pHit->m_nSize ) );
				str.Empty();
				for ( int nStar = pHit->m_nRating ; nStar > 1 ; nStar -- ) str += L"*";
				PageSearchRowColumn( MATCH_COL_RATING, pFile, str );

				str.Empty();
				str += pFile->m_bBusy == TRI_TRUE ? L'B' : L'-';
				str += pFile->m_bPush == TRI_TRUE ? L'F' : L'-';
				str += pFile->m_bStable == TRI_FALSE ? L'U' : L'-';
				PageSearchRowColumn( MATCH_COL_STATUS, pFile, str );

				PageSearchRowColumn( MATCH_COL_COUNT, pFile, (CString)inet_ntoa( pHit->m_pAddress ) );
				PageSearchRowColumn( MATCH_COL_SPEED, pFile, pHit->m_sSpeed );
				PageSearchRowColumn( MATCH_COL_CLIENT, pFile, pHit->m_pVendor->m_sName );

				Output( L"searchRowEnd" );
				Prepare( L"column_" );
				Prepare( L"row_" );
			}
		}
	}

	Output( L"searchBottom" );
	Prepare();
	Output( L"searchFooter" );
}

void CRemote::PageSearchHeaderColumn(int nColumnID, LPCTSTR pszCaption, LPCTSTR pszAlign)
{
	CString str;
	str.Format( L"%i", nColumnID );
	Add( L"column_id", str );
	Add( L"column_align", pszAlign );
	Add( L"column_caption", pszCaption );
	Output( L"searchColumn" );
	Prepare( L"column_" );
}

void CRemote::PageSearchRowColumn(int nColumnID, CMatchFile* pFile, LPCTSTR pszValue, LPCTSTR pszAlign)
{
	CString str;
	str.Format( L"%i", nColumnID );
	Add( L"column_id", str );
	Add( L"column_align", pszAlign );
	Add( L"row_urn", pFile->GetURN() );
	Add( L"row_value", pszValue );
	Output( L"searchRowColumn" );
	Prepare( L"column_" );
	Prepare( L"row_" );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : newsearch

void CRemote::PageNewSearch()
{
	if ( CheckCookie() ) return;

	CMainWnd* pMainWnd = (CMainWnd*)theApp.m_pMainWnd;
	if ( pMainWnd == NULL || ! pMainWnd->IsKindOf( RUNTIME_CLASS(CMainWnd) ) ) return;

	CSingleLock pLock( &theApp.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	const CString strSearch = GetKey( L"search" );
	const CString strSchema = GetKey( L"schema" );

	if ( strSearch.IsEmpty() || ( ! strSchema.IsEmpty() && SchemaCache.Get( strSchema ) == NULL ) )
	{
		m_sRedirect = L"home";
		return;
	}

	CQuerySearchPtr pSearch	= new CQuerySearch();
	pSearch->m_sSearch		= strSearch;
	pSearch->m_pSchema		= SchemaCache.Get( strSchema );

	CString strURI;
	if ( pSearch->m_pSchema != NULL )
		strURI = pSearch->m_pSchema->GetURI();

	Settings.Search.LastSchemaURI = strURI;

	pMainWnd->PostMessage( WM_OPENSEARCH, (WPARAM)pSearch.Detach() );
	pLock.Unlock();
	Sleep( 500 );

	m_sRedirect = L"search";
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : downloads

void CRemote::PageDownloads()
{
	if ( CheckCookie() ) return;
	m_nTab = tabDownloads;

	CString str;
	str.Format( L"%i", GetRandomNum( 0i32, _I32_MAX ) );

	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );

	Prepare();		// Header
	Add( L"random", str );
	Output( L"downloadsHeader" );

	BOOL bExclusive = ! GetKey( L"group_exclusive" ).IsEmpty();
	BOOL bReveal = ! GetKey( L"group_reveal" ).IsEmpty();

	for ( POSITION posGroup = DownloadGroups.GetIterator() ; posGroup != NULL ; )
	{
		CDownloadGroup* pGroup = DownloadGroups.GetNext( posGroup );

		CString strGroupID;
		strGroupID.Format( L"%p", pGroup );
		Add( L"group_id", strGroupID );

		if ( bExclusive )
			pGroup->m_bRemoteSelected = ( GetKey( L"group_exclusive" ) == strGroupID );
		else if ( bReveal )
			pGroup->m_bRemoteSelected = TRUE;
		else if ( GetKey( L"group_select" ) == strGroupID )
			pGroup->m_bRemoteSelected = TRUE;
		else if ( GetKey( L"group_deselect" ) == strGroupID )
			pGroup->m_bRemoteSelected = FALSE;

		Add( L"group_caption", pGroup->m_sName );
		if ( pGroup->m_bRemoteSelected )
			Add( L"group_selected", L"true" );
		Output( L"downloadsTab" );
		Prepare( L"group_" );
	}

	if ( ! GetKey( L"filter_set" ).IsEmpty() )
	{
		Settings.Downloads.FilterMask &= ~( DLF_ACTIVE | DLF_PAUSED | DLF_QUEUED | DLF_SOURCES | DLF_SEED );
		if ( GetKey( L"filter_active" ) == L"1" ) Settings.Downloads.FilterMask |= DLF_ACTIVE;
		if ( GetKey( L"filter_paused" ) == L"1" ) Settings.Downloads.FilterMask |= DLF_PAUSED;
		if ( GetKey( L"filter_queued" ) == L"1" ) Settings.Downloads.FilterMask |= DLF_QUEUED;
		if ( GetKey( L"filter_sources" ) == L"1" ) Settings.Downloads.FilterMask |= DLF_SOURCES;
		if ( GetKey( L"filter_seeds" ) == L"1" )  Settings.Downloads.FilterMask |= DLF_SEED;
		Settings.Downloads.ShowSources = ( GetKey( L"filter_show_all" ) == L"1" );
	}

	Add( L"filter_active", ( Settings.Downloads.FilterMask & DLF_ACTIVE ) ? L"checked=\"checked\"" : L"" );
	Add( L"filter_paused", ( Settings.Downloads.FilterMask & DLF_PAUSED ) ? L"checked=\"checked\"" : L"" );
	Add( L"filter_queued", ( Settings.Downloads.FilterMask & DLF_QUEUED ) ? L"checked=\"checked\"" : L"" );
	Add( L"filter_sources", ( Settings.Downloads.FilterMask & DLF_SOURCES ) ? L"checked=\"checked\"" : L"" );
	Add( L"filter_seeds", ( Settings.Downloads.FilterMask & DLF_PAUSED ) ? L"checked=\"checked\"" : L"" );
	Add( L"filter_show_all", Settings.Downloads.ShowSources ? L"checked=\"checked\"" : L"" );
	Output( L"downloadsTop" );

	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload != NULL ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		CString strDownloadID;
		strDownloadID.Format( L"%p", pDownload );

		if ( GetKey( L"modify_id" ) == strDownloadID )
		{
			CString strAction = GetKey( L"modify_action" );
			strAction.MakeLower();

			if ( strAction == L"expand" )
			{
				if ( CDownloadsCtrl::IsExpandable( pDownload ) )
					pDownload->m_bExpanded = TRUE;
			}
			else if ( strAction == L"collapse" )
			{
				if ( CDownloadsCtrl::IsExpandable( pDownload ) )
					pDownload->m_bExpanded = FALSE;
			}
			else if ( strAction == L"resume" )
			{
				pDownload->Resume();
			}
			else if ( strAction == L"pause" )
			{
				if ( ! pDownload->IsPaused() && ! pDownload->IsTasking() )
					pDownload->Pause();
			}
			else if ( strAction == L"cancel" )
			{
				if ( ! pDownload->IsTasking() )
					pDownload->Remove();
				continue;
			}
			else if ( strAction == L"clear" )
			{
				if ( pDownload->IsCompleted() && ! pDownload->IsPreviewVisible() )
				{
					pDownload->Remove();
					continue;
				}
			}
			else if ( strAction == L"more_sources" )
			{
				// roo_koo_too improvement
				pDownload->FindMoreSources();
			}
		}

		if ( CDownloadsCtrl::IsFiltered( pDownload ) ) continue;

		CDownloadGroup* pGroup = NULL;

		for ( POSITION posGroup = DownloadGroups.GetIterator() ; posGroup != NULL ; )
		{
			pGroup = DownloadGroups.GetNext( posGroup );
			if ( pGroup->m_bRemoteSelected && pGroup->Contains( pDownload ) ) break;
			pGroup = NULL;
		}

		if ( pGroup == NULL ) continue;

		Add( L"download_id", strDownloadID );
		Add( L"download_filename", pDownload->GetDisplayName() );
		Add( L"download_size", ( pDownload->m_nSize == SIZE_UNKNOWN ) ?
			LoadString( IDS_STATUS_UNKNOWN ) : Settings.SmartVolume( pDownload->m_nSize ) );
		int nProgress = int( pDownload->GetProgress() );
		str.Format( L"%i", nProgress );
		Add( L"download_percent", str );
		str.Format( L"%i", 100 - nProgress );
		Add( L"download_percent_inverse", str );
		Add( L"download_speed", Settings.SmartSpeed( pDownload->GetMeasuredSpeed() ) );
		if ( CDownloadsCtrl::IsExpandable( pDownload ) )
		{
			if ( pDownload->m_bExpanded )
				Add( L"download_is_expanded", L"true" );
			else
				Add( L"download_is_collapsed", L"true" );
		}
		if ( pDownload->IsCompleted() )
			Add( L"download_is_complete", L"true" );
		else if ( pDownload->IsPaused() )
			Add( L"download_is_paused", L"true" );

		Add( L"download_status", pDownload->GetDownloadStatus() );
		Add( L"download_sources", pDownload->GetDownloadSources() );
		Output( L"downloadsDownload" );

		if ( pDownload->m_bExpanded && CDownloadsCtrl::IsExpandable( pDownload ) )
		{
			for ( POSITION posSource = pDownload->GetIterator() ; posSource ; )
			{
				CDownloadSource* pSource = pDownload->GetNext( posSource );

				ASSERT( pSource->m_pDownload == pDownload );

				CString strSourceID;
				strSourceID.Format( L"%p", pSource );

				if ( GetKey( L"modify_id" ) == strSourceID )
				{
					CString strModifyAction = GetKey( L"modify_action" );
					strModifyAction.MakeLower();

					if ( strModifyAction == L"access" )
					{
						// Only create a new Transfer if there isn't already one
						if ( pSource->IsIdle() && pSource->m_nProtocol != PROTOCOL_ED2K )
						{
							if ( pDownload->IsPaused() )
								pDownload->Resume();	// Workaround duplicate

							pDownload->Resume();

							if ( pSource->m_bPushOnly )
								pSource->PushRequest();
							else if ( CDownloadTransfer* pTransfer = pSource->CreateTransfer() )
								pTransfer->Initiate();
						}
					}
					else if ( strModifyAction == L"forget" )
					{
						pSource->Remove( TRUE, TRUE );
						continue;
					}
				}

				if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
				{
					Add( L"source_id", strSourceID );
					Add( L"source_agent", pSource->m_sServer );
					Add( L"source_nick", pSource->m_sNick );

					if ( ! pSource->IsIdle() )
					{
						Add( L"source_status", pSource->GetState( FALSE ) );
						Add( L"source_volume", Settings.SmartVolume( pSource->GetDownloaded() ) );
						if ( DWORD nSpeed = pSource->GetMeasuredSpeed() )
							Add( L"source_speed", Settings.SmartSpeed( nSpeed ) );
						Add( L"source_address", pSource->GetAddress() );
						Add( L"source_caption", pSource->GetAddress() + L" - " + pSource->m_sNick );
					}
					else	// No transfer
					{
						Add( L"source_address", CString( inet_ntoa( pSource->m_pAddress ) ) );
						Add( L"source_caption", CString( inet_ntoa( pSource->m_pAddress ) ) + L" - " + pSource->m_sNick );

						if ( pSource->m_tAttempt > 0 )
						{
							DWORD tNow = GetTickCount();

							if ( pSource->m_tAttempt >= tNow )
							{
								tNow = ( pSource->m_tAttempt - tNow ) / 1000;
								CString strSourceStatus;
								strSourceStatus.Format( L"%.2u:%.2u", tNow / 60, tNow % 60 );
								Add( L"source_status", strSourceStatus );
							}
						}
					}

					Output( L"downloadsSource" );
					Prepare( L"source_" );
				}
			}
		}

		Prepare( L"download_" );
	} // for POSITION loop

	Output( L"downloadsBottom" );
	Output( L"downloadsFooter" );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : newDownload

void CRemote::PageNewDownload()
{
	if ( CheckCookie() ) return;

	CEnvyURL pURI;
	if ( pURI.Parse( GetKey( L"uri" ) ) )
		Downloads.Add( pURI );

	m_sRedirect = L"downloads?group_reveal=all";
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : uploads

void CRemote::PageUploads()
{
	if ( CheckCookie() ) return;
	m_nTab = tabUploads;

	CSingleLock pLock( &UploadQueues.m_pSection, FALSE );
	if ( ! SafeLock( pLock ) )
		return;

	Prepare();		// Header

	CString strRandom;
	strRandom.Format( L"%i", GetRandomNum( 0i32, _I32_MAX ) );
	Add( L"random", strRandom );

	Output( L"uploadsHeader" );

	for ( POSITION posQueue = CUploadsCtrl::GetQueueIterator() ; posQueue != NULL ; )
	{
		CUploadQueue* pQueue = CUploadsCtrl::GetNextQueue( posQueue );

		CString strQueueID;
		strQueueID.Format( L"%p", pQueue );

		if ( GetKey( L"queue_expand" ) == strQueueID )
			pQueue->m_bExpanded = TRUE;
		else if ( GetKey( L"queue_collapse" ) == strQueueID )
			pQueue->m_bExpanded = FALSE;

		POSITION posFile = CUploadsCtrl::GetFileIterator( pQueue );
		if ( posFile == NULL ) continue;

		Prepare();
		Add( L"queue_id", strQueueID );
		Add( L"queue_caption", pQueue->m_sName );
		if ( pQueue->m_bExpanded )
			Add( L"queue_expanded", L"true" );

		if ( pQueue != UploadQueues.m_pTorrentQueue && pQueue != UploadQueues.m_pHistoryQueue )
		{
			CString str;
			str.Format( L"%u", pQueue->GetTransferCount() );
			Add( L"queue_transfers", str );
			str.Format( L"%u", pQueue->GetQueuedCount() );
			Add( L"queue_queued", str );
			Add( L"queue_bandwidth", Settings.SmartSpeed( pQueue->GetMeasuredSpeed() ) );
		}

		Output( L"uploadsQueueStart" );

		if ( pQueue->m_bExpanded )
		{
			while ( posFile != NULL )
			{
				int nPosition;
				CUploadFile* pFile = CUploadsCtrl::GetNextFile( pQueue, posFile, &nPosition );
				if ( pFile == NULL ) continue;
				CUploadTransfer* pTransfer = pFile->GetActive();

				CString strFileID;
				strFileID.Format( L"%p", pFile );

				if ( GetKey( L"drop" ) == strFileID )
				{
					pFile->Remove();
					continue;
				}

				Add( L"file_id", strFileID );
				Add( L"file_filename", pFile->m_sName );
				Add( L"file_size", Settings.SmartVolume( pFile->m_nSize ) );

				if ( pTransfer != NULL )
				{
					Add( L"file_address", pTransfer->m_sAddress );
					Add( L"file_nick", pTransfer->m_sRemoteNick );
					Add( L"file_user", pTransfer->m_sAddress + L" - " + pTransfer->m_sRemoteNick );
					Add( L"file_agent", pTransfer->m_sUserAgent );
				}

				CString str;
				if ( pTransfer == NULL || pTransfer->m_nState == upsNull )
				{
					LoadString( str, IDS_STATUS_COMPLETED );
				}
				else if ( pTransfer->m_nProtocol == PROTOCOL_BT )
				{
					CUploadTransferBT* pBT = (CUploadTransferBT*)pTransfer;

					if ( ! pBT->m_bInterested )
						LoadString( str, IDS_STATUS_UNINTERESTED );
					else if ( pBT->m_bChoked )
						LoadString( str, IDS_STATUS_CHOKED );
					else if ( DWORD nSpeed = pTransfer->GetMeasuredSpeed() )
						str = Settings.SmartSpeed( nSpeed );
				}
				else if ( nPosition > 0 )
				{
					LoadString( str, IDS_STATUS_Q );
					str.Format( L"%s %i", (LPCTSTR)str, nPosition );
				}
				else
				{
					if ( DWORD nSpeed = pTransfer->GetMeasuredSpeed() )
						str = Settings.SmartSpeed( nSpeed );
					else
						LoadString( str, IDS_STATUS_NEXT );
				}
				Add( L"file_speed", str );
				Add( L"file_status", str );

				Output( L"uploadsFile" );
				Prepare( L"file_" );
			}
		}

		Output( L"uploadsQueueEnd" );
		Prepare( L"queue_" );
	}

	Prepare();
	Output( L"uploadsFooter" );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : network

void CRemote::PageNetwork()
{
	if ( CheckCookie() ) return;
	m_nTab = tabNetwork;

	CSingleLock pLock( &Network.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	DWORD nNeighbourID = 0;
	_stscanf( GetKey( L"drop" ), L"%lu", &nNeighbourID );

	if ( nNeighbourID != 0 )
	{
		if ( CNeighbour* pNeighbour = Neighbours.Get( nNeighbourID ) )
			pNeighbour->Close( IDS_CONNECTION_CLOSED );
	}

	Prepare();		// Header

	CString str;
	str.Format( L"%i", GetRandomNum( 0i32, _I32_MAX ) );
	Add( L"random", str );
	Output( L"networkHeader" );

	PageNetworkNetwork( PROTOCOL_G2, &Settings.Gnutella2.Enabled, L"Gnutella2" );	// protocolNames[ PROTOCOL_G2 ]
	PageNetworkNetwork( PROTOCOL_G1, &Settings.Gnutella1.Enabled, L"Gnutella" ); 	// protocolNames[ PROTOCOL_G1 ]
	PageNetworkNetwork( PROTOCOL_ED2K, &Settings.eDonkey.Enabled, L"eDonkey" );		// protocolNames[ PROTOCOL_ED2K ]
	PageNetworkNetwork( PROTOCOL_DC, &Settings.DC.Enabled, L"DC++" );				// protocolNames[ PROTOCOL_DC ] )
	PageNetworkNetwork( PROTOCOL_BT, &Settings.BitTorrent.Enabled, L"BitTorrent" );	// protocolNames[ PROTOCOL_BT ] )

	Output( L"networkFooter" );
}

void CRemote::PageNetworkNetwork(int nID, bool* pbConnect, LPCTSTR pszName)
{
	CSingleLock pLock( &Network.m_pSection );

	CString str;
	str.Format( L"%i", nID );

	if ( GetKey( L"connect" ) == str )
	{
		*pbConnect = TRUE;
		Network.Connect( TRUE );
	}
	else if ( GetKey( L"disconnect" ) == str )
	{
		*pbConnect = FALSE;

		if ( SafeLock( pLock ) )
		{
			for ( POSITION pos = Neighbours.GetIterator() ; pos != NULL ; )
			{
				CNeighbour* pNeighbour = Neighbours.GetNext( pos );
				if ( pNeighbour->m_nProtocol == PROTOCOL_NULL ||
					 pNeighbour->m_nProtocol == nID )
					pNeighbour->Close( IDS_CONNECTION_CLOSED );
			}
			pLock.Unlock();
		}
	}

	Add( L"network_id", str );
	Add( L"network_caption", pszName );
	if ( *pbConnect ) Add( L"network_connected", L"true" );
	Output( L"networkNetStart" );

	pLock.Lock();

	for ( POSITION pos = Neighbours.GetIterator() ; pos != NULL ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		if ( pNeighbour->m_nProtocol != nID ) continue;
		pNeighbour->Measure();

		str.Format( L"%p", pNeighbour );
		Add( L"row_id", str );
		Add( L"row_address", pNeighbour->m_sAddress );
	//	Add( L"row_mode", Neighbours.GetName( pNeighbour ) );	// ToDo
		Add( L"row_agent", pNeighbour->m_sUserAgent );
	//	Add( L"row_nick", Neighbours.GetNick( pNeighbour ) );	// ToDo
		str.Format( L"%u -/- %u", pNeighbour->m_nInputCount, pNeighbour->m_nOutputCount );
		Add( L"row_packets", str );
		str.Format( L"%s -/- %s",
			(LPCTSTR)Settings.SmartSpeed( pNeighbour->m_mInput.nMeasure ),
			(LPCTSTR)Settings.SmartSpeed( pNeighbour->m_mOutput.nMeasure ) );
		Add( L"row_bandwidth", str );
		str.Format( L"%s -/- %s",
			(LPCTSTR)Settings.SmartVolume( pNeighbour->m_mInput.nTotal ),
			(LPCTSTR)Settings.SmartVolume( pNeighbour->m_mOutput.nTotal ) );
		Add( L"row_total", str );

		switch ( pNeighbour->m_nState )
		{
		case nrsConnecting:
			LoadString( str, IDS_NEIGHBOUR_CONNECTING );
			break;
		case nrsHandshake1:
		case nrsHandshake2:
		case nrsHandshake3:
			LoadString( str, IDS_NEIGHBOUR_HANDSHAKING );
			break;
		case nrsRejected:
			LoadString( str, IDS_NEIGHBOUR_REJECTED );
			break;
		case nrsClosing:
			LoadString( str, IDS_NEIGHBOUR_CLOSING );
			break;
		case nrsConnected:
			{
				const DWORD tNow = ( GetTickCount() - pNeighbour->m_tConnected ) / 1000;	// Seconds
				if ( tNow > 86400 )
					str.Format( L"%u:%.2u:%.2u:%.2u", tNow / 86400, ( tNow / 3600 ) % 24, ( tNow / 60 ) % 60, tNow % 60 );
				else
					str.Format( L"%u:%.2u:%.2u", tNow / 3600, ( tNow / 60 ) % 60, tNow % 60 );
			}
			break;
		case nrsNull:
		default:
			LoadString( str, IDS_NEIGHBOUR_UNKNOWN );
			break;
		}
		Add( L"row_time", str );

		if ( pNeighbour->GetUserCount() )
		{
			if ( pNeighbour->GetUserLimit() )
				str.Format( L"%u/%u", pNeighbour->GetUserCount(), pNeighbour->GetUserLimit() );
			else
				str.Format( L"%u", pNeighbour->GetUserCount() );
			Add( L"row_leaves", str );
		}

		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
		{
		//	CG1Neighbour* pG1 = reinterpret_cast<CG1Neighbour*>(pNeighbour);

			switch ( pNeighbour->m_nNodeType )
			{
			case ntNode:
				LoadString( str, IDS_NEIGHBOUR_G1PEER );
				break;
			case ntHub:
				LoadString( str, IDS_NEIGHBOUR_G1ULTRA );
				break;
			case ntLeaf:
				LoadString( str, IDS_NEIGHBOUR_G1LEAF );
				break;
			}

			Add( L"row_mode", str );
			str.Empty();
		}
		else if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
		{
			CG2Neighbour* pG2 = static_cast<CG2Neighbour*>(pNeighbour);

			switch ( pNeighbour->m_nNodeType )
			{
			case ntNode:
				LoadString( str, IDS_NEIGHBOUR_G2PEER );
				break;
			case ntHub:
				LoadString( str, IDS_NEIGHBOUR_G2HUB );
				break;
			case ntLeaf:
				LoadString( str, IDS_NEIGHBOUR_G2LEAF );
				break;
			}

			Add( L"row_mode", str );
			str.Empty();

			if ( pG2->m_pProfile )
				str = pG2->m_pProfile->GetNick();
		}
		else if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
		{
			CEDNeighbour* pED2K = static_cast<CEDNeighbour*>(pNeighbour);

			if ( pED2K->m_nClientID > 0 )
				LoadString( str, CEDPacket::IsLowID( pED2K->m_nClientID ) ? IDS_NEIGHBOUR_ED2K_LOWID : IDS_NEIGHBOUR_ED2K_HIGHID );
			else
				str = L"eDonkey2000";

			Add( L"row_mode", str );

			str = pED2K->m_sServerName;
		}
		else if ( pNeighbour->m_nProtocol == PROTOCOL_DC )
		{
			str = pNeighbour->m_sServerName;
		}

		Add( L"row_nick", str );
		str = pNeighbour->m_sAddress + L" - " + str;
		Add( L"row_caption", str );

		Output( L"networkRow" );
		Prepare( L"row_" );
	}

	Output( L"networkNetEnd" );
	Prepare( L"network_" );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : banner

void CRemote::PageBanner(CString& strPath)
{
	ResourceRequest( strPath, m_pResponse, m_sHeader );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : image server

void CRemote::PageImage(CString& strPath)
{
	if ( CheckCookie() ) return;

	strPath = strPath.Mid( 15 );
	if ( strPath.Find( L'%' ) >= 0 ) return;
	if ( strPath.Find( L'/' ) >= 0 ) return;
	if ( strPath.Find( L'\\' ) >= 0 ) return;

	strPath = Settings.General.Path + L"\\Remote\\Resources\\" + strPath;

	CFile hFile;
	if ( hFile.Open( strPath, CFile::modeRead ) )
	{
		m_pResponse.EnsureBuffer( (DWORD)hFile.GetLength() );
		hFile.Read( m_pResponse.m_pBuffer, (UINT)hFile.GetLength() );
		m_pResponse.m_nLength += (DWORD)hFile.GetLength();
		hFile.Close();
	}
}
