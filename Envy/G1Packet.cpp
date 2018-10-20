//
// G1Packet.cpp
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

// CG1Packet represents a Gnutella packet, and CG1PacketPool keeps lists of them
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CG1Packet
// http://getenvy.com/archives/shareazawiki/Developers.Code.CG1Packet.html

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "G1Packet.h"
#include "Buffer.h"
#include "Datagrams.h"
#include "DiscoveryServices.h"
#include "HostCache.h"
#include "LibraryMaps.h"
#include "Network.h"
#include "Security.h"
#include "Statistics.h"
#include "VendorCache.h"

#include "Handshakes.h"
#include "LocalSearch.h"
#include "Neighbour.h"
#include "Neighbours.h"
#include "QueryHit.h"
#include "QuerySearch.h"
#include "SearchManager.h"
#include "SchemaCache.h"
#include "GProfile.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// When the program runs, create a single, global CG1PacketPool called POOL to hold a bunch of Gnutella packets
CG1Packet::CG1PacketPool CG1Packet::POOL;

//////////////////////////////////////////////////////////////////////
// CG1Packet construction

// Make a new CG1Packet object
CG1Packet::CG1Packet()
	: CPacket( PROTOCOL_G1 )
	, m_nType		( 0 )
	, m_nTypeIndex	( 0 )
	, m_nTTL		( 0 )
	, m_nHops		( 0 )
	, m_nHash		( 0 )
{
}

CG1Packet::~CG1Packet()
{
	// CPacket destructor will take care of freeing memory from this CG1Packet object
}

