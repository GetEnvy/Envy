//
// CtrlSearchPanel.cpp
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
#include "CtrlSearchPanel.h"
#include "WndSearch.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "ManagedSearch.h"
#include "QuerySearch.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Colors.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CSearchPanel, CTaskPanel)
BEGIN_MESSAGE_MAP(CSearchPanel, CTaskPanel)
	ON_WM_CREATE()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchInputBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchInputBox, CTaskBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, OnCloseUpSchemas)
	ON_COMMAND(IDC_SEARCH_START, OnSearchStart)
	ON_COMMAND(IDC_SEARCH_STOP, OnSearchStop)
	ON_COMMAND(IDC_SEARCH_PREFIX, OnSearchPrefix)
	ON_COMMAND(IDC_SEARCH_PREFIX_SHA1, OnSearchPrefixSHA1)
	ON_COMMAND(IDC_SEARCH_PREFIX_TIGER, OnSearchPrefixTiger)
	ON_COMMAND(IDC_SEARCH_PREFIX_SHA1_TIGER, OnSearchPrefixSHA1Tiger)
	ON_COMMAND(IDC_SEARCH_PREFIX_ED2K, OnSearchPrefixED2K)
	ON_COMMAND(IDC_SEARCH_PREFIX_BTH, OnSearchPrefixBTH)
	ON_COMMAND(IDC_SEARCH_PREFIX_MD5, OnSearchPrefixMD5)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchAdvancedBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchAdvancedBox, CTaskBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_SEARCH_GNUTELLA2, OnG2Clicked)
	ON_BN_CLICKED(IDC_SEARCH_GNUTELLA1, OnG1Clicked)
	ON_BN_CLICKED(IDC_SEARCH_EDONKEY, OnED2KClicked)
	ON_BN_CLICKED(IDC_SEARCH_DC, OnDCClicked)
	ON_MESSAGE(WM_CTLCOLORSTATIC, OnCtlColorStatic)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchSchemaBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchSchemaBox, CTaskBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchResultsBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchResultsBox, CTaskBox)
	ON_WM_PAINT()
END_MESSAGE_MAP()

#define BOX_MARGIN	6

/////////////////////////////////////////////////////////////////////////////
// CSearchPanel construction

CSearchPanel::CSearchPanel()
	: m_bSendSearch	( FALSE )
	, m_bAdvanced	( FALSE )
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchPanel message handlers

BOOL CSearchPanel::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CTaskPanel::Create( L"CSearchPanel", WS_VISIBLE, rect, pParentWnd, 0 );
}

int CSearchPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskPanel::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_bAdvanced = Settings.Search.AdvancedPanel && Settings.General.GUIMode != GUI_BASIC;

	// Set Box Heights & Caption Icons
	m_boxSearch.Create( this, 136, L"Search", IDR_SEARCHFRAME );
	m_boxSchema.Create( this, 0, L"Schema", IDR_SEARCHFRAME );
	m_boxAdvanced.Create( this, 92, L"Options", IDR_SEARCHFRAME );
	m_boxResults.Create(  this, 86, L"Results", IDR_SEARCHMONITORFRAME );

	// 1: Basic search box
	AddBox( &m_boxSearch );

	// 2: Optional Metadata
	AddBox( &m_boxSchema );
	SetStretchBox( &m_boxSchema );	// The metadata box varies in height to fill available space

	if ( m_bAdvanced )
	{
		// 3: Advanced size&network options
		AddBox( &m_boxAdvanced );

		// If the resolution is low, minimise the advanced box by default
		if ( GetSystemMetrics( SM_CYSCREEN ) < 1024 )
			m_boxAdvanced.Expand( FALSE );

		// 4: Results summary
		AddBox( &m_boxResults );
	}

	OnSkinChange();

	return 0;
}

void CSearchPanel::OnSkinChange()
{
	m_boxSearch.SetCaption( LoadString( IDS_SEARCH_PANEL_INPUT_CAPTION ) );
	m_boxResults.SetCaption( LoadString( IDS_SEARCH_PANEL_RESULTS_CAPTION ) );
	m_boxAdvanced.SetCaption( LoadString( IDS_SEARCH_PANEL_ADVANCED ) );

	SetWatermark( L"CSearchPanel" );
	SetFooter( L"CSearchPanel.Footer" );

	m_boxSearch.SetWatermark( L"CSearchInputBox" );
	m_boxSearch.SetCaptionmark( L"CSearchInputBox.Caption" );
	m_boxSearch.OnSkinChange();

	m_boxAdvanced.SetWatermark( L"CSearchAdvancedBox" );
	m_boxAdvanced.SetCaptionmark( L"CSearchAdvancedBox.Caption" );
	m_boxAdvanced.OnSkinChange();

	m_boxSchema.SetWatermark( L"CSearchSchemaBox" );
	m_boxSchema.SetCaptionmark( L"CSearchSchemaBox.Caption" );

	m_boxResults.SetWatermark( L"CSearchResultsBox" );
	m_boxResults.SetCaptionmark( L"CSearchResultsBox.Caption" );

	Invalidate();
}

void CSearchPanel::SetSearchFocus()
{
	m_boxSearch.m_wndSearch.SetFocus();
}

