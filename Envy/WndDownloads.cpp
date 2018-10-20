//
// WndDownloads.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2016
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
#include "WndDownloads.h"
//#include "CtrlDownloads.h"		// Header
//#include "CtrlDownloadTabBar.h"	// Header
//#include "WndPanel.h"				// Header

#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadTask.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferED2K.h"
#include "DownloadGroups.h"
#include "EDClient.h"
#include "EDClients.h"

#include "Library.h"
#include "LibraryMaps.h"
#include "MatchListView.h"
#include "SharedFile.h"
#include "FileExecutor.h"
#include "EnvyURL.h"
#include "ChatWindows.h"

#include "Skin.h"
#include "WindowManager.h"
#include "WndUploads.h"
#include "WndBrowseHost.h"
#include "DlgDownload.h"
#include "DlgDownloadReviews.h"
#include "DlgSettingsManager.h"
#include "DlgDeleteFile.h"
#include "DlgFilePropertiesSheet.h"
#include "DlgDownloadSheet.h"
#include "DlgURLCopy.h"
#include "DlgURLExport.h"
#include "DlgHelp.h"
#include "Network.h"

#include "FragmentedFile.h" // Merge File Command

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//#define GROUPSBAR 24	// Settings.Skin.GroupsbarHeight


IMPLEMENT_SERIAL(CDownloadsWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CDownloadsWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_MDIACTIVATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SETCURSOR()
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_RESUME, OnUpdateDownloadsResume)
	ON_COMMAND(ID_DOWNLOADS_RESUME, OnDownloadsResume)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_PAUSE, OnUpdateDownloadsPause)
	ON_COMMAND(ID_DOWNLOADS_PAUSE, OnDownloadsPause)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_CLEAR, OnUpdateDownloadsClear)
	ON_COMMAND(ID_DOWNLOADS_CLEAR, OnDownloadsClear)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_LAUNCH, OnUpdateDownloadsLaunch)
	ON_COMMAND(ID_DOWNLOADS_LAUNCH, OnDownloadsLaunch)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_VIEW_REVIEWS, OnUpdateDownloadsViewReviews)
	ON_COMMAND(ID_DOWNLOADS_VIEW_REVIEWS, OnDownloadsViewReviews)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_REMOTE_PREVIEW, OnUpdateDownloadsRemotePreview)
	ON_COMMAND(ID_DOWNLOADS_REMOTE_PREVIEW, OnDownloadsRemotePreview)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_SOURCES, OnUpdateDownloadsSources)
	ON_COMMAND(ID_DOWNLOADS_SOURCES, OnDownloadsSources)
	ON_COMMAND(ID_DOWNLOADS_CLEAR_COMPLETED, OnDownloadsClearCompleted)
	ON_COMMAND(ID_DOWNLOADS_CLEAR_PAUSED, OnDownloadsClearPaused)
	ON_UPDATE_COMMAND_UI(ID_TRANSFERS_DISCONNECT, OnUpdateTransfersDisconnect)
	ON_COMMAND(ID_TRANSFERS_DISCONNECT, OnTransfersDisconnect)
	ON_UPDATE_COMMAND_UI(ID_TRANSFERS_FORGET, OnUpdateTransfersForget)
	ON_COMMAND(ID_TRANSFERS_FORGET, OnTransfersForget)
	ON_UPDATE_COMMAND_UI(ID_TRANSFERS_CHAT, OnUpdateTransfersChat)
	ON_COMMAND(ID_TRANSFERS_CHAT, OnTransfersChat)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MERGE, OnUpdateDownloadsMergeLocal)
	ON_COMMAND(ID_DOWNLOADS_MERGE, OnDownloadsMergeLocal)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_ADD_SOURCE, OnUpdateDownloadsAddSource)
	ON_COMMAND(ID_DOWNLOADS_ADD_SOURCE, OnDownloadsAddSource)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_BOOST, OnUpdateDownloadsBoost)
	ON_COMMAND(ID_DOWNLOADS_BOOST, OnDownloadsBoost)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_ENQUEUE, OnUpdateDownloadsEnqueue)
	ON_COMMAND(ID_DOWNLOADS_ENQUEUE, OnDownloadsEnqueue)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_AUTO_CLEAR, OnUpdateDownloadsAutoClear)
	ON_COMMAND(ID_DOWNLOADS_AUTO_CLEAR, OnDownloadsAutoClear)
	ON_UPDATE_COMMAND_UI(ID_TRANSFERS_CONNECT, OnUpdateTransfersConnect)
	ON_COMMAND(ID_TRANSFERS_CONNECT, OnTransfersConnect)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_SHOW_SOURCES, OnUpdateDownloadsShowSources)
	ON_COMMAND(ID_DOWNLOADS_SHOW_SOURCES, OnDownloadsShowSources)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MONITOR, OnUpdateDownloadsMonitor)
	ON_COMMAND(ID_DOWNLOADS_MONITOR, OnDownloadsMonitor)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FOLDER, OnUpdateDownloadsFolder)
	ON_COMMAND(ID_DOWNLOADS_FOLDER, OnDownloadsFolder)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_LAUNCH_COPY, OnUpdateDownloadsLaunchCopy)
	ON_COMMAND(ID_DOWNLOADS_LAUNCH_COPY, OnDownloadsLaunchCopy)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILE_DELETE, OnUpdateDownloadsFileDelete)
	ON_COMMAND(ID_DOWNLOADS_FILE_DELETE, OnDownloadsFileDelete)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_RATE, OnUpdateDownloadsRate)
	ON_COMMAND(ID_DOWNLOADS_RATE, OnDownloadsRate)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MOVE_UP, OnUpdateDownloadsMove)
	ON_COMMAND(ID_DOWNLOADS_MOVE_UP, OnDownloadsMoveUp)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MOVE_DOWN, OnUpdateDownloadsMove)
	ON_COMMAND(ID_DOWNLOADS_MOVE_DOWN, OnDownloadsMoveDown)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MOVE_TOP, OnUpdateDownloadsMove)
	ON_COMMAND(ID_DOWNLOADS_MOVE_TOP, OnDownloadsMoveTop)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MOVE_BOTTOM, OnUpdateDownloadsMove)
	ON_COMMAND(ID_DOWNLOADS_MOVE_BOTTOM, OnDownloadsMoveBottom)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_ALL, OnUpdateDownloadsFilterAll)
	ON_COMMAND(ID_DOWNLOADS_FILTER_ALL, OnDownloadsFilterAll)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_ACTIVE, OnUpdateDownloadsFilterActive)
	ON_COMMAND(ID_DOWNLOADS_FILTER_ACTIVE, OnDownloadsFilterActive)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_PAUSED, OnUpdateDownloadsFilterPaused)
	ON_COMMAND(ID_DOWNLOADS_FILTER_PAUSED, OnDownloadsFilterPaused)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_QUEUED, OnUpdateDownloadsFilterQueued)
	ON_COMMAND(ID_DOWNLOADS_FILTER_QUEUED, OnDownloadsFilterQueued)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_SOURCES, OnUpdateDownloadsFilterSources)
	ON_COMMAND(ID_DOWNLOADS_FILTER_SOURCES, OnDownloadsFilterSources)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_SEEDS, OnUpdateDownloadsFilterSeeds)
	ON_COMMAND(ID_DOWNLOADS_FILTER_SEEDS, OnDownloadsFilterSeeds)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_LAUNCH_COMPLETE, OnUpdateDownloadsLaunchComplete)
	ON_COMMAND(ID_DOWNLOADS_LAUNCH_COMPLETE, OnDownloadsLaunchComplete)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_SHARE, OnUpdateDownloadsShare)
	ON_COMMAND(ID_DOWNLOADS_SHARE, OnDownloadsShare)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_URI, OnUpdateDownloadsURI)
	ON_COMMAND(ID_DOWNLOADS_URI, OnDownloadsURI)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_SHOW, OnUpdateDownloadGroupShow)
	ON_COMMAND(ID_DOWNLOAD_GROUP_SHOW, OnDownloadGroupShow)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_CLEAR_INCOMPLETE, OnUpdateDownloadsClearIncomplete)
	ON_COMMAND(ID_DOWNLOADS_CLEAR_INCOMPLETE, OnDownloadsClearIncomplete)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_CLEAR_COMPLETE, OnUpdateDownloadsClearComplete)
	ON_COMMAND(ID_DOWNLOADS_CLEAR_COMPLETE, OnDownloadsClearComplete)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_EDIT, OnUpdateDownloadsEdit)
	ON_COMMAND(ID_DOWNLOADS_EDIT, OnDownloadsEdit)
	ON_COMMAND(ID_DOWNLOADS_HELP, OnDownloadsHelp)
	ON_COMMAND(ID_DOWNLOADS_SETTINGS, OnDownloadsSettings)
	ON_COMMAND(ID_DOWNLOADS_FILTER_MENU, OnDownloadsFilterMenu)
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadsWnd construction

