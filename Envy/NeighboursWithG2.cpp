//
// NeighboursWithG2.cpp
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

// Adds 2 methods helpful for Gnutella2 that look at the list of neighbours
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CNeighboursWithG2
// http://getenvy.com/archives/shareazawiki/Developers.Code.CNeighboursWithG2.html

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "NeighboursWithG2.h"
#include "G2Neighbour.h"
#include "G2Packet.h"
#include "Network.h"
#include "HubHorizon.h"
#include "RouteCache.h"
#include "HostCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG2 construction (Nothing needed)

CNeighboursWithG2::CNeighboursWithG2()
{
}

CNeighboursWithG2::~CNeighboursWithG2()
{
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG2 connect

// Set the ping route cache duration from Gnutella settings, and setup the Gnutella2 hub horizon pool
void CNeighboursWithG2::Connect()
{
	// Set the ping route cache duration from the program settings for Gnutella
	CNeighboursWithG1::Connect();

	// Setup the Gnutella 2 hub horizon pool
	HubHorizonPool.Setup();
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG2 create query web packet

// Takes a GUID, and a neighbour to except from the packet we will make
// Makes a Gnutella2 query web packet, containing the IP addresses of the computers we are connected to and from the Gnutella2 host cache
CG2Packet* CNeighboursWithG2::CreateQueryWeb(const Hashes::Guid& oGUID, bool bWithHubs, CNeighbour* pExcept, bool bDone)
{
	// Make a new Gnutella2 Query Ack packet
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_QUERY_ACK, TRUE );
	if ( ! pPacket )
		return NULL;

	// Start it with the text "TS" and the time now
	const DWORD tNow = static_cast< DWORD >( time( NULL ) );	// Number of seconds since 1970
	pPacket->WritePacket( G2_PACKET_TIMESTAMP, 4 );
	pPacket->WriteLongBE( tNow );

	// Write in header information about us
	pPacket->WritePacket( G2_PACKET_FROM_ADDRESS, 4 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WritePacket( G2_PACKET_RETRY_AFTER, 4 );
	pPacket->WriteLongBE( Settings.Gnutella2.QueryThrottle );

	if ( bDone )
	{
		pPacket->WritePacket( G2_PACKET_QUERY_DONE, 8 );
		pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );

		if ( bWithHubs )
		{
			const WORD nLeafs = (WORD)GetCount( PROTOCOL_G2, nrsConnected, ntLeaf );
			pPacket->WriteShortBE( nLeafs );

			// Loop through the connected computers
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				// Get the neighbour object at this position, and move pos to the next one
				CG2Neighbour* pNeighbour = (CG2Neighbour*)GetNext( pos );

				// If this neighbour is running Gnutella2 software
				if ( pNeighbour->m_nProtocol == PROTOCOL_G2 &&	// The remote computer is running Gnutella2 software, and
					 pNeighbour->m_nNodeType != ntLeaf      &&	// Our connection to it is not down to a leaf, and
					 pNeighbour->m_nState >= nrsConnected   &&	// We've finished the handshake with it, and
					 pNeighbour != pExcept )					// This isn't the computer the caller warned us to except
				{
					// Write information about this connected computer into the packet
					pPacket->WritePacket( G2_PACKET_QUERY_DONE, 8 );
					pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );
					pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );
					pPacket->WriteShortBE( (WORD)pNeighbour->m_nLeafCount );
				}
			}

			// Will put up to 3 or 25 IP addresses in the packet:
			// If caller didn't give us a computer to ignore make nCount 3, if it did give us an except make nCount 25
			int nCount = ( pExcept == NULL ) ? 3 : 25;

			CQuickLock oLock( HostCache.Gnutella2.m_pSection );

			// Loop, starting with the newest entry in the Gnutella2 host cache, then stepping to the one before that
			for ( CHostCacheIterator i = HostCache.Gnutella2.Begin() ; i != HostCache.Gnutella2.End() ; ++i )
			{
				CHostCacheHostPtr pHost = (*i);

				// If this host cache entry is good
				if ( pHost->CanQuote( tNow ) &&								// If this host cache entry hasn't expired, and
					 Get( pHost->m_pAddress ) == NULL &&					// We're connected to that IP address right now, and
					 HubHorizonPool.Find( &pHost->m_pAddress ) == NULL )	// The IP address is also in the hub horizon pool
				{
					// Add the IP address to the packet we're making, to encourage recipient to try this address
					pPacket->WritePacket( G2_PACKET_QUERY_SEARCH, 10 );
					pPacket->WriteLongLE( pHost->m_pAddress.S_un.S_addr );
					pPacket->WriteShortBE( pHost->m_nPort );
					pPacket->WriteLongBE( pHost->Seen() );

					// Lower the count, if it is then 0, leave the loop
					if ( ! --nCount ) break;
				}
			}

			// Give the packet we're making to our own hub horizon pool
			HubHorizonPool.AddHorizonHubs( pPacket );
		}
		else // No hubs
		{
			pPacket->WriteShortBE( 0 );
		}
	}

	// Finish the packet with a 0 byte and the guid the caller gave us, and return it
	pPacket->WriteByte( 0 );
	pPacket->Write( oGUID );

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG2 random hub selector

// Takes a connected computer to ignore, and a GUID (do)
// Randomly chooses a neighbour from amongst those that are connected, running Gnutella2, hubs, and don't know about the GUID
// Returns a pointer to that randomly selected neighbour
CG2Neighbour* CNeighboursWithG2::GetRandomHub(CG2Neighbour* pExcept, const Hashes::Guid& oGUID)
{
	// Make a new local empty list that will hold pointers to neighbours
	CArray< CG2Neighbour* > pRandom;

	// Loop through each computer we're connected to
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		// Get the neighbour under the current position, and move to the next one in the list
		CNeighbour* pNeighbour = GetNext( pos );

		// If this is a Gnutella2 hub
		if ( pNeighbour->m_nState == nrsConnected   &&	// We've finished the handshake with this computer, and
			 pNeighbour->m_nProtocol == PROTOCOL_G2 &&	// It's running Gnutella2 software, and
			 pNeighbour->m_nNodeType != ntLeaf      &&	// Our connection to it isn't down to a leaf, and
			 pNeighbour != pExcept )					// It's not the one the caller told us to avoid
		{
			// And it doesn't know the given GUID, add it to the random list
			if ( static_cast< CG2Neighbour* >( pNeighbour )->m_pGUIDCache->Lookup( oGUID ) == NULL )
				pRandom.Add( static_cast< CG2Neighbour* >( pNeighbour ) );
		}
	}

	// If we didn't find any neighbours to put in the list, return null
	INT_PTR nSize = pRandom.GetSize();
	if ( ! nSize ) return NULL;

	// Choose a random number between 0 and nSize - 1, use it as an index, and return the neighbour at it
	nSize = GetRandomNum< INT_PTR >( 0, nSize - 1 );
	return pRandom.GetAt( nSize );
}
