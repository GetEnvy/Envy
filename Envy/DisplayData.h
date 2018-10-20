//
// DisplayData.h
//
// This file is part of Envy (getenvy.com) © 2014
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

#pragma once

#include "FileFragments.hpp"

class CDownload;
class CDownloadSource;
class CDownloadDisplayData;
class CSourceDisplayData;
class CUploadFile;
class CUploadQueue;
class CUploadTransfer;
class CUploadDisplayData;
class CQueueDisplayData;


class CDownloadDisplayData
{
public:
	CDownloadDisplayData();
	CDownloadDisplayData(const CDownload* pDownload);
	CDownloadDisplayData& operator=(const CDownloadDisplayData& pDownload);

struct VERIFYRANGE
{
	QWORD			nOffset;
	QWORD			nLength;
	BOOL			bSuccess;
};

public:
	QWORD			m_nSize;				// pDownload->m_nSize
	CString			m_sName;				// pDownload->m_sName
	CString			m_sDisplayName;			// pDownload->GetDisplayName()
	BOOL			m_bSelected;			// pDownload->m_bSelected
	BOOL			m_bExpanded;			// pDownload->m_bExpanded
	BOOL			m_bExpandable;			// IsExpandable(pDownload)
	BOOL			m_bTrying;				// pDownload->IsTrying()	(m_tBegan>0)
	BOOL			m_bClearing;			// pDownload->m_bClearing
	BOOL			m_bCompleted;			// pDownload->IsCompleted()
	BOOL			m_bFailedVerify;		// pDownload->m_bVerify == TRI_FALSE
	DWORD			m_nVolumeComplete;		// pDownload->GetVolumeComplete()		Or Seeding: pDownload->m_nTorrentUploaded
	float			m_fProgress;			// pDownload->GetProgress()
	float			m_fRatio;				// pDownload->GetRatio()	Seeding
	UINT			m_nRating;				// pDownload->GetReviewAverage()
	DWORD			m_nAverageSpeed;		// pDownload->GetAverageSpeed()	DWORD
	CString			m_sDownloadSources;		// pDownload->GetDownloadSources()
	BOOL			m_bMultiFileTorrent;	// pDownload->IsMultiFileTorrent()
	BOOL			m_bSeeding;				// pDownload->IsSeeding()
	BOOL			m_bSearching;			// pDownload->IsSearching()
	CString			m_sDownloadStatus;		// pDownload->GetDownloadStatus()
	DWORD			m_nSourceCount;			// pDownload->GetSourceCount()
	Fragments::List	m_oEmptyFragments;
	CArray< VERIFYRANGE > m_pVerifyRanges;	// pDownload->GetNextVerifyRange()
	CArray< CSourceDisplayData > m_pSourcesData;
};


class CSourceDisplayData
{
public:
	CSourceDisplayData();
	CSourceDisplayData(const CDownloadSource* pSource);
	CSourceDisplayData& operator=(const CSourceDisplayData& pSource);

public:
	BOOL			m_bSelected;			// pSource->m_bSelected
	DWORD			m_nProtocol;			// pSource->m_nProtocol
	QWORD			m_nSize;				// pDownload->m_nSize
	DWORD			m_nState;				// pSource->GetState()
	CString			m_sState;				// pSource->GetState( FALSE )
	DWORD			m_tAttempt;				// pSource->m_tAttempt
	BOOL			m_bTrying;				// pSource->m_tAttempt && pDownload->IsTrying()
	BOOL			m_bIdle;				// pSource->IsIdle()
	BOOL			m_bPushOnly;			// pSource->m_bPushOnly		(ed2k)
	QWORD			m_nDownloaded;			// pSource->GetDownloaded()
	DWORD			m_nSpeed;				// pSource->GetMeasuredSpeed()
	CString			m_sServer;				// pSource->m_sServer
	DWORD			m_nAddress;				// pSource->m_pAddress.S_un.S_addr
	CString			m_sAddress;				// CString( inet_ntoa( pSource->m_pAddress )	Or ed2k/dc: CString( inet_ntoa( pSource->m_pServerAddress )
	CString			m_sAddressGet;			// pSource->GetAddress()
	WORD			m_nServerPort;			// pSource->m_nServerPort
	WORD			m_nPort;				// pSource->m_nPort
	WORD			m_nPortGet;				// pSource->GetPort()
	CString			m_sNick;				// pSource->m_sNick
	CString			m_sCountry;				// pSource->m_sCountry
	int				m_nColor;				// m_pDownload->GetSourceColor() index
	BOOL			m_bReadContent;			// pSource->m_bReadContent
	BOOL			m_bHasFragments;		// IsOnline() && HasUsefulRanges() || ! m_oPastFragments.empty()
	BOOL			m_bTransferBackwards;	// m_pTransfer->m_bRecvBackwards
	DWORD			m_nTransferLength;		// m_pTransfer->m_nLength
	DWORD			m_nTransferOffset;		// m_pTransfer->m_nOffset
	DWORD			m_nTransferPosition;	// m_pTransfer->m_nPosition
	Fragments::List	m_oAvailable;			// pSource->m_oAvailable
	Fragments::List	m_oPastFragments;		// pSource->m_oPastFragments
};


