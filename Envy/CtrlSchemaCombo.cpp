//
// CtrlSchemaCombo.cpp
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
#include "CtrlSchemaCombo.h"

#include "Schema.h"
#include "SchemaCache.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CSchemaCombo, CComboBox)
	ON_MESSAGE(WM_CTLCOLORLISTBOX, OnCtlColorListBox)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropDown)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSchemaCombo construction

CSchemaCombo::CSchemaCombo()
	: m_pWndProc	( NULL )
	, m_hListBox	( 0 )
{
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCombo operations

BOOL CSchemaCombo::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CComboBox::Create( dwStyle|WS_CHILD|WS_TABSTOP|WS_VSCROLL|
		CBS_DROPDOWNLIST|CBS_OWNERDRAWVARIABLE|CBS_HASSTRINGS|CBS_SORT, rect, pParentWnd, nID );
}

void CSchemaCombo::SetEmptyString(UINT nID)
{
	LoadString( m_sNoSchemaText, nID );
}

void CSchemaCombo::Load(LPCTSTR pszSelectURI, int nType, int nAvailability, BOOL bReset)
{
	if ( ( GetStyle() & CBS_OWNERDRAWVARIABLE ) == 0 )
		ModifyStyle( 0, CBS_OWNERDRAWVARIABLE|CBS_HASSTRINGS );

	SetExtendedUI();

	m_nType			= nType;
	m_nAvailability	= nAvailability;

	if ( bReset ) ResetContent();

	if ( bReset && m_sNoSchemaText.GetLength() )
	{
		SetItemData( AddString( L" " ), 0 );
		SetCurSel( 0 );
	}

	for ( POSITION pos = SchemaCache.GetIterator() ; pos ; )
	{
		CSchemaPtr pSchema = SchemaCache.GetNext( pos );

		BOOL bSelected = pSchema->CheckURI( pszSelectURI );

		if ( ! bReset )
		{
			int nIndex = FindSchema( pSchema );

			if ( nIndex >= 0 )
			{
				if ( bSelected ) SetCurSel( nIndex );
				continue;
			}
		}

		if ( ( bSelected || pSchema->m_nType == nType || nType == -1 ) &&
			 ( bSelected || pSchema->m_nAvailability <= nAvailability ) )
		{
			int nIndex = AddString( pSchema->m_sTitle );
			SetItemData( nIndex, (LPARAM)pSchema );

			if ( bSelected ) SetCurSel( nIndex );
		}
	}

	if ( bReset && nAvailability < CSchema::saMax )
		SetItemData( AddString( L"ZZZ" ), 0 );
}

void CSchemaCombo::Select(LPCTSTR pszURI)
{
	for ( int nItem = 0 ; nItem < GetCount() ; nItem++ )
	{
		CSchemaPtr pSchema = (CSchemaPtr)GetItemData( nItem );

		if ( pSchema != NULL && pSchema->CheckURI( pszURI ) )
		{
			SetCurSel( nItem );
			return;
		}
	}

	SetCurSel( 0 );
}

void CSchemaCombo::Select(CSchemaPtr pSelect)
{
	for ( int nItem = 0 ; nItem < GetCount() ; nItem++ )
	{
		CSchemaPtr pSchema = (CSchemaPtr)GetItemData( nItem );

		if ( pSchema == pSelect )
		{
			SetCurSel( nItem );
			return;
		}
	}

	SetCurSel( 0 );
}

CSchemaPtr CSchemaCombo::GetSelected() const
{
	int nSel = GetCurSel();
	if ( nSel < 0 ) return NULL;
	return (CSchemaPtr)GetItemData( nSel );
}

CString CSchemaCombo::GetSelectedURI() const
{
	CString str;
	int nSel = GetCurSel();
	if ( nSel < 0 ) return str;
	if ( CSchemaPtr pSchema = (CSchemaPtr)GetItemData( nSel ) )
		return pSchema->GetURI();
	return str;
}

