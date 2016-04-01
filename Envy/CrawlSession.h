//
// CrawlSession.h
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

class CG2Packet;
class CCrawlNode;


class CCrawlSession
{
public:
	CCrawlSession();
	virtual ~CCrawlSession();

public:
	BOOL		m_bActive;
	CList< CCrawlNode*, CCrawlNode* > m_pNodes;

public:
	void		Clear();
	void		Bootstrap();
	void		SendCrawl(SOCKADDR_IN* pHost);
	int			GetHubCount();
	int			GetLeafCount();

	void		OnRun();
	void		OnCrawl(const SOCKADDR_IN* pHost, CG2Packet* pPacket);

protected:
	CCrawlNode*	Find(const IN_ADDR* pAddress, BOOL bCreate);

	friend class CCrawlNode;
};


class CCrawlNode
{
public:
	CCrawlNode();
	virtual ~CCrawlNode();

public:
	SOCKADDR_IN		m_pHost;
	int				m_nType;
	int				m_nLeaves;
	CString			m_sNick;
	float			m_nLatitude;
	float			m_nLongitude;

	POSITION		m_nUnique;
	DWORD			m_tDiscovered;
	DWORD			m_tCrawled;
	DWORD			m_tResponse;

	CList< CCrawlNode* > m_pNeighbours;

	enum { ntUnknown, ntHub, ntLeaf };

public:
	void	OnCrawl(CCrawlSession* pSession, CG2Packet* pPacket);
protected:
	void	OnNode(CCrawlSession* pSession, CG2Packet* pPacket, DWORD nPacket, int nType);

	enum { parseSelf, parseHub, parseLeaf };
};

extern CCrawlSession CrawlSession;
