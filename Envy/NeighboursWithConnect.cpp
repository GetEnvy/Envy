//
// NeighboursWithConnect.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "NeighboursWithConnect.h"
#include "Neighbours.h"
#include "ShakeNeighbour.h"
#include "EDNeighbour.h"
#include "DCNeighbour.h"
#include "BTPacket.h"
#include "Kademlia.h"
#include "Network.h"
#include "Datagrams.h"
#include "Security.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
//#include "Scheduler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect construction

CNeighboursWithConnect::CNeighboursWithConnect()
	: m_nBandwidthIn	( 0 )
	, m_nBandwidthOut	( 0 )
	, m_nStableCount	( 0 )
	, m_bG2Leaf			( FALSE )
	, m_bG2Hub			( FALSE )
	, m_bG1Leaf			( FALSE )
	, m_bG1Ultrapeer	( FALSE )
	, m_tHubG2Promotion	( 0 )
	, m_tPresent		( )
	, m_tPriority		( )
	, m_tLastConnect	( 0 )
{
}

CNeighboursWithConnect::~CNeighboursWithConnect()
{
}

void CNeighboursWithConnect::Close()
{
	CNeighboursWithRouting::Close();

	m_nBandwidthIn	= 0;
	m_nBandwidthOut	= 0;
	m_nStableCount	= 0;
	m_bG2Leaf		= FALSE;
	m_bG2Hub		= FALSE;
	m_bG1Leaf		= FALSE;
	m_bG1Ultrapeer	= FALSE;
	m_tHubG2Promotion = 0;
	ZeroMemory( &m_tPresent, sizeof( m_tPresent ) );
	ZeroMemory( &m_tPriority, sizeof( m_tPriority ) );
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect connection initiation

// Maintain calls CHostCacheHost::ConnectTo, which calls this
// Takes an IP address and port number from the host cache, and connects to it
// Returns a pointer to the new neighbour in the connected list, or null if no connection was made
CNeighbour* CNeighboursWithConnect::ConnectTo(
	const IN_ADDR& pAddress,	// IP address from the host cache to connect to, like 67.163.208.23
	WORD       nPort,			// Port number that goes with that IP address, like 6346
	PROTOCOLID nProtocol,		// Protocol name, like PROTOCOL_G1 for Gnutella
	BOOL       bAutomatic,		// True to (do)
	BOOL       bNoUltraPeer)	// By default, false to not (do)
{
	// Don't connect to self
	if ( Settings.Connection.IgnoreOwnIP && Network.IsSelfIP( pAddress ) )
		return NULL;

	// Don't connect to blocked addresses
	if ( Security.IsDenied( &pAddress ) )
	{
		// If not automatic (do), report that this address is on the block list, and return no new connection made
		if ( ! bAutomatic )
			theApp.Message( MSG_ERROR, IDS_NETWORK_SECURITY_OUTGOING, (LPCTSTR)CString( inet_ntoa( pAddress ) ) );
		return NULL;
	}

	// If automatic (do) and the network object knows this IP address is firewalled and can't receive connections, give up
	if ( bAutomatic && Network.IsFirewalledAddress( &pAddress, TRUE ) )
		return NULL;

	// Get this thread exclusive access to the network (do) while this method runs
	// When control leaves the method, pLock will go out of scope and release access
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 200 ) )
		return NULL;

	// If the list of connected computers already has this IP address
	if ( Get( pAddress ) )
	{
		// If not automatic (do), report that we're already connected to that computer, and return no new connection made
		if ( ! bAutomatic )
			theApp.Message( MSG_ERROR, IDS_CONNECTION_ALREADY_ABORT, (LPCTSTR)CString( inet_ntoa( pAddress ) ) );
		return NULL;
	}

	// If the caller wants automatic behavior, then make this connection request also connect the network it is for
	if ( ! bAutomatic )
	{
		// Activate the appropriate network (if required)
		switch ( nProtocol )
		{
		case PROTOCOL_G1:
			Settings.Gnutella1.Enabled = true;
			break;
		case PROTOCOL_G2:
			Settings.Gnutella2.Enabled = true;
			break;
		case PROTOCOL_ED2K:
			Settings.eDonkey.Enabled = true;
			CloseDonkeys();		// Reset the eDonkey2000 network (do)
			break;
		case PROTOCOL_BT:
			Settings.BitTorrent.Enabled = true;
			Settings.BitTorrent.EnableDHT = true;
			break;
		case PROTOCOL_DC:
			Settings.DC.Enabled = true;
			break;
		case PROTOCOL_KAD:
			Settings.eDonkey.Enabled = true;
			break;
		//default:
		//	ASSERT( ! nProtocol );
		}
	}

	// Run network connect (do), and leave if it reports an error
	if ( ! Network.Connect() )
		return NULL;

	// Create a compatible Neighbour object type connected to the IP address, and return the pointer to it

	switch ( nProtocol )
	{
	case PROTOCOL_ED2K:
		{
			augment::auto_ptr< CEDNeighbour > pNeighbour( new CEDNeighbour() );
			if ( pNeighbour->ConnectTo( &pAddress, nPort, bAutomatic ) )
				return pNeighbour.release();			// Started connecting to an ed2k neighbour
		}
		break;

	case PROTOCOL_DC:
		{
			augment::auto_ptr< CDCNeighbour > pNeighbour( new CDCNeighbour() );
			if ( pNeighbour->ConnectTo( &pAddress, nPort, bAutomatic ) )
				return pNeighbour.release();			// Started connecting to a dc++ neighbour
		}
		break;

	case PROTOCOL_BT:
		{
			DHT.Ping( &pAddress, nPort );
		}
		break;

	case PROTOCOL_KAD:
		{
			SOCKADDR_IN pHost = { AF_INET, htons( nPort ), pAddress };
			Kademlia.Bootstrap( &pHost );
		}
		break;

	default:	// PROTOCOL_G1/PROTOCOL_G2
		{
			augment::auto_ptr< CShakeNeighbour > pNeighbour( new CShakeNeighbour() );
			if ( pNeighbour->ConnectTo( &pAddress, nPort, bAutomatic, bNoUltraPeer ) )
			{
				// If we only want G1 connections now, specify that to begin with
				if ( Settings.Gnutella.SpecifyProtocol )
					pNeighbour->m_nProtocol = nProtocol;
				return pNeighbour.release();			// Started connecting to a Gnutella/G2 neighbour
			}
		}
	}

	return NULL;	// Unable to connect, some other protocol?
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect accept a connection