void CSearchPanel::ShowSearch(const CManagedSearch* pManaged)
{
	if ( ! pManaged )
	{
		OnSchemaChange();
		return;
	}

	CQuerySearchPtr pSearch = pManaged->GetSearch();
	if ( ! pSearch )
	{
		OnSchemaChange();
		return;
	}

	m_boxSearch.m_wndSearch.SetWindowText( pSearch->GetSearch() );

	m_boxSearch.m_wndSchemas.Select( pSearch->m_pSchema );

	if ( m_bAdvanced )
	{
		m_boxAdvanced.m_wndCheckBoxG2.SetCheck( pManaged->m_bAllowG2 ? BST_CHECKED : BST_UNCHECKED);
		m_boxAdvanced.m_wndCheckBoxG1.SetCheck( pManaged->m_bAllowG1 ? BST_CHECKED : BST_UNCHECKED );
		m_boxAdvanced.m_wndCheckBoxED2K.SetCheck( pManaged->m_bAllowED2K ? BST_CHECKED : BST_UNCHECKED );
		m_boxAdvanced.m_wndCheckBoxDC.SetCheck( pManaged->m_bAllowDC ? BST_CHECKED : BST_UNCHECKED );

		CString strSize;
		if ( pSearch->m_nMinSize > 0 && pSearch->m_nMinSize < SIZE_UNKNOWN )
			strSize = Settings.SmartVolume(pSearch->m_nMinSize, Bytes, true );
		else
			strSize.Empty();
		if ( m_boxAdvanced.m_wndSizeMin.m_hWnd != NULL )
			m_boxAdvanced.m_wndSizeMin.SetWindowText( strSize );

		if ( pSearch->m_nMaxSize > 0 && pSearch->m_nMaxSize < SIZE_UNKNOWN )
			strSize = Settings.SmartVolume( pSearch->m_nMaxSize, Bytes, true );
		else
			strSize.Empty();
		if ( m_boxAdvanced.m_wndSizeMax.m_hWnd != NULL )
			m_boxAdvanced.m_wndSizeMax.SetWindowText( strSize );
	}

	OnSchemaChange();

	if ( pSearch->m_pXML != NULL )
		m_boxSchema.m_wndSchema.UpdateData( pSearch->m_pXML->GetFirstElement(), FALSE );
}

void CSearchPanel::ShowStatus(BOOL bStarted, BOOL bSearching, DWORD nHubs, DWORD nLeaves, DWORD nFiles, DWORD nHits, DWORD nBadHits)
{
	CString strCaption;
	LoadString( strCaption, bStarted ?
		( bSearching ? IDS_SEARCH_PANEL_SEARCHING : IDS_SEARCH_PANEL_MORE ) : IDS_SEARCH_PANEL_START );
	m_boxSearch.m_wndStart.SetText( strCaption );

	LoadString( strCaption, bStarted ? IDS_SEARCH_PANEL_STOP : IDS_SEARCH_PANEL_CLEAR );
	m_boxSearch.m_wndStop.SetText( strCaption );

	m_boxSearch.m_wndStart.EnableWindow( ! bStarted );
	m_boxSearch.m_wndPrefix.EnableWindow( ! bStarted );
	m_boxSearch.m_wndPrefix.ShowWindow( Settings.General.GUIMode != GUI_BASIC );

	m_boxResults.Update( bStarted, nHubs, nLeaves, nFiles, nHits, nBadHits );
}

void CSearchPanel::OnSchemaChange()
{
	CSchemaPtr pSchema = m_boxSearch.m_wndSchemas.GetSelected();

	m_boxSchema.m_wndSchema.SetSchema( pSchema, TRUE );
	m_boxSchema.SetSize( pSchema != NULL ? 1 : 0 );

	if ( pSchema != NULL )
	{
		HICON hIcon = ShellIcons.ExtractIcon( pSchema->m_nIcon16, 16 );
		m_boxSchema.SetIcon( hIcon );
		CString strTitle = pSchema->m_sTitle;
		int nPos = strTitle.Find( L':' );
		if ( nPos > 0 ) strTitle = strTitle.Mid( nPos + 1 );
		m_boxSchema.SetCaption( strTitle );
	}

	CBaseMatchWnd* pMainSearchFrame = static_cast< CBaseMatchWnd* >(GetParent());
	if ( pMainSearchFrame )
	{
		CList< CSchemaMember* > pColumns;

		if ( pSchema )
		{
			CString strMembers = pSchema->m_sDefaultColumns;
			for ( POSITION pos = pSchema->GetMemberIterator(); pos; )
			{
				CSchemaMember* pMember = pSchema->GetNextMember( pos );

				if ( strMembers.Find( L"|" + pMember->m_sName + L"|" ) >= 0 )
					pColumns.AddTail( pMember );
			}
		}

		pMainSearchFrame->m_wndList.SelectSchema( pSchema, &pColumns );
	}
}

CSearchPtr CSearchPanel::GetSearch()
{
	CSearchPtr pManaged( new CManagedSearch() );
	CQuerySearchPtr pSearch = pManaged->GetSearch();

	CString strSearch;
	m_boxSearch.m_wndSearch.GetWindowText( strSearch );
	strSearch.Trim();

	pSearch->SetSearch( strSearch );

	if ( CSchemaPtr pSchema = m_boxSearch.m_wndSchemas.GetSelected() )
	{
		pSearch->m_pSchema	= pSchema;
		pSearch->m_pXML		= pSchema->Instantiate();

		m_boxSchema.m_wndSchema.UpdateData(
			pSearch->m_pXML->AddElement( pSchema->m_sSingular ), TRUE );

		Settings.Search.LastSchemaURI = pSchema->GetURI();
	}
	else
	{
		Settings.Search.LastSchemaURI.Empty();
	}

	if ( m_bAdvanced )
	{
		pManaged->m_bAllowG2	= m_boxAdvanced.m_wndCheckBoxG2.GetCheck();
		pManaged->m_bAllowG1	= m_boxAdvanced.m_wndCheckBoxG1.GetCheck();
		pManaged->m_bAllowED2K	= m_boxAdvanced.m_wndCheckBoxED2K.GetCheck();
		pManaged->m_bAllowDC	= m_boxAdvanced.m_wndCheckBoxDC.GetCheck();

		if ( ! pManaged->m_bAllowG2 &&
			 ! pManaged->m_bAllowG1 &&
			 ! pManaged->m_bAllowED2K &&
			 ! pManaged->m_bAllowDC )
		{
			m_boxAdvanced.m_wndCheckBoxG2.SetCheck( BST_CHECKED );
			m_boxAdvanced.m_wndCheckBoxG1.SetCheck( BST_CHECKED );
			m_boxAdvanced.m_wndCheckBoxED2K.SetCheck( BST_CHECKED );
			m_boxAdvanced.m_wndCheckBoxDC.SetCheck( BST_CHECKED );
			pManaged->m_bAllowG2	= TRUE;
			pManaged->m_bAllowG1	= TRUE;
			pManaged->m_bAllowED2K	= TRUE;
			pManaged->m_bAllowDC	= TRUE;
		}

		if ( m_boxAdvanced.m_wndSizeMin.m_hWnd != NULL )
		{
			CString strWindowValue;

			m_boxAdvanced.m_wndSizeMin.GetWindowText( strWindowValue );
			if ( strWindowValue.IsEmpty() || _tcsicmp( strWindowValue, L"any" ) == 0 )
			{
				pSearch->m_nMinSize = 0;
			}
			else
			{
				if ( ! _tcsstr( strWindowValue, L"B" ) && ! _tcsstr( strWindowValue, L"b" ) )
					strWindowValue += L"B";
				pSearch->m_nMinSize = Settings.ParseVolume( strWindowValue );
			}

			m_boxAdvanced.m_wndSizeMax.GetWindowText( strWindowValue );
			if ( strWindowValue.IsEmpty() || _tcsicmp( strWindowValue, L"any" ) == 0 || _tcsicmp( strWindowValue, L"max" ) == 0 )
			{
				pSearch->m_nMaxSize = SIZE_UNKNOWN;
			}
			else
			{
				if ( !_tcsstr( strWindowValue, L"B" ) && !_tcsstr( strWindowValue, L"b" ) )
					strWindowValue += L"B";
				pSearch->m_nMaxSize = Settings.ParseVolume( strWindowValue );
			}

			// Check it wasn't invalid
			if ( pSearch->m_nMinSize > pSearch->m_nMaxSize )
				pSearch->m_nMaxSize = SIZE_UNKNOWN;
		}
	}

	pSearch->PrepareCheck();

	return pManaged;
}

