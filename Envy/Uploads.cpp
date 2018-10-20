//
// Uploads.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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
#include "Uploads.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransfer.h"
#include "UploadTransferHTTP.h"
#include "UploadTransferED2K.h"
#include "UploadTransferBT.h"

#include "Buffer.h"
#include "Transfers.h"
#include "Neighbours.h"
#include "Statistics.h"
#include "Remote.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CUploads Uploads;


//////////////////////////////////////////////////////////////////////
// CUploads construction

CUploads::CUploads()
	: m_nCount		( 0 )
	, m_nBandwidth	( 0 )
	, m_nBestSpeed	( 0 )
	, m_nTorrentSpeed ( 0 )
	, m_bStable		( FALSE )
{
}

CUploads::~CUploads()
{
	Clear( FALSE );
}

//////////////////////////////////////////////////////////////////////
// CUploads clear

void CUploads::Clear(BOOL bMessage)
{
	CQuickLock oTransfersLock( Transfers.m_pSection );

	for ( POSITION pos = GetIterator(); pos; )
	{
		GetNext( pos )->Remove( bMessage );
	}

	ASSERT( GetCount( NULL, -1 ) == 0 );
}

//////////////////////////////////////////////////////////////////////
// CUploads counting

DWORD CUploads::GetCount(CUploadTransfer* pExcept, int nState) const
{
	if ( pExcept == NULL && nState == -1 )
		return (DWORD)m_pList.GetCount();

	DWORD nCount = 0;

	for ( POSITION pos = GetIterator(); pos; )
	{
		CUploadTransfer* pUpload = GetNext( pos );

		if ( pUpload != pExcept )
		{
			switch ( nState )
			{
			case -1:
				nCount++;
				break;
			case -2:
				if ( pUpload->m_nState > upsNull ) nCount++;
				break;
			default:
				if ( pUpload->m_nState == nState ) nCount++;
				break;
			}
		}
	}

	return nCount;
}