void CG1Packet::Reset()
{
	CPacket::Reset();

	m_oGUID.clear();
	m_nType		 = 0;
	m_nTypeIndex = 0;
	m_nTTL		 = 0;
	m_nHops		 = 0;
	m_nHash		 = 0;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet new

// Takes a packet type like ping, a TTL number, and a GUID for the packet
// Makes a new packet with these values and returns a pointer to it
CG1Packet* CG1Packet::New(int nType, DWORD nTTL, const Hashes::Guid& oGUID)
{
	// Get a pointer to a packet in the single global Gnutella packet pool
	CG1Packet* pPacket = (CG1Packet*)POOL.New();	// Calls CPacketPool::New, defined in Packet.h

	// Copy the given type and corresponding type index into it
	pPacket->m_nType = (BYTE)nType;
	pPacket->m_nTypeIndex = GnutellaTypeToIndex( pPacket->m_nType );

	// Set the TTL and hops counts
	pPacket->m_nTTL = (BYTE)( nTTL > 0 ? nTTL : Settings.Gnutella1.DefaultTTL );	// If the given TTL is 0, use the default instead
	pPacket->m_nHops = 0;	// This packet hasn't traveled across the Internet at all yet

	// No hash yet
	pPacket->m_nHash = 0;

	if ( oGUID )			// If the caller has given us a GUID to use in the packet
		pPacket->m_oGUID = oGUID;				// Copy the GUID into the packet
	else					// If the caller didn't give us a GUID to use
		Network.CreateID( pPacket->m_oGUID );	// Create a GUID for this packet

	// Return a pointer to the packet
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet type conversion

// Takes the byte in a Gnutella packet that describes what type it is, like ping or pong
// Returns the corresponding enumerantion index number, like 0, 1, 2, the program uses to mean the same thing
int CG1Packet::GnutellaTypeToIndex(BYTE nType)
{
	// Sort by the type byte, and return the corresponding index number which means the same thing
	switch ( nType )
	{
	case G1_PACKET_PING:		return G1_PACKTYPE_PING;		// Byte 0x00 is index 1, ping
	case G1_PACKET_PONG:		return G1_PACKTYPE_PONG;		// Byte 0x01 is index 2, pong
	case G1_PACKET_BYE:			return G1_PACKTYPE_BYE; 		// Byte 0x02 is index 3, bye
	case G1_PACKET_QUERY_ROUTE:	return G1_PACKTYPE_QUERY_ROUTE; // Byte 0x30 is index 4, query route
	case G1_PACKET_VENDOR:										// Bytes 0x31 and 0x32 are index 5, vendor
	case G1_PACKET_VENDOR_APP:	return G1_PACKTYPE_VENDOR;
	case G1_PACKET_PUSH:		return G1_PACKTYPE_PUSH;		// Byte 0x40 is index 6, push
	case G1_PACKET_QUERY:		return G1_PACKTYPE_QUERY;		// Byte 0x80 is index 7, query
	case G1_PACKET_HIT:			return G1_PACKTYPE_HIT;			// Byte 0x81 is index 8, hit
	default:					return G1_PACKTYPE_UNKNOWN;		// All other bytes are index 0, unknown
	}
}

//////////////////////////////////////////////////////////////////////
// CG1Packet hopping

// Adjusts the TTL and hops counts of this packet to record a trip across the Internet
// If the packet's TTL starts out less than 2, returns false without changing the numbers (do)
BOOL CG1Packet::Hop()
{
	// Make sure the packet's TTL is 2 or more
	if ( m_nTTL < 2 ) return FALSE;

	// Record a trip across the Internet
	m_nTTL--;	// One less trip to live
	m_nHops++;	// One more hop made

	// Report that we checked and changed the numbers
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet hashing

// Calculate a simple hash of the bytes of the packet, and store it in the member DWORD m_nHash
void CG1Packet::CacheHash()
{
	// Point pInput at the packet's buffer, where the bytes of the packet are stored
	BYTE* pInput = m_pBuffer;

	// Start out with the packet's hash set to 0
	m_nHash = 0;

	// Loop once for every byte in the packet
	for ( DWORD nPosition = m_nLength; nPosition; nPosition-- )			// If there are 10 bytes in the packet, loops 10 times, the loop doesn't use this index
	{
		// Use the byte under pInput to adjust a simple 4 byte hash of the packet
		m_nHash = ( m_nHash << 8 ) | ( ( m_nHash >> 24 ) ^ *pInput++ ); 	// After reading the byte under pInput, move to the next one
	}

	// If the last bit in the hash is 0, make it a 1
	m_nHash |= 1;
}

// Obsolete: Computes the SHA hash of a packet
// Takes the number of bytes in the packet to hash, or leave that blank to hash all of them, and writes under pHash   (Redefinition of default Parameter?)
//BOOL CG1Packet::GetPacketHash(Hashes::Sha1Hash& oHash, DWORD nLength) const
//{
//	// If the caller didn't specify a length, use the length of the packet
//	if ( nLength == 0xFFFFFFFF ) nLength = m_nLength;
//	if ( (DWORD)m_nLength < nLength ) return FALSE;		// Make sure the caller didn't ask for more than that
//
//	// Hash the data, writing the hash under pHash, and report success
//	CSHA pSHA;											// Make a local CSHA object which will compute the hash
//	pSHA.Add( &m_oGUID[ 0 ], Hashes::Guid::byteCount ); // Start by hashing the GUID of the packet
//	pSHA.Add( &m_nType, sizeof( m_nType ) );			// Then throw in the type byte
//	pSHA.Add( m_pBuffer, nLength );						// After that, hash the bytes of the packet
//	pSHA.Finish();										// Tell the object that is all
//	pSHA.GetHash( &oHash[ 0 ] );						// Have the object write the hash under pHash
//	oHash.validate();
//	return TRUE;										// Report success
//}

//////////////////////////////////////////////////////////////////////
// CG1Packet string conversion

// Use this array to lookup a packet index and get title text, like CG1Packet::m_pszPackets[ 1 ] resolves to "Ping"
LPCTSTR CG1Packet::m_pszPackets[ G1_PACKTYPE_MAX ] =	// There are 9 packet types, with values 0 through 8
{
	// 0 is unknown, 1 Ping, 2 Pong, and so on
	L"Unknown", L"Ping", L"Pong", L"Bye", L"QRP", L"Vendor", L"Push", L"Query", L"Hit"
};

// Uses the packet type index in this packet to look up text that describes that type, Returns text like "Ping" or "Pong"
CString CG1Packet::GetType() const
{
	// Return the pointer to the text literal defined above this method
	return m_pszPackets[ m_nTypeIndex ];
}

// Describes the GUID of this packet as text using base 16 notation, Returns a string
CString CG1Packet::GetGUID() const
{
	// Compose a string like "0001020304050607080910111213141516" with two characters to describe each of the 16 bytes of the GUID
	return m_oGUID.toString< Hashes::base16Encoding >();
}

//////////////////////////////////////////////////////////////////////
// CG1Packet buffer output

// Takes a pointer to a buffer
// Writes this Gnutella packet into it, composing a Gnutella packet header and then adding the payload from the packet's buffer
void CG1Packet::ToBuffer(CBuffer* pBuffer, bool /*bTCP*/)
{
	// Compose a Gnutella packet header with values from this CG1Packet object
	GNUTELLAPACKET pHeader;						// Make a local GNUTELLAPACKET structure called pHeader
	pHeader.m_oGUID		= m_oGUID.storage();	// Copy in the GUID
	pHeader.m_nType		= m_nType;				// Copy in the type byte
	pHeader.m_nTTL		= m_nTTL;				// Copy in the TTL and hops counts
	pHeader.m_nHops		= m_nHops;
	pHeader.m_nLength	= (LONG)m_nLength;		// Copy in the payload length number of bytes

	// Abort if the buffer isn't big enough for the packet
	if ( ! pBuffer->EnsureBuffer( sizeof( pHeader ) + m_nLength ) ) return;

	// Add the Gnutella packet header and packet payload to the buffer
	pBuffer->Add( &pHeader, sizeof( pHeader ) );	// First, copy the bytes of the Gnutella packet header structure we made right here
	pBuffer->Add( m_pBuffer, m_nLength );			// This packet object's buffer is the payload, copy that in after the header
}

//////////////////////////////////////////////////////////////////////
// CG1Packet debug

// Takes text that describes something that happened when debugging the program
// Writes it into a line at the bottom of the file Envy.log
#ifdef _DEBUG
void CG1Packet::Debug(LPCTSTR pszReason) const
{
	CString strOutput;
	strOutput.Format( L"[G1] %s Type: %s [%u/%u]", pszReason, (LPCTSTR)GetType(), m_nTTL, m_nHops );
	CPacket::Debug( strOutput );
}
#endif	// Deubg

int CG1Packet::GGEPReadCachedHosts(const CGGEPBlock& pGGEP)
{
	int nCount = -1;

	CGGEPItem* pIPPs = pGGEP.Find( GGEP_HEADER_PACKED_IPPORTS, 6 );
	if ( pIPPs && ( pIPPs->m_nLength - pIPPs->m_nPosition ) % 6 == 0 )
	{
		nCount = 0;
		while ( pIPPs->m_nPosition != pIPPs->m_nLength )
		{
			DWORD nAddress = 0;
			WORD nPort = 0;
			pIPPs->Read( (void*)&nAddress, 4 );
			pIPPs->Read( (void*)&nPort, 2 );
			DEBUG_ONLY( theApp.Message( MSG_DEBUG, L"[G1] Got host %s:%u", (LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nAddress ) ), nPort ) );
			CHostCacheHostPtr pCachedHost = HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort );
			if ( pCachedHost ) nCount++;
		}
	}

	if ( Settings.Experimental.EnableDIPPSupport )
	{
		CGGEPItem* pGDNAs = pGGEP.Find( GGEP_HEADER_GDNA_PACKED_IPPORTS, 6 );
		if ( ! pGDNAs ) 	// Older GDNA has a bug: they send DIP but receive DIPP (fixed in latest versions)
			pGDNAs = pGGEP.Find( GGEP_HEADER_GDNA_PACKED_IPPORTS_x, 6 );
		if ( pGDNAs && ( pGDNAs->m_nLength - pGDNAs->m_nPosition ) % 6 == 0 )
		{
			if ( nCount == -1 ) nCount = 0;
			while ( pGDNAs->m_nPosition != pGDNAs->m_nLength )
			{
				DWORD nAddress = 0;
				WORD nPort = 0;
				pGDNAs->Read( (void*)&nAddress, 4 );
				pGDNAs->Read( (void*)&nPort, 2 );
				DEBUG_ONLY( theApp.Message( MSG_DEBUG, L"Got GDNA host %s:%u", (LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nAddress ) ), nPort ) );
				CHostCacheHostPtr pCachedHost = HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort );
				if ( pCachedHost ) nCount++;
				HostCache.G1DNA.Add( (IN_ADDR*)&nAddress, nPort, 0, L"GDNA" );
			}
		}
	}

	return nCount;
}

