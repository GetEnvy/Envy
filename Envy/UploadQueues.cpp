//
// UploadQueues.cpp
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
#include "UploadQueues.h"
#include "UploadQueue.h"
#include "UploadTransfer.h"
#include "SharedFile.h"
#include "Download.h"
#include "Network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CUploadQueues UploadQueues;


//////////////////////////////////////////////////////////////////////
// CUploadQueues construction

CUploadQueues::CUploadQueues()
	: m_pTorrentQueue	( new CUploadQueue() )
	, m_pHistoryQueue	( new CUploadQueue() )
	, m_bDonkeyLimited	( FALSE )
{
	m_pHistoryQueue->m_bExpanded = FALSE;
}

CUploadQueues::~CUploadQueues()
{
	Clear();
	delete m_pHistoryQueue;
	delete m_pTorrentQueue;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues enqueue and dequeue

BOOL CUploadQueues::Enqueue(CUploadTransfer* pUpload, BOOL bForce)
{
	CQuickLock oLock( m_pSection );

	ASSERT( pUpload != NULL );

	Dequeue( pUpload );

	ASSERT( pUpload->m_pQueue == NULL );

	if ( pUpload->m_nSize == 0 ) return FALSE;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadQueue* pQueue = GetNext( pos );

		if ( pQueue->CanAccept(	pUpload->m_nProtocol, pUpload->m_sName, pUpload->m_nSize,
								( pUpload->m_bFilePartial ? CUploadQueue::ulqPartial : CUploadQueue::ulqLibrary ),
								pUpload->m_sFileTags ) )
		{
			if ( pQueue->Enqueue( pUpload, bForce, bForce ) ) return TRUE;
		}
	}

	return FALSE;
}

