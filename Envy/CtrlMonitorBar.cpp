//
// CtrlMonitorBar.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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
#include "CtrlMonitorBar.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Skin.h"
#include "GraphItem.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//#define MONITORBARWIDTH 120	// Settings.Skin.MonitorbarWidth


BEGIN_MESSAGE_MAP(CMonitorBarCtrl, CControlBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl construction

CMonitorBarCtrl::CMonitorBarCtrl()
	: m_nMaximumIn	( 0 )
	, m_nMaximumOut	( 0 )
	, m_bTabIn		( FALSE )
	, m_bTabOut		( FALSE )
	, m_hTabIn		( NULL )
	, m_hTabOut		( NULL )
	, m_hUpDown		( NULL )
	, m_rcTrackIn	( 0, 0, 0, 0 )
	, m_rcTrackOut	( 0, 0, 0, 0 )
{
	m_pRxItem		= new CGraphItem( GRC_TOTAL_BANDWIDTH_IN, 1.0f, Colors.m_crMonitorDownloadBar );
	m_pTxItem		= new CGraphItem( GRC_TOTAL_BANDWIDTH_OUT, 1.0f, Colors.m_crMonitorUploadBar );
	m_pSnapBar[0]	= NULL;
	m_pSnapBar[1]	= NULL;
}

CMonitorBarCtrl::~CMonitorBarCtrl()
{
	delete m_pRxItem;
	delete m_pTxItem;
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl system message handlers

BOOL CMonitorBarCtrl::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc( 0, 0, 0, 0 );
	dwStyle |= WS_CHILD|WS_CLIPCHILDREN;
	return CWnd::Create( NULL, NULL, dwStyle, rc, pParentWnd, nID, NULL );
}

int CMonitorBarCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( Settings.Skin.MenuBorders )
		m_dwStyle |= CBRS_BORDER_3D;	// CBRS_TOOLTIPS in WndMain

	if ( lpCreateStruct->dwExStyle & WS_EX_LAYOUTRTL )
	{
		lpCreateStruct->dwExStyle ^= WS_EX_LAYOUTRTL;
		SetWindowLongPtr( this->m_hWnd, GWL_EXSTYLE, lpCreateStruct->dwExStyle );
	}

	OnSkinChange();

	SetTimer( 1, 120, NULL );

	return 0;
}

void CMonitorBarCtrl::OnDestroy()
{
	KillTimer( 1 );

	if ( m_hTabIn )  DestroyIcon( m_hTabIn );
	if ( m_hTabOut ) DestroyIcon( m_hTabOut );
	if ( m_hUpDown ) DestroyIcon( m_hUpDown );

	CControlBar::OnDestroy();
}

INT_PTR CMonitorBarCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	if ( ! pTI )
		return CWnd::OnToolHitTest( point, pTI );

	CString strTip;
	if ( m_rcTrackIn.PtInRect( point ) )
	{
		LoadString( strTip, IDS_MONITORBAR_LIMIT_IN );
		pTI->rect = m_rcTrackIn;
	}
	else if ( m_rcTrackOut.PtInRect( point ) )
	{
		LoadString( strTip, IDS_MONITORBAR_LIMIT_OUT );
		pTI->rect = m_rcTrackOut;
	}
	else
	{
		return -1;
	}

	pTI->lpszText	= _tcsdup( strTip );
	pTI->uFlags		= TTF_NOTBUTTON;
	pTI->hwnd		= GetSafeHwnd();
	pTI->uId		= NULL;

	return pTI->uId;
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl layout message handlers

CSize CMonitorBarCtrl::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	const int nHeight = Settings.General.GUIMode == GUI_WINDOWED ? 30 : 38;
	CSize size( Settings.Skin.MonitorbarWidth, nHeight );

	for ( int nSnap = 1 ; nSnap >= 0 ; nSnap-- )
	{
		if ( m_pSnapBar[ nSnap ] != NULL && m_pSnapBar[ nSnap ]->IsVisible() )
		{
			size.cy = m_pSnapBar[ nSnap ]->CalcFixedLayout( FALSE, TRUE ).cy;
			break;
		}
	}

	return size;
}