CDownloadsWnd::CDownloadsWnd()
	: CPanelWnd( TRUE, TRUE )
	, m_bMouseCaptured( false )
{
	Create( IDR_DOWNLOADSFRAME );
}

CDownloadsWnd::~CDownloadsWnd()
{
}

HRESULT CDownloadsWnd::GetGenericView(IGenericView** ppView)
{
	*ppView = NULL;
	if ( IsWindow( m_wndDownloads.GetSafeHwnd() ) )
		*ppView = CMatchListView::Attach( L"CDownloadsWnd", &Downloads );

	return *ppView ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadsWnd system message handlers

int CDownloadsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndTabBar.Create( this, WS_CHILD|CBRS_TOP, AFX_IDW_TOOLBAR+1 ) ) return -1;
	m_wndTabBar.SetBarStyle( m_wndTabBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_BOTTOM );
	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndToolBar.SetSyncObject( &Transfers.m_pSection );

	m_wndDownloads.Create( this, IDC_DOWNLOADS );

	m_wndDownloads.ModifyStyleEx( 0, WS_EX_COMPOSITED );	// Stop flicker XP+, CPU intensive

	LoadState( NULL, TRUE );

	SetTimer( 2, Settings.Interface.RefreshRateText, NULL );
	PostMessage( WM_TIMER, 2 );

	SetTimer( 4, 10000, NULL );
	PostMessage( WM_TIMER, 4 );

	m_pDragList		= NULL;
	m_pDragImage	= NULL;
	m_tSel			= 0;

#ifdef XPSUPPORT
	m_hCursMove		= AfxGetApp()->LoadCursor( theApp.m_bIsWinXP ? IDR_MOVE_XP : IDR_MOVE );
	m_hCursCopy		= AfxGetApp()->LoadCursor( theApp.m_bIsWinXP ? IDR_COPY_XP : IDR_COPY );
#else // x64 Vista+
	m_hCursMove		= AfxGetApp()->LoadCursor( IDR_MOVE );
	m_hCursCopy		= AfxGetApp()->LoadCursor( IDR_COPY );
#endif

	m_nMoreSourcesLimiter	= 8;
	m_tMoreSourcesTimer		= 0;
	m_tLastUpdate			= 0;

	return 0;
}

void CDownloadsWnd::OnDestroy()
{
	CancelDrag();
	KillTimer( 6 );
	KillTimer( 4 );
	KillTimer( 2 );
	SaveState();
	CPanelWnd::OnDestroy();
}

void CDownloadsWnd::OnSkinChange()
{
	CRect rc;
	GetClientRect( &rc );
	OnSize( 0, rc.Width(), rc.Height() );

	CPanelWnd::OnSkinChange();
	Skin.Translate( L"CDownloadCtrl", &m_wndDownloads.m_wndHeader);
	Skin.CreateToolBar( L"CDownloadsWnd", &m_wndToolBar );
	m_wndDownloads.OnSkinChange();
	m_wndTabBar.OnSkinChange();
}

void CDownloadsWnd::Update()
{
	if ( ! IsWindow( m_hWnd ) ) return;

	if ( m_bMouseCaptured )
	{
		ClipCursor( NULL );
		ReleaseCapture();
		m_bMouseCaptured = false;
	}

	int nCookie = 0;

	m_tLastUpdate = GetTickCount();

	if ( Settings.General.GUIMode != GUI_BASIC && Settings.Downloads.ShowGroups )
	{
		nCookie = (int)m_tLastUpdate;
		m_wndTabBar.Update( nCookie );
	}

	m_wndDownloads.Update( nCookie );
}

BOOL CDownloadsWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndToolBar.m_hWnd )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}

	if ( m_wndTabBar.m_hWnd )
	{
		if ( m_wndTabBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}

	return CPanelWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CDownloadsWnd::OnSize(UINT nType, int cx, int cy)
{
	CRect rc( 0, 0, cx, cy - Settings.Skin.ToolbarHeight );

	BOOL bTabs = Settings.Downloads.ShowGroups && ( Settings.General.GUIMode != GUI_BASIC );

	if ( bTabs )
		rc.top += Settings.Skin.GroupsbarHeight;
	else
		m_wndTabBar.ShowWindow( SW_HIDE );

	HDWP hPos = BeginDeferWindowPos( bTabs ? 3 : 2 );
	DeferWindowPos( hPos, m_wndDownloads, NULL,
		0, rc.top, cx, rc.Height(), SWP_SHOWWINDOW|SWP_NOZORDER );
	DeferWindowPos( hPos, m_wndToolBar, NULL,
		0, rc.bottom, cx, Settings.Skin.ToolbarHeight, SWP_SHOWWINDOW|SWP_NOZORDER );
	if ( bTabs ) DeferWindowPos( hPos, m_wndTabBar, NULL,
		0, 0, cx, Settings.Skin.GroupsbarHeight, SWP_SHOWWINDOW|SWP_NOZORDER );
	EndDeferWindowPos( hPos );

	CPanelWnd::OnSize( nType, cx, cy );
}

void CDownloadsWnd::OnTimer(UINT_PTR nIDEvent)
{
	// Reset Selection Timer event (posted by ctrldownloads)
	if ( nIDEvent == 5 )
		m_tSel = 0;

	// Window Update event (2 second timer)
	if ( nIDEvent == 2 && m_pDragList == NULL )
	{
		// ToDo: Make intelligent, only lock if previews needed
		CSingleLock pLock( &Transfers.m_pSection );
		if ( pLock.Lock( 100 ) )
		{
			for ( POSITION pos = Downloads.GetIterator(); pos; )
			{
				CDownload* pDownload = Downloads.GetNext( pos );
				if ( ! pDownload->GotPreview() || ! pDownload->m_bWaitingPreview )
					continue;

				pDownload->m_bWaitingPreview = FALSE;
				CString strPreview = pDownload->m_sPath + L".png";
				pLock.Unlock();

				CFileExecutor::Execute( strPreview );

				break;	// Show next preview on next update
			}
			pLock.Unlock();
		}

		// Refresh the window if visible, or hasn't been updated in 10 seconds
		if ( ( IsWindowVisible() && IsActive( FALSE ) ) || ( GetTickCount() > m_tLastUpdate + 10*1000 ) )
			Update();
	}

	// Clear Completed event (10 second timer)
	if ( nIDEvent == 4 )
	{
		const DWORD tNow = GetTickCount();

		if ( tNow > m_tMoreSourcesTimer + 6*60*1000 )	// 6 minute limit
		{
			if ( m_nMoreSourcesLimiter < 15 )
				m_nMoreSourcesLimiter++;

			m_tMoreSourcesTimer = tNow;
		}

		CSingleLock pLock( &Transfers.m_pSection );
		if ( ! pLock.Lock( 100 ) ) return;

		// Purge old failed sources ( X-NAlt mesh )
		for ( POSITION pos = Downloads.GetIterator(); pos; )
		{
			CDownload* pDownload = Downloads.GetNext( pos );
			pDownload->ExpireFailedSources();
		}

		// If some kind of auto-clear is active
		if ( Settings.Downloads.AutoClear || Settings.BitTorrent.AutoClear )
		{
			// Loop through all downloads
			for ( POSITION pos = Downloads.GetIterator(); pos; )
			{
				CDownload* pDownload = Downloads.GetNext( pos );

				if ( pDownload->IsCompleted() &&				// If the download has completed
					 pDownload->IsPreviewVisible() == FALSE &&	// And isn't previewing
					 tNow > pDownload->m_tCompleted + Settings.Downloads.ClearDelay )
				{
					// We might want to clear this download
					if ( Settings.BitTorrent.AutoClear && pDownload->IsTorrent() == true )		// It's a torrent and may be ratio-limited
					{
						// If we're seeding and have reached the required ratio
						if ( pDownload->IsSeeding() && ( Settings.BitTorrent.ClearRatio < pDownload->GetRatio() ) )
							pDownload->Remove();
					}
					else // A normal download
					{
						// Check the general auto clear setting
						if ( Settings.Downloads.AutoClear )
							pDownload->Remove();
					}
				}
			}
		}
	}
}

void CDownloadsWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );

	if ( bActivate )
	{
		Update();
		m_wndDownloads.SetFocus();
	}
}

void CDownloadsWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	AfxGetMainWnd()->SendMessage( WM_MEASUREITEM, nIDCtl, (LPARAM)lpMeasureItemStruct );
}

void CDownloadsWnd::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	AfxGetMainWnd()->SendMessage( WM_DRAWITEM, nIDCtl, (LPARAM)lpDrawItemStruct );
}

void CDownloadsWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
	{
		m_wndDownloads.ClientToScreen( &point );

		Prepare();
		if ( m_bSelDownload )
		{
			if ( m_bSelCompleted && m_bSelTorrent )
				Skin.TrackPopupMenu( L"CDownloadsWnd.Seeding", point, ID_DOWNLOADS_LAUNCH_COMPLETE );
			else if ( m_bSelCompleted )
				Skin.TrackPopupMenu( L"CDownloadsWnd.Completed", point, ID_DOWNLOADS_LAUNCH_COMPLETE );
			else
				Skin.TrackPopupMenu( L"CDownloadsWnd.Download", point, ID_DOWNLOADS_LAUNCH_COPY );
			return;
		}
		Skin.TrackPopupMenu( L"CDownloadsWnd.Default", point, ID_DOWNLOADS_HELP );
		return;
	}

	CPoint ptLocal( point );
	m_wndDownloads.ScreenToClient( &ptLocal );
	m_tSel = 0;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CDownload* pDownload;
	CDownloadSource* pSource;

	if ( m_wndDownloads.HitTest( ptLocal, &pDownload, &pSource, NULL, NULL ) )
	{
		if ( pSource )
		{
			pLock.Unlock();
			Skin.TrackPopupMenu( L"CDownloadsWnd.Source", point, ID_TRANSFERS_CONNECT );
			return;
		}
		if ( pDownload )
		{
			if ( pDownload->IsSeeding() )
			{
				pLock.Unlock();
				Skin.TrackPopupMenu( L"CDownloadsWnd.Seeding", point, ID_DOWNLOADS_LAUNCH_COMPLETE );
				return;
			}
			if ( pDownload->IsCompleted() )
			{
				pLock.Unlock();
				Skin.TrackPopupMenu( L"CDownloadsWnd.Completed", point, ID_DOWNLOADS_LAUNCH_COMPLETE );
				return;
			}

			pLock.Unlock();
			Skin.TrackPopupMenu( L"CDownloadsWnd.Download", point, ID_DOWNLOADS_LAUNCH_COPY );
			return;
		}
	}

	pLock.Unlock();
	Skin.TrackPopupMenu( L"CDownloadsWnd.Default", point, ID_DOWNLOADS_HELP );
}

BOOL CDownloadsWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		switch ( pMsg->wParam )
		{
		//case VK_SHIFT:
		//case VK_CONTROL:
		//	return TRUE;	// Skip async keys?

		case VK_TAB:		// Toggle window focus to Uploads
			GetManager()->Open( RUNTIME_CLASS(CUploadsWnd) );
			return TRUE;

		case VK_ESCAPE:
			if ( ! m_pDragList )	// Clear selections, when not dragging
			{
				m_wndDownloads.DeselectAll();
				Update();
			}
			break;

		case VK_UP:
			if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0 )
			{
				OnDownloadsMoveUp();
				return TRUE;
			}
			break;
		case VK_DOWN:
			if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0 )
			{
				OnDownloadsMoveDown();
				return TRUE;
			}
			break;
		case VK_HOME:
			if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0 )
			{
				OnDownloadsMoveTop();
				return TRUE;
			}
			break;
		case VK_END:
			if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0 )
			{
				OnDownloadsMoveBottom();
				return TRUE;
			}
			break;
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}

void CDownloadsWnd::Select(CDownload* pSelect)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;
	if ( ! pSelect ) return;	// Folder?

	DWORD nIndex = 0;

	// Set "All" SuperGroup if needed
	if ( m_wndDownloads.m_nGroupCookie != 0 && m_wndDownloads.m_nGroupCookie != pSelect->m_nGroupCookie )
	{
		m_wndTabBar.Select();
		m_wndDownloads.m_nGroupCookie = 0;
	}

	// Find file for Download Monitor "Show" or right-click
	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload == pSelect )
		{
			m_wndDownloads.SelectTo( nIndex );
			pDownload->m_bSelected = TRUE;
			nIndex = UINT_MAX;	// Stop counting at MAXDWORD
		}
		else
		{
			pDownload->m_bSelected = FALSE;
			if ( nIndex < UINT_MAX &&
				 ( m_wndDownloads.m_nGroupCookie == 0 || m_wndDownloads.m_nGroupCookie == pDownload->m_nGroupCookie ) &&
				 ! m_wndDownloads.IsFiltered( pDownload ) )
				nIndex++;
		}

		if ( pDownload->m_bExpanded )
		{
			for ( POSITION posSource = pDownload->GetIterator(); posSource; )
			{
				CDownloadSource* pSource = pDownload->GetNext( posSource );
				pSource->m_bSelected = FALSE;
				if ( nIndex < UINT_MAX && ( Settings.Downloads.ShowSources || ! pSource->IsIdle() ) )
					nIndex++;
			}
		}
	}

	pLock.Unlock();

	m_tSel = 0;
	Invalidate();
}

