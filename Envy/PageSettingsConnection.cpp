//
// PageSettingsConnection.cpp
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
#include "PageSettingsConnection.h"
#include "DlgHelp.h"
#include "Network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CConnectionSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CConnectionSettingsPage, CSettingsPage)
	ON_WM_SHOWWINDOW()
	ON_CBN_EDITCHANGE(IDC_INBOUND_HOST, &CConnectionSettingsPage::OnEditChangeInboundHost)
	ON_CBN_SELCHANGE(IDC_INBOUND_HOST, &CConnectionSettingsPage::OnChangedInboundHost)
	ON_EN_CHANGE(IDC_INBOUND_PORT, &CConnectionSettingsPage::OnChangeInboundPort)
	ON_BN_CLICKED(IDC_INBOUND_RANDOM, &CConnectionSettingsPage::OnInboundRandom)
	ON_BN_CLICKED(IDC_ENABLE_UPNP, &CConnectionSettingsPage::OnClickedEnableUpnp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CConnectionSettingsPage property page

CConnectionSettingsPage::CConnectionSettingsPage() : CSettingsPage(CConnectionSettingsPage::IDD)
	, m_nInPort				( 0 )
	, m_bInBind				( FALSE )
	, m_bIgnoreLocalIP		( FALSE )
	, m_bEnableUPnP			( FALSE )
	, m_nTimeoutConnection	( 0ul )
	, m_nTimeoutHandshake	( 0ul )
	, m_bInRandom			( FALSE )
{
}

CConnectionSettingsPage::~CConnectionSettingsPage()
{
}

void CConnectionSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INBOUND_PORT, m_wndInPort);
	DDX_Control(pDX, IDC_INBOUND_SPEED, m_wndInSpeed);
	DDX_Control(pDX, IDC_OUTBOUND_SPEED, m_wndOutSpeed);
	DDX_Control(pDX, IDC_INBOUND_HOST, m_wndInHost);
	DDX_Control(pDX, IDC_OUTBOUND_HOST, m_wndOutHost);
	DDX_Control(pDX, IDC_INBOUND_BIND, m_wndInBind);
	DDX_Control(pDX, IDC_TIMEOUT_HANDSHAKE_SPIN, m_wndTimeoutHandshake);
	DDX_Control(pDX, IDC_TIMEOUT_CONNECTION_SPIN, m_wndTimeoutConnection);
	DDX_Check(pDX, IDC_IGNORE_LOCAL, m_bIgnoreLocalIP);
	DDX_Check(pDX, IDC_INBOUND_BIND, m_bInBind);
	DDX_CBString(pDX, IDC_INBOUND_HOST, m_sInHost);
	DDX_Text(pDX, IDC_INBOUND_PORT, m_nInPort);
	DDX_CBString(pDX, IDC_OUTBOUND_HOST, m_sOutHost);
	DDX_Text(pDX, IDC_TIMEOUT_CONNECTION, m_nTimeoutConnection);
	DDX_Text(pDX, IDC_TIMEOUT_HANDSHAKE, m_nTimeoutHandshake);
	DDX_Control(pDX, IDC_CAN_ACCEPT, m_wndCanAccept);
	DDX_CBString(pDX, IDC_OUTBOUND_SPEED, m_sOutSpeed);
	DDX_CBString(pDX, IDC_INBOUND_SPEED, m_sInSpeed);
	DDX_Check(pDX, IDC_INBOUND_RANDOM, m_bInRandom);
	DDX_Check(pDX, IDC_ENABLE_UPNP, m_bEnableUPnP);
}

/////////////////////////////////////////////////////////////////////////////
// CConnectionSettingsPage message handlers

