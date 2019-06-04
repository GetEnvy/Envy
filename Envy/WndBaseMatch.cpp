//
// WndBaseMatch.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2015
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
#include "Settings.h"
#include "Envy.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "MatchObjects.h"
#include "Network.h"
#include "Packet.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "Library.h"
#include "SharedFile.h"
#include "Downloads.h"
#include "Download.h"
#include "Transfers.h"
#include "Security.h"
#include "ChatWindows.h"
#include "MatchListView.h"
#include "RelatedSearch.h"
#include "ResultFilters.h"

#include "WndMain.h"
#include "WndBaseMatch.h"
#include "WndDownloads.h"
#include "WndSearch.h"
#include "WndLibrary.h"
#include "WndBrowseHost.h"
#include "DlgFilterSearch.h"
#include "DlgNewSearch.h"
#include "DlgHitColumns.h"
#include "DlgURLCopy.h"
#include "DlgURLExport.h"
#include "DlgExistingFile.h"
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CBaseMatchWnd, CPanelWnd)

BEGIN_MESSAGE_MAP(CBaseMatchWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEWHEEL()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_MDIACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_SEARCH_DOWNLOAD, OnUpdateSearchDownload)
	ON_COMMAND(ID_SEARCH_DOWNLOAD, OnSearchDownload)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_DOWNLOADNOW, OnUpdateSearchDownloadNow)
	ON_COMMAND(ID_SEARCH_DOWNLOADNOW, OnSearchDownloadNow)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_URI, OnUpdateSearchURI)
	ON_COMMAND(ID_SEARCH_URI, OnSearchURI)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_CHAT, OnUpdateSearchChat)
	ON_COMMAND(ID_SEARCH_CHAT, OnSearchChat)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FILTER, OnUpdateSearchFilter)
	ON_COMMAND(ID_SEARCH_FILTER, OnSearchFilter)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FILTER_REMOVE, OnUpdateSearchFilterRemove)
	ON_COMMAND(ID_SEARCH_FILTER_REMOVE, OnSearchFilterRemove)
	ON_COMMAND(ID_SEARCH_COLUMNS, OnSearchColumns)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_BITPRINTS_WEB, OnUpdateLibraryBitprintsWeb)
	ON_COMMAND(ID_LIBRARY_BITPRINTS_WEB, OnLibraryBitprintsWeb)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_BAN, OnUpdateSecurityBan)
	ON_COMMAND(ID_SECURITY_BAN, OnSecurityBan)
	ON_UPDATE_COMMAND_UI(ID_HITMONITOR_SEARCH, OnUpdateHitMonitorSearch)
	ON_COMMAND(ID_HITMONITOR_SEARCH, OnHitMonitorSearch)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_COMMAND(ID_SEARCH_FILTER_RAW, OnSearchFilterRaw)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_THIS, OnUpdateSearchForThis)
	ON_COMMAND(ID_SEARCH_FOR_THIS, OnSearchForThis)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_SIMILAR, OnUpdateSearchForSimilar)
	ON_COMMAND(ID_SEARCH_FOR_SIMILAR, OnSearchForSimilar)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_ARTIST, OnUpdateSearchForArtist)
	ON_COMMAND(ID_SEARCH_FOR_ARTIST, OnSearchForArtist)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_ALBUM, OnUpdateSearchForAlbum)
	ON_COMMAND(ID_SEARCH_FOR_ALBUM, OnSearchForAlbum)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_SERIES, OnUpdateSearchForSeries)
	ON_COMMAND(ID_SEARCH_FOR_SERIES, OnSearchForSeries)
	ON_EN_KILLFOCUS(IDC_FILTER_BOX, OnKillFocusFilter)
	ON_BN_CLICKED(AFX_IDW_TOOLBAR, OnToolbarReturn)
	ON_BN_DOUBLECLICKED(AFX_IDW_TOOLBAR, OnToolbarEscape)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SCHEMA_MENU_MIN, ID_SCHEMA_MENU_MAX, OnUpdateBlocker)
	ON_UPDATE_COMMAND_UI_RANGE(3000, 3100, OnUpdateFilters)
	ON_COMMAND_RANGE(3000, 3100, OnFilters)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBaseMatchWnd construction