void CMonitorBarCtrl::OnSize(UINT nType, int cx, int cy)
{
	CControlBar::OnSize( nType, cx, cy );
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl timer

void CMonitorBarCtrl::OnTimer(UINT_PTR /*nIDEvent*/)
{
	m_pTxItem->Update();
	m_pRxItem->Update();

	m_nMaximumIn  = max( m_pRxItem->GetMaximum(), Settings.Connection.InSpeed * 1000 );
	m_nMaximumOut = max( m_pTxItem->GetMaximum(), Settings.Connection.OutSpeed * 1000 );

	if ( IsWindowVisible() )
		Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl display

void CMonitorBarCtrl::OnSkinChange()
{
	if ( m_bmWatermark.m_hObject ) m_bmWatermark.DeleteObject();
	if ( HBITMAP hWatermark = Skin.GetWatermark( L"CMonitorBar" ) )
		m_bmWatermark.Attach( hWatermark );

	if ( m_hTabIn )  DestroyIcon( m_hTabIn );
	if ( m_hTabOut ) DestroyIcon( m_hTabOut );
	if ( m_hUpDown ) DestroyIcon( m_hUpDown );
	m_hTabIn  = CoolInterface.ExtractIcon( IDI_POINTER_ARROW_IN, false );
	m_hTabOut = CoolInterface.ExtractIcon( IDI_POINTER_ARROW_OUT, false );
	m_hUpDown = CoolInterface.ExtractIcon( IDI_UPDOWN_ARROW, false );

	m_pRxItem->m_nColor = Colors.m_crMonitorDownloadBar;
	m_pTxItem->m_nColor = Colors.m_crMonitorUploadBar;

	if ( m_hWnd != NULL && IsWindowVisible() )
	{
		CalcFixedLayout( FALSE, FALSE );
		Invalidate();
	}
}

void CMonitorBarCtrl::DoPaint(CDC* pDC)
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CSize size = rcClient.Size();
	CDC* pMemDC = CoolInterface.GetBuffer( *pDC, size );
	if ( Settings.General.LanguageRTL )
		SetLayout( pMemDC->m_hDC, 0 );

	if ( ! CoolInterface.DrawWatermark( pMemDC, &rcClient, &m_bmWatermark ) )
		pMemDC->FillSolidRect( &rcClient, Colors.m_crMidtone );

	if ( Settings.Skin.MenuBorders )
		DrawBorders( pMemDC, rcClient );
	else
		rcClient.DeflateRect( 2, 3, 2, 1 );

	for ( int nY = rcClient.top + 4 ; nY < rcClient.bottom - 4 ; nY += 2 )
	{
		pMemDC->Draw3dRect( rcClient.left + 3, nY, 4, 1,
			Colors.m_crDisabled, Colors.m_crDisabled );
	}

	m_pTxItem->SetHistory( rcClient.Width(), TRUE );
	m_pRxItem->SetHistory( rcClient.Width(), TRUE );

	CRect rcHistory( rcClient.left + 10, rcClient.top + 2, rcClient.right - 25, rcClient.bottom - 5 );
	PaintHistory( pMemDC, &rcHistory );

	CRect rcCurrent( rcClient.right - 13, rcClient.top + 2, rcClient.right - 7, rcClient.bottom - 5 );
	PaintCurrent( pMemDC, &rcCurrent, m_pTxItem, m_nMaximumOut );	// Upload Graph Right
	rcCurrent.OffsetRect( -7, 0 );
	PaintCurrent( pMemDC, &rcCurrent, m_pRxItem, m_nMaximumIn );	// Download Graph Left

	DrawIconEx( pMemDC->GetSafeHdc(), rcClient.right - 21, rcClient.bottom - 15, m_hUpDown, 16, 16, 0, NULL, DI_NORMAL );

	m_rcTrackIn.SetRect( rcClient.right - 25, rcClient.top + 1, rcClient.right - 15, rcClient.bottom - 5 );
	m_rcTrackOut.SetRect( rcClient.right - 12, rcClient.top + 1, rcClient.right - 2, rcClient.bottom - 5 );
	PaintTabs( pMemDC );

	GetClientRect( &rcClient );
	pDC->BitBlt( rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), pMemDC, 0, 0, SRCCOPY );
	if ( Settings.General.LanguageRTL )
		SetLayout( pMemDC->m_hDC, LAYOUT_RTL );
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl painting components

void CMonitorBarCtrl::PaintHistory(CDC* pDC, CRect* prc)
{
	CRect rc( prc );

	pDC->Draw3dRect( &rc, Colors.m_crSys3DShadow, Colors.m_crSys3DHighlight );
	rc.DeflateRect( 1, 1 );
	pDC->FillSolidRect( &rc, ( ( m_bTabIn && Settings.Live.BandwidthScaleIn > 100 ) || ( m_bTabOut && Settings.Live.BandwidthScaleOut > 100 ) ) ?
		Colors.m_crMonitorHistoryBackMax : Colors.m_crMonitorHistoryBack );

	if ( m_bTabIn || m_bTabOut )
	{
		CString str;

		CFont* pfOld = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );
		pDC->SetBkMode( TRANSPARENT );
		pDC->SetTextColor( Colors.m_crMonitorHistoryText );

		if ( ( m_bTabIn && Settings.Live.BandwidthScaleIn > 100 ) ||
			( m_bTabOut && Settings.Live.BandwidthScaleOut > 100 ) )
		{
			str = L"MAX";
			pDC->DrawText( str, &rc, DT_SINGLELINE|DT_VCENTER|DT_CENTER );
		}
		else
		{
			DWORD nRate = m_bTabOut ? Settings.Connection.OutSpeed : Settings.Connection.InSpeed;
			nRate = nRate * ( m_bTabOut ? Settings.Live.BandwidthScaleOut : Settings.Live.BandwidthScaleIn ) / 100;
			str.Format( L" %u%%  %s",
				m_bTabOut ? Settings.Live.BandwidthScaleOut : Settings.Live.BandwidthScaleIn,
				(LPCTSTR)Settings.SmartSpeed( nRate, Kilobits ) );

			// Center text if possible
			CRect rcText( 0, 0, 0, 0 );
			pDC->DrawText( str, &rcText, DT_CALCRECT|DT_VCENTER );
			pDC->DrawText( str, &rc, DT_SINGLELINE|DT_VCENTER|(rcText.Width() < rc.Width() ? DT_CENTER : DT_LEFT) );
		}

		pDC->SelectObject( pfOld );
		return;
	}

	if ( m_nMaximumIn < 1000 && m_nMaximumOut < 1000 ) return;	// Initial 0

	const DWORD nMax = min( m_pTxItem->m_nLength, (DWORD)rc.Width() );
	int nX = rc.right - 1;

	for ( DWORD nPos = 0 ; nPos < nMax ; nPos++, nX-- )
	{
		DWORD nTxValue = m_pTxItem->GetValueAt( nPos );
		DWORD nRxValue = m_pRxItem->GetValueAt( nPos );

		nTxValue = rc.bottom - nTxValue * rc.Height() / m_nMaximumOut;
		nRxValue = rc.bottom - nRxValue * rc.Height() / m_nMaximumIn;

		if ( nTxValue < nRxValue )
		{
			if ( nTxValue < (DWORD)rc.bottom )
			{
				pDC->FillSolidRect( nX, nTxValue + 1, 1, rc.bottom - nTxValue - 1, Colors.m_crMonitorUploadBar );
				pDC->SetPixel( nX, nTxValue, Colors.m_crMonitorUploadLine );
			}
			if ( nRxValue < (DWORD)rc.bottom )
			{
				pDC->FillSolidRect( nX, nRxValue + 1, 1, rc.bottom - nRxValue - 1, Colors.m_crMonitorDownloadBar );
				pDC->SetPixel( nX, nRxValue, Colors.m_crMonitorDownloadLine );
			}
		}
		else
		{
			if ( nRxValue < (DWORD)rc.bottom )
			{
				pDC->FillSolidRect( nX, nRxValue + 1, 1, rc.bottom - nRxValue - 1, Colors.m_crMonitorDownloadBar );
				pDC->SetPixel( nX, nRxValue, Colors.m_crMonitorDownloadLine );
			}
			if ( nTxValue < (DWORD)rc.bottom )
			{
				pDC->FillSolidRect( nX, nTxValue + 1, 1, rc.bottom - nTxValue - 1, Colors.m_crMonitorUploadBar );
				pDC->SetPixel( nX, nTxValue, Colors.m_crMonitorUploadLine );
			}
		}
	}
}