void CDownloadsWnd::Prepare()
{
	const DWORD tNow = GetTickCount();
	if ( tNow < m_tSel + Settings.Interface.RefreshRateUI ) return;

	m_nSelectedDownloads = m_nSelectedSources = 0;
	m_bSelAny = m_bSelDownload = m_bSelSource = m_bSelTrying = m_bSelPaused = FALSE;
	m_bSelNotPausedOrMoving = m_bSelNoPreview = m_bSelNotCompleteAndNoPreview = FALSE;
	m_bSelCompletedAndNoPreview = m_bSelStartedAndNotMoving = m_bSelCompleted = FALSE;
	m_bSelNotMoving = m_bSelBoostable = FALSE;
	m_bSelIdleSource = m_bSelActiveSource = m_bSelMoreSourcesOK = FALSE;
	m_bSelTorrent = m_bSelSeeding = FALSE;
	m_bSelBrowse = m_bSelChat = m_bSelShareState = FALSE;
	m_bSelSourceAcceptConnections = m_bSelSourceExtended = m_bSelHasReviews = FALSE;
	m_bSelRemotePreviewCapable = FALSE;
	m_bSelShareConsistent = TRUE;
	m_bSelHasSize = m_bSelHasHash = m_bSelHasName = FALSE;

	m_bConnectOkay = FALSE;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( Settings.Interface.RefreshRateUI ) )
		return;

	BOOL bFirstShare = TRUE;
	BOOL bPreviewDone = FALSE;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload->m_bSelected )
		{
			m_nSelectedDownloads++;
			m_bSelAny = TRUE;
			m_bSelDownload = TRUE;
			if ( pDownload->IsCompleted() )
				m_bSelCompleted = TRUE;
			if ( ! pDownload->IsMoving() )
				m_bSelNotMoving = TRUE;
			if ( pDownload->IsTrying() )
				m_bSelTrying = TRUE;
			if ( ! pDownload->IsBoosted() )
				m_bSelBoostable = TRUE;
			if ( pDownload->IsTorrent() )
			{
				m_bSelTorrent = TRUE;
				if ( pDownload->IsSeeding() )
					m_bSelSeeding = TRUE;
			}
			if ( pDownload->IsPaused() )
				m_bSelPaused = TRUE;
			else if ( ! pDownload->IsMoving() )
			{
				m_bSelNotPausedOrMoving = TRUE;
				if ( pDownload->IsTrying() && pDownload->FindSourcesAllowed( tNow ) )
					m_bSelMoreSourcesOK = TRUE;
			}

			if ( ! pDownload->IsPreviewVisible() )
			{
				m_bSelNoPreview = TRUE;
				if ( pDownload->IsCompleted() )
					m_bSelCompletedAndNoPreview = TRUE;
				else
					m_bSelNotCompleteAndNoPreview = TRUE;
			}
			if ( pDownload->IsStarted() && ! pDownload->IsMoving() )
				m_bSelStartedAndNotMoving = TRUE;
			if ( bFirstShare )
			{
				m_bSelShareState = pDownload->IsShared();
				bFirstShare = FALSE;
			}
			else if ( ( m_bSelShareState != 0 ) != pDownload->IsShared() )
			{
				m_bSelShareState = FALSE;
				m_bSelShareConsistent = FALSE;
			}

			if ( pDownload->GetReviewCount() )
				m_bSelHasReviews = TRUE;
			if ( pDownload->HasHash() )
				m_bSelHasHash = TRUE;
			if ( pDownload->m_nSize && pDownload->m_nSize < SIZE_UNKNOWN )
				m_bSelHasSize = TRUE;
			if ( ! pDownload->m_sName.IsEmpty() )
				m_bSelHasName = TRUE;
		}

		pDownload->m_bRemotePreviewCapable = FALSE;

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_bSelected )
			{
				m_nSelectedSources++;
				m_bSelAny = TRUE;
				m_bSelSource = TRUE;
				m_bSelSourceExtended = pSource->m_bClientExtended;
				if ( pSource->IsIdle() )
					m_bSelIdleSource = TRUE;
				else
					m_bSelActiveSource = TRUE;
				if ( pSource->m_bClientExtended || pSource->m_nProtocol == PROTOCOL_ED2K )
				{
					m_bSelBrowse = TRUE;
					m_bSelChat = TRUE;
				}
				else if ( pSource->m_nProtocol == PROTOCOL_DC )
				{
					// ToDo: Implement DC++ private chat
					// m_bSelChat = TRUE;
					m_bSelBrowse = TRUE;
				}
				if ( ! pSource->m_bPushOnly )
					m_bSelSourceAcceptConnections = TRUE;
			}

			// Check if we could get remote previews (only from the connected sources for the efficiency)
			if ( ! bPreviewDone &&
				   pDownload->m_bSelected &&
				 ! pDownload->m_bRemotePreviewCapable &&
				 ! pSource->m_bPreviewRequestSent &&
				   pSource->IsOnline() )
			{
				if ( pSource->IsPreviewCapable() )
					pDownload->m_bRemotePreviewCapable = TRUE;
			}
		}

		if ( pDownload->m_bSelected )
		{
			m_bSelRemotePreviewCapable = pDownload->m_bRemotePreviewCapable || pDownload->GotPreview();
			bPreviewDone = TRUE;
		}
	}

	if ( ! Settings.Connection.RequireForTransfers || Network.IsConnected() )
		m_bConnectOkay = TRUE;

	m_tSel = GetTickCount();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadsWnd command handlers

void CDownloadsWnd::OnUpdateDownloadsResume(CCmdUI* pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelPaused || ( m_bSelDownload && ! m_bSelTrying && ! m_bSelCompleted ) );
	pCmdUI->Enable(    m_bSelPaused || ( m_bSelDownload && ! m_bSelTrying && ! m_bSelCompleted ) );
}

void CDownloadsWnd::OnDownloadsResume()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload->m_bSelected )
			pDownload->Resume();
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsPause(CCmdUI* pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( ( m_bSelSeeding && ! m_bSelPaused ) || ! m_bSelCompletedAndNoPreview && ( m_bSelNotPausedOrMoving || ! m_bSelDownload ) );
	pCmdUI->Enable(    ( m_bSelSeeding && ! m_bSelPaused ) || ! m_bSelCompletedAndNoPreview && m_bSelNotPausedOrMoving );
}

void CDownloadsWnd::OnDownloadsPause()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		if ( CDownload* pDownload = Downloads.GetNext( pos ) )
		{
			if ( pDownload->m_bSelected && ! pDownload->IsPaused() && ! pDownload->IsMoving() )
				pDownload->Pause( TRUE );
		}
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsClear(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelNoPreview || m_bSelSource );
}

void CDownloadsWnd::OnDownloadsClear()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	// Delete-key handling only

	CList<CDownload*> pList;
	m_wndDownloads.GetSelectedList( pList, TRUE );

	pLock.Unlock();

	// If no downloads selected then process selected sources
	if ( pList.IsEmpty() )
	{
		OnTransfersForget();
		return;
	}

	m_wndDownloads.Update();

	while ( ! pList.IsEmpty() )
	{
		if ( ! SafeLock( pLock ) ) break;

		CDownload* pDownload = pList.RemoveHead();

		if ( Downloads.Check( pDownload ) && ! pDownload->IsTasking() )
		{
			if ( pDownload->IsPreviewVisible() )
			{
				// Can't
			}
			else if ( pDownload->IsCompleted() )
			{
				pDownload->Remove();
			}
			else if ( pDownload->IsStarted() )
			{
				CDeleteFileDlg dlg;
				dlg.m_sName = pDownload->m_sName;
				const BOOL bShared = pDownload->IsShared() == true;

				pLock.Unlock();
				if ( dlg.DoModal() != IDOK )
				{
					pDownload->m_bClearing = FALSE;
					while ( ! pList.IsEmpty() )
					{
						pDownload = pList.RemoveHead();
						pDownload->m_bClearing = FALSE;
					}
					break;
				}
				pLock.Lock();

				if ( Downloads.Check( pDownload ) && ! pDownload->IsTasking() )
				{
					dlg.Create( pDownload, bShared );
					pDownload->Remove();
				}
			}
			else
			{
				pDownload->Remove();
			}
		}
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsClearIncomplete(CCmdUI *pCmdUI)
{
	Prepare();

	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelNotCompleteAndNoPreview || ! m_bSelDownload );
	pCmdUI->Enable( m_bSelNotCompleteAndNoPreview );
}

void CDownloadsWnd::OnDownloadsClearIncomplete()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CDownload*> pList;
	if ( ! m_wndDownloads.GetSelectedList( pList, TRUE ) )
		return;

	pLock.Unlock();				// Break if above takes too long
	m_wndDownloads.Update();	// Show Clearing status

	while ( ! pList.IsEmpty() )
	{
		if ( ! SafeLock( pLock ) ) continue;

		CDownload* pDownload = pList.RemoveHead();
		if ( ! Downloads.Check( pDownload ) )
			continue;

		if ( pDownload->IsCompleted() || pDownload->IsSeeding() || pDownload->IsPreviewVisible() )
			continue;

		if ( pDownload->IsStarted() )
		{
			CDeleteFileDlg dlg;
			dlg.m_sName = pDownload->m_sName;
			const BOOL bShared = pDownload->IsShared();

			pLock.Unlock();
			if ( dlg.DoModal() != IDOK )
			{
				pDownload->m_bClearing = FALSE;
				while ( ! pList.IsEmpty() )
				{
					pDownload = pList.RemoveHead();
					pDownload->m_bClearing = FALSE;
				}
				break;
			}
			pLock.Lock();

			if ( Downloads.Check( pDownload ) )
				dlg.Create( pDownload, bShared );
		}

		if ( Downloads.Check( pDownload ) && ! pDownload->IsTasking() )
			pDownload->Remove();
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsClearComplete(CCmdUI *pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelCompletedAndNoPreview );
	pCmdUI->Enable( m_bSelCompletedAndNoPreview );
}

