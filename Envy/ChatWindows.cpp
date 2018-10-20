//
// ChatWindows.cpp
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "ChatWindows.h"
#include "ChatSession.h"
#include "WndChat.h"
#include "WndPrivateChat.h"
#include "Buffer.h"
#include "EDClient.h"
#include "EDClients.h"
#include "Transfers.h"
#include "Neighbours.h"
#include "Network.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CChatWindows ChatWindows;


//////////////////////////////////////////////////////////////////////
// CChatWindows construction

CChatWindows::CChatWindows()
{
}

CChatWindows::~CChatWindows()
{
}

//////////////////////////////////////////////////////////////////////
// CChatWindows list access

POSITION CChatWindows::GetIterator() const
{
	return m_pList.GetHeadPosition();
}

CChatWnd* CChatWindows::GetNext(POSITION& pos) const
{
	return m_pList.GetNext( pos );
}

//////////////////////////////////////////////////////////////////////
// CChatWindows close all

//void CChatWindows::Close()
//{
//	for ( POSITION pos = GetIterator(); pos; )
//	{
//		GetNext( pos )->GetParent()->DestroyWindow();
//	}
//
//	m_pList.RemoveAll();
//}

//////////////////////////////////////////////////////////////////////
// CChatWindows private chat windows

CPrivateChatWnd* CChatWindows::FindPrivate(const Hashes::Guid& oGUID, bool bLive) const
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		CPrivateChatWnd* pFrame = static_cast<CPrivateChatWnd*>( GetNext( pos ) );

		if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatWnd) ) )
		{
			if ( pFrame->Find( oGUID, bLive ) )
				return pFrame;
		}
	}

	return NULL;
}

CPrivateChatWnd* CChatWindows::FindPrivate(const SOCKADDR_IN* pAddress) const
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		CPrivateChatWnd* pFrame = static_cast<CPrivateChatWnd*>( GetNext( pos ) );

		if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatWnd) ) )
		{
			if ( pFrame->Find( pAddress ) )
				return pFrame;
		}
	}

	return NULL;
}

CPrivateChatWnd* CChatWindows::FindED2KFrame(const SOCKADDR_IN* pAddress) const
{
	// For High ID clients
	CString strHighID;
	strHighID.Format( L"%s:%hu", (LPCTSTR)CString( inet_ntoa( pAddress->sin_addr ) ), ntohs( pAddress->sin_port ) );

	for ( POSITION pos = GetIterator(); pos; )
	{
		CPrivateChatWnd* pFrame = static_cast<CPrivateChatWnd*>( GetNext( pos ) );

		if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatWnd) ) )
		{
			if ( pFrame->Find( strHighID ) )
				return pFrame;
		}
	}

	return NULL;
}

CPrivateChatWnd* CChatWindows::FindED2KFrame(DWORD nClientID, const SOCKADDR_IN* pServerAddress) const
{
	// For Low ID clients
	if ( ( nClientID > 0 ) && ( nClientID < 16777216 ) )  // ED2K Low ID
	{
		CString strLowID;
		strLowID.Format( L"%u@%s:%hu",
		nClientID,
		(LPCTSTR)CString( inet_ntoa( pServerAddress->sin_addr ) ),
		pServerAddress->sin_port );

		for ( POSITION pos = GetIterator(); pos; )
		{
			CPrivateChatWnd* pFrame = static_cast<CPrivateChatWnd*>( GetNext( pos ) );

			if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatWnd) ) )
			{
				if ( pFrame->Find( strLowID ) )
					return pFrame;
			}
		}
	}

	return NULL;
}

CPrivateChatWnd* CChatWindows::OpenPrivate(const Hashes::Guid& oGUID, const IN_ADDR* pAddress, WORD nPort, BOOL bMustPush, PROTOCOLID nProtocol, IN_ADDR* pServerAddress, WORD nServerPort)
{
	SOCKADDR_IN pHost = {};

	pHost.sin_family	= PF_INET;
	pHost.sin_addr		= *pAddress;
	pHost.sin_port		= htons( nPort );

	if ( pServerAddress == NULL )
		return OpenPrivate( oGUID, &pHost, bMustPush, nProtocol, NULL );

	SOCKADDR_IN pServer = {};

	pServer.sin_family	= PF_INET;
	pServer.sin_addr	= *pServerAddress;
	pServer.sin_port	= htons( nServerPort );

	return OpenPrivate( oGUID, &pHost, bMustPush, nProtocol, &pServer );
}

CPrivateChatWnd* CChatWindows::OpenPrivate(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, PROTOCOLID nProtocol, SOCKADDR_IN* pServer)
{
	if ( ! MyProfile.IsValid() )
	{
		CString strMessage;
		LoadString( strMessage, IDS_CHAT_NEED_PROFILE );
		if ( MsgBox( strMessage, MB_YESNO|MB_ICONQUESTION ) == IDYES )
			PostMainWndMessage( WM_COMMAND, ID_TOOLS_PROFILE );
		return NULL;
	}

	switch ( nProtocol )
	{
	case PROTOCOL_G1:
		Settings.Gnutella1.Enabled = true;
		return OpenPrivateGnutella( oGUID, pHost, bMustPush, PROTOCOL_ANY );

	case PROTOCOL_G2:
	case PROTOCOL_HTTP:
		Settings.Gnutella2.Enabled = true;
		return OpenPrivateGnutella( oGUID, pHost, bMustPush, PROTOCOL_ANY );

	case PROTOCOL_ED2K:
		Settings.eDonkey.Enabled = true;
		return OpenPrivateED2K( oGUID, pHost, bMustPush, pServer );

	default:
		return NULL;
	}
}

