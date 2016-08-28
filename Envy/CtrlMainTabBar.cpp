//
// CtrlMainTabBar.cpp
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
#include "CtrlMainTabBar.h"

#include "CtrlCoolBar.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Skin.h"
#include "SkinWindow.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CMainTabBarCtrl, CControlBar)

BEGIN_MESSAGE_MAP(CMainTabBarCtrl, CControlBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl construction

CMainTabBarCtrl::CMainTabBarCtrl()
	: m_pSkin	( NULL )
	, m_pHover	( NULL )
	, m_pDown	( NULL )
	, m_dwHoverTime ( 0 )
{
}

CMainTabBarCtrl::~CMainTabBarCtrl()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete m_pItems.GetNext( pos );
	}

	if ( m_dcSkin.m_hDC )
	{
		if ( m_hOldSkin ) m_dcSkin.SelectObject( CBitmap::FromHandle( m_hOldSkin ) );
		m_dcSkin.DeleteDC();
	}

	if ( m_bmSkin.m_hObject ) m_bmSkin.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl operations

BOOL CMainTabBarCtrl::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc;
	dwStyle |= WS_CHILD;
	return CWnd::Create( NULL, NULL, dwStyle, rc, pParentWnd, nID, NULL );
}

BOOL CMainTabBarCtrl::HasLocalVersion()
{
	return ( m_pSkin != NULL ) && ( m_pSkin->m_sLanguage == Settings.General.Language );
}

void CMainTabBarCtrl::OnSkinChange()
{
	if ( m_pItems.GetCount() == 0 )
	{
		m_pItems.AddTail( new TabItem( this, L"_ID_TAB_HOME" ) );
		m_pItems.AddTail( new TabItem( this, L"_ID_TAB_TRANSFERS" ) );
		m_pItems.AddTail( new TabItem( this, L"_ID_TAB_NETWORK" ) );
		m_pItems.AddTail( new TabItem( this, L"_ID_TAB_LIBRARY" ) );
		m_pItems.AddTail( new TabItem( this, L"_ID_TAB_MEDIA" ) );
		m_pItems.AddTail( new TabItem( this, L"_ID_TAB_IRC" ) );
		m_pItems.AddTail( new TabItem( this, L"_ID_TAB_SEARCH" ) );
	//	m_pItems.AddTail( new TabItem( this, L"_ID_TAB_MENU" ) );	// ToDo:
	}

	if ( m_dcSkin.m_hDC )
	{
		if ( m_hOldSkin ) m_dcSkin.SelectObject( CBitmap::FromHandle( m_hOldSkin ) );
		m_dcSkin.DeleteDC();
	}

	if ( m_bmSkin.m_hObject ) m_bmSkin.DeleteObject();

	m_pSkin = Skin.GetWindowSkin( this );

	if ( m_pSkin != NULL )
	{
		BITMAP pInfo;
		CDC dcScreen;

		dcScreen.Attach( ::GetDC( 0 ) );
		m_pSkin->Prepare( &dcScreen );
		m_pSkin->m_bmSkin.GetBitmap( &pInfo );
		m_dcSkin.CreateCompatibleDC( &dcScreen );
		m_bmSkin.CreateCompatibleBitmap( &dcScreen, pInfo.bmWidth, pInfo.bmHeight );
		::ReleaseDC( 0, dcScreen.Detach() );

		m_hOldSkin = (HBITMAP)m_dcSkin.SelectObject( &m_bmSkin )->GetSafeHandle();
		m_dcSkin.BitBlt( 0, 0, pInfo.bmWidth, pInfo.bmHeight, &m_pSkin->m_dcSkin, 0, 0, SRCCOPY );
		m_dcSkin.SelectObject( CBitmap::FromHandle( m_hOldSkin ) );

		m_hOldSkin = NULL;

		for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			// TabItem* pItem
			m_pItems.GetNext( pos )->OnSkinChange( m_pSkin, &m_dcSkin, &m_bmSkin );
		}

		m_hOldSkin = (HBITMAP)m_dcSkin.SelectObject( &m_bmSkin )->GetSafeHandle();
	}

	CMDIFrameWnd* pOwner = (CMDIFrameWnd*)GetOwner();

	if ( pOwner != NULL && pOwner->IsKindOf( RUNTIME_CLASS(CMDIFrameWnd) ) && ! pOwner->IsIconic() )
		pOwner->RecalcLayout();
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl message handlers

int CMainTabBarCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 ) return -1;

	//if ( Settings.Skin.MenuBorders )
	m_dwStyle |= CBRS_BORDER_3D;

	SetTimer( 1, 250, NULL );

	ENABLE_DROP()

	return 0;
}

void CMainTabBarCtrl::OnDestroy()
{
	DISABLE_DROP()

	KillTimer( 1 );

	CControlBar::OnDestroy();
}

void CMainTabBarCtrl::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL /*bDisableIfNoHndler*/)
{
	BOOL bChanged = FALSE;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );
		bChanged |= pItem->Update( pTarget );
	}

	if ( bChanged ) Invalidate();
}

CSize CMainTabBarCtrl::CalcFixedLayout(BOOL bStretch, BOOL /*bHorz*/)
{
	CRect rcBackground;
	CSize size( 0, Settings.Skin.ToolbarHeight );	// ToDo: Use Unique Size

	if ( m_pSkin != NULL && m_pSkin->GetAnchor( L"Background", rcBackground ) )
		size = rcBackground.Size();

	if ( bStretch )
	{
		size.cx = 32000;

		if ( CWnd* pParent = GetOwner() )
		{
			CRect rc;
			pParent->GetWindowRect( &rc );
			if ( rc.Width() > 32 )
				size.cx = rc.Width() + 2;
		}
	}

	return size;
}

CMainTabBarCtrl::TabItem* CMainTabBarCtrl::HitTest(const CPoint& point) const
{
	CPoint ptLocal( point );
	CRect rcClient;

	GetClientRect( &rcClient );
	CalcInsideRect( rcClient, FALSE );
	//if ( Settings.General.LanguageRTL ) ptLocal.x = 2 * rcClient.left + rcClient.Width() - ptLocal.x;
	rcClient.left -= m_cyTopBorder;
	rcClient.top -= m_cxLeftBorder;
	rcClient.right += m_cyBottomBorder;
	rcClient.bottom += m_cxRightBorder;
	ptLocal.Offset( -rcClient.left, -rcClient.top );

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );
		if ( pItem->HitTest( ptLocal ) ) return pItem;
	}

	return NULL;
}

INT_PTR CMainTabBarCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	TabItem* pItem = HitTest( point );

	if ( pItem == NULL ) return -1;
	if ( pTI == NULL ) return 1;

	pTI->uFlags		= 0;
	pTI->hwnd		= GetSafeHwnd();
	pTI->uId		= pItem->m_nID;
	pTI->rect		= pItem->m_rc;
	pTI->lpszText	= LPSTR_TEXTCALLBACK;

	CString strTip;

	if ( LoadString( strTip, pItem->m_nID ) )
	{
		if ( LPCTSTR pszBreak = _tcschr( strTip, '\n' ) )
		{
			pTI->lpszText = _tcsdup( pszBreak + 1 );
		}
		else
		{
			strTip = strTip.SpanExcluding( L"." );
			pTI->lpszText = _tcsdup( strTip );
		}
	}

	return pTI->uId;
}

void CMainTabBarCtrl::DoPaint(CDC* pDC)
{
	ASSERT_VALID( this );
	ASSERT_VALID( pDC );

	CRect rc;
	GetClientRect( &rc );

	if ( m_pSkin == NULL )
	{
		DrawBorders( pDC, rc );
		pDC->FillSolidRect( &rc, Colors.m_crMidtone );
		return;
	}
	else if ( m_hOldSkin == NULL )
	{
		for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			TabItem* pItem = m_pItems.GetNext( pos );
			pItem->OnSkinChange( m_pSkin, &m_dcSkin, &m_bmSkin );
		}

		m_hOldSkin = (HBITMAP)m_dcSkin.SelectObject( &m_bmSkin )->GetSafeHandle();
	}

	CSize size = rc.Size();
	CDC* pBuffer = CoolInterface.GetBuffer( *pDC, size );

	if ( Settings.Skin.MenuBorders )
		DrawBorders( pBuffer, rc );
	else
		rc.DeflateRect( 2, 2, 0, 0 );

	if ( ! CoolInterface.DrawWatermark( pBuffer, &rc, &m_pSkin->m_bmWatermark ) )
		pBuffer->FillSolidRect( &rc, Colors.m_crMidtone );

	CPoint ptOffset = rc.TopLeft();

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );
		pItem->Paint( pBuffer, &m_dcSkin, ptOffset,
			( m_pHover == pItem && m_pDown == NULL ) || ( m_pDown == pItem ),
			( m_pDown == pItem ) && ( m_pHover == pItem ) );
	}

	GetClientRect( &rc );
	//if ( Settings.General.LanguageRTL ) SetLayout( pDC->m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );
	pDC->BitBlt( 0, 0, rc.Width(), rc.Height(), pBuffer, 0, 0, SRCCOPY );
}

