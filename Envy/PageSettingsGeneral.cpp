//
// PageSettingsGeneral.cpp
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
#include "PageSettingsGeneral.h"
#include "DlgHelp.h"
#include "Schema.h"
#include "SchemaCache.h"

#include "CtrlCoolTip.h"	// Dropshadow updates
#include "CtrlMatchTip.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CGeneralSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CGeneralSettingsPage, CSettingsPage)
	ON_CBN_DROPDOWN(IDC_CLOSE_MODE, OnDropdownCloseMode)
	ON_CBN_DROPDOWN(IDC_TRAY_MINIMISE, OnDropdownTrayMinimise)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGeneralSettingsPage property page

CGeneralSettingsPage::CGeneralSettingsPage()
	: CSettingsPage(CGeneralSettingsPage::IDD)
	, m_nRatesInBytes	( -1 )
	, m_bAutoConnect	( FALSE )
	, m_bStartup		( FALSE )
	, m_bPromptURLs 	( FALSE )
	, m_bUpdateCheck	( TRUE )
	, m_bSimpleBar		( FALSE )
	, m_bExpandMatches	( FALSE )
	, m_bExpandDownloads ( FALSE )
	, m_bSwitchToTransfers ( FALSE )
	, m_bHideSearch 	( FALSE )
	, m_bAdultFilter	( FALSE )
	, m_bTipShadow		( TRUE )
	, m_bTrayMinimise	( -1 )
	, m_nCloseMode		( -1 )
	, m_nTipDelay 		( 0 )
{
}

CGeneralSettingsPage::~CGeneralSettingsPage()
{
}

void CGeneralSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_AUTO_CONNECT, m_bAutoConnect);
	DDX_Check(pDX, IDC_AUTO_START, m_bStartup);
	DDX_Check(pDX, IDC_PROMPT_URLS, m_bPromptURLs);
	DDX_Check(pDX, IDC_UPDATE_CHECK, m_bUpdateCheck);
	DDX_Check(pDX, IDC_EXPAND_DOWNLOAD, m_bExpandDownloads);
	DDX_Check(pDX, IDC_DOWNLOADS_SIMPLEBAR, m_bSimpleBar);
	DDX_Check(pDX, IDC_EXPAND_MATCHES, m_bExpandMatches);
	DDX_Check(pDX, IDC_SWITCH_TO_TRANSFERS, m_bSwitchToTransfers);
	DDX_Check(pDX, IDC_HIDE_SEARCH, m_bHideSearch);
	DDX_Check(pDX, IDC_ADULT_FILTER, m_bAdultFilter);
	DDX_Check(pDX, IDC_TIP_SHADOW, m_bTipShadow);
	DDX_Text(pDX, IDC_TIP_DELAY, m_nTipDelay);
	DDX_Control(pDX, IDC_TIP_DELAY_SPIN, m_wndTipSpin);
	DDX_Control(pDX, IDC_TIP_DISPLAY, m_wndTips);
	DDX_Control(pDX, IDC_TIP_ALPHA, m_wndTipAlpha);
	DDX_Control(pDX, IDC_CLOSE_MODE, m_wndCloseMode);
	DDX_Control(pDX, IDC_TRAY_MINIMISE, m_wndTrayMinimise);
	DDX_CBIndex(pDX, IDC_CLOSE_MODE, m_nCloseMode);
	DDX_CBIndex(pDX, IDC_TRAY_MINIMISE, m_bTrayMinimise);
	DDX_CBIndex(pDX, IDC_RATES_IN_BYTES, m_nRatesInBytes);
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralSettingsPage message handlers

