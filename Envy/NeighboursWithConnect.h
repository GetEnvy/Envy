//
// NeighboursWithConnect.h
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

// Determine our hub or leaf role, count connections for each, and make new ones or close them to have the right number
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CNeighboursWithConnect
// http://getenvy.com/shareazawiki/Developers.Code.CNeighboursWithConnect.html

#pragma once

#include "NeighboursWithRouting.h"

class CConnection;

// Determine our hub or leaf role, count connections for each, and make new ones or close them to have the right number
// Continue the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
class CNeighboursWithConnect : public CNeighboursWithRouting
{
protected:
	CNeighboursWithConnect();
	virtual ~CNeighboursWithConnect();

public:
	// All neighbours average incoming/outgoing speed (bytes/second)
	DWORD BandwidthIn() const { return m_nBandwidthIn; }
	DWORD BandwidthOut() const { return m_nBandwidthOut; }

	// Determine our role on the Gnutella2 network
	bool  IsG2Leaf() const;								// Are we acting as a Gnutella2 leaf on at least one connection
	bool  IsG2Hub() const;								// Are we acting as a Gnutella2 hub on at least one connection
	DWORD IsG2HubCapable(BOOL bIgnoreTime = FALSE, BOOL bDebug = FALSE) const;			// True if computer/connection are sufficient to become a Gnutella2 hub 		(bIgnoreTime unused, bDebug pre-run info)

	// Determine our role on the Gnutella network
	bool  IsG1Leaf() const;								// Are we acting as a Gnutella leaf on at least one connection
	bool  IsG1Ultrapeer() const;						// Are we acting as a Gnutella ultrapeer on at least one connection
	DWORD IsG1UltrapeerCapable(BOOL bIgnoreTime = FALSE, BOOL bDebug = FALSE) const;	// True if computer/connection are sufficient to become a Gnutella ultrapeer	(bIgnoreTime unused, bDebug pre-run info)

	// Number of connections we have older than 1.5 seconds and finished handshake
	DWORD GetStableCount() const { return m_nStableCount; }

	// Last time a neighbour connection attempt was made (in ticks)
	DWORD LastConnect() const { return m_tLastConnect; }

	// Determine our needs on the given network, Gnutella or Gnutella2
	bool  NeedMoreHubs(PROTOCOLID nProtocol) const;		// Do we need more hub connections on the given network
	bool  NeedMoreLeafs(PROTOCOLID nProtocol) const;	// Do we need more leaf connections on the given network
	//bool  IsHubLoaded(PROTOCOLID nProtocol) const;	// Do we have more than 75% of the number of hub connections settings says is our limit

	void PeerPrune(PROTOCOLID nProtocol);				// Close hub to hub connections when we get demoted to the leaf role (do)

private:
	DWORD m_nBandwidthIn;				// All neighbours average incoming speed (bytes/second)
	DWORD m_nBandwidthOut;				// All neighbours average outgoing speed (bytes/second)

	// Member variables that tell our current role on the Gnutella and Gnutella2 networks
	BOOL  m_bG2Leaf;					// Are we a leaf to at least one computer on the Gnutella2 network
	BOOL  m_bG2Hub;						// Are we a hub to at least one computer on the Gnutella2 network
	BOOL  m_bG1Leaf;					// Are we a leaf to at least one computer on the Gnutella network
	BOOL  m_bG1Ultrapeer;				// Are we an ultrapeer to at least one computer on the Gnutella network
	DWORD m_nStableCount;				// Number of connections we have older than 1.5 seconds and finished with the handshake
	DWORD m_tHubG2Promotion;			// Time we were promoted to a G2 hub (in seconds)
	DWORD m_tPresent[ PROTOCOL_LAST ];	// Tick count when we last connected to a hub for each network (in seconds)
	DWORD m_tPriority[ PROTOCOL_LAST ];	// Time when we last connected to priority server, to make delay between priority and regular servers (in seconds)
	DWORD m_tLastConnect;				// Last time a neighbour connection attempt was made (in ticks)

public:
	// Connect to a computer at an IP address, and accept a connection from a computer that has connected to us
	CNeighbour* ConnectTo(const IN_ADDR& pAddress, WORD nPort, PROTOCOLID nProtocol, BOOL bAutomatic = FALSE, BOOL bNoUltraPeer = FALSE);
	BOOL OnAccept(CConnection* pConnection);

	// Methods implimented by several classes in the CNeighbours inheritance column
	virtual void OnRun();				// Call DoRun on each neighbour in the list, and maintain the network auto connection
	virtual void Close();

private:
	// Make new connections and close existing ones
	void MaintainNodeStatus();					// Determine our node status
	void Maintain();							// Count how many connections we have, and initiate or close them to match the ideal numbers in settings
	DWORD CalculateSystemPerformanceScore(BOOL bDebug) const;	// Hub promotion "Points"
};