void CDownloadsWnd::OnDownloadsClearComplete()
{
	CWaitCursor pCursor;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CDownload*> pList;
	if ( ! m_wndDownloads.GetSelectedList( pList, TRUE ) )
		return;

	pLock.Unlock();
	m_wndDownloads.Update();

	CDownload* pDownload;
	while ( ! pList.IsEmpty() )
	{
		if ( ! SafeLock( pLock ) )
			continue;
		pDownload = pList.RemoveHead();
		if ( ! pDownload->IsCompleted() || pDownload->IsTasking() || pDownload->IsPreviewVisible() )
			continue;
		pDownload->Remove();
		pLock.Unlock();
		if ( theApp.KeepAlive() )
			m_wndDownloads.Update();
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsViewReviews(CCmdUI* pCmdUI)
{
	Prepare();

	pCmdUI->Enable( m_bSelHasReviews );
}

void CDownloadsWnd::OnDownloadsViewReviews()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected && ( pDownload->GetReviewCount() >= 0 ) )
		{
			// Make sure data is locked while initializing the dialog
			CDownloadReviewDlg dlg( NULL, pDownload );
			pLock.Unlock();

			dlg.DoModal();
			return;
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsRemotePreview(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 150 ) )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	int nSelected = 0;
	CDownload* pDownload = NULL;

	for ( POSITION pos = Downloads.GetIterator(); pos && nSelected < 3; )
	{
		pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected )
			nSelected++;
	}

	if ( nSelected == 1 && ! pDownload->IsTasking() )
		Prepare();	// Why not just run this?
	else
		m_bSelRemotePreviewCapable = FALSE;

	pCmdUI->Enable( m_bSelRemotePreviewCapable );
}

void CDownloadsWnd::OnDownloadsRemotePreview()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( ! pDownload->m_bSelected )
			continue;

		// Check if the saved preview file is available first
		if ( pDownload->GotPreview() )
		{
			// OnTimer event will launch it
			pDownload->m_bWaitingPreview = TRUE;
			break;
		}

		// Don't do anything if we are already working on the file
		if ( pDownload->IsTasking() )
			break;

		// Find first client which supports previews
		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( ! pSource->m_bPreviewRequestSent && pSource->IsOnline() )
			{
				if ( pSource->m_nProtocol == PROTOCOL_ED2K )
				{
					// m_pTransfer is checked for validity by pSource->IsOnline()
					const CDownloadTransferED2K* pEDTransfer =
						static_cast< const CDownloadTransferED2K* >( pSource->GetTransfer() );
					if ( pEDTransfer->m_pClient->m_bEmPreview )
					{
						pEDTransfer->m_pClient->SendPreviewRequest( pDownload );
						// Don't block sending requests for ed2k sources
						// They are not run in threads, so no harm.
						// pDownload->m_bWaitingPreview = TRUE;
						pSource->m_bPreviewRequestSent = TRUE;
						break;
					}
				}
				else if ( pSource->m_nProtocol == PROTOCOL_HTTP && pSource->m_bPreview && pDownload->m_oSHA1 )
				{
					if ( pSource->m_sPreview.IsEmpty() )
					{
						pSource->m_sPreview.Format( L"http://%s:%u/gnutella/preview/v1?%s",
							(LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ), pSource->m_nPort,
							(LPCTSTR)pDownload->m_oSHA1.toUrn() );
					}
					pDownload->PreviewRequest( pSource->m_sPreview );
					pSource->m_bPreviewRequestSent = TRUE;
					break;
				}
			}
		}
		// Only one download can get selected for preview, no need to check the rest
		break;
	}
}

void CDownloadsWnd::OnUpdateDownloadsLaunch(CCmdUI* pCmdUI)
{
	Prepare();

	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( ! m_bSelCompletedAndNoPreview && ( m_bSelStartedAndNotMoving || ! m_bSelCompleted ) );
	pCmdUI->Enable( ! m_bSelCompletedAndNoPreview && ( m_bSelCompleted || m_bSelStartedAndNotMoving ) );
}