// CHandshake::OnRead gets an incoming socket connection, looks at the first 7 bytes, and passes Gnutella and Gnutella2 here
// Takes a pointer to the CHandshake object the program made when it accepted the new connection from the listening socket
// Makes a new CShakeNeighbour object, and calls AttachTo to have it take this incoming connection
// Returns a pointer to the CShakeNeighbour object
BOOL CNeighboursWithConnect::OnAccept(CConnection* pConnection)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 150 ) )
		return TRUE;	// Try again later

	if ( Neighbours.Get( pConnection->m_pHost.sin_addr ) )
	{
		pConnection->Write( _P("GNUTELLA/0.6 503 Duplicate connection\r\n\r\n") );
		pConnection->LogOutgoing();
		pConnection->DelayClose( IDS_CONNECTION_ALREADY_REFUSE );
		return TRUE;
	}

	if ( CShakeNeighbour* pNeighbour = new CShakeNeighbour() )
	{
		pNeighbour->AttachTo( pConnection );
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect

// If we've been demoted to the leaf role for a protocol, this function trims peers after we get a hub (do)
// Takes a protocol like PROTOCOL_G1 or PROTOCOL_G2
// If we don't need any more hub connections, closes them all (do)
void CNeighboursWithConnect::PeerPrune(PROTOCOLID nProtocol)
{
	// True if we need more hub connections for the requested protocol
	BOOL bNeedMore = NeedMoreHubs( nProtocol );

	// True if we need more hub connections for either Gnutella or Gnutella2
	BOOL bNeedMoreAnyProtocol = NeedMoreHubs( PROTOCOL_NULL );

	// Loop through all the neighbours in the list
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		// Get the neighbour at this position in the list, and move to the next one
		CNeighbour* pNeighbour = GetNext( pos );

		// This neighbour is on the network the caller wants us to prune, and
		if ( pNeighbour->m_nProtocol == nProtocol )
		{
			// Our connection to this neighbour is not up to a hub, and
			if ( pNeighbour->m_nNodeType != ntHub )
			{
				// Either we don't need any more hubs, or we're done with the handshake so we know it wont' be a hub, then drop this connection
				if ( ! bNeedMore || pNeighbour->m_nState == nrsConnected )
					pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
			}
		}
		else if ( pNeighbour->m_nProtocol == PROTOCOL_NULL )
		{
			// This must be a Gnutella or Gnutella2 computer in the middle of the handshake
			// If we initiated the connection, we know it's not a leaf trying to contact us, it's probably a hub
			if ( pNeighbour->m_bInitiated )
			{
				// If we don't need any more hubs, on any protocol, drop this connection
				if ( ! bNeedMoreAnyProtocol )
					pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
			}
		}
	}
}

// Determines if we are a leaf on the Gnutella2 network right now
bool CNeighboursWithConnect::IsG2Leaf() const
{
	// If the network is enabled (do) and we have at least 1 connection up to a hub, or say so, then we're a leaf
	return ( Settings.Gnutella2.ClientMode == MODE_LEAF || m_bG2Leaf ) && Network.IsConnected();
}

// Determines if we are a hub on the Gnutella2 network right now
bool CNeighboursWithConnect::IsG2Hub() const
{
	// If the network is enabled (do) and we have at least 1 connection down to a leaf, or say so, then we're a hub
	return ( Settings.Gnutella2.ClientMode == MODE_HUB || m_bG2Hub ) && Network.IsConnected();
}

