//
// PongCache.h
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

class CPongItem;
class CNeighbour;
class CG1Packet;


class CPongCache
{
public:
	CPongCache();
	~CPongCache();

public:
	void		Clear();
	void		ClearNeighbour(CNeighbour* pNeighbour);
	BOOL		ClearIfOld();
	CPongItem*	Add(CNeighbour* pNeighbour, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);
	CPongItem*	Lookup(CNeighbour* pNotFrom, BYTE nHops, CList< CPongItem* >* pIgnore) const;
	CPongItem*	Lookup(CNeighbour* pFrom) const;

//	POSITION	GetIterator() const;
//	CPongItem*	GetNext(POSITION& pos) const;

protected:
	CList< CPongItem* >	m_pCache;
	DWORD		m_nTime;
};


class CPongItem
{
public:
	CPongItem(CNeighbour* pNeighbour, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);
//	virtual ~CPongItem();

public:
	CNeighbour*	m_pNeighbour;
	IN_ADDR		m_pAddress;
	WORD		m_nPort;
	BYTE		m_nHops;
	DWORD		m_nFiles;
	DWORD		m_nVolume;

	CG1Packet*	ToPacket(int nTTL, const Hashes::Guid& oGUID) const;
};
