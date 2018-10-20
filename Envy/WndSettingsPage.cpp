//
// WndSettingsPage.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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
#include "WndSettingsSheet.h"
#include "WndSettingsPage.h"

#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CSettingsPage, CDialog)

BEGIN_MESSAGE_MAP(CSettingsPage, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
//	ON_MESSAGE(WM_CTLCOLORSTATIC, OnCtlColorStatic)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSettingsPage construction

CSettingsPage::CSettingsPage(UINT nIDTemplate, LPCTSTR pszName) : CDialog( nIDTemplate )
{
	if ( pszName )
		m_sName = pszName;

	if ( m_lpszTemplateName != NULL )
		LoadDefaultCaption();
}

CSettingsPage::~CSettingsPage()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsPage operations

BOOL CSettingsPage::LoadDefaultCaption()
{
	CDialogTemplate pTemplate;
	DLGTEMPLATE* pData;
	LPWORD pWord;

	if ( ! pTemplate.Load( m_lpszTemplateName ) ) return FALSE;

	pData = (DLGTEMPLATE*)GlobalLock( pTemplate.m_hTemplate );
	if ( ! pData ) return FALSE;

	if ( ((DLGTEMPLATEEX*)pData)->signature == 0xFFFF )
		pWord = (WORD*)( (DLGTEMPLATEEX*)pData + 1 );
	else
		pWord = (WORD*)( pData + 1 );

	if ( *pWord == 0xFFFF )
		pWord += 2;
	else
		while ( *pWord++ );

	if ( *pWord == 0xFFFF )
		pWord += 2;
	else
		while ( *pWord++ );

	m_sCaption = (wchar_t*)pWord;

	GlobalUnlock( pTemplate.m_hTemplate );

	return ! m_sCaption.IsEmpty();
}

BOOL CSettingsPage::Create(const CRect& rcPage, CWnd* pSheetWnd)
{
	ASSERT_VALID(this);
	ASSERT( m_lpszTemplateName != NULL );

	CDialogTemplate pTemplate;
	LPDLGTEMPLATE pData;

	if ( ! pTemplate.Load( m_lpszTemplateName ) ) return FALSE;
	pData = (LPDLGTEMPLATE)GlobalLock( pTemplate.m_hTemplate );
	if ( ! pData ) return FALSE;

	DWORD dwExStyle = Settings.General.LanguageRTL ? WS_EX_RTLREADING|WS_EX_RIGHT|WS_EX_LEFTSCROLLBAR|WS_EX_LAYOUTRTL :
		WS_EX_LTRREADING|WS_EX_LEFT|WS_EX_RIGHTSCROLLBAR;

	if ( ((DLGTEMPLATEEX*)pData)->signature == 0xFFFF )
	{
		DLGTEMPLATEEX* pEx = (DLGTEMPLATEEX*)pData;
		pEx->style		= WS_CHILDWINDOW|WS_OVERLAPPED|DS_3DLOOK|DS_SETFONT|DS_CONTROL;
		pEx->exStyle	= dwExStyle|WS_EX_WINDOWEDGE|WS_EX_CONTROLPARENT;
	}
	else
	{
		pData->style			= WS_CHILDWINDOW|WS_OVERLAPPED|DS_3DLOOK|DS_SETFONT|DS_CONTROL;
		pData->dwExtendedStyle	= dwExStyle|WS_EX_WINDOWEDGE|WS_EX_CONTROLPARENT;
	}

	GlobalUnlock( pTemplate.m_hTemplate );
	CreateIndirect( pTemplate.m_hTemplate, pSheetWnd );
	SetFont( &theApp.m_gdiFont );

	MoveWindow( rcPage );

	return ( m_hWnd != NULL );
}

CSettingsPage* CSettingsPage::GetPage(CRuntimeClass* pClass) const
{
	return GetSheet()->GetPage( pClass );
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsPage message handlers

BOOL CSettingsPage::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_wndToolTip.Create( this );
	m_wndToolTip.Activate( TRUE );
	m_wndToolTip.SetMaxTipWidth( 200 );
	m_wndToolTip.SetDelayTime( TTDT_AUTOPOP, 20*1000 );		// Show tooltips for 20 seconds

	return TRUE;
}

void CSettingsPage::OnSkinChange()
{
	if ( ! IsWindow( GetSafeHwnd() ) )
		return;		// No created page yet

	if ( m_sName.IsEmpty() )
		m_sName = GetRuntimeClass()->m_lpszClassName;

	SetWindowText( m_sCaption );

	Skin.Apply( m_sName, this, 0, &m_wndToolTip );

	if ( Images.m_bmDialog.m_hObject )
	{
		CDC* pDC = GetDC();
		CRect rc;
		GetClientRect( &rc );
		CoolInterface.DrawWatermark( pDC, &rc, &Images.m_bmDialog );
	}

	// Fix Checkbox/Groupbox color skinning if needed (remove modern Windows theming)

	CoolInterface.FixThemeControls( this );		// Checkbox/Groupbox text colors (Remove theme if needed)

// Moved to CCoolInterface::FixTheme:
//	const BOOL bThemed =
//		GetRValue( Colors.m_crDialogText ) < 100 &&
//		GetGValue( Colors.m_crDialogText ) < 100 &&
//		GetBValue( Colors.m_crDialogText ) < 100;
//
//	for ( CWnd* pWnd = GetWindow( GW_CHILD ); pWnd; pWnd = pWnd->GetNextWindow() )
//	{
//		TCHAR szName[8];
//		GetClassName( pWnd->GetSafeHwnd(), szName, 8 );			// Alt detection method for exceptions
//		if ( _tcsnicmp( szName, _P( L"Button" ) ) == 0 &&
//			 ( pWnd->GetStyle() & BS_CHECKBOX ) || ( ( pWnd->GetStyle() & BS_GROUPBOX ) && pWnd->GetDlgCtrlID() == IDC_STATIC ) )
//			CCoolInterface::EnableTheme( pWnd, bThemed );
//	}
}

void CSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
}