CBaseMatchWnd::CBaseMatchWnd()
	: m_pMatches	( NULL )
	, m_pCoolMenu	( NULL )
	, m_bContextMenu( FALSE )
	, m_tContextMenu( 0 )
	, m_bPaused 	( TRUE )
	, m_bUpdate 	( FALSE )
	, m_bBMWActive	( TRUE )
	, m_nCacheFiles	( 0 )
	, m_tModify 	( static_cast< DWORD >( time( NULL ) ) )
{
	m_pMatches = new CMatchList( this );
}

CBaseMatchWnd::~CBaseMatchWnd()
{
	delete m_pMatches;

	if ( m_pCoolMenu ) delete m_pCoolMenu;
}

void CBaseMatchWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	m_wndList.OnSkinChange();
	m_wndFilter.SetFont( &CoolInterface.m_fntNormal );

	m_wndToolBar.Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CBaseMatchWnd message handlers

int CBaseMatchWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndList.Create( m_pMatches, this );
//	m_wndList.ModifyStyle( 0, WS_TABSTOP );

	m_wndList.ModifyStyleEx( 0, WS_EX_COMPOSITED );		// Stop rare flicker XP+, CPU intensive

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|CBRS_NOALIGN ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndToolBar.ModifyStyleEx( 0, WS_EX_CONTROLPARENT );

	if ( ! m_wndFilter.Create( WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_AUTOHSCROLL, rectDefault, &m_wndToolBar, IDC_FILTER_BOX, L"Search", L"Filter.%.2i" ) ) return -1;
	//m_wndFilter.SetFont( &CoolInterface.m_fntNormal );

	SetTimer( 2, 500, NULL );

	return 0;
}

void CBaseMatchWnd::OnDestroy()
{
	KillTimer( 2 );

	m_pMatches->m_pSection.Lock();
	m_bPaused = TRUE;
	m_bUpdate = FALSE;
	m_pMatches->m_pSection.Unlock();

	CPanelWnd::OnDestroy();
}

BOOL CBaseMatchWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndToolBar.m_hWnd )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}

	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CBaseMatchWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );
	SizeListAndBar( &m_wndList, &m_wndToolBar );
}

void CBaseMatchWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	if ( m_wndList.HitTestHeader( point ) && m_wndList.m_pSchema != NULL )
	{
		CMenu* pMenu = CSchemaColumnsDlg::BuildColumnMenu( m_wndList.m_pSchema, &m_wndList.m_pColumns );

		pMenu->AppendMenu( MF_SEPARATOR, ID_SEPARATOR, (LPCTSTR)NULL );
		CString strText;
		LoadString( strText, IDS_SCHEMAS );
		pMenu->AppendMenu( MF_STRING, ID_SEARCH_COLUMNS, strText + L"..." );

		m_pCoolMenu = new CCoolMenu();
		m_pCoolMenu->AddMenu( pMenu, TRUE );
		m_pCoolMenu->SetWatermark( Skin.GetWatermark( L"CCoolMenu" ) );

		UINT nCmd = pMenu->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD,
			point.x, point.y, this );

		delete pMenu;
		delete m_pCoolMenu;
		m_pCoolMenu = NULL;

		if ( nCmd == ID_SEARCH_COLUMNS )
		{
			OnSearchColumns();
		}
		else if ( nCmd )
		{
			CSchemaMemberList pColumns;
			CSchemaColumnsDlg::ToggleColumnHelper( m_wndList.m_pSchema,
				&m_wndList.m_pColumns, &pColumns, nCmd, TRUE );
			m_wndList.SelectSchema( m_wndList.m_pSchema, &pColumns );
		}

		return;
	}

	m_tContextMenu = GetTickCount();
	m_bContextMenu = TRUE;
	SendMessage( WM_CONTEXTMENU, pWnd ? (WPARAM)pWnd->GetSafeHwnd() : 0, MAKELONG( point.x, point.y ) );
	m_bContextMenu = FALSE;
}

