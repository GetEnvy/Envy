//
// SearchManager.h
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

#include "ManagedSearch.h"

class CG2Packet;
class CQueryHit;


class CSearchManager
{
public:
	CSearchManager();
	~CSearchManager();

public:
	void			OnRun();
	BOOL			OnQueryAck(CG2Packet* pPacket, const SOCKADDR_IN* pAddress, Hashes::Guid& oGUID);
	BOOL			OnQueryHits(const CQueryHit* pHits);
	WORD			OnQueryStatusRequest(const Hashes::Guid& oGUID);

protected:
	typedef CList< CManagedSearch* > CSearchList;

	CMutexEx		m_pSection;
	CSearchList		m_pList;
	DWORD			m_tLastTick;
	int				m_nPriorityClass;
	int				m_nPriorityCount;
	Hashes::Guid	m_oLastSearch;

	bool			Add(CManagedSearch* pSearch);
	bool			Remove(CManagedSearch* pSearch);
	CSearchPtr		Find(const Hashes::Guid& oGUID) const;

	friend class CManagedSearch;	// m_pSection, m_oLastSearch, Add(), Remove()
};

extern CSearchManager SearchManager;