// Takes true if we are running the program in debug mode, and this method should write out debug information
// Determines if the computer and Internet connection here are strong enough for this program to run as a Gnutella2 hub
// Returns false, which is 0, if we can't be a hub, or a number 1+ that is higher the better hub we'd be
DWORD CNeighboursWithConnect::IsG2HubCapable(BOOL bIgnoreTime, BOOL bDebug) const
{
	// Start the rating at 0, which means we can't be a hub
	DWORD nRating = 0;		// Increment this number as we find signs we can be a hub

	// If the caller wants us to report debugging information, start out with a header line
	if ( bDebug ) theApp.Message( MSG_DEBUG, L"Is G2 hub capable?" );		// protocolNames[ PROTOCOL_G2 ]

	// We can't be a Gnutella2 hub if the user has not chosen to connect to Gnutella2 in the program settings
	if ( ! Network.IsConnected() || ! Settings.Gnutella2.Enabled )
	{
		// Finish the lines of debugging information, and report no, we can't be a hub
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: G2 not enabled" );		// protocolNames[ PROTOCOL_G2 ]
		return FALSE;
	}

	// The user can go into settings and check a box to make the program never run in hub mode, even if it could
	if ( Settings.Gnutella2.ClientMode == MODE_LEAF )	// If user disabled hub mode in settings
	{
		// Finish the lines of debugging information, and report no, we can't be a hub
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: hub mode disabled" );
		return FALSE;
	}

	// We are running as a Gnutella2 leaf right now
	if ( IsG2Leaf() )
	{
		// We can never be a hub because we are a leaf (do)
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: leaf" );
		return FALSE;
	}
	else // We are not running as a Gnutella2 leaf right now (do)
	{
		// Note this and keep going
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"OK: not a leaf" );
	}

	// The user can check a box in settings to let the program become a hub without passing the additional tests below
	if ( Settings.Gnutella2.ClientMode == MODE_HUB )
	{
		// Make a note about this and keep going
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"YES: hub mode forced" );
	}
	else // MODE_AUTO: User didn't check the force hub box in settings, so the client will have to pass additional tests below
	{
		// Note base physical memory check in CalculateSystemPerformanceScore

		// Check the connection speed, make sure the download speed is fast enough
		if ( Settings.Connection.InSpeed < 200 )	// If the inbound speed is less than 200 (do)
		{
			// Download speed too slow
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: less than 200 Kb/s in" );
			return FALSE;
		}

		// Make sure the upload speed is fast enough
		if ( Settings.Connection.OutSpeed < 200 )
		{
			// Upload speed too slow
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: less than 200 Kb/s out" );
			return FALSE;
		}

		// Make sure we are not also a forced ultrapeer on gnutella
		if ( IsG1Ultrapeer() )
		{
			// Already ultrapeer mode overhead
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: Gnutella ultrapeer active" );
			return FALSE;
		}

		// Confirm how long the node has been running.
		if ( ! bIgnoreTime )	// This is unhandled (never skipped)  ToDo: Allow for new network?
		{
			if ( Settings.Gnutella2.HubVerified )
			{
				// Systems that have been good hubs before can promote in 2 hours
				if ( Network.GetStableTime() < 7200 )
				{
					if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: not stable for 2 hours" );
					return FALSE;
				}

				if ( bDebug ) theApp.Message( MSG_DEBUG, L"OK: stable for 2 hours" );
			}
			else // Untested hubs need 3 hours uptime to be considered
			{
				if ( Network.GetStableTime() < 10800 )
				{
					if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: not stable for 3 hours" );
					return FALSE;
				}

				if ( bDebug ) theApp.Message( MSG_DEBUG, L"OK: stable for 3 hours" );
			}
		}

		// Make sure the datagram is stable (do)
		if ( Network.IsFirewalled(CHECK_UDP) )
		{
			// Record this is why we can't be a hub, and return no
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: datagram not stable" );
			return FALSE;
		}
		else // The datagram is stable (do)
		{
			// Make a note we passed this test, and keep going
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"OK: datagram stable" );
		}

		// Report that we meet the minimum requirements to be a hub
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"YES: hub capable by test" );
	}

	// If we've made it this far, change the rating number from 0 to 1
	nRating = 1 + CalculateSystemPerformanceScore( bDebug );	// The higher it is, the better a hub we can be

	// The program is not connected to other networks

	if ( ! Settings.Gnutella1.Enabled )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"Gnutella not enabled" );		// protocolNames[ PROTOCOL_G1 ]
	}

	if ( ! Settings.eDonkey.Enabled )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"eDonkey not enabled" );		// protocolNames[ PROTOCOL_ED2K ]
	}

	if ( ! Settings.DC.Enabled )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"DC++ not enabled" );			// protocolNames[ PROTOCOL_DC ]
	}

	if ( ! Settings.BitTorrent.Enabled )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"BitTorrent not enabled" );	// protocolNames[ PROTOCOL_BT ]
	}

	if ( bDebug )
	{
		CString strRating;
		strRating.Format( L"Hub rating: %u", nRating );
		theApp.Message( MSG_DEBUG, strRating );
	}

	// Return 0 if we can't be a Gnutella2 hub, or 1+ if we can, a higher number means we'd be a better hub
	return nRating;
}

// Determines if we are a leaf on the Gnutella network right now
bool CNeighboursWithConnect::IsG1Leaf() const
{
	// If the network is enabled (do) and we have at least 1 connection up to an ultrapeer, then we're a leaf
	return ( Settings.Gnutella1.ClientMode == MODE_LEAF || m_bG1Leaf ) && Network.IsConnected();
}

// Determines if we are an ultrapeer on the Gnutella network right now  (never, unless forced for testing)
bool CNeighboursWithConnect::IsG1Ultrapeer() const
{
	// If the network is enabled (do) and we have at least 1 connection down to a leaf, or say so, then we're an ultrapeer
	return ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER || m_bG1Ultrapeer ) && Network.IsConnected();
}

