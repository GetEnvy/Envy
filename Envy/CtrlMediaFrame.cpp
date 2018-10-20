//
// CtrlMediaFrame.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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
#include "CtrlMediaFrame.h"
#include "WndMedia.h"
#include "WndMain.h"
#include "Plugins.h"
#include "Library.h"
#include "SharedFile.h"
#include "Registry.h"

#include "DlgSettingsManager.h"
#include "DlgMediaVis.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// ToDo: Make these skinnable options ?
//#define SPLIT_SIZE	6				// Settings.Skin.Splitter
#define HEADER_HEIGHT	17
#define STATUS_HEIGHT	18
#define SIZE_INTERNAL	1982
#define SIZE_BARSLIDE	1983
#define TOOLBAR_STICK	3000
#define TOOLBAR_ANIMATE	800
#define VOLUME_FACTOR	100
#define SPEED_FACTOR	100
#define META_DELAY		10000
#define TIME_FACTOR		1000000
#define ONE_SECOND		10000000

#define UPDATE_TIMER	100				// ms delay for controls display
#define VOLUME_KEY_MULTIPLIER 5


#ifndef WM_APPCOMMAND

#define WM_APPCOMMAND					0x319
#define APPCOMMAND_VOLUME_MUTE			8
#define APPCOMMAND_VOLUME_DOWN			9
#define APPCOMMAND_VOLUME_UP			10
#define APPCOMMAND_MEDIA_NEXTTRACK		11
#define APPCOMMAND_MEDIA_PREVIOUSTRACK	12
#define APPCOMMAND_MEDIA_STOP			13
#define APPCOMMAND_MEDIA_PLAY_PAUSE		14
#define FAPPCOMMAND_MASK				0x8000
#define GET_APPCOMMAND_LPARAM(lParam)	((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))

#endif // WM_APPCOMMAND


CMediaFrame* CMediaFrame::m_wndMediaFrame = NULL;

IMPLEMENT_DYNAMIC(CMediaFrame, CWnd)

BEGIN_MESSAGE_MAP(CMediaFrame, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOMMAND()
	ON_WM_SETCURSOR()
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_UPDATE_COMMAND_UI(ID_MEDIA_CLOSE, OnUpdateMediaClose)
	ON_COMMAND(ID_MEDIA_CLOSE, OnMediaClose)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PLAY, OnUpdateMediaPlay)
	ON_COMMAND(ID_MEDIA_PLAY, OnMediaPlay)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PAUSE, OnUpdateMediaPause)
	ON_COMMAND(ID_MEDIA_PAUSE, OnMediaPause)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_STOP, OnUpdateMediaStop)
	ON_COMMAND(ID_MEDIA_STOP, OnMediaStop)
	ON_COMMAND(ID_MEDIA_ZOOM, OnMediaZoom)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_FILL, OnUpdateMediaSizeFill)
	ON_COMMAND(ID_MEDIA_SIZE_FILL, OnMediaSizeFill)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_DISTORT, OnUpdateMediaSizeDistort)
	ON_COMMAND(ID_MEDIA_SIZE_DISTORT, OnMediaSizeDistort)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_ONE, OnUpdateMediaSizeOne)
	ON_COMMAND(ID_MEDIA_SIZE_ONE, OnMediaSizeOne)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_TWO, OnUpdateMediaSizeTwo)
	ON_COMMAND(ID_MEDIA_SIZE_TWO, OnMediaSizeTwo)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_HALF, OnUpdateMediaSizeHalf)
	ON_COMMAND(ID_MEDIA_SIZE_HALF, OnMediaSizeHalf)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_ASPECT_DEFAULT, OnUpdateMediaAspectDefault)
	ON_COMMAND(ID_MEDIA_ASPECT_DEFAULT, OnMediaAspectDefault)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_ASPECT_4_3, OnUpdateMediaAspect43)
	ON_COMMAND(ID_MEDIA_ASPECT_4_3, OnMediaAspect43)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_ASPECT_16_9, OnUpdateMediaAspect169)
	ON_COMMAND(ID_MEDIA_ASPECT_16_9, OnMediaAspect169)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_FULLSCREEN, OnUpdateMediaFullScreen)
	ON_COMMAND(ID_MEDIA_FULLSCREEN, OnMediaFullScreen)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PLAYLIST, OnUpdateMediaPlaylist)
	ON_COMMAND(ID_MEDIA_PLAYLIST, OnMediaPlaylist)
	ON_COMMAND(ID_MEDIA_SETTINGS, OnMediaSettings)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SETTINGS, OnUpdateMediaSettings)
	ON_COMMAND(ID_MEDIA_VIS, OnMediaVis)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_VIS, OnUpdateMediaVis)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_STATUS, OnUpdateMediaStatus)
	ON_COMMAND(ID_MEDIA_STATUS, OnMediaStatus)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_MUTE, OnUpdateMediaMute)
	ON_COMMAND(ID_MEDIA_MUTE, OnMediaMute)
	ON_NOTIFY(MLN_NEWCURRENT, IDC_MEDIA_PLAYLIST, OnNewCurrent)
	ON_MESSAGE(WM_APPCOMMAND, OnMediaKey)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMediaFrame construction

CMediaFrame::CMediaFrame()
	: m_pPlayer			( NULL )
	, m_nState			( smsNull )
	, m_tLastPlay		( 0 )
	, m_tMetadata		( 0 )
	, m_bRepeat			( FALSE )
	, m_bLastMedia		( FALSE )
	, m_bLastNotPlayed	( FALSE )
	, m_bStopFlag		( FALSE )
	, m_bEnqueue		( FALSE )
	, m_bMute			( FALSE )
	, m_bThumbPlay		( FALSE )
	, m_bFullScreen		( FALSE )
	, m_bStatusVisible	( Settings.MediaPlayer.StatusVisible )
	, m_bListVisible	( Settings.MediaPlayer.ListVisible )
	, m_bListWasVisible	( Settings.MediaPlayer.ListVisible )
	, m_nListSize		( Settings.MediaPlayer.ListSize )
	, m_rcVideo			()
	, m_rcStatus		()
	, m_nVidAC			( 0 )
	, m_nVidDC			( 0 )
	, m_bScreenSaverEnabled	( TRUE )
	, m_nScreenSaverTime( 0 )
	, m_nPowerSchemeId	( 0 )
	, m_CurrentGP		()
	, m_CurrentPP		()
{
	UpdateNowPlaying(TRUE);
}

CMediaFrame::~CMediaFrame()
{
	UpdateNowPlaying(TRUE);
}

