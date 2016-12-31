//
// BTPacket.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2007
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "BTPacket.h"

#include "BENode.h"
#include "BTClient.h"
#include "Buffer.h"
#include "Download.h"
#include "Downloads.h"
#include "Datagrams.h"
#include "HostCache.h"
#include "Network.h"
#include "GProfile.h"
#include "Security.h"
#include "Transfers.h"
#include "Statistics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CDHT DHT;	// Services/MainlineDHT

extern "C"
{
	// BitTorrentDHT
	// Source:	https://github.com/jech/dht
	// Info:	http://www.pps.univ-paris-diderot.fr/~jch/software/bittorrent/

	#pragma warning(push,2)
	#ifndef _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS
	#endif
	#include "BitTorrentDHT/dht.h"
	#include "BitTorrentDHT/dht.c"
	#pragma warning(pop)

	// Callback functions:

	int dht_blacklisted(const struct sockaddr *sa, int salen)
	{
		if ( salen != sizeof( SOCKADDR_IN ) )
			return 1;	// IPv6 not supported

		const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)sa;

		return ( pHost->sin_port == 0 ||
			Network.IsFirewalledAddress( &pHost->sin_addr, Settings.Connection.IgnoreOwnIP ) ||
			Network.IsReserved( &pHost->sin_addr ) ||
			Security.IsDenied( &pHost->sin_addr ) ) ? 1 : 0;
	}

	void dht_hash(void *hash_return, int hash_size, const void *v1, int len1, const void *v2, int len2, const void *v3, int len3)
	{
		CMD5 md5;
		md5.Add( v1, len1 );
		md5.Add( v2, len2 );
		md5.Add( v3, len3 );
		md5.Finish();
		CMD5::Digest pDataMD5;
		md5.GetHash( (unsigned char*)&pDataMD5[ 0 ] );
		if ( hash_size > 16 )
			memset( (char*)hash_return + 16, 0, hash_size - 16 );
		memcpy( hash_return, &pDataMD5[ 0 ], ( hash_size > 16 ) ? 16 : hash_size );
	}

	int dht_random_bytes(void *buf, size_t size)
	{
		return CryptGenRandom( theApp.m_hCryptProv, (DWORD)size, (BYTE*)buf ) ? 0 : -1;
	}

	int dht_sendto(int /*s*/, const void *buf, int len, int /*flags*/, const struct sockaddr *to, int tolen)
	{
		if ( tolen != sizeof( SOCKADDR_IN ) )
			return -1;	// IPv6 not supported

		const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)to;

		// ToDo: Remove extra copy
		CBTPacket* pPacket = CBTPacket::New( BT_PACKET_EXTENSION, BT_EXTENSION_NOP, (const BYTE*)buf, len );

		return Datagrams.Send( pHost, pPacket ) ? len : -1;
	}

// Obsolete for reference & deletion
//	void dht_new_node(const unsigned char *id, const struct sockaddr *sa, int salen, int confirm)
//	{
//		if ( salen != sizeof( SOCKADDR_IN ) )
//			return;	// IPv6 not supported
//
//		const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)sa;
//
//		CQuickLock oLock( HostCache.BitTorrent.m_pSection );
//
//		if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( &pHost->sin_addr, htons( pHost->sin_port ) ) )
//		{
//			pCache->m_bDHT = TRUE;
//			if ( confirm == 2 )
//				pCache->m_bCheckedLocally = TRUE;
//			pCache->m_tFailure = 0;
//			pCache->m_nFailures = 0;
//			CopyMemory( &pCache->m_oBtGUID[ 0 ], id, Hashes::BtGuid::byteCount );
//			pCache->m_oBtGUID.validate();
//			HostCache.BitTorrent.m_nCookie++;
//		}
//	}
}

CDHT::CDHT()
	: m_bConnected( false )
{
}