void CSearchPanel::ExecuteSearch()
{
	m_bSendSearch = TRUE;
	GetParent()->SendMessage( WM_COMMAND, ID_SEARCH_SEARCH );
	m_bSendSearch = FALSE;
}

BOOL CSearchPanel::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_RETURN )
		{
			ExecuteSearch();
			return TRUE;
		}
	}

	return CTaskPanel::PreTranslateMessage( pMsg );
}

void CSearchPanel::Enable()
{
	m_boxSearch.m_wndSearch.EnableWindow( TRUE );
	m_boxSearch.m_wndSchemas.EnableWindow( TRUE );
	m_boxSchema.m_wndSchema.Enable();

	m_boxAdvanced.m_wndSizeMin.EnableWindow( TRUE );
	m_boxAdvanced.m_wndSizeMax.EnableWindow( TRUE );
	m_boxAdvanced.m_wndCheckBoxG2.EnableWindow( Settings.Gnutella2.Enabled );
	m_boxAdvanced.m_wndCheckBoxG1.EnableWindow( Settings.Gnutella1.Enabled );
	m_boxAdvanced.m_wndCheckBoxED2K.EnableWindow( Settings.eDonkey.Enabled );
	m_boxAdvanced.m_wndCheckBoxDC.EnableWindow( Settings.DC.Enabled );
}

void CSearchPanel::Disable()
{
	m_boxSearch.m_wndSearch.EnableWindow( FALSE );
	m_boxSearch.m_wndSchemas.EnableWindow( FALSE );
	m_boxSchema.m_wndSchema.Disable();

	m_boxAdvanced.m_wndSizeMin.EnableWindow( FALSE );
	m_boxAdvanced.m_wndSizeMax.EnableWindow( FALSE );
	m_boxAdvanced.m_wndCheckBoxG2.EnableWindow( FALSE );
	m_boxAdvanced.m_wndCheckBoxG1.EnableWindow( FALSE );
	m_boxAdvanced.m_wndCheckBoxED2K.EnableWindow( FALSE );
	m_boxAdvanced.m_wndCheckBoxDC.EnableWindow( FALSE );
}


/////////////////////////////////////////////////////////////////////////////
// CSearchInputBox construction

CSearchInputBox::CSearchInputBox()
{
}

CSearchInputBox::~CSearchInputBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchInputBox message handlers

int CSearchInputBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc( 0, 0, 0, 0 );

	if ( ! m_wndSearch.Create( ES_AUTOHSCROLL | WS_TABSTOP | WS_GROUP, rc,
		this, IDC_SEARCH, L"Search", L"Search.%.2i" ) ) return -1;

	m_wndSearch.SetFont( &theApp.m_gdiFont );
	m_wndSearch.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );
	//m_wndSearch.SetRegistryKey( L"Search", L"Search.%.2i" );

	if ( ! m_wndSchemas.Create( WS_TABSTOP, rc, this, IDC_SCHEMAS ) ) return -1;

	//m_wndSchemas.SetDroppedWidth( 200 );	// RecalcDropWidth()
	LoadString( m_wndSchemas.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );
	m_wndSchemas.Load( Settings.Search.LastSchemaURI );

	m_wndStart.Create( rc, this, IDC_SEARCH_START, WS_TABSTOP | BS_DEFPUSHBUTTON );
	m_wndStart.SetHandCursor( TRUE );

	m_wndStop.Create( rc, this, IDC_SEARCH_STOP, WS_TABSTOP );
	m_wndStop.SetHandCursor( TRUE );

	m_wndPrefix.Create( rc, this, IDC_SEARCH_PREFIX );
	m_wndPrefix.SetHandCursor( TRUE );

	OnSkinChange();

	SetPrimary( TRUE );

	return 0;
}