void CBaseMatchWnd::OnUpdateBlocker(CCmdUI* pCmdUI)
{
	if ( m_pCoolMenu )
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->ContinueRouting();
}

void CBaseMatchWnd::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnMeasureItem( lpMeasureItemStruct );
}

void CBaseMatchWnd::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnDrawItem( lpDrawItemStruct );
}

void CBaseMatchWnd::OnDownload(BOOL bAddToHead)
{
	CList< CMatchFile* > pFiles;
	CList< CQueryHit* > pHits;

	CSingleLock pSingleLock( &m_pMatches->m_pSection, TRUE );

	for ( POSITION pos = m_pMatches->m_pSelectedFiles.GetHeadPosition(); pos; )
	{
		CMatchFile* pFile = m_pMatches->m_pSelectedFiles.GetNext( pos );

		pSingleLock.Unlock();

		switch ( CExistingFileDlg::CheckExisting( pFile ) )
		{
		case CExistingFileDlg::Download:
			pFiles.AddTail( pFile );
			break;
		case CExistingFileDlg::Cancel:
			return;
		//default:
		//	;
		}

		pSingleLock.Lock();
	}

	for ( POSITION pos = m_pMatches->m_pSelectedHits.GetHeadPosition(); pos; )
	{
		CQueryHit* pHit = m_pMatches->m_pSelectedHits.GetNext( pos );

		pSingleLock.Unlock();

		switch ( CExistingFileDlg::CheckExisting( pHit ) )
		{
		case CExistingFileDlg::Download:
			pHits.AddTail( pHit );
			break;
		case CExistingFileDlg::Cancel:
			return;
		//default:
		//	;
		}

		pSingleLock.Lock();
	}

	pSingleLock.Unlock();

	if ( pFiles.IsEmpty() && pHits.IsEmpty() ) return;

	CSyncObject* pSync[2] = { &Network.m_pSection, &Transfers.m_pSection };
	CMultiLock pMultiLock( pSync, 2, TRUE );

	for ( POSITION pos = pFiles.GetHeadPosition(); pos; )
	{
		CMatchFile* pFile = pFiles.GetNext( pos );
		if ( m_pMatches->m_pSelectedFiles.Find( pFile ) != NULL )
			Downloads.Add( pFile, bAddToHead );
	}

	for ( POSITION pos = pHits.GetHeadPosition(); pos; )
	{
		CQueryHit* pHit = pHits.GetNext( pos );
		if ( m_pMatches->m_pSelectedHits.Find( pHit ) != NULL )
		{
			if ( CDownload *pDownload = Downloads.Add( pHit, bAddToHead ) )
			{
				// Send any reviews to the download, so they can be viewed later
				if ( pHit->IsRated() )
					pDownload->AddReview( &pHit->m_pAddress, 2, pHit->m_nRating, pHit->m_sNick, pHit->m_sComments );
			}
		}
	}

	pMultiLock.Unlock();

	m_wndList.Invalidate();

	if ( Settings.Search.SwitchToTransfers && ! m_bContextMenu && GetTickCount() > m_tContextMenu + 5000 )
		GetManager()->Open( RUNTIME_CLASS(CDownloadsWnd) );
}

void CBaseMatchWnd::OnUpdateSearchDownload(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_pMatches->GetSelectedCount() > 0 );
}

void CBaseMatchWnd::OnSearchDownload()
{
	OnDownload( FALSE );
}

void CBaseMatchWnd::OnUpdateSearchDownloadNow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_pMatches->GetSelectedCount() > 0 );
}

void CBaseMatchWnd::OnSearchDownloadNow()
{
	OnDownload( TRUE );
}