void CSettingsPage::SetModified(BOOL bChanged)
{
	ASSERT_VALID(this);
	GetSheet()->SetModified( bChanged );
}

BOOL CSettingsPage::OnApply()
{
	ASSERT_VALID(this);
	OnOK();
	return TRUE;
}

void CSettingsPage::OnReset()
{
	ASSERT_VALID(this);
	OnCancel();
}

void CSettingsPage::OnOK()
{
	ASSERT_VALID(this);
}

void CSettingsPage::OnCancel()
{
	ASSERT_VALID(this);
}

BOOL CSettingsPage::OnSetActive()
{
	ASSERT_VALID(this);
	return TRUE;
}

BOOL CSettingsPage::OnKillActive()
{
	ASSERT_VALID(this);
	if ( ! UpdateData() ) return FALSE;
	return TRUE;
}

BOOL CSettingsPage::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect( &rc );

	if ( Images.m_bmDialog.m_hObject )
		CoolInterface.DrawWatermark( pDC, &rc, &Images.m_bmDialog );
	else
		pDC->FillSolidRect( &rc, Colors.m_crDialog );

	return TRUE;
}

HBRUSH CSettingsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	//HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	// For checkboxes/Groups, need to SetWindowTheme( pWnd->GetSafeHwnd(), L"wstr", L"wstr")

	pDC->SetTextColor( Colors.m_crDialogText );
	pDC->SetBkColor( Colors.m_crDialog );

	// Skinned dialog controls
	if ( nCtlColor == CTLCOLOR_STATIC && Images.m_bmDialog.m_hObject )
	{
		if ( ! ( pWnd->GetStyle() & BS_GROUPBOX ) ||
			 ( GetRValue( Colors.m_crDialogText ) < 50 && GetGValue( Colors.m_crDialogText ) < 50 && GetBValue( Colors.m_crDialogText ) < 50 ) )
			pDC->SetBkMode( TRANSPARENT );

		if ( pWnd->GetDlgCtrlID() == IDC_STATIC )
			return (HBRUSH)GetStockObject( NULL_BRUSH );

		// Named controls  (Dynamic handling)						// || ( pWnd->GetStyle() & SS_REALSIZEIMAGE ) || ( pWnd->GetStyle() & SS_ICON )

		if ( ( pWnd->GetStyle() & ES_READONLY ) )
			return Images.m_brDialog;								// Skip disabled edit boxes (Not disabled text)

		//SetWindowTheme( pWnd->GetSafeHwnd(), L"wstr", L"wstr");	// Allow Checkbox text colors (causes flicker)

		//if ( ! pWnd->IsWindowEnabled() )
		//{
		//	const int nID = pWnd->GetDlgCtrlID();
		//	if ( nID != IDC_REMOTE_URL && nID != IDC_INBOUND_BIND && nID != IDC_G1_SETUP && nID != IDC_ED2K_SETUP && nID != IDC_DC_SETUP )	// Keep exception list updated
		//		return Images.m_brDialog;							// Skip disabled edit boxes (Not disabled text)
		//}

		//TCHAR szName[24];
		//GetClassName( pWnd->GetSafeHwnd(), szName, 24 );			// Alt detection method for exceptions
		//if ( _tcsistr( szName, L"Static" ) )					// "Static" "Button" "ListBox" "ComboBox" "Edit" "RICHEDIT" etc

		// Offset background image brush to mimic transparency
		CRect rc;
		pWnd->GetWindowRect( &rc );
		ScreenToClient( &rc );
		pDC->SetBrushOrg( -rc.left, -rc.top );
	}

	return Images.m_brDialog;
}

