//
// EDClients.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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

#include "EDClient.h"

class CConnection;
class CEDPacket;


class CEDClients
{
public:
	CEDClients();
	virtual ~CEDClients();

private:
	CEDClients(const CEDClients&);				// Declaration only
	CEDClients& operator=(const CEDClients&);	// Declaration only

public:
	mutable CMutex	m_pSection;		// EDClients Guard

private:
	CEDClient*		m_pFirst;
	CEDClient*		m_pLast;
	int				m_nCount;
	DWORD			m_tLastRun;
	DWORD			m_tLastMaxClients;

public:
	void			Add(CEDClient* pClient);
	void			Remove(CEDClient* pClient);
	void			Clear();
	int				GetCount() const;
	bool			PushTo(DWORD nClientID, WORD nClientPort);
	CEDClient*		GetByIP(const IN_ADDR* pAddress) const;
	CEDClient*		Connect(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, const Hashes::Guid& oGUID);
					// Connect to new or known eD2K-client (nClientPort and nServerPort must be in host byte order)
	BOOL			Merge(CEDClient* pClient);
	void			OnRun();
	BOOL			OnAccept(CConnection* pConnection);
	BOOL			OnPacket(const SOCKADDR_IN* pHost, CEDPacket* pPacket);
	bool			IsFull(const CEDClient* pCheckThis = NULL);
	BOOL			IsOverloaded() const;
	BOOL			IsMyDownload(const CDownloadTransferED2K* pDownload) const;

protected:
	CEDClient*		GetByID(DWORD nClientID, IN_ADDR* pServer, const Hashes::Guid& oGUID) const;
	CEDClient*		GetByGUID(const Hashes::Guid& oGUID) const;

	BOOL			OnServerStatus(const SOCKADDR_IN* pHost, CEDPacket* pPacket);		// Server status packet received
	BOOL			OnServerSearchResult(const SOCKADDR_IN* pHost, CEDPacket* pPacket);	// Server search result packet received
};

extern CEDClients EDClients;