void CBaseMatchWnd::OnUpdateSearchURI(CCmdUI* pCmdUI)
{
	const bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;

	INT_PTR nSelected = m_pMatches->m_pSelectedFiles.GetCount() + m_pMatches->m_pSelectedHits.GetCount();

	pCmdUI->Enable( nSelected > 0 );

	pCmdUI->SetText( LoadString( ( nSelected == 1 && ! bShift ) ? IDS_LIBRARY_URI_COPY : IDS_LIBRARY_URI_EXPORT ) );
}

void CBaseMatchWnd::OnSearchURI()
{
	const bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;

	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	INT_PTR nSelected = m_pMatches->m_pSelectedFiles.GetCount() + m_pMatches->m_pSelectedHits.GetCount();

	if ( nSelected < 1 ) return;

	if ( nSelected == 1 && ! bShift )
	{
		CURLCopyDlg dlg;
		if ( CMatchFile* pFile = m_pMatches->GetSelectedFile() )
			dlg.Add( pFile );
		else if ( CQueryHit* pHit = m_pMatches->GetSelectedHit() )
			dlg.Add( pHit );
		pLock.Unlock();
		dlg.DoModal();
	}
	else //if ( nSelected > 0 )
	{
		CURLExportDlg dlg;
		POSITION pos = m_pMatches->m_pSelectedFiles.GetHeadPosition();
		while ( pos )
		{
			CMatchFile* pFile = m_pMatches->m_pSelectedFiles.GetNext( pos );
			dlg.Add( pFile );
		}

		pos = m_pMatches->m_pSelectedHits.GetHeadPosition();
		while ( pos )
		{
			CQueryHit* pHit = m_pMatches->m_pSelectedHits.GetNext( pos );
			dlg.Add( pHit );
		}
		pLock.Unlock();
		dlg.DoModal();
	}
}

void CBaseMatchWnd::OnUpdateSearchChat(CCmdUI* pCmdUI)
{
	CQueryHit* pHit = m_pMatches->GetSelectedHit();
	pCmdUI->Enable( pHit != NULL && pHit->m_bChat && Settings.Community.ChatEnable );
}

void CBaseMatchWnd::OnSearchChat()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	if ( CQueryHit* pHit = m_pMatches->GetSelectedHit() )
	{
		ChatWindows.OpenPrivate( pHit->m_oClientID,
			&pHit->m_pAddress, pHit->m_nPort, pHit->m_bPush == TRI_TRUE,
			pHit->m_nProtocol,
			reinterpret_cast< IN_ADDR* >( &*pHit->m_oClientID.begin() ),
			(WORD)pHit->m_oClientID.begin()[1] );
	}
}

void CBaseMatchWnd::OnUpdateHitMonitorSearch(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Network.IsWellConnected() && m_pMatches->GetSelectedCount() == 1 );
}

void CBaseMatchWnd::OnHitMonitorSearch()
{
	CString strFile;

	if ( CMatchFile* pFile = m_pMatches->GetSelectedFile() )
		strFile = pFile->m_sName;
	else if ( CQueryHit* pHit = m_pMatches->GetSelectedHit() )
		strFile = pHit->m_sName;

	if ( strFile.IsEmpty() ) return;

	CQuerySearchPtr pSearch = new CQuerySearch();
	pSearch->m_sSearch = strFile;

	CNewSearchDlg dlg( NULL, pSearch );
	if ( dlg.DoModal() != IDOK ) return;

	new CSearchWnd( dlg.GetSearch() );
}

void CBaseMatchWnd::OnUpdateSecurityBan(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_pMatches->GetSelectedCount() > 0 );
}

void CBaseMatchWnd::OnSecurityBan()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	if ( POSITION posFiles = m_pMatches->m_pSelectedFiles.GetHeadPosition() )
	{
		while ( posFiles )
		{
			CMatchFile* pFile = m_pMatches->m_pSelectedFiles.GetNext( posFiles );
			pFile->Ban( banForever );
		}
	}
	else if ( POSITION posHits = m_pMatches->m_pSelectedHits.GetHeadPosition() )
	{
		while ( posHits )
		{
			CQueryHit* pHit = m_pMatches->m_pSelectedHits.GetNext( posHits );
			pHit->Ban( banForever );
		}
	}
}

void CBaseMatchWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI)
{
	CQueryHit* pHit = m_pMatches->GetSelectedHit();
	pCmdUI->Enable( pHit != NULL && pHit->m_bBrowseHost );
}

void CBaseMatchWnd::OnBrowseLaunch()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	if ( CQueryHit* pHit = m_pMatches->GetSelectedHit() )
	{
		SOCKADDR_IN pAddress = { AF_INET };
		pAddress.sin_port = htons( pHit->m_nPort );
		pAddress.sin_addr = pHit->m_pAddress;
		new CBrowseHostWnd( pHit->m_nProtocol, &pAddress,
			pHit->m_bPush == TRI_TRUE, pHit->m_oClientID, pHit->m_sNick );
	}
}

void CBaseMatchWnd::OnUpdateLibraryBitprintsWeb(CCmdUI* pCmdUI)
{
	if ( m_pMatches->GetSelectedCount() != 1 || Settings.WebServices.BitprintsWebView.IsEmpty() )
		pCmdUI->Enable( FALSE );
	else if ( CMatchFile* pFile = m_pMatches->GetSelectedFile() )
		pCmdUI->Enable( TRUE );
	else if ( CQueryHit* pHit = m_pMatches->GetSelectedHit() )
		pCmdUI->Enable( TRUE );
}

void CBaseMatchWnd::OnLibraryBitprintsWeb()
{
	if ( ! Settings.WebServices.BitprintsOkay )
	{
		if ( MsgBox( IDS_BITPRINTS_MESSAGE, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
		Settings.WebServices.BitprintsOkay = true;
		Settings.Save();
	}

	if ( m_pMatches->GetSelectedCount() != 1 ) return;

	CString strURN;

	if ( CMatchFile* pFile = m_pMatches->GetSelectedFile() )
	{
		if ( pFile->m_oSHA1 )
			strURN = pFile->m_oSHA1.toString();

		if ( pFile->m_oTiger )
		{
			if ( ! strURN.IsEmpty() )
				strURN	+= L".";
			else
				strURN	= L"tree:tiger:";

			strURN += pFile->m_oTiger.toString();
		}

		if ( pFile->m_oED2K && strURN.IsEmpty() )
			strURN = L"ed2k:" + pFile->m_oED2K.toString();

		if ( pFile->m_oMD5 && strURN.IsEmpty() )
			strURN = L"md5:" + pFile->m_oMD5.toString();
	}
	else if ( CQueryHit* pHit = m_pMatches->GetSelectedHit() )
	{
		if ( pHit->m_oSHA1 )
			strURN = pHit->m_oSHA1.toString();

		if ( pHit->m_oTiger )
		{
			if ( ! strURN.IsEmpty() )
				strURN	+= L".";
			else
				strURN	= L"tree:tiger:";

			strURN += pHit->m_oTiger.toString();
		}

		if ( pHit->m_oED2K && strURN.IsEmpty() )
			strURN = L"ed2k:" + pHit->m_oED2K.toString();

		if ( pHit->m_oMD5 && strURN.IsEmpty() )
			strURN = L"md5:" + pHit->m_oMD5.toString();
	}

	if ( strURN.IsEmpty() ) return;

	CString strURL = Settings.WebServices.BitprintsWebView;
	strURL.Replace( L"(URN)", strURN );
	ShellExecute( GetSafeHwnd(), L"open", strURL, NULL, NULL, SW_SHOWNORMAL );
}

// Defunct Jigle.com P2P Search Code. ToDo: Re-apply
//void CBaseMatchWnd::OnUpdateLibraryJigle(CCmdUI* pCmdUI)
//{
//	if ( m_pMatches->GetSelectedCount() != 1 )
//		pCmdUI->Enable( FALSE );
//	else if ( CMatchFile* pFile = m_pMatches->GetSelectedFile() )
//		pCmdUI->Enable( pFile->m_bED2K );
//	else if ( CQueryHit* pHit = m_pMatches->GetSelectedHit() )
//		pCmdUI->Enable( pHit->m_bED2K );
//}

//void CBaseMatchWnd::OnLibraryJigle()
//{
//	if ( m_pMatches->GetSelectedCount() != 1 ) return;
//
//	CString strED2K;
//
//	if ( CMatchFile* pFile = m_pMatches->GetSelectedFile() )
//	{
//		if ( pFile->m_bED2K )
//			strED2K = CED2K::HashToString( &pFile->m_pED2K );
//	}
//	else if ( CQueryHit* pHit = m_pMatches->GetSelectedHit() )
//	{
//		if ( pHit->m_bED2K )
//			strED2K = CED2K::HashToString( &pHit->m_pED2K );
//	}
//
//	if ( strED2K.IsEmpty() ) return;
//
//	CString strURL;
//	strURL.Format( L"http://jigle.com/search?p=ed2k%%3A%s&v=1", (LPCTSTR)strED2K );
//	ShellExecute( GetSafeHwnd(), L"open", strURL, NULL, NULL, SW_SHOWNORMAL );
//}

void CBaseMatchWnd::OnUpdateSearchForThis(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pCmdUI->Enable( pSearch.CanSearchForThis() );
}

void CBaseMatchWnd::OnSearchForThis()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pLock.Unlock();
	pSearch.RunSearchForThis();
}

void CBaseMatchWnd::OnUpdateSearchForSimilar(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pCmdUI->Enable( pSearch.CanSearchForSimilar() );
}

void CBaseMatchWnd::OnSearchForSimilar()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pLock.Unlock();
	pSearch.RunSearchForSimilar();
}