void CG1Packet::GGEPWriteRandomCache(CGGEPBlock& pGGEP, LPCTSTR pszID)
{
	CArray< SOCKADDR_IN > pHosts;

	if ( _tcsicmp( pszID, GGEP_HEADER_PACKED_IPPORTS ) == 0 )
	{
		CQuickLock oLock( HostCache.Gnutella1.m_pSection );

		for ( CHostCacheIterator i = HostCache.Gnutella1.Begin();
			i != HostCache.Gnutella1.End(); ++i )
		{
			CHostCacheHostPtr pHost = (*i);

			// We won't provide Envy hosts for G1 cache,
			// since users may disable G1 and it will pollute the host caches (??)
			if ( pHost->m_nFailures == 0 &&
				 pHost->m_bCheckedLocally &&
				! ( pHost->m_pVendor && pHost->m_pVendor->m_sCode == L"GDNA" ) )
			{
				SOCKADDR_IN tmp = { AF_INET, pHost->m_nPort, pHost->m_pAddress };
				pHosts.Add( tmp );
			}
		}
	}
	else if ( _tcsicmp( pszID, GGEP_HEADER_GDNA_PACKED_IPPORTS ) == 0 ||
			  _tcsicmp( pszID, GGEP_HEADER_GDNA_PACKED_IPPORTS_x ) == 0 )
	{
		CQuickLock oLock( HostCache.G1DNA.m_pSection );

		for ( CHostCacheIterator i = HostCache.G1DNA.Begin(); i != HostCache.G1DNA.End(); ++i )
		{
			CHostCacheHostPtr pHost = (*i);

			// We won't provide Envy hosts for G1 cache,
			// since users may disable G1 and it will pollute the host caches (??)
			if ( pHost->m_nFailures == 0 &&
				 pHost->m_bCheckedLocally &&
				 ( pHost->m_pVendor && pHost->m_pVendor->m_sCode == L"GDNA" ) )
			{
				SOCKADDR_IN tmp = { AF_INET, pHost->m_nPort, pHost->m_pAddress };
				pHosts.Add( tmp );
			}
		}
	}

	if ( ! pHosts.GetCount() )
		return;

	while ( (DWORD)pHosts.GetCount() > Settings.Gnutella1.MaxHostsInPongs )
	{
		pHosts.RemoveAt( GetRandomNum( 0, (int)pHosts.GetCount() - 1 ) );
	}

	if ( CGGEPItem* pItem = pGGEP.Add( pszID ) )
	{
		for ( int i = 0; i < pHosts.GetCount(); i++ )
		{
			pItem->Write( (void*)&pHosts.GetAt( i ).sin_addr.s_addr, 4 );
			pItem->Write( (void*)&pHosts.GetAt( i ).sin_port, 2 );
			DEBUG_ONLY( theApp.Message( MSG_DEBUG, L"[G1] Sending host through pong (%s)", (LPCTSTR)HostToString( &pHosts.GetAt( i ) ) ) );
		}
	}
}

