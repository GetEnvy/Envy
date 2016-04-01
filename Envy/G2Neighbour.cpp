//
// G2Neighbour.cpp
//
// This file is part of Envy (getenvy.com) © 2016
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "Neighbours.h"
#include "G2Neighbour.h"
#include "G2Packet.h"
#include "G1Packet.h"
#include "Buffer.h"
#include "Network.h"
#include "Security.h"
#include "Statistics.h"
#include "Datagrams.h"
#include "Handshakes.h"
#include "HostCache.h"
#include "RouteCache.h"
#include "VendorCache.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "Library.h"
#include "LocalSearch.h"
#include "SearchManager.h"
#include "QueryHashTable.h"
#include "HubHorizon.h"
#include "GProfile.h"
#include "Uploads.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CG2Neighbour construction

CG2Neighbour::CG2Neighbour(CNeighbour* pBase)
	: CNeighbour( PROTOCOL_G2, pBase )
	, m_bCachedKeys			( FALSE )
	, m_pGUIDCache			( new CRouteCache() )
	, m_pHubGroup			( new CHubHorizonGroup() )
	, m_nLeafCount			( 0 )
	, m_nLeafLimit			( 0 )
	, m_tLastRun			( 0 )
	, m_tAdjust				( 0 )
	, m_tLastPingIn			( 0 )
	, m_tLastPingOut		( 0 )
	, m_nCountPingIn		( 0 )
	, m_nCountPingOut		( 0 )
	, m_tLastRelayPingIn	( 0 )
	, m_tLastRelayPingOut	( 0 )
	, m_nCountRelayPingIn	( 0 )
	, m_nCountRelayPingOut	( 0 )
	, m_tLastRelayedPingIn	( 0 )
	, m_tLastRelayedPingOut	( 0 )
	, m_nCountRelayedPingIn	( 0 )
	, m_nCountRelayedPingOut ( 0 )
	, m_tLastKHLIn			( 0 )
	, m_tLastKHLOut			( 0 )
	, m_nCountKHLIn			( 0 )
	, m_nCountKHLOut		( 0 )
	, m_tLastLNIIn			( 0 )
	, m_tLastLNIOut			( 0 )
	, m_nCountLNIIn			( 0 )
	, m_nCountLNIOut		( 0 )
	, m_tLastHAWIn			( 0 )
	, m_tLastHAWOut			( 0 )
	, m_nCountHAWIn			( 0 )
	, m_nCountHAWOut		( 0 )
	, m_tLastQueryIn		( 0 )
	, m_nCountQueryIn		( 0 )
{
	theApp.Message( MSG_INFO, IDS_HANDSHAKE_ONLINE_G2, (LPCTSTR)m_sAddress, m_sUserAgent.IsEmpty() ? L"Unknown" : (LPCTSTR)m_sUserAgent );

	SendStartups();
}