// Initialize DHT library and load initial hosts
void CDHT::Connect()
{
	ASSUME_LOCK( Network.m_pSection );

	if ( m_bConnected || ! Settings.BitTorrent.Enabled || ! Settings.BitTorrent.EnableDHT )
		return;

	Hashes::BtGuid oID = MyProfile.oGUIDBT;
	if ( dht_init( 0, -1, &oID[ 0 ], theApp.m_pBTVersion ) >= 0 )
	{
		CQuickLock oLock( HostCache.BitTorrent.m_pSection );

		int nCount = 0;
		for ( CHostCacheIterator i = HostCache.BitTorrent.Begin() ; i != HostCache.BitTorrent.End() && nCount < 100 ; ++i )
		{
			CHostCacheHostPtr pCache = (*i);
			if ( pCache->m_oBtGUID )
			{
				SOCKADDR_IN sa = { AF_INET, htons( pCache->m_nPort ), pCache->m_pAddress };
				dht_insert_node( &pCache->m_oBtGUID[ 0 ], (sockaddr*)&sa, sizeof( SOCKADDR_IN ) );
				nCount++;
			}
		}

		if ( nCount == 0 )
		{
			SOCKADDR_IN sa;
			if ( Network.Resolve( L"router.bittorrent.com:6881", 0, &sa ) )
			{
				unsigned char tid[4];
				make_tid( tid, "fn", 0 );
				send_find_node( (sockaddr*)&sa, sizeof(SOCKADDR_IN), tid, 4, &oID[0], 1, 0 );
			}
		}

		m_bConnected = true;
	}
}

// Save hosts from DHT library to host cache and shutdown
void CDHT::Disconnect()
{
	ASSUME_LOCK( Network.m_pSection );

	if ( ! m_bConnected )
		return;

	int nCount = 100, nNoIP6 = 0;
	CAutoVectorPtr< SOCKADDR_IN > pHosts( new SOCKADDR_IN[ nCount ] );
	CAutoVectorPtr< unsigned char > pIDs( new unsigned char[ nCount * Hashes::BtGuid::byteCount ] );
	if ( dht_get_nodes( pHosts, pIDs, &nCount, NULL, NULL, &nNoIP6 ) >= 0 )
	{
		CQuickLock oLock( HostCache.BitTorrent.m_pSection );

		for ( int i = 0 ; i < nCount ; ++i )
		{
			if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( &pHosts[ i ].sin_addr, pHosts[ i].sin_port ) )
			{
			//	pCache->m_bDHT = TRUE;	// Unused
				CopyMemory( &pCache->m_oBtGUID[ 0 ], &pIDs[ i * Hashes::BtGuid::byteCount ], Hashes::BtGuid::byteCount );
				pCache->m_oBtGUID.validate();
			}
		}
	}

	dht_uninit();

	m_bConnected = false;
}

// Search for hash
void CDHT::Search(const Hashes::BtHash& oBTH, bool bAnnounce)
{
	if ( ! m_bConnected || ! Settings.BitTorrent.Enabled || ! Settings.BitTorrent.EnableDHT )
		return;

	CSingleLock oLock( &Network.m_pSection, FALSE );
	if ( oLock.Lock( 250 ) )
		dht_search( &oBTH[ 0 ], bAnnounce ? Network.GetPort() : 0, AF_INET, &CDHT::OnEvent, NULL );
}

// Ping this host
bool CDHT::Ping(const IN_ADDR* pAddress, WORD nPort)
{
	if ( ! m_bConnected || ! Settings.BitTorrent.Enabled || ! Settings.BitTorrent.EnableDHT )
		return false;

	CSingleLock oNetworkLock( &Network.m_pSection, FALSE );
	if ( ! oNetworkLock.Lock( 250 ) )
		return false;

	CSingleLock oLock( &HostCache.BitTorrent.m_pSection, FALSE );
	if ( oLock.Lock( 250 ) )
	{
		if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( pAddress, nPort ) )
		{
			pCache->m_tConnect = static_cast< DWORD >( time( NULL ) );
			if ( pCache->m_tAck == 0 )
				pCache->m_tAck = pCache->m_tConnect;

			HostCache.BitTorrent.m_nCookie++;
		}
		oLock.Unlock();
	}

	SOCKADDR_IN sa = { AF_INET, htons( nPort ), *pAddress };
	return dht_ping_node( (sockaddr*)&sa, sizeof( sa ) ) != -1;
}

