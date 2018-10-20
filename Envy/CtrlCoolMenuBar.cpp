//
// CtrlCoolMenuBar.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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
//#include "CtrlCoolBar.h"
#include "CtrlCoolMenuBar.h"
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "WndMain.h"
#include "WndChild.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define MENUBAR_HEIGHT 28	// Placeholder for Settings.Skin.MenubarHeight


BEGIN_MESSAGE_MAP(CCoolMenuBarCtrl, CCoolBarCtrl)
	ON_WM_TIMER()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_WM_LBUTTONDOWN()
	ON_WM_INITMENUPOPUP()
	ON_WM_MENUSELECT()
	ON_WM_ENTERIDLE()
	ON_WM_ENTERMENULOOP()
	ON_WM_EXITMENULOOP()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl construction

CCoolMenuBarCtrl::CCoolMenuBarCtrl()
{
	m_hMenu		= NULL;
	m_bMenuGray	= TRUE;
	m_bGripper	= TRUE;

	m_bStretch	= theApp.GetProfileInt( L"", L"MenuStretch", TRUE );
	if ( theApp.GetProfileInt( L"", L"MenuHalfHeight", TRUE ) )
		m_nHeight = MENUBAR_HEIGHT;		// Settings.Skin.MenubarHeight set later
}

CCoolMenuBarCtrl::~CCoolMenuBarCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl operations

void CCoolMenuBarCtrl::SetMenu(HMENU hMenu)
{
	m_hMenu = hMenu;

	Clear();

	if ( ! m_hMenu ) return;

	CMenu pMenu;
	pMenu.Attach( m_hMenu );

	for ( UINT nItem = 0; nItem < (UINT)pMenu.GetMenuItemCount(); nItem++ )
	{
		CString strMenu;
		pMenu.GetMenuString( nItem, strMenu, MF_BYPOSITION );

		int nAmp = strMenu.Find( L'&' );
		if ( nAmp >= 0 ) strMenu = strMenu.Left( nAmp ) + strMenu.Mid( nAmp + 1 );

		CCoolBarItem* pItem = new CCoolBarItem( this, nItem + 1 );
		pItem->SetText( L" " + strMenu + L" " );
		m_pItems.AddTail( pItem );
	}

	pMenu.Detach();
}

void CCoolMenuBarCtrl::OpenMenuBar()
{
	if ( m_pDown == NULL )
	{
		if ( ( m_pSelect = GetIndex( 0 ) ) != NULL )
			PostMessage( WM_TIMER, 5 );
	}
}