void CDownloadsWnd::OnDownloadsLaunch()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CDownload*> pList;
	if ( ! m_wndDownloads.GetSelectedList( pList ) )
		return;

	while ( ! pList.IsEmpty() )
	{
		if ( ! SafeLock( pLock ) ) return;

		CDownload* pDownload = pList.RemoveHead();

		if ( Downloads.Check( pDownload ) )
		{
			if ( ! pDownload->Launch( -1, &pLock, TRUE ) )
				break;
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsLaunchComplete(CCmdUI* pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pItem->Show( m_bSelCompleted || ! m_bSelStartedAndNotMoving );
	pCmdUI->Enable( m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsLaunchComplete()
{
	OnDownloadsLaunchCopy();
}

void CDownloadsWnd::OnUpdateDownloadsLaunchCopy(CCmdUI* pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelStartedAndNotMoving && ! m_bSelCompleted );
	pCmdUI->Enable( m_bSelStartedAndNotMoving && ! m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsLaunchCopy()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CDownload*> pList;
	if ( ! m_wndDownloads.GetSelectedList( pList ) )
		return;

	while ( ! pList.IsEmpty() )
	{
		if ( ! SafeLock( pLock ) ) return;
		CDownload* pDownload = pList.RemoveHead();

		if ( Downloads.Check( pDownload ) )
		{
			if ( ! pDownload->Launch( -1, &pLock, FALSE ) )
				break;
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsEnqueue(CCmdUI* pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelStartedAndNotMoving || ! m_bSelCompleted );
	pCmdUI->Enable( m_bSelCompleted || m_bSelStartedAndNotMoving );
}

void CDownloadsWnd::OnDownloadsEnqueue()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CDownload*> pList;
	if ( ! m_wndDownloads.GetSelectedList( pList ) )
		return;

	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();

		if ( Downloads.Check( pDownload ) )
		{
			if ( ! pDownload->Enqueue( -1, &pLock ) )
				break;
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsSources(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelMoreSourcesOK );
}

void CDownloadsWnd::OnDownloadsSources()
{
	int nCount = 0;
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( ! pDownload->m_bSelected )
			continue;

		if ( pDownload->IsMoving() )
			continue;

		nCount++;						// Simultaneous FMS operations count
		m_nMoreSourcesLimiter--;		// Overall (Network use) check.
		if ( m_nMoreSourcesLimiter >= 0 )
		{
			pDownload->FindMoreSources();
		}
		else
		{
			// Warn user
			theApp.Message( MSG_DEBUG, L"Find more sources unable to start due to excessive network traffic" );
			// Prevent ed2k bans, client drops, etc.
			m_tMoreSourcesTimer = GetTickCount();
			if ( m_nMoreSourcesLimiter < -30 ) m_nMoreSourcesLimiter = -30;
		}

		// Also only allow 3 FMS operations at once to avoid being blacklisted
		if ( nCount >= 3 ) break;
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsAddSource(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelNotMoving );
}

void CDownloadsWnd::OnDownloadsAddSource()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CDownload*> pList;
	if ( ! m_wndDownloads.GetSelectedList( pList ) )
		return;

	while ( ! pList.IsEmpty() )
	{
		if ( ! SafeLock( pLock ) ) continue;

		CDownload* pDownload = pList.RemoveHead();

		if ( Downloads.Check( pDownload ) && ! pDownload->IsCompleted() && ! pDownload->IsMoving() )
		{
			CDownloadDlg dlg( NULL, pDownload );

			pLock.Unlock();
			if ( dlg.DoModal() != IDOK ) break;
			pLock.Lock();

			if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) continue;

			for ( POSITION pos = dlg.m_pURLs.GetHeadPosition(); pos; )
			{
				pDownload->AddSourceHit( (CEnvyURL)dlg.m_pURLs.GetNext( pos ) );
			}
		}
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsMergeLocal(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_nSelectedDownloads == 1 && m_bSelNotMoving && ! m_bSelCompleted && m_bSelHasSize && m_bSelHasHash );
}

void CDownloadsWnd::OnDownloadsMergeLocal()
{
	CDownload* pDownload = NULL;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected )
			break;	// Found it
	}

	if ( ! pDownload || ! Downloads.Check( pDownload ) || pDownload->IsCompleted() || pDownload->IsMoving() || ! pDownload->PrepareFile() )
		return;		// Unavailable

	if ( pDownload->NeedTigerTree() && pDownload->NeedHashset() && ! pDownload->IsTorrent() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_COMPLETE_NOHASH, MB_ICONEXCLAMATION );
		return; 	// No hashsets
	}

	const Fragments::List oList( pDownload->GetEmptyFragmentList() );
	if ( ! oList.size() )
		return;		// No available fragments

	pLock.Unlock();

	// Select file(s)
	CString strExt( PathFindExtension( pDownload->m_sName ) );
	if ( ! strExt.IsEmpty() ) strExt = strExt.Mid( 1 );

	if ( pDownload->IsMultiFileTorrent() )
	{
		CFileDialog dlgSelectFile( TRUE, strExt, pDownload->m_sName,
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR |
			OFN_ALLOWMULTISELECT, NULL, this );

		CAutoPtr< TCHAR > szFiles( new TCHAR[ 2048 ]{} );
		dlgSelectFile.GetOFN().lpstrFile = szFiles;
		dlgSelectFile.GetOFN().nMaxFile = 2048;

		if ( dlgSelectFile.DoModal() != IDOK )
			return;

		if ( ! SafeLock( pLock ) )
			return;

		if ( ! Downloads.Check( pDownload ) || pDownload->IsTasking() )
		{
			pLock.Unlock();
			MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
			return;
		}

		CList< CString > oFiles;
		CString strFolder = (LPCTSTR)szFiles;
		for ( LPCTSTR szFile = szFiles; *szFile; )
		{
			szFile += _tcslen( szFile ) + 1;
			if ( *szFile )	// Folder + files
				oFiles.AddTail( strFolder + L"\\" + szFile );
			else	// Single file
				oFiles.AddTail( strFolder );
		}

		if ( oFiles.GetCount() )
			pDownload->MergeFile( &oFiles );
	}
	else	// Single File
	{
		CFileDialog dlgSelectFile( TRUE, strExt, pDownload->m_sName,
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR,
			NULL, this );

		if ( dlgSelectFile.DoModal() != IDOK )
			return;

		if ( ! SafeLock( pLock ) )
			return;

		if ( ! Downloads.Check( pDownload ) || pDownload->IsTasking() )
		{
			pLock.Unlock();
			MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
			return;
		}

		pDownload->MergeFile( dlgSelectFile.GetPathName() );
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsBoost(CCmdUI* pCmdUI)
{
	if ( Settings.Bandwidth.Downloads == 0 || Settings.Live.BandwidthScaleIn > 100 )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	Prepare();
	pCmdUI->Enable( m_bSelDownload );	// Allow toggling
	pCmdUI->SetCheck( m_bSelDownload && ! m_bSelBoostable );
}

void CDownloadsWnd::OnDownloadsBoost()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		if ( CDownload* pDownload = Downloads.GetNext( pos ) )
		{
			if ( pDownload->m_bSelected && pDownload->HasActiveTransfers() )
				pDownload->Boost( ! pDownload->IsBoosted() );
		}
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsURI(CCmdUI* pCmdUI)
{
	Prepare();
	const bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
	pCmdUI->Enable( m_bSelHasHash || m_bSelHasName || m_bSelSource && m_nSelectedDownloads <= 50 );
	pCmdUI->SetText( LoadString( ( ( m_nSelectedSources + m_nSelectedDownloads ) == 1 && ! bShift ) ? IDS_LIBRARY_URI_COPY : IDS_LIBRARY_URI_EXPORT ) );
}

void CDownloadsWnd::OnDownloadsURI()
{
	const bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;

	CList< CEnvyFile > pList;

	CSingleLock pLock( &Transfers.m_pSection, FALSE );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload->m_bSelected &&
			 Downloads.Check( pDownload ) &&
			( ! pDownload->m_sName.IsEmpty() || pDownload->m_oSHA1 || pDownload->m_oTiger || pDownload->m_oED2K || pDownload->m_oBTH || pDownload->m_oMD5 ) )
		{
			pList.AddTail( *pDownload );
		}

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			const CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_bSelected )
			{
				CEnvyFile pFile = *pDownload;
				pFile.m_sURL = pSource->m_sURL;
				pList.AddTail( pFile );
			}
		}
	}

	pLock.Unlock();

	if ( pList.IsEmpty() )
		return;

	if ( pList.GetCount() == 1 && ! bShift )
	{
		CURLCopyDlg dlg;
		dlg.Add( &pList.GetHead() );
		dlg.DoModal();
	}
	else //if ( pList.GetCount() > 0 )
	{
		CURLExportDlg dlg;
		for ( POSITION pos = pList.GetHeadPosition(); pos; )
		{
			dlg.Add( &pList.GetNext( pos ) );
		}
		dlg.DoModal();
	}
}

void CDownloadsWnd::OnUpdateDownloadsShare(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelDownload && m_bSelShareConsistent );
	pCmdUI->SetCheck( m_bSelShareState );
}

void CDownloadsWnd::OnDownloadsShare()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload->m_bSelected )
			pDownload->Share( ! pDownload->IsShared() );
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsMonitor(CCmdUI* pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( ! m_bSelCompleted );
	pCmdUI->Enable( m_bSelDownload && ! m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsMonitor()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CDownload*> pList;
	if ( ! m_wndDownloads.GetSelectedList( pList ) )
		return;

	while ( ! pList.IsEmpty() )
	{
		if ( ! SafeLock( pLock ) ) continue;
		CDownload* pDownload = pList.RemoveHead();
		if ( Downloads.Check( pDownload ) && ! pDownload->IsMoving() )
			pDownload->ShowMonitor( &pLock );
	}
}

void CDownloadsWnd::OnUpdateDownloadsEdit(CCmdUI *pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelNotMoving && m_nSelectedDownloads == 1 );
}

void CDownloadsWnd::OnDownloadsEdit()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( ! pDownload->m_bSelected )
			continue;

		if ( pDownload->IsMoving() )
			return;

		if ( ! pDownload->IsCompleted() || pDownload->IsSeeding() )
		{
			CDownloadSheet dlg( pLock, pDownload );
			dlg.DoModal();		// Note: Requires lock
			break;
		}

		if ( m_bSelCompleted )
		{
			CFilePropertiesSheet dlg;

			for ( DWORD i = 0; i < pDownload->GetFileCount(); i++ )
			{
				CString strPath	= pDownload->GetPath( i );
				CQuickLock oLibraryLock( Library.m_pSection );
				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath ) )
					dlg.Add( pFile );
			}

			pLock.Unlock();
			dlg.DoModal( 0 );
			break;
		}
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsMove(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelDownload );
}

void CDownloadsWnd::OnDownloadsMoveUp()
{
	m_wndDownloads.MoveSelected( -1 );
}

void CDownloadsWnd::OnDownloadsMoveDown()
{
	m_wndDownloads.MoveSelected( 1 );
}

void CDownloadsWnd::OnDownloadsMoveTop()
{
	m_wndDownloads.MoveToTop();
}

void CDownloadsWnd::OnDownloadsMoveBottom()
{
	m_wndDownloads.MoveToEnd();
}


void CDownloadsWnd::OnUpdateTransfersConnect(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelIdleSource && m_bConnectOkay );
}

