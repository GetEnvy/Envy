//
// DisplayData.cpp
//
// This file is part of Envy (getenvy.com) © 2014
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
// Abstracted GUI info from downloads/uploads/etc. primarily to avoid locks/redundancy

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "DisplayData.h"
#include "Download.h"
#include "DownloadSource.h"
#include "UploadTransfer.h"
#include "UploadTransferBT.h"
#include "UploadTransferHTTP.h"
#include "UploadFile.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "CtrlDownloads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////////////
// CDownloadDisplayData/CSourceDisplayData construction

CSourceDisplayData::CSourceDisplayData()
	: m_bSelected			( FALSE )
	, m_nProtocol			( 0 )
	, m_nSize				( 0 )
	, m_nState				( 0 )
	, m_tAttempt			( 0 )
	, m_bIdle				( TRUE )
	, m_bPushOnly			( FALSE )	// ed2k
	, m_nDownloaded			( 0 )
	, m_nSpeed				( 0 )
	, m_nAddress			( 0 )
	, m_nServerPort			( 0 )
	, m_nPort				( 0 )
	, m_nPortGet			( 0 )
	, m_nColor				( 0 )
	, m_bReadContent		( FALSE )
	, m_bHasFragments		( FALSE )
	, m_bTransferBackwards	( FALSE )
	, m_nTransferLength		( 0 )
	, m_nTransferOffset		( 0 )
	, m_nTransferPosition	( 0 )
	, m_oAvailable			( NULL )
	, m_oPastFragments		( NULL )
{
};

CSourceDisplayData::CSourceDisplayData(const CDownloadSource* pSource)
	: m_bSelected			( pSource->m_bSelected )
	, m_nProtocol			( pSource->m_nProtocol )
	, m_tAttempt			( pSource->m_tAttempt )
	, m_nSize				( pSource->m_pDownload->m_nSize )
	, m_nState				( pSource->GetState() )
	, m_sState				( pSource->GetState( FALSE ) )
	, m_bIdle				( pSource->IsIdle() )
	, m_bTrying				( pSource->m_pDownload->IsTrying() )
	, m_bPushOnly			( pSource->m_bPushOnly )	// ed2k
	, m_nDownloaded			( pSource->GetDownloaded() )
	, m_nSpeed				( pSource->GetMeasuredSpeed() )
	, m_sServer				( pSource->m_sServer )
	, m_nAddress			( pSource->m_pAddress.S_un.S_addr )
	, m_sAddress			( ( pSource->m_nProtocol == PROTOCOL_DC || pSource->m_nProtocol == PROTOCOL_ED2K && pSource->m_bPushOnly ) ? CString( inet_ntoa( pSource->m_pServerAddress ) ) : CString( inet_ntoa( pSource->m_pAddress ) ) )
	, m_sAddressGet			( pSource->GetAddress() )
	, m_nServerPort			( pSource->m_nServerPort )
	, m_nPort				( pSource->m_nPort )
	, m_nPortGet			( pSource->GetPort() )
	, m_sNick				( pSource->m_sNick )
	, m_sCountry			( pSource->m_sCountry )
	, m_nColor				( pSource->m_nColor )	// pSource->m_pDownload->GetSourceColor()
	, m_bReadContent		( pSource->m_bReadContent )
	, m_bHasFragments		( pSource->IsOnline() && pSource->HasUsefulRanges() || ! pSource->m_oPastFragments.empty() )
	, m_bTransferBackwards	( pSource->GetTransfer()->m_bRecvBackwards )
	, m_nTransferLength		( pSource->GetTransfer()->m_nLength )
	, m_nTransferOffset		( pSource->GetTransfer()->m_nOffset )
	, m_nTransferPosition	( pSource->GetTransfer()->m_nPosition )
	, m_oAvailable			( pSource->m_oAvailable )		// pSource->m_oAvailable.length_sum()
	, m_oPastFragments		( pSource->m_oPastFragments )
{
	if ( m_bTransferBackwards && m_nTransferOffset >= m_nTransferLength )
		m_nTransferOffset -= m_nTransferLength;
};