CPrivateChatWnd* CChatWindows::OpenPrivateGnutella(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, PROTOCOLID nProtocol)
{
	CPrivateChatWnd* pFrame = NULL;
	if ( oGUID )
	{
		pFrame = FindPrivate( oGUID, false );
		if ( pFrame == NULL )
			pFrame = FindPrivate( oGUID, true );
	}

	if ( pFrame == NULL )
		pFrame = FindPrivate( pHost );

	if ( pFrame == NULL )
	{
		pFrame = new CPrivateChatWnd();
		pFrame->Setup( oGUID, pHost, bMustPush, nProtocol );
	}

	pFrame->PostMessage( WM_COMMAND, ID_CHAT_CONNECT );

	pFrame->Open();

	return pFrame;
}

CPrivateChatWnd* CChatWindows::OpenPrivateED2K(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, SOCKADDR_IN* pServer)
{
	// First, check if it's a low ID user on another server.
	if ( bMustPush && pServer )
	{
		// It's a firewalled user (Low ID). If they are using another server,
		// we can't (shouldn't) contact them. (Places heavy load on ed2k servers)
		CSingleLock pLock1( &Network.m_pSection );
		if ( ! pLock1.Lock( 250 ) ) return NULL;
		if ( Neighbours.Get( pServer->sin_addr ) == NULL ) return NULL;
		pLock1.Unlock();
	}

	// ED2K chat is handled by the EDClient section. (Transfers)
	// We need to find (or create) an EDClient to handle this chat session,
	// since everything on ed2k shares a TCP link.

	// First, lock the section to prevent a problem with other threads
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return NULL;

	// We need to connect to them, so either find or create an EDClient
	CEDClient* pClient;
	if ( pServer )
		pClient = EDClients.Connect(pHost->sin_addr.s_addr, ntohs( pHost->sin_port ), &pServer->sin_addr, ntohs( pServer->sin_port ), oGUID );
	else
		pClient = EDClients.Connect(pHost->sin_addr.s_addr, ntohs( pHost->sin_port ), NULL, 0, oGUID );
	// If we weren't able to create a client (Low-id and no server), then exit.
	if ( ! pClient ) return NULL;
	// Have it connect (if it isn't)
	if ( ! pClient->m_bConnected ) pClient->Connect();
	// Tell it to start a chat session as soon as it's able
	pClient->OpenChat();
	pLock.Unlock();

	// Check for / make active any existing window
	CPrivateChatWnd* pFrame = FindPrivate( pHost );
	// Check for an empty frame
	if ( pFrame == NULL )
	{
		if ( bMustPush )
			pFrame = FindED2KFrame( pHost->sin_addr.s_addr, pServer );
		else
			pFrame = FindED2KFrame( pHost );
	}
	if ( pFrame != NULL )
	{
		// Open window if we found one
		//CWnd* pParent = pFrame->GetParent();
		//if ( pParent->IsIconic() ) pParent->ShowWindow( SW_SHOWNORMAL );
		//pParent->BringWindowToTop();
		//pParent->SetForegroundWindow();
		pFrame->Open();

		// And exit
		return pFrame;
	}

	// Set name (Also used to match incoming connection)
	CString strNick;
	if ( bMustPush && pServer )		// Firewalled user (Low ID)
	{
		strNick.Format( L"%lu@%s:%hu",
			pHost->sin_addr.S_un.S_addr,
			(LPCTSTR)CString( inet_ntoa( pServer->sin_addr ) ),
			ntohs( pServer->sin_port ) );
	}
	else	// Regular user (High ID)
	{
		strNick.Format( L"%s:%hu", (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), ntohs( pHost->sin_port ) );
	}

	// Open an empty (blank) chat frame. This is totally unnecessary- The EDClient will open
	// one as required, but it looks better to open one here.
	pFrame = new CPrivateChatWnd();
	pFrame->Setup( strNick );

// Obsolete: for reference & deletion
//	// Open window
//	CWnd* pParent = pFrame->GetParent();
//	if ( pParent->IsIconic() ) pParent->ShowWindow( SW_SHOWNORMAL );
//	pParent->BringWindowToTop();
//	pParent->SetForegroundWindow();
//	// Put a 'connecting' message in the window
//	CString strMessage;
//	strMessage.Format( LoadString( IDS_CHAT_CONNECTING_TO ), (LPCTSTR)pFrame->m_sNick );
//	pFrame->OnStatusMessage( 0, strMessage );
//
//	if ( oGUID )
//		pFrame = FindPrivate( oGUID );
//	if ( pFrame == NULL )
//		pFrame = FindPrivate( &pHost->sin_addr );
//	if ( pFrame == NULL )
//	{
//		pFrame = new CPrivateChatWnd();
//		pFrame->Initiate( oGUID, pHost, bMustPush );
//	}
//
//	pFrame->PostMessage( WM_COMMAND, ID_CHAT_CONNECT );
//
//	CWnd* pParent = pFrame->GetParent();
//	if ( pParent->IsIconic() ) pParent->ShowWindow( SW_SHOWNORMAL );
//	pParent->BringWindowToTop();
//	pParent->SetForegroundWindow();

	return pFrame;
}

//////////////////////////////////////////////////////////////////////
// CChatWindows add and remove

void CChatWindows::Add(CChatWnd* pFrame)
{
	if ( m_pList.Find( pFrame ) == NULL )
		m_pList.AddTail( pFrame );
}

void CChatWindows::Remove(CChatWnd* pFrame)
{
	if ( POSITION pos = m_pList.Find( pFrame ) )
		m_pList.RemoveAt( pos );
}