BOOL CGeneralSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bStartup				= Settings.CheckStartup();
	m_bAutoConnect			= Settings.Connection.AutoConnect;
	m_bPromptURLs			= ! Settings.General.AlwaysOpenURLs;
	m_bUpdateCheck			= Settings.VersionCheck.UpdateCheck;
	m_bExpandDownloads		= Settings.Downloads.AutoExpand;
	m_bSimpleBar			= Settings.Downloads.SimpleBar;
	m_bExpandMatches		= Settings.Search.ExpandMatches;
	m_bSwitchToTransfers	= Settings.Search.SwitchToTransfers;
	m_bHideSearch			= Settings.Search.HideSearchPanel;
	m_bAdultFilter			= Settings.Search.AdultFilter;
	m_bTipShadow			= Settings.Interface.TipShadow;
	m_bTrayMinimise			= Settings.General.TrayMinimise;
	m_nCloseMode			= Settings.General.CloseMode;

	m_nRatesInBytes			= Settings.General.RatesInBytes
							+ Settings.General.RatesUnit * 2;

	CRect rc;
	m_wndTips.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	m_wndTips.InsertColumn( 0, L"Name", LVCFMT_LEFT, rc.right, 0 );
	m_wndTips.SetExtendedStyle( LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );

	CString strTitle( L"Search Results" );

	if ( CSchemaPtr pSchema = SchemaCache.Get( CSchema::uriSearchFolder ) )
	{
		strTitle = pSchema->m_sTitle;
		const int nColon = strTitle.Find( L':' );
		if ( nColon >= 0 )
			strTitle = strTitle.Mid( nColon + 1 ).Trim();
	}

	Add( strTitle, Settings.Interface.TipSearch );
	LoadString( strTitle, IDR_DOWNLOADSFRAME );
	Add( strTitle, Settings.Interface.TipDownloads );
	LoadString( strTitle, IDR_UPLOADSFRAME );
	Add( strTitle, Settings.Interface.TipUploads );
	LoadString( strTitle, IDR_LIBRARYFRAME );
	Add( strTitle, Settings.Interface.TipLibrary );
	LoadString( strTitle, IDR_NEIGHBOURSFRAME );
	Add( strTitle, Settings.Interface.TipNeighbours );
	LoadString( strTitle, IDR_MEDIAFRAME );
	Add( strTitle, Settings.Interface.TipMedia );

	Settings.SetRange( &Settings.Interface.TipDelay, m_wndTipSpin );
	m_nTipDelay	= Settings.Interface.TipDelay;

	Settings.SetRange( &Settings.Interface.TipAlpha, m_wndTipAlpha );
	m_wndTipAlpha.SetPos( Settings.Interface.TipAlpha );

	//if ( Win2000 )	// Legacy, for reference
	//	GetDlgItem( IDC_TIP_SHADOW )->EnableWindow( FALSE );

	UpdateData( FALSE );

	return TRUE;
}

void CGeneralSettingsPage::Add(LPCTSTR pszName, BOOL bState)
{
	const int nItem = m_wndTips.InsertItem( LVIF_TEXT, m_wndTips.GetItemCount(), pszName, 0, 0, 0, 0 );

	if ( bState )
		m_wndTips.SetItemState( nItem, 2 << 12, LVIS_STATEIMAGEMASK );
}

void CGeneralSettingsPage::OnDropdownCloseMode()
{
	RecalcDropWidth( &m_wndCloseMode );
}

void CGeneralSettingsPage::OnDropdownTrayMinimise()
{
	RecalcDropWidth( &m_wndTrayMinimise );
}

void CGeneralSettingsPage::OnOK()
{
	UpdateData();

	if ( m_bAdultFilter && ! Settings.Search.AdultFilter && ! Settings.Live.AdultWarning )
	{
		Settings.Live.AdultWarning = true;
		CHelpDlg::Show( L"GeneralHelp.AdultFilter" );
	}

	Settings.SetStartup( m_bStartup );
	Settings.Connection.AutoConnect		= m_bAutoConnect != FALSE;
	Settings.General.AlwaysOpenURLs		= ! m_bPromptURLs;
	Settings.VersionCheck.UpdateCheck	= m_bUpdateCheck != FALSE;
	Settings.Downloads.AutoExpand		= m_bExpandDownloads != FALSE;
	Settings.Downloads.SimpleBar		= m_bSimpleBar != FALSE;
	Settings.Search.ExpandMatches		= m_bExpandMatches != FALSE;
	Settings.Search.SwitchToTransfers	= m_bSwitchToTransfers != FALSE;
	Settings.Search.HideSearchPanel		= m_bHideSearch != FALSE;
	Settings.Search.AdultFilter			= m_bAdultFilter != FALSE;
	Settings.General.TrayMinimise		= m_bTrayMinimise != FALSE;
	Settings.General.CloseMode			= m_nCloseMode;

	Settings.General.RatesInBytes		= m_nRatesInBytes % 2 == 1;
	Settings.General.RatesUnit			= m_nRatesInBytes / 2;

	Settings.Interface.TipSearch		= m_wndTips.GetItemState( 0, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipLibrary		= m_wndTips.GetItemState( 1, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipDownloads		= m_wndTips.GetItemState( 2, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipUploads		= m_wndTips.GetItemState( 3, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipNeighbours	= m_wndTips.GetItemState( 4, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipMedia			= m_wndTips.GetItemState( 5, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );

	Settings.Interface.TipDelay 		= m_nTipDelay;
	Settings.Interface.TipAlpha 		= m_wndTipAlpha.GetPos();

	// Update DropShadow
	if ( (BOOL)Settings.Interface.TipShadow != m_bTipShadow )
	{
		Settings.Interface.TipShadow	= m_bTipShadow == TRUE;
		CCoolTipCtrl::m_hClass  = AfxRegisterWndClass( CS_SAVEBITS | ( m_bTipShadow ? CS_DROPSHADOW : 0 ) );
		CMatchTipCtrl::m_hClass = AfxRegisterWndClass( CS_SAVEBITS | ( m_bTipShadow ? CS_DROPSHADOW : 0 ) );
		PostMainWndMessage( WM_SKINCHANGED );
	}

	CSettingsPage::OnOK();
}
