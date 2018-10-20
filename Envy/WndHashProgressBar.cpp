//
// WndHashProgressBar.cpp
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
#include "WndHashProgressBar.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Colors.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CHashProgressBar, CWnd)

BEGIN_MESSAGE_MAP(CHashProgressBar, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

#define WINDOW_WIDTH		310
#define WINDOW_HEIGHT		46


/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar construction

CHashProgressBar::CHashProgressBar()
	: m_nFlash	( 0 )
//	, m_nAlpha	( 0 )
{
}

/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar operations

void CHashProgressBar::Run()
{
	if ( ! Settings.Library.HashWindow || ! LibraryBuilder.GetRemaining() || IsUserFullscreen() )
	{
		// No display
		if ( m_hWnd )
			DestroyWindow();
		m_sCurrent.Empty();
		m_sPrevious.Empty();
		return;
	}

	CString strCurrent = LibraryBuilder.GetCurrent();
	if ( ! strCurrent.IsEmpty() )
		m_sCurrent = PathFindFileName( strCurrent );

	if ( m_hWnd == NULL )
	{
		try
		{
			CreateEx( WS_EX_TOPMOST | WS_EX_TOOLWINDOW, AfxRegisterWndClass( CS_SAVEBITS | ( Settings.Interface.TipShadow ? CS_DROPSHADOW : 0 ) ),
				L"Envy Hashing...", WS_POPUP, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0 );
		}
		catch ( CResourceException* pEx )
		{
			pEx->Delete();
		}
	}

	if ( m_hWnd )
		Update();
}

void CHashProgressBar::Update()
{
	if ( ! m_sCurrent.IsEmpty() && m_sCurrent != m_sPrevious )
	{
		m_sPrevious = m_sCurrent;

		CClientDC dc( this );
		CFont* pOld = (CFont*)dc.SelectObject( &CoolInterface.m_fntCaption );
		CSize sz = dc.GetTextExtent( m_sCurrent );
		dc.SelectObject( pOld );

		int nWidth = 4 + 32 + sz.cx + 3;
		nWidth = max( nWidth, WINDOW_WIDTH );
		nWidth = min( nWidth, GetSystemMetrics( SM_CXSCREEN ) / 2 );
		Show( nWidth, FALSE );

		Invalidate( FALSE );
	}
}

void CHashProgressBar::Show(int nWidth, BOOL /*bShow*/)
{
	CRect rc;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 );
	rc.left = rc.right - nWidth - 4;
	rc.top  = rc.bottom - WINDOW_HEIGHT - 4;
	SetWindowPos( &wndTopMost, rc.left, rc.top, nWidth, WINDOW_HEIGHT, SWP_SHOWWINDOW | SWP_NOACTIVATE );

//	SetTimer( 2, 5, NULL );		// ToDo: Rapid fade-in
}

/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar message handlers

int CHashProgressBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	OnSkinChange();

	m_nFlash = 0;
	SetTimer( 1, 50, NULL );

	return 0;
}

void CHashProgressBar::OnDestroy()
{
	KillTimer( 1 );

//	m_nAlpha = 0;

	CWnd::OnDestroy();
}

void CHashProgressBar::OnSkinChange()
{
	HBITMAP hBitmap = Skin.GetWatermark( L"CHashProgressBar" );
	if ( m_bmImage.m_hObject ) m_bmImage.DeleteObject();
	if ( hBitmap != NULL ) m_bmImage.Attach( hBitmap );
}

