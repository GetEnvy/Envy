//
// DlgDownloadMonitor.cpp
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
#include "DlgDownloadMonitor.h"
#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"
#include "FragmentedFile.h"
#include "FragmentBar.h"
#include "GraphLine.h"
#include "GraphItem.h"
#include "Library.h"
#include "FileExecutor.h"
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "Colors.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "Plugins.h"
#include "WndDownloads.h"
#include "WndMain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


BEGIN_MESSAGE_MAP(CDownloadMonitorDlg, CSkinDialog)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_WM_SYSCOMMAND()
	ON_WM_CONTEXTMENU()
	ON_WM_INITMENUPOPUP()
	ON_MESSAGE(WM_TRAY, OnTray)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnNeedText)
	ON_BN_CLICKED(IDC_MONITOR_SHOW, OnDownloadShow)
	ON_BN_CLICKED(IDC_MONITOR_ACTION, OnDownloadAction)
	ON_BN_CLICKED(IDC_MONITOR_CLOSE, OnDownloadClose)
END_MESSAGE_MAP()

CList< CDownloadMonitorDlg* > CDownloadMonitorDlg::m_pWindows;


/////////////////////////////////////////////////////////////////////////////
// CDownloadMonitorDlg dialog

CDownloadMonitorDlg::CDownloadMonitorDlg(CDownload* pDownload)
	: CSkinDialog( CDownloadMonitorDlg::IDD, NULL )
	, m_pDownload	( pDownload )
	, m_pGraph		( NULL )
	, m_pItem		( NULL )
	, m_bTray		( FALSE )
	, m_bCompleted	( FALSE )
{
	CreateReal( IDD );

	m_pWindows.AddTail( this );
}

CDownloadMonitorDlg::~CDownloadMonitorDlg()
{
	if ( m_pGraph != NULL ) delete m_pGraph;
	if ( POSITION pos = m_pWindows.Find( this ) )
		m_pWindows.RemoveAt( pos );

#ifdef __ITaskbarList3_INTERFACE_DEFINED__	// VS2010+
	m_pTaskbar.Release();
#endif
}

void CDownloadMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange( pDX );
	//DDX_Control(pDX, IDC_MONITOR_STATUS, m_wndStatus);	// Removed
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_MONITOR_SOURCES, m_wndSources);
	DDX_Control(pDX, IDC_MONITOR_SPEED, m_wndSpeed);
	DDX_Control(pDX, IDC_MONITOR_TIME, m_wndTime);
	DDX_Control(pDX, IDC_MONITOR_VOLUME, m_wndVolume);
	DDX_Control(pDX, IDC_MONITOR_ICON, m_wndIcon);
	DDX_Control(pDX, IDC_MONITOR_GRAPH, m_wndGraph);
	DDX_Control(pDX, IDC_MONITOR_FILE, m_wndFile);
	DDX_Control(pDX, IDC_MONITOR_AUTOCLOSE, m_wndAutoClose);
	DDX_Control(pDX, IDC_MONITOR_ACTION, m_wndAction);
	DDX_Control(pDX, IDC_MONITOR_SHOW, m_wndShow);
	DDX_Control(pDX, IDC_MONITOR_CLOSE, m_wndClose);
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadMonitorDlg operations

BOOL CDownloadMonitorDlg::CreateReal(UINT nID)
{
	LPCTSTR lpszTemplateName = MAKEINTRESOURCE( nID );
	BOOL bResult = FALSE;
	HINSTANCE hInst = AfxFindResourceHandle( lpszTemplateName, RT_DIALOG );
	if ( HRSRC hResource = ::FindResource( hInst, lpszTemplateName, RT_DIALOG ) )
	{
		if ( HGLOBAL hTemplate = LoadResource( hInst, hResource ) )
		{
			LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource( hTemplate );
			if ( lpDialogTemplate )
				bResult = CreateDlgIndirect( lpDialogTemplate, NULL, hInst );
			FreeResource( hTemplate );
		}
	}
	return bResult;
}

