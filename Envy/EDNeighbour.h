//
// EDNeighbour.h
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

#include "Neighbour.h"

class CEDPacket;
class CDownloadWithTiger;


class CEDNeighbour : public CNeighbour
{
public:
	CEDNeighbour();
	virtual ~CEDNeighbour();

public:
	DWORD		m_nClientID;
	DWORD		m_nUserCount;
	DWORD		m_nUserLimit;
	DWORD		m_nFileLimit;
	DWORD		m_nTCPFlags;
	DWORD		m_nUDPFlags;
	DWORD		m_nFilesSent;
	CList< Hashes::Guid > m_pQueries;

public:
	BOOL	SendSharedDownload(const CDownloadWithTiger* pDownload);
	DWORD	GetID() const;
private:
	BOOL	OnPacket(CEDPacket* pPacket);
	BOOL	OnRejected(CEDPacket* pPacket);
	BOOL	OnServerMessage(CEDPacket* pPacket);
	BOOL	OnIdChange(CEDPacket* pPacket);
	BOOL	OnServerList(CEDPacket* pPacket);
	BOOL	OnServerStatus(CEDPacket* pPacket);
	BOOL	OnServerIdent(CEDPacket* pPacket);
	BOOL	OnCallbackRequested(CEDPacket* pPacket);
	BOOL	OnSearchResults(CEDPacket* pPacket);
	BOOL	OnFoundSources(CEDPacket* pPacket);
	BOOL	SendLogin();
	BOOL	SendSharedFiles();
	bool	IsGoodSize(QWORD nFileSize) const;		// Acceptable file for current ed2k-server

public:
	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic);
	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	virtual DWORD   GetUserCount() const { return m_nUserCount; }
	virtual DWORD   GetUserLimit() const { return m_nUserLimit; }

	virtual BOOL	ProcessPackets(CBuffer* pInput);	// Process packets from input buffer

protected:
	virtual BOOL	ProcessPackets();					// Process packets from internal input buffer

	virtual BOOL	SendQuery(const CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
};