bool CG1Packet::ReadHUGE(CEnvyFile* pFile)
{
	const DWORD rem = GetRemaining();
	if ( rem == 0 )
		return false;	// End of packet

	const BYTE* p = GetCurrent();
	ASSERT( *p == 'u' );

	// Find length of extension (till packet end, G1_PACKET_HIT_SEP or null bytes)
	DWORD len = 0;
	for ( ; *p != G1_PACKET_HIT_SEP && *p && len < rem; ++p, ++len );

	p = GetCurrent();
	Seek( len, seekCurrent );

	if ( len < 4 || memcmp( p, "urn:", 4 ) != 0 )
		return false;	// Too short or unknown prefix

	if ( len < 16 )
		return true;	// Just a flag that host want a HUGE

	Hashes::Sha1Hash	oSHA1;
	Hashes::TigerHash	oTiger;
	Hashes::Ed2kHash	oED2K;
	Hashes::BtHash		oBTH;
	Hashes::Md5Hash		oMD5;

	CString strURN( (LPCSTR)p, len );

	if ( oSHA1.fromUrn( strURN ) )			// Got SHA1
		pFile->m_oSHA1 = oSHA1;
	else if ( oTiger.fromUrn( strURN ) )	// Got Tiger
		pFile->m_oTiger = oTiger;
	else if ( oED2K.fromUrn( strURN ) ) 	// Got ED2K
		pFile->m_oED2K = oED2K;
	else if ( oMD5.fromUrn( strURN ) )		// Got MD5
		pFile->m_oMD5 = oMD5;
	else if ( oBTH.fromUrn( strURN ) )		// Got BTH base32
		pFile->m_oBTH = oBTH;
	else if ( oBTH.fromUrn< Hashes::base16Encoding >( strURN ) )	// Got BTH base16
		pFile->m_oBTH = oBTH;
	else
		theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, L"[G1] Got packet with unknown HUGE \"%s\" (%d bytes)", strURN, len );

	return true;
}

