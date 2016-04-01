//
// CtrlDownloadTip.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2007
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
#include "CtrlDownloadTip.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Flags.h"
#include "ShellIcons.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferBT.h"
#include "DownloadTransferED2K.h"
#include "EDClient.h"
#include "FragmentedFile.h"
#include "FragmentBar.h"
#include "GraphLine.h"
#include "GraphItem.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CDownloadTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CDownloadTipCtrl, CCoolTipCtrl)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl construction

CDownloadTipCtrl::CDownloadTipCtrl()
	: m_pDownload	( NULL )
	, m_pSource 	( NULL )
	, m_pGraph		( NULL )
	, m_nIcon		( 0 )
	, m_nStatWidth	( 0 )
	, m_bDrawGraph	( FALSE )
{
}

CDownloadTipCtrl::~CDownloadTipCtrl()
{
	if ( m_pGraph ) delete m_pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl events

BOOL CDownloadTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;

	CalcSizeHelper();

	return m_sz.cx > 0;
}

void CDownloadTipCtrl::OnCalcSize(CDC* pDC)
{
	if ( m_pDownload && Downloads.Check( m_pDownload ) )
		OnCalcSize( pDC, m_pDownload );
	else if ( m_pSource && Downloads.Check( m_pSource ) )
		OnCalcSize( pDC, m_pSource );

	m_sz.cx = min( max( m_sz.cx, 400 ), GetSystemMetrics( SM_CXSCREEN ) / 2 );
}

void CDownloadTipCtrl::OnShow()
{
	if ( m_pGraph ) delete m_pGraph;

	m_pGraph = CreateLineGraph();
	m_pItem  = new CGraphItem( 0, 1.0f, RGB( 0, 0, 0xFF ) );
	m_pGraph->AddItem( m_pItem );

	CSingleLock pLock( &Transfers.m_pSection );
	if ( SafeLock( pLock ) )
		TrackerRequests.Request( m_pDownload, BTE_TRACKER_SCRAPE, 0, this );
	//	pLock.Unlock();
}

void CDownloadTipCtrl::OnHide()
{
	if ( m_pGraph ) delete m_pGraph;
	m_pGraph = NULL;
	m_pItem  = NULL;
}

void CDownloadTipCtrl::OnPaint(CDC* pDC)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	if ( m_pDownload && Downloads.Check( m_pDownload ) )
		OnPaint( pDC, m_pDownload );
	else if ( m_pSource && Downloads.Check( m_pSource ) )
		OnPaint( pDC, m_pSource );
	else
		Hide();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl download case

