//
// WizardNetworksPage.cpp
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
#include "WizardNetworksPage.h"
#include "Network.h"
#include "HostCache.h"
#include "Downloads.h"
#include "EnvyURL.h"
#include "DlgUpdateServers.h"
#include "DlgHelp.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CWizardNetworksPage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardNetworksPage, CWizardPage)
	ON_WM_XBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardNetworksPage property page

CWizardNetworksPage::CWizardNetworksPage()
	: CWizardPage(CWizardNetworksPage::IDD)
	, m_bG2Enable	( TRUE )
	, m_bG1Enable	( TRUE )
	, m_bEDEnable	( TRUE )
	, m_bHandleTorrents	( TRUE )
{
}

CWizardNetworksPage::~CWizardNetworksPage()
{
}

void CWizardNetworksPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_G2_ENABLE, m_bG2Enable);
	DDX_Check(pDX, IDC_G1_ENABLE, m_bG1Enable);
	DDX_Check(pDX, IDC_ED2K_ENABLE, m_bEDEnable);
	DDX_Check(pDX, IDC_URI_TORRENT, m_bHandleTorrents);
}

/////////////////////////////////////////////////////////////////////////////
// CWizardNetworksPage message handlers

BOOL CWizardNetworksPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	Skin.Apply( L"CWizardNetworksPage", this );

	m_bG2Enable = Settings.Gnutella2.EnableAlways;
	m_bG1Enable = Settings.Gnutella1.EnableAlways;
	m_bEDEnable = Settings.eDonkey.EnableAlways;
	m_bHandleTorrents = Settings.Web.Torrent;

	if ( Settings.Experimental.LAN_Mode )	// #ifdef LAN_MODE
	{
		GetDlgItem( IDC_G1_ENABLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_ED2K_ENABLE )->EnableWindow( FALSE );
	}

	UpdateData( FALSE );

	return TRUE;
}

BOOL CWizardNetworksPage::OnSetActive()
{
	// Wizard Window Caption Workaround
	CString strCaption;
	GetWindowText( strCaption );
	GetParent()->SetWindowText( strCaption );

	CoolInterface.FixThemeControls( this );		// Checkbox/Groupbox text colors (Remove theme if needed)

	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
	return CWizardPage::OnSetActive();
}

void CWizardNetworksPage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

LRESULT CWizardNetworksPage::OnWizardNext()
{
	UpdateData();

	if ( ! m_bG2Enable )
	{
		if ( MsgBox( IDS_NETWORK_DISABLE_G2, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
		{
			m_bG2Enable = TRUE;
			UpdateData( FALSE );
		}
	}

	if ( Settings.Web.Torrent != ( m_bHandleTorrents != FALSE ) )
	{
		Settings.Web.Torrent = m_bHandleTorrents != FALSE;
		CEnvyURL::Register();
	}

	Settings.Gnutella2.Enabled		= m_bG2Enable != FALSE;
	Settings.Gnutella2.EnableAlways	= m_bG2Enable != FALSE;
	Settings.Gnutella1.Enabled		= m_bG1Enable != FALSE;
	Settings.Gnutella1.EnableAlways	= m_bG1Enable != FALSE;
	Settings.eDonkey.Enabled		= m_bEDEnable != FALSE;
	Settings.eDonkey.EnableAlways	= m_bEDEnable != FALSE;

	// Load server.met from web if needed
	if ( m_bEDEnable && HostCache.eDonkey.GetCount() < 3 )
	{
		CUpdateServersDlg dlg;
	//	dlg.m_sURL = Settings.eDonkey.ServerListURL;
		dlg.DoModal();
	}

	// Load hublist.xml.bz2 from web if needed, various ways
	if ( HostCache.DC.GetCount() < 5 &&
		! ( PathFileExists( Settings.General.DataPath + L"hublist.xml.bz2" ) &&
			theApp.OpenImport( Settings.General.DataPath + L"hublist.xml.bz2" ) ) )
	{
		CUpdateServersDlg dlg;
		dlg.m_sURL = Settings.DC.HubListURL;
		if ( dlg.DoModal() != IDOK &&
			MsgBox( IDS_DOWNLOAD_DC_HUBLIST, MB_ICONQUESTION | MB_YESNO ) == IDYES )
		{
			CEnvyURL pURL;
		//	pURL.Parse( Settings.DC.HubListURL );
			pURL.m_sURL = Settings.DC.HubListURL;
			pURL.m_sName = L"hublist.xml.bz2";
			pURL.m_nAction = CEnvyURL::uriDownload;
			Downloads.Add( pURL );
		}
	}

	if ( ! Network.IsConnected() && ( m_bG2Enable || m_bG1Enable || m_bEDEnable ) )
		Network.Connect( TRUE );

	return 0;
}