bool CG1Packet::ReadXML(CSchemaPtr& pSchema, CXMLElement*& pXML)
{
	const DWORD rem = GetRemaining();
	if ( rem == 0 )
		return false;	// End of packet

	const BYTE* p = GetCurrent();
	ASSERT( *p == '<' || *p == '{' );

	// Find length of extension (till packet end, G1_PACKET_HIT_SEP or null bytes)
	DWORD len = 0;
	for ( ; *p != G1_PACKET_HIT_SEP && *p && len < rem; ++p, ++len );

	p = GetCurrent();
	Seek( len, seekCurrent );

	auto_array< BYTE > pTmp;
	if ( len >= 9 && memcmp( p, "{deflate}", 9 ) == 0 )
	{
		// Compressed text
		p += 9;
		len -= 9;

		// Deflate data
		DWORD nRealSize;
		pTmp = CZLib::Decompress( p, len, &nRealSize );
		if ( ! pTmp.get() )
			return NULL;	// Invalid data
		p = pTmp.get();
		len = nRealSize;
	}
	else if ( len >= 11 && memcmp( p, "{plaintext}", 11 ) == 0 )
	{
		// Plain text with long header
		p += 11;
		len -= 11;
	}
	else if ( len >= 2 && memcmp( p, "{}", 2 ) == 0 )
	{
		// Plain text with short header
		p += 2;
		len -= 2;
	}

	if ( len < 2 )
		return false;	// Too short

	CString strXML( UTF8Decode( (LPCSTR)p, len ) );

	// Fix <tag attribute="valueZ/> -> <tag attribute="value"/>
	//for ( DWORD i = 1; i + 2 < len; ++i )
	//	if ( szXML[ i ] == 0 && szXML[ i + 1 ] == '/' && szXML[ i + 2 ] == '>' )
	//		szXML[ i ] = '\"';

	// Decode XML
	pXML = CXMLElement::FromString( strXML );
	if ( ! pXML )
		pXML = AutoDetectSchema( strXML );	// Reconstruct XML from non-XML legacy data

	if ( SchemaCache.Normalize( pSchema, pXML ) )
		return true;

	// Invalid XML
	if ( pXML )
	{
		pXML->Delete();
		pXML = NULL;
	}

	return false;
}

CXMLElement* CG1Packet::AutoDetectSchema(LPCTSTR pszInfo)
{
	if ( _tcsstr( pszInfo, L" Kbps" ) != NULL &&
		 _tcsstr( pszInfo, L" kHz " ) != NULL )
	{
		return AutoDetectAudio( pszInfo );
	}

	return NULL;
}