void CDownloadMonitorDlg::OnSkinChange(BOOL bSet)
{
	for ( POSITION pos = m_pWindows.GetHeadPosition(); pos; )
	{
		CDownloadMonitorDlg* pDlg = m_pWindows.GetNext( pos );

		if ( bSet )
		{
			pDlg->SkinMe( L"CDownloadMonitorDlg", IDI_DOWNLOAD_MONITOR );
		//	pDlg->Invalidate(); 	// ToDo: Fix Banner Disappearing Here (from above)

			// Quick workaround hack: Don't paint missing banner, but obvious when size changes or moved offscreen  (Not Shareaza bug, ToDo: Fix this properly!)
			CRect rc;
			pDlg->GetClientRect( &rc );
			rc.top += Skin.m_nBanner;
			pDlg->InvalidateRect( rc );
		}
		else
		{
			pDlg->RemoveSkin();
		}

		pDlg->m_pGraph->m_crBack = Colors.m_crMonitorGraphBack;
	}
}

void CDownloadMonitorDlg::CloseAll()
{
	for ( POSITION pos = m_pWindows.GetHeadPosition(); pos; )
	{
		delete m_pWindows.GetNext( pos );
	}
	m_pWindows.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadMonitorDlg message handlers

BOOL CDownloadMonitorDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CDownloadMonitorDlg", IDI_DOWNLOAD_MONITOR );

	CMenu* pMenu = GetSystemMenu( FALSE );
	pMenu->InsertMenu( 0, MF_BYPOSITION|MF_SEPARATOR, ID_SEPARATOR );
	pMenu->InsertMenu( 0, MF_BYPOSITION|MF_STRING, SC_NEXTWINDOW, L"&Always on Top" );

	//CSingleLock pLock( &Transfers.m_pSection );
	//if ( SafeLock( pLock ) )
	//{
	//	m_sName = m_pDownload->m_sName;
	//	bTorrent = ( Downloads.Check( m_pDownload ) && m_pDownload->IsTorrent() &&
	//		( m_pDownload->IsMultiFileTorrent() || ! IsValidExtension( m_sName ) ) );
	//	pLock.Unlock();
	//}

	m_wndFile.SetWindowText( m_sName );
	if ( m_pDownload->IsTorrent() && ( m_pDownload->IsMultiFileTorrent() || ! IsValidExtension( m_sName ) ) )
		m_wndIcon.SetIcon( CoolInterface.ExtractIcon( IDI_MULTIFILE, FALSE, LVSIL_NORMAL ) );
	else
		m_wndIcon.SetIcon( ShellIcons.ExtractIcon( ShellIcons.Get( m_sName, 32 ), 32 ) );

	m_pGraph = new CLineGraph();
	m_pItem  = new CGraphItem( 0, 1.0f, Colors.m_crMonitorGraphLine );	// RGB( 252, 20, 10 )

	m_pGraph->m_bShowLegend		= FALSE;
	m_pGraph->m_bShowAxis		= FALSE;
	m_pGraph->m_nMinGridVert	= 16;
	m_pGraph->m_crGrid			= Colors.m_crMonitorGraphGrid;	// RGB( 230, 230, 180 )
	m_pGraph->m_crBack			= Colors.m_crMonitorGraphBack;	// RGB( 255, 255, 242 )

	m_pGraph->AddItem( m_pItem );

#ifdef __ITaskbarList3_INTERFACE_DEFINED__	// VS2010+
	if ( theApp.m_nWinVer >= WIN_7 )
		m_pTaskbar.CoCreateInstance( CLSID_TaskbarList );
#endif

	OnTimer( 2 );

	CenterWindow();
	ShowWindow( SW_SHOW );

	SetTimer( 1, Settings.Interface.RefreshRateGraph, NULL );	// Graph History, 72ms = 30s display
	SetTimer( 2, Settings.Interface.RefreshRateText, NULL );	// Text Update

	EnableToolTips();

	return TRUE;
}

void CDownloadMonitorDlg::OnDestroy()
{
	KillTimer( 1 );
	KillTimer( 2 );

	if ( m_pDownload != NULL )
	{
		CSingleLock pLock( &Transfers.m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( Downloads.Check( m_pDownload ) )
				m_pDownload->m_pMonitorWnd = NULL;
			m_pDownload = NULL;
		}
	}

	if ( m_bTray )
	{
		Shell_NotifyIcon( NIM_DELETE, &m_pTray );
		m_bTray = FALSE;
	}

	CSkinDialog::OnDestroy();
}

void CDownloadMonitorDlg::PostNcDestroy()
{
	CSkinDialog::PostNcDestroy();
	delete this;
}

