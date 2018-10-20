//
// DlgGraphItem.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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
#include "DlgGraphItem.h"
#include "GraphItem.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CGraphItemDlg, CSkinDialog)
	ON_WM_PAINT()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_BN_CLICKED(IDC_GRAPH_COLOR, OnGraphColor)
	ON_CBN_SELCHANGE(IDC_GRAPH_SOURCE, OnSelChangeGraphSource)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGraphItemDlg dialog

CGraphItemDlg::CGraphItemDlg(CWnd* pParent, CGraphItem* pItem) : CSkinDialog(CGraphItemDlg::IDD, pParent)
	, m_nMultiplier(1.0f)
{
	m_pItem = pItem;
}

void CGraphItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_GRAPH_UNITS, m_wndUnits);
	DDX_Control(pDX, IDC_GRAPH_SOURCE, m_wndSource);
	DDX_Control(pDX, IDC_GRAPH_REMOVE, m_wndRemove);
	DDX_Float(pDX, IDC_GRAPH_PARAM, m_nMultiplier);
	DDX_Control(pDX, IDC_GRAPH_COLOR_BOX, m_wndColorBox);
}

/////////////////////////////////////////////////////////////////////////////
// CGraphItemDlg message handlers

BOOL CGraphItemDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CGraphItemDlg", IDR_TRAFFICFRAME );

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 );
	m_gdiImageList.Add( theApp.LoadIcon( IDR_TRAFFICFRAME ) );

	for ( int nItem = 1; CGraphItem::m_pItemDesc[ nItem ].m_nCode; nItem++ )
	{
		const GRAPHITEM* pItem = &CGraphItem::m_pItemDesc[ nItem ];
		CString strItem;

		::Skin.LoadString( strItem, pItem->m_nStringID );
		int nIndex = m_wndSource.AddString( strItem );
		m_wndSource.SetItemData( nIndex, (LPARAM)pItem );

		if ( pItem->m_nCode == m_pItem->m_nCode )
			m_wndSource.SetCurSel( nIndex );
	}

	m_crColor = m_pItem->m_nColor;
	m_nMultiplier = m_pItem->m_nMultiplier;

	OnSelChangeGraphSource();

	return TRUE;
}

void CGraphItemDlg::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 1024;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CGraphItemDlg::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );

	dc.FillSolidRect( &rcItem,
		GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHT : COLOR_WINDOW ) );

	dc.SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	dc.SetBkMode( TRANSPARENT );

	CPoint pt( rcItem.left + 1, rcItem.top + 1 );

	ImageList_Draw( m_gdiImageList.GetSafeHandle(),
		0, dc.GetSafeHdc(), pt.x, pt.y,
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );

	rcItem.left += 20;
	rcItem.right -= 2;

	CString strText;
	m_wndSource.GetLBText( lpDrawItemStruct->itemID, strText );

	CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_gdiFont );
	dc.DrawText( strText, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX );
	dc.SelectObject( pOldFont );

	dc.Detach();
}

void CGraphItemDlg::OnSelChangeGraphSource()
{
	int nItem = m_wndSource.GetCurSel();
	if ( nItem < 0 ) return;

	GRAPHITEM* pItem = (GRAPHITEM*)m_wndSource.GetItemData( nItem );
	if ( ! pItem ) return;

	switch ( pItem->m_nUnits )
	{
	case 0:
		m_wndUnits.SetWindowText( L"Items" );
		break;
	case 1:
		m_wndUnits.SetWindowText( Settings.General.RatesInBytes ? L"Bytes per Second" : L"Bits per Second" );
		break;
	case 2:
		m_wndUnits.SetWindowText( L"Volume (B/KB/MB/GB/TB" );
		break;
	case 3:
		m_wndUnits.SetWindowText( L"Percentage (%)" );
		break;
	default:
		m_wndUnits.SetWindowText( L"" );
		break;
	}

	UpdateData( FALSE );

	m_wndOK.EnableWindow( TRUE );
}

void CGraphItemDlg::OnGraphColor()
{
	CColorDialog dlg( m_crColor, CC_ANYCOLOR|CC_SOLIDCOLOR, this );

	if ( dlg.DoModal() == IDOK )
	{
		m_crColor = dlg.GetColor();
		Invalidate();
	}
}

void CGraphItemDlg::OnPaint()
{
	CPaintDC dc( this );
	CRect rc;

	m_wndColorBox.GetWindowRect( &rc );
	ScreenToClient( &rc );

	dc.Draw3dRect( &rc, 0, 0 );
	rc.DeflateRect( 1, 1 );
	dc.FillSolidRect( &rc, 0 );
	dc.Draw3dRect( rc.left, ( rc.top + rc.bottom ) / 2 - 1, rc.Width(), 2, m_crColor, m_crColor );
}

void CGraphItemDlg::OnOK()
{
	int nItem = m_wndSource.GetCurSel();
	if ( nItem < 0 ) return;

	GRAPHITEM* pItem = (GRAPHITEM*)m_wndSource.GetItemData( nItem );
	if ( ! pItem ) return;

	UpdateData( TRUE );
	m_pItem->SetCode( pItem->m_nCode );
	m_pItem->m_nColor = m_crColor;
	m_pItem->m_nMultiplier = m_nMultiplier;

	CSkinDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CGraphItemDlg custom dialog data exchange

void PASCAL DDX_Float(CDataExchange* pDX, int nIDC, float& nValue)
{
	HWND hWndCtrl = pDX->PrepareCtrl( nIDC );
	_ASSERTE( hWndCtrl != NULL );

	CWnd* pWnd = CWnd::FromHandle( hWndCtrl );

	// Data from control
	if ( pDX->m_bSaveAndValidate )
	{
		nValue = 1.0f;
		CString str;
		pWnd->GetWindowText( str );
		if ( str.IsEmpty() ) return;

		float nNumber;
		if ( _stscanf( str, L"%f", &nNumber ) == 1 )
			nValue = nNumber;
	}
	else // Data to control
	{
		if ( nValue <= 0 )
			nValue = 1.0;

		CString str;
		str.Format( L"%f", nValue );
		pWnd->SetWindowText( (LPCTSTR)str );
	}
}