void CSearchInputBox::OnSkinChange()
{
	CString strCaption;
	CSearchWnd* pwndSearch = static_cast< CSearchWnd* >( GetParent()->GetParent() );
	BOOL bStarted = ! pwndSearch->IsPaused();
	BOOL bSearching = ! pwndSearch->IsWaitMore();

	LoadString( strCaption, bStarted ?
		( bSearching? IDS_SEARCH_PANEL_SEARCHING : IDS_SEARCH_PANEL_MORE ) : IDS_SEARCH_PANEL_START );
	m_wndStart.SetWindowText( strCaption );
	m_wndStart.SetCoolIcon( ID_SEARCH_SEARCH, FALSE );

	LoadString( strCaption, bStarted ? IDS_SEARCH_PANEL_STOP : IDS_SEARCH_PANEL_CLEAR );
	m_wndStop.SetWindowText( strCaption );
	m_wndStop.SetCoolIcon( ID_SEARCH_STOP, FALSE );

	m_wndPrefix.SetCoolIcon( IDI_HASH );
}

void CSearchInputBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );

	HDWP hDWP = BeginDeferWindowPos( 4 );

	const int width = ( cx - BOX_MARGIN * 3 ) / 2;	// Equal button size

	DeferWindowPos( hDWP, m_wndSearch, NULL,
		BOX_MARGIN, 25, cx - BOX_MARGIN * 2, 20,	// Search Bar
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndSchemas, NULL,
		BOX_MARGIN, 68, cx - BOX_MARGIN * 2, 256,	// Schema Bar
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndStart, NULL,
		BOX_MARGIN, 102, width, 24,					// Search Button
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndStop, NULL,
		BOX_MARGIN * 2 + width, 102, width, 24, 	// Cancel Button
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndPrefix, NULL,
		cx - BOX_MARGIN - 8, 8, 14, 16, 			// Hash Icon
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );

	EndDeferWindowPos( hDWP );
}

void CSearchInputBox::OnPaint()
{
	CPaintDC dc( this );
	CRect rc, rct;
	CString str;

	UINT nFlags = ETO_CLIPPED;
	CDC* pDC = &dc;

	GetClientRect( &rc );

	if ( m_bmWatermark.m_hObject != NULL )
	{
		CSize size = rc.Size();
		pDC = CoolInterface.GetBuffer( dc, size );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmWatermark );
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		pDC->SetBkMode( OPAQUE );
		nFlags |= ETO_OPAQUE;
	}

	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );

	pDC->SetTextColor( Colors.m_crTaskBoxText );	// Was 0
	pDC->SetBkColor( Colors.m_crTaskBoxClient );

	LoadString( str, IDS_SEARCH_PANEL_INPUT_1 );	// "Type Search Here"
	rct.SetRect( BOX_MARGIN + 1, BOX_MARGIN, rc.right - BOX_MARGIN - 8, BOX_MARGIN + 16 );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, str, NULL );
	pDC->ExcludeClipRect( &rct );

	LoadString( str, IDS_SEARCH_PANEL_INPUT_2 );	// "Type of File"
	rct.OffsetRect( 0, 44 );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, str, NULL );
	pDC->ExcludeClipRect( &rct );

	pDC->SelectObject( pOldFont );

	if ( pDC != &dc )
	{
		dc.BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );
		pDC->SelectClipRgn( NULL );
	}
	else
	{
		pDC->FillSolidRect( &rc, Colors.m_crTaskBoxClient );
	}
}

void CSearchInputBox::OnSelChangeSchemas()
{
	CSearchPanel* pPanel = (CSearchPanel*)GetPanel();
	pPanel->OnSchemaChange();
}

void CSearchInputBox::OnCloseUpSchemas()
{
}

void CSearchInputBox::OnSearchStart()
{
	CSearchPanel* pPanel = (CSearchPanel*)GetPanel();
	pPanel->ExecuteSearch();
}

void CSearchInputBox::OnSearchStop()
{
	CString strCaption, strTest;

	LoadString( strTest, IDS_SEARCH_PANEL_CLEAR );
	m_wndStop.GetWindowText( strCaption );

	CWnd* pTarget = GetPanel()->GetParent();

	if ( strCaption == strTest )
		pTarget->PostMessage( WM_COMMAND, ID_SEARCH_CLEAR );
	else
		pTarget->PostMessage( WM_COMMAND, ID_SEARCH_STOP );
}

void CSearchInputBox::OnSearchPrefix()
{
	if ( ! m_wndSearch.IsWindowEnabled() )
		return;

	CMenu mnuPopup;
	mnuPopup.CreatePopupMenu();
	mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_SHA1, L"SHA1" );
	mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_TIGER, L"Tiger" );
	mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_SHA1_TIGER, L"SHA1 + Tiger" );
	mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_ED2K, L"ED2K" );
	mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_MD5, L"MD5" );
	mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_BTH, L"BitTorrent" );

	// ToDo: Fix skinned menu
	//CCoolMenu* pCoolMenu = new CCoolMenu();
	//pCoolMenu->AddMenu( &mnuPopup );
	//pCoolMenu->SetWatermark( Skin.GetWatermark( L"CCoolMenu" ) );

	CPoint pt;
	::GetCursorPos( &pt );
	mnuPopup.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this );

	//delete pCoolMenu;
}