void CDownloadMonitorDlg::OnTimer(UINT_PTR nIDEvent)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	if ( ! m_pDownload || ! Downloads.Check( m_pDownload ) )
	{
		KillTimer( 1 );
		KillTimer( 2 );
		PostMessage( WM_CLOSE );
		return;
	}

	if ( nIDEvent == 1 )		// Rapid Refresh Graph
	{
		if ( m_pDownload->IsPaused() )
			return;				// ToDo: Update once?

		DWORD nSpeed = m_pDownload->GetMeasuredSpeed();
		m_pItem->Add( nSpeed );
		m_pGraph->m_nUpdates++;
		m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );

		if ( nSpeed > 4000 && ! Settings.General.LanguageRTL )
		{
			CString strText;
			strText.Format( L"%s %s %s  (%.2f%%)",
				(LPCTSTR)Settings.SmartVolume( m_pDownload->GetVolumeComplete() ),
				(LPCTSTR)LoadString( IDS_GENERAL_OF ),
				(LPCTSTR)Settings.SmartVolume( m_pDownload->m_nSize ),
				m_pDownload->GetProgress() );

			Update( &m_wndVolume, strText );
		}

		CClientDC dc( this );
		DoPaint( dc );

		if ( m_bCompleted )
			KillTimer( 1 );

		return;
	}
	//else if ( nIDEvent == 2 )	// General Text Refresh Rate

	if ( m_bCompleted ) return;

	bool bCompleted = m_pDownload->IsCompleted();

	CString strText, strFormat, strOf, strNA;
	LoadString( strOf, IDS_GENERAL_OF );
	LoadString( strNA, IDS_TIP_NA );

	// Update file name if it was changed from the Advanced Edit dialog
	Update( &m_wndFile, m_pDownload->m_sName );

	if ( ! m_pDownload->IsStarted() )
		strText.Format( L"%s", (LPCTSTR)m_pDownload->m_sName );
	else if ( Settings.General.LanguageRTL )
		strText.Format( L"%s %s %.2f%%", (LPCTSTR)m_pDownload->m_sName, (LPCTSTR)strOf, m_pDownload->GetProgress() );
	else
		strText.Format( L"%.2f%% %s %s", m_pDownload->GetProgress(), (LPCTSTR)strOf, (LPCTSTR)m_pDownload->m_sName );

	Update( this, strText );

	if ( m_bTray )
	{
		if ( _tcsncmp( m_pTray.szTip, strText, _countof( m_pTray.szTip ) - 1 ) )
		{
			m_pTray.uFlags = NIF_TIP;
			_tcsncpy( m_pTray.szTip, strText, _countof( m_pTray.szTip ) - 1 );
			m_pTray.szTip[ _countof( m_pTray.szTip ) - 1 ] = L'\0';

			Shell_NotifyIcon( NIM_MODIFY, &m_pTray );
		}
	}

	if ( bCompleted )
	{
		if ( m_wndAutoClose.GetCheck() )
		{
			PostMessage( WM_CLOSE );
		}
		else
		{
			ShowWindow( SW_SHOWNORMAL );
			SetForegroundWindow();
		}

		m_bCompleted = TRUE;
	}
	else if ( m_bTray || IsIconic() )
	{
		return;
	}

	CString strAction;
	const int nSourceCount   = bCompleted ? 0 : m_pDownload->GetSourceCount();
	const int nTransferCount = bCompleted ? 0 : m_pDownload->GetTransferCount();

	if ( bCompleted )
	{
		LoadString( strAction, IDS_MONITOR_ACTION_OPEN );
		Update( &m_wndAction, TRUE );
	//	LoadString( strText, IDS_MONITOR_COMPLETED );
	//	Update( &m_wndStatus, strText );
		LoadString( strText, m_pDownload->IsSeeding() ? IDS_STATUS_SEEDING : IDS_MONITOR_COMPLETED_WORD );
		Update( &m_wndSources, strText );
		Update( &m_wndSpeed, L"--" );
		Update( &m_wndTime, L"--" );
	}
	else if ( m_pDownload->IsMoving() )
	{
		LoadString( strAction, IDS_STATUS_MOVING );
		Update( &m_wndAction, FALSE );
	//	Update( &m_wndStatus, strText );
		strText = strAction;
		Update( &m_wndSources, strText );
		Update( &m_wndSpeed, strNA );
		Update( &m_wndTime, strNA );
	}
	else if ( m_pDownload->IsPaused() )
	{
	//	LoadString( strText, IDS_MONITOR_PAUSED );
	//	Update( &m_wndStatus, strText );
		strText.Format( L"%i", nSourceCount );
		Update( &m_wndSources, strText );
		Update( &m_wndSpeed, IDS_STATUS_PAUSED );
		Update( &m_wndTime, strNA );
	}
	else if ( m_pDownload->IsStarted() && m_pDownload->GetProgress() == 100.0f )
	{
		LoadString( strAction, IDS_STATUS_VERIFYING );
		Update( &m_wndAction, FALSE );
	//	LoadString( strText, IDS_MONITOR_VERIFY );
	//	Update( &m_wndSources, strText );
		Update( &m_wndSpeed, strNA );
		Update( &m_wndTime, strNA );
	}
	else if ( nTransferCount > 0 )
	{
	//	LoadString( strText, IDS_MONITOR_DOWNLOADING );
	//	Update( &m_wndStatus, strText );

		strText.Format( L"%i  (%s %i)", nTransferCount, (LPCTSTR)strOf, nSourceCount );
		if ( Settings.General.LanguageRTL ) strText = L"\x202B" + strText;
		Update( &m_wndSources, strText );

		if ( m_pDownload->GetAverageSpeed() > 10 )
			strText = Settings.SmartSpeed( m_pDownload->GetAverageSpeed() );
		else
			LoadString( strText, IDS_TIP_NA );
		Update( &m_wndSpeed, strText );

		DWORD nTime = m_pDownload->GetTimeRemaining();
		strText.Empty();

		if ( nTime == 0xFFFFFFFF )
		{
			LoadString( strText, IDS_TIP_NA );
		}
		else if ( nTime > 90000 )
		{
			LoadString( strFormat, IDS_MONITOR_TIME_DH );
			strText.Format( strFormat, nTime / 86400, ( nTime / 3600 ) % 24 );
		}
		else if ( nTime > 3660 )
		{
			LoadString( strFormat, IDS_MONITOR_TIME_HM );
			strText.Format( strFormat, nTime / 3600, ( nTime % 3600 ) / 60 );
		}
		else if ( nTime > 61 )
		{
			LoadString( strFormat, IDS_MONITOR_TIME_MS );
			strText.Format( strFormat, nTime / 60, nTime % 60 );
		}
		else	// ( nTime < 60 )
		{
			LoadString( strFormat, IDS_MONITOR_TIME_S );
			strText.Format( strFormat, nTime % 60 );
		}

		Update( &m_wndTime, strText );
	}
	else if ( nSourceCount )	// No Transfers
	{
	//	LoadString( strText, IDS_MONITOR_DOWNLOADING );
	//	Update( &m_wndStatus, strText );
		strText.Format( L"%i", nSourceCount );
		Update( &m_wndSources, strText );
		Update( &m_wndSpeed, strNA );
		Update( &m_wndTime, strNA );
	}
	else
	{
	//	LoadString( strText, IDS_MONITOR_SOURCING );
	//	Update( &m_wndStatus, strText );
		LoadString( strText, IDS_MONITOR_NO_SOURCES );
		Update( &m_wndSources, strText );
		Update( &m_wndSpeed, strNA );
		Update( &m_wndTime, strNA );
	}

	if ( m_pDownload->IsStarted() )
	{
		if ( Settings.General.LanguageRTL )
		{
			strText.Format( L"(%.2f%%)  %s %s %s",
				m_pDownload->GetProgress(),
				(LPCTSTR)Settings.SmartVolume( m_pDownload->m_nSize ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( m_pDownload->GetVolumeComplete() ) );
		}
		else
		{
			strText.Format( L"%s %s %s  (%.2f%%)",
				(LPCTSTR)Settings.SmartVolume( m_pDownload->GetVolumeComplete() ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( m_pDownload->m_nSize ),
				m_pDownload->GetProgress() );
		}
		Update( &m_wndVolume, strText );
	}
	else
	{
		LoadString( strText, IDS_TIP_NA );
		Update( &m_wndVolume, strText );
	}

	if ( strAction.IsEmpty() )
		LoadString( strAction, IDS_MONITOR_ACTION_CANCEL );
	Update( &m_wndAction, strAction );
	Update( &m_wndAutoClose, ! bCompleted );

	CClientDC dc( this );
	DoPaint( dc );

	// AppBar on Windows 7
#ifdef __ITaskbarList3_INTERFACE_DEFINED__	// VS2010+
	if ( m_pTaskbar && theApp.m_nWinVer >= WIN_7 )
	{
		static bool bProgressShown = false;

		QWORD nTotal = m_pDownload->m_nSize, nComplete = m_pDownload->GetVolumeComplete();
		if ( nTotal && nTotal != nComplete && ! m_pDownload->IsPaused() )
		{
			bProgressShown = true;

			m_pTaskbar->SetProgressState( GetSafeHwnd(), TBPF_NORMAL );
			m_pTaskbar->SetProgressValue( GetSafeHwnd(), nComplete, nTotal );
		}
		else if ( bProgressShown )
		{
			bProgressShown = false;

			m_pTaskbar->SetProgressState( GetSafeHwnd(), TBPF_NOPROGRESS );
		}
	}
#endif	// ITaskbarList3
}

void CDownloadMonitorDlg::Update(CWnd* pWnd, LPCTSTR pszText)
{
	CString str;
	pWnd->GetWindowText( str );
	if ( str != pszText )
		pWnd->SetWindowText( pszText );
}

void CDownloadMonitorDlg::Update(CWnd* pWnd, BOOL bEnabled)
{
	if ( pWnd->IsWindowEnabled() != bEnabled )
		pWnd->EnableWindow( bEnabled );
}

void CDownloadMonitorDlg::OnPaint()
{
	CPaintDC dc( this );
	DoPaint( dc );
}

void CDownloadMonitorDlg::DoPaint(CDC& dc)
{
	CRect rc;

	// Draw Progress Bar
	m_wndProgress.GetWindowRect( &rc );
	ScreenToClient( &rc );

	DrawProgressBar( &dc, &rc );

	// Draw Bandwidth Graph
	m_wndGraph.GetWindowRect( &rc );
	ScreenToClient( &rc );

	dc.Draw3dRect( &rc, Colors.m_crMonitorGraphBorder, Colors.m_crMonitorGraphBorder );
	rc.DeflateRect( 1, 1 );
	m_pGraph->BufferedPaint( &dc, &rc );
}

void CDownloadMonitorDlg::DrawProgressBar(CDC* pDC, CRect* pRect)
{
	CRect rcCell( pRect );

	pDC->Draw3dRect( &rcCell, Colors.m_crMonitorGraphBorder, Colors.m_crMonitorGraphBorder );
	rcCell.DeflateRect( 1, 1 );

	if ( Transfers.m_pSection.Lock( 50 ) )
	{
		if ( Downloads.Check( m_pDownload ) )
			CFragmentBar::DrawDownload( pDC, &rcCell, m_pDownload, Colors.m_crMonitorGraphBack );
		Transfers.m_pSection.Unlock();
	}
}

void CDownloadMonitorDlg::OnDownloadShow()
{
	CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
	if ( ! pMainWnd || ! IsWindow( pMainWnd->m_hWnd ) ) return;

	CDownloadsWnd* pDownWnd = (CDownloadsWnd*)pMainWnd->m_pWindows.Find( RUNTIME_CLASS(CDownloadsWnd) );
	if ( pDownWnd ) pDownWnd->Select( m_pDownload );

	pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_DOWNLOADS );
	pMainWnd->PostMessage( WM_SYSCOMMAND, SC_RESTORE );
}