BOOL CConnectionSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	CString strAutomatic = GetInOutHostTranslation();
	CComboBox* pOutHost = (CComboBox*) GetDlgItem( IDC_OUTBOUND_HOST );

	// Update all dropdowns
	m_wndInHost.DeleteString( 0 );
	m_wndInHost.AddString( strAutomatic );
	pOutHost->DeleteString( 0 );
	pOutHost->AddString( strAutomatic );

	// Firewall status
	m_wndCanAccept.AddString( LoadString( IDS_GENERAL_AUTO ) );
	m_wndCanAccept.AddString( LoadString( IDS_GENERAL_NO ) );
	m_wndCanAccept.AddString( LoadString( IDS_GENERAL_YES ) );
	//m_wndCanAccept.AddString( L"TCP-Only" );
	//m_wndCanAccept.AddString( L"UDP-Only" );	// "Temp disabled"  ToDo:?

	m_wndCanAccept.SetCurSel( Settings.Connection.FirewallState );

	m_sInHost				= Settings.Connection.InHost;
	m_bInRandom				= Settings.Connection.RandomPort;
	m_nInPort				= m_bInRandom ? 0 : Settings.Connection.InPort;
	m_bInBind				= Settings.Connection.InBind;
	m_sOutHost				= Settings.Connection.OutHost;
	m_bIgnoreLocalIP		= Settings.Connection.IgnoreLocalIP;
	m_bEnableUPnP			= Settings.Connection.EnableUPnP;
	m_nTimeoutConnection	= Settings.Connection.TimeoutConnect / 1000;
	m_nTimeoutHandshake		= Settings.Connection.TimeoutHandshake / 1000;

	DWORD ip = 0;

	// Take an IP address table
	char mib[ sizeof( MIB_IPADDRTABLE ) + 32 * sizeof( MIB_IPADDRROW ) ];
	ULONG nSize = sizeof( mib );
	PMIB_IPADDRTABLE ipAddr = (PMIB_IPADDRTABLE)mib;

	if ( GetIpAddrTable( ipAddr, &nSize, TRUE ) == NO_ERROR )
	{
		CString strIP;
		const DWORD nCount = ipAddr->dwNumEntries;
		for ( DWORD nIf = 0 ; nIf < nCount ; nIf++ )
		{
			ip = ipAddr->table[ nIf ].dwAddr;
			if ( ip == 0x0100007f || ip == 0x0 )
				continue;	// loopback or 0.0.0.0

			MIB_IFROW ifRow = {};
			ifRow.dwIndex = ipAddr->table[ nIf ].dwIndex;
			// Check interface
			if ( GetIfEntry( &ifRow ) != NO_ERROR || ifRow.dwAdminStatus != MIB_IF_ADMIN_STATUS_UP )
				continue;

			strIP.Format( L"%u.%u.%u.%u",
				( ip & 0x0000ff ),
				( ( ip & 0x00ff00 ) >> 8 ),
				( ( ip & 0xff0000 ) >> 16 ),
				( ip >> 24 ) );

			m_wndInHost.InsertString( -1, (LPCTSTR)strIP );
			m_wndOutHost.InsertString( -1, (LPCTSTR)strIP );
		}
	}

	if ( m_sInHost.IsEmpty() ) m_sInHost = strAutomatic;
	if ( m_sOutHost.IsEmpty() ) m_sOutHost = strAutomatic;

	m_wndTimeoutConnection.SetRange( 1, 480 );
	m_wndTimeoutHandshake.SetRange( 1, 480 );

	if ( Settings.Experimental.LAN_Mode )	// #ifdef LAN_MODE
		GetDlgItem( IDC_IGNORE_LOCAL )->EnableWindow( FALSE );

	UpdateData( FALSE );

	m_wndInBind.EnableWindow( m_sInHost != strAutomatic );

	// UPnP is not supported on servers? (Obsolete)
	//if ( IsServer )
	//{
	//	CButton* pWnd = (CButton*)GetDlgItem( IDC_ENABLE_UPNP );
	//	pWnd->EnableWindow( FALSE );
	//}

	return TRUE;
}

void CConnectionSettingsPage::OnEditChangeInboundHost()
{
	CString strAutomatic = GetInOutHostTranslation();

	UpdateData();

	m_wndInBind.EnableWindow( m_sInHost != strAutomatic );
}

void CConnectionSettingsPage::OnChangedInboundHost()
{
	CString strAutomatic = GetInOutHostTranslation();
	CString strSelection;
	int nIndex = m_wndInHost.GetCurSel();
	if ( nIndex != CB_ERR )
		m_wndInHost.GetLBText( nIndex, strSelection );

	m_wndInBind.EnableWindow( strAutomatic != strSelection );
}