BOOL CMainTabBarCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( nHitTest == HTCLIENT )
	{
		CPoint point;
		GetCursorPos( &point );
		ScreenToClient( &point );

		if ( TabItem* pItem = HitTest( point ) )
		{
		//	if ( pItem->m_bEnabled )
			{
				SetCursor( theApp.LoadCursor( IDC_HAND ) );
				return TRUE;
			}
		}
	}

	return CControlBar::OnSetCursor( pWnd, nHitTest, message );
}

void CMainTabBarCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	TabItem* pItem = HitTest( point );

	if ( pItem != m_pHover )
	{
		m_dwHoverTime = pItem ? GetTickCount() : 0;
		m_pHover = pItem;
		Invalidate();
	}

	CControlBar::OnMouseMove( nFlags, point );
}

void CMainTabBarCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );
}

void CMainTabBarCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_pHover = HitTest( point );

	if ( m_pHover != NULL )		// && m_pHover->m_bEnabled
		m_pDown = m_pHover;

	if ( m_pHover == NULL )
		CControlBar::OnLButtonDown( nFlags, point );
	else
		SetCapture();

	Invalidate();
}

void CMainTabBarCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( m_pDown != NULL && m_pHover == m_pDown )
		GetOwner()->PostMessage( WM_COMMAND, m_pDown->m_nID );

	m_pDown = NULL;
	ReleaseCapture();
	Invalidate();

	if ( m_pHover == NULL )
		CControlBar::OnLButtonUp( nFlags, point );
}

void CMainTabBarCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ( m_pDown != NULL || m_pHover != NULL )
	{
		m_pDown = m_pHover = NULL;
		ReleaseCapture();
		Invalidate();
	}

	CControlBar::OnRButtonDown( nFlags, point );
}