void CSearchInputBox::OnSearchPrefixSHA1()
{
	CString strInput;
	m_wndSearch.GetWindowText( strInput );
	strInput.Trim();

	CString strSearch = Hashes::Sha1Hash::urns[ 0 ].signature;
	Hashes::Sha1Hash oSHA1;
	if ( oSHA1.fromUrn( strInput ) || oSHA1.fromUrn< Hashes::base16Encoding >( strInput ) ||
		 oSHA1.fromString( strInput ) || oSHA1.fromString< Hashes::base16Encoding >( strInput ) )
		strSearch += oSHA1.toString();
	else
		strSearch += L"[SHA1]";

	m_wndSearch.SetWindowText( strSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( (int)Hashes::Sha1Hash::urns[ 0 ].signatureLength, -1 );
}

void CSearchInputBox::OnSearchPrefixTiger()
{
	CString strInput;
	m_wndSearch.GetWindowText( strInput );
	strInput.Trim();

	CString strSearch = Hashes::TigerHash::urns[ 0 ].signature;
	Hashes::TigerHash oTiger;
	if ( oTiger.fromUrn( strInput ) || oTiger.fromUrn< Hashes::base16Encoding >( strInput ) ||
		 oTiger.fromString( strInput ) || oTiger.fromString< Hashes::base16Encoding >( strInput ) )
		strSearch += oTiger.toString();
	else
		strSearch += L"[Tiger]";

	m_wndSearch.SetWindowText( strSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( (int)Hashes::TigerHash::urns[ 0 ].signatureLength, -1 );
}

void CSearchInputBox::OnSearchPrefixSHA1Tiger()
{
	CString strInput;
	m_wndSearch.GetWindowText( strInput );
	strInput.Trim();

	CString strSearch = Hashes::TigerHash::urns[ 2 ].signature;
	Hashes::TigerHash oTiger;
	Hashes::Sha1Hash oSHA1;
	oTiger.fromUrn( strInput ) || oTiger.fromUrn< Hashes::base16Encoding >( strInput );
	oSHA1.fromUrn( strInput ) || oSHA1.fromUrn< Hashes::base16Encoding >( strInput );
	if ( ! oSHA1 && ! oTiger )
	{
		// Try {SHA1}.{TIGER} first
		int nPos = strInput.FindOneOf( L" \t,.:-+=" );
		if ( nPos != -1 )
		{
			if ( oTiger.fromString( strInput.Mid( nPos + 1 ) ) || oTiger.fromString< Hashes::base16Encoding >( strInput.Mid( nPos + 1 ) ) )
				oSHA1.fromString( strInput.Left( nPos ) ) || oSHA1.fromString< Hashes::base16Encoding >( strInput.Left( nPos ) );
		}
		if ( ! oSHA1 && ! oTiger )
		{
			// Try {TIGER} or {SHA1}
			if ( ! ( oTiger.fromString( strInput ) || oTiger.fromString< Hashes::base16Encoding >( strInput ) ) )
				oSHA1.fromString( strInput ) || oSHA1.fromString< Hashes::base16Encoding >( strInput );
		}
	}
	strSearch += oSHA1 ? oSHA1.toString() : L"[SHA1]";
	strSearch += L".";
	strSearch += oTiger ? oTiger.toString() : L"[Tiger]";

	m_wndSearch.SetWindowText( strSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( (int)Hashes::TigerHash::urns[ 2 ].signatureLength, -1 );
}

void CSearchInputBox::OnSearchPrefixED2K()
{
	CString strInput;
	m_wndSearch.GetWindowText( strInput );
	strInput.Trim();

	CString strSearch = Hashes::Ed2kHash::urns[ 0 ].signature;
	Hashes::Ed2kHash oEd2k;
	if ( oEd2k.fromUrn( strInput ) || oEd2k.fromString( strInput ) )
		strSearch += oEd2k.toString();
	else
		strSearch += L"[ED2K]";

	m_wndSearch.SetWindowText( strSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( (int)Hashes::Ed2kHash::urns[ 0 ].signatureLength, -1 );
}

void CSearchInputBox::OnSearchPrefixMD5()
{
	CString strInput;
	m_wndSearch.GetWindowText( strInput );
	strInput.Trim();

	CString strSearch = Hashes::Md5Hash::urns[ 0 ].signature;
	Hashes::Md5Hash oMD5;
	if ( oMD5.fromUrn( strInput ) || oMD5.fromString( strInput ) )
		strSearch += oMD5.toString();
	else
		strSearch += L"[MD5]";

	m_wndSearch.SetWindowText( strSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( (int)_tcslen( Hashes::Md5Hash::urns[ 0 ].signature ), -1 );
}

void CSearchInputBox::OnSearchPrefixBTH()
{
	CString strInput;
	m_wndSearch.GetWindowText( strInput );
	strInput.Trim();

	CString strSearch = Hashes::BtHash::urns[ 0 ].signature;
	Hashes::BtHash oBTH;
	if ( oBTH.fromUrn( strInput ) || oBTH.fromUrn< Hashes::base16Encoding >( strInput ) ||
		 oBTH.fromString( strInput ) || oBTH.fromString< Hashes::base16Encoding >( strInput ) )
		strSearch += oBTH.toString();
	else
		strSearch += L"[BTIH]";

	m_wndSearch.SetWindowText( strSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( (int)Hashes::BtHash::urns[ 0 ].signatureLength, -1 );
}


/////////////////////////////////////////////////////////////////////////////
// CSearchAdvancedBox construction

CSearchAdvancedBox::CSearchAdvancedBox()
{
}

CSearchAdvancedBox::~CSearchAdvancedBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchAdvancedBox message handlers

int CSearchAdvancedBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc( 0, 0, 0, 0 );

	// Min combo
	if ( ! m_wndSizeMin.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_AUTOHSCROLL|CBS_DROPDOWN,
		rc, this, IDC_SEARCH_SIZEMIN ) ) return -1;
	m_wndSizeMin.SetFont( &theApp.m_gdiFont );

	m_wndSizeMin.AddString( L"" );
	m_wndSizeMin.AddString( L"100 KB" );
	m_wndSizeMin.AddString( L"1 MB" );
	m_wndSizeMin.AddString( L"10 MB" );
	m_wndSizeMin.AddString( L"50 MB" );
	m_wndSizeMin.AddString( L"100 MB" );
	m_wndSizeMin.AddString( L"200 MB" );
	m_wndSizeMin.AddString( L"500 MB" );
	m_wndSizeMin.AddString( L"1 GB" );
	m_wndSizeMin.AddString( L"4 GB" );

	// Max combo
	if ( ! m_wndSizeMax.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_AUTOHSCROLL|CBS_DROPDOWN,
		rc, this, IDC_SEARCH_SIZEMAX ) ) return -1;
	m_wndSizeMax.SetFont( &theApp.m_gdiFont );

	m_wndSizeMax.AddString( L"" );
	m_wndSizeMax.AddString( L"100 KB" );
	m_wndSizeMax.AddString( L"1 MB" );
	m_wndSizeMax.AddString( L"10 MB" );
	m_wndSizeMax.AddString( L"50 MB" );
	m_wndSizeMax.AddString( L"100 MB" );
	m_wndSizeMax.AddString( L"200 MB" );
	m_wndSizeMax.AddString( L"500 MB" );
	m_wndSizeMax.AddString( L"1 GB" );
	m_wndSizeMax.AddString( L"4 GB" );

	if ( ! m_wndSizeMinMax.Create( L"", WS_CHILD|WS_VISIBLE|SS_CENTER, rc, this ) )
		return -1;
	m_wndSizeMinMax.SetFont( &theApp.m_gdiFont );

	// Network checkboxes
	if ( ! m_wndCheckBoxG2.Create( L"G2", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_CHECKBOX | BS_FLAT, rc, this, IDC_SEARCH_GNUTELLA2 ) ) return -1;
	if ( ! m_wndCheckBoxG1.Create( L"G1", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_CHECKBOX | BS_FLAT, rc, this, IDC_SEARCH_GNUTELLA1 ) ) return -1;
	if ( ! m_wndCheckBoxED2K.Create( L"ED2K", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_CHECKBOX | BS_FLAT, rc, this, IDC_SEARCH_EDONKEY ) ) return -1;
	if ( ! m_wndCheckBoxDC.Create( L"DC++", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_CHECKBOX | BS_FLAT, rc, this, IDC_SEARCH_DC ) ) return -1;

	m_wndCheckBoxG2.SetFont( &theApp.m_gdiFontBold );
	m_wndCheckBoxG1.SetFont( &theApp.m_gdiFontBold );
	m_wndCheckBoxED2K.SetFont( &theApp.m_gdiFontBold );
	m_wndCheckBoxDC.SetFont( &theApp.m_gdiFontBold );

	if ( Settings.Gnutella2.Enabled ) m_wndCheckBoxG2.SetCheck( BST_CHECKED );
	if ( Settings.Gnutella1.Enabled ) m_wndCheckBoxG1.SetCheck( BST_UNCHECKED );	// ToDo: Temporary default until spam is solved
	if ( Settings.eDonkey.Enabled ) m_wndCheckBoxED2K.SetCheck( BST_CHECKED );
	if ( Settings.DC.Enabled ) m_wndCheckBoxDC.SetCheck( BST_CHECKED );

	return 0;
}

void CSearchAdvancedBox::OnSkinChange()
{
	CoolInterface.LoadIconsTo( m_gdiProtocols, protocolIDs );

	// Obsolete, for reference or deletion:
	//for ( int nImage = 1; nImage < nRevStart; nImage++ )
	//{
	//	if ( HICON hIcon = CoolInterface.ExtractIcon( (UINT)protocolCmdMap[ nImage ].commandID, FALSE ) )
	//	{
	//		m_gdiProtocols.Replace( Settings.General.LanguageRTL ? nRevStart - nImage : nImage, hIcon );
	//		DestroyIcon( hIcon );
	//	}
	//}

	CRect rc;
	GetClientRect( &rc );
	OnSize( NULL, rc.Width(), rc.Height() );

	if ( m_brBack.m_hObject ) m_brBack.DeleteObject();
	m_brBack.CreateSolidBrush( m_crBack = Colors.m_crTaskBoxClient );

	//CoolInterface.FixThemeControls( this );		// Checkbox/Groupbox text colors (Remove theme if needed)
	const BOOL bThemed =
		GetRValue( Colors.m_crTaskBoxText ) < 100 &&
		GetGValue( Colors.m_crTaskBoxText ) < 100 &&
		GetBValue( Colors.m_crTaskBoxText ) < 100;

	CoolInterface.EnableTheme( &m_wndCheckBoxG2, bThemed );
	CoolInterface.EnableTheme( &m_wndCheckBoxG1, bThemed );
	CoolInterface.EnableTheme( &m_wndCheckBoxED2K, bThemed );
	CoolInterface.EnableTheme( &m_wndCheckBoxDC, bThemed );
}

void CSearchAdvancedBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );

	HDWP hDWP = BeginDeferWindowPos( 3 );

	if ( m_wndSizeMin.m_hWnd )
	{
		const int nWidth = ( cx - BOX_MARGIN * 3 ) / 2;
		DeferWindowPos( hDWP, m_wndSizeMin, NULL, BOX_MARGIN, 22, nWidth, 219,		// Min Box Pos
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
		DeferWindowPos( hDWP, m_wndSizeMax, NULL, BOX_MARGIN, 61, nWidth, 219,		// Max Box Pos
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	}

	int nY = ( Settings.DC.ShowInterface && Settings.eDonkey.ShowInterface && Settings.Gnutella1.ShowInterface ) ? 8 : 26;
	const int nX = ( cx / 2 ) + BOX_MARGIN + 25;
	const int nWidth = ( cx - BOX_MARGIN * 3 ) / 2 - 26;

	if ( m_wndCheckBoxG2.m_hWnd )
		DeferWindowPos( hDWP, m_wndCheckBoxG2, NULL, nX, nY, nWidth, 14,			// G2 Checkbox Pos
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	/*if ( Settings.Gnutella2.ShowInterface )*/ nY += 20;
	if ( m_wndCheckBoxG1.m_hWnd )
		DeferWindowPos( hDWP, m_wndCheckBoxG1, NULL, nX, nY, nWidth, 14,			// G1 Checkbox Pos
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	if ( Settings.Gnutella1.ShowInterface ) nY += 20;
	if ( m_wndCheckBoxED2K.m_hWnd )
		DeferWindowPos( hDWP, m_wndCheckBoxED2K, NULL, nX, nY, nWidth, 14,			// ED2K Checkbox Pos
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	if ( Settings.eDonkey.ShowInterface ) nY += 20;
	if ( m_wndCheckBoxDC.m_hWnd )
		DeferWindowPos( hDWP, m_wndCheckBoxDC, NULL, nX, nY, nWidth, 14,			// DC++ Checkbox Pos
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );

	EndDeferWindowPos( hDWP );
}

void CSearchAdvancedBox::OnPaint()
{
	CPaintDC dc( this );
	CRect rc, rct;
	CString strControlTitle;

	UINT nFlags = ETO_CLIPPED;
	CDC* pDC = &dc;

	GetClientRect( &rc );

	if ( m_bmWatermark.m_hObject )
	{
		CSize size = rc.Size();
		pDC = CoolInterface.GetBuffer( dc, size );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmWatermark );
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		// Paints the background behind controls except checkboxes (see OnCtlColorStatic)
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor( Colors.m_crTaskBoxClient );
		nFlags |= ETO_OPAQUE;
	}

	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );

	pDC->SetTextColor( Colors.m_crTaskBoxText );	// Was 0

	LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_4 );		// "Min Filesize:"
	rct.SetRect( BOX_MARGIN + 1, BOX_MARGIN, rc.right / 2, BOX_MARGIN + 16 );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, strControlTitle, NULL );
	pDC->ExcludeClipRect( &rct );

	LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_5 );		// "Max Filesize:"
	rct.SetRect( BOX_MARGIN + 1, BOX_MARGIN + 39, rc.right / 2, BOX_MARGIN + 55 );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, strControlTitle, NULL );
	pDC->ExcludeClipRect( &rct );

	if ( ! Settings.DC.ShowInterface || ! Settings.eDonkey.ShowInterface || ! Settings.Gnutella1.ShowInterface )
	{
		LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_3 );	// "Network:"
		rct.SetRect( rc.right / 2 + BOX_MARGIN, BOX_MARGIN, rc.right - BOX_MARGIN, BOX_MARGIN + 16 );
		pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, strControlTitle, NULL );
		pDC->ExcludeClipRect( &rct );
	}

	pDC->SelectObject( pOldFont );

	if ( pDC == &dc )
		pDC->FillSolidRect( &rc, Colors.m_crTaskBoxClient );		// Paint remaining background for unskinned Advanced box

	const int nX = rc.right / 2 + BOX_MARGIN + 1;
	int nY = ( Settings.DC.ShowInterface && Settings.eDonkey.ShowInterface && Settings.Gnutella1.ShowInterface ) ? 6 : 24;

	//if ( Settings.Gnutella2.ShowInterface )
	{
		m_gdiProtocols.Draw( pDC, PROTOCOL_G2, CPoint( nX, nY ), ILD_NORMAL );		// G2 Icon
		nY += 20;
	}
	if ( Settings.Gnutella1.ShowInterface )
	{
		m_gdiProtocols.Draw( pDC, PROTOCOL_G1, CPoint( nX, nY ), ILD_NORMAL );		// G1 Icon
		nY += 20;
	}
	if ( Settings.eDonkey.ShowInterface )
	{
		m_gdiProtocols.Draw( pDC, PROTOCOL_ED2K, CPoint( nX, nY ), ILD_NORMAL );	// ED2K Icon
		nY += 20;
	}
	if ( Settings.DC.ShowInterface )
	{
		m_gdiProtocols.Draw( pDC, PROTOCOL_DC, CPoint( nX, nY ), ILD_NORMAL );		// DC++ Icon
	//	nY += 20;
	}

	if ( pDC != &dc )
		dc.BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );

	m_wndCheckBoxG2.EnableWindow( Settings.Gnutella2.Enabled );
	m_wndCheckBoxG1.EnableWindow( Settings.Gnutella1.Enabled );
	m_wndCheckBoxED2K.EnableWindow( Settings.eDonkey.Enabled );
	m_wndCheckBoxDC.EnableWindow( Settings.DC.Enabled );

	if ( Settings.Gnutella1.ShowInterface )
		m_wndCheckBoxG1.ModifyStyle( 0, WS_VISIBLE );
	else
		m_wndCheckBoxG1.ModifyStyle( WS_VISIBLE, 0 );

	if ( Settings.eDonkey.ShowInterface )
		m_wndCheckBoxED2K.ModifyStyle( 0, WS_VISIBLE );
	else
		m_wndCheckBoxED2K.ModifyStyle( WS_VISIBLE, 0 );

	if ( Settings.DC.ShowInterface )
		m_wndCheckBoxDC.ModifyStyle( 0, WS_VISIBLE );
	else
		m_wndCheckBoxDC.ModifyStyle( WS_VISIBLE, 0 );
}