CXMLElement* CG1Packet::AutoDetectAudio(LPCTSTR pszInfo)
{
	int nBitrate	= 0;
	int nFrequency	= 0;
	int nMinutes	= 0;
	int nSeconds	= 0;
	BOOL bVariable	= FALSE;

	if ( _stscanf( pszInfo, L"%i Kbps %i kHz %i:%i", &nBitrate, &nFrequency, &nMinutes, &nSeconds ) != 4 )
	{
		bVariable = TRUE;
		if ( _stscanf( pszInfo, L"%i Kbps(VBR) %i kHz %i:%i", &nBitrate, &nFrequency, &nMinutes, &nSeconds ) != 4 )
			return NULL;
	}

	if ( CXMLElement* pXML = new CXMLElement( NULL, L"audio" ) )
	{
		CString strValue;
		strValue.Format( L"%i", nMinutes * 60 + nSeconds );
		pXML->AddAttribute( L"seconds", strValue );

		strValue.Format( bVariable ? L"%lu~" : L"%lu", nBitrate );
		pXML->AddAttribute( L"bitrate", strValue );

		return pXML;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// UDP packet handler

BOOL CG1Packet::OnPacket(const SOCKADDR_IN* pHost)
{
	Statistics.Current.Gnutella1.Incoming++;

	SmartDump( pHost, TRUE, FALSE );

	if ( ! Settings.Gnutella1.Enabled ) return TRUE;

	switch ( m_nType )
	{
	case G1_PACKET_PING:
		return OnPing( pHost );
	case G1_PACKET_PONG:
		return OnPong( pHost );
	case G1_PACKET_VENDOR:
		return OnVendor( pHost );
	case G1_PACKET_QUERY:
		return OnQuery( pHost );
	case G1_PACKET_HIT:
		return OnCommonHit( pHost );
	case G1_PACKET_PUSH:
		return OnPush( pHost );

#ifdef _DEBUG
	default:
		CString str;
		str.Format( L"Unknown Gnutella UDP packet from %s", (LPCTSTR)HostToString( pHost ) );
		Debug( str );
#endif	// Debug
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// PING packet handler for G1UDP

BOOL CG1Packet::OnPing(const SOCKADDR_IN* pHost)
{
	Statistics.Current.Gnutella1.PingsReceived++;

	bool bSCP = false;
	bool bDNA = false;

	// A ping packet is just a header, and shouldn't have length

	if ( Settings.Gnutella1.EnableGGEP )
	{
		// There could be a GGEP block here, try checking and adjusting TTL and hops counts
		CGGEPBlock pGGEP;
		if ( pGGEP.ReadFromPacket( this ) )
		{
			if ( CGGEPItem* pItem = pGGEP.Find( GGEP_HEADER_SUPPORT_CACHE_PONGS ) )
			{
				bSCP = true;
			}

			if ( CGGEPItem* pItem = pGGEP.Find( GGEP_HEADER_SUPPORT_GDNA ) )
			{
				bDNA = true;
			}
		}
	}

	// Get statistics about how many files we are sharing
	QWORD nMyVolume = 0;
	DWORD nMyFiles = 0;
	LibraryMaps.GetStatistics( &nMyFiles, &nMyVolume );

	// Make a new pong packet, the response to a ping
	CG1Packet* pPong = New( G1_PACKET_PONG, m_nHops, m_oGUID );

	pPong->WriteShortLE( htons( Network.m_pHost.sin_port ) );
	pPong->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );

	// Then write information about how many files we are sharing
	pPong->WriteLongLE( nMyFiles );
	pPong->WriteLongLE( (DWORD)nMyVolume );

	if ( Settings.Gnutella1.EnableGGEP )
	{
		CGGEPBlock pGGEP;

		// ToDo: Gnutella 1 DHT
		//if ( Settings.Gnutella1.EnableDHT )
		//{
		//	if ( CGGEPItem* pItem = pGGEP.Add( GGEP_HEADER_DHT_SUPPORT ) )
		//	{
		//		pItem->Write( , 3 );
		//	}
		//}

		if ( CGGEPItem* pItem = pGGEP.Add( GGEP_HEADER_DAILY_AVERAGE_UPTIME ) )
		{
			pItem->WriteVary( Network.GetStableTime() );
		}

		// ToDo: Gnutella 1 Client Locale
		//if ( CGGEPItem* pItem = pGGEP.Add( GGEP_HEADER_CLIENT_LOCALE ) )
		//{
		//	pItem->Write( "en#", 4 );
		//}

		// ToDo: Gnutella 1 TLS Encyption
		//if ( Settings.Gnutella1.EnableTLS )
		//	pGGEP.Add( GGEP_HEADER_TLS_SUPPORT ) );

		if ( CGGEPItem* pItem = pGGEP.Add( GGEP_HEADER_VENDOR_INFO ) )
		{
			pItem->Write( VENDOR_CODE, 4 );
			pItem->WriteByte( ( theApp.m_nVersion[ 0 ] << 4 ) | theApp.m_nVersion[ 1 ] );
		}

		if ( bSCP )
			GGEPWriteRandomCache( pGGEP, GGEP_HEADER_PACKED_IPPORTS );

		if ( bDNA && Settings.Experimental.EnableDIPPSupport )
			GGEPWriteRandomCache( pGGEP, GGEP_HEADER_GDNA_PACKED_IPPORTS );

		pGGEP.Write( pPong );
	}

	// Send the pong packet to the remote computer we are currently looping on
	Datagrams.Send( pHost, pPong );

	Statistics.Current.Gnutella1.PongsSent++;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// PONG packet handler

BOOL CG1Packet::OnPong(const SOCKADDR_IN* pHost)
{
	Statistics.Current.Gnutella1.PongsReceived++;

	if ( m_nLength < 14 )
	{
		// Pong packets should be 14 bytes long, drop this strange one
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_SIZE_PACKET, (LPCTSTR)HostToString( pHost ), L"pong" );
		Statistics.Current.Gnutella1.Dropped++;
		return FALSE;
	}

	// Read information from the pong packet
	WORD  nPort 	= ReadShortLE();	// 2 bytes, port number (do) ...of us? the remote computer? the computer that sent the packet?
	DWORD nAddress	= ReadLongLE(); 	// 4 bytes, IP address
	DWORD nFiles	= ReadLongLE(); 	// 4 bytes, the number of files the source computer is sharing
	DWORD nVolume	= ReadLongLE(); 	// 4 bytes, the total size of all those files

	// ToDo: Use share info?
	UNUSED_ALWAYS( nFiles );
	UNUSED_ALWAYS( nVolume );

	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	CString strVendorCode;
	CHAR nVersion[ 4 ] = {};
	DWORD nUptime = 0;
	bool bUltrapeer = false;
	bool bGDNA = false;
	int nCachedHostsCount = -1;

	// If the pong is bigger than 14 bytes, and handshake said it supports GGEP blocks
	if ( m_nLength > 14 && Settings.Gnutella1.EnableGGEP )
	{
		// There is a GGEP block here, and checking and adjusting the TTL and hops counts worked
		CGGEPBlock pGGEP;
		if ( pGGEP.ReadFromPacket( this ) )
		{
			// Read vendor code
			if ( CGGEPItem* pVC = pGGEP.Find( GGEP_HEADER_VENDOR_INFO, 4 ) )
			{
				CHAR szaVendor[ 5 ] = {};
				pVC->Read( szaVendor, 4 );
				strVendorCode = szaVendor;
				if ( pVC->m_nLength == 5 )
				{
					BYTE nVersionPacked = pVC->ReadByte();
					nVersion[ 0 ] = ( nVersionPacked >> 4 ) & 0x0f;
					nVersion[ 1 ] = nVersionPacked & 0x0f;
				}
			}

			// Read daily uptime
			if ( CGGEPItem* pDU = pGGEP.Find( GGEP_HEADER_DAILY_AVERAGE_UPTIME, 1 ) )
			{
				pDU->Read( (void*)&nUptime, 4 );
			}

			// Catch pongs and update host cache only from ultrapeers
			if ( CGGEPItem* pUP = pGGEP.Find( GGEP_HEADER_UP_SUPPORT ) )
			{
				bUltrapeer = true;
			}

			if ( CGGEPItem* pGDNA = pGGEP.Find( GGEP_HEADER_SUPPORT_GDNA ) )
			{
				bGDNA = true;
			}

			nCachedHostsCount = GGEPReadCachedHosts( pGGEP );
		}
	}

	if ( bUltrapeer )
	{
		HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort, 0, strVendorCode, nUptime );

		if ( bGDNA )
			HostCache.G1DNA.Add( (IN_ADDR*)&nAddress, nPort, 0, strVendorCode, nUptime );
	}

	// Update Gnutella UDPHC state
	if ( nCachedHostsCount > 0 )
	{
		if ( CDiscoveryService* pService = DiscoveryServices.GetByAddress(
			&(pHost->sin_addr), ntohs( pHost->sin_port ), CDiscoveryService::dsGnutellaUDPHC ) )
		{
			pService->OnSuccess();
			pService->m_nHosts = nCachedHostsCount;
			pService->OnCopyGiven();
		}
	}

	return TRUE;
}

BOOL CG1Packet::OnVendor(const SOCKADDR_IN* pHost)
{
	if ( m_nLength < 8 || ! Settings.Gnutella1.VendorMsg )
	{
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	// Read the vendor, function, and version numbers from the packet payload
	DWORD nVendor	= ReadLongBE(); 	// 4 bytes, vendor code in ASCII characters, like "RAZA" (do)
	WORD  nFunction	= ReadShortLE();	// 2 bytes, function (do)
	WORD  nVersion	= ReadShortLE();	// 2 bytes, version (do)

	if ( nVendor == 'LIME' )
	{
		if ( nFunction == 23 && nVersion == 2 )
			return TRUE;	// ToDo: HEAD ping
	}

#ifdef _DEBUG
	CString str;
	str.Format( L"Received vendor packet from %s Function: %u Version: %u.",
		(LPCTSTR)HostToString( pHost ), nFunction, nVersion );
	Debug( str );
#else
	UNUSED_ALWAYS( pHost );
#endif	// Debug

	return TRUE;
}

BOOL CG1Packet::OnQuery(const SOCKADDR_IN* pHost)
{
	Statistics.Current.Gnutella1.Queries++;

	// if the packet payload is too long
	if ( m_nLength > Settings.Gnutella1.MaximumQuery )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_TOO_LARGE, (LPCTSTR)HostToString( pHost ) );
		Statistics.Current.Gnutella1.Dropped++;
		return FALSE;
	}

	// if TTL is wrong
	if ( m_nTTL > 1 )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_HIGH_TTL, (LPCTSTR)HostToString( pHost ), m_nTTL, m_nHops );
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	CQuerySearchPtr pSearch = CQuerySearch::FromPacket( this, pHost );
	if ( ! pSearch || pSearch->m_bDropMe )
	{
		if ( ! pSearch )
		{
			theApp.Message( MSG_WARNING, IDS_PROTOCOL_BAD_QUERY, L"G1", (LPCTSTR)HostToString( pHost ) );
			DEBUG_ONLY( Debug( L"Malformed Query." ) );
		}
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	Network.OnQuerySearch( new CLocalSearch( pSearch, PROTOCOL_G1 ) );

	Statistics.Current.Gnutella1.QueriesProcessed++;

	return TRUE;
}

BOOL CG1Packet::OnCommonHit(const SOCKADDR_IN* pHost)
{
	if ( m_nLength < 11 + Hashes::Guid::byteCount )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)HostToString( pHost ) );
		Statistics.Current.Gnutella1.Dropped++;
		return FALSE;
	}

	// if TTL is wrong
	if ( m_nTTL > 1 )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_HIGH_TTL, (LPCTSTR)HostToString( pHost ), m_nTTL, m_nHops );
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	int nHops = 0;
	CQueryHit* pHits = CQueryHit::FromG1Packet( this, &nHops );

	if ( ! pHits )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)HostToString( pHost ) );
		DEBUG_ONLY( Debug( L"Malformed Hit" ) );
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	if ( Security.IsDenied( &pHits->m_pAddress ) )
	{
		Statistics.Current.Gnutella1.Dropped++;
		pHits->Delete();
		return TRUE;
	}

	if ( ! pHits->m_bBogus )
	{
		// Don't route exceeded hits
		if ( nHops <= (int)Settings.Gnutella1.MaximumTTL &&
			SearchManager.OnQueryHits( pHits ) )
		{
			Network.RouteHits( pHits, this );
		}
	}

	Network.OnQueryHits( pHits );

	return TRUE;
}

