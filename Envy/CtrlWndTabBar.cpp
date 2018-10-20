//
// CtrlWndTabBar.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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
#include "CtrlWndTabBar.h"
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "Colors.h"
#include "Images.h"
#include "Skin.h"

#include "WndMain.h"
#include "WndPlugin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//#define TABBARHEIGHT	26	// Settings.Skin.TaskbarHeight
//#define TABWIDTH		140	// Settings.Skin.TaskbarTabWidth

BEGIN_MESSAGE_MAP(CWndTabBar, CControlBar)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWndTabBar construction

CWndTabBar::CWndTabBar()
	: m_pSelected		( NULL )
	, m_pHot			( NULL )
	, m_dwHoverTime		( 0 )
	, m_nCookie 		( 0 )
	, m_bTimer			( FALSE )
	, m_bMenuGray		( FALSE )
	, m_bCloseButton	( FALSE )
	, m_nCloseImage		( 0 )
	, m_nCloseImageHover ( 0 )
	, m_nMessage		( 0 )
	, m_nMaximumWidth	( Settings.Skin.TaskbarTabWidth )
{
}

CWndTabBar::~CWndTabBar()
{
	for ( POSITION pos = m_pItems.GetHeadPosition(); pos; )
	{
		delete m_pItems.GetNext( pos );
	}
	m_pItems.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CWndTabBar operations

BOOL CWndTabBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc;
	dwStyle |= WS_CHILD;
	return CWnd::Create( NULL, NULL, dwStyle, rc, pParentWnd, nID, NULL );
}

void CWndTabBar::SetMessage(UINT nMessageID)
{
	if ( m_nMessage == nMessageID ) return;

	if ( ( m_nMessage = nMessageID ) != 0 )
		Skin.LoadString( m_sMessage, m_nMessage );
	else
		m_sMessage.Empty();

	if ( m_pItems.IsEmpty() ) Invalidate();
}

void CWndTabBar::SetMessage(LPCTSTR pszText)
{
	if ( m_sMessage == pszText ) return;
	m_nMessage = 0;
	m_sMessage = pszText;
	if ( m_pItems.IsEmpty() ) Invalidate();
}

void CWndTabBar::SetMaximumWidth(int nWidth)
{
	if ( Settings.Skin.TaskbarTabWidth > 40 )
		nWidth = Settings.Skin.TaskbarTabWidth;
	if ( m_nMaximumWidth == nWidth ) return;
	m_nMaximumWidth = nWidth;
	Invalidate();
}

void CWndTabBar::SetWatermark(HBITMAP hBitmap)
{
	if ( m_bmImage.m_hObject ) m_bmImage.DeleteObject();
	if ( hBitmap ) m_bmImage.Attach( hBitmap );

	for ( POSITION pos = m_pItems.GetHeadPosition(); pos; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );
		pItem->m_nImage = -1;
	}

	m_pIcons.RemoveAll();
	m_pImages.DeleteImageList();
	m_pImages.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 );
}

void CWndTabBar::MoveLeft( TabItem* pItem )
{
	if ( m_pItems.GetCount() < 2 ) return;

	if ( POSITION pos = m_pItems.Find( pItem ) )
	{
		POSITION pos2 = pos;
		m_pItems.GetPrev( pos2 );
		m_pItems.InsertBefore( pos2, pItem );
		m_pItems.RemoveAt( pos );
		Invalidate();
	}
}

void CWndTabBar::MoveRight( TabItem* pItem )
{
	if ( m_pItems.GetCount() < 2 ) return;

	if ( POSITION pos = m_pItems.Find( pItem ) )
	{
		POSITION pos2 = pos;
		m_pItems.GetNext( pos2 );
		m_pItems.InsertAfter( pos2, pItem );
		m_pItems.RemoveAt( pos );
		Invalidate();
	}
}