LRESULT CSearchAdvancedBox::OnCtlColorStatic(WPARAM wParam, LPARAM /*lParam*/)
{
	//HBRUSH hbr = NULL;
	HDC hDCStatic = (HDC)wParam;

	//hbr = m_brBack;

	SetBkColor( hDCStatic, Colors.m_crTaskBoxClient );
	SetTextColor( hDCStatic, Colors.m_crTaskBoxText );

	return (LRESULT)(HBRUSH)m_brBack;
}

// CSearchAdvancedBox Check Boxes
void CSearchAdvancedBox::OnG2Clicked()
{
	CButton* pBox = &m_wndCheckBoxG2;
	pBox->SetCheck( pBox->GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED );
}

void CSearchAdvancedBox::OnG1Clicked()
{
	CButton* pBox = &m_wndCheckBoxG1;
	pBox->SetCheck( pBox->GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED );
}

void CSearchAdvancedBox::OnED2KClicked()
{
	CButton* pBox = &m_wndCheckBoxED2K;
	pBox->SetCheck( pBox->GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED );
}

void CSearchAdvancedBox::OnDCClicked()
{
	CButton* pBox = &m_wndCheckBoxDC;
	pBox->SetCheck( pBox->GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED );
}


/////////////////////////////////////////////////////////////////////////////
// CSearchSchemaBox construction

