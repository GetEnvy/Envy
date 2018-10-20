//
// UploadTransferBT.h
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

#include "UploadTransfer.h"
#include "FileFragments.hpp"

class CBTClient;
class CBTPacket;


class CUploadTransferBT : public CUploadTransfer
{
public:
	CUploadTransferBT(CBTClient* pClient, CDownload* pDownload);
	virtual ~CUploadTransferBT();

public:
	CBTClient*		m_pClient;
	CDownload*		m_pDownload;

	BOOL			m_bInterested;
	BOOL			m_bChoked;
	int				m_nRandomUnchoke;
	DWORD			m_tRandomUnchoke;
private:
	Fragments::Queue m_oRequested;
	Fragments::Queue m_oServed;

public:
	void			SetChoke(BOOL bChoke);
	virtual void	Close(UINT nError = 0);
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	OnConnected();
	virtual BOOL	OnRun();
public:
	BOOL	OnInterested(CBTPacket* pPacket);
	BOOL	OnUninterested(CBTPacket* pPacket);
	BOOL	OnRequest(CBTPacket* pPacket);
	BOOL	OnCancel(CBTPacket* pPacket);
protected:
	virtual BOOL	OpenFile();
	BOOL	ServeRequests();
};
