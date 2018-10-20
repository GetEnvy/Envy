//
// WizardSheet.cpp
//
// This file is part of Torrent Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008-2014
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "TorrentEnvy.h"
#include "WizardSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PAGE_COLOR		RGB( 255, 255, 255 )	// White areas
//#define BANNER_SIZE	50	// m_nBannerHeight
//#define MENU_SIZE		48	// m_nMenuHeight


/////////////////////////////////////////////////////////////////////////////
// CWizardSheet

IMPLEMENT_DYNAMIC(CWizardSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CWizardSheet, CPropertySheet)
	ON_WM_PAINT()
	ON_WM_SIZE()
//	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_NCLBUTTONUP()
	ON_WM_XBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardSheet construction

CWizardSheet::CWizardSheet(CWnd *pParentWnd, UINT iSelectPage)
{
	BITMAP bm;
	m_bmHeader.LoadBitmap( IDB_BANNER );
	m_bmHeader.GetBitmap( &bm );
	m_bmHeader.SetBitmapDimension( bm.bmWidth, bm.bmHeight );
	m_nBannerHeight = bm.bmHeight;

	m_nMenuHeight = MulDiv( 46, GetDeviceCaps( *GetDesktopWindow()->GetDC(), LOGPIXELSY ), 96 );	// DPI-aware

	m_psh.dwFlags &= ~PSP_HASHELP;
	Construct( L"", pParentWnd, iSelectPage );
	SetWizardMode();
}

//CWizardSheet::~CWizardSheet()
//{
//}

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet page lookup

CWizardPage* CWizardSheet::GetPage(CRuntimeClass* pClass)
{
	const int nCount = GetPageCount();
	for ( int nPage = 0; nPage < nCount; nPage++ )
	{
		CWizardPage* pPage = (CWizardPage*)CPropertySheet::GetPage( nPage );
		ASSERT_KINDOF(CWizardPage, pPage);
		if ( pPage->IsKindOf( pClass ) ) return pPage;
	}

	return NULL;
}