void CDownloadMonitorDlg::OnDownloadAction()	// OnDownloadStop/OnDownloadLaunch
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 300 ) || ! Downloads.Check( m_pDownload ) )
		return;

	if ( m_pDownload->IsCompleted() )
	{
		m_pDownload->Launch( -1, &pLock, FALSE );
		pLock.Unlock();
		PostMessage( WM_CLOSE );
		return;
	}

	if ( m_pDownload->IsStarted() )
	{
		CString strFormat, strPrompt;
		LoadString( strFormat, IDS_DOWNLOAD_CONFIRM_CLEAR );
		strPrompt.Format( strFormat, (LPCTSTR)m_pDownload->m_sName );

		pLock.Unlock();
		if ( MsgBox( strPrompt, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
			return;
		pLock.Lock();
	}

	if ( Downloads.Check( m_pDownload ) && ! m_pDownload->IsMoving() )
	{
		m_pDownload->Remove();
		pLock.Unlock();
		PostMessage( WM_CLOSE );
	}
}

void CDownloadMonitorDlg::OnDownloadClose()
{
	PostMessage( WM_CLOSE );
}

void CDownloadMonitorDlg::OnClose()
{
	DestroyWindow();
}

void CDownloadMonitorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	const UINT nCommand = nID & 0xFFF0;

	switch ( nCommand )
	{
	case SC_MINIMIZE:
		if ( ! ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) )
			break;
	case SC_MAXIMIZE:
		if ( ! m_bTray )
		{
			m_pTray.cbSize				= sizeof( m_pTray );
			m_pTray.hWnd				= GetSafeHwnd();
			m_pTray.uID					= 0;
			m_pTray.uCallbackMessage	= WM_TRAY;
			m_pTray.uFlags				= NIF_ICON | NIF_MESSAGE | NIF_TIP;
			m_pTray.hIcon				= CoolInterface.ExtractIcon( IDI_DOWNLOAD_MONITOR, FALSE );
			_tcsncpy( m_pTray.szTip, Settings.SmartAgent(), _countof( m_pTray.szTip ) - 1 );
			m_pTray.szTip[ _countof( m_pTray.szTip ) - 1 ] = L'\0';
			Shell_NotifyIcon( NIM_ADD, &m_pTray );
			ShowWindow( SW_HIDE );
			m_bTray = TRUE;
		}
		return;
	case SC_RESTORE:
		if ( ! m_bTray )
			break;
		OnTray( WM_LBUTTONDBLCLK, 0 );
		return;
	case SC_NEXTWINDOW:
		if ( GetExStyle() & WS_EX_TOPMOST )
			SetWindowPos( &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
		else
			SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
		return;
	}

	CSkinDialog::OnSysCommand( nID, lParam );
}

