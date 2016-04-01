//
// DownloadTransfer.h
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

#include "Transfer.h"
#include "FileFragments.hpp"

class CDownload;
class CDownloadSource;


typedef std::pair< QWORD, QWORD > blockPair;

class CDownloadTransfer abstract : public CTransfer
{
public:
	CDownloadTransfer(CDownloadSource* pSource, PROTOCOLID nProtocol);
	virtual ~CDownloadTransfer();

public:
	CDownloadTransfer*	m_pDlPrev;
	CDownloadTransfer*	m_pDlNext;

	CString				m_sQueueName;
	DWORD				m_nQueuePos;
	DWORD				m_nQueueLen;
	QWORD				m_nDownloaded;

	BOOL				m_bWantBackwards;
	BOOL				m_bRecvBackwards;		// Got "Content-Encoding: backwards"

protected:
	CDownload*			m_pDownload;
	CDownloadSource*	m_pSource;
	std::vector< bool >	m_pAvailable;
	CTimeAverage< DWORD, 2000 >	m_AverageSpeed;

	DWORD				m_tSourceRequest;		// When source request was last sent (ms)
	Fragments::Queue	m_oRequested;			// List of requested fragments (ED2K/BitTorrent)

public:
	CDownload*			GetDownload() const;	// Get owner download
	CDownloadSource*	GetSource() const;		// Get associated source
	void				SetState(int nState);
	void				DrawStateBar(CDC* pDC, CRect* prcBar, COLORREF crFill, BOOL bTop = FALSE) const;
protected:
	void				ChunkifyRequest(QWORD* pnOffset, QWORD* pnLength, DWORD nChunk, BOOL bVerifyLock) const;
	bool				SelectFragment(const Fragments::List& oPossible, QWORD& nOffset, QWORD& nLength, bool bEndGame = false) const;
private:
	blockPair			SelectBlock(const Fragments::List& oPossible, const std::vector< bool >& pAvailable, bool bEndGame) const;
	void				CheckPart(QWORD* nPart, QWORD nPartBlock, QWORD* nRange, QWORD& nRangeBlock, QWORD* nBestRange) const;
	void				CheckRange(QWORD* nRange, QWORD* nBestRange) const;

public:
	virtual BOOL	Initiate() = 0;
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost(BOOL bBoost = TRUE);
	virtual DWORD	GetAverageSpeed();
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const = 0;
	virtual bool	UnrequestRange(QWORD /*nOffset*/, QWORD /*nLength*/);
	virtual CString	GetStateText(BOOL bLong);
	virtual BOOL	OnRun();
protected:
	virtual bool	SendFragmentRequests() {return false;} /*= 0*/;
};

enum
{
	dtsNull, dtsConnecting, dtsRequesting, dtsHeaders, dtsDownloading,
	dtsFlushing, dtsTiger, dtsHashset, dtsMetadata, dtsBusy, dtsEnqueue, dtsQueued,
	dtsTorrent,

	dtsCountAll = -1,
	dtsCountNotQueued = -2,
	dtsCountNotConnecting = -3,
	dtsCountTorrentAndActive = -4
};