BOOL CUploadQueues::Dequeue(CUploadTransfer* pUpload)
{
	CQuickLock oLock( m_pSection );

	ASSERT( pUpload != NULL );

	if ( Check( pUpload->m_pQueue ) )
		return pUpload->m_pQueue->Dequeue( pUpload );

	pUpload->m_pQueue = NULL;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues position lookup and optional start

int CUploadQueues::GetPosition(CUploadTransfer* pUpload, BOOL bStart)
{
	ASSERT( pUpload != NULL );

	CSingleLock oLock1( &Network.m_pSection );
	if ( oLock1.Lock( 500 ) )
	{
		CSingleLock oLock2( &m_pSection );
		if ( oLock2.Lock( 250 ) )
		{
			if ( Check( pUpload->m_pQueue ) )
				return pUpload->m_pQueue->GetPosition( pUpload, bStart );

			pUpload->m_pQueue = NULL;
		}
	}

	theApp.Message( MSG_ERROR, L"Rejecting Upload connection to %s, network core overloaded.", (LPCTSTR)pUpload->m_sAddress );

	// Upload has no valid queue, or network core overloaded, or shutdown
	return -1;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues position stealing

BOOL CUploadQueues::StealPosition(CUploadTransfer* pTarget, CUploadTransfer* pSource)
{
	CQuickLock oLock( m_pSection );

	ASSERT( pTarget != NULL );
	ASSERT( pSource != NULL );

	Dequeue( pTarget );

	if ( ! Check( pSource->m_pQueue ) ) return FALSE;

	return pSource->m_pQueue->StealPosition( pTarget, pSource );
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues create and delete queues

CUploadQueue* CUploadQueues::Create(LPCTSTR pszName, BOOL bTop)
{
	CQuickLock oLock( m_pSection );

	CUploadQueue* pQueue = new CUploadQueue();
	if ( pszName != NULL ) pQueue->m_sName = pszName;
	pQueue->m_bEnable = ! bTop;

	if ( bTop )
		m_pList.AddHead( pQueue );
	else
		m_pList.AddTail( pQueue );

	return pQueue;
}

void CUploadQueues::Delete(CUploadQueue* pQueue)
{
	CQuickLock oLock( m_pSection );

	if ( ! Check( pQueue ) )
		return;

	if ( POSITION pos = m_pList.Find( pQueue ) )
		m_pList.RemoveAt( pos );

	delete pQueue;
}

BOOL CUploadQueues::Reorder(CUploadQueue* pQueue, CUploadQueue* pBefore)
{
	CQuickLock oLock( m_pSection );

	POSITION pos1 = m_pList.Find( pQueue );
	if ( pos1 == NULL ) return FALSE;

	if ( pBefore != NULL )
	{
		POSITION pos2 = m_pList.Find( pBefore );
		if ( pos2 == NULL || pos1 == pos2 ) return FALSE;
		m_pList.RemoveAt( pos1 );
		m_pList.InsertBefore( pos2, pQueue );
	}
	else
	{
		m_pList.RemoveAt( pos1 );
		m_pList.AddTail( pQueue );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues queue selection

CUploadQueue* CUploadQueues::SelectQueue(PROTOCOLID nProtocol, CLibraryFile const * const pFile)
{
	return SelectQueue( nProtocol, pFile->m_sName, pFile->GetSize(), CUploadQueue::ulqLibrary, pFile->m_sShareTags );
}

CUploadQueue* CUploadQueues::SelectQueue(PROTOCOLID nProtocol, CDownload const * const pFile)
{
	return SelectQueue( nProtocol, pFile->m_sName, pFile->m_nSize, CUploadQueue::ulqPartial );
}

CUploadQueue* CUploadQueues::SelectQueue(PROTOCOLID nProtocol, LPCTSTR pszName, QWORD nSize, DWORD nFileState, LPCTSTR pszShareTags)
{
	CQuickLock oLock( m_pSection );

	int nIndex = 0;
	for ( POSITION pos = GetIterator() ; pos ; nIndex++ )
	{
		CUploadQueue* pQueue = GetNext( pos );
		pQueue->m_nIndex = nIndex;
		if ( pQueue->CanAccept( nProtocol, pszName, nSize, nFileState, pszShareTags ) )
			return pQueue;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues counting

DWORD CUploadQueues::GetTotalBandwidthPoints( BOOL ActiveOnly )
{
	CQuickLock oLock( m_pSection );
	DWORD nCount = 0;
	CUploadQueue *pQptr;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		pQptr = GetNext( pos );
		if ( ActiveOnly )
		{
			if ( ! pQptr->m_bEnable )
				continue;

			if ( ( ( pQptr->m_nProtocols & ( 1 << PROTOCOL_ED2K ) ) != 0 ) && ( Settings.Connection.RequireForTransfers ) )
			{
				if ( ! ( Settings.eDonkey.EnableAlways | Settings.eDonkey.Enabled ) )
					continue;
			}
		}
		nCount += pQptr->m_nBandwidthPoints;
	}

	return nCount;
}

// CPU Intensive
//DWORD CUploadQueues::GetQueueCapacity()
//{
//	CQuickLock oLock( m_pSection );
//	DWORD nCount = 0;
//
//	for ( POSITION pos = GetIterator() ; pos ; )
//	{
//		nCount += GetNext( pos )->GetQueueCapacity();
//	}
//
//	return nCount;
//}

//DWORD CUploadQueues::GetQueuedCount()
//{
//	CQuickLock oLock( m_pSection );
//	DWORD nCount = 0;
//
//	for ( POSITION pos = GetIterator() ; pos ; )
//	{
//		nCount += GetNext( pos )->GetQueuedCount();
//	}
//
//	return nCount;
//}

//DWORD CUploadQueues::GetQueueRemaining()
//{
//	CQuickLock oLock( m_pSection );
//	DWORD nCount = 0;
//
//	for ( POSITION pos = GetIterator() ; pos ; )
//	{
//		nCount += GetNext( pos )->GetQueueRemaining();
//	}
//
//	return nCount;
//}

//DWORD CUploadQueues::GetTransferCount()
//{
//	CQuickLock oLock( m_pSection );
//	DWORD nCount = 0;
//
//	for ( POSITION pos = GetIterator() ; pos ; )
//	{
//		nCount += GetNext( pos )->GetTransferCount();
//	}
//
//	return nCount;
//}

BOOL CUploadQueues::IsTransferAvailable()
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		if ( GetNext( pos )->GetAvailableBandwidth() > 0 ) return TRUE;
	}

	return FALSE;
}

DWORD CUploadQueues::GetMinimumDonkeyBandwidth()
{
	CQuickLock oLock( m_pSection );

	// Check ED2K ratio limiter
	DWORD nTotal = Settings.Connection.OutSpeed * 128;
	DWORD nLimit = Settings.Bandwidth.Uploads;
	DWORD nDonkeyPoints = 0, nTotalPoints = 0, nBandwidth = 0;

	if ( nLimit == 0 || nLimit > nTotal ) nLimit = nTotal;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadQueue* pQueue = GetNext( pos );

		nTotalPoints += pQueue->m_nBandwidthPoints;

		if ( pQueue->m_nProtocols == 0 || ( pQueue->m_nProtocols & ( 1 << PROTOCOL_ED2K ) ) != 0 )
			nDonkeyPoints += pQueue->m_nBandwidthPoints;
	}

	if ( nTotalPoints < 1 ) nTotalPoints = 1;

	nBandwidth = nLimit * nDonkeyPoints / nTotalPoints;

	return nBandwidth;
}

DWORD CUploadQueues::GetCurrentDonkeyBandwidth()
{
	DWORD nBandwidth = 0;
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadQueue* pQueue = GetNext( pos );

		if ( pQueue->m_nProtocols == 0 || ( pQueue->m_nProtocols & ( 1 << PROTOCOL_ED2K ) ) != 0 )
			nBandwidth += pQueue->GetBandwidthLimit( pQueue->m_nMaxTransfers );
	}

	return nBandwidth;
}

BOOL CUploadQueues::CanUpload(PROTOCOLID nProtocol, CLibraryFile const * const pFile, BOOL bCanQueue )
{ 	// Can the specified file be uploaded with the current queue setup?

	// Don't bother with 0 byte files
	if ( pFile->m_nSize == 0 ) return FALSE;

	// Detect Ghosts
	if ( ! pFile->IsAvailable() ) return FALSE;

	// G1 and G2 both use HTTP transfers, Envy doesn't consider them different.
	if ( nProtocol == PROTOCOL_G1 || nProtocol == PROTOCOL_G2 )
		nProtocol = PROTOCOL_HTTP;

	CQuickLock oLock( m_pSection );

	// Check each queue
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadQueue* pQueue = GetNext( pos );

		if ( pQueue->CanAccept(	nProtocol, pFile->m_sName, pFile->m_nSize, CUploadQueue::ulqLibrary, pFile->m_sShareTags ) )
		{
			// If this queue will accept this file, and we don't care if there is space now, or the queue isn't full
			if ( ! bCanQueue || pQueue->IsFull() )
				return TRUE;	// Then this file can be uploaded
		}
	}

	return FALSE;	// This file is not uploadable with the current queue setup
}

// Unused?
//DWORD CUploadQueues::QueueRank(PROTOCOLID nProtocol, CLibraryFile const * const pFile )
//{	// If the specified file was requested now, what queue position would it be in?
//	// 0x7FFF (max int) indicates the file cannot be downloaded
//	// Don't bother with 0 byte files
//	if ( pFile->m_nSize == 0 ) return 0x7FFF;
//	// Detect Ghosts
//	if ( pFile->IsGhost() ) return 0x7FFF;
//	// G1 and G2 both use HTTP transfers, Envy doesn't consider them different.
//	if ( ( nProtocol == PROTOCOL_G1 ) || ( nProtocol == PROTOCOL_G2 ) )
//		nProtocol = PROTOCOL_HTTP;
//
//	CSingleLock pLock( &m_pSection, TRUE );
//	for ( POSITION pos = GetIterator() ; pos ; )	// Check each queue
//	{
//		CUploadQueue* pQueue = GetNext( pos );
//		if ( pQueue->CanAccept(	nProtocol, pFile->m_sName, pFile->m_nSize, CUploadQueue::ulqLibrary, pFile->m_sShareTags ) )
//		{
//			// If this queue will accept this file
//			if ( pQueue->GetQueueRemaining() > 0 )
//				return pQueue->GetQueuedCount();
//		}
//	}
//	return 0x7FFF;	// This file is not uploadable with the current queue setup
//}

//////////////////////////////////////////////////////////////////////
// CUploadQueues clear

void CUploadQueues::Clear()
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pList.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues load and save

BOOL CUploadQueues::Load()
{
	const CString strFile = Settings.General.DataPath + L"UploadQueues.dat";

	{
		CQuickLock oLock( m_pSection );

		LoadString( m_pTorrentQueue->m_sName, IDS_UPLOAD_QUEUE_TORRENT );
		LoadString( m_pHistoryQueue->m_sName, IDS_UPLOAD_QUEUE_HISTORY );
	}

	BOOL bSuccess = FALSE;

	CFile pFile;
	if ( pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load );	// 4 KB buffer
			try
			{
				CQuickLock oLock( m_pSection );

				Serialize( ar );
				ar.Close();

				bSuccess = TRUE;	// Success
			}
			catch ( CException* pException )
			{
				ar.Abort();
				pFile.Abort();
				pException->Delete();
			}
		}
		catch ( CException* pException )
		{
			pFile.Abort();
			pException->Delete();
		}

		pFile.Close();
	}

	if ( ! bSuccess )
		theApp.Message( MSG_ERROR, L"Failed to load upload queues: %s", strFile );

	if ( GetCount() == 0 )
		CreateDefault();

	Validate();

	return TRUE;
}

