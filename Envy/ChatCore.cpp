//
// ChatCore.cpp
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "ChatCore.h"
#include "ChatSession.h"
#include "Buffer.h"
#include "EDClient.h"
#include "DCNeighbour.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CChatCore ChatCore;


//////////////////////////////////////////////////////////////////////
// CChatCore construction

CChatCore::CChatCore()
{
}

CChatCore::~CChatCore()
{
	Close();
}

//////////////////////////////////////////////////////////////////////
// CChatCore session access

POSITION CChatCore::GetIterator() const
{
	ASSUME_LOCK( m_pSection );
	return m_pSessions.GetHeadPosition();
}

CChatSession* CChatCore::GetNext(POSITION& pos) const
{
	ASSUME_LOCK( m_pSection );
	return m_pSessions.GetNext( pos );
}

BOOL CChatCore::Check(CChatSession* pSession) const
{
	ASSUME_LOCK( m_pSection );
	return m_pSessions.Find( pSession ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CChatCore accept new connections

BOOL CChatCore::OnAccept(CConnection* pConnection)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 200 ) )
		return TRUE;	// Try later

	if ( ! Settings.Community.ChatEnable || ! MyProfile.IsValid() )
	{
		theApp.Message( MSG_ERROR, L"Rejecting incoming connection from %s, chat disabled.", (LPCTSTR)pConnection->m_sAddress );
		pConnection->Write( _P("CHAT/0.2 503 Unavailable\r\n\r\n") );
		pConnection->LogOutgoing();
		pConnection->DelayClose( IDS_CONNECTION_CLOSED );
		return TRUE;
	}

	if ( CChatSession* pSession = new CChatSession( PROTOCOL_NULL ) )
	{
		pSession->AttachTo( pConnection );
		return FALSE;
	}

	// Out of memory
	return TRUE;
}

BOOL CChatCore::OnPush(const Hashes::Guid& oGUID, CConnection* pConnection)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChatSession* pSession = GetNext( pos );
		if ( pSession->OnPush( oGUID, pConnection ) )
			return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CChatCore ED2K chat handling

template<>
CChatSession* CChatCore::FindSession< CDCNeighbour >(const CDCNeighbour* pClient, BOOL bCreate)
{
	ASSUME_LOCK( m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChatSession* pSession = GetNext( pos );

		// Check if we already have a session
		if ( pSession->m_pHost.sin_addr.s_addr == pClient->m_pHost.sin_addr.s_addr &&
			 pSession->m_nProtocol == pClient->m_nProtocol )
		{
			// Update details
			pSession->m_oGUID		= pClient->m_oGUID;
			pSession->m_pHost		= pClient->m_pHost;
			pSession->m_sAddress	= pClient->m_sAddress;
			pSession->m_sNick		= pClient->m_sServerName;
			pSession->m_sUserAgent	= pClient->m_sUserAgent;

			// Return existing session
			return pSession;
		}
	}

	if ( ! bCreate )
		return NULL;

	// Create a new chat session
	CChatSession* pSession = new CChatSession( pClient->m_nProtocol );
	pSession->m_oGUID		= pClient->m_oGUID;
	pSession->m_pHost		= pClient->m_pHost;
	pSession->m_sAddress	= pClient->m_sAddress;
	pSession->m_sNick		= pClient->m_sServerName;
	pSession->m_sUserAgent	= pClient->m_sUserAgent;

	// Make new input and output buffer objects
	pSession->CreateBuffers();

	pSession->MakeActive( FALSE );

	return pSession;
}

template<>
CChatSession* CChatCore::FindSession< CEDClient >(const CEDClient* pClient, BOOL bCreate)
{
	ASSUME_LOCK( m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChatSession* pSession = GetNext( pos );

		// Check if we already have a session
		if ( ( ! pSession->m_oGUID || validAndEqual( pSession->m_oGUID, pClient->m_oGUID ) ) &&
			 pSession->m_pHost.sin_addr.s_addr == pClient->m_pHost.sin_addr.s_addr &&
			 pSession->m_nProtocol == pClient->m_nProtocol )
		{
			// Update details
			pSession->m_oGUID		= pClient->m_oGUID;
			pSession->m_pHost		= pClient->m_pHost;
			pSession->m_sAddress	= pClient->m_sAddress;
			pSession->m_sNick		= pClient->m_sNick;
			pSession->m_sUserAgent	= pClient->m_sUserAgent;
			pSession->m_bUnicode	= pClient->m_bEmUnicode;
			pSession->m_nClientID	= pClient->m_nClientID;
			pSession->m_pServer		= pClient->m_pServer;

			pSession->m_bMustPush	= ( pClient->m_nClientID > 0 && pClient->m_nClientID < 16777216 );

			// Return existing session
			return pSession;
		}
	}

	if ( ! bCreate )
		return NULL;

	// Create a new chat session
	CChatSession* pSession = new CChatSession( pClient->m_nProtocol );

	pSession->m_oGUID		= pClient->m_oGUID;
	pSession->m_pHost		= pClient->m_pHost;
	pSession->m_sAddress	= pClient->m_sAddress;
	pSession->m_sNick		= pClient->m_sNick;
	pSession->m_sUserAgent	= pClient->m_sUserAgent;
	pSession->m_bUnicode	= pClient->m_bEmUnicode;
	pSession->m_nClientID	= pClient->m_nClientID;
	pSession->m_pServer		= pClient->m_pServer;
	pSession->m_bMustPush	= ( pClient->m_nClientID > 0 && pClient->m_nClientID < 16777216 );

	//pSession->m_nState	 = cssActive;
	//pSession->m_bConnected = TRUE;
	//pSession->m_tConnected = GetTickCount();

	// Make new input and output buffer objects
	pSession->CreateBuffers();

	pSession->MakeActive();

	return pSession;
}

//////////////////////////////////////////////////////////////////////
// CChatCore session add and remove

void CChatCore::Add(CChatSession* pSession)
{
	CQuickLock pLock( m_pSection );

	if ( m_pSessions.Find( pSession ) == NULL )
		m_pSessions.AddTail( pSession );

	if ( pSession->IsValid() )
		WSAEventSelect( pSession->m_hSocket, GetWakeupEvent(),
			FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );

	StartThread();
}

void CChatCore::Remove(CChatSession* pSession)
{
	CQuickLock pLock( m_pSection );

	if ( POSITION pos = m_pSessions.Find( pSession ) )
		m_pSessions.RemoveAt( pos );

	if ( pSession->IsValid() )
		WSAEventSelect( pSession->m_hSocket, GetWakeupEvent(), 0 );
}

void CChatCore::Close()
{
	CloseThread();

	CQuickLock pLock( m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Close();
	}
}

//////////////////////////////////////////////////////////////////////
// CChatCore thread control

void CChatCore::StartThread()
{
	if ( theApp.m_bClosing )
		return;

	if ( GetCount() == 0 )
		return;

	BeginThread( "ChatCore" );
}

//////////////////////////////////////////////////////////////////////
// CChatCore thread run

void CChatCore::OnRun()
{
	CSingleLock pLock( &m_pSection );

	while ( IsThreadEnabled() )
	{
		Sleep( 50 );
		Doze( 100 );

		if ( pLock.Lock( 250 ) )
		{
			if ( GetCount() == 0 ) break;

			for ( POSITION pos = GetIterator() ; pos ; )
			{
				GetNext( pos )->DoRun();
			}

			pLock.Unlock();
		}
	}
}
