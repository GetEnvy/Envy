//
// UploadTransferED2K.h
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

#include "UploadTransfer.h"
#include "FileFragments.hpp"

class CEDClient;
class CEDPacket;

class CUploadTransferED2K : public CUploadTransfer
{
public:
	CUploadTransferED2K(CEDClient* pClient);
	virtual ~CUploadTransferED2K();

public:
	CEDClient*		m_pClient;					// The remote client.
	int				m_nRanking;					// The last queue position the remote client was sent.
	DWORD			m_tRankingSent;				// The time a queue ranking packet was last sent.
	DWORD			m_tRankingCheck;			// The time the queue position was last checked.
	DWORD			m_tLastRun;					// The time the transfer was last run
private:
	Fragments::Queue m_oRequested;
	Fragments::Queue m_oServed;

public:
	BOOL			Request(const Hashes::Ed2kHash& oED2K);
	virtual void	Close(UINT nError = 0);
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual void	OnQueueKick();
	virtual DWORD	GetMeasuredSpeed();
public:
	BOOL	OnRunEx(DWORD tNow);
	BOOL	OnQueueRelease(CEDPacket* pPacket);
	BOOL	OnRequestParts(CEDPacket* pPacket);
	BOOL	OnReask();
protected:
	void	Cleanup(BOOL bDequeue = TRUE);
	void	Send(CEDPacket* pPacket, BOOL bRelease = TRUE);
	BOOL	CheckRanking();		// Check the client's Q rank. Start upload or send notification if required.
	void	AddRequest(QWORD nOffset, QWORD nLength);
	BOOL	ServeRequests();
	BOOL	StartNextRequest();
	BOOL	DispatchNextChunk();
	BOOL	CheckFinishedRequest();

public:
	BOOL	OnRequestParts64(CEDPacket* pPacket);		// 64bit Large file support
};