void CWndTabBar::MoveFirst( TabItem* pItem )
{
	if ( m_pItems.GetCount() < 2 ) return;

	if ( POSITION pos = m_pItems.Find( pItem ) )
	{
		m_pItems.RemoveAt( pos );
		m_pItems.AddHead( pItem );

		Invalidate();
	}
}

void CWndTabBar::MoveLast( TabItem* pItem )
{
	if ( m_pItems.GetCount() < 2 ) return;

	if ( POSITION pos = m_pItems.Find( pItem ) )
	{
		m_pItems.AddTail( pItem );
		m_pItems.RemoveAt( pos );

		Invalidate();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CWndTabBar message handlers

int CWndTabBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 ) return -1;

	//if ( Settings.Skin.MenuBorders )
	m_dwStyle |= CBRS_BORDER_3D;

	m_pImages.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 );

	ENABLE_DROP()

	return 0;
}

void CWndTabBar::OnDestroy()
{
	DISABLE_DROP()

	KillTimer( 1 );

	CControlBar::OnDestroy();
}

CSize CWndTabBar::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	CSize size( 32767, Settings.Skin.TaskbarHeight );

	if ( CWnd* pParent = AfxGetMainWnd() )
	{
		CRect rc;
		pParent->GetWindowRect( &rc );
		if ( rc.Width() > 32 )
			size.cx = rc.Width();	// + 2; Why?
	}

	return size;
}

void CWndTabBar::OnSkinChange()
{
	SetWatermark( Skin.GetWatermark( L"CWndTabBar" ) );
	SetMaximumWidth( Settings.General.GUIMode == GUI_WINDOWED ? 140 : 200 );
	m_nCloseImage = CoolInterface.ImageForID( ID_CHILD_CLOSE );
	m_nCloseImageHover = CoolInterface.ImageForID( ID_CHILD_CLOSE_HOVER );
	if (m_nCloseImageHover == -1) m_nCloseImageHover = m_nCloseImage;
	// ToDo: m_pImages.SetBkColor if not skinned
}

void CWndTabBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL /*bDisableIfNoHndler*/)
{
	if ( ! IsWindow( m_hWnd ) ) return;
	if ( ! pTarget->IsKindOf( RUNTIME_CLASS(CMainWnd) ) ) return;

	CMainWnd* pMainWnd = (CMainWnd*)pTarget;
	CWindowManager* pManager = &pMainWnd->m_pWindows;

	BOOL bChanged = FALSE;
	m_nCookie++;

	CChildWnd* pActive = pManager->GetActive();

	if ( pActive && pActive->m_bGroupMode && pActive->m_pGroupParent )
		pActive = pActive->m_pGroupParent;

	for ( POSITION posChild = pManager->GetIterator(); posChild; )
	{
		CChildWnd* pChild = pManager->GetNext( posChild );

		BOOL bFound = FALSE;
		CString strCaption;

		if ( pChild->m_bTabMode ) continue;

		pChild->GetWindowText( strCaption );

		if ( pChild->m_bPanelMode )
		{
			if ( strCaption.Find( L"Search : " ) == 0 )
				strCaption = strCaption.Mid( 9 );
			if ( strCaption.Find( L"Browse Host : " ) == 0 )
				strCaption = strCaption.Mid( 14 );
		}

		for ( POSITION pos = m_pItems.GetHeadPosition(); pos; )
		{
			TabItem* pItem = m_pItems.GetNext( pos );

			if ( pItem->m_hWnd == pChild->GetSafeHwnd() )
			{
				bFound = TRUE;
				pItem->m_nCookie = m_nCookie;

				if ( strCaption != pItem->m_sCaption )
				{
					pItem->m_nImage = ImageIndexForWindow( pChild );
					pItem->m_sCaption = strCaption;
					bChanged = TRUE;
				}
				else if ( pItem->m_nImage < 0 )
				{
					pItem->m_nImage = ImageIndexForWindow( pChild );
					bChanged = TRUE;
				}

				if ( pChild->IsWindowVisible() != pItem->m_bVisible )
				{
					pItem->m_bVisible = pChild->IsWindowVisible();
					bChanged = TRUE;
				}

				if ( pChild->m_bAlert != pItem->m_bAlert )
				{
					pItem->m_bAlert = pChild->m_bAlert;
					bChanged = TRUE;
				}

				if ( pChild == pActive && m_pSelected != pItem )
				{
					m_pSelected	= pItem;
					bChanged	= TRUE;
				}
				else if ( m_pSelected == pItem && pChild != pActive )
				{
					m_pSelected	= NULL;
					bChanged	= TRUE;
				}

				break;
			}
		}

		if ( bFound ) continue;

		TabItem* pItem = new TabItem( pChild, m_nCookie, strCaption );
		if ( pActive == pChild ) m_pSelected = pItem;
		pItem->m_nImage = ImageIndexForWindow( pChild );
		m_pItems.AddTail( pItem );
		bChanged = TRUE;
	}

	for ( POSITION pos = m_pItems.GetHeadPosition(); pos; )
	{
		POSITION posOld = pos;
		TabItem* pItem = m_pItems.GetNext( pos );

		if ( pItem->m_nCookie != m_nCookie )
		{
			if ( m_pSelected == pItem ) m_pSelected = NULL;
			delete pItem;
			m_pItems.RemoveAt( posOld );
			bChanged = TRUE;
		}
	}

	if ( bChanged ) Invalidate();
}