void CBaseMatchWnd::OnUpdateSearchForArtist(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pCmdUI->Enable( pSearch.CanSearchForArtist() );
}

void CBaseMatchWnd::OnSearchForArtist()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pLock.Unlock();
	pSearch.RunSearchForArtist();
}

void CBaseMatchWnd::OnUpdateSearchForAlbum(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pCmdUI->Enable( pSearch.CanSearchForAlbum() );
}

void CBaseMatchWnd::OnSearchForAlbum()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pLock.Unlock();
	pSearch.RunSearchForAlbum();
}

void CBaseMatchWnd::OnUpdateSearchForSeries(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pCmdUI->Enable( pSearch.CanSearchForSeries() );
}

void CBaseMatchWnd::OnSearchForSeries()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	CRelatedSearch pSearch( m_pMatches->GetSelectedFile( TRUE ) );
	pLock.Unlock();
	pSearch.RunSearchForSeries();
}

void CBaseMatchWnd::OnUpdateSearchFilter(CCmdUI* pCmdUI)
{
	if ( ! m_pMatches->m_sFilter.IsEmpty() )
	{
	//	int nAmp = m_pMatches->m_sFilter.Find( L'&' );
	//	if ( nAmp >= 0 )
	//	{
	//		CString strFilter =	m_pMatches->m_sFilter.Left( nAmp ) + L'&' +
	//							m_pMatches->m_sFilter.Mid( nAmp );
	//		pCmdUI->SetText( L"&Filtered by \"" + strFilter + L"\"..." );
	//	}
	//	else
	//	{
	//		pCmdUI->SetText( L"&Filtered by \"" + m_pMatches->m_sFilter + L"\"..." );
	//	}
		pCmdUI->SetCheck( TRUE );
	}
	else
	{
	//	pCmdUI->SetText( L"&Filter Results..." );
		pCmdUI->SetCheck( FALSE );
	}
}

void CBaseMatchWnd::OnSearchFilter()
{
	CFilterSearchDlg dlg( NULL, m_pMatches );

	if ( dlg.DoModal() == IDOK )
	{
		CWaitCursor pCursor;

		m_pMatches->Filter();
		m_bUpdate = TRUE;
		PostMessage( WM_TIMER, 2 );
	}
}

