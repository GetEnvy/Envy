//
// DlgNewSearch.cpp
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
#include "QuerySearch.h"
#include "DlgNewSearch.h"

#include "Images.h"
#include "Skin.h"
#include "Schema.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define PART_GAP		8
#define PART_HEIGHT 	20
#define BUTTON_HEIGHT	24
#define BUTTON_WIDTH	72


BEGIN_MESSAGE_MAP(CNewSearchDlg, CSkinDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, OnCloseUpSchemas)
	ON_EN_CHANGE(IDC_SEARCH, OnChangeSearch)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNewSearchDlg dialog

CNewSearchDlg::CNewSearchDlg(CWnd* pParent, CQuerySearch* pSearch, BOOL bLocal, BOOL bAgain)
	: CSkinDialog( CNewSearchDlg::IDD, pParent )
	, m_pSearch( pSearch ? pSearch : new CQuerySearch() )
{
	m_bLocal = bLocal;
	m_bAgain = bAgain;
}

void CNewSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
	DDX_Control(pDX, IDC_SEARCH, m_wndSearch);
}

/////////////////////////////////////////////////////////////////////////////
// CNewSearchDlg message handlers

BOOL CNewSearchDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CNewSearchDlg", IDR_SEARCHFRAME );

	SelectCaption( this, m_bLocal ? 2 : ( m_bAgain ? 1 : 0 ) );

	CRect rc;
	CString strText;
	m_wndSchema.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rc, this, IDC_METADATA );

	LoadString( strText, IDS_SEARCH_PLAIN_TEXT );
	m_wndSchemas.m_sNoSchemaText = strText;
	m_wndSchemas.Load( Settings.Search.LastSchemaURI );

	m_wndSchemas.Select( m_pSearch->m_pSchema );

	OnSelChangeSchemas();

	if ( m_pSearch->m_pXML )
		m_wndSchema.UpdateData( m_pSearch->m_pXML->GetFirstElement(), FALSE );

	Settings.LoadWindow( L"NewSearch", this );

	OnCloseUpSchemas();

	if ( m_pSearch->m_oSHA1 )
	{
		m_wndSearch.SetWindowText( m_pSearch->m_oSHA1.toUrn() );
		m_wndSchema.ShowWindow( SW_HIDE );
	}
	else
	{
		m_wndSearch.SetWindowText( m_pSearch->m_sSearch );
	}

	if ( m_wndSchemas.GetCurSel() > 0 ) m_wndSchemas.SetFocus();

	return FALSE;
}

void CNewSearchDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CSkinDialog::OnGetMinMaxInfo( lpMMI );
	lpMMI->ptMinTrackSize.y = Skin.m_nBanner + 4 * PART_GAP + 3 * BUTTON_HEIGHT + 30; 	// 30 = default frame
	lpMMI->ptMaxTrackSize.x = max( Images.m_bmBanner.GetBitmapDimension().cx + 8, 400 );	//  8 = typical frame
	lpMMI->ptMinTrackSize.x = PART_GAP * 3 + BUTTON_WIDTH * 2 + 13;						// 13 = default frame
}