void CDownloadsWnd::OnTransfersConnect()
{
	CList< CDownloadSource* > pSelectedSources;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload->IsMoving() )	// || pDownload->IsCompleted()
			continue;

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			ASSERT( pSource->m_pDownload == pDownload );

			// Only create a new Transfer if there isn't already one
			if ( pSource->m_bSelected && pSource->IsIdle() )
				pSelectedSources.AddTail( pSource );
		}
	}

	for ( POSITION posSource = pSelectedSources.GetHeadPosition(); posSource; )
	{
		CDownloadSource* pSource = pSelectedSources.GetNext( posSource );
		pSource->m_pDownload->Resume();

		if ( pSource->m_bPushOnly )
		{
			pSource->PushRequest();
		}
		else
		{
			if ( CDownloadTransfer* pTransfer = pSource->CreateTransfer() )
				pTransfer->Initiate();
		}
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateTransfersDisconnect(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelActiveSource );
}

void CDownloadsWnd::OnTransfersDisconnect()
{
	CList< CDownloadSource* > pSelectedSources;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 5000 ) && ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_bSelected && ! pSource->IsIdle() )
				pSelectedSources.AddTail( pSource );
		}
	}

	for ( POSITION posSource = pSelectedSources.GetHeadPosition(); posSource; )
	{
		CDownloadSource* pSource = pSelectedSources.GetNext( posSource );
		pSource->Close();
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateTransfersForget(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelSource );
}

void CDownloadsWnd::OnTransfersForget()
{
	CList< CDownloadSource* > pSelectedSources;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_bSelected )
				pSelectedSources.AddTail( pSource );
		}
	}

	for ( POSITION posSource = pSelectedSources.GetHeadPosition(); posSource; )
	{
		CDownloadSource* pSource = pSelectedSources.GetNext( posSource );
		pSource->Remove( TRUE, TRUE );
	}

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnUpdateTransfersChat(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelChat && Settings.Community.ChatEnable );
}

void CDownloadsWnd::OnTransfersChat()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_bSelected )
			{
				if ( pSource->m_nProtocol == PROTOCOL_HTTP )						// HTTP chat
					ChatWindows.OpenPrivate( Hashes::Guid(), &pSource->m_pAddress, pSource->m_nPort, pSource->m_bPushOnly, pSource->m_nProtocol, &pSource->m_pServerAddress, pSource->m_nServerPort );
				else if ( pSource->m_bClientExtended && ! pSource->m_bPushOnly )	// Client accepts G2 chat connections
					ChatWindows.OpenPrivate( Hashes::Guid(), &pSource->m_pAddress, pSource->m_nPort, FALSE, PROTOCOL_G2 );
				else if ( pSource->m_nProtocol == PROTOCOL_ED2K )					// ED2K chat
					ChatWindows.OpenPrivate( Hashes::Guid(), &pSource->m_pAddress, pSource->m_nPort, pSource->m_bPushOnly, pSource->m_nProtocol, &pSource->m_pServerAddress, pSource->m_nServerPort );
				//else		// Should never be called
				//	theApp.Message( MSG_DEBUG, L"Error while initiating chat- Unable to select protocol" );
			}
		}
	}
}

void CDownloadsWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelBrowse );
}

void CDownloadsWnd::OnBrowseLaunch()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_bSelected )
			{
				SOCKADDR_IN pAddress = { AF_INET };
				pAddress.sin_port = htons( pSource->m_nPort );
				pAddress.sin_addr = pSource->m_pAddress;
				if ( pSource->m_nProtocol == PROTOCOL_HTTP || pSource->m_nProtocol == PROTOCOL_ED2K )	// Many HTTP clients support this
					new CBrowseHostWnd( pSource->m_nProtocol, &pAddress, pSource->m_bPushOnly, pSource->m_oGUID );
				else if ( pSource->m_bClientExtended )			// Over other protocols, you can only contact non-push G2 clients
					new CBrowseHostWnd( pSource->m_nProtocol, &pAddress, FALSE, Hashes::Guid() );
			}
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsFolder(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelDownload || ! m_bSelAny );
}

void CDownloadsWnd::OnDownloadsFolder()
{
	if ( ! m_bSelAny )
	{
		ShellExecute( GetSafeHwnd(), L"open", Settings.Downloads.CompletePath, NULL, NULL, SW_SHOWNORMAL );
		return;
	}

	CStringList pList;
	UINT nLimit = Settings.Library.ExecuteFilesLimit ? Settings.Library.ExecuteFilesLimit : 999;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos && nLimit; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( ! pDownload->m_bSelected || ! ( pDownload->IsCompleted() || pDownload->IsSeeding() ) )
			continue;

		CString strPath = pDownload->GetPath( 0 );

		if ( pDownload->GetFileCount() > 1 )
		{
			CString strName = pDownload->m_sName + L'\\';
			if ( strPath.Find( strName ) > 1 )
				strPath = strPath.Left( strPath.Find( strName ) + strName.GetLength() );
			else
				strPath = strPath.Left( strPath.ReverseFind( L'\\' ) + 1 );
		}

		pList.AddTail( strPath );
		nLimit--;
	}

	pLock.Unlock();

	while ( ! pList.IsEmpty() )
	{
		CString strPath = pList.RemoveHead();

		if ( strPath.Right( 1 ) != L'\\' )
		{
			if ( PathFileExists( SafePath( strPath ) ) )
			{
				MakeSafePath( strPath );
				ShellExecute( GetSafeHwnd(), NULL, L"Explorer.exe", L"/select, " + strPath, NULL, SW_SHOWNORMAL );
				continue;
			}

			strPath = strPath.Left( strPath.ReverseFind( L'\\' ) + 1 );
		}

		if ( PathIsDirectory( strPath ) )
			ShellExecute( GetSafeHwnd(), L"open", strPath, NULL, NULL, SW_SHOWNORMAL );
	}
}

void CDownloadsWnd::OnUpdateDownloadsFileDelete(CCmdUI* pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelCompleted );
	pCmdUI->Enable( m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsFileDelete()
{
	// Create file list of all selected and completed downloads
	CStringList pList, pFolderList;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( ! pDownload->m_bSelected || ! pDownload->IsCompleted() )
			continue;

		pDownload->m_bClearing = TRUE;		// Indicate marked for removal (may take awhile)

		const DWORD nCount = pDownload->GetFileCount();
		for ( DWORD i = 0; i < nCount; ++i )
		{
			pList.AddTail( pDownload->GetPath( i ) );
		}

		// Attempt orphan root folder for torrents
		if ( nCount > 1 )	// pDownload->IsMultiFileTorrent()
		{
			CString strPath = pDownload->GetPath( 0 );
			strPath = strPath.Left( strPath.ReverseFind( L'\\' ) );
			pFolderList.AddTail( strPath );

			CString strPathCheck = pList.GetTail();
			strPathCheck = strPathCheck.Left( strPathCheck.ReverseFind( L'\\' ) );
			if ( strPathCheck.GetLength() < strPath.GetLength() )
				pFolderList.AddTail( strPathCheck );

			const int nRoot = strPathCheck.Find( pDownload->m_sName + L"\\" );
			if ( nRoot > 4 )					// Last file was likely subfolder, try again
				pFolderList.AddTail( strPathCheck.Left( nRoot + pDownload->m_sName.GetLength() ) );
		}
	}

	pLock.Unlock();
	Update();

	if ( ! DeleteFiles( pList ) )
	{
		pLock.Lock();
		for ( POSITION pos = Downloads.GetIterator(); pos; )
		{
			CDownload* pDownload = Downloads.GetNext( pos );
			pDownload->m_bClearing = FALSE;
		}
		pLock.Unlock();
	}
	DeleteFolders( pFolderList );

	Update();
}

void CDownloadsWnd::OnUpdateDownloadsRate(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsRate()
{
	CFilePropertiesSheet dlg;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected && pDownload->IsCompleted() )
		{
			for ( DWORD i = 0; i < pDownload->GetFileCount(); ++i )
			{
				CString strPath	= pDownload->GetPath( i );
				CQuickLock oLibraryLock( Library.m_pSection );
				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath ) )
					dlg.Add( pFile );
			}
		}
	}

	pLock.Unlock();

	dlg.DoModal( 2 );

	Update();
}

void CDownloadsWnd::OnUpdateDownloadsShowSources(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.Downloads.ShowSources );
}

