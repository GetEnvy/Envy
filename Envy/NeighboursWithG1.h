//
// NeighboursWithG1.h
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

// Adds the ping route and pong caches to the CNeighbours object, and methods to route Gnutella ping and pong packets
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CNeighboursWithG1
// http://getenvy.com/archives/shareazawiki/Developers.Code.CNeighboursWithG1.html

#pragma once

#include "NeighboursBase.h"
#include "QuerySearch.h"
#include "PongCache.h"
#include "RouteCache.h"


class CNeighbour;
class CG1Neighbour;


// Add the ping route and pong caches to the CNeighbours object
// Inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
class CNeighboursWithG1 : public CNeighboursBase
{
//Construction
protected:
	CNeighboursWithG1();			// Setup ping route and pong caches
	virtual ~CNeighboursWithG1();	// Delete ping route and pong cache objects

private:
	CRouteCache	m_pPingRoute;
	CPongCache	m_pPongCache;
	DWORD		m_tLastPingOut; 	// When we last sent a multicast ping packet

public:
	void OnG1Ping();					// Relay ping and pong packets to other neighbours
	void OnG1Pong(CG1Neighbour* pFrom, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);

	// Send multicast ping
	void SendPing();
	// Send multicast query
	void SendQuery(CQuerySearchPtr pSearch);

	BOOL AddPingRoute(const Hashes::Guid& oGUID, const CG1Neighbour* pNeighbour);
	CG1Neighbour* GetPingRoute(const Hashes::Guid& oGUID);

	CPongItem*	AddPong(CNeighbour* pNeighbour, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);
	CPongItem*	LookupPong(CNeighbour* pNotFrom, BYTE nHops, CList< CPongItem* >* pIgnore);

protected:
	virtual void Remove(CNeighbour* pNeighbour);	// Remove a neighbour from the ping route and pong caches, network object, and the list
	virtual void Connect(); 			// Sets the ping route duration from settings
	virtual void Close();				// Call Close on each neighbour in the list, reset member variables to 0, and clear the ping route and pong caches
	virtual void OnRun();
};