CMediaFrame* CMediaFrame::GetMediaFrame()
{
//	if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
//	{
//		if ( CMediaWnd* pMediaWnd = static_cast< CMediaWnd* >( pMainWnd->m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) ) )
//		{
//			if ( CMediaFrame* pMediaFrame = static_cast< CMediaFrame* >( pMediaWnd->GetWindow( GW_CHILD ) ) )
//			{
//				ASSERT_KINDOF( CMediaFrame, pMediaFrame );
//				return pMediaFrame;
//			}
//		}
//	}

	return m_wndMediaFrame;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame system message handlers

BOOL CMediaFrame::Create(CWnd* pParentWnd)
{
	CRect rect;
	return CWnd::Create( NULL, L"CMediaFrame", WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, rect, pParentWnd, 0, NULL );
}

int CMediaFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndMediaFrame = this;

	CRect rectDefault;
	SetOwner( GetParent() );

	m_wndList.Create( this, IDC_MEDIA_PLAYLIST );

	if ( ! m_wndListBar.Create( this, WS_CHILD|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndListBar.SetBarStyle( m_wndListBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndListBar.SetOwner( GetOwner() );

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndToolBar.SetOwner( GetOwner() );

	m_wndPosition.Create( WS_CHILD|WS_TABSTOP|TBS_HORZ|TBS_NOTICKS|TBS_TOP,
		rectDefault, &m_wndToolBar, IDC_MEDIA_POSITION );
	m_wndPosition.SetRange( 0, 0 );
	m_wndPosition.SetPageSize( 0 );

	m_wndSpeed.Create( WS_CHILD|WS_TABSTOP|TBS_HORZ|TBS_NOTICKS|TBS_TOP,
		rectDefault, &m_wndToolBar, IDC_MEDIA_SPEED );
	m_wndSpeed.SetRange( 0, 200 );
//	m_wndSpeed.SetPageSize( 50 );
	m_wndSpeed.SetTic( 0 );
	m_wndSpeed.SetTic( 50 );
	m_wndSpeed.SetTic( 100 );
	m_wndSpeed.SetTic( 150 );
	m_wndSpeed.SetTic( 200 );
//	m_wndSpeed.SetTic( 300 );
//	m_wndSpeed.SetTic( 400 );

	m_wndVolume.Create( WS_CHILD|WS_TABSTOP|TBS_HORZ|TBS_NOTICKS|TBS_TOP,
		rectDefault, &m_wndToolBar, IDC_MEDIA_VOLUME );
	m_wndVolume.SetRange( 0, 100 );
	m_wndVolume.SetPageSize( 10 );
	m_wndVolume.SetTic( 0 );
	m_wndVolume.SetTic( 100 );

	if ( Settings.General.LanguageRTL )
	{
		m_wndPosition.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_wndSpeed.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_wndVolume.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	}

	CBitmap bmIcons;
	bmIcons.LoadBitmap( IDB_MEDIA_STATES );
	m_pIcons.Create( 16, 16, ILC_COLOR32|ILC_MASK, 3, 0 ) ||
	m_pIcons.Create( 16, 16, ILC_COLOR24|ILC_MASK, 3, 0 ) ||
	m_pIcons.Create( 16, 16, ILC_COLOR16|ILC_MASK, 3, 0 );
	m_pIcons.Add( &bmIcons, RGB( 0, 255, 0 ) );

	m_wndList.LoadTextList( Settings.General.DataPath + L"Playlist.m3u" );

	UpdateState();

	SetTimer( 1, UPDATE_TIMER, NULL );

	return 0;
}

void CMediaFrame::OnDestroy()
{
	m_wndList.SaveTextList( Settings.General.DataPath + L"Playlist.m3u" );

	Settings.MediaPlayer.ListSize		= m_nListSize;
	Settings.MediaPlayer.ListVisible	= m_bListVisible != FALSE;
	Settings.MediaPlayer.StatusVisible	= m_bStatusVisible != FALSE;

	KillTimer( 2 );
	KillTimer( 1 );

	Cleanup();

	EnableScreenSaver();

	m_wndMediaFrame = NULL;

	CWnd::OnDestroy();
}

BOOL CMediaFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndList.m_hWnd )
	{
		if ( m_wndList.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	if ( m_wndListBar.m_hWnd )
	{
		if ( m_wndListBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	if ( m_wndToolBar.m_hWnd )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}

	return CWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

BOOL CMediaFrame::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		switch ( pMsg->wParam )
		{
		case VK_ESCAPE:
			if ( m_bFullScreen )
				SetFullScreen( FALSE );
			else if ( m_nState == smsPlaying )
				PostMessage( WM_COMMAND, ID_MEDIA_PAUSE );
			return TRUE;

		case VK_RETURN:
			SetFullScreen( ! m_bFullScreen );
			return TRUE;

		case VK_SPACE:
			PostMessage( WM_COMMAND, m_nState == smsPlaying ? ID_MEDIA_PAUSE : ID_MEDIA_PLAY );
			return TRUE;

		//case VK_UP:
		//	OffsetVolume( VOLUME_KEY_MULTIPLIER );
		//	return TRUE;

		//case VK_DOWN:
		//	OffsetVolume( - VOLUME_KEY_MULTIPLIER );
		//	return TRUE;

		case VK_LEFT:
			if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
				m_wndList.PostMessage( WM_COMMAND, ID_MEDIA_PREVIOUS );
		//	else if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		//		OffsetPosition( -2 );
			return TRUE;

		case VK_RIGHT:
			if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
				m_wndList.PostMessage( WM_COMMAND, ID_MEDIA_NEXT );
		//	else if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		//		OffsetPosition( 2 );
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage( pMsg );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame presentation message handlers

void CMediaFrame::OnSkinChange()
{
	OnSize( 0, 0, 0 );
	Skin.CreateToolBar( L"CMediaFrame", &m_wndToolBar );
	Skin.CreateToolBar( L"CMediaList", &m_wndListBar );

	if ( CCoolBarItem* pItem = m_wndToolBar.GetID( IDC_MEDIA_POSITION ) ) pItem->Enable( FALSE );
	if ( CCoolBarItem* pItem = m_wndToolBar.GetID( IDC_MEDIA_SPEED ) )  pItem->Enable( FALSE );
	if ( CCoolBarItem* pItem = m_wndToolBar.GetID( IDC_MEDIA_VOLUME ) ) pItem->Enable( FALSE );

	HICON hIcon = CoolInterface.ExtractIcon( (UINT)ID_MEDIA_STATE_STOP, FALSE );
	if ( hIcon )
	{
		m_pIcons.Replace( 0, hIcon );
		DestroyIcon( hIcon );
	}
	hIcon = CoolInterface.ExtractIcon( (UINT)ID_MEDIA_STATE_PAUSE, FALSE );
	if ( hIcon )
	{
		m_pIcons.Replace( 1, hIcon );
		DestroyIcon( hIcon );
	}
	hIcon = CoolInterface.ExtractIcon( (UINT)ID_MEDIA_STATE_PLAY, FALSE );
	if ( hIcon )
	{
		m_pIcons.Replace( 2, hIcon );
		DestroyIcon( hIcon );
	}

	if ( m_bmLogo.m_hObject ) m_bmLogo.DeleteObject();
	m_bmLogo.m_hObject = Skin.GetWatermark( L"LargeLogo" );
	if ( m_pPlayer && m_bmLogo.m_hObject )
		m_pPlayer->SetLogoBitmap( m_bmLogo );

	m_wndList.OnSkinChange();
}

void CMediaFrame::OnUpdateCmdUI()
{
	m_wndToolBar.OnUpdateCmdUI( (CFrameWnd*)GetOwner(), TRUE );
	m_wndListBar.OnUpdateCmdUI( (CFrameWnd*)GetOwner(), TRUE );
}

void CMediaFrame::SetFullScreen(BOOL bFullScreen)
{
	if ( bFullScreen == m_bFullScreen ) return;

	ShowWindow( SW_HIDE );
	m_tBarTime = GetTickCount();

	m_bFullScreen = bFullScreen;
	if ( m_bFullScreen )
	{
		ModifyStyle( WS_CHILD, 0 );
		SetParent( NULL );

		HMONITOR hMonitor = MonitorFromWindow( AfxGetMainWnd()->GetSafeHwnd(), MONITOR_DEFAULTTOPRIMARY );

		MONITORINFO oMonitor = {0};
		oMonitor.cbSize = sizeof( MONITORINFO );
		GetMonitorInfo( hMonitor, &oMonitor );

		SetWindowPos( &wndTopMost, oMonitor.rcMonitor.left, oMonitor.rcMonitor.top,
			oMonitor.rcMonitor.right - oMonitor.rcMonitor.left,
			oMonitor.rcMonitor.bottom - oMonitor.rcMonitor.top, SWP_FRAMECHANGED|SWP_SHOWWINDOW );

		m_bListWasVisible 	= m_bListVisible;
		m_bListVisible 		= FALSE;
		OnSize( SIZE_INTERNAL, 0, 0 );

		SetTimer( 2, 30, NULL );
	}
	else
	{
		CWnd* pOwner = GetOwner();

		ModifyStyle( 0, WS_CHILD );
		SetParent( pOwner );

		CRect rc;
		pOwner->GetClientRect( &rc );
		SetWindowPos( NULL, 0, 0, rc.right, rc.bottom, SWP_FRAMECHANGED|SWP_SHOWWINDOW );
		m_bListVisible = m_bListWasVisible;
		OnSize( SIZE_INTERNAL, 0, 0 );
		KillTimer( 2 );
	}
}

void CMediaFrame::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != SIZE_INTERNAL && nType != SIZE_BARSLIDE )
		CWnd::OnSize( nType, cx, cy );

	CRect rc;
	GetClientRect( &rc );

	if ( rc.Width() < 32 || rc.Height() < 32 ) return;

	if ( rc.Width() < m_nListSize + Settings.Skin.Splitter )
		m_nListSize = max( 0, rc.Width() - (int)Settings.Skin.Splitter );

	if ( m_bListVisible || ! m_bFullScreen )
	{
		rc.bottom -= Settings.Skin.ToolbarHeight;
		m_wndToolBar.SetWindowPos( NULL, rc.left, rc.bottom, rc.Width(),
			Settings.Skin.ToolbarHeight, SWP_NOZORDER|SWP_SHOWWINDOW );

		if ( m_bListVisible )
		{
			rc.right -= m_nListSize;
			m_wndList.SetWindowPos( NULL, rc.right, rc.top + HEADER_HEIGHT, m_nListSize,
				rc.bottom - Settings.Skin.ToolbarHeight - HEADER_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
			m_wndListBar.SetWindowPos( NULL, rc.right, rc.bottom - Settings.Skin.ToolbarHeight,
				m_nListSize, Settings.Skin.ToolbarHeight, SWP_NOZORDER|SWP_SHOWWINDOW );
			rc.right -= Settings.Skin.Splitter;
		}
		else if ( m_wndList.IsWindowVisible() )
		{
			m_wndList.ShowWindow( SW_HIDE );
			m_wndListBar.ShowWindow( SW_HIDE );
		}
	}
	else
	{
		if ( m_wndList.IsWindowVisible() )
		{
			m_wndList.ShowWindow( SW_HIDE );
			m_wndListBar.ShowWindow( SW_HIDE );
		}

		DWORD tElapse = GetTickCount() - m_tBarTime;
		int nBar = Settings.Skin.ToolbarHeight;

		if ( tElapse < TOOLBAR_STICK )
		{
			nBar = Settings.Skin.ToolbarHeight;
		}
		else if ( tElapse > TOOLBAR_STICK + TOOLBAR_ANIMATE )
		{
			nBar = 0;
			SetCursor( NULL );
			KillTimer( 2 );
		}
		else
		{
			tElapse -= TOOLBAR_STICK;
			nBar = Settings.Skin.ToolbarHeight - ( tElapse * Settings.Skin.ToolbarHeight / TOOLBAR_ANIMATE );
		}

		m_wndToolBar.SetWindowPos( NULL, rc.left, rc.bottom - nBar, rc.Width(),
			Settings.Skin.ToolbarHeight, SWP_NOZORDER|SWP_SHOWWINDOW );
	}

	if ( m_bStatusVisible )
	{
		if ( m_bFullScreen )
		{
			m_rcStatus.SetRect( rc.left, rc.top, rc.right, rc.top + STATUS_HEIGHT );
			rc.top += STATUS_HEIGHT;
		}
		else
		{
			m_rcStatus.SetRect( rc.left, rc.bottom - STATUS_HEIGHT, rc.right, rc.bottom );
			rc.bottom -= STATUS_HEIGHT;
		}
	}

	m_rcVideo = rc;

	if ( m_pPlayer != NULL && nType != SIZE_BARSLIDE )
		m_pPlayer->Reposition( &rc );

	if ( nType != SIZE_BARSLIDE ) Invalidate();
}

void CMediaFrame::OnPaint()
{
	CPaintDC dc( this );

	if ( ! m_pFontDefault.m_hObject )
	{
		LOGFONT pFont = { 80, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			Settings.Fonts.Quality, DEFAULT_PITCH|FF_DONTCARE,
			(WCHAR)(LPCTSTR)Settings.Fonts.DefaultFont };	// "Segoe UI"

		m_pFontDefault.CreatePointFontIndirect( &pFont );

		pFont.lfHeight = 80;
		m_pFontValue.CreatePointFontIndirect( &pFont );
		pFont.lfWeight = FW_BLACK;
		m_pFontKey.CreatePointFontIndirect( &pFont );
	}

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFontDefault );

	CRect rcClient;
	GetClientRect( &rcClient );

	if ( m_bListVisible )
	{
		CRect rcBar( rcClient.right - m_nListSize - Settings.Skin.Splitter,
					 rcClient.top,
					 rcClient.right - m_nListSize,
					 rcClient.bottom );

		dc.FillSolidRect( rcBar.left, rcBar.top, 1, rcBar.Height(), Colors.m_crResizebarEdge );
		dc.FillSolidRect( rcBar.left + 1, rcBar.top, 1, rcBar.Height(), Colors.m_crResizebarHighlight );
		dc.FillSolidRect( rcBar.right - 1, rcBar.top, 1, rcBar.Height(), Colors.m_crResizebarShadow );
		dc.FillSolidRect( rcBar.left + 2, rcBar.top, rcBar.Width() - 3, rcBar.Height(), Colors.m_crResizebarFace );
		dc.ExcludeClipRect( &rcBar );

		rcBar.SetRect( rcBar.right, rcClient.top, rcClient.right, rcClient.top + HEADER_HEIGHT );

		if ( dc.RectVisible( &rcBar ) )
			PaintListHeader( dc, rcBar );
	}

	if ( m_bStatusVisible )
	{
		CRect rcStatus( &m_rcStatus );
		if ( dc.RectVisible( &rcStatus ) )
			PaintStatus( dc, rcStatus );
	}

	if ( dc.RectVisible( &m_rcVideo ) /*&& ! m_pPlayer*/ )
		PaintSplash( dc, m_rcVideo );
	//else
	//	dc.FillSolidRect( &m_rcVideo, Colors.m_crMediaWindowBack );
	// Mediaplayer plugin handles painting of m_rcVideo rectangular itself
	// ToDo: Fix unhandled audio files display

	dc.SelectObject( pOldFont );
}

void CMediaFrame::PaintSplash(CDC& dc, CRect& /*rcBar*/)
{
	if ( ! m_bmLogo.m_hObject )
	{
		m_bmLogo.m_hObject = Skin.GetWatermark( L"LargeLogo" );
		if ( m_pPlayer && m_bmLogo.m_hObject )
		{
			m_pPlayer->SetLogoBitmap( m_bmLogo );
		}
		else
		{
			dc.FillSolidRect( &m_rcVideo, Colors.m_crMediaWindowBack );
			return;
		}
	}

	BITMAP pInfo;
	m_bmLogo.GetBitmap( &pInfo );

	CPoint pt = m_rcVideo.CenterPoint();
	pt.x -= pInfo.bmWidth / 2;
	pt.y -= ( pInfo.bmHeight + 32 ) / 2;

	CDC dcMem;
	dcMem.CreateCompatibleDC( &dc );
	CBitmap* pOldBmp = (CBitmap*)dcMem.SelectObject( &m_bmLogo );
	dc.BitBlt( pt.x, pt.y, pInfo.bmWidth, pInfo.bmHeight, &dcMem, 0, 0, SRCCOPY );
	dcMem.SelectObject( pOldBmp );

	dc.ExcludeClipRect( pt.x, pt.y, pt.x + pInfo.bmWidth, pt.y + pInfo.bmHeight );
	dc.FillSolidRect( &m_rcVideo, Colors.m_crMediaWindowBack );

	// Splash Sub-Text Display

	CRect rcText( m_rcVideo.left, pt.y + pInfo.bmHeight, m_rcVideo.right, pt.y + pInfo.bmHeight + 32 );

	CString strText = LoadString( IDS_MEDIA_TITLE );
	if ( strText.GetLength() < 50 )		// Add spacing to translations
	{
		for ( int nPos = strText.GetLength() - 1; nPos > 0; nPos-- )
		{
			strText.Insert( nPos, L"    " );
		}
	}

	pt.x = ( m_rcVideo.left + m_rcVideo.right ) / 2 - dc.GetTextExtent( strText ).cx / 2;
	pt.y = rcText.top + 8;

	dc.SetBkColor( Colors.m_crMediaWindowBack );
	dc.SetTextColor( Colors.m_crMediaWindowText );
	dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE, &m_rcVideo, strText, NULL );
	//dc.ExcludeClipRect( &rcText );
}

void CMediaFrame::PaintListHeader(CDC& dc, CRect& rcBar)
{
	CString strText = LoadString( IDS_MEDIA_PLAYLIST );
	CSize szText = dc.GetTextExtent( strText );
	CPoint pt = rcBar.CenterPoint();
	pt.x -= szText.cx / 2;
	pt.y -= szText.cy / 2 + 1;

	CBitmap bmHeader;
	if ( Skin.GetWatermark( &bmHeader, L"CMediaList.Header" ) )	// ToDo: Load once
	{
		if ( CoolInterface.DrawWatermark( &dc, &rcBar, &bmHeader ) )
		{
			dc.SetBkMode( TRANSPARENT );
			dc.SetTextColor( Colors.m_crMediaPanelCaptionText );
			dc.ExtTextOut( pt.x, pt.y, ETO_CLIPPED, &rcBar, strText, NULL );
			return;
		}
	}

	// No skinned image:
	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( Colors.m_crMediaPanelCaptionBack );
	dc.SetTextColor( Colors.m_crMediaPanelCaptionText );
	dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE|ETO_CLIPPED, &rcBar, strText, NULL );
}

void CMediaFrame::PaintStatus(CDC& dc, CRect& rcBar)
{
	COLORREF crBack = Colors.m_crMediaStatusBack;
	COLORREF crText = Colors.m_crMediaStatusText;

	dc.SelectObject( &m_pFontValue );
	DWORD dwOptions = Settings.General.LanguageRTL ? ETO_RTLREADING : 0;

	int nY = ( rcBar.top + rcBar.bottom ) / 2 - dc.GetTextExtent( L"Cy" ).cy / 2 - 1;
	CRect rcPart( &rcBar );
	CSize sz;
	CString str;

	BOOL bSkinned = FALSE;	// ToDo: Fix skinning causes flicker

	//if ( Skin.GetWatermark( &bmStatusBar, L"CMediaFrame.StatusBar" ) )
	if ( Images.m_bmMediaStatusBar.m_hObject )
		bSkinned = CoolInterface.DrawWatermark( &dc, &rcBar, &Images.m_bmMediaStatusBar );

	int nState = 0;
	if ( m_nState >= smsPlaying )
		nState = 2;
	else if ( m_nState >= smsPaused )
		nState = 1;
	ImageList_DrawEx( m_pIcons, nState, dc, rcBar.left + 2,
		( rcBar.top + rcBar.bottom ) / 2 - 8, 16, 16,
		( bSkinned ? CLR_NONE : crBack ), CLR_NONE, ILD_NORMAL );

	if ( bSkinned )
	{
		dc.SetBkMode( TRANSPARENT );
		dc.SetTextColor( crText );
	}
	else // Default
	{
		dc.ExcludeClipRect( rcBar.left + 2, ( rcBar.top + rcBar.bottom ) / 2 - 8,
			rcBar.left + 18, ( rcBar.top + rcBar.bottom ) / 2 + 8 );

		dc.SetBkMode( OPAQUE );
		dc.SetBkColor( crBack );
		dc.SetTextColor( crText );
	}

	CMetaItem* pItem = m_pMetadata.GetFirst();
	if ( pItem && IsDisplayMeta( pItem ) )
	{
		dc.SelectObject( &m_pFontKey );
		str 			= Settings.General.LanguageRTL ? ':' + pItem->m_sKey : pItem->m_sKey + ':';
		sz				= dc.GetTextExtent( str );
		rcPart.left 	= rcBar.left + 20;
		rcPart.right	= rcPart.left + sz.cx + 8;
		dc.ExtTextOut( rcPart.left + 4, nY, ( bSkinned ? 0 : ETO_OPAQUE )|ETO_CLIPPED|dwOptions, &rcPart, str, NULL );
		dc.ExcludeClipRect( &rcPart );

		dc.SelectObject( &m_pFontValue );
		sz				= dc.GetTextExtent( pItem->m_sValue );
		rcPart.left		= rcPart.right;
		rcPart.right	= rcPart.left + sz.cx + 8;
		dc.ExtTextOut( rcPart.left + 4, nY, ( bSkinned ? 0 : ETO_OPAQUE )|ETO_CLIPPED|dwOptions, &rcPart, pItem->m_sValue, NULL );
		dc.ExcludeClipRect( &rcPart );
	}
	else
	{
		if ( m_nState >= smsOpen )
		{
			int nSlash = m_sFile.ReverseFind( L'\\' );
			str = nSlash >= 0 ? m_sFile.Mid( nSlash + 1 ) : m_sFile;
		}
		else
		{
			LoadString( str, IDS_MEDIA_EMPTY );
		}

		sz = dc.GetTextExtent( str );
		rcPart.left  = rcBar.left + 20;
		rcPart.right = rcPart.left + sz.cx + 8;
		dc.ExtTextOut( rcPart.left + 4, nY, ( bSkinned ? 0 : ETO_OPAQUE )|ETO_CLIPPED|dwOptions, &rcPart, str, NULL );
		dc.ExcludeClipRect( &rcPart );
	}

	if ( m_nState >= smsOpen )
	{
		CString strFormat = L"%.2i:%.2i" + LoadString( IDS_MEDIA_TIMESPLIT ) + L"%.2i:%.2i";
		if ( Settings.General.LanguageRTL ) strFormat = L"\x200F" + strFormat;

		str.Format( strFormat,
			(int)( ( m_nPosition / ONE_SECOND ) / 60 ),
			(int)( ( m_nPosition / ONE_SECOND ) % 60 ),
			(int)( ( m_nLength / ONE_SECOND ) / 60 ),
			(int)( ( m_nLength / ONE_SECOND ) % 60 ) );

	//	if ( m_nSpeed != 1 )
	//	{
	//		strFormat = L"%.1fX  ";
	//		strFormat.Format( m_nSpeed );
	//		str = strFormat + str;
	//	}

		sz = dc.GetTextExtent( str );
		rcPart.right = rcBar.right;
		rcPart.left  = rcPart.right - sz.cx - 8;

		dc.ExtTextOut( rcPart.left + 4, nY, ( bSkinned ? 0 : ETO_OPAQUE )|ETO_CLIPPED|dwOptions, &rcPart, str, NULL );
		dc.ExcludeClipRect( &rcPart );
	}

	if ( ! bSkinned )
		dc.FillSolidRect( &rcBar, crBack );
	dc.SelectObject( &m_pFontDefault );
}

BOOL CMediaFrame::PaintStatusMicro(CDC& dc, CRect& rcBar)
{
	if ( m_nState <= smsOpen ) return FALSE;

	CString str;
	CRect rcStatus( &rcBar );
	CRect rcPart( &rcBar );
	CSize sz, size = rcBar.Size();
	CDC* pMemDC = CoolInterface.GetBuffer( dc, size );

	DWORD dwOptions = Settings.General.LanguageRTL ? DT_RTLREADING : 0;
	if ( m_nState >= smsOpen )
	{
		CString strFormat = L"%.2i:%.2i" + LoadString( IDS_MEDIA_TIMESPLIT ) + L"%.2i:%.2i";

		str.Format( strFormat,
			(int)( ( m_nPosition / ONE_SECOND ) / 60 ),
			(int)( ( m_nPosition / ONE_SECOND ) % 60 ),
			(int)( ( m_nLength / ONE_SECOND ) / 60 ),
			(int)( ( m_nLength / ONE_SECOND ) % 60 ) );

	//	if ( m_nSpeed != 1 )
	//	{
	//		strFormat = L"%.1fX  ";
	//		strFormat.Format( m_nSpeed );
	//		str = strFormat + str;
	//	}

		if ( Settings.General.LanguageRTL ) str = L"\x200F" + str;

		sz = pMemDC->GetTextExtent( str );
		rcPart.right	= rcStatus.right;
		rcPart.left		= rcPart.right - sz.cx - 2;
		rcStatus.right	= rcPart.left;

		pMemDC->DrawText( str, &rcPart, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_RIGHT );
	}

	CMetaItem* pItem = m_pMetadata.GetFirst();
	if ( pItem && IsDisplayMeta( pItem ) )
	{
		str = Settings.General.LanguageRTL ? ':' + pItem->m_sKey : pItem->m_sKey + ':';

		sz = pMemDC->GetTextExtent( str );
		rcPart.left 	= rcStatus.left;
		rcPart.right	= rcPart.left + sz.cx + 2;
		rcStatus.left	= rcPart.right;

		pMemDC->DrawText( str, &rcPart, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|dwOptions );
		pMemDC->DrawText( pItem->m_sValue, &rcStatus, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|dwOptions );
	}
	else if ( m_nState >= smsOpen )
	{
		int nSlash = m_sFile.ReverseFind( L'\\' );
		str = nSlash >= 0 ? m_sFile.Mid( nSlash + 1 ) : m_sFile;

		pMemDC->DrawText( str, &rcStatus, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|dwOptions );
	}

	if ( Settings.General.LanguageRTL )
		dc.StretchBlt( rcBar.Width() + rcBar.left, rcBar.top, -rcBar.Width(), rcBar.Height(),
			pMemDC, rcBar.left, rcBar.top, rcBar.Width(), rcBar.Height(), SRCCOPY );
	else
		dc.BitBlt( rcBar.left, rcBar.top, rcBar.Width(), rcBar.Height(),
			pMemDC, rcBar.left, rcBar.top, SRCCOPY );

	return TRUE;
}

BOOL CMediaFrame::IsDisplayMeta(CMetaItem* pItem)
{
	return _tcsicmp( pItem->m_sKey, L"duration" ) != 0 &&
		( pItem->m_sValue.GetLength() > 5 ||
		_tcsicmp( pItem->m_sKey, L"year" ) == 0 ||
		_tcsicmp( pItem->m_sKey, L"title" ) == 0 ||
		_tcsicmp( pItem->m_sKey, L"artist" ) == 0 ||
		_tcsicmp( pItem->m_sKey, L"album" ) == 0 ||
		_tcsicmp( pItem->m_sKey, L"track" ) == 0 ||
		_tcsicmp( pItem->m_sKey, L"series" ) == 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame interaction message handlers

void CMediaFrame::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	Skin.TrackPopupMenu( L"CMediaFrame", point,
		m_nState == smsPlaying ? ID_MEDIA_PAUSE : ID_MEDIA_PLAY );
}

void CMediaFrame::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		const DWORD tNow = GetTickCount();

		UpdateState();

		if ( m_bFullScreen && ! m_bListVisible )
		{
			CPoint ptCursor;
			GetCursorPos( &ptCursor );

			if ( ptCursor != m_ptCursor )
			{
				m_tBarTime = tNow;
				m_ptCursor = ptCursor;
				SetTimer( 2, 50, NULL );
			}
		}

		if ( tNow > m_tMetadata + META_DELAY )
		{
			m_tMetadata = tNow;
			m_pMetadata.Shuffle();
		}
	}
	else if ( nIDEvent == 2 )
	{
		OnSize( SIZE_BARSLIDE, 0, 0 );
	}
}

void CMediaFrame::OnClose()
{
	SetFullScreen( FALSE );
}

BOOL CMediaFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_bListVisible )
	{
		CRect rcClient, rc;
		CPoint point;

		GetCursorPos( &point );
		GetClientRect( &rcClient );
		ClientToScreen( &rcClient );

		rc.SetRect(	Settings.General.LanguageRTL ? rcClient.left + m_nListSize :
					rcClient.right - m_nListSize - Settings.Skin.Splitter,
					rcClient.top,
					Settings.General.LanguageRTL ? rcClient.left + m_nListSize + Settings.Skin.Splitter :
					rcClient.right - m_nListSize,
					rcClient.bottom - Settings.Skin.ToolbarHeight );

		if ( rc.PtInRect( point ) )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZEWE ) );
			return TRUE;
		}
	}
	else if ( m_bFullScreen )
	{
		DWORD tElapse = GetTickCount() - m_tBarTime;

		if ( tElapse > TOOLBAR_STICK + TOOLBAR_ANIMATE )
		{
			SetCursor( NULL );
			return TRUE;
		}
	}

	return CWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CMediaFrame::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcClient;
	GetClientRect( &rcClient );
	if ( theApp.m_bMenuWasVisible )
	{
		theApp.m_bMenuWasVisible = FALSE;
		return;
	}
	if ( m_bListVisible )
	{
		CRect rcBar(	rcClient.right - m_nListSize - Settings.Skin.Splitter,
						rcClient.top,
						rcClient.right - m_nListSize,
						rcClient.bottom );

		if ( rcBar.PtInRect( point ) )
		{
			DoSizeList();
			return;
		}
	}

	if ( ( m_bFullScreen && point.y <= STATUS_HEIGHT ) ||
		 ( ! m_bFullScreen && point.y >= rcClient.bottom - STATUS_HEIGHT - Settings.Skin.ToolbarHeight ) )
	{
		OnMediaStatus();
		return;
	}

	CRect rcSenseLess(	rcClient.right - m_nListSize - Settings.Skin.Splitter - 30,
						rcClient.top,
						rcClient.right - m_nListSize - Settings.Skin.Splitter,
						rcClient.bottom );

	if ( rcSenseLess.PtInRect( point ) )
		return;

	if ( m_nState == smsPlaying )
		OnMediaPause();
	else if ( m_nState == smsPaused )
		OnMediaPlay();

	CWnd::OnLButtonDown( nFlags, point );
}