BOOL CCoolMenuBarCtrl::OpenMenuChar(UINT nChar)
{
	CMenu pMenu;
	pMenu.Attach( m_hMenu );

	for ( UINT nItem = 0; nItem < (UINT)pMenu.GetMenuItemCount(); nItem++ )
	{
		CString strMenu;
		pMenu.GetMenuString( nItem, strMenu, MF_BYPOSITION );

		LPCTSTR pszChar = _tcschr( strMenu, '&' );
		if ( ! pszChar++ ) continue;

		if ( toupper( *pszChar ) == toupper( nChar ) )
		{
			pMenu.Detach();
			if ( ( m_pSelect = GetIndex( nItem ) ) != NULL )
				PostMessage( WM_TIMER, 5 );
			return TRUE;
		}
	}

	pMenu.Detach();
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl message handlers

void CCoolMenuBarCtrl::ShowMenu()
{
	if ( m_pHot == NULL || ! IsMenu( m_hMenu ) ) return;

	CMenu* pMenu = CMenu::FromHandle( m_hMenu )->GetSubMenu( m_pHot->m_nID - 1 );

	if ( pMenu == NULL )
	{
		m_pHot = m_pDown = NULL;
		Invalidate();
		return;
	}

	// Detect "_Window" menu to add any child windows  (Found using default commands layout)
	// Note: Ensure this menu detection method is still valid/unique in all languages
	const UINT nFirstID = pMenu->GetMenuItemID( 0 );
	if ( nFirstID == ID_WINDOW_NAVBAR || nFirstID == ID_WINDOW_CASCADE )	// Tabbed/Windowed modes
		UpdateWindowMenu( pMenu );
	else if ( nFirstID == ID_TOOLS_SETTINGS ) // && Settings.Skin.DropMenu	// Get DropMenu submenu
		UpdateWindowMenu( pMenu->GetSubMenu( 4 ) );

	m_pDown = m_pHot;
	Invalidate();

	KillTimer( 1 );

	CRect rc;
	GetItemRect( m_pDown, &rc );
	ClientToScreen( &rc );
	rc.DeflateRect( 1, 2 );

	TPMPARAMS tpm;
	tpm.cbSize = sizeof(tpm);
	tpm.rcExclude = rc;

	m_pMenuBar = this;
	m_hMsgHook = SetWindowsHookEx( WH_MSGFILTER, MenuFilter, NULL, GetCurrentThreadId() );

	CoolMenu.RegisterEdge( Settings.General.LanguageRTL ? rc.right : rc.left, rc.bottom, rc.Width() );

	UINT nCmd = TrackPopupMenuEx( pMenu->GetSafeHmenu(),
		TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_RETURNCMD,
		Settings.General.LanguageRTL ? rc.right : rc.left, rc.bottom, GetSafeHwnd(), &tpm );

	UnhookWindowsHookEx( m_hMsgHook );

	m_hMsgHook = NULL;
	m_pMenuBar = NULL;
	m_pDown = NULL;
	OnTimer( 1 );

	if ( m_pHot != NULL )
	{
		SetTimer( 1, 100, NULL );
		m_bTimer = TRUE;
	}

	Invalidate();
	UpdateWindow();

	if ( nCmd ) GetOwner()->PostMessage( WM_COMMAND, nCmd );
}

void CCoolMenuBarCtrl::UpdateWindowMenu(CMenu* pMenu)
{
	// Add child windows list to detected "_Window" menu

	// Clear any previously added entries
	for ( UINT nIndex = pMenu->GetMenuItemCount() - 1; nIndex; nIndex-- )
	{
		if ( pMenu->GetMenuItemID( nIndex ) < AFX_IDM_FIRST_MDICHILD && pMenu->GetMenuItemID( nIndex ) != ID_SEPARATOR )
			break;
		pMenu->RemoveMenu( nIndex, MF_BYPOSITION );
	}

	CMainWnd* pFrame = theApp.SafeMainWnd();					// Was CMDIFrameWnd* pFrame = (CMDIFrameWnd*)AfxGetMainWnd();
	if ( ! pFrame->IsKindOf( RUNTIME_CLASS(CMDIFrameWnd) ) ) return;

	CWnd* pClient = pFrame->GetWindow( GW_CHILD );
	for ( ; pClient; pClient = pClient->GetNextWindow() )
	{
		TCHAR szClass[64];
		GetClassName( pClient->GetSafeHwnd(), szClass, 64 );
		if ( _tcsicmp( szClass, L"MDIClient" ) == 0 ) break;
	}

	if ( pClient == NULL ) return;

	CMDIChildWnd* pActive = pFrame->m_pWindows.GetActive();		// Was pFrame->MDIGetActive();
	BOOL bSeparator = TRUE;

	for ( UINT nIndex = 1, nID = AFX_IDM_FIRST_MDICHILD; nIndex <= 12; nIndex++, nID++ )
	{
		CChildWnd* pChildWnd = (CChildWnd*)pClient->GetDlgItem( nID );
		if ( ! pChildWnd ) break;

		if ( pChildWnd->m_bTabMode )
		{
			nIndex--;
			continue;
		}

		if ( bSeparator )
		{
			pMenu->AppendMenu( MF_SEPARATOR, ID_SEPARATOR );
			bSeparator = FALSE;
		}

		CString strMenu, strWindow;
		pChildWnd->GetWindowText( strWindow );
		strMenu.Format( L"&%u %s", nIndex, (LPCTSTR)strWindow );

		pMenu->AppendMenu( MF_STRING | ( pChildWnd == pActive ? MF_CHECKED : 0 ), nID, strMenu );
	}
}

void CCoolMenuBarCtrl::ShiftMenu(int nOffset)
{
	INT_PTR nIndex = 0;

	if ( m_pDown )
	{
		nIndex = m_pDown->m_nID - 1 + nOffset;
		if ( nIndex < 0 ) nIndex = GetCount() - 1;
		if ( nIndex >= GetCount() ) nIndex = 0;
	}

	PostMessage( WM_CANCELMODE, 0, 0 );		// Settings.WINE.MenuFix (No SendMessage)

	m_pSelect = GetIndex( static_cast< int >( nIndex ) );
	m_pHot = m_pDown = NULL;
	PostMessage( WM_TIMER, 5 );
}

void CCoolMenuBarCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if ( theApp.m_bClosing )
		return;

	switch ( nIDEvent )
	{
	case 5:
		m_pHot = m_pSelect;
	case 4:
		ShowMenu();
		break;
	default:
		CCoolBarCtrl::OnTimer( nIDEvent );
		break;
	}
}

void CCoolMenuBarCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ( theApp.m_bClosing )
		return;

	CCoolBarItem* pHit = HitTest( point );

	if ( pHit && pHit == m_pHot )
	{
		ShowMenu();
		return;
	}

	CCoolBarCtrl::OnLButtonDown( nFlags, point );
}

