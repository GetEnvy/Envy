//
// PagePropertyAdv.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2006
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
#include "PagePropertyAdv.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "ShellIcons.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// CPropertyPageAdv

IMPLEMENT_DYNAMIC(CPropertyPageAdv, CPropertyPage)

CPropertyPageAdv::CPropertyPageAdv(UINT nIDD)
	: CPropertyPage( nIDD )
	, m_nIcon	( -1 )
{
	m_psp.dwFlags |= PSP_USETITLE;
}

void CPropertyPageAdv::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropertyPageAdv, CPropertyPage)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CPropertyPageAdv message handlers

BOOL CPropertyPageAdv::OnInitDialog()
{
	if ( ! CPropertyPage::OnInitDialog() )
		return FALSE;

	Skin.Apply( NULL, this );

	return TRUE;
}

void CPropertyPageAdv::OnPaint()
{
	CPaintDC dc( this );
	if ( Settings.General.LanguageRTL )
		SetLayout( dc.m_hDC, LAYOUT_RTL );

	// ToDo: Skin tabs background, see CIRCTabCtrl::DrawTabThemed() ?

	if ( m_nIcon >= 0 )
		ShellIcons.Draw( &dc, m_nIcon, 48, 4, 4 );

	for ( CWnd* pWnd = GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		if ( pWnd->GetStyle() & WS_VISIBLE ) continue;

		TCHAR szClass[16];
		GetClassName( pWnd->GetSafeHwnd(), szClass, 16 );
		if ( _tcsicmp( szClass, L"STATIC" ) ) continue;

		CRect rc;
		CString str;
		pWnd->GetWindowText( str );
		pWnd->GetWindowRect( &rc );
		ScreenToClient( &rc );

		if ( str.IsEmpty() || str.GetAt( 0 ) != '-' )
			PaintStaticHeader( &dc, &rc, str );
	}

	dc.SetBkColor( Images.m_bmDialog.m_hObject ? CLR_NONE : Colors.m_crDialogPanel );
}

void CPropertyPageAdv::PaintStaticHeader(CDC* pDC, CRect* prc, LPCTSTR psz)
{
	CFont* pOldFont = (CFont*)pDC->SelectObject( GetFont() );
	CSize sz = pDC->GetTextExtent( psz );

	pDC->SetBkMode( OPAQUE );
	pDC->SetBkColor( Colors.m_crBannerBack );
	pDC->SetTextColor( Colors.m_crBannerText );

	CRect rc( prc );
	rc.bottom	= rc.top + min( rc.Height(), 16 );
	rc.right	= rc.left + sz.cx + 10;

	pDC->ExtTextOut( rc.left + 4, rc.top + 1, ETO_CLIPPED|ETO_OPAQUE,
		&rc, psz, static_cast< UINT >( _tcslen( psz ) ), NULL );

	rc.SetRect( rc.right, rc.top, prc->right, rc.top + 1 );
	pDC->ExtTextOut( rc.left, rc.top, ETO_OPAQUE, &rc, NULL, 0, NULL );

	pDC->SelectObject( pOldFont );
}

BOOL CPropertyPageAdv::OnEraseBkgnd(CDC* pDC)
{
	CPropertySheetAdv* pSheet = (CPropertySheetAdv*)GetParent();
	if ( pSheet->m_pSkin && pSheet->m_pSkin->OnEraseBkgnd( this, pDC ) ) return TRUE;

	CRect rc;
	GetClientRect( &rc );

	if ( Images.m_bmDialogPanel.m_hObject )
		CoolInterface.DrawWatermark( pDC, &rc, &Images.m_bmDialogPanel );
	else
		pDC->FillSolidRect( &rc, Colors.m_crDialogPanel );

	return TRUE;
}

HBRUSH CPropertyPageAdv::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

	// Skinned dialog control panels	(Download/File Properties Tabs + QuickStart Wizard)
	if ( nCtlColor == CTLCOLOR_STATIC && Images.m_bmDialogPanel.m_hObject )
	{
		const int nID = pWnd->GetDlgCtrlID();
		if ( nID != IDC_STATIC || ( pWnd->GetStyle() & SS_REALSIZEIMAGE ) )		// || ( pWnd->GetStyle() & SS_ICON )
		{
			// Handle exceptions:
			// Skip disabled edit boxes (but not selectable metadata or icons, note SS_REALSIZEIMAGE/ES_READONLY conflict)
			if ( ( pWnd->GetStyle() & ES_READONLY ) && ( pWnd->GetStyle() & WS_BORDER ) && nID != IDC_STATIC )
				return hbr;

			// Obsolete skinning fix for checkbox labels, dynamic text, etc., for reference
		//	CRect rc;
		//	pWnd->GetWindowRect( &rc );
		//	CRect rcPos = rc;
		//	pWnd->ScreenToClient( &rc );
		//	ScreenToClient( &rcPos );
		//	CoolInterface.DrawWatermark( pDC, &rc, &Images.m_bmDialogPanel, FALSE, -rcPos.left, -rcPos.top );

			// Offset background image brush to mimic transparency
			CRect rc;
			pWnd->GetWindowRect( &rc );
			ScreenToClient( &rc );
			pDC->SetBrushOrg( -rc.left, -rc.top );

			hbr = Images.m_brDialogPanel;
		}
		else	// Static text	(Note, use IDC_STATIC_1+ for an IDC_STATIC to avoid NULL brush)
			hbr = (HBRUSH)GetStockObject( NULL_BRUSH );

		pDC->SetTextColor( Colors.m_crDialogPanelText );
		pDC->SetBkMode( TRANSPARENT );
	}
	else if ( nCtlColor == CTLCOLOR_BTN && Images.m_bmDialogPanel.m_hObject )
	{
		CRect rc;
		pWnd->GetWindowRect( &rc );
		ScreenToClient( &rc );
		pDC->SetBrushOrg( -rc.left, -rc.top );
		hbr = Images.m_brDialogPanel;
	}
	else if ( nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_DLG )
	{
		pDC->SetTextColor( Colors.m_crDialogPanelText );
		pDC->SetBkColor( Colors.m_crDialogPanel );
		hbr = Images.m_brDialogPanel;
	}

	return hbr;
}