// Run this periodically
void CDHT::OnRun()
{
	ASSUME_LOCK( Network.m_pSection );

	if ( ! Settings.BitTorrent.Enabled || ! Settings.BitTorrent.EnableDHT )
	{
		if ( m_bConnected )
			Disconnect();
		return;
	}
	else
	{
		if ( ! m_bConnected )
			Connect();
	}

	int nNodes = dht_nodes( AF_INET, NULL, NULL, NULL, NULL );
	if ( nNodes == 0 )
	{
		// Need a node
		CSingleLock oLock( &HostCache.BitTorrent.m_pSection, FALSE );
		if ( oLock.Lock( 250 ) )
		{
			const DWORD tNow = static_cast< DWORD >( time( NULL ) );

			// Ping most recent node
			for ( CHostCacheIterator i = HostCache.BitTorrent.Begin() ; i != HostCache.BitTorrent.End() ; ++i )
			{
				CHostCacheHostPtr pCache = (*i);

				if ( pCache->CanConnect( tNow ) )
				{
					pCache->ConnectTo( TRUE );
					break;
				}
			}
		}
	}

	time_t tosleep = 0;
	dht_periodic( NULL, 0, NULL, 0, &tosleep, &CDHT::OnEvent, NULL );
}

// Packet processor
void CDHT::OnPacket(const SOCKADDR_IN* pHost, CBTPacket* pPacket)
{
	ASSUME_LOCK( Network.m_pSection );

	if ( ! m_bConnected || ! Settings.BitTorrent.Enabled || ! Settings.BitTorrent.EnableDHT )
		return;

	// ToDo: Remove extra copy
	CBuffer pBufffer;
	pPacket->ToBuffer( &pBufffer, false );
	pBufffer.Add( "", 1 );	// Zero terminated

	time_t tosleep = 0;
	dht_periodic( pBufffer.m_pBuffer, pBufffer.m_nLength - 1, (sockaddr*)pHost, sizeof( SOCKADDR_IN ), &tosleep, &CDHT::OnEvent, NULL );
}

void CDHT::OnEvent(void* /*closure*/, int evt, const unsigned char* info_hash, const void* data, size_t data_len)
{
	switch ( evt )
	{
	case DHT_EVENT_ADDED:
	case DHT_EVENT_SENT:
		if ( data_len == sizeof( SOCKADDR_IN ) )
		{
			const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)data;

			CQuickLock oLock( HostCache.BitTorrent.m_pSection );

			// Node just added
			if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( &pHost->sin_addr, htons( pHost->sin_port ) ) )
			{
				CopyMemory( &pCache->m_oBtGUID[ 0 ], info_hash, Hashes::BtGuid::byteCount );
				pCache->m_oBtGUID.validate();

				HostCache.BitTorrent.m_nCookie++;
			}
		}
		break;

	case DHT_EVENT_REPLY:
		if ( data_len == sizeof( SOCKADDR_IN ) )
		{
			// Assume UDP is stable
			Datagrams.SetStable();

			const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)data;

			CQuickLock oLock( HostCache.BitTorrent.m_pSection );

			// Got reply from node
			if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.OnSuccess( &pHost->sin_addr, htons( pHost->sin_port ) ) )
			{
				pCache->m_tAck = 0;
				HostCache.BitTorrent.m_nCookie++;
			}
		}
		break;

	case DHT_EVENT_REMOVED:
		if ( data_len == sizeof( SOCKADDR_IN ) )
		{
			const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)data;

			CQuickLock oLock( HostCache.BitTorrent.m_pSection );

			HostCache.BitTorrent.Remove( &pHost->sin_addr );
			HostCache.BitTorrent.m_nCookie++;
		}
		break;

	case DHT_EVENT_VALUES:
		{
			CSingleLock oLock( &Transfers.m_pSection, FALSE );
			if ( oLock.Lock( 250 ) )
			{
				Hashes::BtHash oHash;
				CopyMemory( &oHash[ 0 ], info_hash, Hashes::BtHash::byteCount );
				oHash.validate();

				if ( CDownload* pDownload = Downloads.FindByBTH( oHash ) )
				{
				//	ATLTRACE( "DHT> %s %s\n", (LPCSTR)CT2CA( pDownload->m_oBTH.toString() ), (LPCSTR)CT2CA( pDownload->m_sName ) );

					size_t nCount = data_len / 6;
					//int nMax = Settings.Downloads.SourcesWanted;
					for ( size_t i = 0 ; i < nCount ; ++i )
					{
						const char* p = &((const char*)data)[ i * 6 ];
						pDownload->AddSourceBT( Hashes::BtGuid(), (IN_ADDR*)p, ntohs( *(WORD*)(p + 4) ) );
					//	if ( nMax-- < 0 && pDownload->GetEffectiveSourceCount() > Settings.Downloads.SourcesWanted )
					//		break;
					}
				}
			}
		}
		break;

	//case DHT_EVENT_VALUES6:
	//case DHT_EVENT_SEARCH_DONE:
	//case DHT_EVENT_SEARCH_DONE6:
	//default:
	//	break;
	}
}