int CSchemaCombo::FindSchema(CSchemaPtr pSchema)
{
	for ( int nItem = 0 ; nItem < GetCount() ; nItem++ )
	{
		if ( (CSchemaPtr)GetItemData( nItem ) == pSchema ) return nItem;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCombo message handlers

LRESULT CSchemaCombo::OnCtlColorListBox(WPARAM wParam, LPARAM lParam)
{
	if ( m_hListBox == 0 )
	{
		if ( lParam != 0 && m_hWnd != (HWND)lParam )
		{
			m_hListBox = (HWND)lParam;

			m_pWndProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr( m_hListBox, GWLP_WNDPROC );
			SetWindowLongPtr( m_hListBox, GWLP_USERDATA, (LONG_PTR)this );
			SetWindowLongPtr( m_hListBox, GWLP_WNDPROC, (LONG_PTR)&ListWndProc );

			::InvalidateRect( m_hListBox, NULL, TRUE );
		}
	}

	return DefWindowProc( WM_CTLCOLORLISTBOX, wParam, lParam );
}

void CSchemaCombo::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 1024;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CSchemaCombo::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CPoint pt( rcItem.left + 1, rcItem.top + 1 );
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );
	if ( Settings.General.LanguageRTL )
		SetLayout( dc.m_hDC, LAYOUT_RTL );

	dc.SetTextColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? Colors.m_crHiText : Colors.m_crDropdownText );

	CSchemaPtr pSchema = (CSchemaPtr)lpDrawItemStruct->itemData;

	if ( pSchema )	// Schema Option Item
	{
		//dc.FillSolidRect( &rcItem, GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED ) ? COLOR_HIGHLIGHT : COLOR_WINDOW ) );
		if ( IsWindowEnabled() )
		{
			if ( lpDrawItemStruct->itemState & ODS_SELECTED )
			{
				if ( Images.m_bmSelected.m_hObject )
					CoolInterface.DrawWatermark( &dc, &rcItem, &Images.m_bmSelected, FALSE ); 	// No overdraw
				else
					dc.FillSolidRect( &rcItem, Colors.m_crHighlight );
			}
			else
			{
				dc.FillSolidRect( &rcItem, Colors.m_crDropdownBox );
			}
		}
		else // Disabled
		{
			dc.FillSolidRect( &rcItem, GetBkColor(lpDrawItemStruct->hDC) );
		}

		dc.SetBkMode( TRANSPARENT );

		ShellIcons.Draw( &dc, pSchema->m_nIcon16, 16, pt.x, pt.y,
			CLR_NONE, ( lpDrawItemStruct->itemState & ODS_SELECTED ) );

		rcItem.left += 20;
		rcItem.right -= 2;

		CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_gdiFont );
		CString strURI = pSchema->GetURI();

		if ( dc.GetTextExtent( pSchema->m_sTitle + strURI ).cx > rcItem.Width() - 20
			 && strURI.GetLength() > 8 )
		{
			if ( LPCTSTR pszLeft = _tcschr( (LPCTSTR)strURI + 7, '/' ) )
			{
				int nRight = strURI.ReverseFind( L'/' );
				if ( nRight >= 0 )
				{
					int nLeft = static_cast< int >( pszLeft - (LPCTSTR)strURI );	// !!! (ToDo:?)
					strURI = strURI.Left( nLeft ) + L"/\x2026" + strURI.Mid( nRight );
				}
			}
		}

		if ( dc.GetTextExtent( pSchema->m_sTitle + strURI ).cx <= rcItem.Width() - 20 )
		{
			// COLORREF crBackup = dc.SetTextColor( GetSysColor( COLOR_GRAYTEXT ) );
			dc.DrawText( strURI, &rcItem, DT_SINGLELINE|DT_RIGHT|DT_VCENTER|DT_NOPREFIX );
			// dc.SetTextColor( crBackup );
		}

		dc.SelectObject( &theApp.m_gdiFontBold );
		dc.DrawText( pSchema->m_sTitle, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX );
		dc.SelectObject( pOldFont );
	}
	else if ( lpDrawItemStruct->itemID == 0 )	// Default Option Item
	{
		//dc.FillSolidRect( &rcItem, GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED ) ? COLOR_HIGHLIGHT : COLOR_WINDOW ) );
		if ( IsWindowEnabled() )
		{
			if ( lpDrawItemStruct->itemState & ODS_SELECTED )
			{
				if ( Images.m_bmSelected.m_hObject )
					CoolInterface.DrawWatermark( &dc, &rcItem, &Images.m_bmSelected, FALSE ); 	// No overdraw
				else
					dc.FillSolidRect( &rcItem, Colors.m_crHighlight );
			}
			else
			{
				dc.FillSolidRect( &rcItem, Colors.m_crDropdownBox );
			}
		}
		else // Disabled
		{
			dc.FillSolidRect( &rcItem, GetBkColor(lpDrawItemStruct->hDC) );
		}
		dc.SetBkMode( TRANSPARENT );

		CoolInterface.Draw( &dc, IDR_SEARCHFRAME, 16, pt.x, pt.y,
			CLR_NONE, ( lpDrawItemStruct->itemState & ODS_SELECTED ) );

		rcItem.left += 20;
		rcItem.right -= 2;

		CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_gdiFontBold );
		dc.DrawText( m_sNoSchemaText, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX );
		dc.SelectObject( pOldFont );
	}
	else	// Expand View Item
	{
		dc.Draw3dRect( &rcItem, Colors.m_crDropdownBox, Colors.m_crDropdownBox );
		rcItem.DeflateRect( 1, 1 );

		if ( lpDrawItemStruct->itemState & ODS_SELECTED )
		{
			if ( Images.m_bmSelected.m_hObject )
			{
				CoolInterface.DrawWatermark( &dc, &rcItem, &Images.m_bmSelected, FALSE );
			}
			else
			{
				dc.Draw3dRect( &rcItem, Colors.m_crBorder, Colors.m_crBorder );
				rcItem.DeflateRect( 1, 1 );
				dc.FillSolidRect( &rcItem, Colors.m_crBackSel );
			}
		}
		else
		{
			dc.FillSolidRect( &rcItem, Colors.m_crDropdownBox );	// GetSysColor( COLOR_BTNFACE ) ?
		}

		dc.SetBkMode( TRANSPARENT );

		pt = rcItem.CenterPoint();
		pt.x -= 8;
		pt.y -= 8;

		CoolInterface.Draw( &dc, IDI_CHEVRON, 16, pt.x, pt.y );
	}

	dc.Detach();
}

