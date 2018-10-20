//
// BTPacket.h
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

#pragma once

#include "Packet.h"

class CBENode;

// http://wiki.theory.org/BitTorrentSpecification

#define BT_PROTOCOL_HEADER			"\023BitTorrent protocol"
#define BT_PROTOCOL_HEADER_LEN		20

#define BT_DEFAULT_MULTICAST_ADDRESS "239.192.152.143"
#define BT_DEFAULT_MULTICAST_PORT	6771

// Protocol flags					   7 6 5 4 3 2 1 0
#define BT_FLAG_EXTENSION			0x0000100000000000ui64
#define BT_FLAG_DHT_PORT			0x0100000000000000ui64
#define BT_FLAG_FAST_PEERS			0x0400000000000000ui64
#define BT_FLAG_NAT_TRAVERSAL		0x0800000000000000ui64

//
// Packet Types
//

#define BT_PACKET_CHOKE				0
#define BT_PACKET_UNCHOKE			1
#define BT_PACKET_INTERESTED		2
#define BT_PACKET_NOT_INTERESTED	3
#define BT_PACKET_HAVE				4
#define BT_PACKET_BITFIELD			5
#define BT_PACKET_REQUEST			6
#define BT_PACKET_PIECE				7
#define BT_PACKET_CANCEL			8
#define BT_PACKET_DHT_PORT			9
#define BT_PACKET_EXTENSION 		20		// Extension Protocol  http://www.bittorrent.org/beps/bep_0010.html

#define BT_PACKET_HANDSHAKE			128		// Envy/Shareaza Extension Protocol
#define BT_PACKET_SOURCE_REQUEST	129
#define BT_PACKET_SOURCE_RESPONSE	130

#define BT_PACKET_KEEPALIVE			255

// Packet extensions (for BT_PACKET_EXTENSION)
#define BT_EXTENSION_HANDSHAKE		0
#define BT_EXTENSION_UT_METADATA	1		// Extension for Peers to Send Metadata Files (ex. Size)  http://www.bittorrent.org/beps/bep_0009.html
#define BT_EXTENSION_UT_PEX 		2		// Peer exchange
#define BT_EXTENSION_LT_TEX 		3		// Tracker exchange  http://www.bittorrent.org/beps/bep_0028.html

#define BT_EXTENSION_NOP			255		// Packet without standard header (bencoded data only)

// Packet metadata type (for EXTENDED_PACKET_UT_METADATA)
#define UT_METADATA_REQUEST 		0
#define UT_METADATA_DATA			1
#define UT_METADATA_REJECT			2

// Packet extensions (for BT_PACKET_HANDSHAKE)
#define BT_HANDSHAKE_SOURCE			2		// Source Exchange extension

const LPCSTR BT_DICT_ADDED			= "added";
const LPCSTR BT_DICT_ADDED_F		= "added.f";
const LPCSTR BT_DICT_ANNOUNCE_PEER	= "announce_peer";
const LPCSTR BT_DICT_COMPLETE		= "complete";
const LPCSTR BT_DICT_DATA			= "a";
const LPCSTR BT_DICT_DOWNLOADED		= "downloaded";
const LPCSTR BT_DICT_DROPPED		= "dropped";
const LPCSTR BT_DICT_ERROR			= "e";
const LPCSTR BT_DICT_ERROR_LONG		= "error";
const LPCSTR BT_DICT_EXT_MSG		= "m";					// Dictionary of supported extension messages
const LPCSTR BT_DICT_FAILURE		= "failure reason";
const LPCSTR BT_DICT_FILES			= "files";
const LPCSTR BT_DICT_FIND_NODE		= "find_node";
const LPCSTR BT_DICT_GET_PEERS		= "get_peers";
const LPCSTR BT_DICT_ID				= "id";
const LPCSTR BT_DICT_INCOMPLETE		= "incomplete";
const LPCSTR BT_DICT_INTERVAL		= "interval";
const LPCSTR BT_DICT_LT_TEX			= "lt_tex";
const LPCSTR BT_DICT_METADATA_SIZE	= "metadata_size";
const LPCSTR BT_DICT_MSG_TYPE		= "msg_type";
const LPCSTR BT_DICT_NAME			= "name";
const LPCSTR BT_DICT_NICKNAME		= "nickname";
const LPCSTR BT_DICT_NODES			= "nodes";
const LPCSTR BT_DICT_PEERS			= "peers";
const LPCSTR BT_DICT_PEER_ID		= "peer id";
const LPCSTR BT_DICT_PEER_IP		= "ip";
const LPCSTR BT_DICT_PEER_PORT		= "port";
const LPCSTR BT_DICT_PEER_URL		= "url";
const LPCSTR BT_DICT_PIECE			= "piece";
const LPCSTR BT_DICT_PING			= "ping";
const LPCSTR BT_DICT_PORT			= "p";					// Local TCP listen port
const LPCSTR BT_DICT_QUERY			= "q";
const LPCSTR BT_DICT_RESPONSE		= "r";
const LPCSTR BT_DICT_SRC_EXCHANGE	= "source-exchange";
const LPCSTR BT_DICT_TOKEN			= "token";
const LPCSTR BT_DICT_TOTAL_SIZE		= "total_size";
const LPCSTR BT_DICT_TRANSACT_ID	= "t";
const LPCSTR BT_DICT_TRACKERS		= "tr";					// Tracker List hash
const LPCSTR BT_DICT_TYPE			= "y";
const LPCSTR BT_DICT_YOURIP			= "yourip";				// External IP (IPv4 or IPv6?)
const LPCSTR BT_DICT_UPLOAD_ONLY	= "upload_only";		// Partial Seed
const LPCSTR BT_DICT_USER_AGENT		= "user-agent";
const LPCSTR BT_DICT_UT_METADATA	= "ut_metadata";
const LPCSTR BT_DICT_UT_PEX			= "ut_pex";
const LPCSTR BT_DICT_VALUES			= "values";
const LPCSTR BT_DICT_VENDOR			= "v";					// Client name and version (as utf-8 string)