// Takes true if we are running the program in debug mode, and this method should write out debug information
// Determines if the computer and Internet connection here are strong enough for this program to run as a Gnutella ultrapeer
// Returns false, which is 0, if we can't be an ultrapeer, or a number 1+ that is higher the better ultrapeer we'd be
DWORD CNeighboursWithConnect::IsG1UltrapeerCapable(BOOL bIgnoreTime, BOOL bDebug) const
{
	// Start out the rating as 0, meaning we can't be a Gnutella ultrapeer
	DWORD nRating = 0;		// If we can be an ultrapeer, we'll set this to 1, and then make it higher if we'd be an even better ultrapeer

	// If the caller requested we write out debugging information, start out by titling that the messages that follow come from this method
	if ( bDebug ) theApp.Message( MSG_DEBUG, L"Is Gnutella ultrapeer capable?" );	// protocolNames[ PROTOCOL_G1 ]

	// We can't be a Gnutella ultrapeer if we're not connected to the Gnutella network
	if ( ! Settings.Gnutella1.Enabled || ! Network.IsConnected() )
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: Gnutella not enabled" );		// protocolNames[ PROTOCOL_G1 ]
		return FALSE;
	}

	// The user can go into settings and check a box to make the program never become an ultrapeer, even if it could
	if ( Settings.Gnutella1.ClientMode == MODE_LEAF )
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: ultrapeer mode disabled" );
		return FALSE;
	}

	// We are running as a Gnutella leaf right now
	if ( IsG1Leaf() )
	{
		// We can never be an ultrapeer because we are a leaf (do)
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: leaf" );
		return FALSE;
	}
	else // We are running as a Gnutella ultrapeer already (do)
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"OK: not a leaf" );
	}

	// The user can check a box in settings to let the program become an ultrapeer without passing the additional tests below
	if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
	{
		// Note this and keep going
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"YES: ultrapeer mode forced" );
	}
	else // MODE_AUTO: User didn't check the force ultrapeer box in settings, so the program will have to pass additional tests below
	{
		// If we are a Gnutella2 hub, then we shouldn't also be a Gnutella ultrapeer
		if ( IsG2Hub() )
		{
			// ToDo: Check what sort of machine could handle being both a Gnutella ultrapeer and a Gnutella2 hub at the same time

			// Report the reason we can't be an ultrapeer, and return no
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: Acting as a G2 hub" );
			return FALSE;
		}
		else // We aren't a Gnutella2 hub right now
		{
			// Make a note we passed this test, and keep going
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"OK: not a G2 hub" );		// protocolNames[ PROTOCOL_G2 ]
		}

		// Note base physical memory check in CalculateSystemPerformanceScore

		// Check the connection speed, make sure the download speed is fast enough
		if ( Settings.Connection.InSpeed < 200 )	// If inbound speed is less than 200 kbps (do)
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: less than 200 Kb/s in" );
			return FALSE;
		}

		// Make sure the upload speed is fast enough
		if ( Settings.Connection.OutSpeed < 200 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: less than 200 Kb/s out" );
			return FALSE;
		}

		// Make sure settings aren't limiting the upload speed too much
		if ( Settings.Bandwidth.Uploads <= 4096 && Settings.Bandwidth.Uploads != 0 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: Upload limit set too low" );
			return FALSE;
		}

		// We can only become an ultrapeer if we've been connected for 4 hours or more, it takes awhile for ultrapeers to get leaves, so stability is important
		if ( ! bIgnoreTime )		// This is unhandled (never skipped)
		{
			if ( Network.GetStableTime() < 12000 )	// 14400 seconds is 4 hours, hedge 1200 for 3h20m
			{
				if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: not stable for 4 hours" );
				return FALSE;
			}
			else // Connected to Gnutella network for more than 4 hours
			{
				if ( bDebug ) theApp.Message( MSG_DEBUG, L"OK: stable for 4 hours" );
			}
		}

		// Make sure the datagram is stable (do)
		if ( Network.IsFirewalled(CHECK_UDP) )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"NO: datagram not stable" );
			return FALSE;
		}
		else // The datagram is stable (do)
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, L"OK: datagram stable" );
		}

		if ( bDebug ) theApp.Message( MSG_DEBUG, L"YES: Ultrapeer capable by test" );
	}

	// If we've made it this far, change the rating number from 0 to 1
	// The higher it is, the better an ultrapeer we can be
	nRating = 1 + CalculateSystemPerformanceScore( bDebug );

	// We'll be a better Gnutella ultrapeer if the program isn't connected to other networks

	if ( ! Settings.Gnutella2.Enabled )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"G2 not enabled" );			// protocolNames[ PROTOCOL_G2 ]
	}

	if ( ! Settings.eDonkey.Enabled )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"eDonkey not enabled" );		// protocolNames[ PROTOCOL_ED2K]
	}

	if ( ! Settings.DC.Enabled )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"DC++ not enabled" );			// protocolNames[ PROTOCOL_DC ]
	}

	if ( ! Settings.BitTorrent.Enabled )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"BitTorrent not enabled" );	// protocolNames[ PROTOCOL_BT ]
	}

	// If debug mode is enabled, display the ultrapeer rating in the system window log (do)
	if ( bDebug )
	{
		// Compose text that includes the rating, and show it in the user interface
		CString strRating;
		strRating.Format( L"Ultrapeer rating: %u", nRating );
		theApp.Message( MSG_DEBUG, strRating );
	}

	// Return 0 if we can't be a Gnutella ultrapeer, or 1+ if we can, a higher number means we'd be a better ultrapeer
	return nRating;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect connection capacity

// Counts how many connections to hubs we have for that protocol, and compares that number to settings
// Returns true if we need more hub connections, false if we have enough
bool CNeighboursWithConnect::NeedMoreHubs(PROTOCOLID nProtocol) const
{
	if ( ! Network.IsConnected() ) return false;

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		if ( ! Settings.Gnutella1.Enabled && ! Settings.Gnutella2.Enabled ) return false;
		break;
	case PROTOCOL_G1:
		if ( ! Settings.Gnutella1.Enabled ) return false;
		break;
	case PROTOCOL_G2:
		if ( ! Settings.Gnutella2.Enabled ) return false;
		break;
	default:
		return false;
	}

	// Make an array to count the number of hub connections we have for each network
	DWORD nConnected[PROTOCOL_LAST] = {};

	// Count the number of hubs we are connected to, by looping for each neighbour in the list
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		const CNeighbour* pNeighbour = GetNext( pos );

		// If we've finished the handshake with this neighbour, and our connection to it isn't down to a leaf
		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType != ntLeaf )
			nConnected[ pNeighbour->m_nProtocol ]++;	// Count it as one more for its network
	}

	// Compose the answer for the protocol the caller wants to know about
	switch ( nProtocol )
	{
	// Caller wants to know if we need more hubs for either Gnutella or Gnutella2
	case PROTOCOL_NULL:
		// If we need more Gnutella ultrapeers or Gnutella2 hubs, return true, only return false if we don't need more hubs from either network
		return nConnected[PROTOCOL_G1] < ( IsG1Leaf() ? Settings.Gnutella1.NumHubs : Settings.Gnutella1.NumPeers ) ||
			   nConnected[PROTOCOL_G2] < ( IsG2Leaf() ? Settings.Gnutella2.NumHubs : Settings.Gnutella2.NumPeers );

	// Return true if we need more Gnutella ultrapeer connections
	case PROTOCOL_G1:
		// If we're a leaf, compare our hub count to NumHubs from settings, return true if we don't have enough
		return nConnected[PROTOCOL_G1] < ( IsG1Leaf() ? Settings.Gnutella1.NumHubs : Settings.Gnutella1.NumPeers );	// 2 and 32 defaults

	// Return true if we need more Gnutella2 hub connections
	case PROTOCOL_G2:
		// If we're a leaf, compare our hub count to NumHubs from settings, return true if we don't have enough
		return nConnected[PROTOCOL_G2] < ( IsG2Leaf() ? Settings.Gnutella2.NumHubs : Settings.Gnutella2.NumPeers );		// 2 and 6 defaults
	}

	return false;
}

