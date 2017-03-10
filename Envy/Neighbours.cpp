//
// Neighbours.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
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

// Complete the CNeighbours inheritance column, calling Close on each neighbour when the program exits
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CNeighbours
// http://getenvy.com/shareazawiki/Developers.Code.CNeighbours.html

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "Neighbours.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "GProfile.h"
#include "Network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Create the single global Neighbours object that holds the list of neighbour computers we are connected to.
// This line creates a single global instance of a CNeighbours object, called Neighbours, when Envy starts running.

CNeighbours Neighbours;

//////////////////////////////////////////////////////////////////////
// CNeighbours construction

// The line above creates the one CNeighbours object named Neighbours
// Creating that object calls this empty constructor, then the CNeighboursWithConnect constructor, and so on all the way down to CNeighboursBase

CNeighbours::CNeighbours()
{
}

CNeighbours::~CNeighbours()
{
}

CString CNeighbours::GetName(const CNeighbour* pNeighbour) const
{
	if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
	{
		switch ( pNeighbour->m_nNodeType )
		{
		case ntNode:
			return LoadString( IDS_NEIGHBOUR_G1PEER );
		case ntHub:
			return LoadString( IDS_NEIGHBOUR_G1ULTRA );
		case ntLeaf:
			return LoadString( IDS_NEIGHBOUR_G1LEAF );
		}
	}
	else if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
	{
		switch ( pNeighbour->m_nNodeType )
		{
		case ntNode:
			return LoadString( IDS_NEIGHBOUR_G2PEER );
		case ntHub:
			return LoadString( IDS_NEIGHBOUR_G2HUB );
		case ntLeaf:
			return LoadString( IDS_NEIGHBOUR_G2LEAF );
		}
	}

	return protocolNames[ pNeighbour->m_nProtocol ];
}

CString CNeighbours::GetAgent(const CNeighbour* pNeighbour) const
{
	if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
	{
		const CEDNeighbour* pED2K = static_cast< const CEDNeighbour* >( pNeighbour );

		if ( pED2K->m_nClientID )
			return LoadString( CEDPacket::IsLowID( pED2K->m_nClientID ) ?
				IDS_NEIGHBOUR_ED2K_LOWID : IDS_NEIGHBOUR_ED2K_HIGHID );
		else
			return LoadString( IDS_NEIGHBOUR_ED2K_SERVER );
	}

	return pNeighbour->m_sUserAgent;
}

CString CNeighbours::GetNick(const CNeighbour* pNeighbour) const
{
	return pNeighbour->m_pProfile ? pNeighbour->m_pProfile->GetNick() : pNeighbour->m_sServerName;
}

//CString GetServerName(const CNeighbour* pNeighbour) const
//{
//	return GetServerName( pNeighbour->m_sAddress );
//}

CString CNeighbours::GetServerName(const CString& strIP) const
{
	// ToDo: Utilize DefaultServers.dat or fix properly elsewhere
	if ( IsText( strIP, _P( L"176.103.48.36" ) ) )
		return L"TV Underground";
	if ( IsText( strIP, _P( L"213.152.168.189" ) ) )
		return L"eMule Security";
	if ( StartsWith( strIP, _P( L"176.103.56." ) ) )
		return L"eDonkey Server";
	if ( IsText( strIP, _P( L"195.154.83.5" ) ) )
		return L"Peerates.net";
	return CString();
}

CString CNeighbours::GetNeighbourList(LPCTSTR szFormat) const
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) )
		return CString();

	CString strOutput;
	const DWORD tNow = GetTickCount();

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		const CNeighbour* pNeighbour = Neighbours.GetNext( pos );

		if ( pNeighbour->m_nState == nrsConnected )
		{
			DWORD nTime = ( tNow - pNeighbour->m_tConnected ) / 1000;

			CString strNode;
			strNode.Format( szFormat,
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ),
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ),
				nTime / 3600, ( nTime % 3600 ) / 60, nTime % 60,
				(LPCTSTR)Neighbours.GetName( pNeighbour ),
				(LPCTSTR)Neighbours.GetAgent( pNeighbour ),
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );

			strOutput += strNode;
		}
	}

	return strOutput;
}