//
// Packet
//

class CBTPacket : public CPacket
{
protected:
	CBTPacket();
	virtual ~CBTPacket();

public:
	BYTE				m_nType;
	BYTE				m_nExtension;		// Extension type if packet type is a BT_PACKET_EXTENSION
	augment::auto_ptr< CBENode > m_pNode;	// Extension decoded data

public:
	virtual void		Reset();
	virtual void		ToBuffer(CBuffer* pBuffer, bool bTCP = true);
	static	CBTPacket*	ReadBuffer(CBuffer* pBuffer);
	virtual void		SmartDump(const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique = 0);
	virtual CString		GetType() const;
	virtual CString		ToHex()   const;
	virtual CString		ToASCII() const;

	inline static bool IsEncoded(BYTE nType)
	{
		return
			nType == BT_PACKET_EXTENSION ||
			nType == BT_PACKET_HANDSHAKE ||
			nType == BT_PACKET_SOURCE_REQUEST ||
			nType == BT_PACKET_SOURCE_RESPONSE;
	}

	bool HasEncodedData() const;

// Packet Pool
protected:
	class CBTPacketPool : public CPacketPool
	{
	public:
		virtual ~CBTPacketPool() { Clear(); }
	protected:
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch);
		virtual void FreePoolImpl(CPacket* pPool);
	};

	static CBTPacketPool POOL;

// Allocation
public:
	static CBTPacket* New(BYTE nType = BT_PACKET_EXTENSION, BYTE nExtension = BT_EXTENSION_NOP, const BYTE* pBuffer = NULL, DWORD nLength = 0);

	inline virtual void Delete()
	{
		POOL.Delete( this );
	}

	// Packet handler
	virtual BOOL OnPacket(const SOCKADDR_IN* pHost);

//	BOOL OnPing(const SOCKADDR_IN* pHost);
//	BOOL OnError(const SOCKADDR_IN* pHost);

	friend class CBTPacket::CBTPacketPool;

private:
	CBTPacket(const CBTPacket&);
	CBTPacket& operator=(const CBTPacket&);
};

inline void CBTPacket::CBTPacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof( CBTPacket );
	pPool	= new CBTPacket[ nSize ];
}

inline void CBTPacket::CBTPacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CBTPacket*)pPacket;
}

#pragma pack(push, 1)

typedef struct
{
	DWORD	nLength;
	BYTE	nType;
	DWORD	nPiece;
	DWORD	nOffset;
} BT_PIECE_HEADER;

#pragma pack(pop)


//
// BitTorrentDHT
//

class CDHT
{
public:
	CDHT();

public:
	void Connect();			// Initialize DHT library and load initial hosts
	void Disconnect();		// Save hosts from DHT library to host cache and shutdown
	void Search(const Hashes::BtHash& oBTH, bool bAnnounce = true);	// Search for hash (and announce if needed)
	bool Ping(const IN_ADDR* pAddress, WORD nPort);					// Ping this host
	void OnPacket(const SOCKADDR_IN* pHost, CBTPacket* pPacket);	// Packet processor
	void OnRun();			// Run this periodically

protected:
	bool m_bConnected;

	static void OnEvent(void* closure, int evt, const unsigned char* info_hash, const void* data, size_t data_len);
};

extern CDHT DHT;
