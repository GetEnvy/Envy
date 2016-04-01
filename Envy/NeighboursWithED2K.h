//
// NeighboursWithED2K.h
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

#include "NeighboursWithG2.h"

class CEDNeighbour;
class CDownloadWithTiger;


// Add methods helpful for eDonkey2000 that use the list of connected neighbours
// Continue the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
class CNeighboursWithED2K : public CNeighboursWithG2
{
protected:
	CNeighboursWithED2K();
	virtual ~CNeighboursWithED2K();

public:
	DWORD				m_tLastED2KServerHop;	// The last time the ed2k server was changed due low ID (ticks)
	DWORD				m_nLowIDCount;			// Counts the amount of ed2k server low IDs we got (resets on high ID)

public:
	virtual void OnRun();

	// Get an eDonkey2000 neighbour from the list that's through the handshake and has a client ID
	CEDNeighbour* GetDonkeyServer() const;

	// Do things to all the eDonkey2000 computers we're connected to
	void CloseDonkeys();                           // Disconnect from all the eDonkey2000 computers we're connected to
	void SendDonkeyDownload(const CDownloadWithTiger* pDownload);	// Tell all the connected eDonkey2000 computers about pDownload

	// Send eDonkey2000 packets
	BOOL PushDonkey(DWORD nClientID, const IN_ADDR& pServerAddress, WORD nServerPort); // Send a callback request packet
	BOOL FindDonkeySources(const Hashes::Ed2kHash& oED2K, IN_ADDR* pServerAddress, WORD nServerPort);

// Classes that inherit from this one can get to protected members, but unrelated classes can't
protected:
	void RunGlobalStatsRequests();

	// Hash arrays used by FindDonkeySources
	DWORD				m_tEDSources[256]; 		// 256 MD4 hashes
	Hashes::Ed2kHash	m_oEDSources[256];
};
