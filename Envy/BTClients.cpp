//
// BTClients.cpp
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
#include "Network.h"
#include "Transfers.h"
#include "BTClients.h"
#include "BTClient.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CBTClients BTClients;


//////////////////////////////////////////////////////////////////////
// CBTClients construction

CBTClients::CBTClients()
{
}

CBTClients::~CBTClients()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CBTClients clear

void CBTClients::Clear()
{
	CSingleLock oLock( &m_pListSection, TRUE );
	while ( ! m_pList.IsEmpty() )
	{
		oLock.Unlock();
		m_pList.GetHead()->Close();
		oLock.Lock();
	}
}

//////////////////////////////////////////////////////////////////////
// CBTClients accept new connections

BOOL CBTClients::OnAccept(CConnection* pConnection)
{
	if ( ! Network.IsConnected() || ( Settings.Connection.RequireForTransfers && ! Settings.BitTorrent.Enabled ) )
	{
		theApp.Message( MSG_ERROR, IDS_BT_CLIENT_DROP_CONNECTED, (LPCTSTR)pConnection->m_sAddress );
		return FALSE;
	}

	CSingleLock pLock( &Transfers.m_pSection );
	if ( pLock.Lock( 250 ) )
	{
		if ( CBTClient* pClient = new CBTClient() )
		{
			pClient->AttachTo( pConnection );
			return TRUE;
		}
	}

	theApp.Message( MSG_ERROR, L"Rejecting BitTorrent connection from %s, network core overloaded.", (LPCTSTR)pConnection->m_sAddress );		// protocolNames[ PROTOCOL_BT ]
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBTClients add and remove

void CBTClients::Add(CBTClient* pClient)
{
	CQuickLock oLock( m_pListSection );

	ASSERT( m_pList.Find( pClient ) == NULL );
	m_pList.AddHead( pClient );
}

void CBTClients::Remove(CBTClient* pClient)
{
	CQuickLock oLock( m_pListSection );

	POSITION pos = m_pList.Find( pClient );
	ASSERT( pos != NULL );
	m_pList.RemoveAt( pos );
}
