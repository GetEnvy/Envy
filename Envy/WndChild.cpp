//
// WndChild.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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
#include "WndChild.h"
#include "WndMain.h"
#include "Skin.h"
#include "SkinWindow.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CChildWnd, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildWnd, CMDIChildWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOMMAND()
	ON_WM_MDIACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCRBUTTONUP()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCMOUSELEAVE()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_HELPINFO()
	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

CChildWnd* CChildWnd::m_pCmdMsg = NULL;


/////////////////////////////////////////////////////////////////////////////
// CChildWnd construction

CChildWnd::CChildWnd()
	: m_nResID			( 0 )
	, m_pSkin			( NULL )
	, m_pMainWndCache	( NULL )
	, m_pGroupParent	( NULL )
	, m_nGroupSize		( 0.5f )
	, m_bGroupMode		( FALSE )
	, m_bTabMode		( FALSE )
	, m_bPanelMode		( FALSE )
	, m_bAlert			( FALSE )
{
}

void CChildWnd::RemoveSkin()
{
	m_pSkin = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CChildWnd operations

BOOL CChildWnd::Create(UINT nID, BOOL bVisible)
{
	m_nResID = nID;

	CString strCaption;
	LoadString( strCaption, m_nResID );

	return CMDIChildWnd::Create( NULL, strCaption, WS_CHILD |
		WS_OVERLAPPEDWINDOW | ( bVisible ? WS_VISIBLE : 0 ) |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
}

void CChildWnd::GetWindowText(CString& rString)
{
	if ( m_sCaption.IsEmpty() )
		CMDIChildWnd::GetWindowText( m_sCaption );

	rString = m_sCaption;
}

void CChildWnd::SetWindowText(LPCTSTR lpszString)
{
	if ( m_sCaption != lpszString )
	{
		m_sCaption = lpszString;
		CMDIChildWnd::SetWindowText( lpszString );
	}
}

CMainWnd* CChildWnd::GetMainWnd()
{
	if ( ! m_pMainWndCache )
		m_pMainWndCache = (CMainWnd*)GetMDIFrame();

	return m_pMainWndCache;
}

CWindowManager* CChildWnd::GetManager()
{
	return &GetMainWnd()->m_pWindows;
}

BOOL CChildWnd::IsActive(BOOL bFocused)
{
	CMainWnd* pMainWnd = GetMainWnd();

	if ( bFocused && GetForegroundWindow() != pMainWnd ) return FALSE;

	CChildWnd* pActive = pMainWnd->m_pWindows.GetActive();	// Was (CChildWnd*)pMainWnd->MDIGetActive()

	if ( pActive == this ) return TRUE;
	if ( bFocused ) return FALSE;

	return	( pActive && m_pGroupParent == pActive ) ||
			( pActive && pActive->m_pGroupParent == this );
}

BOOL CChildWnd::IsPartiallyVisible()
{
	if ( IsWindowVisible() == FALSE || IsIconic() == TRUE ) return FALSE;

	CRect rc;
	GetClientRect( &rc );
	ClientToScreen( &rc );

	return	TestPoint( rc.CenterPoint() ) ||
			TestPoint( CPoint( rc.left + 1, rc.top + 1 ) ) ||
			TestPoint( CPoint( rc.right - 1, rc.top + 1 ) ) ||
			TestPoint( CPoint( rc.left + 1, rc.bottom - 2 ) ) ||
			TestPoint( CPoint( rc.right - 2, rc.bottom - 2 ) );
}

BOOL CChildWnd::TestPoint(const CPoint& ptScreen)
{
	CWnd* pHit = WindowFromPoint( ptScreen );

	if ( pHit == NULL )
		return FALSE;

	if ( pHit == this )
		return TRUE;

	if ( ! IsWindow( pHit->m_hWnd ) || ! IsWindow( GetSafeHwnd() ) )
		return FALSE;

	if ( ::GetAncestor( pHit->m_hWnd, GA_ROOT ) != ::GetAncestor( GetSafeHwnd(), GA_ROOT ) )
		return FALSE;

	CPoint ptChild( ptScreen );
	pHit->ScreenToClient( &ptChild );

	CWnd* pChild = pHit->ChildWindowFromPoint( ptChild, CWP_SKIPINVISIBLE );
	if ( pChild == NULL ) pChild = pHit;

	while ( pChild != NULL )
	{
		if ( pChild == this ) return TRUE;
		pChild = pChild->GetParent();
	}

	return FALSE;
}

BOOL CChildWnd::LoadState(LPCTSTR pszName, BOOL bDefaultMaximise)
{
	CRect rcParent, rcChild;
	GetParent()->GetClientRect( &rcParent );

	if ( ! m_bPanelMode && Settings.LoadWindow( pszName, this ) )
	{
		if ( rcParent.Width() > 64 && rcParent.Height() > 32 )
		{
			GetWindowRect( &rcChild );
			GetParent()->ScreenToClient( &rcChild );

			if ( rcChild.right > rcParent.right || rcChild.bottom > rcParent.bottom )
			{
				rcChild.right	= min( rcChild.right, rcParent.right );
				rcChild.bottom	= min( rcChild.bottom, rcParent.bottom );
				MoveWindow( &rcChild );
			}
		}

		OnSkinChange();

		return TRUE;
	}

	if ( m_bPanelMode || bDefaultMaximise )		// Was m_bGroupMode
	{
		if ( m_bTabMode )
		{
			CString strClassName( GetRuntimeClass()->m_lpszClassName );
			CString strName( pszName ? pszName : (LPCTSTR)strClassName );
			m_nGroupSize = (float)theApp.GetProfileInt( L"Windows", strName + L".Splitter", 500 ) / 1000;
		}

		if ( rcParent.Width() > 64 && rcParent.Height() > 32 )
			MoveWindow( &rcParent );
	}

	OnSkinChange();

	return FALSE;
}

BOOL CChildWnd::SaveState(LPCTSTR pszName)
{
	if ( m_bTabMode && m_pGroupParent == NULL )
	{
		CString strName = ( pszName != NULL ) ? CString( pszName ) : CString( GetRuntimeClass()->m_lpszClassName );
		strName += L".Splitter";
		theApp.WriteProfileInt( L"Windows", strName, (int)( m_nGroupSize * 1000 ) );
		return TRUE;
	}

	if ( ! m_bPanelMode )
	{
		Settings.SaveWindow( pszName, this );
		return TRUE;
	}

	return FALSE;
}

BOOL CChildWnd::SetAlert(BOOL bAlert)
{
	if ( m_bAlert == bAlert ) return FALSE;

	CMainWnd* pMainWnd = GetMainWnd();

	if ( bAlert && pMainWnd->m_pWindows.GetActive() == this ) return FALSE;		// Was MDIGetActive()

	m_bAlert = bAlert;

	pMainWnd->OnUpdateCmdUI();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CChildWnd message handlers

int CChildWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CMDIChildWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_bAlert = 1982;
	CChildWnd::OnSkinChange();
	m_bAlert = FALSE;

	GetManager()->Add( this );

	return 0;
}

void CChildWnd::OnDestroy()
{
	GetManager()->Remove( this );

	CMDIChildWnd::OnDestroy();
}

BOOL CChildWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_pCmdMsg == this ) return FALSE;

	if ( CMDIChildWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;

	if ( m_pCmdMsg != NULL ) return FALSE;

	m_pCmdMsg = this;
	BOOL bResult = GetMainWnd()->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
	m_pCmdMsg = NULL;

	return bResult;
}

BOOL CChildWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CChildWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( m_pSkin ) m_pSkin->OnSize( this );

	CMDIChildWnd::OnSize( nType, cx, cy );

	BOOL bMinimized	= IsIconic();
	BOOL bVisible	= IsWindowVisible();

	if ( bMinimized && bVisible )
		ShowWindow( SW_HIDE );
	else if ( ! bMinimized && ! bVisible )
		ShowWindow( SW_SHOW );
}