class CUploadDisplayData
{
public:
	CUploadDisplayData();
	CUploadDisplayData(const CUploadFile* pUpload, CUploadTransfer* pTransfer);
	CUploadDisplayData& operator=(const CUploadDisplayData& pUpload);

public:
	CString			m_sName;				// pUpload->m_sName
	CString			m_sPath;				// pUpload->m_sPath
	QWORD			m_nSize;				// pUpload->m_nSize
	QWORD			m_nUploaded;			// pTransfer->m_nUploaded
	DWORD			m_nSpeed;				// pTransfer->GetMeasuredSpeed()
	DWORD			m_nProtocol;			// pTransfer->m_nProtocol
	BOOL			m_bSelected;			// pUpload->m_bSelected
	BOOL			m_bTorrent;				// pTransfer->m_nProtocol == PROTOCOL_BT
	BOOL			m_bTransferNull;		// pTransfer == NULL || pTransfer->m_nState == upsNull
	BOOL			m_bBaseFile;			// pUpload == pUploadTransfer->m_pBaseFile
	BOOL			m_bBackwards;			// pUpload->IsBackwards() && PROTOCOL_HTTP
	CString			m_sState;				// (Speed column LoadString)
	CString			m_sCountry;				// pTransfer->m_sCountry
	CString			m_sAddress;				// pTransfer->m_sAddress
	CString			m_sRemoteNick;			// pTransfer->m_sRemoteNick
	CString			m_sUserAgent;			// pTransfer->m_sUserAgent
	DWORD			m_nUserRating;			// pTransfer->m_nUserRating
	QWORD			m_nLength;				// pTransfer->m_nLength
	QWORD			m_nOffset;				// pTransfer->m_nOffset
	QWORD			m_nPosition;			// pTransfer->m_nPosition
	Fragments::List	m_oFragments;			// pFile->m_oFragments
};


class CQueueDisplayData
{
public:
	CQueueDisplayData();
	CQueueDisplayData(const CUploadQueue* pQueue);
	CQueueDisplayData& operator=(const CQueueDisplayData& pQueue);

public:
	CString			m_sName;				// pQueue->m_sName
	BOOL			m_bSelected;			// pQueue->m_bSelected
	BOOL			m_bExpanded;			// pQueue->m_bExpanded
	BOOL			m_bHistoryQueue;		// pQueue == UploadQueues.m_pHistoryQueue
	BOOL			m_bTorrentQueue;		// pQueue == UploadQueues.m_pTorrentQueue
	BOOL			m_bHTTPQueue;			// pQueue->m_nProtocols == ( 1 << PROTOCOL_HTTP )
	BOOL			m_bED2KQueue;			// pQueue->m_nProtocols == ( 1 << PROTOCOL_ED2K )
	DWORD			m_nMinTransfers;		// pQueue->GetTransferCount()	(Torrent: pQueue->m_nMinTransfers)
	DWORD			m_nMaxTransfers;		// pQueue->GetQueuedCount() 	(Torrent: pQueue->m_nMaxTransfers)
	DWORD			m_nSpeed;				// pQueue->GetMeasuredSpeed()
	DWORD			m_nCount;				// Uploads
	CArray< CUploadDisplayData > m_pUploadsData;
};
