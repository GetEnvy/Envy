//
// CtrlFontCombo.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2005-2007 and PeerProject 2008-2014
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
#include "CtrlFontCombo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define SPACING			10
#define SYMBOL_WIDTH	15

/////////////////////////////////////////////////////////////////////////////
// CFontCombo construction

CFontCombo::CFontCombo()
	: m_nFontHeight	( 16 )
{
	m_pImages.Create( IDB_FONT_SYMBOLS, SYMBOL_WIDTH, 2, RGB(255,255,255) );
}

//CFontCombo::~CFontCombo()
//{
//}

IMPLEMENT_DYNAMIC(CFontCombo, CComboBox)

BEGIN_MESSAGE_MAP(CFontCombo, CComboBox)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(OCM_DRAWITEM, &CFontCombo::OnOcmDrawItem)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, &CFontCombo::OnDropdown)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontCombo creation and initialization

BOOL CFontCombo::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= WS_CHILD|WS_VSCROLL|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|CBS_SORT;

	return CComboBox::Create( dwStyle, rect, pParentWnd, nID );
}

int CFontCombo::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CComboBox::OnCreate(lpCreateStruct) == -1 ) return -1;
	Initialize();

	return 0;
}

void CFontCombo::PreSubclassWindow()
{
	CComboBox::PreSubclassWindow();
	ModifyStyle( 0, CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|CBS_SORT );
	Initialize();
}

void CFontCombo::Initialize()
{
	CClientDC dc(this);

	ResetContent();
	DeleteAllFonts();

	LOGFONT lf = {};
	lf.lfCharSet = DEFAULT_CHARSET;
	EnumFontFamiliesEx( dc.m_hDC, &lf, (FONTENUMPROC)EnumFontProc, (LPARAM)this, 0 );

	SetCurSel( 0 );
}