int CWndTabBar::ImageIndexForWindow(CWnd* pChild)
{
	CRuntimeClass* pClass = pChild->GetRuntimeClass();
	int nImage = 0;

	if ( pClass != RUNTIME_CLASS(CPluginWnd) )
	{
		if ( m_pIcons.Lookup( pClass, nImage ) ) return nImage;
		nImage = WORD( m_pImages.Add( pChild->GetIcon( FALSE ) ) );
		m_pIcons.SetAt( pClass, nImage );
	}
	else
	{
		HICON hIcon = pChild->GetIcon( FALSE );
		if ( m_pIcons.Lookup( hIcon, nImage ) ) return nImage;
		nImage = WORD( m_pImages.Add( hIcon ) );
		m_pIcons.SetAt( hIcon, nImage );
	}

	return nImage;
}

CWndTabBar::TabItem* CWndTabBar::HitTest(const CPoint& point, CRect* pItemRect) const
{
	if ( m_pItems.IsEmpty() ) return NULL;

	CRect rc;
	GetClientRect( &rc );
	CalcInsideRect( rc, FALSE );

	rc.left -= m_cyTopBorder;
	rc.top  -= m_cxLeftBorder;
	rc.right += m_cyBottomBorder;
	rc.bottom += m_cxRightBorder;

	CRect rcItem( rc.left + 3, rc.top + 1, 0, rc.bottom - 1 );
	rcItem.right = static_cast< LONG >( ( rc.Width() - 3 * m_pItems.GetCount() ) / m_pItems.GetCount() + 3 );
	rcItem.right = min( rcItem.right, m_nMaximumWidth );
	const int nOffset = rcItem.right;

	for ( POSITION pos = m_pItems.GetHeadPosition(); pos; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );
		if ( rcItem.PtInRect( point ) )
		{
			if ( pItemRect ) *pItemRect = rcItem;
			return pItem;
		}
		rcItem.OffsetRect( nOffset, 0 );
	}

	return NULL;
}

INT_PTR CWndTabBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rcItem;
	TabItem* pItem = HitTest( point, &rcItem );

	if ( pItem == NULL ) return -1;
	if ( ! pTI ) return 1;

	pTI->uFlags		= 0;
	pTI->hwnd		= GetSafeHwnd();
	pTI->uId		= (UINT_PTR)pItem->m_hWnd;
	pTI->rect		= rcItem;
	pTI->lpszText	= _tcsdup( pItem->m_sCaption );

	return pTI->uId;
}

void CWndTabBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CDC* pOutDC = pDC;

	CRect rc;
	GetClientRect( &rc );

	if ( m_bmImage.m_hObject != NULL )
	{
		CSize size = rc.Size();
		pDC = CoolInterface.GetBuffer( *pDC, size );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmImage );
	}

	if ( Settings.Skin.MenuBorders )
		DrawBorders( pDC, rc );

	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( m_pItems.GetCount() )
	{
		CRect rcItem( rc.left + 3, rc.top + 1, 0, rc.bottom - 1 );
		rcItem.right = static_cast< LONG >( ( rc.Width() - 10 ) / m_pItems.GetCount() );
		rcItem.right = min( rcItem.right, m_nMaximumWidth );

		for ( POSITION pos = m_pItems.GetHeadPosition(); pos; )
		{
			TabItem* pItem = m_pItems.GetNext( pos );

			pItem->Paint( this, pDC, &rcItem, m_pSelected == pItem, m_pHot == pItem, pDC != pOutDC );
			pDC->ExcludeClipRect( &rcItem );

			rcItem.OffsetRect( rcItem.Width() + 3, 0 );
		}

		if ( pDC == pOutDC )
			pDC->FillSolidRect( &rc, Colors.m_crMidtone );
	}
	else
	{
		CSize sz  = pDC->GetTextExtent( m_sMessage );
		CPoint pt = rc.CenterPoint();
		pt.x -= sz.cx / 2;
		pt.y -= sz.cy / 2 + 1;

		pDC->SetBkColor( Colors.m_crMidtone );
		pDC->SetTextColor( Colors.m_crDisabled );

		if ( pDC == pOutDC )
		{
			pDC->SetBkMode( OPAQUE );
			pDC->ExtTextOut( pt.x, pt.y, ETO_CLIPPED|ETO_OPAQUE, &rc, m_sMessage, NULL );
		}
		else
		{
			pDC->SetBkMode( TRANSPARENT );
			pDC->ExtTextOut( pt.x, pt.y, ETO_CLIPPED, &rc, m_sMessage, NULL );
		}

		m_rcMessage.SetRect( pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy );
	}

	pDC->SelectObject( pOldFont );

	if ( pDC != pOutDC )
	{
		GetClientRect( &rc );
		pOutDC->BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );
		pDC->SelectClipRgn( NULL );
	}
}

void CWndTabBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rc;
	TabItem* pItem = HitTest( point, &rc );

	rc.DeflateRect( 3, 3 );
	rc.left = rc.right - 15;
	if ( m_bCloseButton != rc.PtInRect( point ) )
	{
		m_bCloseButton = ! m_bCloseButton;
		rc.left--;
		InvalidateRect( rc );
	}

	if ( pItem != m_pHot )
	{
		m_dwHoverTime = pItem ? GetTickCount() : 0;
		m_pHot = pItem;
		Invalidate();
	}

	if ( ! m_bTimer && m_pHot )
	{
		SetTimer( 1, 100, NULL );
		m_bTimer = TRUE;
	}
	else if ( m_bTimer && ! m_pHot )
	{
		KillTimer( 1 );
		m_bTimer = FALSE;
	}

	CControlBar::OnMouseMove( nFlags, point );
}

void CWndTabBar::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		CRect rcWindow;
		CPoint point;

		GetClientRect( &rcWindow );
		ClientToScreen( &rcWindow );
		GetCursorPos( &point );

		if ( ! rcWindow.PtInRect( point ) )
		{
			KillTimer( nIDEvent );
			m_pHot		= NULL;
			m_bTimer	= FALSE;
			Invalidate();
		}
	}

	CControlBar::OnTimer( nIDEvent );
}

BOOL CWndTabBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_pItems.GetCount() == 0 && m_nMessage == IDS_TABBAR_CONNECTED )
	{
		CPoint point;
		GetCursorPos( &point );
		ScreenToClient( &point );

		if ( m_rcMessage.PtInRect( point ) )
		{
			SetCursor( theApp.LoadCursor( IDC_HAND ) );
			return TRUE;
		}
	}

	return CControlBar::OnSetCursor( pWnd, nHitTest, message );
}

void CWndTabBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	OnMouseMove( nFlags, point );

	CRect rc;
	if ( TabItem* pItem = HitTest( point, &rc ) )
	{
		if ( ::IsWindow( pItem->m_hWnd ) )
		{
			CChildWnd* pChild = (CChildWnd*)CWnd::FromHandle( pItem->m_hWnd );

			if ( m_bCloseButton &&
				( m_pSelected == pItem || rc.Width() > 70 ) &&
				Settings.General.GUIMode != GUI_WINDOWED )
			{
				pChild->PostMessage( WM_SYSCOMMAND, SC_CLOSE, 0 );
				return;
			}
			else if ( pChild->IsIconic() )
			{
				pChild->ShowWindow( SW_SHOWNORMAL );
			}
			else if ( m_pSelected == pItem && ! pChild->m_bPanelMode )
			{
				pChild->ShowWindow( SW_HIDE );
				pChild->ShowWindow( SW_MINIMIZE );
				return;
			}

			pChild->MDIActivate();
		}
		return;
	}

	if ( m_nMessage == IDS_TABBAR_CONNECTED &&
		 m_pItems.GetCount() == 0 &&
		 m_rcMessage.PtInRect( point ) )
		return;

	CControlBar::OnLButtonDown( nFlags, point );
}

void CWndTabBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//if ( TabItem* pItem = HitTest( point ) )
	//{
	//	CMDIChildWnd* pChild = (CMDIChildWnd*)CWnd::FromHandle( pItem->m_hWnd );
	//	pChild->PostMessage( WM_SYSCOMMAND, pChild->IsZoomed() ? SC_RESTORE : SC_MAXIMIZE );
	//	return;
	//}

	OnLButtonDown( nFlags, point );
}

void CWndTabBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rc;

	if ( TabItem* pItem = HitTest( point, &rc ) )
	{
	}
	else if ( m_nMessage == IDS_TABBAR_CONNECTED )
	{
		if ( m_pItems.GetCount() == 0 && m_rcMessage.PtInRect( point ) )
			GetOwner()->PostMessage( WM_COMMAND, ID_NETWORK_SEARCH );
	}

	CControlBar::OnLButtonUp( nFlags, point );
}

void CWndTabBar::OnMButtonUp(UINT nFlags, CPoint point)
{
	if ( TabItem* pItem = HitTest( point ) )
	{
		CChildWnd* pChild = (CChildWnd*)CWnd::FromHandle( pItem->m_hWnd );
		pChild->PostMessage( WM_SYSCOMMAND, SC_CLOSE, 0 );
	}

	CControlBar::OnMButtonUp( nFlags, point );
}

void CWndTabBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	CRect rcItem;

	if ( TabItem* pItem = HitTest( point, &rcItem ) )
	{
		CChildWnd* pChild = (CChildWnd*)CWnd::FromHandle( pItem->m_hWnd );
		if ( pChild->IsWindowVisible() ) pChild->MDIActivate();

		ClientToScreen( &rcItem );

		CMenu* pMenu = Skin.GetMenu( pChild->m_bPanelMode ? L"CTabBar" : L"CTabBar.Child" );

		UINT nCommand		= 0;
		BOOL bCanRestore	= pChild->IsIconic() || pChild->IsZoomed();
		BOOL bCanMove		= m_pItems.GetCount() > 1;

		//MENUITEMINFO pInfo;
		//pInfo.cbSize	= sizeof( pInfo );
		//pInfo.fMask	= MIIM_STATE;
		//GetMenuItemInfo( pMenu->GetSafeHmenu(), ID_CHILD_RESTORE, FALSE, &pInfo );
		//pInfo.fState = ( pInfo.fState & (~MFS_DEFAULT) ) | ( bCanRestore ? MFS_DEFAULT : 0 );
		//SetMenuItemInfo( pMenu->GetSafeHmenu(), ID_CHILD_RESTORE, FALSE, &pInfo );

		pMenu->EnableMenuItem( ID_CHILD_MINIMISE, MF_BYCOMMAND |
			( pChild->IsIconic() ? MF_GRAYED : 0 ) );
		pMenu->EnableMenuItem( ID_CHILD_MAXIMISE, MF_BYCOMMAND |
			( pChild->IsZoomed() ? MF_GRAYED : 0 ) );
		pMenu->EnableMenuItem( ID_CHILD_RESTORE, MF_BYCOMMAND |
			( bCanRestore ? 0 : MF_GRAYED ) );
		pMenu->EnableMenuItem( ID_MOVETAB_LEFT, MF_BYCOMMAND |
			( ( bCanMove && m_pItems.GetHead() != pItem ) ? 0 : MF_GRAYED ) );
		pMenu->EnableMenuItem( ID_MOVETAB_RIGHT, MF_BYCOMMAND |
			( ( bCanMove && m_pItems.GetTail() != pItem ) ? 0 : MF_GRAYED ) );
		m_bMenuGray = TRUE;

		Invalidate();

		CoolMenu.AddMenu( pMenu, TRUE );
		if ( rcItem.bottom > GetSystemMetrics( SM_CYSCREEN ) / 2 )
		{
			nCommand = pMenu->TrackPopupMenu( TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTALIGN|TPM_BOTTOMALIGN,
				Settings.General.LanguageRTL ? rcItem.right : rcItem.left,
				rcItem.top + 1, this );
		}
		else
		{
			CoolMenu.RegisterEdge( Settings.General.LanguageRTL ? rcItem.right : rcItem.left,
				rcItem.bottom - 1, rcItem.Width() );
			nCommand = pMenu->TrackPopupMenu( TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN,
				Settings.General.LanguageRTL ? rcItem.right : rcItem.left,
				rcItem.bottom - 1, this );
		}

		switch ( nCommand )
		{
		case ID_MOVETAB_LEFT:
			MoveLeft( pItem );
			break;
		case ID_MOVETAB_RIGHT:
			MoveRight( pItem );
			break;
		case ID_MOVETAB_FIRST:
			MoveFirst( pItem );
			break;
		case ID_MOVETAB_LAST:
			MoveLast( pItem );
			break;
		case ID_CHILD_RESTORE:
			pChild->PostMessage( WM_SYSCOMMAND, SC_RESTORE, 0 );
			break;
		case ID_CHILD_MINIMISE:
			pChild->PostMessage( WM_SYSCOMMAND, SC_MINIMIZE, 0 );
			break;
		case ID_CHILD_MAXIMISE:
			pChild->PostMessage( WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
			break;
		case ID_CHILD_CLOSE:
			pChild->PostMessage( WM_SYSCOMMAND, SC_CLOSE, 0 );
			break;
		}

		m_bMenuGray = FALSE;
		Invalidate();

		return;
	}

	CControlBar::OnRButtonUp( nFlags, point );
}

void CWndTabBar::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CoolMenu.OnMeasureItem( lpMeasureItemStruct );
}

void CWndTabBar::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CoolMenu.OnDrawItem( lpDrawItemStruct );
}

/////////////////////////////////////////////////////////////////////////////
// CWndTabBar IDropTarget implementation

IMPLEMENT_DROP(CWndTabBar, CControlBar)