BOOL CUploadQueues::Save()
{
	CString strFile = Settings.General.DataPath + L"UploadQueues.dat";
	CString strTemp = Settings.General.DataPath + L"UploadQueues.tmp";

	CFile pFile;
	if ( pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::store );	// 4 KB buffer
			try
			{
				{
					CQuickLock oLock( m_pSection );

					Serialize( ar );
					ar.Close();
				}

				pFile.Close();

				if ( MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
					return TRUE;	// Success
			}
			catch ( CException* pException )
			{
				ar.Abort();
				pFile.Abort();
				pException->Delete();
			}
		}
		catch ( CException* pException )
		{
			pFile.Abort();
			pException->Delete();
		}

		pFile.Close();
		DeleteFile( strTemp );
	}

	theApp.Message( MSG_ERROR, L"Failed to save upload queues: %s", strFile );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues serialize

// Set at INTERNAL_VERSION on change:
#define UPLOADQUEUES_SER_VERSION 1

// nVersion History:
// 6 - Shareaza 2.3 (ryo-oh-ki)
// 1000 - (6)
// 1 - (Envy 1.0)

void CUploadQueues::Serialize(CArchive& ar)
{
	ASSUME_LOCK( m_pSection );

	int nVersion = UPLOADQUEUES_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;

		ar.WriteCount( GetCount() );

		for ( POSITION pos = GetIterator() ; pos ; )
		{
			GetNext( pos )->Serialize( ar, nVersion );
		}
	}
	else // Loading
	{
		Clear();

		ar >> nVersion;
		if ( nVersion > UPLOADQUEUES_SER_VERSION && nVersion != 1000 )
			AfxThrowUserException();

		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			Create()->Serialize( ar, nVersion );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues create default

void CUploadQueues::CreateDefault()
{
	CQuickLock oLock( m_pSection );

	theApp.Message( MSG_NOTICE, L"Creating default upload queues" );

	CUploadQueue* pQueue = NULL;

	Clear();
	CString strQueueName;

	if ( Settings.Experimental.LAN_Mode )
	{
		LoadString( strQueueName, IDS_FILES_ALL );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 40;
		pQueue->m_nProtocols		= 0;
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqBoth;
		pQueue->m_nCapacity			= 1000;
		pQueue->m_nMinTransfers		= 4;
		pQueue->m_nMaxTransfers		= 6;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;
	}
	else if ( Settings.Connection.OutSpeed > 1200 )	// 1200 Kb/s (Massive connection)
	{
		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_PARTIALS );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 40;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 4;
		pQueue->m_nMaxTransfers		= 6;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 50;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nMinTransfers		= 4;
		pQueue->m_nMaxTransfers		= 6;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 5*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_SMALL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMaxSize			= 1 * 1024 * 1024;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRewardUploaders	= FALSE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_MEDIUM_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMinSize			= 1  * 1024 * 1024 + 1;
		pQueue->m_nMaxSize			= 10 * 1024 * 1024 - 1;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRewardUploaders	= FALSE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_LARGE_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMinSize			= 10 * 1024 * 1024;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 3;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 60*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else if ( Settings.Connection.OutSpeed > 800 )	// 800 Kb/s (Fast Broadband)
	{
		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_PARTIALS );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 40;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 3;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 50;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nMinTransfers		= 3;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 5*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_SMALL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMaxSize			= 1 * 1024 * 1024;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRewardUploaders	= FALSE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_MEDIUM_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMinSize			= 1  * 1024 * 1024 + 1;
		pQueue->m_nMaxSize			= 10 * 1024 * 1024 - 1;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRewardUploaders	= FALSE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_LARGE_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMinSize			= 10 * 1024 * 1024;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 3;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 60*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else if ( Settings.Connection.OutSpeed > 250 )	// >250 Kb/s (Good Broadband)
	{
		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_PARTIALS );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 5*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_SMALL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMaxSize			= 10 * 1024 * 1024;
		pQueue->m_nCapacity			= 8;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRewardUploaders	= FALSE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_LARGE_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMinSize			= 10 * 1024 * 1024;
		pQueue->m_nMinTransfers		= 3;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_nCapacity			= 10;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 60*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else if ( Settings.Connection.OutSpeed > 120 )	// >120 Kb/s (Average Broadband)
	{
		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_PARTIALS );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nCapacity			= 1000;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 5*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_COMPLETE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 40;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_nCapacity			= 10;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 60*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else if ( Settings.Connection.OutSpeed > 50 )	// >50 Kb/s (Slow Broadband/ISDN)
	{
		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqBoth;
		pQueue->m_nCapacity			= 500;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 3;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 30*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqPartial;
		pQueue->m_nCapacity			= 8;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 3;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 20*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_COMPLETE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqLibrary;
		pQueue->m_nCapacity			= 8;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 3;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 20*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else  // < 50 Kb/s (Dial up modem)
	{
		LoadString( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqBoth;
		pQueue->m_nCapacity			= 500;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 2;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 30*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString( strQueueName, IDS_UPLOAD_QUEUE_QUEUE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nFileStateFlag	= CUploadQueue::ulqBoth;
		pQueue->m_nCapacity			= 5;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 2;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 20*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}

	Save();
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues validate

void CUploadQueues::Validate()
{
	CQuickLock oLock( m_pSection );

	const bool bHTTP_NoPartial =
		SelectQueue( PROTOCOL_HTTP, L"Filename", 1, CUploadQueue::ulqPartial ) == NULL ||
		SelectQueue( PROTOCOL_HTTP, L"Filename", SIZE_UNKNOWN - 1, CUploadQueue::ulqPartial ) == NULL;
	const bool bHTTP_NoLibrary =
		SelectQueue( PROTOCOL_HTTP, L"Filename", 1, CUploadQueue::ulqLibrary ) == NULL ||
		SelectQueue( PROTOCOL_HTTP, L"Filename", SIZE_UNKNOWN - 1, CUploadQueue::ulqLibrary ) == NULL;

	const bool bED2K_NoPartial =
		SelectQueue( PROTOCOL_ED2K, L"Filename", 1, CUploadQueue::ulqPartial ) == NULL ||
		SelectQueue( PROTOCOL_ED2K, L"Filename", SIZE_UNKNOWN - 1, CUploadQueue::ulqPartial ) == NULL;
	const bool bED2K_NoLibrary =
		SelectQueue( PROTOCOL_ED2K, L"Filename", 1, CUploadQueue::ulqLibrary ) == NULL ||
		SelectQueue( PROTOCOL_ED2K, L"Filename", SIZE_UNKNOWN - 1, CUploadQueue::ulqLibrary ) == NULL;

	const bool bDC_NoPartial =
		SelectQueue( PROTOCOL_DC, L"Filename", 1, CUploadQueue::ulqPartial ) == NULL ||
		SelectQueue( PROTOCOL_DC, L"Filename", SIZE_UNKNOWN - 1, CUploadQueue::ulqPartial ) == NULL;
	const bool bDC_NoLibrary =
		SelectQueue( PROTOCOL_DC, L"Filename", 1, CUploadQueue::ulqLibrary ) == NULL ||
		SelectQueue( PROTOCOL_DC, L"Filename", SIZE_UNKNOWN - 1, CUploadQueue::ulqLibrary ) == NULL;

	if ( bHTTP_NoPartial || bHTTP_NoLibrary )
	{
		CUploadQueue* pQueue		= Create( LoadString( IDS_UPLOAD_QUEUE_HTTP_GUARD ) );
		pQueue->m_nProtocols		= ( 1 << PROTOCOL_HTTP );
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nFileStateFlag	= ( bHTTP_NoPartial && bHTTP_NoLibrary ) ?
			CUploadQueue::ulqBoth : ( bHTTP_NoPartial ? CUploadQueue::ulqPartial : CUploadQueue::ulqLibrary );

		if ( Settings.Connection.OutSpeed > 100 )
		{
			pQueue->m_nMinTransfers		= 2;
			pQueue->m_nBandwidthPoints	= 30;
			pQueue->m_nCapacity			= 10;
			pQueue->m_nRotateTime		= 10*60;
		}
		else
		{
			pQueue->m_nMinTransfers		= 1;
			pQueue->m_nBandwidthPoints	= 20;
			pQueue->m_nCapacity			= 5;
			pQueue->m_nRotateTime		= 30*60;
		}
	}

	if ( bED2K_NoPartial || bED2K_NoLibrary )
	{
		CUploadQueue* pQueue		= Create( LoadString( IDS_UPLOAD_QUEUE_ED2K_GUARD ) );
		pQueue->m_nProtocols		= ( 1 << PROTOCOL_ED2K );
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nFileStateFlag	= ( bED2K_NoPartial && bED2K_NoLibrary ) ?
			CUploadQueue::ulqBoth : ( bED2K_NoPartial ? CUploadQueue::ulqPartial : CUploadQueue::ulqLibrary );

		if ( Settings.Connection.OutSpeed > 100 )
		{
			pQueue->m_nMinTransfers		= 2;
			pQueue->m_nBandwidthPoints	= 30;
			pQueue->m_nCapacity			= 2000;
			pQueue->m_nRotateTime		= 10*60;
			pQueue->m_bRewardUploaders	= TRUE;
		}
		else
		{
			pQueue->m_nMinTransfers		= 1;
			pQueue->m_nBandwidthPoints	= 20;
			pQueue->m_nCapacity			= 500;
			pQueue->m_nRotateTime		= 30*60;
			pQueue->m_bRewardUploaders	= TRUE;
		}
	}

	if ( bDC_NoPartial || bDC_NoLibrary )
	{
		CUploadQueue* pQueue		= Create( LoadString( IDS_UPLOAD_QUEUE_DC_GUARD ) );
		pQueue->m_nProtocols		= ( 1 << PROTOCOL_DC );
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nFileStateFlag	= ( bDC_NoPartial && bDC_NoLibrary ) ?
			CUploadQueue::ulqBoth : ( bDC_NoPartial ? CUploadQueue::ulqPartial : CUploadQueue::ulqLibrary );

		if ( Settings.Connection.OutSpeed > 100 )
		{
			pQueue->m_nMinTransfers		= 2;
			pQueue->m_nBandwidthPoints	= 30;
			pQueue->m_nCapacity			= 10;
			pQueue->m_nRotateTime		= 10*60;
		}
		else
		{
			pQueue->m_nMinTransfers		= 1;
			pQueue->m_nBandwidthPoints	= 20;
			pQueue->m_nCapacity			= 5;
			pQueue->m_nRotateTime		= 30*60;
		}
	}

	m_bDonkeyLimited = ( GetMinimumDonkeyBandwidth() < 10240 );

	// Display warning if needed
	if ( m_bDonkeyLimited && Settings.eDonkey.Enabled || Settings.eDonkey.EnableAlways )
		theApp.Message( MSG_NOTICE, L"eDonkey upload ratio active: Low uploads may slow downloads." );
}