void CConnectionSettingsPage::OnChangeInboundPort()
{
	UpdateData();
	BOOL bRandom = m_nInPort == 0;

	if ( bRandom != m_bInRandom )
	{
		m_bInRandom = bRandom;
		UpdateData( FALSE );
	}
}

void CConnectionSettingsPage::OnInboundRandom()
{
	UpdateData();

	if ( m_bInRandom && m_nInPort != 0 )
	{
		m_nInPort = 0;
		UpdateData( FALSE );
	}
}

BOOL CConnectionSettingsPage::OnKillActive()
{
	UpdateData();

	if ( ! Settings.ParseVolume( m_sInSpeed, Kilobits ) )
	{
		MsgBox( IDS_SETTINGS_NEED_BANDWIDTH, MB_ICONEXCLAMATION );
		m_wndInSpeed.SetFocus();
		return FALSE;
	}

	if ( ! Settings.ParseVolume( m_sOutSpeed, Kilobits ) )
	{
		MsgBox( IDS_SETTINGS_NEED_BANDWIDTH, MB_ICONEXCLAMATION );
		m_wndOutSpeed.SetFocus();
		return FALSE;
	}

	return CSettingsPage::OnKillActive();
}

void CConnectionSettingsPage::OnOK()
{
	UpdateData();

	CString strAutomatic = GetInOutHostTranslation();

	if ( m_sInHost.CompareNoCase( strAutomatic ) == 0 )
	{
		m_sInHost.Empty();
		m_bInBind = FALSE;
	}
	else
		m_bInBind = TRUE;

	if ( m_sOutHost.CompareNoCase( strAutomatic ) == 0 )
		m_sOutHost.Empty();

	bool bOldEnableUPnP	= Settings.Connection.EnableUPnP;
	DWORD nOldInPort	= Settings.Connection.InPort;
	DWORD nOldOutSpeed	= Settings.Connection.OutSpeed;

	Settings.Connection.FirewallState	= m_wndCanAccept.GetCurSel();
	Settings.Connection.InHost			= m_sInHost;
	Settings.Connection.InPort			= m_nInPort < 65535 ? m_nInPort : 65535;

	// Obsolete for reference & deletion
	//if ( m_bEnableUPnP && ( (DWORD)m_nInPort != Settings.Connection.InPort || ! Settings.Connection.EnableUPnP ) )
	//{
	//	if ( ! Network.UPnPFinder )
	//		Network.UPnPFinder.Attach( new CUPnPFinder );
	//	if ( Network.UPnPFinder->AreServicesHealthy() )
	//		Network.UPnPFinder->StartDiscovery();
	//}

	Settings.Connection.RandomPort			= ( m_bInRandom && m_nInPort == 0 );
	Settings.Connection.EnableUPnP			= m_bEnableUPnP != FALSE;
	Settings.Connection.InBind				= m_bInBind != FALSE;
	Settings.Connection.OutHost				= m_sOutHost;
	Settings.Connection.InSpeed				= (DWORD)Settings.ParseVolume( m_sInSpeed, Kilobits );
	Settings.Connection.OutSpeed			= (DWORD)Settings.ParseVolume( m_sOutSpeed, Kilobits );
	Settings.Connection.IgnoreLocalIP		= m_bIgnoreLocalIP != FALSE;
	Settings.Connection.TimeoutConnect		= m_nTimeoutConnection * 1000;
	Settings.Connection.TimeoutHandshake	= m_nTimeoutHandshake  * 1000;

	if ( Settings.Connection.OutSpeed != nOldOutSpeed )
	{
		// Reset upload limit to 90% of capacity, trimmed down to nearest KB.
		Settings.Bandwidth.Uploads = ( Settings.Connection.OutSpeed / 8 ) *
			( ( 100 - Settings.Uploads.FreeBandwidthFactor ) / 100 ) * 1024;
	}

	UpdateData();

	// Warn the user about upload limiting and ed2k/BT downloads
	if ( ! Settings.Live.UploadLimitWarning &&
		( Settings.eDonkey.Enabled || Settings.eDonkey.EnableAlways || Settings.BitTorrent.EnableAlways ) )
	{
		QWORD nDownload = max( Settings.Bandwidth.Downloads, Settings.Connection.InSpeed * Kilobits / Bytes );
		QWORD nUpload   = Settings.Connection.OutSpeed * Kilobits / Bytes;
		if ( Settings.Bandwidth.Uploads > 0 )
			nUpload = min( Settings.Bandwidth.Uploads, nUpload );

		if ( nUpload * 16 < nDownload )
		{
			CHelpDlg::Show( L"GeneralHelp.UploadWarning" );
			Settings.Live.UploadLimitWarning = true;
		}
	}

	if ( nOldInPort != Settings.Connection.InPort )
	{
		CWaitCursor wc;
		Network.Disconnect();
		Network.Connect( TRUE );
	}
	else if ( bOldEnableUPnP != Settings.Connection.EnableUPnP )
	{
		Network.DeletePorts();
	}

	CSettingsPage::OnOK();
}

