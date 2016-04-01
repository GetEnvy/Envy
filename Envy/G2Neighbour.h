//
// G2Neighbour.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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

#include "Neighbour.h"

class CPacket;
class CG2Packet;
class CHubHorizonGroup;
class CQuerySearch;
class CRouteCache;


class CG2Neighbour : public CNeighbour
{
public:
	CG2Neighbour(CNeighbour* pBase);
	virtual ~CG2Neighbour();

public:
	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	virtual BOOL	SendQuery(const CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);

	virtual DWORD   GetUserCount() const { return m_nLeafCount; }
	virtual DWORD   GetUserLimit() const { return m_nLeafLimit; }
	virtual BOOL	ProcessPackets(CBuffer* pInput);

	BOOL			OnPing(CG2Packet* pPacket, BOOL bTCP = TRUE);
	BOOL			OnPong(CG2Packet* pPacket, BOOL bTCP = TRUE);
	BOOL			OnPacket(CG2Packet* pPacket);
	void			SendLNI();
	void			SendKHL();
	void			SendHAW();
	BOOL			OnLNI(CG2Packet* pPacket);
	BOOL			OnKHL(CG2Packet* pPacket);
	BOOL			OnHAW(CG2Packet* pPacket);
	BOOL			OnQuery(CG2Packet* pPacket);
	BOOL			OnQueryAck(CG2Packet* pPacket);
	BOOL			OnQueryKeyReq(CG2Packet* pPacket);
	BOOL			OnQueryKeyAns(CG2Packet* pPacket);
	bool			OnPush(CG2Packet* pPacket);
	BOOL			OnProfileChallenge(CG2Packet* pPacket);
	BOOL			OnProfileDelivery(CG2Packet* pPacket);

	static CG2Packet* CreateLNIPacket(CG2Neighbour* pOwner = NULL);
	static CG2Packet* CreateKHLPacket(CG2Neighbour* pOwner = NULL);
	static BOOL		ParseKHLPacket(CG2Packet* pPacket, const SOCKADDR_IN* pHost);

public:
	DWORD				m_nLeafCount;
	DWORD				m_nLeafLimit;
	BOOL				m_bCachedKeys;
	CRouteCache*		m_pGUIDCache;
	CHubHorizonGroup*	m_pHubGroup;

protected:
	DWORD				m_tLastRun;
	LONG				m_tAdjust;				// Time adjust of neighbour
	DWORD				m_tLastPingIn;			// Time when /PI packet received
	DWORD				m_tLastPingOut;			// Time when /PI packet sent
	DWORD				m_nCountPingIn;			// Number of /PI packets received
	DWORD				m_nCountPingOut;		// Number of /PI packets sent
	DWORD				m_tLastRelayPingIn;		// Time when /PI/UDP packet received
	DWORD				m_tLastRelayPingOut;	// Time when /PI/UDP packet sent
	DWORD				m_nCountRelayPingIn;	// Number of /PI/UDP packets received
	DWORD				m_nCountRelayPingOut;	// Number of /PI/UDP packets sent
	DWORD				m_tLastRelayedPingIn;	// Time when /PI/RELAY/UDP packet received
	DWORD				m_tLastRelayedPingOut;	// Time when /PI/RELAY/UDP packet sent
	DWORD				m_nCountRelayedPingIn;	// Number of /PI/RELAY/UDP packets received
	DWORD				m_nCountRelayedPingOut;	// Number of /PI/RELAY/UDP packets sent
	DWORD				m_tLastKHLIn;			// Time when KHL packet received
	DWORD				m_tLastKHLOut;			// Time when KHL packet sent
	DWORD				m_nCountKHLIn;			// Number of KHL packets received
	DWORD				m_nCountKHLOut;			// Number of KHL packets sent
	DWORD				m_tLastLNIIn;			// Time when LNI packet received
	DWORD				m_tLastLNIOut;			// Time when LNI packet sent
	DWORD				m_nCountLNIIn;			// Number of LNI packets received
	DWORD				m_nCountLNIOut;			// Number of LNI packets sent
	DWORD				m_tLastHAWIn;			// Time when HAW packet received
	DWORD				m_tLastHAWOut;			// Time when HAW packet sent
	DWORD				m_nCountHAWIn;			// Number of HAW packets received
	DWORD				m_nCountHAWOut;			// Number of HAW packets sent
	DWORD				m_tLastQueryIn;			// Time when Q2 packet received
	DWORD				m_nCountQueryIn;		// Number of Q2 packets received
	CList< CG2Packet* >	m_pOutbound;			// Queue of outbound packets
	BOOL				m_bFirewalled;			// Is the client reporting they are firewalled from /LNI/FW

protected:
	void			SendStartups();

	virtual BOOL	ProcessPackets();			// Process packets from internal input buffer

	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnRun();
};
