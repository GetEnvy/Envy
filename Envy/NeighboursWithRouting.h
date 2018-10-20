//
// NeighboursWithRouting.h
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

// Adds methods that send packets and Gnutella queries to all the computers we're connected to
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CNeighboursWithRouting
// http://getenvy.com/archives/shareazawiki/Developers.Code.CNeighboursWithRouting.html

#pragma once

#include "NeighboursWithED2K.h"

class CPacket;
class CQuerySearch;

// Add methods to broadcast packets and Gnutella queries to all the neighbours in the list
// Continue the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
class CNeighboursWithRouting : public CNeighboursWithED2K
{
protected:
	CNeighboursWithRouting();			// Constructor and destructor don't do anything
	virtual ~CNeighboursWithRouting();

protected:
	typedef struct
	{
		IN_ADDR	m_pAddress;
		DWORD	m_nTime;
	} CIPTime;

	CList< CIPTime > m_pQueries;

public:
	virtual void Connect();

	// Send a packet to all the computers we're connected to
	int Broadcast(CPacket* pPacket, CNeighbour* pExcept = NULL, BOOL bGGEP = FALSE);

	// Limit queries by endpoint addresses
	bool CheckQuery(const CQuerySearch* pSearch);

	// Send a query packet to all the computers we're connected to, translating it to Gnutella and Gnutella2 for computers running that software
	int RouteQuery(const CQuerySearch* pSearch, CPacket* pPacket, CNeighbour* pFrom, BOOL bToHubs);
};
