//
// CtrlHomeSearch.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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
#include "CtrlHomeSearch.h"

#include "Schema.h"
#include "SchemaCache.h"
#include "QuerySearch.h"
#include "WndSearch.h"
#include "Colors.h"
#include "CoolInterface.h"
#include "DlgNewSearch.h"
#include "DlgHelp.h"
#include "Security.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define SCHEMA_WIDTH	164
#define BUTTON_WIDTH	98
#define BUTTON_HEIGHT	24
#define BUTTON_GAP		12
#define ROW_GAP 		11


IMPLEMENT_DYNCREATE(CHomeSearchCtrl, CWnd)

BEGIN_MESSAGE_MAP(CHomeSearchCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_CBN_CLOSEUP(IDC_SEARCH_TEXT, &CHomeSearchCtrl::OnCloseUpText)
	ON_CBN_SELCHANGE(IDC_SEARCH_TEXT, &CHomeSearchCtrl::OnSelChangeText)
	ON_COMMAND(IDC_SEARCH_START, &CHomeSearchCtrl::OnSearchStart)
	ON_COMMAND(IDC_SEARCH_ADVANCED, &CHomeSearchCtrl::OnSearchAdvanced)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHomeSearchCtrl construction

CHomeSearchCtrl::CHomeSearchCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomeSearchCtrl message handlers

BOOL CHomeSearchCtrl::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
	{
		PostMessage( WM_COMMAND, MAKELONG( IDC_SEARCH_START, BN_CLICKED ), (LPARAM)m_wndSearch.GetSafeHwnd() );
		return TRUE;
	}
	return CWnd::PreTranslateMessage( pMsg );
}

BOOL CHomeSearchCtrl::Create(CWnd* pParentWnd, UINT nID)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::CreateEx( WS_EX_CONTROLPARENT, NULL, L"CHomeSearchCtrl",
		WS_CHILD|WS_CLIPCHILDREN, rect, pParentWnd, nID );
}

int CHomeSearchCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc( 0, 0, 0, 0 );

	if ( ! m_wndText.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|WS_VSCROLL|CBS_AUTOHSCROLL|CBS_DROPDOWN,
		rc, this, IDC_SEARCH_TEXT ) ) return -1;

	m_wndText.SetFont( &theApp.m_gdiFont );

	if ( ! m_wndSchema.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP, rc, this, IDC_SCHEMAS ) )
		return -1;

	//m_wndSchema.SetDroppedWidth( 200 );	// RecalcDropWidth()
	LoadString( m_wndSchema.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );
	m_wndSchema.Load( Settings.Search.LastSchemaURI );

	m_wndSearch.Create( rc, this, IDC_SEARCH_START, WS_TABSTOP | BS_DEFPUSHBUTTON );
	m_wndSearch.SetHandCursor( TRUE );

	m_wndAdvanced.Create( rc, this, IDC_SEARCH_ADVANCED, WS_TABSTOP );
	m_wndAdvanced.SetHandCursor( TRUE );

	OnSkinChange( Colors.m_crRichdocBack, Colors.m_crRichdocText );

	return 0;
}

void CHomeSearchCtrl::OnSkinChange(COLORREF crWindow, COLORREF crText)
{
	// Custom RichDoc values passed from CtrlHomeView
	m_crWindow = crWindow;
	m_crText = crText;

	// "Search"
	m_wndSearch.SetWindowText( LoadString( IDS_SEARCH_PANEL_START ) );
	m_wndSearch.SetCoolIcon( ID_SEARCH_SEARCH );

	// "Advanced..."
	m_wndAdvanced.SetWindowText( LoadString( IDS_SEARCH_PANEL_ADVANCED ) + L'\x2026' );
	m_wndAdvanced.SetCoolIcon( ID_SEARCH_DETAILS );

	LoadString( m_wndSchema.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );

	FillHistory();
}

void CHomeSearchCtrl::FillHistory()
{
	m_wndText.ResetContent();

	// Load all
	for ( int i = 0; ; i++ )
	{
		CString strEntry;
		strEntry.Format( L"Search.%.2i", i + 1 );
		CString strValue( theApp.GetProfileString( L"Search", strEntry ) );
		if ( strValue.IsEmpty() )
			break;
		int lf = strValue.Find( L'\n' );
		int nIndex = m_wndText.InsertString( i,
			( lf != -1 ) ? strValue.Left( lf ) : strValue );
		CSchemaPtr pSchema = ( lf != -1 ) ? SchemaCache.Get( strValue.Mid( lf + 1 ) ) : NULL;
		m_wndText.SetItemData( nIndex, (DWORD_PTR)pSchema );
	}

	m_wndText.SetItemData( m_wndText.AddString( LoadString( IDS_SEARCH_PAD_CLEAR_HISTORY ) ), 0 );
}

void CHomeSearchCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	CRect rcClient( 0, 0, cx, cy );
	CRect rcItem;

	rcClient.DeflateRect( 1, 1 );

	rcClient.top += 18;		// Caption text

	// Text Area
	rcItem.SetRect( rcClient.left, rcClient.top + 2, rcClient.right - BUTTON_WIDTH - BUTTON_GAP, rcClient.top + 256 );	// Dropdown
	m_wndText.MoveWindow( &rcItem );

	// Search Button
	rcItem.SetRect( rcClient.right - BUTTON_WIDTH, rcClient.top, rcClient.right, rcClient.top + BUTTON_HEIGHT );
	m_wndSearch.MoveWindow( &rcItem );

	rcClient.top += BUTTON_HEIGHT + ROW_GAP;

	// Filetype Menu
	rcItem.SetRect( rcClient.right - BUTTON_WIDTH - BUTTON_GAP - SCHEMA_WIDTH, rcClient.top, rcClient.right - BUTTON_WIDTH - BUTTON_GAP, rcClient.top + 256 );	// Dropdown
	rcItem.left = max( rcItem.left, rcClient.left );
	m_wndSchema.MoveWindow( &rcItem );

	// Search Tab Button  (Advanced...)
	rcItem.SetRect( rcClient.right - BUTTON_WIDTH, rcClient.top, rcClient.right, rcClient.top + BUTTON_HEIGHT );
	m_wndAdvanced.MoveWindow( &rcItem );
}