void CBaseMatchWnd::OnSearchFilterRaw()
{
	OnSearchFilter();
}

void CBaseMatchWnd::OnUpdateSearchFilterRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_pMatches->m_sFilter.IsEmpty() );
}

void CBaseMatchWnd::OnSearchFilterRemove()
{
	CWaitCursor pCursor;

	m_pMatches->m_sFilter.Empty();
	m_pMatches->Filter();
	m_bUpdate = TRUE;
	PostMessage( WM_TIMER, 2 );
}

void CBaseMatchWnd::OnUpdateFilters(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
}

void CBaseMatchWnd::OnFilters(UINT nID)
{
	int nFilter = nID - 3000;
	if ( nFilter < 0 || (DWORD)nFilter > m_pMatches->m_pResultFilters->m_nFilters - 1 ) return;

	m_pMatches->m_bFilterBusy		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterBusy;
	m_pMatches->m_bFilterPush		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterPush;
	m_pMatches->m_bFilterUnstable	= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterUnstable;
	m_pMatches->m_bFilterReject		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterReject;
	m_pMatches->m_bFilterLocal		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterLocal;
	m_pMatches->m_bFilterBogus		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterBogus;
	m_pMatches->m_bFilterDRM		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterDRM;
	m_pMatches->m_bFilterAdult		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterAdult;
	m_pMatches->m_bFilterSuspicious = m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterSuspicious;
	m_pMatches->m_nFilterMinSize	= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_nFilterMinSize;
	m_pMatches->m_nFilterMaxSize	= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_nFilterMaxSize;
	m_pMatches->m_nFilterSources	= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_nFilterSources;
	m_pMatches->m_sFilter			= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_sFilter;

	m_pMatches->Filter();
	Invalidate();
}

void CBaseMatchWnd::OnKillFocusFilter()
{
	CString strFilter;
	m_wndFilter.GetWindowText( strFilter );

	if ( strFilter != m_pMatches->m_sFilter )
	{
		CWaitCursor pCursor;

		m_pMatches->m_sFilter = strFilter;
		m_pMatches->Filter();
		m_bUpdate = TRUE;
		PostMessage( WM_TIMER, 2 );
	}
}

void CBaseMatchWnd::OnToolbarReturn()
{
	if ( GetFocus() == &m_wndFilter )
		OnKillFocusFilter();
}

void CBaseMatchWnd::OnToolbarEscape()
{
	if ( GetFocus() == &m_wndFilter )
	{
		m_wndFilter.SetWindowText( m_pMatches->m_sFilter );
		m_wndList.SetFocus();
	}
}

void CBaseMatchWnd::OnSearchColumns()
{
	CSchemaColumnsDlg dlg;

	dlg.m_pSchema = m_wndList.m_pSchema;
	dlg.m_pColumns.AddTail( &m_wndList.m_pColumns );

	if ( dlg.DoModal() == IDOK )
		m_wndList.SelectSchema( dlg.m_pSchema, &dlg.m_pColumns );
}

BOOL CBaseMatchWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_wndList.PostMessage( WM_MOUSEWHEEL, MAKELONG( nFlags, zDelta ), MAKELONG( pt.x, pt.y ) );
	return TRUE;
}

void CBaseMatchWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	if ( m_bPanelMode && bActivate )
	{
		CRect rcParent, rcChild;
		GetWindowRect( &rcChild );
		GetParent()->GetClientRect( &rcParent );

		if ( rcParent.Width() != rcChild.Width() || rcParent.Height() != rcChild.Height() )
		{
			SetWindowPos( NULL, 0, 0, rcParent.Width(), rcParent.Height(), SWP_FRAMECHANGED );
			RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME );
		}
	}
	else if ( ! bActivate )
	{
		m_bBMWActive = FALSE;
		m_pMatches->ClearNew();
		m_wndList.Invalidate();
	}

	m_wndList.EnableTips( bActivate );

	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );
}

void CBaseMatchWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( m_wndFilter.m_hWnd == NULL ) return;

//	const DWORD tNow = GetTickCount();

	if ( nIDEvent == 1 )
	{
		if ( GetFocus() != &m_wndFilter )
		{
			CString strFilter;
			m_wndFilter.GetWindowText( strFilter );

			if ( strFilter != m_pMatches->m_sFilter )
				m_wndFilter.SetWindowText( m_pMatches->m_sFilter );
		}

		BOOL bActive = ( GetMainWnd()->m_pWindows.GetActive() == this );	// Was MDIGetActive()

		if ( bActive )
		{
			if ( HWND hFore = ::GetForegroundWindow() )
			{
				DWORD nProcessID;
				GetWindowThreadProcessId( hFore, &nProcessID );
				bActive &= ( nProcessID == GetCurrentProcessId() );
			}
		}

		if ( m_bBMWActive != bActive )
		{
			m_bBMWActive = bActive;

			if ( ! m_bBMWActive )
			{
				m_pMatches->ClearNew();
				m_wndList.Invalidate();
			}
		}

		if ( m_pMatches->m_nFilteredFiles == 0 )
			UpdateMessages( FALSE );
	}
	else if ( ( nIDEvent == 2 && m_bUpdate ) || nIDEvent == 7 )
	{
		CSingleLock pLock( &m_pMatches->m_pSection );
		if ( pLock.Lock( 50 ) )
		{
			m_bUpdate = FALSE;
			m_wndList.Update();
			UpdateMessages();

			if ( m_pMatches->m_nFilteredFiles != m_nCacheFiles )
			{
				m_nCacheFiles = m_pMatches->m_nFilteredFiles;
				SetAlert();
			}
		}
		else
		{
			m_bUpdate = TRUE;
			PostMessage( WM_TIMER, 2 );
		}
	}

	// Lazy save
	if ( m_tModify && static_cast< DWORD >( time( NULL ) ) - m_tModify > 10 )
	{
		m_tModify = 0;

		if ( IsKindOf( RUNTIME_CLASS(CSearchWnd) ) )
			GetManager()->SaveSearchWindows();
		else if ( IsKindOf( RUNTIME_CLASS(CBrowseHostWnd) ) )
			GetManager()->SaveBrowseHostWindows();
	}
}

void CBaseMatchWnd::SanityCheck()
{
	if ( ! Settings.Search.SanityCheck )
		return;

	m_wndList.DestructiveUpdate();

	CQuickLock pLock( m_pMatches->m_pSection );

	m_pMatches->SanityCheck();

	m_pMatches->Filter();
	m_bUpdate = TRUE;
	PostMessage( WM_TIMER, 2 );
}

void CBaseMatchWnd::UpdateMessages(BOOL /*bActive*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CBaseMatchWnd generic view

HRESULT CBaseMatchWnd::GetGenericView(IGenericView** ppView)
{
	if ( m_pMatches == NULL ) return S_FALSE;
	CRuntimeClass* pClass = GetRuntimeClass();
	*ppView = CMatchListView::Attach( CString( pClass->m_lpszClassName ), m_pMatches );
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CBaseMatchWnd serialize

void CBaseMatchWnd::Serialize(CArchive& ar)
{
	m_tModify = 0;
	CString strSchema;

	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	if ( ar.IsStoring() )
	{
		if ( m_pMatches->m_pSchema )
			ar << m_pMatches->m_pSchema->GetURI();
		else
			ar << strSchema;
	}
	else // Loading
	{
		m_bBMWActive = TRUE;
		m_bPaused = TRUE;
		m_bUpdate = TRUE;
		m_nCacheFiles = 0;

		ar >> strSchema;
		if ( CSchemaPtr pSchema = SchemaCache.Get( strSchema ) )
		{
			CSchemaMemberList pColumns;
			CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
			m_wndList.SelectSchema( pSchema, &pColumns );
		}
	}

	try
	{
		m_pMatches->Serialize( ar );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		m_pMatches->Clear();
	}
}