void CCoolMenuBarCtrl::OnSkinChange()
{
	m_nHeight = Settings.Skin.MenubarHeight;
}

/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl menu message forwarding

void CCoolMenuBarCtrl::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

void CCoolMenuBarCtrl::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	GetOwner()->SendMessage( WM_INITMENUPOPUP, (WPARAM)pPopupMenu->GetSafeHmenu(), MAKELONG( nIndex, bSysMenu ) );
}

void CCoolMenuBarCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	GetOwner()->SendMessage( WM_MEASUREITEM, (WPARAM)nIDCtl, (LPARAM)lpMeasureItemStruct );
}

void CCoolMenuBarCtrl::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	GetOwner()->SendMessage( WM_DRAWITEM, (WPARAM)nIDCtl, (LPARAM)lpDrawItemStruct );
}

void CCoolMenuBarCtrl::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	GetOwner()->SendMessage( WM_MENUSELECT, MAKELONG( nItemID, nFlags ), (LPARAM)hSysMenu );
}

void CCoolMenuBarCtrl::OnEnterMenuLoop(BOOL bIsTrackPopupMenu)
{
	GetOwner()->SendMessage( WM_ENTERMENULOOP, (WPARAM)bIsTrackPopupMenu );
}

void CCoolMenuBarCtrl::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	GetOwner()->SendMessage( WM_EXITMENULOOP, (WPARAM)bIsTrackPopupMenu );
}

void CCoolMenuBarCtrl::OnEnterIdle(UINT nWhy, CWnd* pWho)
{
	GetOwner()->SendMessage( WM_ENTERIDLE, (WPARAM)nWhy, (LPARAM)pWho->GetSafeHwnd() );
}

/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl menu message hooking

CCoolMenuBarCtrl* CCoolMenuBarCtrl::m_pMenuBar = NULL;
HHOOK CCoolMenuBarCtrl::m_hMsgHook = NULL;

LRESULT CCoolMenuBarCtrl::MenuFilter(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG* pMsg = (MSG*)lParam;

	if ( m_pMenuBar && nCode == MSGF_MENU )
	{
		if ( m_pMenuBar->OnMenuMessage( pMsg ) ) return TRUE;
	}

	return CallNextHookEx( m_hMsgHook, nCode, wParam, lParam );
}

BOOL CCoolMenuBarCtrl::OnMenuMessage(MSG* pMsg)
{
	if ( theApp.m_bClosing )
		return FALSE;

	switch ( pMsg->message )
	{
	case WM_MOUSEMOVE:
		{
			//CPoint pt( LOWORD( pMsg->lParam ), HIWORD( pMsg->lParam ) );
			CPoint pt( pMsg->lParam );
			ScreenToClient( &pt );

			if ( m_pMouse == pt ) return TRUE;
			m_pMouse = pt;

			CCoolBarItem* pHit = HitTest( pt );

			if ( pHit && pHit != m_pDown )
			{
				m_pHot	= pHit;
				m_pDown	= NULL;

				PostMessage( WM_CANCELMODE, 0, 0 );		// Settings.WINE.MenuFix (No SendMessage)
				PostMessage( WM_TIMER, 4 );
				return TRUE;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			//CPoint pt( LOWORD( pMsg->lParam ), HIWORD( pMsg->lParam ) );
			CPoint pt( pMsg->lParam );

			CWnd* pWnd = CWnd::WindowFromPoint( pt );

			if ( pWnd )
			{
				TCHAR szClass[2];
				GetClassName( pWnd->GetSafeHwnd(), szClass, 2 );
				if ( szClass[0] == '#' ) return FALSE;
			}

			ScreenToClient( &pt );

			CCoolBarItem* pHit = HitTest( pt );

			if ( pHit == NULL )
			{
				m_pHot = m_pDown = NULL;
				PostMessage( WM_CANCELMODE, 0, 0 );		// Was Settings.WINE.MenuFix (No SendMessage)
				return TRUE;
			}
			if ( pHit == m_pDown )
			{
				m_pDown = NULL;
				PostMessage( WM_CANCELMODE, 0, 0 );
				return TRUE;
			}
		}
		break;
	case WM_KEYDOWN:
		switch ( pMsg->wParam )
		{
		case VK_LEFT:
			ShiftMenu( -1 );
			return TRUE;
		case VK_RIGHT:
			ShiftMenu( 1 );
			return TRUE;
		case VK_ESCAPE:
			PostMessage( WM_CANCELMODE, 0, 0 );
			m_pHot = m_pDown = NULL;
			return TRUE;
		}
		break;
	//default:
	//	break;
	}

	return FALSE;
}