CBTPacket::CBTPacketPool CBTPacket::POOL;


//////////////////////////////////////////////////////////////////////
// CBTPacket construction

CBTPacket::CBTPacket()
	: CPacket( PROTOCOL_BT )
	, m_nType		( BT_PACKET_EXTENSION )
	, m_nExtension	( BT_EXTENSION_NOP )
	, m_pNode		( new CBENode() )
{
}

CBTPacket::~CBTPacket()
{
}

void CBTPacket::Reset()
{
	CPacket::Reset();

	m_nType			= BT_PACKET_EXTENSION;
	m_nExtension	= BT_EXTENSION_NOP;
	m_pNode.reset	( new CBENode() );
}

CBTPacket* CBTPacket::New(BYTE nType, BYTE nExtension, const BYTE* pBuffer, DWORD nLength)
{
	CBTPacket* pPacket = (CBTPacket*)POOL.New();
	ASSERT( pPacket && pPacket->m_pNode.get() );
	if ( pPacket )
	{
		pPacket->m_nType = nType;
		pPacket->m_nExtension = nExtension;
		if ( pBuffer && nLength )
		{
			DWORD nRead = 0;
			if ( IsEncoded( nType ) )
			{
				pPacket->m_pNode.reset( CBENode::Decode( pBuffer, nLength, &nRead ) );
				if ( ! pPacket->m_pNode.get() )
				{
					pPacket->Release();
					return NULL;
				}
			}
			// Read rest of packet (if any) as raw data
			if ( nLength > nRead )
			{
				if ( ! pPacket->Write( pBuffer + nRead, nLength - nRead ) )
				{
					pPacket->Release();
					return NULL;
				}
			}
		}
	}
	return pPacket;
}

bool CBTPacket::HasEncodedData() const
{
	return m_pNode.get() && ! m_pNode->IsType( CBENode::beNull );
}

//////////////////////////////////////////////////////////////////////
// CBTPacket serialize