void CMainTabBarCtrl::OnTimer(UINT_PTR /*nIDEvent*/)
{
	if ( m_pHover != NULL && m_pDown == NULL )
	{
		CPoint point;
		CRect rect;

		GetCursorPos( &point );
		GetWindowRect( &rect );

		if ( rect.PtInRect( point ) == FALSE )
		{
			m_pHover = NULL;
			Invalidate();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl IDropTarget implementation

IMPLEMENT_DROP(CMainTabBarCtrl, CControlBar)

BOOL CMainTabBarCtrl::OnDrop(IDataObject* pDataObj, DWORD /*grfKeyState*/, POINT ptScreen, DWORD* /*pdwEffect*/, BOOL /*bDrop*/)
{
	// Mouse move imitation during dragging
	CPoint pt( ptScreen );
	ScreenToClient( &pt );
	OnMouseMove( 0, pt );

	if ( pDataObj )
	{
		// DragEnter or DragOver
		if ( m_pHover && m_dwHoverTime )
		{
			if ( GetTickCount() - m_dwHoverTime >= DRAG_HOVER_TIME )
			{
				m_dwHoverTime = 0;
				switch ( m_pHover->m_nID )
				{
				// Ignore Some Buttons
				case ID_TAB_CONNECT:
				case ID_NETWORK_CONNECT:
				case ID_NETWORK_DISCONNECT:
				case ID_TAB_SEARCH:
					break;
				default:
					GetOwner()->PostMessage( WM_COMMAND, m_pHover->m_nID );
				}
			}
		}
	}
	else	// DragLeave
	{
		m_dwHoverTime = 0;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem

// Match pszState Order for m_rcSrc
enum {
	STATE_DEFAULT,
	STATE_SELECTED,
	STATE_HOVER,
	STATE_DOWN
//	STATE_DISABLED	// Unused
};

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem construction

CMainTabBarCtrl::TabItem::TabItem(CMainTabBarCtrl* pCtrl, LPCTSTR pszName)
{
	m_pCtrl		= pCtrl;
	m_nID		= CoolInterface.NameToID( pszName + 1 );
	m_sName		= pszName;
	m_bSelected	= FALSE;
//	m_bEnabled	= TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem skin change

void CMainTabBarCtrl::TabItem::OnSkinChange(CSkinWindow* pSkin, CDC* pdcCache, CBitmap* pbmCache)
{
	// Note order must match STATE_ enum
	static LPCTSTR pszState[] =
	{
		L".Default", L".Selected", L".Hover", L".Down", NULL	// Null to end loop
	};

	// Deprecated use, order must match
	static LPCTSTR pszStateLegacy[] =
	{
		L".Up", L".Checked", L".Hover", L".Down", NULL	// ".Disabled" Unused
	};

	m_rc.SetRectEmpty();
	pSkin->GetAnchor( m_sName, m_rc );

	// Mimic main window toolbar text
	if ( CCoolBarCtrl* pBar = Skin.GetToolBar( L"CMainWnd" ) )
	if ( CCoolBarItem* pItem = pBar->GetID( m_nID ) )
	{
		m_sTitle = pItem->m_sText;
		switch ( Skin.m_NavBarMode )
		{
		case CSkin::NavBarUpper:
			m_sTitle.MakeUpper();
			break;
		case CSkin::NavBarLower:
			m_sTitle.MakeLower();
			break;
		//default:
		//	break;
		}
	}

	for ( int nState = 0 ; pszState[ nState ] != NULL ; nState++ )
	{
		CRect* pRect = &m_rcSrc[ nState ];
		pRect->SetRectEmpty();

		if ( pSkin->GetPart( m_sName + pszState[ nState ], *pRect ) ||
			 pSkin->GetPart( m_sName + pszStateLegacy[ nState ], *pRect ) )		// Deprecated fallback
		{
			CBitmap* pOld = (CBitmap*)pdcCache->SelectObject( pbmCache );

			if ( pSkin->m_bmWatermark.m_hObject != NULL )
			{
				pdcCache->IntersectClipRect( pRect );
				CoolInterface.DrawWatermark( pdcCache, pRect, &pSkin->m_bmWatermark, m_rc.left, m_rc.top );
				pdcCache->SelectClipRgn( NULL );
			}
			else
			{
				pdcCache->FillSolidRect( pRect, Colors.m_crMidtone );
			}

			pdcCache->SelectObject( pOld );

			pSkin->PreBlend( pbmCache, *pRect, *pRect );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem GUI update

BOOL CMainTabBarCtrl::TabItem::Update(CFrameWnd* pWnd)
{
//	BOOL bEnabled  = m_bEnabled;
	BOOL bSelected = m_bSelected;

	DoUpdate( pWnd, FALSE );	// TRUE handles bEnabled (runtime error otherwise)

	return ( bSelected != m_bSelected );	// || bEnabled != m_bEnabled
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem hit test

BOOL CMainTabBarCtrl::TabItem::HitTest(const CPoint& point) const
{
	return m_rc.PtInRect( point );
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem paint

void CMainTabBarCtrl::TabItem::Paint(CDC* pDstDC, CDC* pSrcDC, const CPoint& ptOffset, BOOL bHover, BOOL bDown)
{
	if ( ! m_rc.Width() || ! m_rc.Height() )
		return;		// No button

	CRect* pPart = NULL;

//	if ( ! m_bEnabled )
//		pPart = &m_rcSrc[ STATE_DISABLED ];
	if ( bDown )
		pPart = &m_rcSrc[ STATE_DOWN ];
	else if ( m_bSelected )
		pPart = &m_rcSrc[ STATE_SELECTED ];
	else if ( bHover )
		pPart = &m_rcSrc[ STATE_HOVER ];
	else
		pPart = &m_rcSrc[ STATE_DEFAULT ];

	CRect rcTarget( m_rc );
	rcTarget += ptOffset;

	pDstDC->BitBlt( rcTarget.left, rcTarget.top, rcTarget.Width(), rcTarget.Height(),
		pSrcDC, pPart->left, pPart->top, SRCCOPY );

	// Draw button label

	if ( m_sTitle.IsEmpty() )
		return;		// No label

	if ( Colors.m_crNavBarText == CLR_NONE
		&& Colors.m_crNavBarTextUp == CLR_NONE
		&& Colors.m_crNavBarTextHover == CLR_NONE
		&& Colors.m_crNavBarTextChecked == CLR_NONE )
	{
		return;		// No caption text
	}

	COLORREF crNavBarText = Colors.m_crNavBarText;
	COLORREF crNavBarShadow = Colors.m_crNavBarShadow;
	COLORREF crNavBarOutline = Colors.m_crNavBarOutline;

	if ( bDown )
	{
		crNavBarText = Colors.m_crNavBarTextDown != CLR_NONE ? Colors.m_crNavBarTextDown : Colors.m_crNavBarText;
		crNavBarShadow = Colors.m_crNavBarShadowDown != CLR_NONE ? Colors.m_crNavBarShadowDown : Colors.m_crNavBarShadow;
		crNavBarOutline = Colors.m_crNavBarOutlineDown != CLR_NONE ? Colors.m_crNavBarOutlineDown : Colors.m_crNavBarOutline;
	}
	else if ( m_bSelected )
	{
		crNavBarText = Colors.m_crNavBarTextChecked != CLR_NONE ? Colors.m_crNavBarTextChecked : Colors.m_crNavBarText;
		crNavBarShadow = Colors.m_crNavBarShadowChecked != CLR_NONE ? Colors.m_crNavBarShadowChecked : Colors.m_crNavBarShadow;
		crNavBarOutline = Colors.m_crNavBarOutlineChecked != CLR_NONE ? Colors.m_crNavBarOutlineChecked : Colors.m_crNavBarOutline;
	}
	else if ( bHover )
	{
		crNavBarText = Colors.m_crNavBarTextHover != CLR_NONE ? Colors.m_crNavBarTextHover : Colors.m_crNavBarText;
		crNavBarShadow = Colors.m_crNavBarShadowHover != CLR_NONE ? Colors.m_crNavBarShadowHover : Colors.m_crNavBarShadow;
		crNavBarOutline = Colors.m_crNavBarOutlineHover != CLR_NONE ? Colors.m_crNavBarOutlineHover : Colors.m_crNavBarOutline;
	}
	else if ( Colors.m_crNavBarTextUp != CLR_NONE )
	{
		crNavBarText = Colors.m_crNavBarTextUp;
		crNavBarShadow = Colors.m_crNavBarShadowUp != CLR_NONE ? Colors.m_crNavBarShadowUp : Colors.m_crNavBarShadow;
		crNavBarOutline = Colors.m_crNavBarOutlineUp != CLR_NONE ? Colors.m_crNavBarOutlineUp : Colors.m_crNavBarOutline;
	}

	if ( Settings.General.LanguageRTL )
		rcTarget.left -= Skin.m_ptNavBarOffset.x;
	else
		rcTarget.left += Skin.m_ptNavBarOffset.x;

	rcTarget.top += Skin.m_ptNavBarOffset.y;

	CFont* pOldFont = pDstDC->SelectObject(
		m_bSelected ? &CoolInterface.m_fntNavBarActive :
		bHover ? &CoolInterface.m_fntNavBarHover :
		&CoolInterface.m_fntNavBar );

	pDstDC->SetBkMode( TRANSPARENT );

	if ( crNavBarOutline != CLR_NONE )
	{
		pDstDC->SetTextColor( crNavBarOutline );
		for ( int x = -1 ; x < 2 ; x++ )
		{
			for ( int y = -1 ; y < 2 ; y++ )
			{
				if ( x || y )
					pDstDC->DrawText( m_sTitle, rcTarget + CPoint( x, y ), DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );
			}
		}
	}
	if ( crNavBarShadow != CLR_NONE )
	{
		pDstDC->SetTextColor( crNavBarShadow );
		pDstDC->DrawText( m_sTitle, rcTarget + CPoint( 1, 1 ), DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );
	}
	if ( crNavBarText != CLR_NONE )
	{
		pDstDC->SetTextColor( crNavBarText );
		pDstDC->DrawText( m_sTitle, rcTarget, DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );
	}

	pDstDC->SelectObject( pOldFont );
}

//void CMainTabBarCtrl::TabItem::Enable(BOOL bEnable)
//{
//	m_bEnabled = bEnable;
//}

void CMainTabBarCtrl::TabItem::SetCheck(BOOL bCheck)
{
	m_bSelected = bCheck;
}