void CMediaFrame::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnMediaFullScreen();

	if ( m_nState == smsPlaying )
		OnMediaPause();
	else if ( m_nState == smsPaused )
		OnMediaPlay();

	CWnd::OnLButtonDblClk( nFlags, point );
}

void CMediaFrame::OnMButtonDown(UINT nFlags, CPoint point)
{
	OnMediaFullScreen();

	CWnd::OnMButtonDown( nFlags, point );
}

BOOL CMediaFrame::DoSizeList()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;
	CRect rcClient;
	CPoint point;

	GetClientRect( &rcClient );
	ClientToScreen( &rcClient );
	ClipCursor( &rcClient );
	SetCapture();

	GetClientRect( &rcClient );

	int nOffset = 0xFFFF;

	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );

		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}

		GetCursorPos( &point );
		ScreenToClient( &point );

		int nSplit = rcClient.right - point.x;

		if ( nOffset == 0xFFFF )
			nOffset = m_nListSize - nSplit;
		nSplit += nOffset;

		nSplit = max( nSplit, 0 );
		nSplit = min( nSplit, rcClient.right - (int)Settings.Skin.Splitter );

		if ( nSplit < 8 )
			nSplit = 0;
		if ( nSplit > rcClient.right - Settings.Skin.Splitter - 8 )
			nSplit = rcClient.right - Settings.Skin.Splitter;

		if ( nSplit != m_nListSize )
		{
			m_nListSize = nSplit;
			OnSize( SIZE_INTERNAL, 0, 0 );
			Invalidate();
		}
	}

	ReleaseCapture();
	ClipCursor( NULL );

	return TRUE;
}