// Counts how many connections to leaves we have for that protocol, and compares that number to settings limit
// Returns true if we need more leaf connections, false if we have enough
bool CNeighboursWithConnect::NeedMoreLeafs(PROTOCOLID nProtocol) const
{
	if ( ! Network.IsConnected() ) return false;

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		if ( ! Settings.Gnutella1.Enabled && ! Settings.Gnutella2.Enabled ) return false;
		break;

	case PROTOCOL_G1:
		if ( ! Settings.Gnutella1.Enabled ) return false;
		break;

	case PROTOCOL_G2:
		if ( ! Settings.Gnutella2.Enabled ) return false;
		break;

	default:
		return false;
	}

	// Make an array to count the number of leaf connections we have for each network
	DWORD nConnected[ PROTOCOL_LAST ] = {};

	// Count the number of leaf connections we have, by looping for each neighbour in the list
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		const CNeighbour* pNeighbour = GetNext( pos );

		// If we've finished the handshake with this neighbour, and our connection to is down to a leaf
		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType == ntLeaf )
			nConnected[ pNeighbour->m_nProtocol ]++;	// Count it as one more for its network
	}

	// Compose the answer for the protocol the caller wants to know about
	switch ( nProtocol )
	{
	// The caller wants to know if we need more leaves for either Gnutella or Gnutella2
	case PROTOCOL_NULL:
		// If we need more Gnutella or Gnutella2 leaves, return true, only return false if we don't need more leaves from either network
		return nConnected[PROTOCOL_G1] < Settings.Gnutella1.NumLeafs ||
			   nConnected[PROTOCOL_G2] < Settings.Gnutella2.NumLeafs;

	// Return true if we need more Gnutella ultrapeer connections
	case PROTOCOL_G1:
		// Compare our leaf count to NumLeafs from settings, return true if we don't have enough
		return //IsG1UltrapeerCapable() &&
			nConnected[ PROTOCOL_G1 ] < Settings.Gnutella1.NumLeafs;	// Gnutella NumLeafs is 0 by default, we always have enough leaves

	// Return true if we need more Gnutella2 hub connections
	case PROTOCOL_G2:
		// Compare our leaf count to NumLeafs from settings, return true if we don't have enough
		return //IsG2HubCapable() &&
			nConnected[ PROTOCOL_G2 ] < Settings.Gnutella2.NumLeafs;	// Gnutella2 NumLeafs is ~400
	}

	return false;
}

// Determines if we are running at at least 75% of our capacity as a hub on the network
// Returns true if we are that busy, false if we have unused capacity
//bool CNeighboursWithConnect::IsHubLoaded(PROTOCOLID nProtocol) const
//{
//	if ( ! Network.IsConnected() ) return false;
//
//	switch ( nProtocol )
//	{
//	case PROTOCOL_NULL:
//		if ( ! Settings.Gnutella1.Enabled && ! Settings.Gnutella2.Enabled ) return false;
//		break;
//
//	case PROTOCOL_G1:
//		if ( ! Settings.Gnutella1.Enabled ) return false;
//		break;
//
//	case PROTOCOL_G2:
//		if ( ! Settings.Gnutella2.Enabled ) return false;
//		break;
//
//	default:
//		return false;
//	}
//
//	// Make an array to count connections for each network the program connects to
//	DWORD nConnected[ PROTOCOL_LAST ] = {};
//
//	// Count how many leaves are connected to us, by looping for each neighbour in the list
//	for ( POSITION pos = GetIterator() ; pos ; )
//	{
//		CNeighbour* pNeighbour = GetNext( pos );
//
//		// If we're done with the handshake for this neighbour, and the connection is down to a leaf
//		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType == ntLeaf )
//			nConnected[ pNeighbour->m_nProtocol ]++;	// Increment the count for this network
//	}
//
//	// Return information based on the protocl the caller wants to know about
//	switch ( nProtocol )
//	{
//	// The caller wants to know about Gntuella and Gnutella2 combined
//	case PROTOCOL_NULL:
//		// If the total connection number is bigger than 75% of the totals from settings, say yes
//		return nConnected[ PROTOCOL_G1 ] + nConnected[PROTOCOL_G2] >= ( Settings.Gnutella1.NumLeafs + Settings.Gnutella2.NumLeafs ) * 3 / 4;
//
//	// The caller wants to know about Gnutella
//	case PROTOCOL_G1:
//		// If we've got more than 75% of the leaf number from settings, say yes
//		return nConnected[ PROTOCOL_G1 ] > Settings.Gnutella1.NumLeafs * 3 / 4;
//
//	// The caller wants to know about Gnutella2
//	case PROTOCOL_G2:
//		// If we've got more than 75% of the leaf number from settings, say yes
//		return nConnected[ PROTOCOL_G2 ] > Settings.Gnutella2.NumLeafs * 3 / 4;
//	}
//
//	return false;
//}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect run event

// Call DoRun on each neighbour in the list, and maintain the network auto connection
void CNeighboursWithConnect::OnRun()
{
	// Call DoRun on each neighbour in the list, control goes to CNeighboursBase::OnRun()
	CNeighboursWithRouting::OnRun();

	if ( ! Network.m_pSection.Lock( 100 ) ) return;

	// Maintain the network (do)
	// Count how many connections of each type we have, calculate how many we should have, and close and open connections accordingly
	// Makes new connections by getting IP addresses from the host caches for each network

	MaintainNodeStatus();

	if ( Network.IsConnected() && Network.m_bAutoConnect )
		Maintain();

	Network.m_pSection.Unlock();
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect maintain connection

void CNeighboursWithConnect::MaintainNodeStatus()
{
	BOOL  bG2Leaf		= FALSE;
	BOOL  bG2Hub		= FALSE;
	BOOL  bG1Leaf		= FALSE;
	BOOL  bG1Ultrapeer	= FALSE;
	DWORD tEstablish	= GetTickCount() - 1500;
	DWORD nStableCount	= 0;
	DWORD nBandwidthIn	= 0;
	DWORD nBandwidthOut	= 0;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );

		// We're done with the handshake with this neighbour
		if ( pNeighbour->m_nState == nrsConnected )
		{
			pNeighbour->Measure();
			nBandwidthIn += pNeighbour->m_mInput.nMeasure;
			nBandwidthOut += pNeighbour->m_mOutput.nMeasure;

			if ( pNeighbour->m_tConnected < tEstablish )
				nStableCount++;

			// We're connected to this neighbour and exchanging Gnutella or Gnutella2 packets
			if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
			{
				// If our connection to this remote computer is up to a hub, we are a leaf, if it's down to a leaf, we are a hub
				if ( pNeighbour->m_nNodeType == ntHub )
					bG2Leaf = TRUE;
				else
					bG2Hub  = TRUE;
			}
			else if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
			{
				// If our connection to this remote computer is up to a hub, we are a leaf, if it's down to a leaf, we are an ultrapeer
				if ( pNeighbour->m_nNodeType == ntHub )
					bG1Leaf = TRUE;
				else
					bG1Ultrapeer = TRUE;
			}
		}
	}

	m_bG2Leaf		= bG2Leaf;
	m_bG2Hub		= bG2Hub;
	m_bG1Leaf		= bG1Leaf;
	m_bG1Ultrapeer	= bG1Ultrapeer;
	m_nStableCount	= nStableCount;
	m_nBandwidthIn	= nBandwidthIn;
	m_nBandwidthOut	= nBandwidthOut;
}