LRESULT CDownloadMonitorDlg::OnTray(WPARAM /*wParam*/, LPARAM lParam)
{
	if ( LOWORD(lParam) == WM_LBUTTONDBLCLK && m_bTray )
	{
		Shell_NotifyIcon( NIM_DELETE, &m_pTray );
		ShowWindow( SW_SHOWNORMAL );
		SetForegroundWindow();
		m_bTray = FALSE;
	}

	return 0;
}

HBRUSH CDownloadMonitorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndFile )
		pDC->SelectObject( &theApp.m_gdiFontBold );

	return hbr;
}

void CDownloadMonitorDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
	if ( ! pMainWnd || ! IsWindow( pMainWnd->m_hWnd ) ) return;

	CDownloadsWnd* pDownWnd = (CDownloadsWnd*)pMainWnd->m_pWindows.Find( RUNTIME_CLASS(CDownloadsWnd) );
	if ( ! pDownWnd ) return;

	CSingleLock pLock( &Transfers.m_pSection );

	if ( ! pLock.Lock( 250 ) || ! Downloads.Check( m_pDownload ) ) return;

	pDownWnd->Select( m_pDownload );

	pLock.Unlock();

	const BOOL bCompleted = m_pDownload->IsCompleted();

	CMenu* pPopup = ::Skin.GetMenu( bCompleted ?
		( m_pDownload->IsSeeding() ? L"CDownloadsWnd.Seeding" : L"CDownloadsWnd.Completed" ) : L"CDownloadsWnd.Download" );
	if ( ! pPopup ) return;

	MENUITEMINFO pInfo;
	pInfo.cbSize	= sizeof( pInfo );
	pInfo.fMask		= MIIM_STATE;
	GetMenuItemInfo( pPopup->GetSafeHmenu(), bCompleted ?
		ID_DOWNLOADS_LAUNCH_COMPLETE : ID_DOWNLOADS_LAUNCH_COPY, FALSE, &pInfo );
	pInfo.fState	|= MFS_DEFAULT;
	SetMenuItemInfo( pPopup->GetSafeHmenu(), bCompleted ?
		ID_DOWNLOADS_LAUNCH_COMPLETE : ID_DOWNLOADS_LAUNCH_COPY, FALSE, &pInfo );

	CoolMenu.AddMenu( pPopup, TRUE );

	UINT nID = pPopup->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD,
		point.x, point.y, pDownWnd );

	if ( nID )
		pDownWnd->SendMessage( WM_COMMAND, nID );
}

BOOL CDownloadMonitorDlg::OnNeedText(UINT /*nID*/, NMHDR* pTTTH, LRESULT* /*pResult*/)
{
	// ToDo: Fix This? It does not get a notification from the static window (!)
	if ( pTTTH->idFrom == IDC_MONITOR_FILE )
	{
		TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pTTTH;
		pTTT->lpszText = (LPTSTR)(LPCTSTR)m_sName;
	}

	return TRUE;
}

void CDownloadMonitorDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	DWORD nCheck = ( GetExStyle() & WS_EX_TOPMOST ) ? MF_CHECKED : MF_UNCHECKED;
	pPopupMenu->CheckMenuItem( SC_NEXTWINDOW, MF_BYCOMMAND|nCheck );

	CSkinDialog::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
}
