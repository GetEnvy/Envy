//
// PageSettingsBitTorrent.cpp
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
#include "PageSettingsBitTorrent.h"
#include "WndMain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CBitTorrentSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CBitTorrentSettingsPage, CSettingsPage)
	ON_BN_CLICKED(IDC_TORRENT_AUTOCLEAR, &CBitTorrentSettingsPage::OnTorrentsAutoClear)
	ON_BN_CLICKED(IDC_TORRENTS_BROWSE, &CBitTorrentSettingsPage::OnTorrentsBrowse)
	ON_BN_CLICKED(IDC_TORRENTS_TORRENTMAKERBROWSE, &CBitTorrentSettingsPage::OnMakerBrowse)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBitTorrentSettingsPage property page

CBitTorrentSettingsPage::CBitTorrentSettingsPage()
	: CSettingsPage(CBitTorrentSettingsPage::IDD)
	, m_bEnableDHT		( TRUE )
	, m_bEndGame		( FALSE )
	, m_bPrefBTSources	( TRUE )
	, m_bAutoClear		( FALSE )
	, m_nClearPercentage ( 0 )
	, m_nDownloads		( 0 )
	, m_nLinks			( 0 )
{
}

CBitTorrentSettingsPage::~CBitTorrentSettingsPage()
{
}

void CBitTorrentSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ENABLE_DHT, m_bEnableDHT);
	DDX_Check(pDX, IDC_TORRENT_PREFERENCE, m_bPrefBTSources);
	DDX_Check(pDX, IDC_TORRENT_ENDGAME, m_bEndGame);
	DDX_Text(pDX, IDC_TORRENT_CLIENTLINKS, m_nLinks);
	DDX_Control(pDX, IDC_TORRENT_LINKS_SPIN, m_wndLinksSpin);
	DDX_Text(pDX, IDC_TORRENT_DOWNLOADS, m_nDownloads);
	DDX_Control(pDX, IDC_TORRENT_DOWNLOADS_SPIN, m_wndDownloadsSpin);
	DDX_Check(pDX, IDC_TORRENT_AUTOCLEAR, m_bAutoClear);
	DDX_Control(pDX, IDC_TORRENT_CLEAR_PERCENTAGE, m_wndClearPercentage);
	DDX_Control(pDX, IDC_TORRENT_CLEAR_SPIN, m_wndClearPercentageSpin);
	DDX_Text(pDX, IDC_TORRENT_CLEAR_PERCENTAGE, m_nClearPercentage);
	DDX_Text(pDX, IDC_TORRENT_DEFAULTTRACKER, m_sTracker);
	DDX_Control(pDX, IDC_TORRENTS_BROWSE, m_wndTorrentPath);
	DDX_Text(pDX, IDC_TORRENTS_FOLDER, m_sTorrentPath);
	DDX_Control(pDX, IDC_TORRENTS_TORRENTMAKERBROWSE, m_wndMakerPath);
	DDX_Text(pDX, IDC_TORRENTS_TORRENTMAKER, m_sToolPath);
}

/////////////////////////////////////////////////////////////////////////////
// CBitTorrentSettingsPage message handlers

BOOL CBitTorrentSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();
	m_bEnableDHT		= Settings.BitTorrent.EnableDHT;
	m_bEndGame			= Settings.BitTorrent.Endgame;
	m_bPrefBTSources	= Settings.BitTorrent.PreferenceBTSources;
	m_bAutoClear		= Settings.BitTorrent.AutoClear;
	m_nClearPercentage	= Settings.BitTorrent.ClearRatio;
	m_nLinks			= Settings.BitTorrent.DownloadConnections;
	m_nDownloads		= Settings.BitTorrent.DownloadTorrents;
	m_sTracker			= Settings.BitTorrent.DefaultTracker;
	m_sTorrentPath		= Settings.Downloads.TorrentPath;
	m_sToolPath			= Settings.BitTorrent.TorrentCreatorPath;

	m_wndTorrentPath.SetIcon( IDI_BROWSE );
	m_wndMakerPath.SetIcon( IDI_BROWSE );

	m_wndClearPercentage.EnableWindow( m_bAutoClear );

	DWORD nMaxTorrents = ( Settings.GetOutgoingBandwidth() / 2 ) + 2;
	nMaxTorrents = min( 10ul, nMaxTorrents );

	m_wndClearPercentageSpin.SetRange( 100, 999 );

	m_wndLinksSpin.SetRange( 0, 200 );
	m_wndDownloadsSpin.SetRange( 0, (WORD)nMaxTorrents );
	UpdateData( FALSE );

	m_wndTorrentFolder.SubclassDlgItem( IDC_TORRENTS_FOLDER, this );

	return TRUE;
}

