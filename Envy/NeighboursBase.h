//
// NeighboursBase.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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

// Keeps a list of CNeighbour objects, with methods to go through them, add and remove them, and count them
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CNeighboursBase
// http://getenvy.com/shareazawiki/Developers.Code.CNeighboursBase.html

#pragma once

class CNeighbour;

// Keeps a list of CNeighbour objects, with methods to go through them, add and remove them, and count them
// Begin the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase

class CNeighboursBase
{
protected:
	CNeighboursBase();
	virtual ~CNeighboursBase();

public:
	POSITION    GetIterator() const;				// Call GetIterator to get the POSITION value
	CNeighbour* GetNext(POSITION& pos) const;		// Give the POSITION to GetNext to get the neighbour beneath it and move to the next one
	CNeighbour* Get(DWORD_PTR nUnique) const;		// Lookup a neighbour by its unique number, like 2, 3, 4, and so on
	CNeighbour* Get(const IN_ADDR& pAddress) const;	// Lookup a neighbour by the remote computer's IP address
	CNeighbour* GetNewest(PROTOCOLID nProtocol, int nState, int nNodeType) const;	// Finds the newest neighbour object

	// Count how many computers we are connected to, specifying various filtering characteristics
	// pass -1 to not filter by protocol, state, or node type
	DWORD GetCount(PROTOCOLID nProtocol, int nState, int nNodeType) const;
	//BOOL NeighbourExists(PROTOCOLID nProtocol, int nState, int nNodeType) const;	// Use this if you just want to know if there are any or not

	// Add and remove neighbour objects from the list
	virtual void Add(CNeighbour* pNeighbour);
	virtual void Remove(CNeighbour* pNeighbour);

protected:
	// Methods implemented by several classes in the CNeighbours inheritance column
	virtual void Connect();	// Does nothing, but inheriting classes have Connect methods with code in them
	virtual void Close();	// Calls Close on all the neighbours in the list, and resets member variables back to 0
	virtual void OnRun();	// Calls DoRun on each neighbour in the list, making them send and receive data

private:
	typedef CMap< IN_ADDR, const IN_ADDR&, CNeighbour*, CNeighbour*& > CAMap;
	typedef CMap< DWORD_PTR, const DWORD_PTR&, CNeighbour*, CNeighbour*& > CNMap;

	CAMap	m_pNeighbours;	// The list of remote computers we are connected to
	CNMap	m_pIndex;		// Additional index
	DWORD	m_nRunCookie;	// OnRun uses this to run each neighbour once even if GetNext returns the same one more than once in the loop
};