void CNewSearchDlg::OnSize(UINT nType, int cx, int cy)
{
	CSkinDialog::OnSize( nType, cx, cy );

	if ( ! IsWindow( m_wndSchema.m_hWnd ) ) return;

	m_wndSearch.SetWindowPos( NULL,
		PART_GAP, Skin.m_nBanner + PART_GAP + 2, cx - PART_GAP * 2, PART_HEIGHT, SWP_NOZORDER );
	m_wndSchemas.SetWindowPos( NULL,
		PART_GAP, Skin.m_nBanner + PART_GAP * 2 + PART_HEIGHT + 2, cx - PART_GAP * 2, PART_HEIGHT, SWP_NOZORDER );

	if ( cy > PART_GAP * 5 + PART_HEIGHT * 2 + BUTTON_HEIGHT + Skin.m_nBanner )
	{
		m_wndSchema.SetWindowPos( NULL,
			PART_GAP, Skin.m_nBanner + PART_GAP * 3 + BUTTON_HEIGHT * 2,
			cx - PART_GAP * 2, cy - PART_GAP * 6 - PART_HEIGHT * 2 - BUTTON_HEIGHT - Skin.m_nBanner,
			SWP_NOZORDER|SWP_SHOWWINDOW );
	}
	else
	{
		m_wndSchema.ShowWindow( SW_HIDE );
	}

	m_wndCancel.SetWindowPos( NULL,
		cx - BUTTON_WIDTH - PART_GAP, cy - PART_GAP - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER );
	m_wndOK.SetWindowPos( NULL,
		cx - BUTTON_WIDTH - PART_GAP - BUTTON_WIDTH - PART_GAP, cy - PART_GAP - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER );
}

void CNewSearchDlg::OnSelChangeSchemas()
{
	CSchemaPtr pSchema = m_wndSchemas.GetSelected();
	m_wndSchema.SetSchema( pSchema, TRUE );
}

void CNewSearchDlg::OnCloseUpSchemas()
{
	CSchemaPtr pSchema = m_wndSchemas.GetSelected();

	CRect rcWindow;
	GetWindowRect( &rcWindow );

	if ( pSchema != NULL )
	{
		if ( rcWindow.Height() <= 200 )
			SetWindowPos( NULL, 0, 0, rcWindow.Width(), 264, SWP_NOMOVE|SWP_NOZORDER );

		PostMessage( WM_KEYDOWN, VK_TAB );
	}
	else
	{
		m_wndSearch.SetFocus();
	}
}

void CNewSearchDlg::OnChangeSearch()
{
	CString strSearch;
	m_wndSearch.GetWindowText( strSearch );

	BOOL bHash = FALSE;
	Hashes::TigerHash oTiger;
	Hashes::Sha1Hash oSHA1;
	Hashes::Ed2kHash oED2K;
	Hashes::Md5Hash oMD5;
	Hashes::BtHash oBTH;

	bHash |= static_cast< BOOL >( oSHA1.fromUrn( strSearch ) );
	bHash |= static_cast< BOOL >( oTiger.fromUrn( strSearch ) );
	bHash |= static_cast< BOOL >( oED2K.fromUrn( strSearch ) );
	bHash |= static_cast< BOOL >( oMD5.fromUrn( strSearch ) );
	bHash |= static_cast< BOOL >( oBTH.fromUrn( strSearch ) ||
		oBTH.fromUrn< Hashes::base16Encoding >( strSearch ) );

	if ( m_wndSchema.IsWindowVisible() == bHash )
		m_wndSchema.ShowWindow( bHash ? SW_HIDE : SW_SHOW );
}

void CNewSearchDlg::OnOK()
{
	Settings.SaveWindow( L"NewSearch", this );

	m_wndSearch.GetWindowText( m_pSearch->m_sSearch );

	CSchemaPtr pSchema = m_wndSchemas.GetSelected();

	if ( m_pSearch->m_pXML != NULL ) delete m_pSearch->m_pXML;

	m_pSearch->m_pSchema	= NULL;
	m_pSearch->m_pXML		= NULL;

	if ( pSchema != NULL )
	{
		m_pSearch->m_pSchema	= pSchema;
		m_pSearch->m_pXML		= pSchema->Instantiate();

		m_wndSchema.UpdateData( m_pSearch->m_pXML->AddElement( pSchema->m_sSingular ), TRUE );

		Settings.Search.LastSchemaURI = pSchema->GetURI();
	}
	else
	{
		Settings.Search.LastSchemaURI.Empty();
	}

	if ( ! m_pSearch->CheckValid() )
	{
		m_wndSearch.SetFocus();
		return;
	}

	CSkinDialog::OnOK();
}