BOOL CBitTorrentSettingsPage::OnSetActive()
{
	DWORD nMaxTorrents = ( Settings.GetOutgoingBandwidth() / 2 ) + 2;
	nMaxTorrents = min( 10ul, nMaxTorrents );

	m_nDownloads = min( m_nDownloads, (int)nMaxTorrents );
	m_wndDownloadsSpin.SetRange( 0, (WORD)nMaxTorrents );

	UpdateData( FALSE );

	return CSettingsPage::OnSetActive();
}

void CBitTorrentSettingsPage::OnTorrentsAutoClear()
{
	UpdateData();
	m_wndClearPercentage.EnableWindow( m_bAutoClear );
	m_wndClearPercentageSpin.EnableWindow( m_bAutoClear );
}

void CBitTorrentSettingsPage::OnTorrentsBrowse()
{
	CString strPath( BrowseForFolder( L"Select folder for torrents:",
		m_sTorrentPath ) );
	if ( strPath.IsEmpty() )
		return;

	UpdateData( TRUE );
	m_sTorrentPath = strPath;
	UpdateData( FALSE );
}

void CBitTorrentSettingsPage::OnMakerBrowse()
{
	CFileDialog dlg( TRUE, L"exe", L"TorrentEnvy.exe", OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
		L"Executable Files|*.exe|" + LoadString( IDS_FILES_ALL ) + L"|*.*||", this );

	if ( dlg.DoModal() != IDOK ) return;

	UpdateData( TRUE );
	m_sToolPath = dlg.GetPathName();
	UpdateData( FALSE );
}

void CBitTorrentSettingsPage::OnOK()
{
	UpdateData( TRUE );

	m_nClearPercentage = min( m_nClearPercentage, 999 );
	m_nClearPercentage = max( m_nClearPercentage, 100 );

	// Guestimate a good value based on available bandwidth
	if ( Settings.GetOutgoingBandwidth() < 16 )
		m_nLinks = min( m_nLinks, 200 );
	else if ( Settings.GetOutgoingBandwidth() < 32 )
		m_nLinks = min( m_nLinks, 300 );
	else if ( Settings.GetOutgoingBandwidth() < 64 )
		m_nLinks = min( m_nLinks, 500 );
	else
		m_nLinks = min( m_nLinks, 800 );

	m_nDownloads = min( m_nDownloads, (int)( ( Settings.GetOutgoingBandwidth() / 2 ) + 2 ) );

	UpdateData( FALSE );

	Settings.BitTorrent.EnableDHT			= m_bEnableDHT != FALSE;
	Settings.BitTorrent.Endgame				= m_bEndGame != FALSE;
	Settings.BitTorrent.PreferenceBTSources	= m_bPrefBTSources != FALSE;
	Settings.BitTorrent.DownloadConnections	= m_nLinks;
	Settings.BitTorrent.DownloadTorrents	= m_nDownloads;
	Settings.BitTorrent.AutoClear			= m_bAutoClear != FALSE;
	Settings.BitTorrent.ClearRatio			= m_nClearPercentage;
	Settings.BitTorrent.DefaultTracker		= m_sTracker;
	Settings.Downloads.TorrentPath			= m_sTorrentPath;
	Settings.BitTorrent.TorrentCreatorPath 	= m_sToolPath;

	if ( ! StartsWith( Settings.BitTorrent.DefaultTracker, _P( L"http://" ) ) &&
		 ! StartsWith( Settings.BitTorrent.DefaultTracker, _P( L"udp://" ) ) )
		Settings.SetDefault( &Settings.BitTorrent.DefaultTracker );

	CSettingsPage::OnOK();
}
