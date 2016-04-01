//
// G1Neighbour.h
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

// A CG1Neighbour object represents a remote computer running Gnutella software with which we are exchanging Gnutella packets
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CG1Neighbour
// http://getenvy.com/shareazawiki/Developers.Code.CG1Neighbour.html

#pragma once

#include "Neighbour.h"

class CG1Packet;
class CG1PacketBuffer;
class CPongItem;
class CGGEPItem;

// A CG1Neighbour object represents a remote computer running Gnutella software with which we are exchanging Gnutella packets
class CG1Neighbour : public CNeighbour		// Inherit from CNeighbour and from that CConnection to get compression features and the connection socket
{
public:
	// Make a new CG1Neighbour object, and delete this one
	CG1Neighbour(CNeighbour* pBase);		// Takes a pointer to a CShakeNeighbour object that determined the remote computer is running Gnutella
	virtual ~CG1Neighbour();

protected:
	// The tick count when something last happened
	DWORD m_tLastPingIn;		// When the remote computer last sent us a ping packet
	DWORD m_tLastPingOut;		// When we last sent a ping packet to the remote computer
	DWORD m_tClusterHost;		// When we last called SendClusterAdvisor (do)
	DWORD m_tClusterSent;		// When that method last sent a vendor specific cluster advisor packet

	// (do)
	BYTE m_nPongNeeded[PONG_NEEDED_BUFFER];	// This is just an array of 32 bytes

	// Information about the most recent ping packet the remote computer has sent us
	Hashes::Guid m_pLastPingID;	// The GUID of the most recent ping packet the remote computer has sent us
	BYTE  m_nLastPingHops;		// The number of hops that packet has traveled, adding 1 (do)

	// A hops flow byte specific to BearShare (do)
	BYTE  m_nHopsFlow;

	// Holds the packets we are going to send to the remote computer
	CG1PacketBuffer* m_pOutbound;

public:
	// Send a packet to the remote computer
	virtual BOOL Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);

	// Query packet
	virtual BOOL SendQuery(const CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);

	// Push packet
	void SendG2Push(const Hashes::Guid& oGUID, CPacket* pPacket);

	// Ping and Pong packets
	BOOL SendPing(const Hashes::Guid& oGUID = Hashes::Guid());
	void OnNewPong(const CPongItem* pPong);

	virtual BOOL ProcessPackets(CBuffer* pInput);	// Process packets from specified buffer

protected:
	virtual BOOL ProcessPackets();					// Process packets from internal input buffer

	// Send and receive packets
	virtual BOOL OnRead();		// Read in data from the socket, decompress it, and call ProcessPackets
	virtual BOOL OnWrite();		// Sends all the packets from the outbound packet buffer to the remote computer
	virtual BOOL OnRun();		// Makes sure the remote computer hasn't been silent too long, and sends a ping every so often

protected:
	// Read and respond to packets from the remote computer
	BOOL OnPacket(CG1Packet* pPacket);	// Sorts the packet and calls one of the methods below

	// Ping and pong packets
	BOOL OnPing(CG1Packet* pPacket);
	BOOL OnPong(CG1Packet* pPacket);

	// Bye packet
	BOOL OnBye(CG1Packet* pPacket);

	// Vendor specific packet
	BOOL OnVendor(CG1Packet* pPacket);

	// Push packet
	BOOL OnPush(CG1Packet* pPacket);

	// Query and query hit packets
	BOOL OnQuery(CG1Packet* pPacket);
	BOOL OnHit(CG1Packet* pPacket);

	// Cluster advisor vendor specific packet
	void SendClusterAdvisor();
	BOOL OnClusterAdvisor(CG1Packet* pPacket);
};