CSourceDisplayData& CSourceDisplayData::operator=(const CSourceDisplayData& pSource)
{
	m_bSelected				= pSource.m_bSelected;
	m_nProtocol				= pSource.m_nProtocol;
	m_tAttempt				= pSource.m_tAttempt;
	m_nSize					= pSource.m_nSize;
	m_nState				= pSource.m_nState;
	m_sState				= pSource.m_sState;
	m_bIdle					= pSource.m_bIdle;
	m_bTrying				= pSource.m_bTrying;
	m_bPushOnly				= pSource.m_bPushOnly;
	m_nDownloaded			= pSource.m_nDownloaded;
	m_nSpeed				= pSource.m_nSpeed;
	m_sServer				= pSource.m_sServer;
	m_nAddress				= pSource.m_nAddress;
	m_sAddress				= pSource.m_sAddress;
	m_sAddressGet			= pSource.m_sAddressGet;
	m_nServerPort			= pSource.m_nServerPort;
	m_nPort					= pSource.m_nPort;
	m_nPortGet				= pSource.m_nPortGet;
	m_sNick					= pSource.m_sNick;
	m_sCountry				= pSource.m_sCountry;
	m_nColor				= pSource.m_nColor;
	m_bReadContent			= pSource.m_bReadContent;
	m_bHasFragments			= pSource.m_bHasFragments;
	m_bTransferBackwards	= pSource.m_bTransferBackwards;
	m_nTransferLength		= pSource.m_nTransferLength;
	m_nTransferOffset		= pSource.m_nTransferOffset;
	m_nTransferPosition		= pSource.m_nTransferPosition;
	m_oAvailable			= pSource.m_oAvailable;
	m_oPastFragments		= pSource.m_oPastFragments;

	return *this;
}


CDownloadDisplayData::CDownloadDisplayData()
	: m_nSize				( 0 )
	, m_bSelected			( FALSE )
	, m_bExpanded			( FALSE )
	, m_bExpandable			( FALSE )
	, m_bClearing			( FALSE )
	, m_bCompleted			( FALSE )
	, m_bFailedVerify		( FALSE )
	, m_nVolumeComplete		( 0 )
	, m_fProgress			( 0. )
	, m_fRatio				( 0. )
	, m_nRating				( 0 )
	, m_nAverageSpeed		( 0 )
	, m_bMultiFileTorrent	( FALSE )
	, m_bSeeding			( FALSE )
	, m_bSearching			( FALSE )
	, m_bTrying				( FALSE )
	, m_oEmptyFragments		( NULL )
	, m_nSourceCount		( 0 )
{
//	pSourceList.SetSize( 0 );
};

CDownloadDisplayData::CDownloadDisplayData(const CDownload* pDownload)
	: m_sName				( pDownload->m_sName )
	, m_sDisplayName		( pDownload->GetDisplayName() )
	, m_nSize				( pDownload->m_nSize )
	, m_bSelected			( pDownload->m_bSelected )
	, m_bExpanded			( pDownload->m_bExpanded )
	, m_bExpandable			( CDownloadsCtrl::IsExpandable( pDownload ) )
	, m_bClearing			( pDownload->m_bClearing )
	, m_bCompleted			( pDownload->IsCompleted() )
	, m_bFailedVerify		( pDownload->m_bVerify == TRI_FALSE )
	, m_nVolumeComplete		( pDownload->IsSeeding() ? pDownload->m_nTorrentUploaded : pDownload->GetVolumeComplete() )
	, m_fProgress			( pDownload->GetProgress() )
	, m_fRatio				( pDownload->IsSeeding() ? pDownload->GetRatio() : 0. )
	, m_nRating				( pDownload->GetReviewAverage() )
	, m_nAverageSpeed		( pDownload->GetAverageSpeed() )
	, m_sDownloadSources	( pDownload->GetDownloadSources() )
	, m_bMultiFileTorrent	( pDownload->IsMultiFileTorrent() )
	, m_bSeeding			( pDownload->IsSeeding() )
	, m_bSearching			( pDownload->IsSearching() )
	, m_bTrying				( pDownload->IsTrying() )
	, m_sDownloadStatus		( pDownload->GetDownloadStatus() )
	, m_oEmptyFragments		( pDownload->GetEmptyFragmentList() )
	, m_nSourceCount		( pDownload->GetSourceCount() )
{
	m_pSourcesData.SetSize( m_nSourceCount );
};