void CHashProgressBar::OnPaint()
{
//	CPaintDC dcClient( this );
//
//	Draw( &dcClient );
//}

//void CHashProgressBar::Draw(CDC* pDC)
//{
	CRect rcClient;
	GetClientRect( &rcClient );

	//CDC dc;
	//dc.CreateCompatibleDC( pDC );
	CPaintDC dc( this );

	dc.Draw3dRect( &rcClient, Colors.m_crTipBorder, Colors.m_crTipBorder );
	rcClient.DeflateRect( 1, 1 );

	if ( ! CoolInterface.DrawWatermark( &dc, &rcClient, &m_bmImage ) )
		dc.FillSolidRect( &rcClient, Colors.m_crTipBack );

	dc.SetBkMode( TRANSPARENT );

	// Icon
	if ( ! ShellIcons.Draw( &dc, ShellIcons.Get( m_sCurrent, 32 ), 32,
			rcClient.left + 5, rcClient.top + 4, ( m_bmImage.m_hObject ? CLR_NONE : Colors.m_crTipBack ) ) )
	{
		HICON hIcon = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_SEARCH_FOLDER), IMAGE_ICON, 32, 32, 0 );
		DrawIconEx( dc, rcClient.left + 5, rcClient.top + 4, hIcon, 32, 32, 0, NULL, DI_NORMAL );
	}

	// Text
	CFont* pOld = dc.SelectObject( &CoolInterface.m_fntNormal );

	CString strText = L"x";
	CRect rcX( rcClient.right - 20, rcClient.top + 1,
		rcClient.right - 5, rcClient.top + 20 );
	dc.SetTextColor( Colors.m_crTipText );
	dc.DrawText( strText, rcX, DT_RIGHT | DT_SINGLELINE );

	//if ( m_nFlash++ % 30 > 15 )
	//	dc.SetTextColor( Colors.m_crTextStatus );

	strText.Format( LoadString( IDS_HASH_MESSAGE ), LibraryBuilder.GetRemaining() );

	CSize sz = dc.GetTextExtent( strText );
	CRect rcText( rcClient.left + 32 + 12, rcClient.top + 3,
		rcClient.right - 6, rcClient.top + 3 + sz.cy );
	dc.DrawText( strText, rcText, DT_LEFT | DT_SINGLELINE );

	//dc.SelectObject( &CoolInterface.m_fntCaption );
	rcText.top = rcText.bottom + 4;
	rcText.bottom = rcClient.bottom - 10;
	dc.DrawText( m_sCurrent, rcText, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS );

	dc.SelectObject( pOld );

	// Progress bar
	CRect rcProgress = rcClient;
	rcProgress.DeflateRect( 3, 40, 2, 2 );
	rcProgress.top = rcProgress.bottom - 2;
	float nPercentage = LibraryBuilder.GetProgress() / 100;
	if ( nPercentage < 0 || nPercentage > 1 ) nPercentage = 1;
	rcProgress.right = rcProgress.left + (INT)( rcProgress.Width() * nPercentage );

	dc.Draw3dRect( &rcProgress, Colors.m_crFragmentPass, Colors.m_crFragmentPass );
	rcProgress.top--;
	dc.Draw3dRect( &rcProgress, Colors.m_crFragmentPass, Colors.m_crTipText );

	//pDC->BitBlt( rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), &dc, 0, 0, SRCCOPY );
}

BOOL CHashProgressBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHashProgressBar::OnTimer(UINT_PTR /*nIDEvent*/)
{
	//if ( nIDEvent == 2 )	// Fade In
	//{
	//	m_nAlpha += 2;
	//	if ( m_nAlpha >= Settings.Interface.TipAlpha )
	//	{
	//		m_nAlpha = Settings.Interface.TipAlpha;
	//		KillTimer( 2 );
	//	}
	//
	//	SetLayeredWindowAttributes( NULL, m_nAlpha, LWA_ALPHA );
	//
	//	return;
	//}

	//if ( nIDEvent == 3 )	// Fade Out
	//{
	//	m_nAlpha -= 2;
	//	if ( m_nAlpha < 5 )
	//	{
	//		m_nAlpha = 0;
	//		KillTimer( 3 );
	//	}
	//
	//	SetLayeredWindowAttributes( NULL, m_nAlpha, LWA_ALPHA );
	//
	//	return;
	//}

	CRect rcClient;
	GetClientRect( &rcClient );
	if ( m_nFlash % 15 == 1 )	// Cycle text 3x per 2 seconds (Ocassionally update text)
		rcClient.DeflateRect( 40, 3, 2, 2 );
	else						// Update only progress bar 20x per second
		rcClient.DeflateRect( 4, 40, 2, 2 );
	InvalidateRect( rcClient, FALSE );
}

void CHashProgressBar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	if ( point.y < 14 && point.x > ( WINDOW_WIDTH - 14 ) )	// "x"
		Settings.Library.HashWindow = false;

	ShowWindow( SW_HIDE );
	KillTimer( 1 );
}
