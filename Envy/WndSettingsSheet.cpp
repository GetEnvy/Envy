//
// WndSettingsSheet.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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
#include "WndSettingsSheet.h"
#include "WndSettingsPage.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//#define TOPMARGIN		51	// Header: Skin.m_nBanner + 1
#define LEFTMARGIN		0	// Tree left-edge gap
#define LISTDIVIDER		6	// Splitter Bar
#define LISTWIDTH		118	// Tree Window
#define BUTTONWIDTH		76	// Dialog Menu Buttons
#define BUTTONGAP		7	// Dialog Menu Padding


IMPLEMENT_DYNAMIC(CSettingsSheet, CSkinDialog)

BEGIN_MESSAGE_MAP(CSettingsSheet, CSkinDialog)
	ON_WM_PAINT()
	ON_COMMAND(IDRETRY, OnApply)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SETTINGS_TREE, OnSelectPage)
	//ON_NOTIFY(TVN_ITEMEXPANDING, IDC_SETTINGS_TREE, OnTreeExpanding)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSettingsSheet construction

CSettingsSheet::CSettingsSheet(CWnd* pParent, UINT nCaptionID)
	: CSkinDialog( 0, pParent )
	, m_pPage			( NULL )
	, m_pFirst			( NULL )
	, m_bModified		( FALSE )
	, m_nButtonHeight	( 24 )
{
	if ( nCaptionID )
		LoadString( m_sCaption, nCaptionID );
}

CSettingsSheet::~CSettingsSheet()
{
}

void CSettingsSheet::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange( pDX );
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsSheet operations

void CSettingsSheet::AddPage(CSettingsPage* pPage, LPCTSTR pszCaption)
{
	if ( pszCaption ) pPage->m_sCaption = pszCaption;
	pPage->m_bGroup = FALSE;
	m_pPages.Add( pPage );
}

void CSettingsSheet::AddGroup(CSettingsPage* pPage, LPCTSTR pszCaption)
{
	if ( pszCaption ) pPage->m_sCaption = pszCaption;
	pPage->m_bGroup = TRUE;
	m_pPages.Add( pPage );
}

CSettingsPage* CSettingsSheet::GetPage(INT_PTR nPage) const
{
	return m_pPages.GetAt( nPage );
}

CSettingsPage* CSettingsSheet::GetPage(CRuntimeClass* pClass) const
{
	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = GetPage( nPage );
		if ( pPage->IsKindOf( pClass ) ) return pPage;
	}
	return NULL;
}

CSettingsPage* CSettingsSheet::GetPage(LPCTSTR pszClass) const
{
	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = GetPage( nPage );
		if ( _tcscmp( CString( pPage->GetRuntimeClass()->m_lpszClassName ), pszClass ) == 0 ) return pPage;
	}
	return NULL;
}

INT_PTR CSettingsSheet::GetPageIndex(CSettingsPage* pPage) const
{
	for ( INT_PTR nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		if ( pPage == GetPage( nPage ) ) return nPage;
	}
	return -1;
}

INT_PTR CSettingsSheet::GetPageCount() const
{
	return m_pPages.GetSize();
}

CSettingsPage* CSettingsSheet::GetActivePage() const
{
	return m_pPage;
}