void CWizardSheet::DoReset()
{
	const int nCount = GetPageCount();
	for ( int nPage = 0; nPage < nCount; nPage++ )
	{
		CWizardPage* pPage = (CWizardPage*)CPropertySheet::GetPage( nPage );
		ASSERT_KINDOF(CWizardPage, pPage);
		if ( pPage->m_hWnd != NULL ) pPage->OnReset();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet message handlers

BOOL CWizardSheet::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	SetIcon( theApp.LoadIcon( IDR_MAINFRAME ), TRUE );
	SetFont( &theApp.m_fntNormal );

	// ToDo: Fix Minimize Button Properly
	ModifyStyle( 0, WS_MINIMIZEBOX );

	CRect rc;
	GetWindowRect( &rc );
	ScreenToClient( &rc );
	const UINT nWidth = rc.Width();

	// Set Cancel
	GetDlgItem( 2 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( rc.Height() - 3 - rc.left, -1 );
	GetDlgItem( 2 )->MoveWindow( &rc );
	GetDlgItem( 2 )->SetWindowText( L"E&xit" );

	// Set Back
	GetDlgItem( 0x3023 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( nWidth - rc.left - rc.Width() - rc.Width() - rc.Height() - 10, -1 );
	GetDlgItem( 0x3023 )->MoveWindow( &rc );

	// Set Next
	GetDlgItem( 0x3024 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( nWidth - rc.left - rc.Width() - rc.Height() - 3, -1 );
	GetDlgItem( 0x3024 )->MoveWindow( &rc );
	GetDlgItem( 0x3025 )->MoveWindow( &rc );	// Ready button

	// Set Help
	//GetDlgItem( 0x0009 )->GetWindowRect( &rc );
	//ScreenToClient( &rc );
	//rc.OffsetRect( 18 - rc.left + rc.Width() + 8, -1 );
	//GetDlgItem( 0x0009 )->MoveWindow( &rc );

	if ( GetDlgItem( 0x0009 ) ) GetDlgItem( 0x0009 )->ShowWindow( SW_HIDE );
	if ( GetDlgItem( 0x3026 ) ) GetDlgItem( 0x3026 )->ShowWindow( SW_HIDE );

	return TRUE;
}

BOOL CWizardSheet::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	CWnd* pWnd = GetActivePage();

	if ( pWnd != NULL )
	{
		if ( GetWindowLongPtr( pWnd->GetSafeHwnd(), GWLP_USERDATA ) == TRUE )
		{
			pWnd = NULL;
		}
		else
		{
			SetWindowLongPtr( pWnd->GetSafeHwnd(), GWLP_USERDATA, TRUE );
			pWnd->SetFont( &theApp.m_fntNormal, FALSE );
			pWnd = pWnd->GetWindow( GW_CHILD );
		}
	}

	while ( pWnd != NULL )
	{
		TCHAR szName[32];

		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( !_tcscmp( szName, L"Static" ) )
			pWnd->SetFont( &theApp.m_fntNormal, FALSE );
		else if ( _tcscmp( szName, L"RICHEDIT" ) )
			pWnd->SetFont( &theApp.m_fntNormal, TRUE );

		pWnd = pWnd->GetNextWindow();
	}

	return CPropertySheet::OnChildNotify( message, wParam, lParam, pLResult );
}

void CWizardSheet::OnSize(UINT nType, int cx, int cy)
{
	CPropertySheet::OnSize( nType, cx, cy );

	if ( CWnd* pWnd = GetWindow( GW_CHILD ) )
	{
		GetClientRect( &m_rcPage );

		m_rcPage.top += m_nBannerHeight;
		m_rcPage.bottom -= m_nMenuHeight;

		pWnd->SetWindowPos( NULL, m_rcPage.left, m_rcPage.top, m_rcPage.Width(), m_rcPage.Height(), SWP_NOSIZE );
	}
}

void CWizardSheet::OnPaint()
{
	CPaintDC dc( this );

	CRect rc;
	GetClientRect( &rc );

	// Banner
	CRect rcHeader = rc;
	rcHeader.bottom = rcHeader.top + m_nBannerHeight;
	//rc.top += m_nBannerHeight;

	const int nWidth = rc.Width();
	const int nImageSize = m_bmHeader.GetBitmapDimension().cx;

	CDC mdc;
	mdc.CreateCompatibleDC( &dc );
	CBitmap* pOldBitmap = (CBitmap*)mdc.SelectObject( &m_bmHeader );
	dc.BitBlt( rcHeader.left, rcHeader.top, nWidth, m_nBannerHeight, &mdc, 0, 0, SRCCOPY );
	if ( nImageSize < nWidth )
	{
		for ( int nOffset = nImageSize; nOffset < nWidth; nOffset += 2 )
			dc.BitBlt( nOffset, rcHeader.top, 2, m_nBannerHeight, &mdc, nImageSize - 2, 0, SRCCOPY );
	}
	mdc.SelectObject( pOldBitmap );
	mdc.DeleteDC();

	// Bevels
	//dc.Draw3dRect( 0, m_nBannerHeight, rc.Width() + 1, 1,
	//	RGB( 128, 128, 128 ), RGB( 128, 128, 128 ) );
	dc.Draw3dRect( 0, rc.bottom - m_nMenuHeight, nWidth + 1, 2,
		RGB( 140, 140, 140 ), RGB( 254, 254, 254 ) );

	rc.top = rc.bottom - m_nMenuHeight + 2;

	// Version Text
	CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_fntTiny );
	CString str = L"v" + theApp.m_sVersion;
	CSize sz = dc.GetTextExtent( str );

	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( GetSysColor( COLOR_BTNFACE ) );
	dc.SetTextColor( GetSysColor( COLOR_BTNSHADOW ) );
	dc.ExtTextOut( rc.right - sz.cx - 2, rc.bottom - sz.cy - 1, ETO_CLIPPED|ETO_OPAQUE, &rc, str, NULL );
	dc.SelectObject( pOldFont );
}

//BOOL CWizardSheet::OnEraseBkgnd(CDC* /*pDC*/)
//{
//	return TRUE;
//}

BOOL CWizardSheet::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint pt;
	GetCursorPos( &pt );
	ScreenToClient( &pt );

	if ( pt.y < m_nBannerHeight )
	{
		CRect rc;
		GetClientRect( &rc );
		if ( rc.PtInRect( pt ) )
		{
			SetCursor( theApp.LoadCursor( IDC_HAND ) );
			return TRUE;
		}
	}

	return CPropertySheet::OnSetCursor( pWnd, nHitTest, message );
}

void CWizardSheet::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if ( point.y < m_nBannerHeight )
		ShellExecute( NULL, NULL, L"http://getenvy.com/wiki/TorrentEnvy/", NULL, NULL, SW_SHOWNORMAL );
}