void CMonitorBarCtrl::PaintCurrent(CDC* pDC, CRect* prc, CGraphItem* pItem, DWORD nMaximum)
{
	CRect rc( prc );

	pDC->Draw3dRect( &rc, Colors.m_crSys3DShadow, Colors.m_crSys3DHighlight );
	rc.DeflateRect( 1, 1 );
	pDC->FillSolidRect( &rc, ( Settings.Live.BandwidthScaleIn > 100 && Settings.Live.BandwidthScaleOut > 100 ) ?
		Colors.m_crMonitorHistoryBackMax : Colors.m_crMonitorHistoryBack );

	if ( nMaximum == 0 || pItem->m_nLength < 1 ) return;

	DWORD nValue = (DWORD)pItem->GetValue( pItem->m_nCode );
	nValue = nValue * rc.Height() / nMaximum ;
	pDC->FillSolidRect( rc.left, rc.bottom - nValue, rc.Width(), nValue, pItem->m_nColor );

	// Menu Icon Hover Bug Workaround  (ToDo: Also Fix in Proper Place)
	pDC->SetBkColor( Colors.m_crMidtone );
}

void CMonitorBarCtrl::PaintTabs(CDC* pDC)
{
// Obsolete:
//	float nPosition = Settings.Live.BandwidthScale > 100 ? 1.0f : (float)Settings.Live.BandwidthScale / 110.0f;

	if ( Settings.Live.BandwidthScaleIn > 100 )
		m_rcTabIn.top	= m_rcTrackIn.top - 8;
	else if ( Settings.Live.BandwidthScaleIn > 98 )
		m_rcTabIn.top	= m_rcTrackIn.top - 6;
	else if ( Settings.Live.BandwidthScaleIn < 2 )
		m_rcTabIn.top	= m_rcTrackIn.bottom - 7;
	else
		m_rcTabIn.top	= m_rcTrackIn.bottom - (int)( (float)Settings.Live.BandwidthScaleIn / 100.00f * (float)m_rcTrackIn.Height() ) - 7;
	m_rcTabIn.bottom	= m_rcTabIn.top + 16;
	m_rcTabIn.left		= m_rcTrackIn.right - 14;
	m_rcTabIn.right		= m_rcTabIn.left + 16;

	DrawIconEx( pDC->GetSafeHdc(), m_rcTabIn.left, m_rcTabIn.top, m_hTabIn, 16, 16, 0, NULL, DI_NORMAL );

	if ( Settings.Live.BandwidthScaleOut > 100 )
		m_rcTabOut.top	= m_rcTrackOut.top - 8;
	else if ( Settings.Live.BandwidthScaleOut > 98 )
		m_rcTabOut.top	= m_rcTrackOut.top - 6;
	else if ( Settings.Live.BandwidthScaleOut < 2 )
		m_rcTabOut.top	= m_rcTrackOut.bottom - 7;
	else
		m_rcTabOut.top	= m_rcTrackOut.bottom - (int)( (float)Settings.Live.BandwidthScaleOut / 100.00f * (float)m_rcTrackOut.Height() ) - 7;
	m_rcTabOut.bottom	= m_rcTabOut.top + 16;
	m_rcTabOut.left 	= m_rcTrackOut.left - 2;
	m_rcTabOut.right	= m_rcTabOut.left + 16;

	DrawIconEx( pDC->GetSafeHdc(), m_rcTabOut.left, m_rcTabOut.top, m_hTabOut, 16, 16, 0, NULL, DI_NORMAL );
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl tracking

BOOL CMonitorBarCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	if ( m_rcTrackIn.PtInRect( point ) || m_rcTrackOut.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	return CControlBar::OnSetCursor( pWnd, nHitTest, message );
}

void CMonitorBarCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcTrack;

	if ( m_rcTrackIn.PtInRect( point ) )
	{
		m_bTabIn = TRUE;
		rcTrack = m_rcTrackIn;
	}
	else if ( m_rcTrackOut.PtInRect( point ) )
	{
		m_bTabOut = TRUE;
		rcTrack = m_rcTrackOut;
	}
	else
	{
		if ( GetDC()->m_hDC )	// Strange rare crashfix
			CControlBar::OnLButtonDown( nFlags, point );
		return;
	}

	MSG* pMsg = &AfxGetThreadState()->m_msgCur;

	ClientToScreen( &rcTrack );
	ClipCursor( &rcTrack );
	ScreenToClient( &rcTrack );

	Invalidate();

	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );

		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}

		if ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 )
			break;	// Simultaneous Right-click trap

		if ( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 )
			break;	// Escape other traps?

		CPoint pt;
		GetCursorPos( &pt );
		ScreenToClient( &pt );

		DWORD nScale;
		if ( pt.y <= rcTrack.top )
			nScale = 101;
		else if ( pt.y <= rcTrack.top + 2 )
			nScale = 100;
		//else if ( pt.y >= rcTrack.bottom - 1 )
		//	nScale = 0; 	// Disable?
		else
			nScale = (DWORD)( 100.0f * (float)( rcTrack.bottom - pt.y ) / (float)( rcTrack.Height() - 2 ) );

		if ( m_bTabIn && nScale != Settings.Live.BandwidthScaleIn )
			Settings.Live.BandwidthScaleIn = nScale;
		else if ( m_bTabOut && nScale != Settings.Live.BandwidthScaleOut )
			Settings.Live.BandwidthScaleOut = nScale;
		else
			continue;

		Invalidate();
	}

	m_bTabIn = m_bTabOut = FALSE;

	ReleaseCapture();
	ClipCursor( NULL );

	Invalidate();
}