BOOL CSettingsSheet::SetActivePage(CSettingsPage* pPage)
{
	//if ( pPage == NULL || pPage == m_pPage ) return FALSE;

	//ASSERT_KINDOF(CSettingsPage, pPage);

	CWaitCursor wc;

	if ( m_hWnd == NULL )
	{
		m_pFirst = pPage;
		return TRUE;
	}

	if ( m_pPage )
	{
		if ( ! m_pPage->OnKillActive() )
			return FALSE;
		m_pPage->ShowWindow( SW_HIDE );
		m_pPage = NULL;
	}

	CRect rc( LISTWIDTH + LEFTMARGIN + LISTDIVIDER, 0, 0, 0 );
	if ( GetDlgItem( IDC_BANNER ) )
		rc.top	= Skin.m_nBanner + 1;
	rc.right	= rc.left + m_szPages.cx;
	rc.bottom	= rc.top  + m_szPages.cy;

	InvalidateRect( &rc );
	UpdateWindow();

	if ( pPage )
	{
		if ( pPage->m_hWnd == NULL && ! CreatePage( rc, pPage ) )
			return FALSE;
		if ( ! pPage->OnSetActive() )
			return FALSE;
	}

	m_pPage = pPage;

	if ( ! pPage )
		return FALSE;

	m_pPage->MoveWindow( rc );

	m_pPage->ShowWindow( SW_SHOW );

	CString strCaption;
	m_pPage->GetWindowText( strCaption );
	strCaption.Trim();
	if ( ! strCaption.IsEmpty() )
		SetWindowText( m_sCaption + L" - " + strCaption );
	else
		SetWindowText( m_sCaption );

	for ( HTREEITEM hGroup = m_wndTree.GetRootItem() ; hGroup ; hGroup = m_wndTree.GetNextItem( hGroup, TVGN_NEXT ) )
	{
		if ( m_wndTree.GetItemData( hGroup ) == (DWORD_PTR)m_pPage )
		{
			if ( ( m_wndTree.GetItemState( hGroup, TVIS_SELECTED ) & TVIS_SELECTED ) == 0 )
				m_wndTree.SelectItem( hGroup );
		}
		for ( HTREEITEM hItem = m_wndTree.GetChildItem( hGroup ) ; hItem ; hItem = m_wndTree.GetNextItem( hItem, TVGN_NEXT ) )
		{
			if ( m_wndTree.GetItemData( hItem ) == (DWORD_PTR)m_pPage )
			{
				if ( ( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) == 0 )
					m_wndTree.SelectItem( hItem );
			}
		}
	}

	return TRUE;
}

//BOOL CSettingsSheet::SetActivePage(int nPage)
//{
//	return SetActivePage( GetPage( nPage ) );
//}

BOOL CSettingsSheet::IsModified() const
{
	return m_bModified;
}

void CSettingsSheet::SetModified(BOOL bChanged)
{
	if ( m_bModified == bChanged ) return;
	m_bModified = bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsSheet message handlers

INT_PTR CSettingsSheet::DoModal()
{
	char pBuf[ sizeof( DLGTEMPLATE ) + 16 ] = {};
	DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)pBuf;

	DWORD dwExStyle = Settings.General.LanguageRTL ? WS_EX_RTLREADING|WS_EX_RIGHT|WS_EX_LEFTSCROLLBAR|WS_EX_LAYOUTRTL :
		WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR;

	pTemplate->style			= WS_POPUPWINDOW|WS_VISIBLE|WS_DLGFRAME|WS_OVERLAPPED|DS_MODALFRAME;
	pTemplate->dwExtendedStyle	= dwExStyle|WS_EX_DLGMODALFRAME|WS_EX_WINDOWEDGE|WS_EX_CONTROLPARENT;

	pTemplate->cdit	= 0;
	pTemplate->x	= 0;
	pTemplate->y	= 0;
	pTemplate->cx	= 100;
	pTemplate->cy	= 100;

	m_pPage			= NULL;
	m_bModified		= FALSE;

	CSkinDialog::InitModalIndirect( pTemplate, m_pParentWnd );

	INT_PTR nResult = CSkinDialog::DoModal();

	m_pParentWnd	= NULL;
	m_pFirst		= m_pPage;
	m_pPage			= NULL;

	return nResult;
}

