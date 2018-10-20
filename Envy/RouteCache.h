//
// RouteCache.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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

#define ROUTE_HASH_SIZE		1024
#define ROUTE_HASH_MASK		1023

class CNeighbour;


class CRouteCacheItem
{
public:
	CRouteCacheItem();

	CRouteCacheItem*	m_pNext;
	const CNeighbour*	m_pNeighbour;
	SOCKADDR_IN			m_pEndpoint;
	Hashes::Guid		m_oGUID;
	DWORD				m_tAdded;
};


class CRouteCacheTable
{
public:
	CRouteCacheTable();
	virtual ~CRouteCacheTable();

protected:
	CRouteCacheItem*	m_pHash[ ROUTE_HASH_SIZE ];
	CRouteCacheItem*	m_pFree;
	CRouteCacheItem*	m_pBuffer;
	DWORD				m_nBuffer;
	DWORD				m_nUsed;
	DWORD				m_tFirst;
	DWORD				m_tLast;

public:
	CRouteCacheItem*	Find(const Hashes::Guid& oGUID);
	CRouteCacheItem*	Add(const Hashes::Guid& oGUID, const CNeighbour* pNeighbour, const SOCKADDR_IN* pEndpoint, DWORD nTime = 0);
	void				Remove(CNeighbour* pNeighbour);
	void				Resize(DWORD nSize);
	DWORD				GetNextSize(DWORD nDesired);
	void				Clear();

	inline BOOL IsFull() const
	{
		return m_nUsed == m_nBuffer;
	}
};


class CRouteCache
{
public:
	CRouteCache();
	virtual ~CRouteCache();

protected:
	DWORD				m_nSeconds;
	CRouteCacheTable	m_pTable[2];
	CRouteCacheTable*	m_pRecent;
	CRouteCacheTable*	m_pHistory;

public:
	void				SetDuration(DWORD nSeconds);
	BOOL				Add(const Hashes::Guid& oGUID, const CNeighbour* pNeighbour);
	BOOL				Add(const Hashes::Guid& oGUID, const SOCKADDR_IN* pEndpoint);
	void				Remove(CNeighbour* pNeighbour);
	void				Clear();

	CRouteCacheItem*	Add(const Hashes::Guid& oGUID, const CNeighbour* pNeighbour, const SOCKADDR_IN* pEndpoint, DWORD tAdded);
	CRouteCacheItem*	Lookup(const Hashes::Guid& oGUID, CNeighbour** ppNeighbour = NULL, SOCKADDR_IN* pEndpoint = NULL);
};