BOOL CWndTabBar::OnDrop(IDataObject* pDataObj, DWORD /*grfKeyState*/, POINT ptScreen, DWORD* /*pdwEffect*/, BOOL /*bDrop*/)
{
	// Mouse move imitation during dragging
	CPoint pt( ptScreen );
	ScreenToClient( &pt );
	OnMouseMove( 0, pt );

	if ( pDataObj )
	{
		// DragEnter or DragOver
		if ( m_pHot && m_dwHoverTime )
		{
			if ( GetTickCount() - m_dwHoverTime >= DRAG_HOVER_TIME )
			{
				m_dwHoverTime = 0;
				CChildWnd* pChild = (CChildWnd*)CWnd::FromHandle( m_pHot->m_hWnd );
				if ( pChild->IsIconic() )
					pChild->ShowWindow( SW_SHOWNORMAL );
				pChild->MDIActivate();
			}
		}
	}
	else
	{
		// DragLeave
		m_dwHoverTime = 0;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CWndTabBar::TabItem construction

CWndTabBar::TabItem::TabItem(CChildWnd* pWnd, DWORD nCookie, LPCTSTR pszCaption)
{
	m_hWnd		= pWnd->GetSafeHwnd();
	m_pClass	= pWnd->GetRuntimeClass();
	m_bVisible	= pWnd->IsWindowVisible();
	m_bAlert	= pWnd->m_bAlert;
	m_nCookie	= nCookie;
	m_nImage	= -1;

	if ( pszCaption != NULL )
		m_sCaption = pszCaption;
	else
		pWnd->GetWindowText( m_sCaption );
}

CWndTabBar::TabItem::~TabItem()
{
}

/////////////////////////////////////////////////////////////////////////////
// CWndTabBar::TabItem paint

void CWndTabBar::TabItem::Paint(CWndTabBar* pBar, CDC* pDC, CRect* pRect, BOOL bSelected, BOOL bHot, BOOL bTransparent)
{
	CRect rc( pRect );
	COLORREF crBack;
	BOOL bSkinned = FALSE;

	if ( ! Settings.Skin.MenuBorders )
	{
		if ( ! bTransparent )
			pDC->FillSolidRect( rc, Colors.m_crMidtone );
		rc.DeflateRect( 0, 2 );
	}

	rc.InflateRect( 1, 1 );

	if ( bSelected && ( bHot || pBar->m_bMenuGray ) )
		bSkinned = Images.DrawButtonState( pDC, &rc, TASKBARBUTTON_PRESS );		// "CWndTabBar.Active.Hover"
	else if ( bSelected )
		bSkinned = Images.DrawButtonState( pDC, &rc, TASKBARBUTTON_ACTIVE );	// "CWndTabBar.Active"
	else if ( bHot )
		bSkinned = Images.DrawButtonState( pDC, &rc, TASKBARBUTTON_HOVER );		// "CWndTabBar.Hover"
	else
		bSkinned = Images.DrawButtonState( pDC, &rc, TASKBARBUTTON_DEFAULT );	// "CWndTabBar.Tab"

	rc.DeflateRect( 1, 1 );

	if ( bSkinned )
	{
		crBack = CLR_NONE;
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		if ( bHot || ( bSelected && m_bVisible ) )
		{
			crBack = ( bHot && bSelected ) ? Colors.m_crBackCheckSel : Colors.m_crBackSel;
			pDC->Draw3dRect( &rc, Colors.m_crBorder, Colors.m_crBorder );
		}
		else if ( bSelected && pBar->m_bMenuGray )
		{
			crBack = Colors.m_crBackNormal;
			pDC->Draw3dRect( &rc, Colors.m_crDisabled, Colors.m_crDisabled );
		}
		else
		{
			crBack = bTransparent ? CLR_NONE : Colors.m_crMidtone;
			if ( crBack != CLR_NONE ) pDC->Draw3dRect( &rc, crBack, crBack );
		}

		if ( crBack != CLR_NONE ) pDC->SetBkColor( crBack );
	}

	rc.DeflateRect( 1, 1 );

	CPoint ptImage( rc.left + 3, rc.top + 1 );

	// Icon
	if ( m_nImage != -1 )
	{
		if ( bSelected )
		{
			ImageList_DrawEx( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
				ptImage.x, ptImage.y, 0, 0, crBack, CLR_NONE, ILD_NORMAL );
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		}
		else if ( ! bHot )
		{
			ImageList_DrawEx( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
				ptImage.x, ptImage.y, 0, 0, crBack, Colors.m_crShadow, ILD_BLEND50 );
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		}
		else // bHot
		{
			ptImage.Offset( 1, 1 );

			if ( crBack != CLR_NONE )
			{
				pDC->SetTextColor( Colors.m_crShadow );
				ImageList_DrawEx( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
					ptImage.x, ptImage.y, 0, 0, crBack, CLR_NONE, ILD_MASK );

				ptImage.Offset( -2, -2 );

				pDC->FillSolidRect( ptImage.x, ptImage.y, 18, 2, crBack );
				pDC->FillSolidRect( ptImage.x, ptImage.y + 2, 2, 16, crBack );
			}
			else	// Skinned
			{
			//	ImageList_DrawEx( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
			//		ptImage.x, ptImage.y, 0, 0, CLR_NONE, Colors.m_crShadow, ILD_BLEND50|ILD_BLEND25 );

				pBar->m_pImages.DrawIndirect( pDC, m_nImage, ptImage, CSize( 16, 16 ), CPoint(0),
					ILD_BLEND50|ILD_BLEND25|ILD_ROP, MERGECOPY, CLR_NONE, Colors.m_crShadow, ILS_SATURATE|ILS_ALPHA, 160, Colors.m_crShadow );

				ptImage.Offset( -2, -2 );

			//	pBar->m_pImages.DrawIndirect( pDC, m_nImage, ptImage, CSize( 16, 16 ), CPoint(0),
			//		ILD_ROP, MERGECOPY, CLR_NONE, CLR_NONE, ILS_SHADOW, 0, Colors.m_crShadow );
			}

			ImageList_Draw( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(), ptImage.x, ptImage.y, ILD_NORMAL );

			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 18, ptImage.y + 18 );

			ptImage.Offset( 1, 1 );
		}
	}

	rc.left += 16 + 6;

	// ID_CHILD_CLOSE "X"
	if ( ( bSelected || ( bHot && rc.Width() > 70 ) ) && Settings.General.GUIMode != GUI_WINDOWED )
	{
		rc.right -= 16;
		ptImage.x = rc.right;
		CoolInterface.DrawEx( pDC, ( bHot && pBar->m_bCloseButton ) ? pBar->m_nCloseImageHover : pBar->m_nCloseImage,
			ptImage, (CSize)0, crBack, Colors.m_crShadow, ( !bHot && pBar->m_nCloseImage == pBar->m_nCloseImageHover ) ? ILD_BLEND50 : ILD_NORMAL );
		if ( crBack != CLR_NONE )
		{
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
			pDC->FillSolidRect( rc.right, rc.top, 16, rc.Height(), crBack );
		}
	}

	CString strText = m_sCaption;

	if ( pDC->GetTextExtent( strText ).cx > rc.Width() )
	{
		while ( pDC->GetTextExtent( strText + L'\x2026' ).cx > rc.Width() && ! strText.IsEmpty() )
		{
			strText = strText.Left( strText.GetLength() - 1 );
		}

		if ( ! strText.IsEmpty() ) strText += L'\x2026';
	}

	rc.left -= 16 + 6;	// Paint spaces

	if ( m_bAlert )
		pDC->SetTextColor( Colors.m_crTextAlert );
	else if ( ! m_bVisible && ! bHot )
		pDC->SetTextColor( Colors.m_crDisabled );
	else if ( bSelected || bHot )
		pDC->SetTextColor( Colors.m_crCmdTextSel );
	else
		pDC->SetTextColor( Colors.m_crCmdText );

	if ( crBack != CLR_NONE )
	{
		pDC->SetBkColor( crBack );
		pDC->SetBkMode( OPAQUE );
		pDC->ExtTextOut( rc.left + 16 + 6, rc.top + 2, ETO_CLIPPED|ETO_OPAQUE, &rc, strText, NULL );
	}
	else
	{
		pDC->SetBkMode( TRANSPARENT );
		pDC->ExtTextOut( rc.left + 16 + 6, rc.top + 2, ETO_CLIPPED, &rc, strText, NULL );
	}
}