CDownloadDisplayData& CDownloadDisplayData::operator=(const CDownloadDisplayData& pDownload)
{
	m_sName					= pDownload.m_sName;
	m_sDisplayName			= pDownload.m_sDisplayName;
	m_nSize					= pDownload.m_nSize;
	m_bSelected				= pDownload.m_bSelected;
	m_bExpanded				= pDownload.m_bExpanded;
	m_bExpandable			= pDownload.m_bExpandable;
	m_bClearing				= pDownload.m_bClearing;
	m_bCompleted			= pDownload.m_bCompleted;
	m_bFailedVerify			= pDownload.m_bFailedVerify;
	m_nVolumeComplete		= pDownload.m_nVolumeComplete;
	m_fProgress				= pDownload.m_fProgress;
	m_fRatio				= pDownload.m_fRatio;
	m_nRating				= pDownload.m_nRating;
	m_nAverageSpeed			= pDownload.m_nAverageSpeed;
	m_sDownloadSources		= pDownload.m_sDownloadSources;
	m_bMultiFileTorrent		= pDownload.m_bMultiFileTorrent;
	m_bSeeding				= pDownload.m_bSeeding;
	m_bSearching			= pDownload.m_bSearching;
	m_bTrying				= pDownload.m_bTrying;
	m_sDownloadStatus		= pDownload.m_sDownloadStatus;
	m_oEmptyFragments		= pDownload.m_oEmptyFragments;
	m_nSourceCount			= pDownload.m_nSourceCount;
	m_pSourcesData.Copy( pDownload.m_pSourcesData );
	m_pVerifyRanges.SetSize( pDownload.m_pVerifyRanges.GetSize() );
	m_pVerifyRanges.Copy( pDownload.m_pVerifyRanges );

	return *this;
}


//////////////////////////////////////////////////////////////////////////////
// CUploadDisplayData/CQueueDisplayData construction

CUploadDisplayData::CUploadDisplayData()
	: m_bSelected		( FALSE )
	, m_nSize			( 0 )
	, m_nUploaded		( 0 )
	, m_nSpeed			( 0 )
	, m_nProtocol		( 0 )
	, m_bTorrent		( FALSE )
	, m_bTransferNull	( FALSE )
	, m_bBaseFile		( FALSE )
	, m_bBackwards		( FALSE )
	, m_nUserRating		( 0 )
	, m_nLength			( 0 )
	, m_nOffset			( 0 )
	, m_nPosition		( 0 )
	, m_oFragments		( NULL )
{
};

CUploadDisplayData::CUploadDisplayData(const CUploadFile* pUpload, CUploadTransfer* pTransfer)
	: m_bSelected		( pUpload->m_bSelected )
	, m_sName			( pUpload->m_sName )
	, m_sPath			( pUpload->m_sPath )
	, m_nSize			( pUpload->m_nSize )
	, m_nUploaded		( pTransfer->m_nUploaded )
	, m_nSpeed			( pTransfer->GetMeasuredSpeed() )
	, m_nProtocol		( pTransfer->m_nProtocol )
	, m_bTorrent		( pTransfer->m_nProtocol == PROTOCOL_BT )
	, m_bTransferNull	( pTransfer == NULL || pTransfer->m_nState == upsNull )
	, m_bBaseFile		( pUpload == pTransfer->m_pBaseFile )
	, m_bBackwards		( pTransfer->m_nProtocol == PROTOCOL_HTTP && ((CUploadTransferHTTP*)pTransfer)->IsBackwards() )
//	, m_sState			( )			// (Speed column LoadString below)
	, m_sCountry		( pTransfer->m_sCountry )
	, m_sAddress		( pTransfer->m_sAddress )
	, m_sRemoteNick		( pTransfer->m_sRemoteNick )
	, m_sUserAgent		( pTransfer->m_sUserAgent )
	, m_nUserRating		( pTransfer->m_nUserRating )
	, m_nLength			( pTransfer->m_nLength )
	, m_nOffset			( pTransfer->m_nOffset )
	, m_nPosition		( pTransfer->m_nPosition )
	, m_oFragments		( pUpload->m_oFragments )
{
	if ( m_bTorrent && ! m_bTransferNull )
	{
		CUploadTransferBT* pBT = (CUploadTransferBT*)pTransfer;

		if ( ! pBT->m_bInterested )
			LoadString( m_sState, IDS_STATUS_UNINTERESTED );
		else if ( pBT->m_bChoked )
			LoadString( m_sState, IDS_STATUS_CHOKED );
	}
};

