//
// DlgDownload.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2015
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
#include "EnvyURL.h"
#include "DlgDownload.h"
#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


IMPLEMENT_DYNAMIC(CDownloadDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CDownloadDlg, CSkinDialog)
	ON_EN_CHANGE(IDC_URL, OnChangeURL)
	ON_BN_CLICKED(IDC_TORRENT_FILE, OnTorrentFile)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadDlg dialog

CDownloadDlg::CDownloadDlg(CWnd* pParent, CDownload* pDownload) : CSkinDialog( CDownloadDlg::IDD, pParent )
{
	m_pDownload = pDownload;
}

void CDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TORRENT_FILE, m_wndTorrentFile);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_URL, m_wndURL);
	DDX_Text(pDX, IDC_URL, m_sURL);
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadDlg message handlers

BOOL CDownloadDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CDownloadDlg", IDR_DOWNLOADSFRAME );

	if ( theApp.GetClipboard( m_sURL ) )
	{
		m_sURL.Trim( L" \t\r\n\"" );

		CEnvyURL pURL;
		if ( ! pURL.Parse( m_sURL, m_pURLs ) )
			m_sURL.Empty();
	}

	UpdateData( FALSE );
	OnChangeURL();

	return TRUE;
}

void CDownloadDlg::OnChangeURL()
{
	UpdateData();

	CEnvyURL pURL;
	m_wndOK.EnableWindow( pURL.Parse( m_sURL, m_pURLs ) &&
		( m_pDownload == NULL ||
		pURL.m_nAction == CEnvyURL::uriSource ||
		pURL.m_nAction == CEnvyURL::uriDownload ) );
}

void CDownloadDlg::OnTorrentFile()
{
	UpdateData();

	CFileDialog dlg( TRUE, L"torrent", ( Settings.Downloads.TorrentPath + L"\\." ), OFN_HIDEREADONLY,
		L"Torrent Files|*.torrent|" + LoadString( IDS_FILES_ALL ) + L"|*.*||", this );

	if ( dlg.DoModal() != IDOK ) return;

	if ( ! m_pDownload )
	{
		theApp.OpenTorrent( dlg.GetPathName() );
	}
	else	// Update existing torrent
	{
		CBTInfo pInfo;
		if ( ! pInfo.LoadTorrentFile( dlg.GetPathName() ) )
			return;

		CSingleLock pTransfersLock( &Transfers.m_pSection );
		if ( SafeLock( pTransfersLock ) )
		{
			if ( Downloads.Check( m_pDownload ) )
				m_pDownload->SetTorrent( &pInfo );
		}
	}

	EndDialog( IDCANCEL );
}

void CDownloadDlg::OnOK()
{
	UpdateData();

	CEnvyURL pURL;
	if ( pURL.Parse( m_sURL, m_pURLs ) )
		CSkinDialog::OnOK();
}