// CPropertySheetAdv

IMPLEMENT_DYNAMIC(CPropertySheetAdv, CPropertySheet)

BEGIN_MESSAGE_MAP(CPropertySheetAdv, CPropertySheet)
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCMOUSELEAVE()
	ON_WM_SIZE()
//	ON_WM_ERASEBKGND()
	ON_WM_HELPINFO()
	ON_MESSAGE(WM_SETTEXT, &CPropertySheetAdv::OnSetText)
END_MESSAGE_MAP()

CPropertySheetAdv::CPropertySheetAdv()
	: CPropertySheet( L"" )
	, m_pSkin	( NULL )
{
	m_psh.dwFlags &= ~PSP_HASHELP;
}

BOOL CPropertySheetAdv::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	m_pSkin = Skin.GetWindowSkin( L"CTorrentSheet" );
	if ( m_pSkin == NULL ) m_pSkin = Skin.GetWindowSkin( this );
	if ( m_pSkin == NULL ) m_pSkin = Skin.GetWindowSkin( L"CDialog" );

	if ( m_pSkin )
	{
		CRect rc;
		GetClientRect( &rc );
		m_pSkin->CalcWindowRect( &rc );
		SetWindowPos( NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED );
		OnSize( 1982, 0, 0 );
	}

	return bResult;
}

void CPropertySheetAdv::SetTabTitle(CPropertyPage* pPage, CString& strTitle)
{
	CString strClass( pPage->GetRuntimeClass()->m_lpszClassName );
	CString strTabLabel = Skin.GetDialogCaption( strClass );
	if ( ! strTabLabel.IsEmpty() )
		strTitle = strTabLabel;
	pPage->m_psp.pszTitle = strTitle.GetBuffer();
}

void CPropertySheetAdv::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if ( m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CPropertySheet::OnNcCalcSize( bCalcValidRects, lpncsp );
}

LRESULT CPropertySheetAdv::OnNcHitTest(CPoint point)
{
	if ( m_pSkin )
		return m_pSkin->OnNcHitTest( this, point, ( GetStyle() & WS_THICKFRAME ) ? TRUE : FALSE );

	return CPropertySheet::OnNcHitTest( point );
}

BOOL CPropertySheetAdv::OnNcActivate(BOOL bActive)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		BOOL bResult = CPropertySheet::OnNcActivate( bActive );
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		m_pSkin->OnNcActivate( this, bActive || ( m_nFlags & WF_STAYACTIVE ) );
		return bResult;
	}

	return CPropertySheet::OnNcActivate( bActive );
}

void CPropertySheetAdv::OnNcPaint()
{
	if ( m_pSkin )
		m_pSkin->OnNcPaint( this );
	else
		CPropertySheet::OnNcPaint();
}

void CPropertySheetAdv::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDown( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonDown(nHitTest, point);
}

void CPropertySheetAdv::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonUp( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonUp( nHitTest, point );
}

void CPropertySheetAdv::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDblClk( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonDblClk( nHitTest, point );
}

void CPropertySheetAdv::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	if ( m_pSkin ) m_pSkin->OnNcMouseMove( this, nHitTest, point );
	CPropertySheet::OnNcMouseMove( nHitTest, point );
}

void CPropertySheetAdv::OnNcMouseLeave()
{
	if ( m_pSkin ) m_pSkin->OnNcMouseLeave( this );
}

void CPropertySheetAdv::OnSize(UINT nType, int cx, int cy)
{
	if ( m_pSkin ) m_pSkin->OnSize( this );

	if ( nType != 1982 ) CPropertySheet::OnSize( nType, cx, cy );
}

LRESULT CPropertySheetAdv::OnSetText(WPARAM /*wParam*/, LPARAM /*lParam*/)
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

BOOL CPropertySheetAdv::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	return FALSE;
}

// ToDo: Handle background of tab control, clashes with below skinning
//BOOL CPropertySheetAdv::OnEraseBkgnd(CDC* pDC)
//{
//	if ( m_pSkin && m_pSkin->OnEraseBkgnd( this, pDC ) ) return TRUE;
//
//	CRect rc;
//	GetClientRect( &rc );
//
//	if ( Images.m_bmDialogPanel.m_hObject )
//		CoolInterface.DrawWatermark( pDC, &rc, &Images.m_bmDialog );
//	else
//		pDC->FillSolidRect( &rc, Colors.m_crDialog );
//
//	return TRUE;
//}