// As the program runs, CNetwork::OnRun calls this method periodically and repeatedly
// Counts how many connections we have for each network and in each role, and connects to more from the host cache or disconnects from some
void CNeighboursWithConnect::Maintain()
{
	// Get the time
	const DWORD tTimer = GetTickCount();						// Time in ticks (milliseconds)
	const DWORD tNow   = static_cast< DWORD >( time( NULL ) );	// Time in seconds

	// Don't initiate neighbour connections too quickly if connections are limited
	if ( Settings.Connection.ConnectThrottle && tTimer >= m_tLastConnect && tTimer <= m_tLastConnect + Settings.Connection.ConnectThrottle )
		return;

	DWORD nCount[ PROTOCOL_LAST ][3] = {}, nLimit[ PROTOCOL_LAST ][3] = {};

	// Loop down the list of connected neighbours, sorting each by network and role and counting it
	// Also prune leaf to leaf connections, which shouldn't happen
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		// Get the next neighbour in the list
		CNeighbour* pNeighbour = GetNext( pos );

		// We're done with the handshake and connected to this remote computer
		if ( pNeighbour->m_nState == nrsConnected )
		{
			if ( pNeighbour->m_nNodeType != ntHub && m_bG2Leaf && pNeighbour->m_nProtocol == PROTOCOL_G2 )
			{
				// We're both Gnutella2 leaves:
				// Two leaves shouldn't connect, disconnect
				pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
			}
			else if ( pNeighbour->m_nNodeType != ntHub && m_bG1Leaf && pNeighbour->m_nProtocol == PROTOCOL_G1 )
			{
				// We're both Gnutella leaves:
				// Two leaves shouldn't connect, disconnect
				pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
			}
			else if ( pNeighbour->m_nNodeType != ntLeaf )
			{
				// This connection is to a hub above us, or a hub just like us:
				// Count one more hub for this connection's protocol if we've been connected for several seconds.
				if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K || pNeighbour->m_nProtocol == PROTOCOL_DC || tTimer > pNeighbour->m_tConnected + 8000 )
					nCount[ pNeighbour->m_nProtocol ][ ntHub ]++;
				else
					nCount[ pNeighbour->m_nProtocol ][ ntNode ]++;
			}
			else
			{
				// We must be a hub, and this connection must be down to a leaf:
				// Count one more leaf for this connection's protocol
				nCount[ pNeighbour->m_nProtocol ][ ntLeaf ]++;
			}
		}
		else //if ( pNeighbour->m_nState < nrsConnected )
		{
			// We're still going through the handshake with this remote computer
			// Count one more connection in the 0 column for this protocol
			nCount[ pNeighbour->m_nProtocol ][ ntNode ]++;	// ntNode is 0
		}
	}

	// Set our "promoted to hub" timer
	if ( ! m_bG2Hub )
		m_tHubG2Promotion = 0;			// If we're not a hub, time promoted is 0
	else if ( m_tHubG2Promotion == 0 )
		m_tHubG2Promotion = tNow;		// If we've just been promoted, set the timer

	// Check if we have verified if we make a good G2 hub
	if ( ! Settings.Gnutella2.HubVerified && m_tHubG2Promotion > 0 && Network.IsConnected() )
	{
		// If we have been a hub for at least 8 hours
		if ( tNow > m_tHubG2Promotion + 8 * 60 * 60 )
		{
			// And we're loaded ( 75% capacity ), then we probably make a pretty good hub
			if ( nCount[ PROTOCOL_G2 ][ ntHub ] > ( Settings.Gnutella2.NumLeafs * 3 / 4 ) )
				Settings.Gnutella2.HubVerified = true;
		}
	}

	if ( ! Settings.Gnutella1.Enabled )
	{
		// Set the limit as no Gnutella hub or leaf connections allowed at all
		nLimit[ PROTOCOL_G1 ][ ntHub ] = nLimit[ PROTOCOL_G1 ][ ntLeaf ] = 0;
	}
	else if ( m_bG1Leaf )	// We're a leaf on the Gnutella network
	{
		nLimit[ PROTOCOL_G1 ][ ntHub ] = Settings.Gnutella1.NumHubs;		// 3 ultrapeers by default
	}
	else	// We're an ultrapeer on the Gnutella network
	{
		// Set the limit for Gnutella ultrapeer connections as whichever number from settings is bigger, peers or hubs
		nLimit[ PROTOCOL_G1 ][ ntHub ] = max( Settings.Gnutella1.NumPeers, Settings.Gnutella1.NumHubs );	// Defaults are 32 and 3

		// Set the limit for Gnutella leaf connections from settings
		nLimit[ PROTOCOL_G1 ][ ntLeaf ] = Settings.Gnutella1.NumLeafs;		// 50 leaves by default
	}

	if ( ! Settings.Gnutella2.Enabled )
	{
		// Set the limit as no Gnutella2 hub or leaf connections allowed at all
		nLimit[ PROTOCOL_G2 ][ ntHub ] = nLimit[ PROTOCOL_G2 ][ ntLeaf ] = 0;
	}
	else if ( m_bG2Leaf )	// We're a leaf on the Gnutella2 network
	{
		// Set the limit for Gnutella2 hub connections from settings, should be no more than 3
		nLimit[ PROTOCOL_G2 ][ ntHub ] = Settings.Gnutella2.NumHubs;		// 2 hubs by default
	}
	else	// We're a hub on the Gnutella2 network
	{
		// Set the limit for G2 hub connections as whichever number from settings is bigger, peers or hubs
		nLimit[ PROTOCOL_G2 ][ ntHub ] = max( Settings.Gnutella2.NumPeers, Settings.Gnutella2.NumHubs );	// Defaults are 6 and 2

		// Set the limit for G2 leaf connections from user settings
		nLimit[ PROTOCOL_G2 ][ ntLeaf ] = Settings.Gnutella2.NumLeafs;		// 300 leaves by default
	}

	nLimit[ PROTOCOL_ED2K ][ ntHub ] = Settings.eDonkey.Enabled ? Settings.eDonkey.NumServers : 0;		// 1 server by default

	nLimit[ PROTOCOL_DC ][ ntHub ] = Settings.DC.Enabled ? Settings.DC.NumServers : 0;					// 1 hub by default

	// Add the count of connections where we don't know the network yet to the 0 column of both Gnutella and Gnutella2
	nCount[ PROTOCOL_G1 ][ ntNode ] += nCount[ PROTOCOL_NULL ][ ntNode ];
	nCount[ PROTOCOL_G2 ][ ntNode ] += nCount[ PROTOCOL_NULL ][ ntNode ];

	// Connect to more computers or disconnect from some to get the connection counts where settings wants them to be
	for ( int nProtocol = PROTOCOL_NULL ; nProtocol < PROTOCOL_LAST ; ++nProtocol )		// Loop once for each protocol
	{
		// If we're connected to a hub of this protocol, store the tick count now in m_tPresent for this protocol
		if ( nCount[ nProtocol ][ ntHub ] > 0 ) m_tPresent[ nProtocol ] = tNow;

		// If we don't have enough hubs for this protocol
		if ( nCount[ nProtocol ][ ntHub ] < nLimit[ nProtocol ][ ntHub ] )
		{
			// Don't try to connect to G1 right away, wait a few seconds to reduce the number of connections
			if ( nProtocol != PROTOCOL_G2 && Settings.Gnutella2.Enabled )
			{
				if ( ! Network.ReadyToTransfer( tTimer ) ) return;
			}

			// We are going to try to connect to a computer running Gnutella or Gnutella2 software
			DWORD nAttempt;
			if ( nProtocol == PROTOCOL_ED2K )
			{
				// For ed2k we try one attempt at a time to begin with, but we can step up to
				// 2 at a time after a few seconds if the FastConnect option is selected.
				if ( Settings.eDonkey.FastConnect && Network.ReadyToTransfer( tTimer ) )
					nAttempt = 2;
				else
					nAttempt = 1;
			}
			else if ( nProtocol == PROTOCOL_DC )
			{
				// DC++ Slow connecting
				nAttempt = 1;
			}
			else
			{
				// For Gnutella and Gnutella2, try connection to the number of free slots multiplied by the connect factor from settings
				nAttempt = ( nLimit[ nProtocol ][ ntHub ] - nCount[ nProtocol ][ ntHub ] );
				nAttempt *= Settings.Gnutella.ConnectFactor;
			}

			// Lower the needed hub number to avoid hitting Windows XP Service Pack 2's half open connection limit
			nAttempt = min( nAttempt, ( Settings.Downloads.MaxConnectingSources - 1 ) );

			CHostCacheList* pCache = HostCache.ForProtocol( (PROTOCOLID)nProtocol );

			CSingleLock oLock( &pCache->m_pSection, FALSE );
			if ( ! oLock.Lock( 250 ) )
				return;

			// Handle priority servers
			// Loop into the host cache until we have as many handshaking connections as we need hub connections
			for ( CHostCacheIterator i = pCache->Begin() ;
				i != pCache->End() && nCount[ nProtocol ][ ntNode ] < nAttempt ;
				++i )
			{
				CHostCacheHostPtr pHost = (*i);

				// If we can connect to this priority host, try it
				if ( pHost->m_bPriority &&
					 pHost->CanConnect( tNow ) &&
					 pHost->ConnectTo( TRUE ) )
				{
					m_tPriority[ nProtocol ] = tNow;

					pHost->m_nFailures = 0;
					pHost->m_tFailure = 0;
					pHost->m_bCheckedLocally = TRUE;

					// Count that we now have one more connection, and we don't know its network role yet
					nCount[ nProtocol ][ ntNode ]++;

					// Prevent queries while we connect with this computer (do)
					pHost->m_tQuery = tNow;

					// If settings wants to limit how frequently this method can run
					if ( Settings.Connection.ConnectThrottle )
					{
						m_tLastConnect = tTimer;
						return;
					}
				}
			}

			// 10 second delay between priority and regular servers
			if ( tNow > m_tPriority[ nProtocol ] + 10 )
			{
				// Handle regular servers, if we need more connections for this network, get IP addresses from the host cache and try to connect to them
				for ( CHostCacheIterator i = pCache->Begin() ;
					i != pCache->End() && nCount[ nProtocol ][ ntNode ] < nAttempt ;
					++i )
				{
					CHostCacheHostPtr pHost = (*i);

					// If we can connect to this IP address from the host cache, try it
					if ( ! pHost->m_bPriority &&
						pHost->CanConnect( tNow ) &&
						pHost->ConnectTo( TRUE ) )
					{
						// Make sure the connection we just made matches the protocol we're looping for right now
						//ASSERT( pHost->m_nProtocol == nProtocol );
						pHost->m_nFailures = 0;
						pHost->m_tFailure = 0;
						pHost->m_bCheckedLocally = TRUE;

						// Count that we now have one more handshaking connection for this network
						nCount[ nProtocol ][ ntNode ]++;

						// Prevent queries while we log on (do)
						pHost->m_tQuery = tNow;

						// If settings wants to limit how frequently this method can run
						if ( Settings.Connection.ConnectThrottle )
						{
							m_tLastConnect = tTimer;
							return;
						}
					}
				}
			}

			// If we don't have any handshaking connections for this network, and we've been connected to a hub for more than 30 seconds
			if ( nCount[ nProtocol ][ ntNode ] == 0 ||			// We don't have any handshaking connections for this network, or
				 tNow > m_tPresent[ nProtocol ] + 30 )			// We've been connected to a hub for more than 30 seconds
			{
				const DWORD tDiscoveryLastExecute = DiscoveryServices.LastExecute();

				if ( nProtocol == PROTOCOL_G2 && Settings.Gnutella2.Enabled )
				{
					// We're looping for Gnutella2 right now
					// Execute the discovery services (do)
					if ( pCache->IsEmpty() && tNow >= tDiscoveryLastExecute + 8 )
						DiscoveryServices.Execute( TRUE, PROTOCOL_G2, 1 );
				}
				else if ( nProtocol == PROTOCOL_G1 && Settings.Gnutella1.Enabled )
				{
					// We're looping for Gnutella right now
					// If the Gnutella host cache is empty (do), execute discovery services (do)
					if ( pCache->IsEmpty() && tNow >= tDiscoveryLastExecute + 8 )
						DiscoveryServices.Execute( TRUE, PROTOCOL_G1, 1 );
				}
			}
		}
		else if ( nCount[ nProtocol ][ ntHub ] > nLimit[ nProtocol ][ ntHub ] ) 	// We're over the limit we just calculated
		{
			// Otherwise we have too many hub connections for this protocol,
			// so find the hub we connected to most recently to remove it.
			CNeighbour* pNewest = NULL;
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				// Loop through the list of neighbours
				CNeighbour* pNeighbour = GetNext( pos );

				// If this is a hub connection that connected to us recently
				if ( ( pNeighbour->m_nNodeType != ntLeaf ) &&		// If this connection isn't down to a leaf, and
					 ( pNeighbour->m_nProtocol == nProtocol ) &&	// This connection is for the protocol we're looping on right now, and
					 ( pNeighbour->m_bAutomatic ||					// The neighbour is automatic, or
					  ! pNeighbour->m_bInitiated ||					// The neighbour connected to us, or
					  nLimit[ nProtocol ][ ntHub ] == 0 ) ) 		// We're not supposed to be connected to this network at all
				{
					// If this is the newest hub, remember it.
					if ( pNewest == NULL || pNeighbour->m_tConnected > pNewest->m_tConnected )
						pNewest = pNeighbour;
				}
			}

			// Disconnect from one hub
			if ( pNewest ) pNewest->Close();	// Close the connection
		}

		// If we're over our leaf connection limit for this network
		if ( nCount[ nProtocol ][ ntLeaf ] > nLimit[ nProtocol ][ ntLeaf ] )
		{
			// Find the leaf we most recently connected to
			CNeighbour* pNewest = NULL;
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				// Loop for each neighbour in the list
				CNeighbour* pNeighbour = GetNext( pos );

				// This connection is down to a leaf and the protocol is correct
				if ( pNeighbour->m_nNodeType == ntLeaf && pNeighbour->m_nProtocol == nProtocol )
				{
					// If we haven't found the newest yet, or this connection is younger than the current newest, this is it
					if ( pNewest == NULL || pNeighbour->m_tConnected > pNewest->m_tConnected )
						pNewest = pNeighbour;
				}
			}

			// Disconnect from one leaf
			if ( pNewest ) pNewest->Close();	// Close the connection
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Hub Selection Rating "Points":