DWORD CUploads::GetTorrentCount(int nState) const
{
	DWORD nCount = 0;

	for ( POSITION pos = GetIterator(); pos; )
	{
		CUploadTransfer* pUpload = GetNext( pos );

		if ( pUpload->m_nProtocol == PROTOCOL_BT )
		{
			switch ( nState )
			{
			case -1:
				nCount++;
				break;
			case -2:
				if ( pUpload->m_nState > upsNull ) nCount++;
				break;
			case -3:
				if ( pUpload->m_nState >= upsUploading ) nCount++;
				break;
			default:
				if ( pUpload->m_nState == nState ) nCount++;
				break;
			}
		}
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CUploads status

void CUploads::SetStable(DWORD nSpeed)
{
	m_bStable = TRUE;
	m_nBestSpeed = max( m_nBestSpeed, nSpeed );
}

DWORD CUploads::GetBandwidth() const
{
	DWORD nTotal = 0;

	for ( POSITION pos = GetIterator(); pos; )
	{
		nTotal += GetNext( pos )->GetMeasuredSpeed();
	}

	return nTotal;
}

DWORD CUploads::GetBandwidthLimit() const
{
	DWORD nTotal = Settings.Connection.OutSpeed * 128;		// Kilobits/s to Bytes/s
	DWORD nLimit = Settings.Bandwidth.Uploads;
	if ( nLimit == 0 || nLimit > nTotal )
		nLimit = nTotal;

	// Limit if hub mode (~50%)
	if ( Settings.Uploads.HubUnshare && ( Neighbours.IsG2Hub() || Neighbours.IsG1Ultrapeer() ) )
		nLimit = nLimit * Settings.Bandwidth.HubUploads / 100;

	// Limit if torrents are active (~90%)	Note same cap value given to both torrents and others!  Deceptive simple headroom.
	if ( UploadQueues.m_pTorrentQueue->m_nMinTransfers )	// ( Uploads.m_nTorrentSpeed > 0 )
		nLimit = nLimit * Settings.BitTorrent.BandwidthPercentage / 100;

	return nLimit;
}

//////////////////////////////////////////////////////////////////////
// CUploads per-host limiting

BOOL CUploads::AllowMoreTo(const IN_ADDR* pAddress) const
{
	DWORD nCount = 0;

	for ( POSITION pos = GetIterator(); pos; )
	{
		const CUploadTransfer* pUpload = GetNext( pos );

		if ( pUpload->m_nState == upsUploading ||
			 pUpload->m_nState == upsQueued )
		{
			if ( pUpload->m_pHost.sin_addr.s_addr == pAddress->s_addr )
				nCount++;
		}
	}

	return ( nCount <= Settings.Uploads.MaxPerHost );
}

BOOL CUploads::CanUploadFileTo(const IN_ADDR* pAddress, const CEnvyFile* pFile) const
{
	DWORD nCount = 0;

	for ( POSITION pos = GetIterator(); pos; )
	{
		const CUploadTransfer* pUpload = GetNext( pos );

		if ( pUpload->m_nState == upsUploading ||
			 pUpload->m_nState == upsQueued )
		{
			if ( pUpload->m_pHost.sin_addr.s_addr == pAddress->s_addr )
			{
				nCount++;

				// If we're already uploading this file to this client
				if ( *pUpload == *pFile )
					return FALSE;
			}
		}
	}

	return ( nCount < Settings.Uploads.MaxPerHost );
}

BOOL CUploads::EnforcePerHostLimit(CUploadTransfer* pHit, BOOL bRequest)
{
	DWORD nCount = 0;

	for ( POSITION pos = GetIterator(); pos; )
	{
		const CUploadTransfer* pUpload = GetNext( pos );

		if ( pUpload->m_nState == upsUploading ||
			 pUpload->m_nState == upsQueued ||
			 pUpload->m_nState == upsPreQueue )
		{
			if ( pUpload->m_pHost.sin_addr.s_addr == pHit->m_pHost.sin_addr.s_addr )
				nCount++;
		}
	}

	if ( nCount <= Settings.Uploads.MaxPerHost )
		return FALSE;

	while ( nCount > Settings.Uploads.MaxPerHost )
	{
		CUploadTransfer* pNewest = NULL;

		for ( POSITION pos = GetIterator(); pos; )
		{
			CUploadTransfer* pUpload = GetNext( pos );

			if ( pUpload->m_nState == upsUploading ||
				 pUpload->m_nState == upsQueued ||
				 pUpload->m_nState == upsPreQueue )
			{
				if ( pUpload != pHit && pUpload->m_pHost.sin_addr.s_addr == pHit->m_pHost.sin_addr.s_addr )
				{
					if ( bRequest && pUpload->m_pBaseFile == pHit->m_pBaseFile )
					{
						pNewest = pUpload;
						break;
					}

					if ( pNewest == NULL || pUpload->m_tConnected > pNewest->m_tConnected )
						pNewest = pUpload;
				}
			}
		}

		if ( pNewest == NULL )
			return TRUE;	// Drop current connection

		if ( bRequest )
		{
			if ( pNewest->m_pBaseFile == pHit->m_pBaseFile && ! pNewest->m_bLive )
			{
				UploadQueues.StealPosition( pHit, pNewest );

				theApp.Message( MSG_ERROR, IDS_UPLOAD_DROPPED_OLDER, (LPCTSTR)pNewest->m_sAddress );

				pNewest->Close( FALSE );
				nCount--;
			}
			else
			{
				return TRUE;	// Drop current connection
			}
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_DROPPED_NEWER, (LPCTSTR)pNewest->m_sAddress );

			pNewest->Close( FALSE );
			nCount--;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploads run

void CUploads::OnRun()
{
	CSingleLock oTransfersLock( &Transfers.m_pSection );
	if ( ! oTransfersLock.Lock( 250 ) )
		return;

	CSingleLock oUploadQueuesLock( &UploadQueues.m_pSection );
	if ( ! oUploadQueuesLock.Lock( 250 ) )
		return;

	m_nCount = 0;
	m_nBandwidth = 0;

	// Set measured queue speeds to 0
	for ( POSITION pos = UploadQueues.GetIterator(); pos; )
	{
		UploadQueues.GetNext( pos )->m_nMeasured = 0;
	}
	UploadQueues.m_pTorrentQueue->m_nMeasured = 0;
	UploadQueues.m_pTorrentQueue->m_nMinTransfers = 0;
	UploadQueues.m_pTorrentQueue->m_nMaxTransfers = 0;

	for ( POSITION pos = GetIterator(); pos; )
	{
		CUploadTransfer* pTransfer = GetNext( pos );

		if ( pTransfer->m_nState == upsNull )
			continue;

		// Only call this when we are going to use the returned value
		DWORD nMeasured = pTransfer->GetMeasuredSpeed();

		if ( pTransfer->m_nProtocol == PROTOCOL_BT )
		{
			// This is a torrent transfer
			CUploadTransferBT* pBT = (CUploadTransferBT*)pTransfer;
			if ( ! pBT->m_bInterested || pBT->m_bChoked )
			{
				// Choked- Increment appropriate torrent counter
				UploadQueues.m_pTorrentQueue->m_nMaxTransfers ++;
			}
			else
			{
				// Active torrent. (Uploading or requesting)

				// Increment normal counters
				m_nCount ++;
				m_nBandwidth += nMeasured;

				// Increment torrent counters
				UploadQueues.m_pTorrentQueue->m_nMinTransfers ++;
				UploadQueues.m_pTorrentQueue->m_nMeasured += nMeasured;

				//theApp.Message( MSG_NOTICE, L"Transfer %s, Port: %i", pTransfer->m_sAddress, pTransfer->m_pHost.sin_port );
			}
		}
		else if ( pTransfer->m_nState == upsUploading )
		{
			// Regular transfer that's uploading

			m_nCount ++;
			m_nBandwidth += nMeasured;

			if ( UploadQueues.Check( pTransfer->m_pQueue ) )
				pTransfer->m_pQueue->m_nMeasured += nMeasured;
		}
	}

	// When there are any active torrents, assign bandwidth to BT, otherwise set zero
	if ( UploadQueues.m_pTorrentQueue->m_nMinTransfers )
		m_nTorrentSpeed = GetBandwidthLimit() / UploadQueues.m_pTorrentQueue->m_nMinTransfers;
	else
		m_nTorrentSpeed = 0;
}

//////////////////////////////////////////////////////////////////////
// CUploads get connection

BOOL CUploads::OnAccept(CConnection* pConnection)
{
	CSingleLock oTransfersLock( &Transfers.m_pSection );
	if ( oTransfersLock.Lock( 250 ) )
	{
		if ( pConnection->StartsWith( _P("GET /remote/") ) ||
			 pConnection->StartsWith( _P("GET /remote HTTP") ) )		// User entered remote page into browser, but forgot trailing '/'
		{
			if ( Settings.Remote.Enable )
			{
				if ( new CRemote( pConnection ) )
					return FALSE;
			}

			theApp.Message( MSG_ERROR, L"Rejecting incoming connection from %s, remote interface disabled.", (LPCTSTR)pConnection->m_sAddress );

			pConnection->SendHTML( IDR_HTML_FILENOTFOUND );
			pConnection->DelayClose( IDS_CONNECTION_CLOSED );
			return TRUE;
		}

		if ( CUploadTransfer* pUpload = new CUploadTransferHTTP() )
		{
			for ( POSITION pos = GetIterator(); pos; )
			{
				CUploadTransfer* pTest = GetNext( pos );

				if ( pTest->m_pHost.sin_addr.S_un.S_addr == pConnection->m_pHost.sin_addr.S_un.S_addr )
					pTest->m_bLive = FALSE;
			}

			pUpload->AttachTo( pConnection );
			return FALSE;
		}
	}

	theApp.Message( MSG_INFO, L"Rejecting incoming connection from %s, network core overloaded.", (LPCTSTR)pConnection->m_sAddress );

	pConnection->SendHTML( IDR_HTML_BUSY );
	pConnection->DelayClose( IDS_CONNECTION_CLOSED );

	return TRUE;
}

void CUploads::OnRename(LPCTSTR pszSource, LPCTSTR pszTarget)
{
	CQuickLock oTransfersLock( Transfers.m_pSection );

	for ( POSITION pos = GetIterator(); pos; )
	{
		GetNext( pos )->OnRename( pszSource, pszTarget );
	}
}

//////////////////////////////////////////////////////////////////////
// CUploads add and remove

void CUploads::Add(CUploadTransfer* pUpload)
{
	POSITION pos = m_pList.Find( pUpload );
	ASSERT( pos == NULL );
	m_pList.AddHead( pUpload );
	UNUSED_ALWAYS( pos );
}

void CUploads::Remove(CUploadTransfer* pUpload)
{
	POSITION pos = m_pList.Find( pUpload );
	ASSERT( pos != NULL );
	m_pList.RemoveAt( pos );
//	UNUSED_ALWAYS( pos );
}