BOOL CSchemaCombo::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_SPACE || pMsg->wParam == VK_RETURN )
		{
			if ( GetDroppedState() )
			{
				if ( ! OnClickItem( GetCurSel(), TRUE ) )
					ShowDropDown( FALSE );

				return TRUE;
			}
			if ( pMsg->wParam == VK_SPACE )
			{
				ShowDropDown();
				return TRUE;
			}
		}
		else if ( pMsg->wParam == VK_DOWN )
		{
			if ( OnClickItem( GetCurSel() + 1, TRUE ) )
				return TRUE;
		}
	}

	return CComboBox::PreTranslateMessage( pMsg );
}

void CSchemaCombo::OnDropDown()
{
	m_sPreDrop = GetSelectedURI();

	RecalcDropWidth( this, 16 );
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCombo subclassed list box

LRESULT PASCAL CSchemaCombo::ListWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CSchemaCombo* pThis = (CSchemaCombo*)(LONG_PTR)GetWindowLongPtr( hWnd, GWLP_USERDATA );

	if ( pThis->m_nAvailability < CSchema::saMax &&
		 ( nMsg == WM_LBUTTONDOWN || nMsg == WM_LBUTTONUP ) )
	{
		//CPoint pt( LOWORD( lParam ), HIWORD( lParam ) );
		CPoint pt( lParam );
		CRect rcClient;

		::GetClientRect( hWnd, &rcClient );

		if ( rcClient.PtInRect( pt ) )
		{
			LRESULT nItemHeight = ::SendMessage( hWnd, LB_GETITEMHEIGHT, 0, 0 );
			LRESULT nTopIndex   = ::SendMessage( hWnd, LB_GETTOPINDEX, 0, 0 );
			int nIndex = static_cast< int >( nTopIndex + pt.y / nItemHeight );

			CRect rcItem;
			::SendMessage( hWnd, LB_GETITEMRECT, nIndex, (LPARAM)&rcItem );

			if ( rcItem.PtInRect( pt ) )
			{
				if ( pThis->OnClickItem( nIndex, nMsg == WM_LBUTTONDOWN ) )
				{
					if ( nMsg == WM_LBUTTONDOWN ) return 0;

					nIndex = pThis->GetCurSel();
					if ( nIndex >= 0 )
						CallWindowProc( pThis->m_pWndProc, hWnd, LB_SETCURSEL, nIndex, 0 );

					::GetWindowRect( hWnd, &rcClient );
					LRESULT nHeight = pThis->GetCount() * nItemHeight + 2;

					if ( rcClient.Height() < nHeight )
					{
						rcClient.bottom = min( LONG( GetSystemMetrics( SM_CYSCREEN ) - 1 ), rcClient.top + (LONG)nHeight );

						::MoveWindow( hWnd, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), TRUE );
					}

					return 0;
				}
			}
		}
	}

	return CallWindowProc( pThis->m_pWndProc, hWnd, nMsg, wParam, lParam );
}

BOOL CSchemaCombo::OnClickItem(int nItem, BOOL bDown)
{
	if ( nItem > 0 && GetItemData( nItem ) == 0 )
	{
		if ( bDown )
		{
			int nOldTop = GetTopIndex();
			int nDelta = ( nOldTop != CB_ERR && nItem > nOldTop ) ? ( nItem - nOldTop ) : 0;
			DeleteString( nItem );
			Load( m_sPreDrop, m_nType, CSchema::saMax, FALSE );
			int nCurSel = GetCurSel();
			if ( nCurSel != CB_ERR )
			{
				if ( nCurSel >= nDelta )
					SetTopIndex( nCurSel - nDelta );

				SetCurSel( nCurSel );
			}
		}
		return TRUE;
	}

	return FALSE;
}