CSearchSchemaBox::CSearchSchemaBox()
{
}

CSearchSchemaBox::~CSearchSchemaBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchSchemaBox message handlers

int CSearchSchemaBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc;
	if ( ! m_wndSchema.Create( WS_VISIBLE, rc, this, 0 ) ) return -1;

	m_wndSchema.m_nCaptionWidth	= 0;
	m_wndSchema.m_nItemHeight	= 42;
	m_wndSchema.m_bShowBorder	= FALSE;

	return 0;
}

void CSearchSchemaBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );
	m_wndSchema.SetWindowPos( NULL, 0, 1, cx, cy - 1, SWP_NOZORDER | SWP_NOACTIVATE );
}


/////////////////////////////////////////////////////////////////////////////
// CSearchResultsBox

CSearchResultsBox::CSearchResultsBox()
	: m_bActive 	( FALSE )
	, m_nFiles		( 0 )
	, m_nHits		( 0 )
	, m_nBadHits	( 0 )
	, m_nHubs		( 0 )
	, m_nLeaves 	( 0 )
{
	Expand( Settings.Search.ResultsPanel ? TRUE : FALSE );
//	Expand( theApp.GetProfileInt( L"Settings", L"SearchPanelResults", TRUE ) );
}

CSearchResultsBox::~CSearchResultsBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchResultsBox message handlers

