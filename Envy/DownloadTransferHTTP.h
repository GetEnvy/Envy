//
// DownloadTransferHTTP.h
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

#pragma once

#include "DownloadTransfer.h"


class CDownloadTransferHTTP : public CDownloadTransfer
{
public:
	CDownloadTransferHTTP(CDownloadSource* pSource);
	virtual ~CDownloadTransferHTTP();

protected:
	DWORD			m_nRequests;
	DWORD			m_tContent;
	BOOL			m_bBadResponse;
	BOOL			m_bBusyFault;
	BOOL			m_bRangeFault;
	BOOL			m_bKeepAlive;
	BOOL			m_bTigerFetch;
	BOOL			m_bTigerIgnore;
	CString			m_sTigerTree;		// X-TigerTree-Path
	CString			m_sMetadata;		// X-Metadata-Path
	BOOL			m_bMetaFetch;
	BOOL			m_bGotRange;
	BOOL			m_bGotRanges;
	BOOL			m_bQueueFlag;
	QWORD			m_nContentLength;
	CString			m_sContentType;
	DWORD			m_nRetryDelay;		// Delay for queuing
	DWORD			m_nRetryAfter;		// Got "Retry-After: x" seconds
	BOOL			m_bRedirect;
	CString			m_sRedirectionURL;
	BOOL			m_bGzip;			// Got "Content-Encoding: gzip" or x-gzip
	BOOL			m_bCompress;		// Got "Content-Encoding: compress" or x-compress
	BOOL			m_bDeflate;			// Got "Content-Encoding: deflate"
	BOOL			m_bChunked;			// Got "Transfer-Encoding: chunked"
	enum ChunkState
	{
		Header,							// Reading chunk header "Length<CR><LF>"
		Body,							// Reading chunk body
		BodyEnd,						// Waiting for chunk body ending "<CR><LF>"
		Footer							// Bypassing data after last chunk
	};
	ChunkState		m_ChunkState;
	QWORD			m_nChunkLength;

public:
	virtual BOOL	Initiate();
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost();
	virtual DWORD	GetAverageSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const;
	virtual BOOL	OnRun();

protected:
	BOOL			StartNextFragment();
	BOOL			SendRequest();
	BOOL			ReadResponseLine();
	BOOL			ReadContent();
	BOOL			ReadTiger(bool bDropped = false);
	BOOL			ReadMetadata();
	BOOL			ReadFlush();

	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual void	OnDropped();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();
};
