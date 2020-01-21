//
// WizardSheet.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2020
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2016
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
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "GProfile.h"
#include "WizardSheet.h"

#include "WizardWelcomePage.h"
#include "WizardConnectionPage.h"
#include "WizardNetworksPage.h"
#include "WizardFoldersPage.h"
#include "WizardSharePage.h"
#include "WizardInterfacePage.h"
#include "WizardProfilePage.h"
#include "WizardFinishedPage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define CONTROLBAR_HEIGHT	44		// m_nNavBar Default
#define BUTTON_GAP			8

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet

IMPLEMENT_DYNAMIC(CWizardSheet, CPropertySheetAdv)

BEGIN_MESSAGE_MAP(CWizardSheet, CPropertySheetAdv)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_XBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet run wizard

BOOL CWizardSheet::RunWizard(CWnd* pParent)
{
	CWizardSheet pSheet( pParent );

	CWizardWelcomePage		pWelcome;
	CWizardConnectionPage	pConnection;
	CWizardNetworksPage		pNetworks;
	CWizardFoldersPage		pFolders;
	CWizardSharePage		pShare;
	CWizardInterfacePage	pInterface;
	CWizardProfilePage		pProfile;
	CWizardFinishedPage		pFinished;

	pSheet.AddPage( &pWelcome );
	pSheet.AddPage( &pConnection );
	pSheet.AddPage( &pNetworks );
	pSheet.AddPage( &pFolders );
	pSheet.AddPage( &pShare );
	pSheet.AddPage( &pInterface );
	pSheet.AddPage( &pProfile );
	pSheet.AddPage( &pFinished );

	BOOL bSuccess = ( pSheet.DoModal() == IDOK );

	Settings.Save();

	return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CWizardSheet construction

CWizardSheet::CWizardSheet(CWnd *pParentWnd, UINT iSelectPage)
{
	Construct( L"", pParentWnd, iSelectPage );

	SetWizardMode();
}

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet message handlers

BOOL CWizardSheet::OnInitDialog()
{
	CPropertySheetAdv::OnInitDialog();

	CRect rc;
	GetClientRect( &rc );
	const int nCenter = rc.Width() / 2 + 1;		// Accomodate HighDPI

	m_nNavBar = SCALE( CONTROLBAR_HEIGHT );

	SetIcon( CoolInterface.ExtractIcon( IDR_MAINFRAME, FALSE ), FALSE );
	SetFont( &theApp.m_gdiFont );

//	LoadString( IDS_WIZARD );
//	SetWindowText( strMessage );

	if ( GetDlgItem( IDCANCEL ) )
	{
		GetDlgItem( IDCANCEL )->GetWindowRect( &rc );
		ScreenToClient( &rc );
		rc.OffsetRect( ( nCenter - rc.Width() - BUTTON_GAP - ( rc.Width() / 2 ) ) - rc.left, -1 );
		GetDlgItem( IDCANCEL )->MoveWindow( &rc );
		GetDlgItem( IDCANCEL )->SetWindowText( LoadString( IDS_WIZARD_EXIT ) );
	}

	if ( GetDlgItem( ID_WIZBACK ) )
	{
		GetDlgItem( ID_WIZBACK )->GetWindowRect( &rc );
		ScreenToClient( &rc );
		rc.OffsetRect( ( nCenter - ( rc.Width() / 2 ) ) - rc.left, -1 );
		GetDlgItem( ID_WIZBACK )->MoveWindow( &rc );
		GetDlgItem( ID_WIZBACK )->SetWindowText( L"< " + LoadString( IDS_GENERAL_BACK ) );
	}

	if ( GetDlgItem( ID_WIZNEXT ) )
	{
		GetDlgItem( ID_WIZNEXT )->GetWindowRect( &rc );
		ScreenToClient( &rc );
		rc.OffsetRect( ( nCenter + BUTTON_GAP + ( rc.Width() / 2 ) ) - rc.left, -1 );
		GetDlgItem( ID_WIZNEXT )->MoveWindow( &rc );
		GetDlgItem( ID_WIZNEXT )->SetWindowText( LoadString( IDS_GENERAL_NEXT ) + L" >" );

		GetDlgItem( ID_WIZFINISH )->MoveWindow( &rc );
		GetDlgItem( ID_WIZFINISH )->SetWindowText( LoadString( IDS_GENERAL_FINISH ) );
	}

	if ( GetDlgItem( IDHELP ) )
		GetDlgItem( IDHELP )->ShowWindow( SW_HIDE );

	if ( GetDlgItem( 0x3026 ) )		// ATL_IDC_STATIC1?
		GetDlgItem( 0x3026 )->ShowWindow( SW_HIDE );

	//m_bmHeader.Attach( Skin.GetWatermark( L"Banner" ) );	// Use Images.m_bmBanner

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
			pWnd->SetFont( &theApp.m_gdiFont, FALSE );
			pWnd = pWnd->GetWindow( GW_CHILD );
		}
	}

	while ( pWnd != NULL )
	{
		TCHAR szName[32];

		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( _tcscmp( szName, L"Static" ) == 0 )
			pWnd->SetFont( &theApp.m_gdiFont, FALSE );
		else if ( _tcscmp( szName, L"RICHEDIT" ) != 0 )
			pWnd->SetFont( &theApp.m_gdiFont, TRUE );

		pWnd = pWnd->GetNextWindow();
	}

	return CPropertySheetAdv::OnChildNotify( message, wParam, lParam, pLResult );
}