void CBTPacket::ToBuffer(CBuffer* pBuffer, bool /*bTCP*/)
{
	if ( m_nType == BT_PACKET_KEEPALIVE )
	{
		// Keep-Alive packet
		DWORD nZero = 0;
		pBuffer->Add( &nZero, 4 );
	}
	else if ( m_nType == BT_PACKET_EXTENSION && m_nExtension == BT_EXTENSION_NOP )
	{
		// Packet without header
		if ( HasEncodedData() )
			m_pNode->Encode( pBuffer );

		pBuffer->Add( m_pBuffer, m_nLength );
	}
	else	// Full packet
	{
		// Reserve memory for packet length field
		DWORD nZero = 0;
		pBuffer->Add( &nZero, 4 );
		DWORD nOldLength = pBuffer->m_nLength;

		// Add packet type
		pBuffer->Add( &m_nType, 1 );

		// Add packet extension
		if ( m_nType == BT_PACKET_EXTENSION )
			pBuffer->Add( &m_nExtension, 1 );

		// Add bencoded data
		if ( HasEncodedData() )
			m_pNode->Encode( pBuffer );

		// Add payload
		pBuffer->Add( m_pBuffer, m_nLength );

		// Set packet total length
		*(DWORD*)( pBuffer->m_pBuffer + nOldLength - 4 ) =
			swapEndianess( pBuffer->m_nLength - nOldLength );
	}

	ASSERT( pBuffer->m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CBTPacket unserialize

CBTPacket* CBTPacket::ReadBuffer(CBuffer* pBuffer)
{
	DWORD nLength = (DWORD) - 1;
	bool bKeepAlive = false;
	bool bValid = true;

	// Skip subsequent keep-alive packets
	do
	{
		if ( pBuffer->m_nLength < sizeof( DWORD ) )
			bValid = false;
		else
		{
			nLength = transformFromBE( pBuffer->ReadDWORD() );
			if ( pBuffer->m_nLength - sizeof( DWORD ) < nLength )
				bValid = false;
		}

		if ( ! bKeepAlive && nLength == 0 )
			bKeepAlive = true;

		if ( bValid )
			pBuffer->Remove( sizeof( DWORD ) );		// Remove size marker
	}
	while ( bKeepAlive && bValid && nLength == 0 );

	CBTPacket* pPacket = NULL;
	if ( bKeepAlive )
	{
		pPacket = CBTPacket::New( BT_PACKET_KEEPALIVE );
	}
	else if ( bValid )
	{
		if ( pBuffer->m_pBuffer[ 0 ] == BT_PACKET_EXTENSION )
		{
			// Read extension packet
			pPacket = CBTPacket::New( BT_PACKET_EXTENSION,
				pBuffer->m_pBuffer[ 1 ], pBuffer->m_pBuffer + 2, nLength - 2 );
		}
		else
		{
			// Read standard packet
			pPacket = CBTPacket::New( pBuffer->m_pBuffer[ 0 ],
				BT_EXTENSION_NOP, pBuffer->m_pBuffer + 1, nLength - 1 );
		}

		pBuffer->Remove( nLength );
	}

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CBTPacket debugging
void CBTPacket::SmartDump(const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique)
{
	switch ( m_nType )
	{
	case BT_PACKET_REQUEST:
	case BT_PACKET_PIECE:
	case BT_PACKET_HAVE:
		return; 	// Ignore uninterested packets
	}

	CPacket::SmartDump( pAddress, bUDP, bOutgoing, nNeighbourUnique );
}

CString CBTPacket::GetType() const
{
	switch ( m_nType )
	{
	case BT_PACKET_CHOKE:
		return CString( L"Choke" );
	case BT_PACKET_UNCHOKE:
		return CString( L"Unchoke" );
	case BT_PACKET_INTERESTED:
		return CString( L"Interested" );
	case BT_PACKET_NOT_INTERESTED:
		return CString( L"NotInterested" );
	case BT_PACKET_HAVE:
		return CString( L"Have" );
	case BT_PACKET_BITFIELD:
		return CString( L"Bitfield" );
	case BT_PACKET_REQUEST:
		return CString( L"Request" );
	case BT_PACKET_PIECE:
		return CString( L"Piece" );
	case BT_PACKET_CANCEL:
		return CString( L"Cancel" );
	case BT_PACKET_DHT_PORT:
		return CString( L"DHT port" );
	case BT_PACKET_HANDSHAKE:
		return CString( L"ExtHandshake" );
	case BT_PACKET_SOURCE_REQUEST:
		return CString( L"SrcRequest" );
	case BT_PACKET_SOURCE_RESPONSE:
		return CString( L"SrcResponse" );
	case BT_PACKET_KEEPALIVE:
		return CString( L"Keep-Alive" );
	case BT_PACKET_EXTENSION:
		switch ( m_nExtension )
		{
		case BT_EXTENSION_HANDSHAKE:
			return CString( L"Handshake" );
		case BT_EXTENSION_NOP:
			return CString( L"DHT" );
	//	case BT_EXTENSION_UT_METADATA:
	//		return CString( L"UT Metadata" );
	//	case BT_EXTENSION_UT_PEX:
	//		return CString( L"UT PEX" );
	//	case BT_EXTENSION_LT_TEX:
	//		return CString( L"LT TEX" );
		}
		{
			CString strType;
			strType.Format( L"Extension %d", m_nExtension );
			return strType;
		}
	}

	CString strType;
	strType.Format( L"%d", m_nType );
	return strType;
}

CString CBTPacket::ToHex() const
{
	return CPacket::ToHex();
}

CString CBTPacket::ToASCII() const
{
	CString strText;

	switch ( m_nType )
	{
	case BT_PACKET_DHT_PORT:
		strText.Format( L"port: %u", ntohs( *(WORD*)m_pBuffer ) );
		break;
	case BT_PACKET_BITFIELD:
		strText.Format( L"length: %u bytes", m_nLength );
		break;
	default:
		strText = HasEncodedData() ? m_pNode->Encode() : CPacket::ToASCII();
	}

	return strText;
}

BOOL CBTPacket::OnPacket(const SOCKADDR_IN* pHost)
{
	ASSERT( m_nType == BT_PACKET_EXTENSION );
	ASSERT( m_nExtension == BT_EXTENSION_NOP );

	Statistics.Current.BitTorrent.Incoming++;

	SmartDump( pHost, TRUE, FALSE );

	if ( ! m_pNode->IsType( CBENode::beDict ) )
		return FALSE;

	if ( ! Settings.BitTorrent.Enabled || ! Settings.BitTorrent.EnableDHT )
		return TRUE;

	{
		CQuickLock oLock( HostCache.BitTorrent.m_pSection );

		if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.OnSuccess( &pHost->sin_addr, htons( pHost->sin_port ) ) )
		{
			// Get version
			const CBENode* pVersion = m_pNode->GetNode( BT_DICT_VENDOR );	// "v"
			if ( pVersion && pVersion->IsType( CBENode::beString ) )
				pCache->m_sName = CBTClient::GetUserAgentAzureusStyle( (LPBYTE)pVersion->m_pValue, 4 );

			HostCache.BitTorrent.m_nCookie++;
		}
	}

	CBENode* pYourIP = m_pNode->GetNode( BT_DICT_YOURIP );					// "yourip"
	if ( pYourIP && pYourIP->IsType( CBENode::beString ) )
	{
		if ( pYourIP->m_nValue == 4 )	// IPv4
			Network.AcquireLocalAddress( *(const IN_ADDR*)pYourIP->m_pValue );
	}

	DHT.OnPacket( pHost, this );

	return TRUE;

//	// Get packet type and transaction id
//	const CBENode* pType = m_pNode->GetNode( BT_DICT_TYPE );
//	const CBENode* pTransID = m_pNode->GetNode( BT_DICT_TRANSACT_ID );
//	if ( ! pType || ! pType->IsType( CBENode::beString ) ||
//		 ! pTransID || ! pTransID->IsType( CBENode::beString ) )
//		return FALSE;
//
//	CString strType = pType->GetString();
//	if ( strType == BT_DICT_QUERY )
//	{
//		// Query message
//		CBENode* pQueryMethod = m_pNode->GetNode( BT_DICT_QUERY );			// "q"
//		if ( ! pQueryMethod || ! pQueryMethod->IsType( CBENode::beString ) )
//			return FALSE;
//
//		CString strQueryMethod = pQueryMethod->GetString();
//		if ( strQueryMethod == BT_DICT_PING ) 								// "ping"
//			return OnPing( pHost );
//		//else if ( strQueryMethod == BT_DICT_FIND_NODE ) 					// "find_node"
//		//	; // ToDo: Find node
//		//else if ( strQueryMethod == BT_DICT_GET_PEERS ) 					// "get_peers"
//		//	; // ToDo: Get peers
//		//else if ( strQueryMethod == BT_DICT_ANNOUNCE_PEER )				// "announce_peer"
//		//	; // ToDo: Announce peer
//		//else if ( strQueryMethod == BT_DICT_ERROR_LONG )					// "error"
//		//	; // ToDo: ??
//
//		return TRUE;
//	}
//	else if ( strType == BT_DICT_RESPONSE )
//	{
//		// Response message
//		const CBENode* pResponse = m_pNode->GetNode( BT_DICT_RESPONSE );	// "r"
//		if ( ! pResponse || ! pResponse->IsType( CBENode::beDict ) )
//			return FALSE;
//
//		Hashes::BtGuid oNodeGUID;
//		const CBENode* pNodeID = pResponse->GetNode( BT_DICT_ID );			// "id"
//		if ( ! pNodeID || ! pNodeID->GetString( oNodeGUID ) )
//			return FALSE;
//
//		pCache->m_oBtGUID = oNodeGUID;
//		pCache->m_sDescription = oNodeGUID.toString();
//
//		// ToDo: Check queries pool for pTransID
//
//		// Save access token
//		const CBENode* pToken = pResponse->GetNode( BT_DICT_TOKEN );		// "token"
//		if ( pToken && pToken->IsType( CBENode::beString ) )
//		{
//			pCache->m_Token.SetSize( (INT_PTR)pToken->m_nValue );
//			CopyMemory( pCache->m_Token.GetData(), pToken->m_pValue, (size_t)pToken->m_nValue );
//		}
//
//		const CBENode* pPeers = pResponse->GetNode( BT_DICT_VALUES );		// "values"
//		if ( pPeers && pPeers->IsType( CBENode::beList ) )
//			;	// ToDo: Handle "values" ?
//
//		const CBENode* pNodes = pResponse->GetNode( BT_DICT_NODES );		// "nodes"
//		if ( pNodes && pNodes->IsType( CBENode::beString ) )
//			;	// ToDo: Handle "nodes" ?
//
//		return TRUE;
//	}
//	else if ( strType == BT_DICT_ERROR )
//	{
//		// Error message
//		const CBENode* pError = m_pNode->GetNode( BT_DICT_ERROR );			// "e"
//		if ( ! pError || ! pError->IsType( CBENode::beList ) )
//			return FALSE;
//
//		return OnError( pHost );
//	}
//
//	return FALSE;
}

//BOOL CBTPacket::OnPing(const SOCKADDR_IN* pHost)
//{
//	const CBENode* pTransID = m_pNode->GetNode( BT_DICT_TRANSACT_ID );		// "t"
//
//	const CBENode* pQueryData = m_pNode->GetNode( BT_DICT_DATA );			// "a"
//	if ( ! pQueryData || ! pQueryData->IsType( CBENode::beDict ) )
//		return FALSE;
//
//	Hashes::BtGuid oNodeGUID;
//	const CBENode* pNodeID = pQueryData->GetNode( BT_DICT_ID );				// "id"
//	if ( ! pNodeID || ! pNodeID->GetString( oNodeGUID ) )
//		return FALSE;
//
//	{
//		CQuickLock oLock( HostCache.BitTorrent.m_pSection );
//
//		if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( &pHost->sin_addr, htons( pHost->sin_port ) ) )
//		{
//			pCache->m_oBtGUID = oNodeGUID;
//			pCache->m_sDescription = oNodeGUID.toString();
//			HostCache.BitTorrent.m_nCookie++;
//		}
//	}
//
//	// Send pong
//
//	CBTPacket* pPacket = CBTPacket::New();
//	CBENode* pRoot = pPacket->m_pNode.get();
//	ASSERT( pRoot );
//
//	pRoot->Add( BT_DICT_RESPONSE )->Add( BT_DICT_ID )->SetString( MyProfile.oGUIDBT );		// "r" "id"
//	pRoot->Add( BT_DICT_TYPE )->SetString( BT_DICT_RESPONSE );								// "y" "r"
//	pRoot->Add( BT_DICT_TRANSACT_ID )->SetString( (LPCSTR)pTransID->m_pValue, (size_t)pTransID->m_nValue ); 	// "t"
//	pRoot->Add( BT_DICT_VENDOR )->SetString( theApp.m_pBTVersion, 4 );						// "v"
//
//	Datagrams.Send( pHost, pPacket );
//
//	return TRUE;
//}

//BOOL CBTPacket::OnError(const SOCKADDR_IN* /*pHost*/)
//{
//	return TRUE;
//}

//BOOL CBTPacket::Ping(const SOCKADDR_IN* pHost)
//{
//	CBTPacket* pPingPacket = CBTPacket::New();
//	CBENode* pPing = pPingPacket->m_pNode.get();
//	pPing->Add( BT_DICT_DATA )->Add( BT_DICT_ID )->SetString( MyProfile.oGUIDBT );	// "a"
//	pPing->Add( BT_DICT_TYPE )->SetString( BT_DICT_QUERY );				// "q"
//	pPing->Add( BT_DICT_TRANSACT_ID )->SetString( "1234" );				// ToDo:?
//	pPing->Add( BT_DICT_QUERY )->SetString( BT_DICT_PING );				// "ping"
//	pPing->Add( BT_DICT_VENDOR )->SetString( theApp.m_pBTVersion, 4 );
//	return Datagrams.Send( pHost, pPingPacket );
//}

//BOOL CBTPacket::GetPeers(const SOCKADDR_IN* pHost, const Hashes::BtGuid& oNodeGUID, const Hashes::BtHash& oGUID)
//{
//	CBENode pGetPeers;
//	CBENode* pGetPeersData = pGetPeers.Add( BT_DICT_DATA );				// "a"
//	pGetPeersData->Add( BT_DICT_ID )->SetString( oNodeGUID );
//	pGetPeersData->Add( "info_hash" )->SetString( oGUID );
//	pGetPeers.Add( BT_DICT_TYPE )->SetString( BT_DICT_QUERY );			// "q"
//	pGetPeers.Add( BT_DICT_TRANSACT_ID )->SetString( "4567" );			// ToDo:?
//	pGetPeers.Add( BT_DICT_QUERY )->SetString( BT_DICT_GET_PEERS );		// "get_peers"
//	pGetPeers.Add( BT_DICT_VENDOR )->SetString( theApp.m_pBTVersion, 4 );
//	CBuffer pOutput;
//	pGetPeers.Encode( &pOutput );
//	return Datagrams.Send( pHost, pOutput );
//}
