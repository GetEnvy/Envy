//
// DlgUpgrade.cpp
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
#include "DlgUpgrade.h"
#include "VersionChecker.h"
#include "EnvyURL.h"
#include "Downloads.h"
#include "Network.h"
#include "WndMain.h"
#include "WndDownloads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//BEGIN_MESSAGE_MAP(CUpgradeDlg, CSkinDialog)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUpgradeDlg dialog

CUpgradeDlg::CUpgradeDlg(CWnd* pParent) : CSkinDialog(CUpgradeDlg::IDD, pParent)
//	, m_sMessage	( L"" )
	, m_bCheck		( FALSE )
{
}

void CUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_DONT_CHECK, m_bCheck);
	DDX_Text(pDX, IDC_MESSAGE, m_sMessage);
}

/////////////////////////////////////////////////////////////////////////////
// CUpgradeDlg message handlers

BOOL CUpgradeDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CUpgradeDlg", IDR_MAINFRAME );

	m_sMessage	= Settings.VersionCheck.UpgradePrompt;
	m_bCheck	= FALSE;

	UpdateData( FALSE );

	return TRUE;
}

void CUpgradeDlg::OnOK()
{
	ParseCheckAgain();

	CEnvyURL pURL;

	pURL.m_nAction			= CEnvyURL::uriDownload;
	pURL.m_sName			= Settings.VersionCheck.UpgradeFile;
	pURL.m_sURL				= Settings.VersionCheck.UpgradeSources;
	pURL.m_oSHA1.fromString(  Settings.VersionCheck.UpgradeSHA1 );
	pURL.m_oTiger.fromString( Settings.VersionCheck.UpgradeTiger );

	if ( Settings.VersionCheck.UpgradeSize.GetLength() )
	{
		QWORD nSize = 0;
		if ( _stscanf( Settings.VersionCheck.UpgradeSize.GetString(), L"%I64i", &nSize ) == 1 && nSize )
			pURL.m_nSize = nSize;
	}

	Downloads.Add( pURL );

	if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );

	CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
	pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );

	CSkinDialog::OnOK();
}

void CUpgradeDlg::OnCancel()
{
	ParseCheckAgain();

	CSkinDialog::OnCancel();
}

void CUpgradeDlg::ParseCheckAgain()
{
	UpdateData();

	if ( m_bCheck )
		VersionChecker.SetNextCheck( 21 );		// ToDo: Set Update Delay
}