CG2Neighbour::~CG2Neighbour()
{
	delete m_pHubGroup;
	delete m_pGUIDCache;

	for ( POSITION pos = m_pOutbound.GetHeadPosition() ; pos ; )
	{
		CG2Packet* pOutbound = m_pOutbound.GetNext( pos );
		pOutbound->Release();
	}
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour read and write events

BOOL CG2Neighbour::OnRead()
{
	CNeighbour::OnRead();

	return ProcessPackets();
}

BOOL CG2Neighbour::OnWrite()
{
	CLockedBuffer pOutputLocked( GetOutput() );

	CBuffer* pOutput = m_pZOutput ? m_pZOutput : (CBuffer*)pOutputLocked;

	while ( pOutput->m_nLength == 0 && m_nOutbound > 0 )
	{
		CG2Packet* pPacket = m_pOutbound.RemoveHead();
		m_nOutbound--;

		pPacket->ToBuffer( pOutput );
		pPacket->Release();

		CNeighbour::OnWrite();
	}

	CNeighbour::OnWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour run event

BOOL CG2Neighbour::OnRun()
{
	if ( ! CNeighbour::OnRun() )
		return FALSE;

	const DWORD tNow = GetTickCount();

	// Check incoming LNI traffic
	if ( m_nCountLNIIn == 0 && tNow > m_tConnected + ( Settings.Gnutella2.LNIPeriod * 3 ) )
	{
		// No LNI packet was received during 3 periods (dead or anonymous host)
		Close( IDS_CONNECTION_TIMEOUT_TRAFFIC );
		return FALSE;
	}

	// Is it time to send TCP ping? Don't ping neighbour if we recently got any packets.
	if ( tNow >= m_tLastPingOut + Settings.Gnutella2.PingRate &&
		 tNow >= m_tLastPacket + Settings.Connection.TimeoutTraffic / 2 )
	{
		Send( CG2Packet::New( G2_PACKET_PING ) );
		m_tLastPingOut = tNow;
		m_nCountPingOut++;
	}

	// We are unsure in our UDP capabilities therefore
	// we perform limited "two hop" ping ourself using this neighbour
	if ( m_nCountRelayPingOut < 3 &&
		 tNow >= m_tLastRelayPingOut + Settings.Gnutella2.PingRate &&
		 Network.IsListening() && Network.IsFirewalled(CHECK_UDP) )
	{
		CG2Packet* pPing = CG2Packet::New( G2_PACKET_PING, TRUE );
		pPing->WritePacket( G2_PACKET_UDP, 6 );
		pPing->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pPing->WriteShortBE( htons( Network.m_pHost.sin_port ) );
		Send( pPing );
		m_tLastRelayPingOut = tNow;
		m_nCountRelayPingOut++;
	}

	// Is it time to send LNI?
	if ( tNow > m_tLastLNIOut + Settings.Gnutella2.LNIPeriod )
		SendLNI();

	// Is it time to send KHL?
	if ( tNow > m_tLastKHLOut + ( Settings.Gnutella2.KHLPeriod * ( Neighbours.IsG2Leaf() ? 3 : 1 ) ) )
		SendKHL();

	// Is it time to send HAW?
	if ( tNow > m_tLastHAWOut + Settings.Gnutella2.HAWPeriod &&
		 m_nNodeType == ntNode && ! Neighbours.IsG2Leaf() &&
		 ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() ) )
		SendHAW();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour send packet

BOOL CG2Neighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL bBuffered)
{
	BOOL bSuccess = FALSE;

	if ( m_nState >= nrsConnected && pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		m_nOutputCount++;
		Statistics.Current.Gnutella2.Outgoing++;

		if ( bBuffered )
		{
			if ( m_nOutbound >= Settings.Gnutella1.PacketBufferSize )
			{
				CG2Packet* pRemove = m_pOutbound.RemoveTail();
				pRemove->Release();
				m_nOutbound--;
				Statistics.Current.Gnutella2.Lost++;
			}

			pPacket->AddRef();
			m_pOutbound.AddHead( static_cast< CG2Packet* >( pPacket ) );
			m_nOutbound++;
		}
		else
		{
			if ( m_pZOutput )
				pPacket->ToBuffer( m_pZOutput );
			else
				Write( pPacket );
		}

		QueueRun();

		pPacket->SmartDump( &m_pHost, FALSE, TRUE, (DWORD_PTR)this );

		bSuccess = TRUE;
	}

	if ( bRelease )
		pPacket->Release();

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour startup events

void CG2Neighbour::SendStartups()
{
	CG2Packet* pPing = CG2Packet::New( G2_PACKET_PING, TRUE );

	if ( Network.IsListening() )
	{
		pPing->WritePacket( G2_PACKET_UDP, 6 );
		pPing->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pPing->WriteShortBE( htons( Network.m_pHost.sin_port ) );
	}

	Send( pPing, TRUE, TRUE );
	m_tLastPingOut = GetTickCount();

	Datagrams.Send( &m_pHost, CG2Packet::New( G2_PACKET_PING ) );

	SendLNI();
	Send( CG2Packet::New( G2_PACKET_PROFILE_CHALLENGE ), TRUE, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour packet dispatch

BOOL CG2Neighbour::ProcessPackets()
{
	CLockedBuffer pInputLocked( GetInput() );

	CBuffer* pInput = m_pZInput ? m_pZInput : (CBuffer*)pInputLocked;

	return ProcessPackets( pInput );
}

BOOL CG2Neighbour::ProcessPackets(CBuffer* pInput)
{
	if ( ! pInput )
		return FALSE;

	BOOL bSuccess = TRUE;
	while ( bSuccess && pInput->m_nLength )
	{
		const BYTE nInput = *(pInput->m_pBuffer);

		if ( nInput == 0 )
		{
			pInput->Remove( 1 );
			continue;
		}

		BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
		BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
		BYTE nFlags		= ( nInput & 0x07 );

		if ( pInput->m_nLength < nLenLen + nTypeLen + 2ul )
			break;

		DWORD nLength = 0;

		if ( nFlags & G2_FLAG_BIG_ENDIAN )
		{
			BYTE* pLenIn = pInput->m_pBuffer + 1;

			for ( BYTE nIt = nLenLen ; nIt ; nIt-- )
			{
				nLength <<= 8;
				nLength |= *pLenIn++;
			}
		}
		else
		{
			BYTE* pLenIn	= pInput->m_pBuffer + 1;
			BYTE* pLenOut	= (BYTE*)&nLength;
			for ( BYTE nLenCnt = nLenLen ; nLenCnt-- ; )
			{
				*pLenOut++ = *pLenIn++;
			}
		}

		if ( nLength >= Settings.Gnutella.MaximumPacket )
		{
			Close( IDS_PROTOCOL_TOO_LARGE );
			return FALSE;
		}

		if ( pInput->m_nLength < nLength + nLenLen + nTypeLen + 2ul )
			break;

		CG2Packet* pPacket = CG2Packet::New( pInput->m_pBuffer );

		try
		{
			bSuccess = OnPacket( pPacket );
		}
		catch ( CException* pException )
		{
			pException->Delete();
			// bSuccess = FALSE;	// ToDo: Should this be like this?
			bSuccess = TRUE;
		}

		pPacket->Release();

		pInput->Remove( nLength + nLenLen + nTypeLen + 2 );
	}

	if ( ! bSuccess )
		Close( 0 );

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour packet handler

BOOL CG2Neighbour::OnPacket(CG2Packet* pPacket)
{
	m_nInputCount++;
	m_tLastPacket = GetTickCount();
	Statistics.Current.Gnutella2.Incoming++;

	pPacket->SmartDump( &m_pHost, FALSE, FALSE, (DWORD_PTR)this );

	if ( Network.RoutePacket( pPacket ) )
		return TRUE;

	switch ( pPacket->m_nType )
	{
	case G2_PACKET_PING:
		return OnPing( pPacket );
	case G2_PACKET_PONG:
		return OnPong( pPacket );
	case G2_PACKET_LNI:
		return OnLNI( pPacket );
	case G2_PACKET_KHL:
		return OnKHL( pPacket );
	case G2_PACKET_HAW:
		return OnHAW( pPacket );
	case G2_PACKET_QUERY:
		return OnQuery( pPacket );
	case G2_PACKET_QUERY_WRAP:
		break;		// G2_PACKET_QUERY_WRAP deprecated and ignored
	case G2_PACKET_HIT:
		return OnCommonHit( pPacket );
	case G2_PACKET_HIT_WRAP:
		return OnCommonHit( pPacket );
	case G2_PACKET_QUERY_ACK:
		return OnQueryAck( pPacket );
	case G2_PACKET_QUERY_KEY_REQ:
		return OnQueryKeyReq( pPacket );
	case G2_PACKET_QUERY_KEY_ANS:
		return OnQueryKeyAns( pPacket );
	case G2_PACKET_QHT:
		return OnCommonQueryHash( pPacket );
	case G2_PACKET_PUSH:
		return OnPush( pPacket );
	case G2_PACKET_PROFILE_CHALLENGE:
		return OnProfileChallenge( pPacket );
	case G2_PACKET_PROFILE_DELIVERY:
		return OnProfileDelivery( pPacket );
	default:
		theApp.Message( MSG_DEBUG, L"TCP: Received unexpected packet %s from %s",
			pPacket->GetType(), (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour PING packet handler

BOOL CG2Neighbour::OnPing(CG2Packet* pPacket, BOOL bTCP)
{
	Statistics.Current.Gnutella2.PingsReceived++;

	const DWORD tNow = GetTickCount();
	DWORD nAddress = 0;
	WORD nPort = 0;
	BOOL bRelay = FALSE;
	BOOL bUDP = FALSE;

	if ( pPacket->m_bCompound )
	{
		G2_PACKET nType;
		DWORD nLength;
		while ( pPacket->ReadPacket( nType, nLength ) )
		{
			DWORD nNext = pPacket->m_nPosition + nLength;
			if ( nType == G2_PACKET_UDP && nLength >= 6 )
			{
				nAddress	= pPacket->ReadLongLE();
				nPort		= pPacket->ReadShortBE();
				bUDP		= TRUE;
			}
			else if ( nType == G2_PACKET_RELAY )
			{
				bRelay = TRUE;
			}
			pPacket->m_nPosition = nNext;
		}
	}

	if ( ! bUDP )
	{
		// Skip "/PI/RELAY without /PI/UDP" error packet
		if ( bRelay )
			return TRUE;

		// This is a direct ping packet
		if ( tNow < m_tLastPingIn + Settings.Gnutella1.PingFlood )
			return TRUE;	// We are flooded
		m_tLastPingIn = tNow;
		m_nCountPingIn++;
		if ( bTCP )
			Send( CG2Packet::New( G2_PACKET_PONG ) );
		else
			Datagrams.Send( &m_pHost, CG2Packet::New( G2_PACKET_PONG ) );
		Statistics.Current.Gnutella2.PongsSent++;

		return TRUE;
	}
	else if ( ! nPort ||
		 Network.IsFirewalledAddress( (IN_ADDR*)&nAddress ) ||
		 Network.IsReserved( (IN_ADDR*)&nAddress ) ||
		 Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		// Invalid /PI/UDP address
		return TRUE;
	}
	else if ( bRelay && bTCP )
	{
		// This is a TCP relayed ping packet
		if ( tNow < m_tLastRelayedPingIn + Settings.Gnutella1.PingFlood )
			return TRUE;	// We are flooded
		m_tLastRelayedPingIn = tNow;
		m_nCountRelayedPingIn++;

		CG2Packet* pPong = CG2Packet::New( G2_PACKET_PONG, TRUE );
		pPong->WritePacket( G2_PACKET_RELAY, 0 );
		Datagrams.Send( (IN_ADDR*)&nAddress, nPort, pPong );
		Statistics.Current.Gnutella2.PongsSent++;

		return TRUE;
	}
	else if ( ! bRelay && bTCP )
	{
		// This is a TCP relayed ping request packet
		if ( tNow < m_tLastRelayPingIn + Settings.Gnutella1.PingFlood )
			return TRUE;	// We are flooded
		m_tLastRelayPingIn = tNow;
		m_nCountRelayPingIn++;

		BYTE* pRelay = pPacket->WriteGetPointer( 7, 0 );
		if ( pRelay == NULL )
		{
		//	theApp.Message( MSG_DEBUG, L"Memory allocation error in CG2Neighbour::OnPing()" );
			return TRUE;
		}

		*pRelay++ = 0x60;
		*pRelay++ = 0;
		*pRelay++ = 'R';
		*pRelay++ = 'E';
		*pRelay++ = 'L';
		*pRelay++ = 'A';
		*pRelay++ = 'Y';

		CArray< CG2Neighbour* > pG2Nodes;

		CSingleLock pLock( &Network.m_pSection );
		if ( SafeLock( pLock ) )
		{
			for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
			{
				CNeighbour* pNeighbour = Neighbours.GetNext( pos );
				if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
				{
					CG2Neighbour* pNeighbour2 = static_cast< CG2Neighbour* >( pNeighbour );
					if ( pNeighbour2->m_nState == nrsConnected &&
						 pNeighbour2 != this &&
						 ! pNeighbour2->m_bFirewalled &&
						 tNow - pNeighbour2->m_tLastRelayedPingOut >= Settings.Gnutella2.PingRate )
					{
						pG2Nodes.Add( pNeighbour2 );
					}
				}
			}
			pLock.Unlock();
		}

		INT_PTR nCount( pG2Nodes.GetCount() );
		if ( nCount > Settings.Gnutella2.PingRelayLimit )
			nCount = Settings.Gnutella2.PingRelayLimit;

		for ( INT_PTR nCur( 0 ) ; nCur < nCount ; ++nCur )
		{
			INT_PTR nRand( GetRandomNum< INT_PTR >( 0, pG2Nodes.GetCount() - 1 ) );

			CG2Neighbour* pNeighbour( pG2Nodes.GetAt( nRand ) );
			pNeighbour->Send( pPacket, FALSE );
			pNeighbour->m_tLastRelayedPingOut = tNow;
			++pNeighbour->m_nCountRelayedPingOut;
			++Statistics.Current.Gnutella2.PingsSent;
			pG2Nodes.RemoveAt( nRand );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour PONG packet handler

BOOL CG2Neighbour::OnPong(CG2Packet* pPacket, BOOL bTCP)
{
	Statistics.Current.Gnutella2.PongsReceived++;

	BOOL bRelayed = FALSE;
	if ( pPacket->m_bCompound )
	{
		G2_PACKET nType;
		DWORD nLength;
		while ( pPacket->ReadPacket( nType, nLength ) )
		{
			const DWORD nOffset = pPacket->m_nPosition + nLength;
			if ( nType == G2_PACKET_RELAY )
				bRelayed = TRUE;
			pPacket->m_nPosition = nOffset;
		}
	}

	if ( bRelayed && ! bTCP && ! Network.IsConnectedTo( &m_pHost.sin_addr ) )
		Datagrams.SetStable();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour LOCAL NODE INFO : send

void CG2Neighbour::SendLNI()
{
	Send( CreateLNIPacket( this ), TRUE, FALSE );

	m_tLastLNIOut = GetTickCount();
	m_nCountLNIOut ++;
}

CG2Packet* CG2Neighbour::CreateLNIPacket(CG2Neighbour* pOwner)
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_LNI, TRUE );

	QWORD nMyVolume = 0;
	DWORD nMyFiles = 0;
	LibraryMaps.GetStatistics( &nMyFiles, &nMyVolume );

	WORD nLeafs = 0;

	CSingleLock pLock( &Network.m_pSection );
	if ( SafeLock( pLock ) )
	{
		for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );

			if ( pNeighbour != pOwner &&
				 pNeighbour->m_nState == nrsConnected &&
				 pNeighbour->m_nNodeType == ntLeaf )
			{
				nMyFiles  += pNeighbour->m_nFileCount;
				nMyVolume += pNeighbour->m_nFileVolume;
				nLeafs++;
			}
		}
		pLock.Unlock();
	}

	pPacket->WritePacket( G2_PACKET_NODE_ADDRESS, 6 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );

	pPacket->WritePacket( G2_PACKET_NODE_GUID, 16 );
	pPacket->Write( Hashes::Guid( MyProfile.oGUID ) );

	pPacket->WritePacket( G2_PACKET_VENDOR, 4 );
	pPacket->WriteString( VENDOR_CODE, FALSE );

	pPacket->WritePacket( G2_PACKET_LIBRARY_STATUS, 8 );
	pPacket->WriteLongBE( (DWORD)nMyFiles );
	pPacket->WriteLongBE( (DWORD)nMyVolume );
	if ( Network.IsFirewalled() )
	{
		theApp.Message( MSG_DEBUG, L"Sending LNI/FW" );
		pPacket->WritePacket( G2_PACKET_FW, 0 );
	}

	if ( ! Neighbours.IsG2Leaf() )
	{
		pPacket->WritePacket( G2_PACKET_HUB_STATUS, 4 );
		pPacket->WriteShortBE( nLeafs );
		pPacket->WriteShortBE( WORD( Settings.Gnutella2.NumLeafs ) );

		pPacket->WritePacket( G2_PACKET_QUERY_KEY, 0 );
	}

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour LOCAL NODE INFO : receive

BOOL CG2Neighbour::OnLNI(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	m_tLastLNIIn = GetTickCount();
	m_nCountLNIIn ++;

	G2_PACKET nType;
	DWORD nLength;
	m_bFirewalled = FALSE;	// Node may report /LNI/FW flag initially and later discover firewall status and not send it.

	m_nLeafCount = 0;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		switch ( nType )
		{
		case G2_PACKET_NODE_ADDRESS:
			if ( nLength >= 6 )
			{
				DWORD nAddress = pPacket->ReadLongLE();
				WORD nPort = pPacket->ReadShortBE();
				if ( nPort && nAddress )	// Skip 0.0.0.0:0
				{
					m_pHost.sin_addr.s_addr = nAddress;
					m_pHost.sin_port = htons( nPort );
				}
			}
			break;
		case G2_PACKET_NODE_GUID:
			if ( nLength >= 16 )
				pPacket->Read( m_oGUID );
			break;
		case G2_PACKET_VENDOR:
			if ( nLength >= 4 )
			{
				CHAR szVendor[5] = { 0, 0, 0, 0, 0 };
				pPacket->Read( szVendor, 4 );
				m_pVendor = VendorCache.Lookup( szVendor );
				if ( ! m_pVendor && m_sServerName.IsEmpty() )	// Unknown
					m_sServerName = szVendor;
			}
			break;
		case G2_PACKET_LIBRARY_STATUS:
			if ( nLength >= 8 )
			{
				m_nFileCount	= pPacket->ReadLongBE();
				m_nFileVolume	= pPacket->ReadLongBE();
			}
			break;
		case G2_PACKET_HUB_STATUS:
			if ( nLength >= 2 )
			{
				m_nLeafCount = pPacket->ReadShortBE();
				if ( nLength >= 4 )
					m_nLeafLimit = pPacket->ReadShortBE();
			}
			break;
		case G2_PACKET_FW:
			if ( nLength == 0 )
			{
				theApp.Message( MSG_INFO, L"Received /LNI/FW from %s:%lu",
					(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), htons( m_pHost.sin_port ) );

				if ( m_nNodeType == ntHub )
					theApp.Message( MSG_ERROR, L"Hub %s:%lu sent LNI with firewall flag",
						(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), htons( m_pHost.sin_port ) );

				m_bFirewalled = TRUE;
			}
			break;
		case G2_PACKET_QUERY_KEY:
			m_bCachedKeys = TRUE;
			break;
		}

		pPacket->m_nPosition = nNext;
	}

	if ( m_pVendor != NULL && m_nNodeType != ntLeaf )
		HostCache.Gnutella2.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ), 0, m_pVendor->m_sCode );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour KNOWN HUB LIST : send

void CG2Neighbour::SendKHL()
{
	Send( CreateKHLPacket( this ), TRUE, TRUE );

	m_tLastKHLOut = GetTickCount();
	m_nCountKHLOut ++;
}

CG2Packet* CG2Neighbour::CreateKHLPacket(CG2Neighbour* pOwner)
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_KHL, TRUE );
	int nCount = Settings.Gnutella2.KHLHubCount;
	const DWORD tNow = static_cast< DWORD >( time( NULL ) );
	//BOOL bIsHub = ( ! Neighbours.IsG2Leaf() ) && ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() );

	CSingleLock pLock( &Network.m_pSection );
	if ( SafeLock( pLock ) )
	{
		for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
		{
			CG2Neighbour* pNeighbour = (CG2Neighbour*)Neighbours.GetNext( pos );

			if ( pNeighbour != pOwner &&
				 pNeighbour->m_nProtocol == PROTOCOL_G2 &&
				 pNeighbour->m_nState == nrsConnected &&
				 pNeighbour->m_nNodeType != ntLeaf &&
				! Network.IsSelfIP( pNeighbour->m_pHost.sin_addr ) )
			{
				if ( pNeighbour->m_pVendor && pNeighbour->m_pVendor->m_sCode.GetLength() == 4 )
				{
					pPacket->WritePacket( G2_PACKET_NEIGHBOUR_HUB, 16 + 6, TRUE );	// 4
					pPacket->WritePacket( G2_PACKET_HUB_STATUS, 4 );				// 4
					pPacket->WriteShortBE( (WORD)pNeighbour->m_nLeafCount );		// 2
					pPacket->WriteShortBE( (WORD)pNeighbour->m_nLeafLimit );		// 2
					pPacket->WritePacket( G2_PACKET_VENDOR, 4 );					// 3
					pPacket->WriteString( pNeighbour->m_pVendor->m_sCode );			// 5
				}
				else
				{
					pPacket->WritePacket( G2_PACKET_NEIGHBOUR_HUB, 9 + 6, TRUE );	// 4
					pPacket->WritePacket( G2_PACKET_HUB_STATUS, 4 );				// 4
					pPacket->WriteShortBE( (WORD)pNeighbour->m_nLeafCount );		// 2
					pPacket->WriteShortBE( (WORD)pNeighbour->m_nLeafLimit );		// 2
					pPacket->WriteByte( 0 );										// 1
				}

				pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );	// 4
				pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );		// 2
			}
		}
		pLock.Unlock();
	}

	pPacket->WritePacket( G2_PACKET_TIMESTAMP, 4 );
	pPacket->WriteLongBE( tNow );

	CQuickLock oLock( HostCache.Gnutella2.m_pSection );

	for ( CHostCacheIterator i = HostCache.Gnutella2.Begin() ;
		i != HostCache.Gnutella2.End() && nCount > 0 ; ++i )
	{
		CHostCacheHostPtr pHost = (*i);

		if ( pHost->CanQuote( tNow ) &&
			 Neighbours.Get( pHost->m_pAddress ) == NULL &&
			 ! Network.IsSelfIP( pHost->m_pAddress ) )
		{
			int nLength = 10;

			if ( pHost->m_pVendor && pHost->m_pVendor->m_sCode.GetLength() == 4 )
				nLength += 7;
			if ( pOwner && pOwner->m_nNodeType == ntLeaf && pHost->m_nKeyValue != 0 && pHost->m_nKeyHost == Network.m_pHost.sin_addr.S_un.S_addr )
				nLength += 8;
			if ( nLength > 10 )
				nLength ++;

			pPacket->WritePacket( G2_PACKET_CACHED_HUB, nLength, nLength > 10 );

			if ( pHost->m_pVendor && pHost->m_pVendor->m_sCode.GetLength() == 4 )
			{
				pPacket->WritePacket( G2_PACKET_VENDOR, 4 );				// 3
				pPacket->WriteString( pHost->m_pVendor->m_sCode, FALSE );	// 4
			}

			if ( pOwner && pOwner->m_nNodeType == ntLeaf && pHost->m_nKeyValue != 0 && pHost->m_nKeyHost == Network.m_pHost.sin_addr.S_un.S_addr )
			{
				pPacket->WritePacket( G2_PACKET_QUERY_KEY, 4 );				// 4
				pPacket->WriteLongBE( pHost->m_nKeyValue );					// 4
			}

			if ( nLength > 10 ) pPacket->WriteByte( 0 );					// 1

			pPacket->WriteLongLE( pHost->m_pAddress.S_un.S_addr );			// 4
			pPacket->WriteShortBE( pHost->m_nPort );						// 2
			pPacket->WriteLongBE( pHost->Seen() );							// 4

			nCount--;
		}
	}

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour KNOWN HUB LIST : receive

BOOL CG2Neighbour::OnKHL(CG2Packet* pPacket)
{
	m_tLastKHLIn = GetTickCount();
	m_nCountKHLIn++;

	return ParseKHLPacket( pPacket, &m_pHost );
}

BOOL CG2Neighbour::ParseKHLPacket(CG2Packet* pPacket, const SOCKADDR_IN* pHost)
{
	BOOL bInvalid = FALSE;
	CG2Neighbour* pOwner( NULL );
	if ( CNeighbour* pNeighbour = Neighbours.Get( pHost->sin_addr ) )
	{
		if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
			pOwner = static_cast< CG2Neighbour* >( pNeighbour );
	}

	if ( pOwner == NULL )
		theApp.Message( MSG_NOTICE, L"G2 KHL Packet Error from %s", (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );

	if ( pPacket->m_bCompound )
	{
		G2_PACKET nType, nInnerType;
		DWORD nLength, nInner;
		BOOL bCompound;
		LONG tAdjust = ( pOwner ) ? pOwner->m_tAdjust : 0;
		const DWORD tNow = static_cast< DWORD >( time( NULL ) );

		if ( pOwner && pOwner->m_pHubGroup )
			pOwner->m_pHubGroup->Clear();

		while ( pPacket->ReadPacket( nType, nLength, &bCompound ) )
		{
			const DWORD nNext = pPacket->m_nPosition + nLength;

			if ( nType == G2_PACKET_NEIGHBOUR_HUB ||
				 nType == G2_PACKET_CACHED_HUB )
			{
				DWORD nAddress = 0, nKey = 0, tSeen = tNow;
				WORD nPort = 0, nLeafs = 0, nLeafLimit = 0;
				CString strVendor;
				//Hashes::Guid oNodeID;
				//DWORD nFileCount = 0, nFileVolume = 0;

				if ( bCompound || nType == G2_PACKET_NEIGHBOUR_HUB )
				{
					while ( pPacket->m_nPosition < nNext && pPacket->ReadPacket( nInnerType, nInner ) )
					{
						const DWORD nNextX = pPacket->m_nPosition + nInner;

						switch ( nInnerType )
						{
						case G2_PACKET_NODE_ADDRESS:
							if ( nInner >= 6 )
							{
								nAddress = pPacket->ReadLongLE();
								nPort = pPacket->ReadShortBE();
							}
							break;
						case G2_PACKET_VENDOR:
							if ( nInner >= 4 )
								strVendor = pPacket->ReadString( 4 );
							break;
						case G2_PACKET_QUERY_KEY:
							if ( nInner >= 4 )
							{
								nKey = pPacket->ReadLongBE();
								if ( pOwner ) pOwner->m_bCachedKeys = TRUE;
							}
							break;
						case G2_PACKET_TIMESTAMP:
							if ( nInner >= 4 )
								tSeen = pPacket->ReadLongBE() + tAdjust;
							break;
						case G2_PACKET_HUB_STATUS:
							if ( nInner >= 2 )
							{
								nLeafs = pPacket->ReadShortBE();
								if ( nInner >= 4 )
									nLeafLimit = pPacket->ReadShortBE();
							}
							break;
						case G2_PACKET_NODE_GUID:
						// Obsolete for Morpheus
						//	if ( nInner >= 16 )
						//		pPacket->Read( oNodeID );
							break;
						case G2_PACKET_LIBRARY_STATUS:
						// Obsolete for Morpheus
						//	if ( nInner >= 8 )
						//	{
						//		nFileCount	= pPacket->ReadLongBE();
						//		nFileVolume	= pPacket->ReadLongBE();
						//	}
							break;
						default:
							bInvalid = TRUE;
						}

						pPacket->m_nPosition = nNextX;
					}

					nLength = nNext - pPacket->m_nPosition;
				}

				if ( nLength >= 6 )
				{
					nAddress = pPacket->ReadLongLE();
					nPort = pPacket->ReadShortBE();
					if ( nLength >= 10 )
						tSeen = pPacket->ReadLongBE() + tAdjust;
				}

				if ( nPort &&
					! Network.IsFirewalledAddress( (IN_ADDR*)&nAddress, TRUE ) &&
					! Network.IsReserved( (IN_ADDR*)&nAddress ) &&
					! Security.IsDenied( (IN_ADDR*)&nAddress ) )
				{
					CQuickLock oLock( HostCache.Gnutella2.m_pSection );

					CHostCacheHostPtr pCached = HostCache.Gnutella2.Add(
						(IN_ADDR*)&nAddress, nPort, tSeen, strVendor );
					if ( pCached != NULL )
					{
						if ( nLeafs ) pCached->m_nUserCount = nLeafs;			// Hack
						if ( nLeafLimit ) pCached->m_nUserLimit = nLeafLimit;	// Hack
						// if ( oNodeID )			// Not implemented
						// if ( nFileCount )		// Not implemented
						// if ( nFileVolume )		// Not implemented
						if ( pOwner && pOwner->m_nNodeType == ntHub )
						{
							if ( pCached->m_nKeyValue == 0 ||
									pCached->m_nKeyHost != Network.m_pHost.sin_addr.S_un.S_addr )
								pCached->SetKey( nKey, &(pOwner->m_pHost.sin_addr) );
						}
					}

					if ( nType == G2_PACKET_NEIGHBOUR_HUB && pOwner )
					{
						if ( pOwner->m_pHubGroup )
							pOwner->m_pHubGroup->Add( (IN_ADDR*)&nAddress, nPort );
					}

					HostCache.Gnutella2.m_nCookie++;
				}
			}
			else if ( nType == G2_PACKET_TIMESTAMP && nLength >= 4 )
			{
				tAdjust = (LONG)tNow - (LONG)pPacket->ReadLongBE();
			}
			else if ( nType == G2_PACKET_YOURIP && nLength >= 4 )
			{
				IN_ADDR pMyAddress;
				pMyAddress.s_addr = pPacket->ReadLongLE();
				Network.AcquireLocalAddress( pMyAddress );
			}
			else
			{
				bInvalid = TRUE;
			}

			pPacket->m_nPosition = nNext;
		}

		if ( pOwner ) pOwner->m_tAdjust = tAdjust;

		bInvalid = bInvalid || ( pPacket->GetRemaining() != 0 );
	}
	else
		bInvalid = TRUE;

	if ( bInvalid )
		theApp.Message( MSG_ERROR, L"[G2] Invalid KHL packet received from %s", (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour HUB ADVERTISEMENT WALKER : send

void CG2Neighbour::SendHAW()
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_HAW, TRUE );

	WORD nLeafs = 0;

	Hashes::Guid oGUID;
	Network.CreateID( oGUID );

	CSingleLock pLock( &Network.m_pSection );
	if ( SafeLock( pLock ) )
	{
		for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );

			if ( pNeighbour != this &&
				 pNeighbour->m_nState == nrsConnected &&
				 pNeighbour->m_nNodeType == ntLeaf )
			{
				nLeafs++;
			}
		}
	}

	pPacket->WritePacket( G2_PACKET_NODE_ADDRESS, 6 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );

	pPacket->WritePacket( G2_PACKET_HUB_STATUS, 2 );
	pPacket->WriteShortBE( nLeafs );

	pPacket->WritePacket( G2_PACKET_VENDOR, 4 );
	pPacket->WriteString( VENDOR_CODE );	// 5 bytes

	pPacket->WriteByte( 100 );	// TTL
	pPacket->WriteByte( 0 );	// Hops
	pPacket->Write( oGUID );

	Send( pPacket, TRUE, TRUE );

	m_pGUIDCache->Add( oGUID, this );

	m_tLastHAWOut = GetTickCount();
	m_nCountHAWOut++;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour HUB ADVERTISEMENT WALKER : receive

BOOL CG2Neighbour::OnHAW(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	m_tLastHAWIn = GetTickCount();
	m_nCountHAWIn ++;

	CString strVendor;
	G2_PACKET nType;
	DWORD nLength;

	DWORD nAddress	= 0;
	WORD nPort		= 0;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		const DWORD nNext = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_VENDOR && nLength >= 4 )
		{
			strVendor = pPacket->ReadString( 4 );
		}
		else if ( nType == G2_PACKET_NODE_ADDRESS && nLength >= 6 )
		{
			nAddress	= pPacket->ReadLongLE();
			nPort		= pPacket->ReadShortBE();
		}

		pPacket->m_nPosition = nNext;
	}

	if ( pPacket->GetRemaining() < 2 + 16 )
		return TRUE;

	if ( ! nPort ||
		 Network.IsFirewalledAddress( (IN_ADDR*)&nAddress, TRUE ) ||
		 Network.IsReserved( (IN_ADDR*)&nAddress ) ||
		 Security.IsDenied( (IN_ADDR*)&nAddress ) )
		return TRUE;

	BYTE* pPtr	= pPacket->m_pBuffer + pPacket->m_nPosition;
	BYTE nTTL	= pPacket->ReadByte();
	BYTE nHops	= pPacket->ReadByte();

	Hashes::Guid oGUID;
	pPacket->Read( oGUID );

	HostCache.Gnutella2.Add( (IN_ADDR*)&nAddress, nPort, 0, strVendor );

	if ( nTTL > 0 && nHops < 255 )
	{
		m_pGUIDCache->Add( oGUID, this );

		pPtr[0] = nTTL  - 1;
		pPtr[1] = nHops + 1;

		if ( CG2Neighbour* pNeighbour = Neighbours.GetRandomHub( this, oGUID ) )
			pNeighbour->Send( pPacket, FALSE, TRUE );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour QUERY packet handler

BOOL CG2Neighbour::SendQuery(const CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal)
{
	// If the caller didn't give us a packet, or one that isn't for our protocol, leave now
	if ( pPacket == NULL || pPacket->m_nProtocol != PROTOCOL_G2 )
		return FALSE;

	return CNeighbour::SendQuery( pSearch, pPacket, bLocal );
}

BOOL CG2Neighbour::OnQuery(CG2Packet* pPacket)
{
	DWORD tPrevQueryIn = m_tLastQueryIn;
	m_tLastQueryIn = GetTickCount();
	m_nCountQueryIn++;
	Statistics.Current.Gnutella2.Queries++;		// All Incoming

	CQuerySearchPtr pSearch = CQuerySearch::FromPacket( pPacket );
	if ( ! pSearch || pSearch->m_bDropMe )
	{
		if ( ! pSearch )
		{
			theApp.Message( MSG_WARNING, IDS_PROTOCOL_BAD_QUERY, L"G2", (LPCTSTR)m_sAddress );
			DEBUG_ONLY( pPacket->Debug( L"G2 Malformed Query." ) );
		}
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( Security.IsDenied( &pSearch->m_pEndpoint.sin_addr ) )
	{
		theApp.Message( MSG_DEBUG, IDS_PROTOCOL_BAD_QUERY, L"G2 security", (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	const BOOL bUseUDP = pSearch->m_bUDP &&
		pSearch->m_pEndpoint.sin_addr.s_addr != m_pHost.sin_addr.s_addr;

	if ( bUseUDP && m_nNodeType == ntLeaf )						// Forbid UDP answer for leaf query
	{
		theApp.Message( MSG_INFO, IDS_PROTOCOL_BAD_QUERY, L"G2 UDP leaf", (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( ! Network.QueryRoute->Add( pSearch->m_oGUID, this ) )	// Forbid looped query
	{
		theApp.Message( MSG_INFO, IDS_PROTOCOL_BAD_QUERY, L"G2 looped", (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( ( bUseUDP && ! Neighbours.CheckQuery( pSearch ) ) ||	// Too many UDP queries for one return address
		 ( m_nNodeType == ntLeaf && m_tLastQueryIn < tPrevQueryIn + 5 * 1000 ) )	// Too many queries for this leaf (maximum 1 query per 5 seconds)
	{
		// Ack without hub list with retry time
		if ( m_nNodeType == ntLeaf )
			Send( Neighbours.CreateQueryWeb( pSearch->m_oGUID, false, NULL, false ) );

		theApp.Message( MSG_WARNING, IDS_PROTOCOL_EXCEEDS_LIMIT,
			(LPCTSTR)( CString( inet_ntoa( m_pHost.sin_addr ) ) + L" [TCP]" ),
			(LPCTSTR)CString( inet_ntoa( pSearch->m_pEndpoint.sin_addr ) ) );

		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( m_nNodeType != ntHub )
		Neighbours.RouteQuery( pSearch, pPacket, this, m_nNodeType == ntLeaf );

	if ( bUseUDP )		//&& ! Network.IsFirewalled()
		Network.OnQuerySearch( new CLocalSearch( pSearch, PROTOCOL_G2 ) );
	else
		Network.OnQuerySearch( new CLocalSearch( pSearch, this ) );

	// Ack with hub list
	if ( m_nNodeType == ntLeaf )
		Send( Neighbours.CreateQueryWeb( pSearch->m_oGUID, true, this ), TRUE, FALSE );

	Statistics.Current.Gnutella2.QueriesProcessed++;

	return TRUE;
}

BOOL CG2Neighbour::OnQueryAck(CG2Packet* pPacket)
{
	HostCache.Gnutella2.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
	Hashes::Guid oGuid;
	SearchManager.OnQueryAck( pPacket, &m_pHost, oGuid );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour QUERY KEY REQUEST packet handler

BOOL CG2Neighbour::OnQueryKeyReq(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	if ( m_nNodeType != ntLeaf ) return TRUE;

	DWORD nLength, nAddress = 0;
	BOOL bCacheOkay = TRUE;
	G2_PACKET nType;
	WORD nPort = 0;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		const DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_QUERY_ADDRESS && nLength >= 6 )
		{
			nAddress	= pPacket->ReadLongLE();
			nPort		= pPacket->ReadShortBE();
		}
		else if ( nType == G2_PACKET_QUERY_REFRESH )
		{
			bCacheOkay = FALSE;
		}

		pPacket->m_nPosition = nOffset;
	}

	if ( ! nPort ||
		Network.IsFirewalledAddress( (IN_ADDR*)&nAddress, TRUE ) ||
		Network.IsReserved( (IN_ADDR*)&nAddress ) ||
		Security.IsDenied( (IN_ADDR*)&nAddress ) )
		return TRUE;

	if ( bCacheOkay )
	{
		CQuickLock oLock( HostCache.Gnutella2.m_pSection );

		CHostCacheHostPtr pCached = HostCache.Gnutella2.Find( (IN_ADDR*)&nAddress );
		if ( pCached != NULL && pCached->m_nKeyValue != 0 &&
			pCached->m_nKeyHost == Network.m_pHost.sin_addr.S_un.S_addr )
		{
			CG2Packet* pAnswer = CG2Packet::New( G2_PACKET_QUERY_KEY_ANS, TRUE );
			pAnswer->WritePacket( G2_PACKET_QUERY_ADDRESS, 6 );
			pAnswer->WriteLongLE( nAddress );
			pAnswer->WriteShortBE( nPort );
			pAnswer->WritePacket( G2_PACKET_QUERY_KEY, 4 );
			pAnswer->WriteLongBE( pCached->m_nKeyValue );
			pAnswer->WritePacket( G2_PACKET_QUERY_CACHED, 0 );
			Send( pAnswer );
			return TRUE;
		}
	}

	CG2Packet* pRequest = CG2Packet::New( G2_PACKET_QUERY_KEY_REQ, TRUE );
	pRequest->WritePacket( G2_PACKET_SEND_ADDRESS, 4 );
	pRequest->WriteLongLE( m_pHost.sin_addr.S_un.S_addr );
	Datagrams.Send( (IN_ADDR*)&nAddress, nPort, pRequest, TRUE, NULL, FALSE );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour QUERY KEY ANSWER packet handler

BOOL CG2Neighbour::OnQueryKeyAns(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	if ( m_nNodeType != ntHub ) return TRUE;

	DWORD nKey = 0, nAddress = 0;
	WORD nPort = 0;

	G2_PACKET nType;
	DWORD nLength, nOffset;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		nOffset = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_QUERY_KEY && nLength >= 4 )
		{
			nKey	 = pPacket->ReadLongBE();
		}
		else if ( nType == G2_PACKET_QUERY_ADDRESS && nLength >= 6 )
		{
			nAddress = pPacket->ReadLongLE();
			nPort	 = pPacket->ReadShortBE();
		}
		else if ( nType == G2_PACKET_QUERY_CACHED )
		{
			m_bCachedKeys = TRUE;
		}

		pPacket->m_nPosition = nOffset;
	}

	CQuickLock oLock( HostCache.Gnutella2.m_pSection );

	CHostCacheHostPtr pCache = HostCache.Gnutella2.Add( (IN_ADDR*)&nAddress, nPort );
	if ( pCache != NULL )
	{
		theApp.Message( MSG_DEBUG, L"Got a query key for %s:%i via neighbour %s: 0x%x",
			(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nAddress ) ), nPort, (LPCTSTR)m_sAddress, nKey );
		pCache->SetKey( nKey, &m_pHost.sin_addr );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour PUSH packet handler

bool CG2Neighbour::OnPush(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound )
		return true;

	DWORD nLength = pPacket->GetRemaining();

	// Check if packet is too small
	if ( ! pPacket->SkipCompound( nLength, 6 ) )
	{
		// Ignore packet and return that it was handled
		theApp.Message( MSG_NOTICE, IDS_PROTOCOL_SIZE_PACKET, m_sAddress, L"push" );
		DEBUG_ONLY( pPacket->Debug( L"BadPush" ) );
		++Statistics.Current.Gnutella2.Dropped;
		++m_nDropCount;
		return true;
	}

	// Get IP address and port
	const DWORD nAddress	= pPacket->ReadLongLE();
	const WORD nPort		= pPacket->ReadShortBE();

	// Check the security list to make sure the IP address isn't on it
	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		// Failed security check, ignore packet and return that it was handled
		++Statistics.Current.Gnutella2.Dropped;
		++m_nDropCount;
		return true;
	}

	// Check that remote client has a port number, isn't firewalled or using reserved address
	if ( ! nPort
		|| Network.IsFirewalledAddress( (IN_ADDR*)&nAddress )
		|| Network.IsReserved( (IN_ADDR*)&nAddress ) )
	{
		// Can't push open connection, ignore packet and return that it was handled
		theApp.Message( MSG_NOTICE, IDS_PROTOCOL_ZERO_PUSH, m_sAddress );
		++Statistics.Current.Gnutella2.Dropped;
		++m_nDropCount;
		return true;
	}

	// Setup push connection
	Handshakes.PushTo( (IN_ADDR*)&nAddress, nPort );

	// Packet was handled
	return true;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour USER PROFILE CHALLENGE packet handler

BOOL CG2Neighbour::OnProfileChallenge(CG2Packet* /*pPacket*/)
{
	if ( ! MyProfile.IsValid() ) return TRUE;

	CG2Packet* pProfile = CG2Packet::New( G2_PACKET_PROFILE_DELIVERY, TRUE );

	CString strXML = MyProfile.GetPublicXML( /*m_sUserAgent*/ NULL, TRUE )->ToString( TRUE );

	pProfile->WritePacket( G2_PACKET_XML, pProfile->GetStringLen( strXML ) );
	pProfile->WriteString( strXML, FALSE );

	Send( pProfile, TRUE, TRUE );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour USER PROFILE DELIVERY packet handler

BOOL CG2Neighbour::OnProfileDelivery(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		const DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_XML )
		{
			if ( CXMLElement* pXML = CXMLElement::FromString( pPacket->ReadString( nLength ), TRUE ) )
			{
				if ( m_pProfile == NULL ) m_pProfile = new CGProfile();
				if ( ! m_pProfile->FromXML( pXML ) ) delete pXML;
			}
		}

		pPacket->m_nPosition = nOffset;
	}

	if ( m_pProfile )
	{
		CQuickLock oLock( HostCache.Gnutella2.m_pSection );

		if ( CHostCacheHostPtr pHost = HostCache.Gnutella2.Find( &m_pHost.sin_addr ) )
		{
			pHost->m_sName = m_pProfile->GetNick();
			HostCache.Gnutella2.m_nCookie ++;
		}
	}

	return TRUE;
}