void CWizardSheet::OnSize(UINT nType, int cx, int cy)
{
	CPropertySheetAdv::OnSize( nType, cx, cy );

	if ( CWnd* pWnd = GetWindow( GW_CHILD ) )
	{
		GetClientRect( &m_rcPage );

		m_rcPage.top += Skin.m_nBanner;
		m_rcPage.bottom -= m_nNavBar + 2;

		pWnd->SetWindowPos( NULL, m_rcPage.left, m_rcPage.top, m_rcPage.Width(), m_rcPage.Height(), SWP_NOSIZE );
	}
}

void CWizardSheet::OnPaint()
{
	CPaintDC dc( this );

	CRect rc;
	GetClientRect( &rc );
	//rc.bottom = Images.m_nBanner;

	// ToDo: Use Images.DrawButtonState( &dc, &rc, IMAGE_BANNER );

	CDC mdc;
	mdc.CreateCompatibleDC( &dc );
	CBitmap* pOldBitmap = (CBitmap*)mdc.SelectObject( &Images.m_bmBanner );
	dc.BitBlt( 0, 0, rc.right + 1, Skin.m_nBanner, &mdc, 0, 0, SRCCOPY );
	mdc.SelectObject( pOldBitmap );
	mdc.DeleteDC();

	//dc.Draw3dRect( 0, Skin.m_nBanner, rc.Width() + 1, 1, RGB( 128, 128, 128 ), RGB( 128, 128, 128 ) );

	//GetClientRect( &rc );
	rc.top = rc.bottom - m_nNavBar;

	dc.Draw3dRect( 0, rc.top - 2, rc.Width() + 1, 2, RGB( 142, 141, 140 ), RGB( 255, 255, 255 ) );	// ToDo: Skinned bevel color?

	if ( Images.m_bmDialog.m_hObject )
		CoolInterface.DrawWatermark( &dc, &rc, &Images.m_bmDialog );
	else
		dc.FillSolidRect( rc.left, rc.top, rc.Width(), m_nNavBar, Colors.m_crSysBtnFace );	// Colors.m_crDialog?
}

HBRUSH CWizardSheet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Skinned dialog controls
	if ( nCtlColor == CTLCOLOR_STATIC && Images.m_bmDialog.m_hObject )
	{
		// Offset background image brush to mimic transparency
		CRect rc;
		pWnd->GetWindowRect( &rc );
		ScreenToClient( &rc );
		pDC->SetBrushOrg( -rc.left, -rc.top );
	}

	return Images.m_brDialog;
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

IMPLEMENT_DYNCREATE(CWizardPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CWizardPage, CPropertyPageAdv)
	ON_WM_SIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardPage construction

CWizardPage::CWizardPage(UINT nID) : CPropertyPageAdv( nID )
{
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage message handlers

void CWizardPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPageAdv::OnSize(nType, cx, cy);

	CWizardSheet* pSheet = (CWizardSheet*)GetParent();

	if ( cx != pSheet->m_rcPage.Width() )
		MoveWindow( &pSheet->m_rcPage );
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage operations

CWizardSheet* CWizardPage::GetSheet()
{
	return (CWizardSheet*)GetParent();
}

void CWizardPage::SetWizardButtons(DWORD dwFlags)
{
	GetSheet()->SetWizardButtons( dwFlags );
}

void CWizardPage::StaticReplace(LPCTSTR pszSearch, LPCTSTR pszReplace)
{
	const int nLen = (int)_tcslen( pszSearch );

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
			strText	= strText.Left( nPos ) + pszReplace + strText.Mid( nPos + nLen );
		}

		pChild->SetWindowText( strText );
	}
}

BOOL CWizardPage::IsConnectionCapable()
{
	return
		! theApp.m_bLimitedConnections
#ifdef XPSUPPORT
		|| Settings.General.IgnoreXPLimits							// The connection rate limiting (XPsp2) makes multi-network performance awful
#endif
		&& ( Settings.Connection.InSpeed > 256 )					// Must have a decent connection to be worth it. (Or extra traffic will slow downloads)
		&& ( Settings.GetOutgoingBandwidth() > 16 );				// If your outbound bandwidth is too low, the ED2K ratio will throttle you anyway
}