CString CConnectionSettingsPage::GetInOutHostTranslation()
{
	CString strAutomatic, strInCombo, strOutCombo, strNew;

	LoadString( strAutomatic, IDS_SETTINGS_AUTOMATIC_IP );

	m_wndInHost.GetLBText( 0, strInCombo );
	CComboBox* pOutHost = (CComboBox*) GetDlgItem( IDC_OUTBOUND_HOST );
	pOutHost->GetLBText( 0, strOutCombo );

	// Get non-english string, if any
	strNew = strInCombo.CompareNoCase( L"Automatic" ) == 0 ? strOutCombo : strInCombo;
	return strAutomatic.CompareNoCase( L"Automatic" ) == 0 ? strNew : strAutomatic;
}

void CConnectionSettingsPage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CSettingsPage::OnShowWindow(bShow, nStatus);

	if ( ! bShow ) return;

	// Update the bandwidth combo values

	// Update speed units
	m_sOutSpeed	= Settings.SmartSpeed( Settings.Connection.OutSpeed, Kilobits );
	m_sInSpeed	= Settings.SmartSpeed( Settings.Connection.InSpeed, Kilobits );

	// Remove any existing strings
	m_wndInSpeed.ResetContent();
	m_wndOutSpeed.ResetContent();

	// Add the new ones
	const DWORD nSpeeds[] =
	{
		56, 128, 256, 384, 512, 640, 768, 1024, 1550, 2048, 3072,
		4096, 5120, 8192, 10240, 12288, 24576, 45000, 102400, 155000
	};

	for ( int nSpeed = 0 ; nSpeed < sizeof( nSpeeds ) / sizeof( DWORD ) ; nSpeed++ )
	{
		CString strSpeed = Settings.SmartSpeed( nSpeeds[ nSpeed ], Kilobits );
		if ( Settings.ParseVolume( strSpeed, Kilobits )
			&& m_wndInSpeed.FindStringExact( -1, strSpeed ) == CB_ERR )
		{
			m_wndInSpeed.AddString( strSpeed );
			m_wndOutSpeed.AddString( strSpeed );
		}
	}

	UpdateData( FALSE );
}

void CConnectionSettingsPage::OnClickedEnableUpnp()
{
	if ( ! m_bEnableUPnP )
	{
		// Obsolete for reference & deletion
		//if ( ! theApp.m_pUPnPFinder )
		//	theApp.m_pUPnPFinder.Attach( new CUPnPFinder );

		// If the UPnP Device Host service is not running ask the user to start it.
		// It is not wise to have a delay up to 1 minute, especially that we would
		// need to wait until this and SSDP service are started.
		// If the upnphost service can not be started Envy will lock up.

		if ( ! IsServiceHealthy( L"upnphost" ) )
		{
			CButton* pBox = (CButton*)GetDlgItem( IDC_ENABLE_UPNP );
			pBox->SetCheck( BST_UNCHECKED );
			MsgBox( IDS_UPNP_SERVICES_ERROR, MB_OK | MB_ICONEXCLAMATION );
		}
	}

	UpdateData();
}
