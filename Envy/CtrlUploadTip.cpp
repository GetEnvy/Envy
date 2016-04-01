//
// CtrlUploadTip.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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
#include "CtrlUploadTip.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Transfers.h"
#include "UploadFile.h"
#include "UploadFiles.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransfer.h"
#include "GraphLine.h"
#include "GraphItem.h"
#include "FragmentedFile.h"
#include "FragmentBar.h"
#include "Flags.h"

// Torrent Scrape:
#include "Download.h"
#include "Downloads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CUploadTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CUploadTipCtrl, CCoolTipCtrl)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUploadTipCtrl construction

CUploadTipCtrl::CUploadTipCtrl()
	: m_pUploadFile	( NULL )
	, m_pGraph		( NULL )
	, m_pItem		( NULL )
	, m_nHeaderWidth( 0 )
{
}

CUploadTipCtrl::~CUploadTipCtrl()
{
	delete m_pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CUploadTipCtrl events

BOOL CUploadTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;

	CalcSizeHelper();

	return m_sz.cx > 0;
}

void CUploadTipCtrl::OnShow()
{
	if ( m_pGraph ) delete m_pGraph;

	m_pGraph = CreateLineGraph();
	m_pItem  = new CGraphItem( 0, 1.0f, RGB( 0xFF, 0, 0 ) );	// ToDo: Skin tip graphlines?  Colors.m_cr

	m_pGraph->AddItem( m_pItem );

	CSingleLock pLock( &Transfers.m_pSection );
	if ( pLock.Lock( 500 ) )
	{
		CDownload* pDownload = Downloads.FindByBTH( m_pUploadFile->GetActive()->m_oBTH );	// Transfers lock required
		TrackerRequests.Request( pDownload, BTE_TRACKER_SCRAPE, 0, this );
	//	pLock.Unlock();
	}
}

void CUploadTipCtrl::OnHide()
{
	delete m_pGraph;
	m_pGraph = NULL;
	m_pItem  = NULL;

	if ( ! m_sSeedsPeers.IsEmpty() ) m_sSeedsPeers.Empty();
}