LRESULT CMediaFrame::OnMediaKey(WPARAM wParam, LPARAM lParam)
{
	if ( wParam != 1 && ! IsTopParentActive() ) return 0;
	if ( mixerGetNumDevs() < 1 ) return 0;

	int nVolumeTick = 0;
	int nVolumeDir = ( GET_APPCOMMAND_LPARAM( lParam ) == APPCOMMAND_VOLUME_DOWN ? -1 : 1 );

	switch ( GET_APPCOMMAND_LPARAM( lParam ) )
	{
	case APPCOMMAND_MEDIA_NEXTTRACK:
		GetOwner()->PostMessage( WM_COMMAND, ID_MEDIA_NEXT );
		return 1;
	case APPCOMMAND_MEDIA_PREVIOUSTRACK:
		GetOwner()->PostMessage( WM_COMMAND, ID_MEDIA_PREVIOUS );
		return 1;
	case APPCOMMAND_MEDIA_STOP:
		GetOwner()->PostMessage( WM_COMMAND, ID_MEDIA_STOP );
		return 1;
	case APPCOMMAND_VOLUME_MUTE:
		{
			MMRESULT result;
			HMIXER hMixer;
			// Obtain a handle to the mixer device
			result = mixerOpen( &hMixer, MIXER_OBJECTF_MIXER, 0, 0, 0 );
			if ( result != MMSYSERR_NOERROR ) return 0;

			// Get the speaker line of the mixer device
			MIXERLINE ml = {0};
			ml.cbStruct = sizeof( MIXERLINE );
			ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
			result = mixerGetLineInfo( reinterpret_cast<HMIXEROBJ>(hMixer), &ml, MIXER_GETLINEINFOF_COMPONENTTYPE );
			if ( result != MMSYSERR_NOERROR ) return 0;

			// Get the mute control of the speaker line
			MIXERLINECONTROLS mlc = {0};
			MIXERCONTROL mc = {0};
			mlc.cbStruct = sizeof( MIXERLINECONTROLS );
			mlc.dwLineID = ml.dwLineID;
			mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
			mlc.cControls = 1;
			mlc.pamxctrl = &mc;
			mlc.cbmxctrl = sizeof( MIXERCONTROL );
			result = mixerGetLineControls( reinterpret_cast<HMIXEROBJ>(hMixer), &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE );
			if ( result != MMSYSERR_NOERROR ) return 0;

			// Set 1 channel if it controls mute state for all channels
			if ( MIXERCONTROL_CONTROLF_UNIFORM & mc.fdwControl )
				ml.cChannels = 1;

			// Get the current mute values for all channels
			MIXERCONTROLDETAILS mcd = {0};
			MIXERCONTROLDETAILS_BOOLEAN* pmcd_b = new MIXERCONTROLDETAILS_BOOLEAN[ ml.cChannels ];
			mcd.cbStruct = sizeof( mcd );
			mcd.cChannels = ml.cChannels;
			mcd.cMultipleItems = mc.cMultipleItems;
			mcd.dwControlID = mc.dwControlID;
			mcd.cbDetails = sizeof( MIXERCONTROLDETAILS_BOOLEAN ) * ml.cChannels;
			mcd.paDetails = pmcd_b;
			result = mixerGetControlDetails( reinterpret_cast<HMIXEROBJ>(hMixer), &mcd,
				MIXER_GETCONTROLDETAILSF_VALUE );

			if ( result == MMSYSERR_NOERROR )
			{
				// Change mute values for all channels
				LONG lNewValue = LONG( pmcd_b->fValue == 0 );
				while ( ml.cChannels-- )
					pmcd_b[ ml.cChannels ].fValue = lNewValue;

				// Set the mute status
				result = mixerSetControlDetails( reinterpret_cast<HMIXEROBJ>(hMixer), &mcd, MIXER_SETCONTROLDETAILSF_VALUE );
			}
			delete [] pmcd_b;
		}
		// Now mute Envy player control (probably not needed)
		GetOwner()->PostMessage( WM_COMMAND, ID_MEDIA_MUTE );
		return 1;
	case APPCOMMAND_VOLUME_DOWN:
	case APPCOMMAND_VOLUME_UP:
		KillTimer( 1 );
		nVolumeTick = m_wndVolume.GetPos() + (nVolumeDir * VOLUME_KEY_MULTIPLIER);
		if ( nVolumeDir == -1 && nVolumeTick >= 0 ||
			 nVolumeDir == 1 && nVolumeTick <= 100 )
			 m_wndVolume.SetPos( nVolumeTick );
		else
			nVolumeTick = nVolumeDir == -1 ? 0 : 100;
		Settings.MediaPlayer.Volume = (double)nVolumeTick / 100.0f;
		if ( m_pPlayer )
			m_pPlayer->SetVolume( Settings.MediaPlayer.Volume );
		UpdateState();
		SetTimer( 1, UPDATE_TIMER, NULL );
		return 1;
	case APPCOMMAND_MEDIA_PLAY_PAUSE:
		GetOwner()->PostMessage( WM_COMMAND, m_nState == smsPlaying ? ID_MEDIA_PAUSE : ID_MEDIA_PLAY );
		return 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame thumb bars

void CMediaFrame::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if ( pScrollBar == (CScrollBar*)&m_wndVolume )
	{
		double nVolume = (double)m_wndVolume.GetPos() / 100.0f;

		if ( nVolume != Settings.MediaPlayer.Volume )
		{
			Settings.MediaPlayer.Volume = nVolume;
			if ( m_pPlayer )
				m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
		}
	}

	if ( ! m_pPlayer ) return;

	MediaState nState = smsNull;
	if ( FAILED( m_pPlayer->GetState( &nState ) ) ) return;
	if ( nState < smsOpen ) return;

	if ( pScrollBar == (CScrollBar*)&m_wndPosition )
	{
		LONGLONG nLength = 0;
		if ( FAILED( m_pPlayer->GetLength( &nLength ) ) ) return;
		nLength /= TIME_FACTOR;

		LONGLONG nPosition = 0;
		if ( FAILED( m_pPlayer->GetPosition( &nPosition ) ) ) return;
		nPosition /= TIME_FACTOR;

		switch ( nSBCode )
		{
		case TB_TOP:
			nPosition = 0;
			break;
		case TB_BOTTOM:
			nPosition = nLength;
			break;
		case TB_LINEUP:
			nPosition -= 5;
			break;
		case TB_LINEDOWN:
			nPosition += 5;
			break;
		case TB_PAGEUP:
		case TB_PAGEDOWN:
			{
				CRect rc1, rc2;
				CPoint pt;

				GetCursorPos( &pt );
				pScrollBar->GetWindowRect( &rc1 );
				((CSliderCtrl*)pScrollBar)->GetChannelRect( &rc2 );

				if ( rc1.PtInRect( pt ) )
				{
					rc2.OffsetRect( rc1.left, rc1.top );
					if ( pt.x <= rc2.left )
						nPosition = 0;
					else if ( pt.x >= rc2.right )
						nPosition = nLength;
					else
						nPosition = (LONGLONG)( (double)( pt.x - rc2.left ) / (double)rc2.Width() * (double)nLength );
				}
			}
			break;
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
			nPosition = (int)nPos;
			break;
		}

		if ( nState == smsOpen ) nPosition = 0;
		if ( nPosition < 0 ) nPosition = 0;
		if ( nPosition > nLength ) nPosition = nLength;

		if ( nState == smsPlaying )
		{
			m_pPlayer->Pause();
			m_bThumbPlay = TRUE;
		}

		m_pPlayer->SetPosition( nPosition * TIME_FACTOR );
		m_wndPosition.SetPos( (int)nPosition );

		if ( m_bThumbPlay && nSBCode == TB_ENDTRACK )
		{
			m_pPlayer->Play();
			m_bThumbPlay = FALSE;
		}

		UpdateState();
		UpdateWindow();
	}
	else if ( pScrollBar == (CScrollBar*)&m_wndSpeed )
	{
		double nNewSpeed = (double)m_wndSpeed.GetPos() / 100.0f;
		double nOldSpeed;

		if ( nSBCode == TB_TOP || nSBCode == TB_BOTTOM )
		{
			nNewSpeed = 1.0f;
			m_wndSpeed.SetPos( 100 );
		}

		m_pPlayer->GetSpeed( &nOldSpeed );
		if ( nNewSpeed != nOldSpeed )
			m_pPlayer->SetSpeed( nNewSpeed );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame command handlers

void CMediaFrame::OnUpdateMediaClose(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetItemCount() > 0 );
}

void CMediaFrame::OnMediaClose()
{
	Cleanup();
	m_wndList.Clear();
}

void CMediaFrame::OnUpdateMediaPlay(CCmdUI* pCmdUI)
{
	MediaState nState = m_nState;
	if ( m_bThumbPlay && nState == smsPaused )
		nState = smsPlaying;
	pCmdUI->Enable( m_nState < smsPlaying );
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pItem->Show( nState != smsPlaying );
}

void CMediaFrame::OnMediaPlay()
{
	if ( m_nState < smsOpen )
	{
		if ( m_wndList.GetCount() == 0 )
			PostMessage( WM_COMMAND, ID_MEDIA_OPEN );
		else
			m_wndList.GetNext();
	}
	else
	{
		if ( m_pPlayer != NULL ) m_pPlayer->Play();
		UpdateState();
	}

	UpdateNowPlaying();
}

void CMediaFrame::OnUpdateMediaPause(CCmdUI* pCmdUI)
{
	MediaState nState = m_nState;
	if ( m_bThumbPlay && nState == smsPaused )
		nState = smsPlaying;
	pCmdUI->Enable( nState == smsPlaying );
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pItem->Show( nState == smsPlaying );
}

void CMediaFrame::OnMediaPause()
{
	if ( m_pPlayer ) m_pPlayer->Pause();
	UpdateState();

	EnableScreenSaver();

	UpdateNowPlaying(TRUE);
}

void CMediaFrame::OnUpdateMediaStop(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_nState > smsOpen );
}

void CMediaFrame::OnMediaStop()
{
	if ( m_pPlayer ) m_pPlayer->Stop();
	m_bStopFlag = TRUE;
	m_wndList.Reset();
	EnableScreenSaver();

	UpdateNowPlaying(TRUE);
}

void CMediaFrame::OnUpdateMediaFullScreen(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bFullScreen );
}

