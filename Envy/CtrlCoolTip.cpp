//
// CtrlCoolTip.cpp
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
#include "CtrlCoolTip.h"

#include "Images.h"
#include "Colors.h"
#include "CoolInterface.h"
#include "GraphLine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CCoolTipCtrl, CWnd)

BEGIN_MESSAGE_MAP(CCoolTipCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()

LPCTSTR CCoolTipCtrl::m_hClass = NULL;


/////////////////////////////////////////////////////////////////////////////
// CCoolTipCtrl construction

CCoolTipCtrl::CCoolTipCtrl()
	: m_pbEnable	( NULL )
	, m_hAltWnd 	( NULL )
	, m_bTimer		( FALSE )
	, m_bVisible	( FALSE )
	, m_tOpen		( 0 )
{
	if( m_hClass == NULL )
		m_hClass = AfxRegisterWndClass( CS_SAVEBITS | ( Settings.Interface.TipShadow ? CS_DROPSHADOW : 0 ) );
}

CCoolTipCtrl::~CCoolTipCtrl()
{
	if ( m_hWnd != NULL ) DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CCoolTipCtrl operations

BOOL CCoolTipCtrl::Create(CWnd* pParentWnd, bool* pbEnable)
{
	CRect rc( 0, 0, 0, 0 );

	const DWORD dwStyleEx = WS_EX_TOPMOST | ( Settings.General.LanguageRTL ? WS_EX_LAYOUTRTL : 0 );
	if ( ! CWnd::CreateEx( dwStyleEx, m_hClass, NULL, WS_POPUP|WS_DISABLED, rc, pParentWnd, 0, NULL ) )
		return FALSE;

	SetOwner( pParentWnd );
	m_pbEnable = pbEnable;

	return TRUE;
}

void CCoolTipCtrl::Hide()
{
	m_tOpen = 0;

	if ( m_bVisible )
	{
		OnHide();

		ShowWindow( SW_HIDE );
		ModifyStyleEx( WS_EX_LAYERED, 0 );
		m_bVisible = FALSE;
		GetCursorPos( &m_pOpen );
	}

	if ( m_bTimer )
	{
		KillTimer( 1 );
		KillTimer( 2 );
		m_bTimer = FALSE;
	}
}

void CCoolTipCtrl::ShowImpl(bool bChanged)
{
	if ( m_pbEnable != NULL && *m_pbEnable == false )
		return;

	CPoint point;
	GetCursorPos( &point );
	if ( ! WindowFromPointBelongsToOwner( point ) )
		return;

	if ( m_bVisible )
	{
		if ( ! bChanged )
			return;

		Hide();
	}
	else if ( point != m_pOpen )
	{
		m_pOpen = point;
		m_tOpen = GetTickCount() + Settings.Interface.TipDelay;

		if ( ! m_bTimer )
		{
			SetTimer( 1, Settings.Interface.RefreshRateGraph, NULL );
			SetTimer( 2, TIP_TIMER_ASYNC, NULL );
			m_bTimer = TRUE;
		}
		return;
	}

	if ( m_bVisible )
		return;

	m_sz.cx = m_sz.cy = 0;

	if ( ! OnPrepare() )
		return;

	CRect rc( m_pOpen.x + TIP_OFFSET_X, m_pOpen.y + TIP_OFFSET_Y, 0, 0 );
	rc.right = rc.left + m_sz.cx + TIP_MARGIN * 2;
	rc.bottom = rc.top + m_sz.cy + TIP_MARGIN * 2;

	HMONITOR hMonitor = MonitorFromPoint( m_pOpen, MONITOR_DEFAULTTONEAREST );

	MONITORINFO oMonitor = {0};
	oMonitor.cbSize = sizeof( MONITORINFO );
	GetMonitorInfo( hMonitor, &oMonitor );

	if ( rc.right >= oMonitor.rcWork.right )
		rc.OffsetRect( oMonitor.rcWork.right - rc.right - 4, 0 );

	if ( rc.bottom >= oMonitor.rcWork.bottom )
		rc.OffsetRect( 0, - ( m_sz.cy + TIP_MARGIN * 2 + TIP_OFFSET_Y + 4 ) );

	m_bVisible = TRUE;

	OnShow();

	if ( Settings.Interface.TipAlpha == 255 )
	{
		ModifyStyleEx( WS_EX_LAYERED, 0 );
	}
	else
	{
		ModifyStyleEx( 0, WS_EX_LAYERED );
		SetLayeredWindowAttributes( 0, (BYTE)Settings.Interface.TipAlpha, LWA_ALPHA );
	}

	SetWindowPos( &wndTop, rc.left, rc.top, rc.Width(), rc.Height(),
		SWP_ASYNCWINDOWPOS|SWP_SHOWWINDOW|SWP_NOACTIVATE );
	UpdateWindow();

	if ( ! m_bTimer )
	{
		SetTimer( 1, Settings.Interface.RefreshRateGraph, NULL );
		SetTimer( 2, TIP_TIMER_ASYNC, NULL );
		m_bTimer = TRUE;
	}
}

void CCoolTipCtrl::CalcSizeHelper()
{
	CClientDC dc( this );

	m_sz.cx = m_sz.cy = 0;

	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntBold );

	OnCalcSize( &dc );

	dc.SelectObject( pOldFont );
}

void CCoolTipCtrl::AddSize(CDC* pDC, LPCTSTR pszText, int nBase)
{
	m_sz.cx = max( m_sz.cx, (LONG)GetSize( pDC, pszText ) + nBase );
}

int CCoolTipCtrl::GetSize(CDC* pDC, LPCTSTR pszText) const
{
	CRect rcText( 0, 0, 0, 0 );
	const DWORD dwFlags = DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX |
		( Settings.General.LanguageRTL ? DT_RTLREADING : 0 );
	pDC->DrawText( pszText, -1, &rcText, dwFlags );
	return rcText.Width();
}

void CCoolTipCtrl::GetPaintRect(RECT* pRect)
{
	pRect->top = 0;
	pRect->left = 0;
	pRect->right = m_sz.cx;
	pRect->bottom = m_sz.cy;
}

void CCoolTipCtrl::DrawText(CDC* pDC, POINT* pPoint, LPCTSTR pszText, int nBase)
{
	POINT pt = { pPoint->x + nBase, pPoint->y };
	DrawText( pDC, &pt, pszText );
}

void CCoolTipCtrl::DrawText(CDC* pDC, POINT* pPoint, LPCTSTR pszText, SIZE* pTextMaxSize)
{
	CRect rcText( 0, 0, 0, 0 );
	const DWORD dwFlags = DT_SINGLELINE | DT_NOPREFIX |
		( Settings.General.LanguageRTL ? DT_RTLREADING : 0 );
	pDC->DrawText( pszText, -1, &rcText, dwFlags | DT_CALCRECT );
	if ( pTextMaxSize )
	{
		if ( pTextMaxSize->cx > 0 && pTextMaxSize->cx < rcText.Width() )
			rcText.right = rcText.left + pTextMaxSize->cx;
		if ( pTextMaxSize->cy > 0 && pTextMaxSize->cy < rcText.Height() )
			rcText.bottom = rcText.top + pTextMaxSize->cy;
	}
	rcText.MoveToXY( pPoint->x, pPoint->y );
	if ( ! Images.m_bmToolTip.m_hObject )
		pDC->FillSolidRect( &rcText, Colors.m_crTipBack );
	pDC->DrawText( pszText, -1, &rcText, dwFlags | DT_END_ELLIPSIS );
	if ( ! Images.m_bmToolTip.m_hObject )
		pDC->ExcludeClipRect( &rcText );
}

void CCoolTipCtrl::DrawRule(CDC* pDC, POINT* pPoint, BOOL bPos)
{
	pPoint->y += 5;
	if ( bPos )
	{
		pDC->Draw3dRect( pPoint->x, pPoint->y,
			m_sz.cx + ( TIP_MARGIN - 3 ) - pPoint->x, 1, Colors.m_crTipBorder, Colors.m_crTipBorder );
		pDC->ExcludeClipRect( pPoint->x, pPoint->y,
			m_sz.cx + ( TIP_MARGIN - 3 ), pPoint->y + 1 );
	}
	else
	{
		pDC->Draw3dRect( -( TIP_MARGIN - 3 ), pPoint->y,
			m_sz.cx + ( TIP_MARGIN - 3 ) * 2, 1, Colors.m_crTipBorder, Colors.m_crTipBorder );
		pDC->ExcludeClipRect( -( TIP_MARGIN - 3 ), pPoint->y,
			m_sz.cx + ( TIP_MARGIN - 3 ), pPoint->y + 1 );
	}
	pPoint->y += 6;
}

BOOL CCoolTipCtrl::WindowFromPointBelongsToOwner(const CPoint& point)
{
	CWnd* pOwner = GetOwner();
	if ( ! pOwner || ! IsWindow( pOwner->GetSafeHwnd() ) )
		return FALSE;

	CRect rc;
	pOwner->GetWindowRect( &rc );

	if ( ! rc.PtInRect( point ) )
		return FALSE;

	CWnd* pWnd = WindowFromPoint( point );

	while ( pWnd )
	{
		if ( pWnd == pOwner ) return TRUE;
		if ( m_hAltWnd != NULL && pWnd->GetSafeHwnd() == m_hAltWnd ) return TRUE;
		if ( ! IsWindow( pWnd->m_hWnd ) ) return FALSE;
		pWnd = pWnd->GetParent();
	}

	return FALSE;
}

CLineGraph* CCoolTipCtrl::CreateLineGraph()
{
	CLineGraph* pGraph = new CLineGraph();

	pGraph->m_bShowLegend	= FALSE;
	pGraph->m_bShowAxis		= FALSE;
	pGraph->m_nMinGridVert	= 16;

	pGraph->m_crBack = Colors.m_crTipGraph;
	pGraph->m_crGrid = Colors.m_crTipGraphGrid;

	return pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CCoolTipCtrl message handlers

int CCoolTipCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	m_bTimer = FALSE;
	return 0;
}

void CCoolTipCtrl::OnDestroy()
{
	if ( m_bTimer )
	{
		KillTimer( 1 );
		KillTimer( 2 );
		m_bTimer = FALSE;
	}
	if ( m_bVisible ) Hide();
	CWnd::OnDestroy();
}

BOOL CCoolTipCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CCoolTipCtrl::OnPaint()
{
	if ( ! IsWindow( GetSafeHwnd() ) || ! IsWindowVisible() ) return;

	CPaintDC dc( this );

	CRect rc;
	GetClientRect( &rc );

	// Obsolete: Unbuffered solid color default
//	if ( ! Images.m_bmToolTip.m_hObject )
//	{
//		CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntBold );
//
//		dc.Draw3dRect( &rc, Colors.m_crTipBorder, Colors.m_crTipBorder );
//		rc.DeflateRect( 1, 1 );
//
//		dc.SetViewportOrg( TIP_MARGIN, TIP_MARGIN );
//		dc.SetTextColor( Colors.m_crTipText );
//		dc.SetBkMode( TRANSPARENT );
//		OnPaint( &dc );
//		dc.SetViewportOrg( 0, 0 );
//		dc.FillSolidRect( &rc, Colors.m_crTipBack );
//		dc.SelectObject( pOldFont );
//		return;
//	}

	// Flicker-free:

	CSize size = rc.Size();
	CDC* pMemDC = CoolInterface.GetBuffer( dc, size );
//	if ( Settings.General.LanguageRTL )
//		SetLayout( pMemDC->m_hDC, 0 );

	CFont* pOldFont = (CFont*)pMemDC->SelectObject( &CoolInterface.m_fntBold );

	pMemDC->Draw3dRect( &rc, Colors.m_crTipBorder, Colors.m_crTipBorder );
	rc.DeflateRect( 1, 1 );

	if ( Images.m_bmToolTip.m_hObject )	// (System.ToolTip)
	{
		CoolInterface.DrawWatermark( pMemDC, &rc, &Images.m_bmToolTip, FALSE );
		pMemDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		pMemDC->FillSolidRect( &rc, Colors.m_crTipBack );
		pMemDC->SetBkColor( Colors.m_crTipBack );
		pMemDC->SetBkMode( OPAQUE );
	}

	pMemDC->SetTextColor( Colors.m_crTipText );
	pMemDC->SetViewportOrg( TIP_MARGIN, TIP_MARGIN );
	OnPaint( pMemDC );
	pMemDC->SetViewportOrg( 0, 0 );
	pMemDC->SelectObject( pOldFont );

	rc.InflateRect( 1, 1 );
	dc.BitBlt( 0, 0, rc.Width(), rc.Height(), pMemDC, 0, 0, SRCCOPY );

//	if ( Settings.General.LanguageRTL )
//		SetLayout( pMemDC->m_hDC, LAYOUT_RTL );
}

void CCoolTipCtrl::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
{
	Hide();
}

void CCoolTipCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Hide();
	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CCoolTipCtrl::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CPoint point;
	GetCursorPos( &point );

	if ( ! WindowFromPointBelongsToOwner( point ) )
	{
		if ( m_bVisible )
			Hide();
		return;
	}

	if ( ! m_bVisible && m_tOpen && GetTickCount() >= m_tOpen )
	{
		m_tOpen = 0;
		if ( point == m_pOpen || m_hAltWnd != NULL )
			ShowImpl();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCoolTipCtrl events

BOOL CCoolTipCtrl::OnPrepare()
{
	return FALSE;
}

void CCoolTipCtrl::OnCalcSize(CDC* /*pDC*/)
{
}

void CCoolTipCtrl::OnShow()
{
}

void CCoolTipCtrl::OnHide()
{
}

void CCoolTipCtrl::OnPaint(CDC* /*pDC*/)
{
}