void CDownloadsWnd::OnDownloadsShowSources()
{
	Settings.Downloads.ShowSources = ! Settings.Downloads.ShowSources;
	Update();
}

void CDownloadsWnd::OnDownloadsClearCompleted()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	Downloads.ClearCompleted();

	pLock.Unlock();
	Update();
}

void CDownloadsWnd::OnDownloadsClearPaused()
{
	if ( MsgBox( IDS_DOWNLOAD_CONFIRM_CLEAR_PAUSED, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) == IDYES )
	{
		Downloads.ClearPaused();
		Update();
	}
}

void CDownloadsWnd::OnUpdateDownloadsAutoClear(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.Downloads.AutoClear );
}

void CDownloadsWnd::OnDownloadsAutoClear()
{
	Settings.Downloads.AutoClear = ! Settings.Downloads.AutoClear;
	if ( Settings.Downloads.AutoClear ) OnTimer( 4 );
}

void CDownloadsWnd::OnDownloadsFilterMenu()
{
	CMenu* pMenu = Skin.GetMenu( L"CDownloadsWnd.Filter" );
	m_wndToolBar.ThrowMenu( ID_DOWNLOADS_FILTER_MENU, pMenu, NULL, FALSE, TRUE );
}

void CDownloadsWnd::OnUpdateDownloadsFilterAll(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_ALL ) == DLF_ALL );
}

void CDownloadsWnd::OnDownloadsFilterAll()
{
	Settings.Downloads.FilterMask |= DLF_ALL;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterActive(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_ACTIVE ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterActive()
{
	Settings.Downloads.FilterMask ^= DLF_ACTIVE;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterPaused(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_PAUSED ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterPaused()
{
	Settings.Downloads.FilterMask ^= DLF_PAUSED;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterQueued(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_QUEUED ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterQueued()
{
	Settings.Downloads.FilterMask ^= DLF_QUEUED;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterSources(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_SOURCES ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterSources()
{
	Settings.Downloads.FilterMask ^= DLF_SOURCES;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterSeeds(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_SEED ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterSeeds()
{
	Settings.Downloads.FilterMask ^= DLF_SEED;
	Update();
}

void CDownloadsWnd::OnDownloadsSettings()
{
	CSettingsManagerDlg::Run( L"CDownloadsSettingsPage" );
}

void CDownloadsWnd::OnUpdateDownloadGroupShow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.General.GUIMode != GUI_BASIC );
	pCmdUI->SetCheck( Settings.General.GUIMode != GUI_BASIC && Settings.Downloads.ShowGroups );
}

void CDownloadsWnd::OnDownloadGroupShow()
{
	Settings.Downloads.ShowGroups = ! Settings.Downloads.ShowGroups;

	CRect rc;
	GetClientRect( &rc );
	OnSize( SIZE_RESTORED, rc.right, rc.bottom );

	Update();
}

void CDownloadsWnd::OnDownloadsHelp()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CDownload* pDownload = NULL;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) break;
		pDownload = NULL;
	}

	CString strHelp;
	if ( pDownload == NULL )
		strHelp = L"DownloadHelp.Select";
	else if ( pDownload->IsSeeding() )
		strHelp = L"DownloadHelp.Seeding";
	else if ( pDownload->IsMoving() )
		strHelp = pDownload->IsCompleted() ?
			L"DownloadHelp.Completed" : L"DownloadHelp.Moving";
	else if ( pDownload->IsPaused() )
		strHelp = ( pDownload->GetFileError() != ERROR_SUCCESS ) ?
			L"DownloadHelp.DiskFull" : L"DownloadHelp.Paused";
	else if ( pDownload->IsStarted() && pDownload->GetProgress() == 100.0f )
		strHelp = L"DownloadHelp.Verifying";
	else if ( pDownload->IsDownloading() )
		strHelp = L"DownloadHelp.Downloading";
	else if ( ! pDownload->IsTrying() )
		strHelp = L"DownloadHelp.Queued";
	else if ( pDownload->GetSourceCount() > 0 )
		strHelp = L"DownloadHelp.Pending";
	else if ( pDownload->m_nSize == SIZE_UNKNOWN )
		strHelp = L"DownloadHelp.Searching";
	else if ( pDownload->GetTaskType() == dtaskAllocate )
		strHelp = L"DownloadHelp.Creating";
	else if ( pDownload->m_bTorrentTrackerError )
		strHelp = L"DownloadHelp.Tracker";
	else
		strHelp = L"DownloadHelp.Searching";

	pLock.Unlock();
	CHelpDlg::Show( strHelp );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadsWnd drag and drop

void CDownloadsWnd::DragDownloads(CList< CDownload* >* pList, CImageList* pImage, const CPoint& ptScreen)
{
	if ( m_pDragList )
	{
		CancelDrag();
		return;
	}

	m_pDragList  = pList;
	m_pDragImage = pImage;

	CRect rcClient;
	GetClientRect( &rcClient );
	ClientToScreen( &rcClient );

	ClipCursor( &rcClient );
	SetCapture();
	SetFocus();
	UpdateWindow();

	OnSetCursor( NULL, 0, 0 );

	CPoint ptStart( ptScreen );
	ScreenToClient( &ptStart );

	CRect rcList;
	m_wndDownloads.GetWindowRect( &rcList );
	ScreenToClient( &rcList );
	m_pDragOffs = rcList.TopLeft();
	m_pDragOffs.y -= 4;

	m_pDragImage->DragEnter( this, ptStart + m_pDragOffs );
}

void CDownloadsWnd::CancelDrag()
{
	if ( ! m_pDragList ) return;

	ClipCursor( NULL );
	ReleaseCapture();

	m_pDragImage->DragLeave( this );
	m_pDragImage->EndDrag();
	delete m_pDragImage;
	m_pDragImage = NULL;

	delete m_pDragList;
	m_pDragList = NULL;

	CPoint point( 0, 0 );
	m_wndTabBar.DropObjects( NULL, point );
	m_wndDownloads.DropObjects( NULL, point );
}

void CDownloadsWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( m_pDragList )
	{
		m_pDragImage->DragMove( point + m_pDragOffs );
		ClientToScreen( &point );
		if ( Settings.Downloads.ShowGroups )
			m_wndTabBar.OnMouseMoveDrag( point );
		m_wndDownloads.OnMouseMoveDrag( point );	// Was DropShowTarget( m_pDragList )
	}

	CPanelWnd::OnMouseMove( nFlags, point );
}

void CDownloadsWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( m_pDragList )
	{
		ClipCursor( NULL );
		ReleaseCapture();

		m_pDragImage->DragLeave( this );
		m_pDragImage->EndDrag();
		delete m_pDragImage;
		m_pDragImage = NULL;

		ClientToScreen( &point );
		m_wndTabBar.DropObjects( m_pDragList, point );
		m_wndDownloads.DropObjects( m_pDragList, point );

		delete m_pDragList;
		m_pDragList = NULL;

		Update();
	}

	CPanelWnd::OnLButtonUp( nFlags, point );
}

void CDownloadsWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ( m_pDragList )
	{
		CancelDrag();
		return;
	}

	CPanelWnd::OnRButtonDown( nFlags, point );
}

void CDownloadsWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ( nChar == VK_ESCAPE && m_pDragList )
	{
		CancelDrag();
		OnSetCursor( NULL, 0, 0 );
	}

	if ( nChar == VK_SHIFT || nChar == VK_CONTROL )		// Skip async keys
		return;

	CPanelWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CDownloadsWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ( m_pDragList )
		OnSetCursor( NULL, 0, 0 );

	CPanelWnd::OnKeyUp( nChar, nRepCnt, nFlags );
}

BOOL CDownloadsWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_pDragList )
	{
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
			SetCursor( m_hCursCopy );
		else
			SetCursor( m_hCursMove );

		return TRUE;
	}

	return CPanelWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CDownloadsWnd::OnCaptureChanged(CWnd *pWnd)
{
	CPanelWnd::OnCaptureChanged(pWnd);
	m_bMouseCaptured = true;
	Update();
}