void CDownloadTipCtrl::OnCalcSize(CDC* pDC, CDownload* pDownload)
{
	if ( ! pDownload ) return;

	PrepareDownloadInfo( pDownload );

	AddSize( pDC, m_sName );
	m_sz.cy = TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( ! m_sSHA1.IsEmpty() )
	{
		AddSize( pDC, m_sSHA1 );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( ! m_sED2K.IsEmpty() )
	{
		AddSize( pDC, m_sED2K );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( ! m_sBTH.IsEmpty() )
	{
		AddSize( pDC, m_sBTH );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( ! m_sMD5.IsEmpty() )
	{
		AddSize( pDC, m_sMD5 );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( ! m_sTiger.IsEmpty() )
	{
		AddSize( pDC, m_sTiger );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	m_sz.cy += TIP_RULE;
	AddSize( pDC, m_sSize, 80 );
	AddSize( pDC, m_sType, 80 );
	m_sz.cy += 34;	// Icon
	m_sz.cy += TIP_RULE;

	// File error
	if ( pDownload->GetFileError() != ERROR_SUCCESS )
	{
		if ( ! pDownload->GetFileErrorString().IsEmpty() )
		{
			AddSize( pDC, pDownload->GetFileErrorString() );
			m_sz.cy += TIP_TEXTHEIGHT;
		}
		AddSize( pDC, GetErrorString( pDownload->GetFileError() ) );
		m_sz.cy += TIP_TEXTHEIGHT;
		m_sz.cy += TIP_RULE;
	}

	if ( pDownload->IsTorrent() )
	{
		// Torrent Tracker error
		if ( pDownload->m_bTorrentTrackerError &&
			 pDownload->m_sTorrentTrackerError.GetLength() > 1 )		// ToDo: Rare crash for IsEmpty?
		{
			AddSize( pDC, pDownload->m_sTorrentTrackerError );
			m_sz.cy += TIP_TEXTHEIGHT;
			m_sz.cy += TIP_RULE;
		}

		m_sz.cy += TIP_TEXTHEIGHT;		// Torrent ratio
	}

	if ( pDownload->IsTasking() )		// Moving or Merging Progress
	{
		m_sz.cy += TIP_TEXTHEIGHT;
		m_sz.cy += TIP_RULE;
	}

	if ( pDownload->IsSeeding() )
		m_sz.cy += TIP_TEXTHEIGHT;		// Seed: Just Sources (for Seeds/Peers)
	else if ( pDownload->IsCompleted() )
		m_sz.cy += TIP_TEXTHEIGHT * 2;	// Done: ETA and downloaded
	else
		m_sz.cy += TIP_TEXTHEIGHT * 4;	// Speed, ETA, Downloaded, No. Sources

	// Number of reviews
	if ( pDownload->GetReviewCount() > 0 )
		m_sz.cy += TIP_TEXTHEIGHT;

	// URL
	if ( ! m_sURL.IsEmpty() )
	{
		m_sz.cy += TIP_RULE;
		AddSize( pDC, m_sURL );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	// Progress bar (not applicable for seeding torrents)
	if ( ! pDownload->IsSeeding() )
	{
		m_sz.cy += TIP_GAP;
		m_sz.cy += TIP_BARHEIGHT;
	}

	// Graph (Only for files in progress)
	if ( pDownload->IsCompleted() )
	{
		m_bDrawGraph = FALSE;
	}
	else
	{
		m_sz.cy += TIP_GAP;
		m_sz.cy += TIP_GRAPHHEIGHT;
		m_bDrawGraph = TRUE;
	}

	// Position dynamic numbers offset based on max static text label width
	m_nStatWidth = pDC->GetTextExtent( LoadString( IDS_MONITOR_TOTAL_SPEED ) ).cx + 12;
	const int nWidth = pDC->GetTextExtent( LoadString( IDS_MONITOR_ESTIMATED_TIME ) ).cx + 12;
	if ( m_nStatWidth < nWidth )
		m_nStatWidth = nWidth;
}

void CDownloadTipCtrl::OnPaint(CDC* pDC, CDownload* pDownload)
{
	CPoint pt( 0, 0 );
	CSize sz( m_sz.cx, TIP_TEXTHEIGHT );

	const CString strOf   = LoadString( IDS_GENERAL_OF );
	const CString strSize = LoadString( IDS_TIP_SIZE ) + L": ";
	const CString strType = LoadString( IDS_TIP_TYPE ) + L": ";

	DrawText( pDC, &pt, m_sName, &sz );
	pt.y += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( ! m_sSHA1.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sSHA1 );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! m_sTiger.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sTiger );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! m_sED2K.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sED2K );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! m_sBTH.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sBTH );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! m_sMD5.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sMD5 );
		pt.y += TIP_TEXTHEIGHT;
	}

	DrawRule( pDC, &pt );

	ShellIcons.Draw( pDC, m_nIcon, 32, pt.x, pt.y, ( Images.m_bmToolTip.m_hObject ) ? CLR_NONE : Colors.m_crTipBack );
	pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );

	pt.y += 2;
	pt.x += 40;		// Icon offset
	const int nDataOffset = max( pDC->GetTextExtent( strSize ).cx, pDC->GetTextExtent( strType ).cx ) + 3;

	DrawText( pDC, &pt, strSize );
	pt.x += nDataOffset;
	DrawText( pDC, &pt, m_sSize );
	pt.x -= nDataOffset;
	pt.y += TIP_TEXTHEIGHT;
	DrawText( pDC, &pt, strType );
	pt.x += nDataOffset;
	DrawText( pDC, &pt, m_sType );
	pt.x -= nDataOffset + 40;
	pt.y -= TIP_TEXTHEIGHT + 2;
	pt.y += 34;		// Icon

	DrawRule( pDC, &pt );

	const int nSourceCount		= pDownload->GetSourceCount();
	const int nTransferCount	= pDownload->GetTransferCount();
	const int nReviewCount		= pDownload->GetReviewCount();

	CString strFormat, strETA, strSpeed, strVolume, strSources, strReviews, strTorrentUpload;
	LoadString( strFormat, IDS_TIP_NA );

	if ( pDownload->IsMoving() )
	{
		LoadString( strETA, IDS_MONITOR_COMPLETED_WORD );
		strSpeed = strFormat;
		LoadString( strSources, IDS_MONITOR_COMPLETED_WORD );
	}
	else if ( pDownload->IsPaused() )
	{
		strETA = strFormat;
		strSpeed = strFormat;
		strSources.Format( L"%i", nSourceCount );
	}
	else if ( nTransferCount )
	{
		const DWORD nTime = pDownload->GetTimeRemaining();

		if ( nTime != 0xFFFFFFFF )
		{
			if ( nTime > 86400 )
			{
				LoadString( strFormat, IDS_MONITOR_TIME_DH );
				strETA.Format( strFormat, nTime / 86400, ( nTime / 3600 ) % 24 );
			}
			else if ( nTime > 3600 )
			{
				LoadString( strFormat, IDS_MONITOR_TIME_HM );
				strETA.Format( strFormat, nTime / 3600, ( nTime % 3600 ) / 60 );
			}
			else if ( nTime > 60 )
			{
				LoadString( strFormat, IDS_MONITOR_TIME_MS );
				strETA.Format( strFormat, nTime / 60, nTime % 60 );
			}
			else
			{
				LoadString( strFormat, IDS_MONITOR_TIME_S );
				strETA.Format( strFormat, nTime % 60 );
			}
		}

		strSpeed = Settings.SmartSpeed( pDownload->GetAverageSpeed() );

		if ( pDownload->IsBoosted() )
		{
			LoadString( strFormat, IDS_TIP_PRIORITY );
			strSpeed += L"   " + strFormat;
		}

		strSources.Format( L"%i %s %i", nTransferCount, strOf, nSourceCount );
		if ( Settings.General.LanguageRTL ) strSources = L"\x202B" + strSources;
	}
	else if ( nSourceCount )
	{
		strETA = strSpeed = strFormat;
		strSources.Format( L"%i", nSourceCount );
	}
	else
	{
		strETA = strSpeed = strFormat;
		LoadString( strSources, IDS_MONITOR_NO_SOURCES );
	}

	// Update Torrent Seeds/Peers from last Tracker Scrape
	if ( pDownload->IsTorrent() && ( pDownload->m_pTorrent.m_nTrackerSeeds || pDownload->m_pTorrent.m_nTrackerPeers ) )
	{
		m_sSeedsPeers.Format( L"   ( %u seeds %u peers )",	// ToDo: Translation ?
			pDownload->m_pTorrent.m_nTrackerSeeds, pDownload->m_pTorrent.m_nTrackerPeers );

		if ( pDownload->IsSeeding() )
			strSources = m_sSeedsPeers.Mid(2);	// "  ( "
		else
			strSources += m_sSeedsPeers;
	}

	if ( nReviewCount > 0 )
		strReviews.Format( L"%i", nReviewCount );

	if ( pDownload->IsStarted() && pDownload->m_nSize < SIZE_UNKNOWN )
	{
		if ( Settings.General.LanguageRTL )
		{
			strVolume.Format( L"( %.2f%% )  %s %s %s",
				pDownload->GetProgress(),
				(LPCTSTR)Settings.SmartVolume( pDownload->m_nSize ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( pDownload->GetVolumeComplete() ) );
		}
		else
		{
			strVolume.Format( L"%s %s %s   ( %.2f%% )",
				(LPCTSTR)Settings.SmartVolume( pDownload->GetVolumeComplete() ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( pDownload->m_nSize ),
				pDownload->GetProgress() );
		}
	}
	else
	{
		LoadString( strVolume, IDS_TIP_NA );
	}

	if ( pDownload->IsTorrent() )
	{
		if ( Settings.General.LanguageRTL )
		{
			strTorrentUpload.Format( L"( %.2f%% )  %s %s %s",
				pDownload->GetRatio(),
				(LPCTSTR)Settings.SmartVolume( pDownload->m_pTorrent.m_nTotalDownload ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( pDownload->m_pTorrent.m_nTotalUpload ) );
		}
		else
		{
			strTorrentUpload.Format( L"%s %s %s   ( %.2f%% )",
				(LPCTSTR)Settings.SmartVolume( pDownload->m_pTorrent.m_nTotalUpload ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( pDownload->m_pTorrent.m_nTotalDownload ),
				pDownload->GetRatio() );
		}
	}

	// Draw the pop-up box

	const int nTextAlign = 3;	// Offset text labels a few pixels

	// File error
	if ( pDownload->GetFileError() != ERROR_SUCCESS )
	{
		COLORREF crOld = pDC->SetTextColor( Colors.m_crTextAlert );
		if ( ! pDownload->GetFileErrorString().IsEmpty() )
		{
			DrawText( pDC, &pt, pDownload->GetFileErrorString(), 3 );
			pt.y += TIP_TEXTHEIGHT;
		}
		DrawText( pDC, &pt, GetErrorString( pDownload->GetFileError() ), 3 );
		pDC->SetTextColor( crOld );
		pt.y += TIP_TEXTHEIGHT;
		DrawRule( pDC, &pt );
	}

	// Tracker error
	if ( pDownload->m_bTorrentTrackerError && pDownload->m_sTorrentTrackerError.GetLength() > 1 )
	{
		DrawText( pDC, &pt, pDownload->m_sTorrentTrackerError, nTextAlign );
		pt.y += TIP_TEXTHEIGHT;
		DrawRule( pDC, &pt );
	}

	if ( pDownload->IsTasking() )
	{
		// Moving or Merging
		CString strProgress = LoadString( IDS_STATUS_MOVING ) + L":";
		DrawText( pDC, &pt, strProgress, nTextAlign );
		strProgress.Format( L"%.2f%%", pDownload->GetProgress() );
		DrawText( pDC, &pt, strProgress, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
		DrawRule( pDC, &pt );
	}

	if ( ! pDownload->IsCompleted() )
	{
		// Speed. Not for completed files
		LoadString( strFormat, IDS_MONITOR_TOTAL_SPEED );
		DrawText( pDC, &pt, strFormat, nTextAlign );
		DrawText( pDC, &pt, strSpeed, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! pDownload->IsSeeding() )
	{
		// ETA. Not for seeding torrents.
		LoadString( strFormat, IDS_MONITOR_ESTIMATED_TIME );
		DrawText( pDC, &pt, strFormat, nTextAlign );
		DrawText( pDC, &pt, strETA, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;

		// Volume downloaded. Not for seeding torrents
		LoadString( strFormat, IDS_MONITOR_VOLUME_DOWNLOADED );
		DrawText( pDC, &pt, strFormat, nTextAlign );
		DrawText( pDC, &pt, strVolume, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( pDownload->IsTorrent() )
	{
		// Upload ratio. Only for torrents
		LoadString( strFormat, IDS_MONITOR_VOLUME_UPLOADED );
		DrawText( pDC, &pt, strFormat, nTextAlign );
		DrawText( pDC, &pt, strTorrentUpload, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! pDownload->IsCompleted() || pDownload->IsSeeding() )
	{
		// No. Sources. Not for completed files, except seeds.
		LoadString( strFormat, IDS_MONITOR_NUMBER_SOURCES );
		DrawText( pDC, &pt, strFormat, nTextAlign );
		DrawText( pDC, &pt, strSources, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( nReviewCount > 0 )
	{
		// No. Reviews
		LoadString( strFormat, IDS_MONITOR_NUMBER_REVIEWS );
		DrawText( pDC, &pt, strFormat, nTextAlign );
		DrawText( pDC, &pt, strReviews, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}

	// Draw URL if present
	if ( ! m_sURL.IsEmpty() )
	{
		DrawRule( pDC, &pt );
		DrawText( pDC, &pt, m_sURL );
		pt.y += TIP_TEXTHEIGHT;
	}

	// Progress Bar (not for seeding torrents)
	if ( ! pDownload->IsSeeding() )
	{
		pt.y += TIP_GAP;
		DrawProgressBar( pDC, &pt, pDownload );
		pt.y += TIP_GAP;
	}

	// Don't draw empty graph
	if ( m_bDrawGraph && m_pGraph )
	{
		CRect rc( pt.x, pt.y, m_sz.cx, pt.y + TIP_GRAPHHEIGHT );
		pDC->Draw3dRect( &rc, Colors.m_crTipBorder, Colors.m_crTipBorder );
		rc.DeflateRect( 1, 1 );
		m_pGraph->BufferedPaint( pDC, &rc );
		rc.InflateRect( 1, 1 );
		pDC->ExcludeClipRect( &rc );
		pt.y += TIP_GRAPHHEIGHT;
	}
}

void CDownloadTipCtrl::PrepareDownloadInfo(CDownload* pDownload)
{
	PrepareFileInfo( pDownload );

	if ( Settings.General.GUIMode == GUI_BASIC )
		return;

	// We also report if we have a hashset, and if hash is trusted (Debug mode only)
	CString strNoHashset, strUntrusted;
	LoadString( strNoHashset, IDS_TIP_NOHASHSET );
	LoadString( strUntrusted, IDS_TIP_UNTRUSTED );

	m_sSHA1 = pDownload->m_oSHA1.toShortUrn();
	if ( ! m_sSHA1.IsEmpty() && ! pDownload->m_bSHA1Trusted )
		m_sSHA1 += L" (" + strUntrusted + L")";

	m_sTiger = pDownload->m_oTiger.toShortUrn();
	if ( ! m_sTiger.IsEmpty() )
	{
		if ( ! pDownload->m_pTigerBlock )
		{
			if ( pDownload->m_bTigerTrusted )
				m_sTiger += L" (" + strNoHashset + L")";
			else
				m_sTiger += L" (" + strNoHashset + L", " + strUntrusted + L")";
		}
		else if ( ! pDownload->m_bTigerTrusted )
		{
			m_sTiger += L" (" + strUntrusted + L")";
		}
	}

	m_sED2K = pDownload->m_oED2K.toShortUrn();
	if ( ! m_sED2K.IsEmpty() )
	{
		if ( ! pDownload->m_pHashsetBlock )
		{
			if ( pDownload->m_bED2KTrusted )
				m_sED2K += L" (" + strNoHashset + L")";
			else
				m_sED2K += L" (" + strNoHashset + L", " + strUntrusted + L")";
		}
		else if ( ! pDownload->m_bED2KTrusted )
		{
			m_sED2K += L" (" + strUntrusted + L")";
		}
	}

	m_sBTH = pDownload->m_oBTH.toShortUrn();
	if ( ! m_sBTH.IsEmpty() )
	{
		if ( ! pDownload->m_pTorrentBlock )
		{
			if ( pDownload->m_bBTHTrusted )
				m_sBTH += L" (" + strNoHashset + L")";
			else
				m_sBTH += L" (" + strNoHashset + L", " + strUntrusted + L")";
		}
		else if ( ! pDownload->m_bBTHTrusted )
		{
			m_sBTH += L" (" + strUntrusted + L")";
		}
	}

	m_sMD5 = pDownload->m_oMD5.toShortUrn();
	if ( ! m_sMD5.IsEmpty() )
	{
		if ( ! pDownload->m_bMD5Trusted )
			m_sMD5+= L" (" + strUntrusted + L")";
	}

	// Prepare torrent data for display
	if ( pDownload->IsTorrent() )
	{
		m_sURL = pDownload->m_pTorrent.GetTrackerAddress();

		if ( pDownload->m_pTorrent.m_nTrackerSeeds || pDownload->m_pTorrent.m_nTrackerPeers )
			m_sSeedsPeers.Format( L"   ( %u seeds %u peers )",	// ToDo: Translation ?
				pDownload->m_pTorrent.m_nTrackerSeeds, pDownload->m_pTorrent.m_nTrackerPeers );
	}

	// Moved Scrape to CBTInfo:
	//if ( m_pDownload && pDownload->IsTorrent() )
	//{
	//	CString strURL = m_sURL;
	//	if ( strURL.Find( L"http" ) == 0 &&
	//		strURL.Replace( L"/announce", L"/scrape" ) == 1 )
	//	{
	//		CSingleLock oLock( &Transfers.m_pSection );
	//		if ( ! oLock.Lock( 500 ) ) return;
	//
	//		// Fetch scrape only for the given info hash
	//		strURL = strURL.TrimRight( L'&' ) +
	//			( ( strURL.Find( L'?' ) != -1 ) ? L'&' : L'?' ) +
	//			L"info_hash=" + CBTTrackerRequest::Escape( m_pDownload->m_pTorrent.m_oBTH ) +
	//			L"&peer_id="  + CBTTrackerRequest::Escape( m_pDownload->m_pPeerID );
	//
	//		oLock.Unlock();
	//
	//		CHttpRequest pRequest;
	//		pRequest.SetURL( strURL );
	//		pRequest.AddHeader( L"Accept-Encoding", L"deflate, gzip" );
	//		pRequest.EnableCookie( false );
	//		pRequest.SetUserAgent( Settings.SmartAgent() );
	//
	//		if ( pRequest.Execute( FALSE ) && pRequest.InflateResponse() )
	//		{
	//			CBuffer* pResponse = pRequest.GetResponseBuffer();
	//
	//			if ( pResponse != NULL && pResponse->m_pBuffer != NULL )
	//			{
	//				if ( CBENode* pNode = CBENode::Decode( pResponse ) )
	//				{
	//					theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"[BT] Received BitTorrent tracker response: %s", pNode->Encode() );
	//
	//					if ( ! oLock.Lock( 300 ) ) return;
	//					if ( ! Downloads.Check( m_pDownload ) || ! m_pDownload->IsTorrent() ) return;
	//					LPBYTE nKey = &m_pDownload->m_pTorrent.m_oBTH[ 0 ];
	//					oLock.Unlock();
	//
	//					CBENode* pFiles = pNode->GetNode( "files" );
	//					CBENode* pFile = pFiles->GetNode( nKey, Hashes::BtHash::byteCount );
	//					if ( ! pFile->IsType( CBENode::beDict ) ) return;
	//
	//					int nTrackerSeeds = 0;
	//					int nTrackerPeers = 0;
	//
	//					if ( CBENode* pSeeds = pFile->GetNode( "complete" ) )
	//					{
	//						if ( ! pSeeds->IsType( CBENode::beInt ) ) return;
	//						nTrackerSeeds = (int)( pSeeds->GetInt() & ~0xFFFF0000 ); 	// QWORD Caution: Don't get negative values from buggy trackers
	//					}
	//
	//					if ( CBENode* pPeers = pFile->GetNode( "incomplete" ) )
	//					{
	//						if ( ! pPeers->IsType( CBENode::beInt ) ) return;
	//						nTrackerPeers = (int)( pPeers->GetInt() & ~0xFFFF0000 );
	//					}
	//
	//					if ( nTrackerSeeds > 0 || nTrackerPeers > 0 )
	//						m_sSeedsPeers.Format( L"   ( %i seeds %i peers )", nTrackerSeeds, nTrackerPeers );
	//
	//					delete pNode;
	//				}
	//			}
	//		}
	//	}
	//}
}

void CDownloadTipCtrl::PrepareFileInfo(CDownload* pDownload)	// CEnvyFile
{
	m_sName = pDownload->m_sName;
	m_sSize = Settings.SmartVolume( pDownload->m_nSize );
	if ( pDownload->m_nSize == SIZE_UNKNOWN )
		m_sSize = L"?";

	m_sSHA1.Empty();
	m_sTiger.Empty();
	m_sED2K.Empty();
	m_sBTH.Empty();
	m_sMD5.Empty();
	m_sURL.Empty();

	// Special-case multifile icon handling
	if ( pDownload->IsMultiFileTorrent() )
	{
		if ( Settings.General.LanguageDefault )
			m_sType.Format( L"BitTorrent  ( %u files )", pDownload->m_pTorrent.GetCount() );
		else
			m_sType.Format( L"BitTorrent  ( %u %s )", pDownload->m_pTorrent.GetCount(), LoadString( IDS_FILES ) );
		m_nIcon = ShellIcons.Get( L".torrent", 32 );							// ToDo: IDI_MULTIFILE ?
		return;
	}

	m_nIcon = ShellIcons.Get( m_sName, 32 );
	m_sType = ShellIcons.GetTypeString( m_sName );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl source case

void CDownloadTipCtrl::OnCalcSize(CDC* pDC, CDownloadSource* pSource)
{
	// Is this a firewalled eDonkey client
	if ( pSource->m_nProtocol == PROTOCOL_ED2K && pSource->m_bPushOnly == TRUE )
	{
		m_sName.Format( L"%lu@%s:%u",
			pSource->m_pAddress.S_un.S_addr,
			(LPCTSTR)CString( inet_ntoa( pSource->m_pServerAddress ) ),
			pSource->m_nServerPort );
	}
	else if ( ! pSource->IsIdle() )	// Or an active transfer
	{
		m_sName.Format( L"%s:%u",
			(LPCTSTR)pSource->GetAddress(),
			ntohs( pSource->GetPort() ) );
	}
	else	// Or just queued
	{
		m_sName.Format( L"%s:%u",
			(LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ),
			pSource->m_nPort );
	}

	// Add the Nickname if there is one and they are being shown
	if ( Settings.Search.ShowNames && ! pSource->m_sNick.IsEmpty() )
		m_sName = pSource->m_sNick + L" (" + m_sName + L")";

	// Indicate if this is a firewalled client
	if ( pSource->m_bPushOnly )
		m_sName += L" (push)";

	m_sCountryName = pSource->m_sCountryName;

	m_sURL = pSource->m_sURL;

	m_pHeaderName.RemoveAll();
	m_pHeaderValue.RemoveAll();

	if ( ! pSource->IsIdle() && Settings.General.GUIMode != GUI_BASIC )
	{
		const CDownloadTransfer* pTransfer = pSource->GetTransfer();
		for ( int nHeader = 0 ; nHeader < pTransfer->m_pHeaderName.GetSize() ; nHeader++ )
		{
			CString strName  = pTransfer->m_pHeaderName.GetAt( nHeader );
			CString strValue = pTransfer->m_pHeaderValue.GetAt( nHeader );

			if ( strValue.GetLength() > 64 )
				strValue = strValue.Left( 64 ) + L"...";

			m_pHeaderName.Add( strName );
			m_pHeaderValue.Add( strValue );
		}
	}

	AddSize( pDC, m_sName );
	m_sz.cy += TIP_TEXTHEIGHT;

	pDC->SelectObject( &CoolInterface.m_fntNormal );
	AddSize( pDC, m_sCountryName );
	m_sz.cy += TIP_TEXTHEIGHT + TIP_RULE;

	AddSize( pDC, m_sURL, 80 );
	m_sz.cy += TIP_TEXTHEIGHT * 4;

	m_sz.cy += TIP_GAP;
	m_sz.cy += TIP_BARHEIGHT;
	m_sz.cy += TIP_GAP;
	m_sz.cy += TIP_GRAPHHEIGHT;
	m_sz.cy += TIP_GAP;

	int nValueWidth = 0;
	m_nHeaderWidth = 0;

	for ( int nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader++ )
	{
		CString strName		= m_pHeaderName.GetAt( nHeader );
		CString strValue	= m_pHeaderValue.GetAt( nHeader );
		CSize szKey			= pDC->GetTextExtent( strName + ':' );
		CSize szValue		= pDC->GetTextExtent( strValue );

		m_nHeaderWidth		= max( m_nHeaderWidth, int(szKey.cx) );
		nValueWidth			= max( nValueWidth, int(szValue.cx) );

		m_sz.cy += TIP_TEXTHEIGHT;
	}

	if ( m_nHeaderWidth ) m_nHeaderWidth += TIP_GAP;
	m_sz.cx = max( m_sz.cx, m_nHeaderWidth + nValueWidth );
}

void CDownloadTipCtrl::OnPaint(CDC* pDC, CDownloadSource* pSource)
{
	CPoint pt( 0, 0 );
	CSize sz( m_sz.cx, TIP_TEXTHEIGHT );

	DrawText( pDC, &pt, m_sName );
	pt.y += TIP_TEXTHEIGHT;

	const int nFlagIndex = Flags.GetFlagIndex( pSource->m_sCountry );
	if ( nFlagIndex >= 0 )
	{
		Flags.Draw( nFlagIndex, pDC->GetSafeHdc(), pt.x, pt.y, Colors.m_crTipBack );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + FLAG_WIDTH, pt.y + 16 );
	}

	pt.x += 25;
	pt.y += 2;

	pDC->SelectObject( &CoolInterface.m_fntNormal );
	DrawText( pDC, &pt, m_sCountryName );
	pt.y += TIP_TEXTHEIGHT;

	pt.x -= 25;

	DrawRule( pDC, &pt );

	CString strStatus, strSpeed, strText;

	if ( ! pSource->IsIdle() )
	{
		strStatus = pSource->GetState( TRUE );

		if ( DWORD nLimit = pSource->GetLimit() )
		{
			strSpeed.Format( L"%s %s %s",
				Settings.SmartSpeed( pSource->GetMeasuredSpeed() ),
				(LPCTSTR)LoadString( IDS_GENERAL_OF ),
				Settings.SmartSpeed( nLimit ) );
		}
		else
		{
			strSpeed = Settings.SmartSpeed( pSource->GetMeasuredSpeed() );
		}
	}
	else
	{
		LoadString( strStatus, IDS_STATUS_INACTIVE );
		LoadString( strSpeed, IDS_TIP_NA );
	}

	LoadString( strText, IDS_TIP_SPEED );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strSpeed, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_STATUS );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strStatus, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_USERAGENT );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, pSource->m_sServer, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_URL );
	DrawText( pDC, &pt, strText );
	pt.x += 80;
	sz.cx -= 80;
	DrawText( pDC, &pt, m_sURL, &sz );
	pt.x -= 80;
	sz.cx += 80;
	pt.y += TIP_TEXTHEIGHT;
	pt.y += TIP_GAP;

	DrawProgressBar( pDC, &pt, pSource );
	pt.y += TIP_GAP;

	CRect rc( pt.x, pt.y, m_sz.cx, pt.y + TIP_GRAPHHEIGHT );
	pDC->Draw3dRect( &rc, Colors.m_crTipBorder, Colors.m_crTipBorder );
	if ( m_pGraph )
	{
		rc.DeflateRect( 1, 1 );
		m_pGraph->BufferedPaint( pDC, &rc );
		rc.InflateRect( 1, 1 );
		pDC->ExcludeClipRect( &rc );
	}
	pt.y += TIP_GRAPHHEIGHT;
	pt.y += TIP_GAP;

	for ( int nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader++ )
	{
		CString strName  = m_pHeaderName.GetAt( nHeader );
		CString strValue = m_pHeaderValue.GetAt( nHeader );

		DrawText( pDC, &pt, strName + ':' );
		DrawText( pDC, &pt, strValue, m_nHeaderWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl progress case

void CDownloadTipCtrl::DrawProgressBar(CDC* pDC, CPoint* pPoint, CDownload* pDownload)
{
	CRect rcCell( pPoint->x, pPoint->y, m_sz.cx, pPoint->y + TIP_BARHEIGHT );
	pPoint->y += TIP_BARHEIGHT;

	pDC->Draw3dRect( &rcCell, Colors.m_crTipBorder, Colors.m_crTipBorder );
	rcCell.DeflateRect( 1, 1 );

	CFragmentBar::DrawDownload( pDC, &rcCell, pDownload, Colors.m_crTipBack );

	rcCell.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rcCell );
}

void CDownloadTipCtrl::DrawProgressBar(CDC* pDC, CPoint* pPoint, CDownloadSource* pSource)
{
	CRect rcCell( pPoint->x, pPoint->y, m_sz.cx, pPoint->y + TIP_BARHEIGHT );
	pPoint->y += TIP_BARHEIGHT;

	pDC->Draw3dRect( &rcCell, Colors.m_crTipBorder, Colors.m_crTipBorder );
	rcCell.DeflateRect( 1, 1 );

	pSource->Draw( pDC, &rcCell, Colors.m_crTransferRanges );

	rcCell.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rcCell );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl timer

void CDownloadTipCtrl::OnTimer(UINT_PTR nIDEvent)
{
	CCoolTipCtrl::OnTimer( nIDEvent );

	//if ( nIDEvent == 2 )	// Async
	//{
	//	CSingleLock pLock( &Transfers.m_pSection );
	//	if ( ! pLock.Lock( 100 ) )
	//		return;

	//	// Trigger tracker scrape if needed
	//	if ( ! m_pDownload || ! m_pDownload->IsTorrent() )
	//		return;

	//	pLock.Unlock();

	//	if ( m_pDownload->m_pTorrent.ScrapeTracker() )
	//		Invalidate( FALSE );

	//	return;
	//}

	if ( m_pGraph == NULL )
		return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 15 ) )
		return;

	if ( ( m_pDownload && m_pDownload->IsCompleted() ) || ( m_pSource && m_pSource->IsIdle() ) )
		return; 	// No change

	DWORD nSpeed = 0;
	if ( m_pDownload && Downloads.Check( m_pDownload ) )
		nSpeed = m_pDownload->GetMeasuredSpeed();
	else if ( m_pSource && Downloads.Check( m_pSource ) )
		nSpeed = m_pSource->GetMeasuredSpeed();

	pLock.Unlock();

	m_pItem->Add( nSpeed );
	m_pGraph->m_nUpdates++;
	m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );

//	CRect rcUpdateGraph;
//	rcUpdateGraph.top    = m_sz.cy - TIP_BARHEIGHT - TIP_GAP - TIP_GRAPHHEIGHT;
//	rcUpdateGraph.bottom = m_sz.cy + TIP_GAP + 2;	// ?
//	rcUpdateGraph.right  = m_sz.cx + TIP_GAP;	// ?
//	rcUpdateGraph.left   = TIP_GAP;
//	InvalidateRect( &rcUpdateGraph, FALSE );

	Invalidate( FALSE );
}

/* BTTrackerRequest Virtual */
void CDownloadTipCtrl::OnTrackerEvent(bool bSuccess, LPCTSTR /*pszReason*/, LPCTSTR /*pszTip*/, CBTTrackerRequest* pEvent)
{
	ASSUME_LOCK( Transfers.m_pSection );

	//m_nRequest = 0;		// Need no cancel

	if ( ! bSuccess )
		return;

	DWORD nComplete   = pEvent->GetComplete();		// ->m_nSeeders
	DWORD nIncomplete = pEvent->GetIncomplete();	// ->m_nLeechers

	m_pDownload->m_pTorrent.m_nTrackerSeeds = nComplete;
	m_pDownload->m_pTorrent.m_nTrackerPeers = nIncomplete;

	m_sSeedsPeers.Format( L"   ( %u seeds %u peers )", nComplete, nIncomplete );	// ToDo: Translation ?
}