void CMediaFrame::OnMediaFullScreen()
{
	SetFullScreen( ! m_bFullScreen );
}

void CMediaFrame::OnMediaZoom()
{
	if ( CMenu* pMenu = Skin.GetMenu( L"CMediaFrame.Zoom" ) )
	{
		m_wndToolBar.ThrowMenu( ID_MEDIA_ZOOM, pMenu );
	}
}

void CMediaFrame::OnUpdateMediaSizeFill(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == smzFill );
}

void CMediaFrame::OnMediaSizeFill()
{
	ZoomTo( smzFill );
}

void CMediaFrame::OnUpdateMediaSizeDistort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == smzDistort );
}

void CMediaFrame::OnMediaSizeDistort()
{
	ZoomTo( smzDistort );
}

void CMediaFrame::OnUpdateMediaSizeOne(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == 1 );
}

void CMediaFrame::OnMediaSizeOne()
{
	ZoomTo( (MediaZoom)1 );
}

void CMediaFrame::OnUpdateMediaSizeTwo(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == 2 );
}

void CMediaFrame::OnMediaSizeTwo()
{
	ZoomTo( (MediaZoom)2 );
}

void CMediaFrame::OnUpdateMediaSizeHalf(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == smzHalf );
}

void CMediaFrame::OnMediaSizeHalf()
{
	ZoomTo( (MediaZoom)4 );
}

