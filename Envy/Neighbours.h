//
// Neighbours.h
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

// Complete the CNeighbours inheritance column, calling Close on each neighbour when the program exits
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CNeighbours
// http://getenvy.com/archives/envywiki/Developers.Code.CNeighbours.html

#pragma once

#include "NeighboursWithConnect.h"

// Complete the CNeighbours inheritance column, calling Close on each neighbour when the program exits
// End the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase

class CNeighbours : public CNeighboursWithConnect
{
public:
	CNeighbours();
	virtual ~CNeighbours();

public:
	CString GetName(const CNeighbour* pNeighbour) const;
	CString GetAgent(const CNeighbour* pNeighbour) const;
	CString GetNick(const CNeighbour* pNeighbour) const;
//	CString GetServerName(const CNeighbour* pNeighbour) const;
	CString GetServerName(const CString& strIP) const;
	CString GetNeighbourList(LPCTSTR szFormat) const;

	// Let CNeighbour and CShakeNeighbour look at private members of CNeighbours class
	friend class CNeighbour;
	friend class CShakeNeighbour;
};

// Access the single global Neighbours object that holds the list of neighbour computers we are connected to
// When Envy starts running, this line creates a single global instance of a CNeighbours object, called Neighbours

extern CNeighbours Neighbours;