void CSearchResultsBox::Update(BOOL bSearching, DWORD nHubs, DWORD nLeaves, DWORD nFiles, DWORD nHits, DWORD nBadHits)
{
	m_bActive	= bSearching;
	m_nHubs		= nHubs;
	m_nLeaves	= nLeaves;
	m_nFiles	= nFiles;
	m_nHits		= nHits;
	m_nBadHits	= nBadHits;

	Invalidate();
}

void CSearchResultsBox::OnPaint()
{
	CString strText, strFormat;
	UINT nFlags = ETO_CLIPPED;

	CPaintDC dc( this );
	CDC* pDC = &dc;

	CRect rc;
	GetClientRect( &rc );

	if ( m_bmWatermark.m_hObject )
	{
		CSize size = rc.Size();
		pDC = CoolInterface.GetBuffer( dc, size );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmWatermark );
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor( Colors.m_crTaskBoxClient );
		nFlags |= ETO_OPAQUE;
	}

	CFont* pOldFont = (CFont*)pDC->SelectObject( &theApp.m_gdiFontBold );

	pDC->SetTextColor( Colors.m_crTaskBoxText );	// Was 0

	LoadString( strText, IDS_SEARCH_PANEL_RESULTS_STATUS );
	DrawText( pDC, BOX_MARGIN, BOX_MARGIN, nFlags, strText );
	LoadString( strText, IDS_SEARCH_PANEL_RESULTS_FOUND );
	DrawText( pDC, BOX_MARGIN, BOX_MARGIN + 32, nFlags, strText );

	pDC->SelectObject( &theApp.m_gdiFont );

	if ( m_bActive )
	{
		LoadString( strFormat, IDS_SEARCH_PANEL_RESULTS_ACTIVE );
		strText.Format( strFormat, m_nHubs, m_nLeaves );
	}
	else
	{
		LoadString( strText, IDS_SEARCH_PANEL_RESULTS_INACTIVE );
	}

	DrawText( pDC, BOX_MARGIN + 8, BOX_MARGIN + 15, nFlags, strText );

	if ( m_nFiles )
	{
		LoadString( strFormat, IDS_SEARCH_PANEL_RESULTS_FORMAT );

		if ( strFormat.Find( L'|' ) >= 0 )
		{
			if ( m_nFiles == 1 && m_nHits == 1 )
				Skin.SelectCaption( strFormat, 0 );
			else if ( m_nFiles == 1 )
				Skin.SelectCaption( strFormat, 1 );
			else
				Skin.SelectCaption( strFormat, 2 );

			strText.Format( strFormat,
				m_nFiles, m_nHits );
		}
		else
		{
			strText.Format( strFormat,
				m_nFiles, m_nFiles != 1 ? L"s" : L"",
				m_nHits, m_nHits != 1 ? L"s" : L"" );
		}
	}
	else
	{
		LoadString( strText, IDS_SEARCH_PANEL_RESULTS_NONE );
	}

	DrawText( pDC, BOX_MARGIN + 8, BOX_MARGIN + 32 + 15, nFlags, strText );

	// ToDo: Change Filtered Results Count from Files to Hits (WndSearch.cpp L.795)
	if ( m_nBadHits && Settings.General.GUIMode != GUI_BASIC )
	{
		LoadString( strFormat, IDS_SEARCH_PANEL_FILTERED );

		//if ( strFormat.Find( L'|' ) >= 0 )
		//{
		//	if ( m_nBadHits == 1 )
		//		Skin.SelectCaption( strFormat, 0 );
		//	else
		//		Skin.SelectCaption( strFormat, 1 );
		//}

		strText.Format( strFormat, m_nBadHits );

		DrawText( pDC, BOX_MARGIN + 8, BOX_MARGIN + 32 + 30, nFlags, strText );
	}

	pDC->SelectObject( pOldFont );

	if ( pDC != &dc )
	{
		dc.BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );
		pDC->SelectClipRgn( NULL );
	}
	else
	{
		pDC->FillSolidRect( &rc, Colors.m_crTaskBoxClient );
	}
}

void CSearchResultsBox::DrawText(CDC* pDC, int nX, int nY, UINT nFlags, LPCTSTR pszText)
{
	CSize cz = pDC->GetTextExtent( pszText, static_cast< int >( _tcslen( pszText ) ) );
	CRect rc( nX, nY, nX + cz.cx, nY + cz.cy );

	pDC->ExtTextOut( nX, nY, nFlags, &rc, pszText, static_cast< UINT >( _tcslen( pszText ) ), NULL );
	pDC->ExcludeClipRect( nX, nY, nX + cz.cx, nY + cz.cy );
}

void CSearchResultsBox::OnExpanded(BOOL bOpen)
{
	Settings.Search.ResultsPanel = ( bOpen != FALSE );
//	theApp.WriteProfileInt( L"Settings", L"SearchPanelResults", bOpen );
}