BOOL CSettingsSheet::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SetWindowText( m_sCaption );

	CRect rect;
	m_wndTree.Create( WS_CHILD|WS_TABSTOP|WS_VISIBLE|/*TVS_PRIVATEIMAGELISTS|*/
		TVS_FULLROWSELECT|TVS_TRACKSELECT|TVS_SHOWSELALWAYS|TVS_NOSCROLL, rect, this, IDC_SETTINGS_TREE );
		// Note: If needed, use TVS_NOHSCROLL instead to add vertical scrollbar to Settings TreeView
	m_wndTree.SetIndent( 16 );

	m_wndOK.Create( L"OK", WS_CHILD|WS_TABSTOP|WS_VISIBLE|BS_DEFPUSHBUTTON, rect, this, IDOK );
	m_wndOK.SetFont( &theApp.m_gdiFont );
	m_wndCancel.Create( L"Cancel", WS_CHILD|WS_TABSTOP|WS_VISIBLE, rect, this, IDCANCEL );
	m_wndCancel.SetFont( &theApp.m_gdiFont );
	m_wndApply.Create( L"Apply", WS_CHILD|WS_TABSTOP|WS_VISIBLE, rect, this, IDRETRY );
	m_wndApply.SetFont( &theApp.m_gdiFont );

	if ( m_pFirst == NULL ) m_pFirst = GetPage( INT_PTR(0) );
	SetActivePage( m_pFirst );

	BuildTree();

	return TRUE;
}

void CSettingsSheet::BuildTree()
{
	HTREEITEM hGroup = TVI_ROOT;

	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = GetPage( nPage );

		if ( pPage->m_bGroup ) hGroup = TVI_ROOT;

		HTREEITEM hItem = m_wndTree.InsertItem(
			TVIF_PARAM|TVIF_TEXT|TVIF_STATE,
			pPage->m_sCaption, 0, 0, TVIS_EXPANDED|(TVIS_BOLD*pPage->m_bGroup),
			TVIS_EXPANDED|TVIS_BOLD, (LPARAM)pPage, hGroup, TVI_LAST );

		if ( pPage->m_bGroup )
			hGroup = hItem;

		if ( pPage == m_pPage )
			m_wndTree.SelectItem( hItem );
	}
}

BOOL CSettingsSheet::SkinMe(LPCTSTR pszSkin, UINT nIcon, BOOL bLanguage)
{
	EnableBanner( FALSE );

	TEXTMETRIC txtMetric;
	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject( &theApp.m_gdiFont );
	pDC->GetTextMetrics( &txtMetric );
	pDC->SelectObject( pOldFont );
	ReleaseDC( pDC );

	m_nButtonHeight = ( txtMetric.tmHeight + txtMetric.tmExternalLeading ) + 10;

	m_szPages.cx = m_szPages.cy = 0;

	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = GetPage( nPage );
		CDialogTemplate pTemplate;

		if ( pPage->GetTemplateName() == NULL )  continue;

		if ( pTemplate.Load( pPage->GetTemplateName() ) )
		{
			CSize size;
			pTemplate.GetSizeInPixels( &size );
			m_szPages.cx = max( m_szPages.cx, size.cx );
			m_szPages.cy = max( m_szPages.cy, size.cy );
		}
	}

	CRect rcWindow( 0, 0, m_szPages.cx + LISTWIDTH + LISTDIVIDER + LEFTMARGIN, m_szPages.cy + m_nButtonHeight + BUTTONGAP*2 + 1 );
	CalcWindowRect( &rcWindow );
	SetWindowPos( &wndTop, 0, 0, rcWindow.Width(), rcWindow.Height(), SWP_NOMOVE|SWP_NOZORDER );

	CRect rcTree( LEFTMARGIN, 0, LISTWIDTH, m_szPages.cy + 1 );	// Skin.m_nBanner
	m_wndTree.MoveWindow( &rcTree );
	m_wndTree.SetBkColor( Colors.m_crDialogMenu );
	m_wndTree.SetTextColor( Colors.m_crDialogMenuText );

	CRect rcButton( m_szPages.cx + LISTWIDTH + LISTDIVIDER - BUTTONWIDTH - BUTTONGAP - 1,
		m_szPages.cy + BUTTONGAP + 1, BUTTONWIDTH, m_nButtonHeight );
	rcButton.right += rcButton.left;
	rcButton.bottom += rcButton.top;

	m_wndCancel.MoveWindow( &rcButton );
	rcButton.OffsetRect( - BUTTONWIDTH - BUTTONGAP, 0 );
	m_wndOK.MoveWindow( &rcButton );
	rcButton.OffsetRect( - BUTTONWIDTH - BUTTONGAP, 0 );
	m_wndApply.MoveWindow( &rcButton );

	CenterWindow();

	SetActivePage( m_pPage ? m_pPage : m_pFirst );

	return CSkinDialog::SkinMe( pszSkin, nIcon, bLanguage );
}