void CMediaFrame::OnUpdateMediaAspectDefault(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.MediaPlayer.Zoom != smzDistort );
	pCmdUI->SetCheck( Settings.MediaPlayer.Aspect == smaDefault );
}

void CMediaFrame::OnMediaAspectDefault()
{
	AspectTo( smaDefault );
}

void CMediaFrame::OnUpdateMediaAspect43(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.MediaPlayer.Zoom != smzDistort );
	pCmdUI->SetCheck( fabs( Settings.MediaPlayer.Aspect - 4.0 / 3.0 ) < 0.1 );
}

void CMediaFrame::OnMediaAspect43()
{
	AspectTo( 4.0 / 3.0 );
}

void CMediaFrame::OnUpdateMediaAspect169(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.MediaPlayer.Zoom != smzDistort );
	pCmdUI->SetCheck( fabs( Settings.MediaPlayer.Aspect - 16.0 / 9.0 ) < 0.1 );
}

void CMediaFrame::OnMediaAspect169()
{
	AspectTo( 16.0 /9.0 );
}

void CMediaFrame::OnUpdateMediaPlaylist(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bListVisible );
}

void CMediaFrame::OnMediaPlaylist()
{
	m_bListVisible = ! m_bListVisible;
	m_tBarTime = GetTickCount();
	OnSize( SIZE_INTERNAL, 0, 0 );
}

void CMediaFrame::OnUpdateMediaStatus(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bStatusVisible );
}

void CMediaFrame::OnMediaStatus()
{
	m_bStatusVisible = ! m_bStatusVisible;
	OnSize( SIZE_INTERNAL, 0, 0 );
}

void CMediaFrame::OnUpdateMediaVis(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bFullScreen );
}

void CMediaFrame::OnMediaVis()
{
	CMediaVisDlg dlg( this );
	if ( dlg.DoModal() == IDOK )
		PrepareVis();
}

void CMediaFrame::OnUpdateMediaSettings(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bFullScreen );
}

void CMediaFrame::OnMediaSettings()
{
	CSettingsManagerDlg::Run( L"CMediaSettingsPage" );
}

void CMediaFrame::OnUpdateMediaMute(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bMute );
}

