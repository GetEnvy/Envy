//
// Neighbour.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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

// CNeighbour is in the middle of the CConnection inheritance tree, adding compression and a bunch of member variables
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CNeighbour
// http://getenvy.com/archives/shareazawiki/Developers.Code.CNeighbour.html

#pragma once

#include "Connection.h"
#include "zlib.h"

class CBuffer;
class CPacket;
class CVendor;
class CGProfile;
class CQuerySearch;
class CQueryHashTable;

// Keep track of what stage of communications we are in with the remote computer
typedef enum NeighbourStateEnum
{
	// One of these states describes what's happening with our connection to the remote computer right now
	nrsNull,		// No state recorded yet, the CNeighbour constructor sets m_nState to nrsNull
	nrsConnecting,	// We called CConnection::ConnectTo, and are now waiting for the remote computer to do something
	nrsHandshake1,	// We've finished sending a group of headers, and await the response
	nrsHandshake2,	// We're reading the initial header group the remote computer has sent
	nrsHandshake3,	// We're reading the final header group from the remote computer
	nrsRejected,	// The remote computer started with "GNUTELLA/0.6", but did not say "200 OK"
	nrsClosing, 	// We called DelayClose to send buffered data and then close the socket connection
	nrsConnected	// The handshake is over, the CNeighbour copy constructor sets m_nState to nrsConnected
} NrsState;

// Record if the remote computer is in the same network role as us, or in a higher or lower one
typedef enum NeighbourNodeEnum
{
	// The remote computer can be a leaf, or ultrapeer, or hub, and so can we
	ntNode,			// We are both Gnutella ultrapeers or Gnutella2 hubs
	ntHub,			// We are a leaf, and this connection is to a Gnutella ultrapeer or Gnutella2 hub above us
	ntLeaf,			// We are a Gnutella ultrapeer or Gnutella2 hub, and this connection is to a leaf below us
	ntLast			// Workaround count hack
} NrsNode;

// Make the m_nPongNeeded buffer an array of 32 bytes
const uchar PONG_NEEDED_BUFFER = 32;


// Define the CNeighbour class to inherit from CConnection, picking up a socket and methods to connect it and read data through it
class CNeighbour : public CConnection
{
protected:
	CNeighbour(PROTOCOLID nProtocol);
	CNeighbour(PROTOCOLID nProtocol, CNeighbour* pBase);
	virtual ~CNeighbour();

// State
public:
	DWORD		m_nRunCookie;			// The number of times this neighbour has been run, CNeighboursBase::OnRun uses this to run each neighbour in the list once
	NrsState	m_nState;				// Neighbour state, like connecting, handshake 1, 2, or 3, or rejected
	CVendor*	m_pVendor;
	CGProfile*	m_pProfile;
	Hashes::Guid m_oGUID;
	Hashes::Guid m_oMoreResultsGUID;	// GUID of the last search, used to get more results (do)
	CString		m_sServerName;			// Server name, primarily for eD2K and DC++ hubs

// Capabilities
public:
	NrsNode		m_nNodeType;			// This connection is to a hub above us, ntHub, a leaf below us, ntLeaf, or a hub just like us, ntNode
	BOOL		m_bAutomatic;
	BOOL		m_bQueryRouting;
	BOOL		m_bPongCaching;
	BOOL		m_bVendorMsg;			// True if the remote computer told us it supports vendor-specific messages
	BOOL		m_bGGEP;
	BOOL		m_bBadClient;			// Is the remote client running a 'bad' client- GPL rip, buggy, etc. (not banned, though)
	DWORD		m_tLastQuery;			// The time we last got a query packet, recorded as the number of seconds since 1970

	DWORD		m_nDegree;				// "X-Degree: n" (-1 if not set)
	DWORD		m_nMaxTTL;				// "X-Max-TTL: n" (-1 if not set)
	BOOL		m_bDynamicQuerying;		// "X-Dynamic-Querying: 0.1" (default: false)
	BOOL		m_bUltrapeerQueryRouting;	// "X-Ultrapeer-Query-Routing: 0.1" (default: false)
	BOOL		m_bRequeries;			// "X-Requeries: false" (default: true)
	BOOL		m_bExtProbes;			// "X-Ext-Probes: 0.1" (default: false)
	CString 	m_sLocalePref;			// "X-Locale-Pref: en" ("" if not set) -UNUSED?

// Statistics
public:
	DWORD		m_nInputCount;
	DWORD		m_nOutputCount;
	DWORD		m_nDropCount;
	DWORD		m_nLostCount;
	DWORD		m_nOutbound;

	// If the remote computer sends us a pong packet it made, copy the sharing statistics here
	DWORD		m_nFileCount; 			// The number of files the remote computer is sharing, according to the pong packet it sent us
	DWORD		m_nFileVolume;			// The total size of all of those files, according to the same pong packet

// Query Hash Tables
public:
	CQueryHashTable* m_pQueryTableRemote;
	CQueryHashTable* m_pQueryTableLocal;

// Internals
protected:
	DWORD		m_tLastPacket;			// The time that we received the last packet
	CBuffer*	m_pZInput;				// The remote computer is sending compressed data, we'll save it in m_pInput, and then decompress it to here
	CBuffer*	m_pZOutput;				// We are sending the remote computer compressed data, we're writing it here, and then compressing it to m_pOutput
	DWORD		m_nZInput;				// The number of decompressed bytes of data the remote computer sent us
	DWORD		m_nZOutput;				// The number of not yet compressed bytes of data we've sent the remote computer
	z_streamp	m_pZSInput;				// Pointer to the zlib z_stream structure for decompression
	z_streamp	m_pZSOutput;			// Pointer to the zlib z_stream structure for compression
	BOOL		m_bZFlush;				// True to flush the compressed output buffer to the remote computer
	DWORD		m_tZOutput;				// The time that Zlib last compressed something
	BOOL		m_bZInputEOS;			// Got End Of Stream while decompressing incoming data

public:
	DWORD		GetMaxTTL() const;		// Get maximum TTL which is safe for both sides
	void		GetCompression(float& nInRate, float& nOutRate) const;	// Calculate average compression rate in either direction for this connection

	virtual DWORD GetUserCount() const { return 0; }	// Returns hub/server leaf/user count variable
	virtual DWORD GetUserLimit() const { return 0; }	// Returns hub/server leaf/user limit variable

	virtual void Close(UINT nError = IDS_CONNECTION_CLOSED);
	virtual void DelayClose(UINT nError);	// Send the buffer then close the socket, record the error given
	virtual BOOL Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	virtual BOOL SendQuery(const CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal); 	// Validate query
	virtual BOOL ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic);
	virtual BOOL ProcessPackets(CBuffer* /*pInput*/) { return TRUE; }	// Process packets from input buffer

protected:
	virtual BOOL ProcessPackets() { return TRUE; }		// Process packets from internal input buffer

	virtual BOOL OnRun();
	virtual void OnDropped();
	virtual BOOL OnRead();
	virtual BOOL OnWrite();
	virtual BOOL OnCommonHit(CPacket* pPacket);
	virtual BOOL OnCommonQueryHash(CPacket* pPacket);
};