BOOL CG1Packet::OnPush(const SOCKADDR_IN* pHost)
{
	if ( m_nLength < 26 )
	{
		theApp.Message( MSG_NOTICE, IDS_PROTOCOL_SIZE_PACKET, (LPCTSTR)HostToString( pHost ), L"push" );
		Statistics.Current.Gnutella1.Dropped++;
		return FALSE;
	}

	Hashes::Guid oClientID;
	Read( oClientID );
	DWORD nFileIndex = ReadLongLE();	// 4 bytes, the file index
	DWORD nAddress	 = ReadLongLE();	// 4 bytes, the IP address of
	WORD nPort		 = ReadShortLE();	// 2 bytes, the port number

	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	// ToDo: Gnutella 1 TLS Encyption?
	//bool bTLS = false;
	//
	//if ( m_nLength > 26 && Settings.Gnutella1.EnableTLS )
	//{
	//	CGGEPBlock pGGEP;
	//	if ( pGGEP.ReadFromPacket( this ) )
	//	{
	//		if ( pGGEP.Find( GGEP_HEADER_TLS_SUPPORT ) )
	//			bTLS = true;
	//	}
	//}

	if ( ! nPort || ( m_nHops && (
		Network.IsFirewalledAddress( (IN_ADDR*)&nAddress ) ||
		Network.IsReserved( (IN_ADDR*)&nAddress ) ) ) )
	{
		theApp.Message( MSG_NOTICE, IDS_PROTOCOL_ZERO_PUSH, (LPCTSTR)HostToString( pHost ) );
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	if ( validAndEqual( oClientID, Hashes::Guid( MyProfile.oGUID ) ) )
	{
		Handshakes.PushTo( (IN_ADDR*)&nAddress, nPort, nFileIndex );
		return TRUE;
	}

	return TRUE;
}