void CMediaFrame::OnMediaMute()
{
	m_bMute = ! m_bMute;

	if ( m_pPlayer )
		m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame public media operations

BOOL CMediaFrame::PlayFile(LPCTSTR pszFile)
{
	UpdateWindow();

	const DWORD tNow = GetTickCount();
	BOOL bTimeout = tNow > m_tLastPlay + 500;

	m_tLastPlay = tNow;

	if ( ! bTimeout )
		return EnqueueFile( pszFile );

	return m_wndList.Open( pszFile );
}

BOOL CMediaFrame::EnqueueFile(LPCTSTR pszFile)
{
	m_bEnqueue = TRUE;
	BOOL bResult = m_wndList.Enqueue( pszFile, TRUE );
	m_bLastMedia = FALSE;
	m_bLastNotPlayed = TRUE;
	m_bEnqueue = FALSE;
	return bResult;
}

BOOL CMediaFrame::IsPlaying()
{
	return m_pPlayer != NULL && m_nState == smsPlaying;
}

void CMediaFrame::OnFileDelete(LPCTSTR pszFile)
{
	// Only remove from the list, the player cleans up itself
	if ( m_sFile.CompareNoCase( pszFile ) == 0 )
		m_wndList.Remove( pszFile );
}

float CMediaFrame::GetPosition()
{
	if ( m_pPlayer && m_nState >= smsOpen && m_nLength > 0 )
		return (float)m_nPosition / (float)m_nLength;

	return 0;
}

BOOL CMediaFrame::SeekTo(float nPosition)
{
	if ( m_pPlayer && m_nState >= smsPaused && m_nLength > 0 )
	{
		m_nPosition = (LONGLONG)( nPosition * (float)m_nLength );
		m_pPlayer->SetPosition( m_nPosition );
		OnTimer( 1 );
		return TRUE;
	}

	return FALSE;
}

float CMediaFrame::GetVolume()
{
	return (float)Settings.MediaPlayer.Volume;
}

BOOL CMediaFrame::SetVolume(float nVolume)
{
	Settings.MediaPlayer.Volume = (double)nVolume;
	if ( m_pPlayer )
		m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
	OnTimer( 1 );
	return ( m_pPlayer != NULL );
}

// ToDo: Implement these?
//void CMediaFrame::OffsetVolume(int nVolumeOffset)
//{
//	KillTimer( 1 );
//	int nVolumeTick = max( min( m_wndVolume.GetPos() + nVolumeOffset, 100 ), 0 );
//	m_wndVolume.SetPos( nVolumeTick );
//	Settings.MediaPlayer.Volume = (double)nVolumeTick / 100.0f;
//	if ( m_pPlayer )
//		m_pPlayer->SetVolume( Settings.MediaPlayer.Volume );
//	UpdateState();
//	SetTimer( 1, UPDATE_TIMER, NULL );
//}

//void CMediaFrame::OffsetPosition(int nPositionOffset)
//{
//	KillTimer( 1 );
//	if ( m_pPlayer )
//	{
//		bool bPlaying = ( m_nState == smsPlaying );
//		if ( bPlaying )
//			m_pPlayer->Pause();
//		LONGLONG nPos = 0, nLen = 0;
//		m_pPlayer->GetPosition( &nPos );
//		m_pPlayer->GetLength( &nLen );
//		nPos = max( min( nPos + nPositionOffset * TIME_FACTOR, nLen ), 0 );
//		m_pPlayer->SetPosition( nPos );
//		if ( bPlaying )
//			m_pPlayer->Play();
//	}
//	UpdateState();
//	SetTimer( 1, UPDATE_TIMER, NULL );
//}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame private media operations

BOOL CMediaFrame::Prepare()
{
	m_bThumbPlay = FALSE;

	if ( m_pPlayer ) return TRUE;
	if ( GetSafeHwnd() == NULL ) return FALSE;

	CWaitCursor pCursor;
	CLSID pCLSID;

	if ( Plugins.LookupCLSID( L"MediaPlayer", L"Default", pCLSID ) )
	{
		HINSTANCE hRes = AfxGetResourceHandle();
		CoCreateInstance( pCLSID, NULL, CLSCTX_ALL, IID_IMediaPlayer, (void**)&m_pPlayer );
		AfxSetResourceHandle( hRes );
	}

	if ( m_pPlayer == NULL )
	{
		pCursor.Restore();
		MsgBox( IDS_MEDIA_PLUGIN_CREATE, MB_ICONEXCLAMATION );
		return FALSE;
	}

	CoLockObjectExternal( m_pPlayer, TRUE, TRUE );
	ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	m_pPlayer->Create( GetSafeHwnd() );		// (LONG_PTR) ?
	m_pPlayer->SetZoom( Settings.MediaPlayer.Zoom );
	m_pPlayer->SetAspect( Settings.MediaPlayer.Aspect );
	m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
	if ( Settings.General.LanguageRTL ) ModifyStyleEx( 0, WS_EX_LAYOUTRTL, 0 );

	if ( m_bmLogo.m_hObject )
		m_pPlayer->SetLogoBitmap( (HBITMAP)m_bmLogo.m_hObject );

	HINSTANCE hRes = AfxGetResourceHandle();
	//PrepareVis();
	AfxSetResourceHandle( hRes );

	OnSize( SIZE_INTERNAL, 0, 0 );
	UpdateState();

	return TRUE;
}

// ToDo: Re-Implement Audio Visualizations!
BOOL CMediaFrame::PrepareVis()
{
	// Note visualizations currently unavailable
	if ( Settings.MediaPlayer.VisPath.IsEmpty() || Settings.MediaPlayer.VisCLSID.IsEmpty() )
		return FALSE;

	CLSID pCLSID;
	if ( Hashes::fromGuid( Settings.MediaPlayer.VisCLSID, &pCLSID ) &&
		 Plugins.LookupEnable( pCLSID ) )
	{
		CComPtr< IAudioVisPlugin > pPlugin;
		HRESULT hr = pPlugin.CoCreateInstance( pCLSID );
		if ( SUCCEEDED( hr ) && pPlugin )
		{
			//if ( ! Settings.MediaPlayer.VisPath.IsEmpty() )
			{
				CComQIPtr< IWrappedPluginControl > pWrap( pPlugin );
				if ( pWrap )
				{
					hr = pWrap->Load( CComBSTR( Settings.MediaPlayer.VisPath ), 0 );
					if ( FAILED( hr ) )
						return FALSE;
				}
			}

			if ( ! m_pPlayer )
				return FALSE;

			hr = m_pPlayer->SetPluginSize( Settings.MediaPlayer.VisSize );
			if ( FAILED( hr ) )
				return FALSE;

			m_pPlayer->SetPlugin( pPlugin );
		}
	}

	return TRUE;
}

BOOL CMediaFrame::OpenFile(LPCTSTR pszFile)
{
	if ( ! Prepare() ) return FALSE;

	if ( m_sFile == pszFile )
	{
		m_pPlayer->Stop();
		UpdateState();
		return TRUE;
	}

	CWaitCursor pCursor;
	m_sFile.Empty();
	m_pMetadata.Clear();

	HINSTANCE hRes = AfxGetResourceHandle();

	BSTR bsFile = CString( pszFile ).AllocSysString();
	HRESULT hr = PluginPlay( bsFile );

	SysFreeString( bsFile );

	AfxSetResourceHandle( hRes );

	UpdateState();
	pCursor.Restore();
	m_tMetadata = GetTickCount();

	if ( FAILED(hr) )
	{
		LPCTSTR pszBase = _tcsrchr( pszFile, '\\' );
		pszBase = pszBase ? pszBase + 1 : pszFile;
		CString strMessage, strFormat;
		LoadString( strFormat, IDS_MEDIA_LOAD_FAIL );
		strMessage.Format( strFormat, pszBase );
		m_pMetadata.Add( L"Error", strMessage );
		LoadString( strMessage, IDS_MEDIA_LOAD_FAIL_HELP );
		m_pMetadata.Add( L"Error", strMessage );
		return FALSE;
	}

	m_sFile = pszFile;

	{
		CSingleLock oLock( &Library.m_pSection, TRUE );

		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( pszFile ) )
		{
			m_pMetadata.Add( L"Filename", pFile->m_sName );
			m_pMetadata.Setup( pFile->m_pSchema, FALSE );
			m_pMetadata.Combine( pFile->m_pMetadata );
			m_pMetadata.Clean( 1024 );
			oLock.Unlock();

			CMetaItem* pWidth	= m_pMetadata.Find( L"Width" );
			CMetaItem* pHeight	= m_pMetadata.Find( L"Height" );

			if ( pWidth != NULL && pHeight != NULL )
			{
				pWidth->m_sKey = L"Dimensions";
				pWidth->m_sValue += 'x' + pHeight->m_sValue;
				m_pMetadata.Remove( L"Height" );
			}
		}
	}

	if ( hr != S_OK )
		m_pMetadata.Add( L"Warning", LoadString( IDS_MEDIA_PARTIAL_RENDER ) );

	return TRUE;
}

HRESULT CMediaFrame::PluginPlay(BSTR bsFilePath)
{
	__try
	{
		m_pPlayer->Stop();

		return m_pPlayer->Open( bsFilePath );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	//	theApp.Message( MSG_ERROR, L"Media Player failed to open file: %s", bsFilePath );
		Cleanup();
		return E_FAIL;
	}
}