void CHomeSearchCtrl::OnPaint()
{
	CRect rcClient, rcItem;
	CString str;

	CPaintDC dc( this );

	GetClientRect( &rcClient );
	rcClient.DeflateRect( 1, 1 );

	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntBold );
	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( m_crWindow );
	dc.SetTextColor( m_crText );

	LoadString( str, IDS_SEARCH_PAD_WORDS );
	rcItem.SetRect( rcClient.left, rcClient.top, rcClient.right, rcClient.top + 16 );
	dc.ExtTextOut( rcItem.left + 2, rcItem.top + 2, ETO_CLIPPED|ETO_OPAQUE, &rcItem, str, NULL );
	dc.ExcludeClipRect( &rcItem );

	rcClient.top += 18;
	rcClient.top += BUTTON_HEIGHT + ROW_GAP;

	rcItem.SetRect( rcClient.left, rcClient.top,
		rcClient.right - BUTTON_WIDTH - BUTTON_GAP - SCHEMA_WIDTH - 10, rcClient.top + BUTTON_HEIGHT - 2 );

	LoadString( str, IDS_SEARCH_PAD_TYPE );
	CSize sz = dc.GetTextExtent( str );
	dc.ExtTextOut( rcItem.right - sz.cx, ( rcItem.top + rcItem.bottom ) / 2 - sz.cy / 2,
		ETO_CLIPPED|ETO_OPAQUE, &rcItem, str, NULL );
	dc.ExcludeClipRect( &rcItem );

	dc.SelectObject( pOldFont );
	GetClientRect( &rcClient );
	dc.FillSolidRect( &rcClient, m_crWindow );
}

void CHomeSearchCtrl::OnCloseUpText()
{
	const int nSel = m_wndText.GetCurSel();
	if ( nSel < 0 ) return;

	if ( nSel == m_wndText.GetCount() - 1 )
	{
		m_wndText.SetWindowText( L"" );

		// Delete all
		Settings.ClearSearches();

		m_wndSchema.Select( (CSchemaPtr)NULL );
		FillHistory();
	}
	else
	{
		m_wndSchema.Select( (CSchemaPtr)m_wndText.GetItemData( nSel ) );
	}
}

void CHomeSearchCtrl::OnSelChangeText()
{
	OnCloseUpText();
}

void CHomeSearchCtrl::Search(bool bAutostart)
{
	CString strText;

	m_wndText.GetWindowText( strText );
	strText.Trim();

	if ( _tcscmp( strText, LoadString( IDS_SEARCH_PAD_CLEAR_HISTORY ) ) == 0 )
		return;

	// Check if user pasted download link to search input box
	if ( theApp.OpenURL( strText, TRUE ) )
	{
		m_wndText.SetWindowText( L"" );
		return;
	}

	CString strURI;

	CSchemaPtr pSchema = m_wndSchema.GetSelected();
	if ( pSchema != NULL ) strURI = pSchema->GetURI();

	Settings.Search.LastSchemaURI = strURI;

	CQuerySearchPtr pSearch = new CQuerySearch();
	pSearch->SetSearch( strText );
	pSearch->m_bAutostart	= bAutostart;
	pSearch->m_pSchema		= pSchema;
	BOOL bValid = pSearch->CheckValid( false );
	if ( ! bValid && bAutostart )
	{
		// Invalid search, open help window
		CQuerySearch::SearchHelp();
	}
	else if ( AdultFilter.IsSearchFiltered( pSearch->m_sSearch ) && bAutostart )
	{
		// Adult search blocked, open help window
		CHelpDlg::Show( L"SearchHelp.AdultSearch" );
	}
	else
	{
		if ( bValid )
		{
			// Load all
			CString strEntry;
			CStringList oList;
			for ( int i = 0; ; i++ )
			{
				strEntry.Format( L"Search.%.2i", i + 1 );
				CString strValue( theApp.GetProfileString( L"Search", strEntry ) );
				if ( strValue.IsEmpty() )
					break;
				int lf = strValue.Find( L'\n' );
				if ( strText.CompareNoCase( ( lf != -1 ) ? strValue.Left( lf ) : strValue ) )
					oList.AddTail( strValue );
			}

			// Cut to 200 items
			while ( oList.GetCount() >= 200 )	// ToDo: Setting?
				oList.RemoveTail();

			// New one (at top)
			oList.AddHead( strURI.IsEmpty() ? strText : ( strText + L'\n' + strURI ) );

			// Save list
			POSITION pos = oList.GetHeadPosition();
			for ( int i = 0; pos; ++i )
			{
				strEntry.Format( L"Search.%.2i", i + 1 );
				theApp.WriteProfileString( L"Search", strEntry, oList.GetNext( pos ) );
			}

			FillHistory();
		}

		new CSearchWnd( pSearch );
	}

	m_wndText.SetWindowText( L"" );
}

void CHomeSearchCtrl::OnSearchStart()
{
	Search( true );
}

void CHomeSearchCtrl::OnSearchAdvanced()
{
//	Search( false );
	GetTopLevelParent()->PostMessage( WM_COMMAND, ID_TAB_SEARCH );
}

void CHomeSearchCtrl::Activate()
{
	FillHistory();
	m_wndText.SetFocus();
}