void CUploadTipCtrl::OnCalcSize(CDC* pDC)
{
	if ( ! m_pUploadFile || ! UploadFiles.Check( m_pUploadFile ) ) return;
	CUploadTransfer* pUpload = m_pUploadFile->GetActive();

	m_sAddress = inet_ntoa( pUpload->m_pHost.sin_addr );
	if ( ! pUpload->m_sRemoteNick.IsEmpty() )
		m_sAddress = pUpload->m_sRemoteNick + L" (" + m_sAddress + L")";

	m_pHeaderName.RemoveAll();
	m_pHeaderValue.RemoveAll();

	if ( Settings.General.GUIMode != GUI_BASIC )
	{
		for ( int nHeader = 0 ; nHeader < pUpload->m_pHeaderName.GetSize() ; nHeader++ )
		{
			CString strName  = pUpload->m_pHeaderName.GetAt( nHeader );
			CString strValue = pUpload->m_pHeaderValue.GetAt( nHeader );

			if ( strValue.GetLength() > 64 ) strValue = strValue.Left( 64 ) + L"...";

			m_pHeaderName.Add( strName );
			m_pHeaderValue.Add( strValue );
		}
	}

	AddSize( pDC, m_pUploadFile->m_sName );
	AddSize( pDC, m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	AddSize( pDC, pUpload->m_sCountryName );

	m_sz.cy += TIP_TEXTHEIGHT * 3 + 4;
	m_sz.cy += TIP_RULE;
	m_sz.cy += TIP_TEXTHEIGHT * 4;
	m_sz.cy += TIP_GAP;
	m_sz.cy += TIP_BARHEIGHT;
	m_sz.cy += TIP_GAP;
	m_sz.cy += TIP_GRAPHHEIGHT;
	//m_sz.cy += TIP_GAP;

	int nValueWidth = 0;
	m_nHeaderWidth = 0;

	for ( int nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader++ )
	{
		CString strName		= m_pHeaderName.GetAt( nHeader );
		CString strValue	= m_pHeaderValue.GetAt( nHeader );
		CSize szKey			= pDC->GetTextExtent( strName + ':' );
		CSize szValue		= pDC->GetTextExtent( strValue );

		m_nHeaderWidth		= max( m_nHeaderWidth, szKey.cx );
		nValueWidth			= max( nValueWidth, szValue.cx );

		m_sz.cy += TIP_TEXTHEIGHT;
	}

	if ( m_nHeaderWidth ) m_nHeaderWidth += TIP_GAP;
	m_sz.cx = min( max( max( m_sz.cx, m_nHeaderWidth + nValueWidth ), 400 ),
		GetSystemMetrics( SM_CXSCREEN ) / 2 );
}

void CUploadTipCtrl::OnPaint(CDC* pDC)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	if ( ! m_pUploadFile || ! UploadFiles.Check( m_pUploadFile ) )
	{
		pLock.Unlock();
		Hide();
		return;
	}

	CUploadTransfer* pUpload = m_pUploadFile->GetActive();

	CPoint pt( 0, 0 );

	DrawText( pDC, &pt, m_pUploadFile->m_sName );
	pt.y += TIP_TEXTHEIGHT;
	DrawText( pDC, &pt, m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	pt.y += TIP_TEXTHEIGHT;

	const int nFlagIndex = Flags.GetFlagIndex( pUpload->m_sCountry );
	if ( nFlagIndex >= 0 )
	{
		Flags.Draw( nFlagIndex, pDC->GetSafeHdc(), pt.x, pt.y,
			( Images.m_bmToolTip.m_hObject ) ? CLR_NONE : Colors.m_crTipBack, CLR_NONE, ILD_NORMAL );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + FLAG_WIDTH, pt.y + 16 );
	}

	pt.x += FLAG_WIDTH + 9;
	pt.y += 2;
	DrawText( pDC, &pt, pUpload->m_sCountryName );
	pt.x -= FLAG_WIDTH + 9;
	pt.y += TIP_TEXTHEIGHT + 2;

	DrawRule( pDC, &pt );

	CString strStatus, strSpeed, strTransfer, strText, strOf;
	LoadString( strOf, IDS_GENERAL_OF );

	if ( pUpload->GetMaxSpeed() > 10 )
	{
		strSpeed.Format( L"%s %s %s  (%s)",
			(LPCTSTR)Settings.SmartSpeed( pUpload->GetMeasuredSpeed() ),
			(LPCTSTR)strOf,
			(LPCTSTR)Settings.SmartSpeed( pUpload->m_nBandwidth ),
			(LPCTSTR)Settings.SmartSpeed( pUpload->GetMaxSpeed() ) );
	}
	else
	{
		strSpeed.Format( L"%s %s %s",
			(LPCTSTR)Settings.SmartSpeed( pUpload->GetMeasuredSpeed() ),
			(LPCTSTR)strOf,
			(LPCTSTR)Settings.SmartSpeed( pUpload->m_nBandwidth ) );
	}

	if ( pUpload->m_nSize > 1 )
	{
		strTransfer.Format( L"%s %s %s  (%.2f%%)",
			(LPCTSTR)Settings.SmartVolume( pUpload->m_nUploaded ),
			(LPCTSTR)strOf,
			(LPCTSTR)Settings.SmartVolume( pUpload->m_nSize ),
			float( pUpload->m_nUploaded * 10240 / pUpload->m_nSize ) / 100.00f );
	}
	else
	{
		strTransfer = Settings.SmartVolume( pUpload->m_nSize );
	}

	int nQueue = UploadQueues.GetPosition( pUpload, FALSE );
	if ( m_pUploadFile != pUpload->m_pBaseFile || pUpload->m_nState == upsNull )
	{
		LoadString( strStatus, IDS_STATUS_INACTIVE );
	}
	else if ( nQueue == 0 )
	{
		if ( pUpload->m_nState == upsQueued )
		{
			LoadString( strText, IDS_TIP_NEXT );
			strStatus.Format( L"%s: %s",
				(LPCTSTR)pUpload->m_pQueue->m_sName, (LPCTSTR)strText );
		}
		else
		{
			LoadString( strText, pUpload->m_bPriority ? IDS_TIP_PRIORITY : IDS_STATUS_ACTIVE );
			strStatus.Format( L"%s: %s",
				(LPCTSTR)pUpload->m_pQueue->m_sName, (LPCTSTR)strText );
		}
	}
	else if ( nQueue > 0 )
	{
		strStatus.Format( L"%s: %i %s %u",
			(LPCTSTR)pUpload->m_pQueue->m_sName,
			nQueue, (LPCTSTR)strOf, pUpload->m_pQueue->GetQueuedCount() );
	}
	else // ?
	{
		LoadString( strStatus, pUpload->m_bPriority ? IDS_TIP_PRIORITY : IDS_STATUS_ACTIVE );
	}

	// Add Torrent Seeds/Peers from last Tracker Scrape
	if ( pUpload->m_nProtocol == PROTOCOL_BT )
		strStatus += m_sSeedsPeers;

	LoadString( strText, IDS_TIP_STATUS );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strStatus, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_USERAGENT );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, pUpload->m_sUserAgent, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_SPEED );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strSpeed, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_TRANSFER );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strTransfer, 80 );
	pt.y += TIP_TEXTHEIGHT;

	// Progress Bar
	pt.y += TIP_GAP;
	DrawProgressBar( pDC, &pt, m_pUploadFile );
	pt.y += TIP_GAP;

	pLock.Unlock();

	// Graph Bar
	CRect rc( pt.x, pt.y, m_sz.cx, pt.y + TIP_GRAPHHEIGHT );
	pDC->Draw3dRect( &rc, Colors.m_crTipBorder, Colors.m_crTipBorder );
	rc.DeflateRect( 1, 1 );
	if ( m_pGraph ) m_pGraph->BufferedPaint( pDC, &rc );
	rc.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rc );
	pt.y += TIP_GRAPHHEIGHT;
	//pt.y += TIP_GAP;

	for ( int nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader++ )
	{
		CString strName  = m_pHeaderName.GetAt( nHeader );
		CString strValue = m_pHeaderValue.GetAt( nHeader );

		DrawText( pDC, &pt, strName + ':' );
		DrawText( pDC, &pt, strValue, m_nHeaderWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
}

void CUploadTipCtrl::DrawProgressBar(CDC* pDC, CPoint* pPoint, CUploadFile* pFile)
{
	CRect rcCell( pPoint->x, pPoint->y, m_sz.cx, pPoint->y + TIP_TEXTHEIGHT );
	pPoint->y += TIP_TEXTHEIGHT;

	pDC->Draw3dRect( &rcCell, Colors.m_crTipBorder, Colors.m_crTipBorder );
	rcCell.DeflateRect( 1, 1 );

	CFragmentBar::DrawUpload( pDC, &rcCell, pFile, Colors.m_crTipBack );

	rcCell.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rcCell );
}