BOOL CALLBACK CFontCombo::EnumFontProc(LPENUMLOGFONTEX lplf, NEWTEXTMETRICEX* lpntm, DWORD dwFontType, LPVOID lpData)
{
	CFontCombo *pThis = reinterpret_cast<CFontCombo*>(lpData);

	if ( lpntm->ntmTm.tmCharSet != OEM_CHARSET && lpntm->ntmTm.tmCharSet != SYMBOL_CHARSET &&
		 dwFontType != DEVICE_FONTTYPE && _tcsicmp( lplf->elfLogFont.lfFaceName, L"Small Fonts" ) != 0 )
	{
		int nFamily = lplf->elfLogFont.lfPitchAndFamily ? lplf->elfLogFont.lfPitchAndFamily >> 4 : 6;
		if ( nFamily < 4 )	// Don't use unknown, decorative and script fonts
		{
			// Filter out vertical fonts starting with @
			if ( lplf->elfLogFont.lfFaceName[ 0 ] != '@' && pThis->AddFont( lplf->elfLogFont.lfFaceName ) )
			{
				int nIndex = pThis->AddString( lplf->elfLogFont.lfFaceName );
				if ( nIndex == -1 ) return FALSE;
				if ( pThis->SetItemData( nIndex, dwFontType ) == 0 )
					return FALSE;
			}
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFontCombo message handlers

void CFontCombo::OnDestroy()
{
	DeleteAllFonts();
	CComboBox::OnDestroy();
}

void CFontCombo::OnDropdown()
{
	RecalcDropWidth( this, SYMBOL_WIDTH * 2 );
}

LRESULT CFontCombo::OnOcmDrawItem(WPARAM /*wParam*/, LPARAM lParam)
{
	DrawItem( (LPDRAWITEMSTRUCT)lParam );

	return 1;
}

void CFontCombo::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	if ( lpDrawItemStruct->CtlType != ODT_COMBOBOX ) return;

	CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	CRect rcItem( &lpDrawItemStruct->rcItem );

	int nOldDC = pDC->SaveDC();

	if ( Settings.General.LanguageRTL )
		SetLayout( pDC->m_hDC, LAYOUT_RTL );

	CString strCurrentFont;
	GetLBText( lpDrawItemStruct->itemID, strCurrentFont );

	CFont* pFont = (CFont*)pDC->SelectObject( strCurrentFont == m_sSelectedFont ?
		&theApp.m_gdiFontBold : &theApp.m_gdiFont );
	pDC->SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED ) ?
		COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	if ( ! IsWindowEnabled() )
		pDC->FillSolidRect( &rcItem, GetBkColor( lpDrawItemStruct->hDC ) );
	else if ( lpDrawItemStruct->itemState & ODS_SELECTED )
		pDC->FillSolidRect( &rcItem, GetSysColor( COLOR_HIGHLIGHT ) );
	else
		pDC->FillSolidRect( &rcItem, GetSysColor( COLOR_WINDOW ) );

	pDC->SetBkMode( TRANSPARENT );

	DWORD dwData = GetItemData( lpDrawItemStruct->itemID );
	if ( dwData & TRUETYPE_FONTTYPE )
		m_pImages.Draw( pDC, 0, CPoint( rcItem.left + 5, rcItem.top + 4 ),
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );
	else
		m_pImages.Draw( pDC, 1, CPoint( rcItem.left + 5, rcItem.top + 4 ),
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );

	rcItem.left += SYMBOL_WIDTH;

	CFont* pFontValid;
	if ( m_pFonts.Lookup( strCurrentFont, (void*&)pFontValid ) == NULL ) return;

	CSize sz = pDC->GetTextExtent( strCurrentFont );
	int nPosY = ( rcItem.Height() - sz.cy ) / 2;
	pDC->TextOut( rcItem.left + SPACING, rcItem.top + nPosY, strCurrentFont );

	pDC->SelectObject( pFont );
	pDC->RestoreDC( nOldDC );
}

/////////////////////////////////////////////////////////////////////////////
// CFontCombo implementation

BOOL CFontCombo::AddFont(const CString& strFontName)
{
	CFont* pFont = NULL;

	// Sometimes font with the same name exists on the system, check it.
	if ( m_pFonts.Lookup( strFontName, (void*&)pFont ) != NULL )
		return FALSE;

	pFont = new CFont;

	if ( pFont->CreateFont( m_nFontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		Settings.Fonts.Quality, DEFAULT_PITCH, strFontName ) )
	{
		m_pFonts.SetAt( strFontName, pFont );
	}

	return TRUE;
}

void CFontCombo::SelectFont(const CString& strFontName)
{
	int nIndex = FindString( -1, strFontName );
	if ( nIndex != CB_ERR )
	{
		SetCurSel( nIndex );
		m_sSelectedFont = strFontName;
	}
	else
	{
		nIndex = FindString( -1, Settings.Fonts.DefaultFont );
		if ( nIndex != CB_ERR )
		{
			SetCurSel( nIndex );
			if ( m_sSelectedFont.IsEmpty() )
				m_sSelectedFont = Settings.Fonts.DefaultFont;
		}
		else
			SetCurSel( 0 );
	}
}

CString CFontCombo::GetSelectedFont() const
{
	int nIndex = GetCurSel();
	if ( nIndex == CB_ERR )
		return Settings.Fonts.DefaultFont;

	CString strFontName;
	GetLBText( nIndex, strFontName );
	return strFontName;
}

void CFontCombo::SetFontHeight(int nNewHeight, BOOL bReinitialize)
{
	if ( nNewHeight == m_nFontHeight )
		return;

	m_nFontHeight = nNewHeight;
	if ( bReinitialize )
		Initialize();
}

int CFontCombo::GetFontHeight() const
{
	return m_nFontHeight;
}

void CFontCombo::DeleteAllFonts()
{
	CString str;
	for ( POSITION pos = m_pFonts.GetStartPosition(); pos; )
	{
		CFont* pFont = NULL;
		m_pFonts.GetNextAssoc( pos, str, (void*&)pFont );
		if ( pFont != NULL ) delete pFont;
	}
	m_pFonts.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CFontCombo custom dialog data exchange

void PASCAL DDX_FontCombo(CDataExchange* pDX, int nIDC, CString& strFontName)
{
	HWND hWndCtrl = pDX->PrepareCtrl( nIDC );
	_ASSERTE( hWndCtrl != NULL );

	CFontCombo* pCombo = static_cast<CFontCombo*>(CWnd::FromHandle( hWndCtrl ));

	if ( pDX->m_bSaveAndValidate )	// Data from control
		strFontName = pCombo->m_sSelectedFont = pCombo->GetSelectedFont();
	else	// Data to control
		pCombo->SelectFont( strFontName );

	// Obsolete:
	//if ( pDX->m_bSaveAndValidate )
	//{
	//	// Data from control
	//	int nIndex = pCombo->GetCurSel();
	//	if ( nIndex != CB_ERR )
	//	{
	//		pCombo->GetLBText( nIndex, strFontName );
	//		pCombo->m_sSelectedFont = strFontName;
	//	}
	//	else
	//		strFontName = Settings.Fonts.DefaultFont;
	//}
	//else
	//{
	//	// Data to control
	//	int nIndex = pCombo->FindString( -1, strFontName );
	//	if ( nIndex != CB_ERR )
	//	{
	//		pCombo->SetCurSel( nIndex );
	//		if ( pCombo->m_sSelectedFont.IsEmpty() )
	//			pCombo->m_sSelectedFont = strFontName;
	//	}
	//	else
	//	{
	//		nIndex = pCombo->FindString( -1, Settings.Fonts.DefaultFont );
	//		if ( nIndex != CB_ERR )
	//		{
	//			pCombo->SetCurSel( nIndex );
	//			if ( pCombo->m_sSelectedFont.IsEmpty() )
	//				pCombo->m_sSelectedFont = Settings.Fonts.DefaultFont;
	//		}
	//		else
	//			pCombo->SetCurSel( 0 );
	//	}
	//}
}