void CWizardSheet::OnNcLButtonUp(UINT nHitTest, CPoint /*point*/)
{
	// Broken Minimize Button Workaround. (Still requires double-click?)  ToDo: Fix properly & Remove this
	if ( nHitTest == HTCLOSE )
		PostMessage( WM_SYSCOMMAND, SC_CLOSE );
	else if ( nHitTest == HTMINBUTTON )
		ShowWindow( SW_MINIMIZE );
}

void CWizardSheet::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		this->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		this->PressButton( PSBTN_NEXT );
}


/////////////////////////////////////////////////////////////////////////////
// CWizardPage

IMPLEMENT_DYNCREATE(CWizardPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CWizardPage, CPropertyPage)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_PRESSBUTTON, OnPressButton)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardPage construction

CWizardPage::CWizardPage(UINT nID) : CPropertyPage( nID )
{
	m_brPageColor.CreateSolidBrush( PAGE_COLOR );
}

//CWizardPage::~CWizardPage()
//{
//}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage message handlers

HBRUSH CWizardPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT /*nCtlColor*/)
{
	if ( pWnd != NULL && pWnd->GetDlgCtrlID() == IDC_TITLE )
		pDC->SelectObject( &theApp.m_fntBold );

	pDC->SetBkColor( PAGE_COLOR );
	return (HBRUSH)m_brPageColor.GetSafeHandle();
}

void CWizardPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);

	CWizardSheet* pSheet = (CWizardSheet*)GetParent();

	if ( cx != pSheet->m_rcPage.Width() )
		MoveWindow( &pSheet->m_rcPage );
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage operations

void CWizardPage::Next()
{
	PostMessage( WM_PRESSBUTTON, PSBTN_NEXT );
}

LRESULT CWizardPage::OnPressButton(WPARAM wParam, LPARAM /*lParam*/)
{
	UpdateWindow();

	Sleep( 250 );

	GetSheet()->PressButton( (int)wParam );

	return 0;
}

CWizardSheet* CWizardPage::GetSheet()
{
	return (CWizardSheet*)GetParent();
}

CWizardPage* CWizardPage::GetPage(CRuntimeClass* pClass)
{
	return GetSheet()->GetPage( pClass );
}

void CWizardPage::SetWizardButtons(DWORD dwFlags)
{
	GetSheet()->SetWizardButtons( dwFlags );
}

void CWizardPage::StaticReplace(LPCTSTR pszSearch, LPCTSTR pszReplace)
{
	for ( CWnd* pChild = GetWindow( GW_CHILD ); pChild; pChild = pChild->GetNextWindow() )
	{
		TCHAR szName[32];
		GetClassName( pChild->GetSafeHwnd(), szName, 32 );

		if ( _tcscmp( szName, L"Static" ) != 0 )
			continue;

		CString strText;
		pChild->GetWindowText( strText );

		for ( ;; )
		{
			int nPos = strText.Find( pszSearch );
			if ( nPos < 0 ) break;
			strText	= strText.Left( nPos ) + CString( pszReplace )
					+ strText.Mid( nPos + static_cast< int >( _tcslen( pszSearch ) ) );
		}

		pChild->SetWindowText( strText );
	}
}