CUploadDisplayData& CUploadDisplayData::operator=(const CUploadDisplayData& pUpload)
{
	m_sName				= pUpload.m_sName;
	m_sPath				= pUpload.m_sPath;
	m_nSize				= pUpload.m_nSize;
	m_nUploaded			= pUpload.m_nUploaded;
	m_nSpeed			= pUpload.m_nSpeed;
	m_nProtocol			= pUpload.m_nProtocol;
	m_bSelected			= pUpload.m_bSelected;
	m_bTorrent			= pUpload.m_bTorrent;
	m_bTransferNull		= pUpload.m_bTransferNull;
	m_bBaseFile			= pUpload.m_bBaseFile;
	m_bBackwards		= pUpload.m_bBackwards;
	m_sState			= pUpload.m_sState;
	m_sCountry			= pUpload.m_sCountry;
	m_sAddress			= pUpload.m_sAddress;
	m_sRemoteNick		= pUpload.m_sRemoteNick;
	m_sUserAgent		= pUpload.m_sUserAgent;
	m_nUserRating		= pUpload.m_nUserRating;
	m_nLength			= pUpload.m_nLength;
	m_nOffset			= pUpload.m_nOffset;
	m_nPosition			= pUpload.m_nPosition;
	m_oFragments		= pUpload.m_oFragments;

	return *this;
}


CQueueDisplayData::CQueueDisplayData()
	: m_bSelected		( FALSE )
	, m_bExpanded		( FALSE )
	, m_bHistoryQueue	( FALSE )
	, m_bTorrentQueue	( FALSE )
	, m_bHTTPQueue		( FALSE )
	, m_bED2KQueue		( FALSE )
	, m_nMinTransfers	( 0 )
	, m_nMaxTransfers	( 0 )
	, m_nSpeed			( 0 )
	, m_nCount			( 0 )
{
};

CQueueDisplayData::CQueueDisplayData(const CUploadQueue* pQueue)
	: m_bSelected		( pQueue->m_bSelected )
	, m_bExpanded		( pQueue->m_bExpanded )
	, m_bHistoryQueue	( pQueue == UploadQueues.m_pHistoryQueue )
	, m_bTorrentQueue	( pQueue == UploadQueues.m_pTorrentQueue )
	, m_bHTTPQueue		( pQueue->m_nProtocols == ( 1 << PROTOCOL_HTTP ) )
	, m_bED2KQueue		( pQueue->m_nProtocols == ( 1 << PROTOCOL_ED2K ) )
	, m_nMinTransfers	( m_bTorrentQueue ? pQueue->m_nMinTransfers : pQueue->GetTransferCount() )
	, m_nMaxTransfers	( m_bTorrentQueue ? pQueue->m_nMaxTransfers : pQueue->GetQueuedCount() )
	, m_nCount			( m_bExpanded ? m_nMinTransfers : 0 )
	, m_nSpeed			( pQueue->GetMeasuredSpeed() )
	, m_sName			( pQueue->m_sName )
{
};

CQueueDisplayData& CQueueDisplayData::operator=(const CQueueDisplayData& pQueue)
{
	m_bSelected			= pQueue.m_bSelected;
	m_bExpanded			= pQueue.m_bExpanded;
	m_sName				= pQueue.m_sName;
	m_bHistoryQueue		= pQueue.m_bHistoryQueue;
	m_bTorrentQueue		= pQueue.m_bTorrentQueue;
	m_bHTTPQueue		= pQueue.m_bHTTPQueue;
	m_bED2KQueue		= pQueue.m_bED2KQueue;
	m_nMinTransfers		= pQueue.m_nMinTransfers;
	m_nMaxTransfers		= pQueue.m_nMaxTransfers;
	m_nSpeed			= pQueue.m_nSpeed;
	m_nCount			= pQueue.m_nCount;
	m_pUploadsData.Copy( pQueue.m_pUploadsData );

	return *this;
}