//LRESULT CSettingsPage::OnCtlColorStatic(WPARAM wParam, LPARAM lParam)
//{
//	HBRUSH hbr = Images.m_brDialog;
//	HDC hDCStatic = (HDC)wParam;
//	HWND hWnd = (HWND)lParam;
//
//	SetTextColor( hDCStatic, Colors.m_crDialogText );
//	SetBkColor( hDCStatic, Colors.m_crDialog );
//
//	if ( Images.m_bmDialog.m_hObject )
//	{
//		CRect rc;
//		CWnd::FromHandle( hWnd )->GetWindowRect( &rc );
//		ScreenToClient( &rc );
//		CDC::FromHandle( hDCStatic )->SetBrushOrg( -rc.left, -rc.top );
//		SetBkMode( hDCStatic, TRANSPARENT );
//	}
//
//	return (LRESULT)hbr;
//}

BOOL CSettingsPage::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST )
	{
		MSG msg;
		CopyMemory( &msg, pMsg, sizeof( MSG ) );
		HWND hWndParent = ::GetParent( msg.hwnd );

		while ( hWndParent && hWndParent != m_hWnd )
		{
			msg.hwnd = hWndParent;
			hWndParent = ::GetParent( hWndParent );
		}

		if ( msg.hwnd )
			m_wndToolTip.RelayEvent( &msg );
	}

	return CDialog::PreTranslateMessage(pMsg);
}


/////////////////////////////////////////////////////////////////////////////
// CEditPath	( Launch from double-click aware standard Edit boxes )

IMPLEMENT_DYNAMIC(CEditPath, CEdit)

BEGIN_MESSAGE_MAP(CEditPath, CEdit)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

void CEditPath::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CEdit::OnLButtonDblClk( nFlags, point );

	CString strPath;
	GetWindowText( strPath );

	if ( strPath.IsEmpty() )
		return;

	MakeSafePath( strPath );

	if ( PathIsDirectory( strPath ) )
		ShellExecute( GetSafeHwnd(), NULL, strPath, NULL, NULL, SW_SHOWDEFAULT );
	else if ( GetFileAttributes( strPath ) != INVALID_FILE_ATTRIBUTES )
		ShellExecute( GetSafeHwnd(), NULL, L"Explorer.exe", L"/select, " + strPath, NULL, SW_SHOWNORMAL );
}
