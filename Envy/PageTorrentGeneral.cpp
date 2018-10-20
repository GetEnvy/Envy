//
// PageTorrentGeneral.cpp
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

#include "DlgDownloadSheet.h"
#include "PageTorrentGeneral.h"
#include "Transfers.h"
#include "Downloads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CTorrentGeneralPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CTorrentGeneralPage, CPropertyPageAdv)
	ON_WM_PAINT()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentGeneralPage property page

CTorrentGeneralPage::CTorrentGeneralPage() :
	CPropertyPageAdv( CTorrentGeneralPage::IDD )
{
}

CTorrentGeneralPage::~CTorrentGeneralPage()
{
}

void CTorrentGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TORRENT_NAME, m_sName);
	DDX_Text(pDX, IDC_TORRENT_COMMENTS, m_sComment);
	DDX_Text(pDX, IDC_TORRENT_CREATEDBY, m_sCreatedBy );
	DDX_Text(pDX, IDC_TORRENT_CREATIONDATE, m_sCreationDate );
	DDX_Text(pDX, IDC_TORRENT_OTHER, m_sTorrentOther );
	DDX_Text(pDX, IDC_TORRENT_UPLOADTOTAL, m_sUploadTotal );
	DDX_Control(pDX, IDC_TORRENT_STARTDOWNLOADS, m_wndStartDownloads);
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentGeneralPage message handlers

BOOL CTorrentGeneralPage::OnInitDialog()
{
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;

	ASSUME_LOCK( Transfers.m_pSection );

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	ASSERT( pDownload && pDownload->IsTorrent() );

	CBTInfo& oInfo = pDownload->m_pTorrent;

	m_sName			= oInfo.m_sName;
	m_sComment		= oInfo.m_sComment;
	m_sCreatedBy	= oInfo.m_sCreatedBy;
	if ( oInfo.m_tCreationDate > 0 )
	{
		CTime pTime( (time_t)oInfo.m_tCreationDate );
		m_sCreationDate = pTime.Format( L"%Y-%m-%d  %H:%M" );
	}

	// Assembler 'other' string
	if ( oInfo.m_bPrivate )
	{
		m_sTorrentOther += LoadString( IDS_BT_PRIVATE );
		m_sTorrentOther += L", ";
	}
	if ( oInfo.HasEncodingError() )
	{
		m_sTorrentOther += LoadString( IDS_BT_ENCODING );
		m_sTorrentOther += L", ";
	}

	// Cut off last comma
	if ( ! m_sTorrentOther.IsEmpty() )
		m_sTorrentOther = m_sTorrentOther.Left( m_sTorrentOther.GetLength() - 2 );

	m_wndStartDownloads.SetCurSel( oInfo.m_nStartDownloads );

	m_sUploadTotal.Format( L" %s",
		(LPCTSTR)Settings.SmartVolume( oInfo.m_nTotalUpload ) );

	UpdateData( FALSE );

	return TRUE;
}

BOOL CTorrentGeneralPage::OnApply()
{
	if ( ! UpdateData() )
		return FALSE;

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return FALSE;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload )
		return CPropertyPageAdv::OnApply();		// Invalid download

	CBTInfo& oInfo = pDownload->m_pTorrent;

	// Update the starting of torrent transfers
	oInfo.m_nStartDownloads = m_wndStartDownloads.GetCurSel();

	return CPropertyPageAdv::OnApply();
}
