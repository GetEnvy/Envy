//
// DownloadTransferED2K.h
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

#pragma once

#include "DownloadTransfer.h"
#include "zlib.h"

class CEDClient;
class CEDPacket;

class CDownloadTransferED2K : public CDownloadTransfer
{
public:
	CDownloadTransferED2K(CDownloadSource* pSource);
	virtual ~CDownloadTransferED2K();

public:
	CEDClient*		m_pClient;
	DWORD			m_tRanking;			// When queue ranking was last received
	bool			m_bHashset;
	bool			m_bUDP;
protected:
	z_streamp		m_pInflatePtr;
	CBuffer*		m_pInflateBuffer;
	QWORD			m_nInflateOffset;
	QWORD			m_nInflateLength;
	QWORD			m_nInflateRead;
	QWORD			m_nInflateWritten;

public:
	BOOL	OnRunEx(DWORD tNow);
	BOOL	OnFileReqAnswer(CEDPacket* pPacket);
	BOOL	OnFileNotFound(CEDPacket* pPacket);
	BOOL	OnFileStatus(CEDPacket* pPacket);
	BOOL	OnHashsetAnswer(CEDPacket* pPacket);
	BOOL	OnQueueRank(CEDPacket* pPacket);
	BOOL	OnRankingInfo(CEDPacket* pPacket);
	BOOL	OnFileComment(CEDPacket* pPacket);
	BOOL	OnStartUpload(CEDPacket* pPacket);
	BOOL	OnFinishUpload(CEDPacket* pPacket);
	BOOL	OnSendingPart(CEDPacket* pPacket);
	BOOL	OnCompressedPart(CEDPacket* pPacket);
	BOOL	OnSendingPart64(CEDPacket* pPacket);		// 64bit Large file support
	BOOL	OnCompressedPart64(CEDPacket* pPacket);
	void	SetQueueRank(int nRank);
protected:
	void	Send(CEDPacket* pPacket, BOOL bRelease = TRUE);
	BOOL	SendPrimaryRequest();
	BOOL	SendSecondaryRequest();
	void	ClearRequests();
	BOOL	RunQueued(DWORD tNow);
//	BOOL	SelectFragment(const Fragments::List& oPossible, QWORD& nOffset, QWORD& nLength);

public:
	virtual BOOL	Initiate();
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost();
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const;
protected:
	virtual bool	SendFragmentRequests();
};