void CMediaFrame::Cleanup()
{
	m_sFile.Empty();
	m_pMetadata.Clear();

	if ( m_pPlayer )
	{
		HINSTANCE hRes = AfxGetResourceHandle();
		__try
		{
			m_pPlayer->Close();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
		__try
		{
			m_pPlayer->Destroy();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
		CoLockObjectExternal( m_pPlayer, FALSE, TRUE );
		m_pPlayer = NULL;
		AfxSetResourceHandle( hRes );
	}

	UpdateState();
	Invalidate();

	EnableScreenSaver();
}

void CMediaFrame::ZoomTo(MediaZoom nZoom)
{
	if ( Settings.MediaPlayer.Zoom == nZoom ) return;
	Settings.MediaPlayer.Zoom = nZoom;
	if ( ! m_pPlayer ) return;

	HRESULT hr = m_pPlayer->SetZoom( Settings.MediaPlayer.Zoom );
	if ( FAILED( hr ) )
		Cleanup();	// TRUE?
}

void CMediaFrame::AspectTo(double nAspect)
{
	if ( Settings.MediaPlayer.Aspect == nAspect ) return;
	Settings.MediaPlayer.Aspect = nAspect;
	if ( m_pPlayer == NULL ) return;

	HRESULT hr = m_pPlayer->SetAspect( Settings.MediaPlayer.Aspect );
	if ( FAILED( hr ) )
		Cleanup();	// TRUE
}

void CMediaFrame::UpdateState()
{
	// On UPDATE_TIMER
	m_nState = smsNull;

	if ( m_pPlayer ) m_pPlayer->GetState( &m_nState );

	if ( m_nState >= smsOpen )
	{
		m_nLength = 0;
		if ( m_pPlayer ) m_pPlayer->GetLength( &m_nLength );
		const int nLength = (int)( m_nLength / TIME_FACTOR );

		m_nPosition = 0;
		if ( m_pPlayer ) m_pPlayer->GetPosition( &m_nPosition );
		const int nPosition = (int)( m_nPosition / TIME_FACTOR );

		if ( ! m_wndPosition.IsWindowEnabled() )
			m_wndPosition.EnableWindow( TRUE );
		m_wndPosition.SetRangeMax( nLength );
		m_wndPosition.SetPos( nPosition );

		double nSpeed = 1.0f;
		m_pPlayer->GetSpeed( &nSpeed );
		m_wndSpeed.SetPos( (int)( nSpeed * 100 ) );
		if ( ! m_wndSpeed.IsWindowEnabled() )
			m_wndSpeed.EnableWindow( TRUE );

		if ( ! m_bMute )
		{
			Settings.MediaPlayer.Volume = 1.0f;
			if ( m_pPlayer ) m_pPlayer->GetVolume( &Settings.MediaPlayer.Volume );
		}

		if ( m_nState == smsPlaying && nPosition >= nLength && nPosition != 0 )
			m_wndList.GetNext();
	}
	else
	{
		if ( m_nState != smsNull && m_wndList.GetCount() == 0 )
			Cleanup();
		m_wndPosition.SetPos( 0 );
		m_wndPosition.SetRange( 0, 0 );
		if ( m_wndPosition.IsWindowEnabled() )
			m_wndPosition.EnableWindow( FALSE );
		m_wndSpeed.SetPos( 100 );
		if ( m_wndSpeed.IsWindowEnabled() )
			m_wndSpeed.EnableWindow( FALSE );
	}

	m_wndVolume.SetPos( (int)( Settings.MediaPlayer.Volume * 100 ) );

	if ( m_bStatusVisible )
		InvalidateRect( &m_rcStatus );
}

void CMediaFrame::OnNewCurrent(NMHDR* /*pNotify*/, LRESULT* pResult)
{
	int nCurrent = m_wndList.GetCurrent();
	m_wndList.UpdateWindow();
	m_wndPosition.SetPos( 0 );
	m_bLastMedia = ( nCurrent == m_wndList.GetItemCount() - 1 );
	m_bRepeat = Settings.MediaPlayer.Repeat;

	if ( m_bStopFlag )
	{
		m_bStopFlag = FALSE;
		m_bLastNotPlayed = FALSE;
		if ( m_pPlayer ) Cleanup();
		if ( ! m_bScreenSaverEnabled ) EnableScreenSaver();
		*pResult = 0;
		return;
	}

	//	Prior-track-cleanup workaround.  ToDo: Fix Mediaplayer plugin instead
	const double nVolume = Settings.MediaPlayer.Volume; 	// Current volume restored below
	if ( m_pPlayer )
	{
		m_pPlayer->Close();
		m_pPlayer->Destroy();
		m_pPlayer = NULL;
	}

	if ( nCurrent >= 0 )	// Not last in list
	{
		BOOL bPlayIt;
		BOOL bCorrupted = FALSE;

		if ( m_bEnqueue )
		{
			bPlayIt = FALSE;
		}
		else // Play
		{
			// Play when repeat is on or when whithin the playlist
			bPlayIt = m_bRepeat || ! m_bLastMedia || m_bLastNotPlayed;
			// The whole playlist was played and the list was reset
			if ( ! m_bRepeat && m_bLastNotPlayed && ! m_bLastMedia )
			{
				if ( m_nState != smsPlaying )
					bPlayIt = FALSE;
				else // New file was clicked, clear the flag
					m_bLastNotPlayed = FALSE;
			}

			if ( ! m_pPlayer || bPlayIt || m_wndList.GetItemCount() == 1 )
			{
				bPlayIt = TRUE;
				bCorrupted = ! OpenFile( m_wndList.GetPath( nCurrent ) );
			}
		}

		if ( bPlayIt && ! bCorrupted )
		{
			m_pPlayer->Play();

			if ( m_bScreenSaverEnabled ) DisableScreenSaver();
			// Check if last was not played; flag only when we are playing the file before it
			if ( ! m_bLastNotPlayed )
				m_bLastNotPlayed = ( nCurrent == m_wndList.GetItemCount() - 2 );
			UpdateState();

			// Prior-track-cleanup workaround fix.  ToDo: Remove these.
			Settings.MediaPlayer.Volume = nVolume;
			m_pPlayer->SetVolume( m_bMute ? 0 : nVolume );
		}
		else if ( bCorrupted )	// File was corrupted, move to the next file
		{
			nCurrent = m_wndList.GetNext( FALSE );
			if ( m_wndList.GetItemCount() != 1 )
				m_wndList.SetCurrent( nCurrent );
			else if ( m_pPlayer )
				Cleanup();		// Cleanup when no exception happened but file couldn't be opened
		}
		else
		{
			// Reset list and cleanup
			m_bLastNotPlayed = FALSE;
			m_wndList.Reset( TRUE );
			m_bStopFlag = FALSE;
			if ( m_pPlayer )
				Cleanup();
		}
	}
	else if ( m_wndList.GetItemCount() > 0 )	// List was reset; current file was set to -1
	{
		nCurrent = m_wndList.GetCurrent();		// Get file #0

		if ( m_pPlayer )
		{
			if ( ! m_bRepeat )
				m_bStopFlag = TRUE;
			else
				nCurrent = m_wndList.GetNext( FALSE );
			if ( ! m_bEnqueue )
				m_wndList.SetCurrent( nCurrent );
		}
	}
	else if ( m_pPlayer )
	{
		Cleanup();
	}

	*pResult = 0;

	UpdateNowPlaying();
}

/////////////////////////////////////////////////////////////////////////////
// Screen saver Enable / Disable functions

void CMediaFrame::DisableScreenSaver()
{
	if ( m_bScreenSaverEnabled )
	{
		GetActivePwrScheme( &m_nPowerSchemeId );				// Get ID of current power scheme
		GetCurrentPowerPolicies( &m_CurrentGP, &m_CurrentPP );	// Get active policies

		m_nVidAC = m_CurrentPP.user.VideoTimeoutAc;				// Save current values
		m_nVidDC = m_CurrentPP.user.VideoTimeoutDc;

		m_CurrentPP.user.VideoTimeoutAc = 0;					// Disallow display shutoff
		m_CurrentPP.user.VideoTimeoutDc = 0;

		// Set new values
		SetActivePwrScheme( m_nPowerSchemeId, &m_CurrentGP, &m_CurrentPP );

		BOOL bParam = FALSE;
		BOOL bRetVal = SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, 0, &bParam, 0 );
		if ( bRetVal && bParam )
		{
			// Turn off screen saver after saving current timeout value
			SystemParametersInfo( SPI_GETSCREENSAVETIMEOUT, 0, &m_nScreenSaverTime, 0 );
			SystemParametersInfo( SPI_SETSCREENSAVETIMEOUT, 0, NULL, 0 );
		}

		m_bScreenSaverEnabled = FALSE;
	}
}

void CMediaFrame::EnableScreenSaver()
{
	if ( m_bScreenSaverEnabled )
		return;

	// Restore previous values
	m_CurrentPP.user.VideoTimeoutAc = m_nVidAC;
	m_CurrentPP.user.VideoTimeoutDc = m_nVidDC;

	// Set original values
	SetActivePwrScheme( m_nPowerSchemeId, &m_CurrentGP, &m_CurrentPP );

	// Restore screen saver timeout value if it's not zero.
	// Otherwise if the screen saver was inactive, it toggles it to active state and shutoff stops working (MS bug?)
	if ( m_nScreenSaverTime > 0 )
		SystemParametersInfo( SPI_SETSCREENSAVETIMEOUT, m_nScreenSaverTime, NULL, 0 );

	m_bScreenSaverEnabled = TRUE;
}

void CMediaFrame::UpdateScreenSaverStatus(BOOL bWindowActive)
{
	if ( bWindowActive )
	{
		if ( m_bScreenSaverEnabled && IsPlaying() )
			DisableScreenSaver();
	}
	else
	{
	//	if ( ! m_bScreenSaverEnabled )
			EnableScreenSaver();
	}
}

void CMediaFrame::UpdateNowPlaying(BOOL bEmpty)
{
	if ( bEmpty )
	{
		m_sNowPlaying.Empty();
	}
	else
	{
		m_sNowPlaying = m_sFile;

		// Strip path
		LPCTSTR pszFileName = _tcsrchr( m_sNowPlaying, '\\' );
		if ( pszFileName )
		{
			const int nFileNameLen = static_cast< int >( _tcslen( pszFileName ) );
			m_sNowPlaying = m_sNowPlaying.Right( nFileNameLen - 1 );
		}

		// Strip extension
		LPCTSTR pszExt = _tcsrchr( m_sNowPlaying, '.' );
		if ( pszExt )
		{
			const int nFileNameLen = static_cast< int >( pszExt - m_sNowPlaying );
			m_sNowPlaying = m_sNowPlaying.Left( nFileNameLen );
		}
	}

	CRegistry::SetString( L"MediaPlayer", L"NowPlaying", m_sNowPlaying );

	//Plugins.OnEvent(EVENT_CHANGEDSONG);	// ToDo: Maybe plug-ins can be alerted in some way
}