BOOL CSettingsSheet::CreatePage(const CRect& rc, CSettingsPage* pPage)
{
	if ( ! pPage->Create( rc, this ) )
		return FALSE;

	pPage->OnSkinChange();

	return TRUE;
}

//void CSettingsSheet::OnTreeExpanding(NMHDR* /*pNotify*/, LRESULT *pResult)
//{
//	*pResult = TRUE;
//}

void CSettingsSheet::OnSelectPage(NMHDR* pNotify, LRESULT *pResult)
{
	*pResult = NULL;

	if ( ( ((NM_TREEVIEW*)pNotify )->itemNew.state & TVIS_SELECTED ) == 0 ) return;
	CSettingsPage* pPage = (CSettingsPage*)m_wndTree.GetItemData( m_wndTree.GetSelectedItem() );
	if ( pPage == NULL || pPage == m_pPage ) return;

	SetActivePage( pPage );
}

void CSettingsSheet::OnPaint()
{
	CPaintDC dc( this );
	DoPaint( dc );
}

void CSettingsSheet::DoPaint(CDC& dc)
{
	// Draw Splitter Bar
	CRect rc( LEFTMARGIN, Skin.m_nBanner, 0, 0 );

	rc.left		+= LISTWIDTH;
	rc.right	= rc.left + LISTDIVIDER;
	rc.bottom	= rc.top  + m_szPages.cy + 1;

	dc.FillSolidRect( rc.left, rc.top, 1, rc.Height(), Colors.m_crResizebarEdge );
	dc.FillSolidRect( rc.left + 1, rc.top, 1, rc.Height(), Colors.m_crResizebarHighlight );
	dc.FillSolidRect( rc.right - 1, rc.top, 1, rc.Height(), Colors.m_crResizebarShadow );
	dc.FillSolidRect( rc.left + 2, rc.top, rc.Width() - 3, rc.Height(), Colors.m_crResizebarFace );

	GetClientRect( &rc );
	rc.top = rc.bottom - ( m_nButtonHeight + BUTTONGAP*2 );

	dc.FillSolidRect( rc.left, rc.top, rc.Width(), 1, Colors.m_crSysBtnFace );
	dc.FillSolidRect( rc.left, rc.top + 1, rc.Width(), 1, Colors.m_crSys3DHighlight );
}

void CSettingsSheet::OnOK()
{
	if ( m_pPage && ! m_pPage->OnKillActive() ) return;

	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = GetPage( nPage );
		if ( pPage->m_hWnd ) pPage->OnOK();
	}

	EndDialog( IDOK );
}

void CSettingsSheet::OnCancel()
{
	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = GetPage( nPage );
		if ( pPage->m_hWnd ) pPage->OnCancel();
	}

	EndDialog( IDCANCEL );
}

void CSettingsSheet::OnApply()
{
	if ( m_pPage && ! m_pPage->OnKillActive() ) return;

	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = GetPage( nPage );
		if ( pPage->m_hWnd && ! pPage->OnApply() ) return;
	}

	SetModified( FALSE );
}

BOOL CSettingsSheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if ( LOWORD( wParam ) == IDOK )
	{
		OnOK();
		return TRUE;
	}
	if ( LOWORD( wParam ) == IDCANCEL )
	{
		OnCancel();
		return TRUE;
	}

	return CSkinDialog::OnCommand(wParam, lParam);
}
