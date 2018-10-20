//
// DownloadTransferBT.h
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

class CBTClient;
class CBTPacket;


class CDownloadTransferBT : public CDownloadTransfer
{
public:
	CDownloadTransferBT(CDownloadSource* pSource, CBTClient* pClient);
	virtual ~CDownloadTransferBT();

public:
	CBTClient*		m_pClient;
	BOOL			m_bChoked;
	BOOL			m_bInterested;

	BOOL			m_bAvailable;
	DWORD			m_tRunThrottle;

public:
	BOOL			OnBitfield(CBTPacket* pPacket);
	BOOL			OnHave(CBTPacket* pPacket);
	BOOL			OnChoked(CBTPacket* pPacket);
	BOOL			OnUnchoked(CBTPacket* pPacket);
	BOOL			OnPiece(CBTPacket* pPacket);
	BOOL			OnSourceResponse(CBTPacket* pPacket);
	void			SendFinishedBlock(DWORD nBlock);
protected:
	void			ShowInterest();

public:
	virtual BOOL	Initiate();
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	Boost();
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual DWORD	GetMeasuredSpeed();
	virtual CString	GetStateText(BOOL bLong);
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const;
	virtual bool	UnrequestRange(QWORD nOffset, QWORD nLength);
protected:
	virtual bool	SendFragmentRequests();
};