DWORD CNeighboursWithConnect::CalculateSystemPerformanceScore(BOOL bDebug) const
{
	DWORD nRating = 0;

	MEMORYSTATUSEX pMemory = { sizeof( MEMORYSTATUSEX ) };
	GlobalMemoryStatusEx( &pMemory );

	if ( pMemory.ullTotalPhys > 1000 * 1024 * 1024 )		// Computer has at least 1 GB of memory
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"More than 1 GB RAM" );
	}
	else if ( pMemory.ullTotalPhys < 500 * 1024 * 1024 )	// Computer has less than 512 MB of memory
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"Less than 512 MB RAM" );
		Settings.Gnutella2.ClientMode = MODE_LEAF;			// Skip future auto hub promotion checks
		Settings.Gnutella1.ClientMode = MODE_LEAF;
		return 0;
	}

	if ( pMemory.ullAvailPhys > 800 * 1024 * 1024 )			// Computer has at least ~1 GB of free memory
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"More than 800 MB free RAM" );
	}
	else if ( pMemory.ullAvailPhys < 20 * 1024 * 1024 )		// Computer has no free memory
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"No free RAM" );
		return 0;
	}

	// Our Internet connection allows fast downloads
	if ( Settings.Connection.InSpeed > 1000 )	// More than 1 Mbps inbound (do)
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"More than 1 Mb/s in" );
	}

	// Our Internet connection allows fast uploads
	if ( Settings.Connection.OutSpeed > 1000 )	// More than 1 Mbps outbound (do)
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"More than 1 Mb/s out" );
	}

	// If the program has been connected (do) for more than 8 hours, give it another point
	if ( Network.GetStableTime() > 28800 )		// 8 hours uptime in seconds
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"Stable for 8 hours" );
	}

	// If the scheduler isn't enabled, award another point
	if ( ! Settings.Scheduler.Enable )
	{
		nRating++;
	}
	//else if ( Scheduler.GetHoursTo( BANDWIDTH_STOP|SYSTEM_DISCONNECT|SYSTEM_EXIT|SYSTEM_SHUTDOWN ) > 6 )
	//{
	//	nRating++;
	//	if ( bDebug ) theApp.Message( MSG_DEBUG, L"Scheduler won't disconnect in the next 6 hours" );
	//}

	// The user has disabled BitTorrent, so that won't be taking up any bandwidth
	if ( ! Settings.BitTorrent.Enabled || ! Settings.BitTorrent.EnableAlways )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"BT is not in use" );
	}

	// Having more CPUs has significant effect on performance
	if ( System.dwNumberOfProcessors > 1 )
	{
		nRating += System.dwNumberOfProcessors / 2;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"%u Processors", System.dwNumberOfProcessors );
	}

	// 64-bit benefit
#ifdef WIN64
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, L"Mode: 64-bit" );
	}
#endif	// x64

	// ToDo: Find out more about the computer to award more rating points
	// CPU: Add a CPU check, award a point if this computer has a fast processor
	// Router: Some routers can't handle the 100+ socket connections an ultrapeer maintains, check for a router and lower the score
	// File transfer: Check how many files are shared and the file transfer in both directions, the perfect ultrapeer isn't sharing or doing any file transfer at all

	return nRating;
}