/////////////////////////////////////////////////////////////////////////////
// CUploadTipCtrl message handlers

void CUploadTipCtrl::OnTimer(UINT_PTR nIDEvent)
{
	CCoolTipCtrl::OnTimer( nIDEvent );

	if ( m_pGraph == NULL ) return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 10 ) ) return;

	if ( ! m_pUploadFile || ! UploadFiles.Check( m_pUploadFile ) )
	{
		pLock.Unlock();
		Hide();
		return;
	}

	if ( CUploadTransfer* pUpload = m_pUploadFile->GetActive() )
	{
		DWORD nSpeed = pUpload->GetMeasuredSpeed();
		pLock.Unlock();

		if ( nIDEvent == 1 )
		{
			m_pItem->Add( nSpeed );
			m_pGraph->m_nUpdates++;
			m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );

		//	if ( pUpload->m_nState != upsNull && m_pGraph->m_nMaximum >= 8 )
		//	{
		//		CRect rcUpdateGraph;
		//		SystemParametersInfo( SPI_GETWORKAREA, 0, rcUpdateGraph, 0 );
		//		rcUpdateGraph.top += TIP_TEXTHEIGHT * 8 + TIP_GAP + TIP_GAP;
		//		InvalidateRect( &rcUpdateGraph, FALSE );
		//	}
		}

		// Update torrent Seeds/Peers asynchronously, Scrape if needed
		if ( nIDEvent == 2 && pUpload->m_nProtocol == PROTOCOL_BT && m_sSeedsPeers.IsEmpty() )
		{
			if ( pLock.Lock( 100 ) )
			{
				CDownload* pDownload = Downloads.FindByBTH( m_pUploadFile->GetActive()->m_oBTH );		// Transfers lock required
				pLock.Unlock();
				if ( pDownload && pDownload->m_pTorrent.m_nTrackerSeeds || pDownload->m_pTorrent.m_nTrackerPeers )
					m_sSeedsPeers.Format( L"   ( %u seeds %u peers )",			// ToDo: Translation ?
						pDownload->m_pTorrent.m_nTrackerSeeds, pDownload->m_pTorrent.m_nTrackerPeers );
			}
		}

		Invalidate( FALSE );
	}
}

/* BTTrackerRequest Virtual */
void CUploadTipCtrl::OnTrackerEvent(bool bSuccess, LPCTSTR /*pszReason*/, LPCTSTR /*pszTip*/, CBTTrackerRequest* pEvent)
{
	ASSUME_LOCK( Transfers.m_pSection );

	//m_nRequest = 0;		// Need no cancel

	if ( ! bSuccess )
		return;

	DWORD nComplete   = pEvent->GetComplete();		// ->m_nSeeders
	DWORD nIncomplete = pEvent->GetIncomplete();	// ->m_nLeechers

	m_sSeedsPeers.Format( L"   ( %u seeds %u peers )", nComplete, nIncomplete );			// ToDo: Translation ?

	CSingleLock pLock( &Transfers.m_pSection );
	if ( pLock.Lock( 150 ) )
	{
		CDownload* pDownload = Downloads.FindByBTH( m_pUploadFile->GetActive()->m_oBTH );	// Transfers lock required
		pDownload->m_pTorrent.m_nTrackerSeeds = nComplete;
		pDownload->m_pTorrent.m_nTrackerPeers = nIncomplete;
	//	pLock.Unlock();
	}
}