void CChildWnd::SizeListAndBar(CWnd* pList, CWnd* pBar)
{
	CRect rc;
	GetClientRect( &rc );
	rc.bottom -= Settings.Skin.ToolbarHeight;

	HDWP hPos = BeginDeferWindowPos( 2 );
	DeferWindowPos( hPos, pList->GetSafeHwnd(), NULL,
		rc.left, rc.top, rc.Width(), rc.Height(),
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hPos, pBar->GetSafeHwnd(), NULL,
		rc.left, rc.bottom, rc.Width(), Settings.Skin.ToolbarHeight,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	EndDeferWindowPos( hPos );
}

void CChildWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch ( nID & 0xFFF0 )
	{
	case SC_MINIMIZE:
		if ( m_bTabMode ) break;
		ShowWindow( SW_HIDE );
		ShowWindow( SW_MINIMIZE );
		break;
	case SC_MAXIMIZE:
		if ( m_bTabMode ) break;
		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			CMDIChildWnd::OnSysCommand( nID, lParam );
		}
		else
		{
			CRect rc;
			ShowWindow( SW_SHOWNORMAL );
			GetParent()->GetClientRect( &rc );
			MoveWindow( &rc );
		}
		break;
	case SC_MOVE:
	case SC_SIZE:
	//case SC_PREVWINDOW:
	//case SC_NEXTWINDOW:
		if ( m_bPanelMode || m_bTabMode ) break;
		CMDIChildWnd::OnSysCommand( nID, lParam );
		break;
	case SC_CLOSE:
		if ( m_bTabMode )
			PostMessage( WM_SYSCOMMAND, SC_NEXTWINDOW );
		else
			CMDIChildWnd::OnSysCommand( nID, lParam );
		break;
	default:
		CMDIChildWnd::OnSysCommand( nID, lParam );
	}
}

void CChildWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	if ( theApp.m_bClosing || GetManager()->m_bIgnoreActivate ) return;

	if ( bActivate && m_bAlert ) SetAlert( FALSE );

	CMDIChildWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );

	if ( bActivate && m_bGroupMode )
		GetManager()->ActivateGrouped( this );
}

void CChildWnd::OnNcRButtonUp(UINT nHitTest, CPoint point)
{
	if ( nHitTest == HTCAPTION )
	{
		CWnd* pWnd = ( Settings.General.GUIMode != GUI_WINDOWED ? this : (CWnd*)GetMainWnd() );
		pWnd->PostMessage( WM_CONTEXTMENU, (WPARAM)pWnd->GetSafeHwnd(), MAKELONG( point.x, point.y ) );
		return;
	}

	CMDIChildWnd::OnNcRButtonUp( nHitTest, point );
}

/////////////////////////////////////////////////////////////////////////////
// CChildWnd skin forwarders

void CChildWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if ( m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CMDIChildWnd::OnNcCalcSize( bCalcValidRects, lpncsp );
}

LRESULT CChildWnd::OnNcHitTest(CPoint point)
{
	if ( m_pSkin )
		return m_pSkin->OnNcHitTest( this, point, ! m_bPanelMode );

	return CMDIChildWnd::OnNcHitTest( point );
}

void CChildWnd::OnNcPaint()
{
	if ( m_pSkin )
		m_pSkin->OnNcPaint( this );
	else
		CMDIChildWnd::OnNcPaint();
}

BOOL CChildWnd::OnNcActivate(BOOL bActive)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		BOOL bResult = CMDIChildWnd::OnNcActivate( bActive );
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		m_pSkin->OnNcActivate( this, bActive || ( m_nFlags & WF_STAYACTIVE ) );

		return bResult;
	}

	return CMDIChildWnd::OnNcActivate( bActive );
}

void CChildWnd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	if ( m_pSkin ) m_pSkin->OnNcMouseMove( this, nHitTest, point );
	CMDIChildWnd::OnNcMouseMove(nHitTest, point);
}

void CChildWnd::OnNcMouseLeave()
{
	if ( m_pSkin ) m_pSkin->OnNcMouseLeave( this );
}

void CChildWnd::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDown( this, nHitTest, point ) ) return;
	CMDIChildWnd::OnNcLButtonDown( nHitTest, point );
}

void CChildWnd::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonUp( this, nHitTest, point ) ) return;
	CMDIChildWnd::OnNcLButtonUp( nHitTest, point );
}

void CChildWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDblClk( this, nHitTest, point ) ) return;
	CMDIChildWnd::OnNcLButtonDblClk( nHitTest, point );
}

LRESULT CChildWnd::OnSetText(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		LRESULT lResult = Default();
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		if ( m_pSkin ) m_pSkin->OnSetText( this );
		return lResult;
	}

	return Default();
}

/////////////////////////////////////////////////////////////////////////////
// CChildWnd event handlers

void CChildWnd::OnSkinChange()
{
	m_pSkin = Skin.GetWindowSkin( this );
	OnSize( 0, 0, 0 );

	// ToDo: Remove this mode?
	CoolInterface.EnableTheme( this, ( m_pSkin == NULL ) &&
		( Settings.General.GUIMode == GUI_WINDOWED ) );

	if ( m_nResID )
	{
		CoolInterface.SetIcon( m_nResID, Settings.General.LanguageRTL, FALSE, this );

		CString strCaption;
		LoadString( strCaption, m_nResID );

		SetWindowText( L"" );
		SetWindowText( strCaption );
	}

	if ( m_bAlert != 1982 )
	{
		SetWindowRgn( NULL, FALSE );
		SetWindowPos( NULL, 0, 0, 0, 0,
			SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_FRAMECHANGED );
		if ( m_pSkin ) m_pSkin->OnSize( this );
	}
}

HRESULT CChildWnd::GetGenericView(IGenericView** ppView)
{
	*ppView = NULL;

	return S_FALSE;
}

BOOL CChildWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	return FALSE;
}

BOOL CChildWnd::PreTranslateMessage(MSG* pMsg)
{
	// Hack to enable TAB-navigation
	if ( pMsg->wParam == VK_TAB &&
		( pMsg->message == WM_KEYDOWN ||
		  pMsg->message == WM_KEYUP ||
		  pMsg->message == WM_CHAR ) )
	{
		if ( IsDialogMessage( pMsg ) )
			return TRUE;
	}

	return CMDIChildWnd::PreTranslateMessage( pMsg );
}

BOOL CChildWnd::DestroyWindow()
{
	RemoveSkin();

	return CMDIChildWnd::DestroyWindow();
}
