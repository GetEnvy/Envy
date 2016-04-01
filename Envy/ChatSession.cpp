//
// ChatSession.cpp
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
#include "ChatSession.h"
#include "ChatCore.h"
#include "ChatWindows.h"
#include "WndPrivateChat.h"

#include "GProfile.h"
#include "G2Packet.h"
#include "DCClient.h"
#include "DCPacket.h"
#include "EDPacket.h"
#include "EDClient.h"
#include "EDClients.h"
#include "DCNeighbour.h"
#include "Neighbours.h"
#include "Network.h"
#include "Transfers.h"
#include "ImageFile.h"
#include "Buffer.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CChatSession construction

CChatSession::CChatSession(PROTOCOLID nProtocol, CPrivateChatWnd* pFrame)
	: CConnection	( nProtocol )
	, m_nState		( cssNull )
	, m_bMustPush	( FALSE )
	, m_bUnicode	( FALSE )
	, m_bOld		( ( nProtocol == PROTOCOL_ED2K || nProtocol == PROTOCOL_DC ) ? TRI_TRUE : TRI_UNKNOWN )
	, m_nClientID	( 0 )
	, m_tPushed		( 0 )
	, m_pProfile	( NULL )
	, m_pWndPrivate	( pFrame )
{
	ZeroMemory( &m_pServer, sizeof( m_pServer ) );
	m_pServer.sin_family = AF_INET;

	ChatCore.Add( this );
}

CChatSession::~CChatSession()
{
	CQuickLock pLock( ChatCore.m_pSection );

	ASSERT( ! IsValid() );

	ClearMessages();

	delete m_pProfile;

	ChatCore.Remove( this );
}

//////////////////////////////////////////////////////////////////////
// CChatSession connect

BOOL CChatSession::Connect()
{
	CQuickLock pLock( ChatCore.m_pSection );

	// If we are already connected/handshaking/connecting, don't try again.
	if ( m_nState > cssNull )
		return FALSE;

	// ED2K Clients have their connection controlled by ED2KClient. (One connection used for many things)
	if ( m_nProtocol == PROTOCOL_ED2K )
		return Send( (CEDPacket*)NULL );
	if ( m_nProtocol == PROTOCOL_DC )
		return Send( (CDCPacket*)NULL );

	if ( m_bMustPush )
	{
		if ( ! SendPush( FALSE ) )
		{
			StatusMessage( cmtError, IDS_CHAT_CANT_PUSH, (LPCTSTR)HostToString( &m_pHost ) );		// Was CString( inet_ntoa( m_pHost.sin_addr ) )
			return FALSE;
		}
	}
	else
	{
		if ( CConnection::ConnectTo( &m_pHost ) )
		{
			ChatCore.Add( this );
			StatusMessage( cmtStatus, IDS_CHAT_CONNECTING_TO, (LPCTSTR)HostToString( &m_pHost ) );	// Was m_sAddress
		}
		else
		{
			StatusMessage( cmtError, IDS_CHAT_CANT_CONNECT, (LPCTSTR)HostToString( &m_pHost ) );
			return FALSE;
		}
	}

	m_nState = cssConnecting;

	return TRUE;
}

TRISTATE CChatSession::GetConnectedState() const
{
	if ( m_nState == cssNull ) return TRI_FALSE;
	if ( m_nState >= cssActive ) return TRI_TRUE;
	return TRI_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////
// CChatSession handle an incoming DC++ or ED2K packets

void CChatSession::OnMessage(CPacket* pPacket)
{
	// Open a window (if one is not already open)
	//PostOpenWindow();
	MakeActive( FALSE );

	// Put the packet into the input buffer so it can be 'received' (and displayed) later.
	if ( pPacket && IsInputExist() )
		pPacket->ToBuffer( GetInput() );
}

//////////////////////////////////////////////////////////////////////
// If this client wasn't active, it is now.

void CChatSession::MakeActive(BOOL bAddUsers)
{
	CQuickLock pLock( ChatCore.m_pSection );

	if ( m_nState != cssActive )
	{
		m_nState = cssActive;

		m_bConnected = TRUE;
		m_tConnected = GetTickCount();

		StatusMessage( cmtInfo, IDS_CHAT_PRIVATE_ONLINE, (LPCTSTR)m_sNick );

		PlaySound( L"Sound_IncomingChat", NULL, SND_APPLICATION|SND_ALIAS|SND_ASYNC );

		if ( bAddUsers )
		{
			AddUser( new CChatUser( cutMe, MyProfile.GetNick() ) );
			AddUser( new CChatUser( cutUser, m_sNick ) );
		}

		if ( ! m_pWndPrivate )
			PostMainWndMessage( WM_OPENCHAT, (WPARAM)this );
	}
}

//////////////////////////////////////////////////////////////////////
// CChatSession attach to (accept) an incoming connection

void CChatSession::AttachTo(CConnection* pConnection)
{
	CQuickLock pLock( ChatCore.m_pSection );

	CConnection::AttachTo( pConnection );

	m_sNick = HostToString( &m_pHost );

	m_nState = cssRequest1;
	ChatCore.Add( this );
}

//////////////////////////////////////////////////////////////////////
// CChatSession push functionality

BOOL CChatSession::SendPush(BOOL /*bAutomatic*/)
{
	if ( ! m_oGUID )
		return FALSE;

	if ( m_nProtocol == PROTOCOL_ED2K || m_nProtocol == PROTOCOL_DC )
		return FALSE;

	if ( Network.SendPush( m_oGUID, 0 ) )
	{
		m_nState = cssNull;
		CConnection::Close();

		m_tConnected = m_tPushed = GetTickCount();
		StatusMessage( cmtStatus, IDS_CHAT_PUSH_SENT, (LPCTSTR)HostToString( &m_pHost ) );	// Was CString( inet_ntoa( m_pHost.sin_addr ) )

		return TRUE;
	}

	return FALSE;
}

BOOL CChatSession::OnPush(const Hashes::Guid& oGUID, CConnection* pConnection)
{
	CQuickLock pLock( ChatCore.m_pSection );

	if ( m_tPushed == 0 ) return FALSE;
	if ( m_nProtocol == PROTOCOL_ED2K || m_nProtocol == PROTOCOL_DC ) return FALSE;
	if ( ! m_oGUID || validAndUnequal( m_oGUID, oGUID ) ) return FALSE;

	if ( m_nState > cssConnecting ) return FALSE;
	if ( m_nState > cssNull ) Close();

	CConnection::AttachTo( pConnection );

	StatusMessage( cmtStatus, IDS_CHAT_PUSH_DONE, (LPCTSTR)HostToString( &m_pHost ) );	// Was m_sAddress
	ChatCore.Add( this );
	OnConnected();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession close

void CChatSession::Close(UINT nError)
{
	CQuickLock pLock( ChatCore.m_pSection );

	if ( m_nState != cssNull )
	{
		m_nState = cssNull;
		StatusMessage( cmtStatus, IDS_CHAT_CLOSED );
	}

	// Delete all users
	DeleteUser( NULL );

	// Moved to OnCloseWindow()
	//if ( m_nProtocol == PROTOCOL_DC )
	//{
	//	CSingleLock pLock( &Network.m_pSection );
	//	if ( pLock.Lock( 250 ) )
	//	{
	//		if ( CNeighbour* pNeighbour = Neighbours.Get( m_pHost.sin_addr ) )
	//		{
	//			pNeighbour->Close();
	//		}
	//	}
	//}

	CConnection::Close( nError );

	if ( m_pWndPrivate == NULL )
		delete this;
}

//////////////////////////////////////////////////////////////////////
// CChatSession connection handler

BOOL CChatSession::OnConnected()
{
	StatusMessage( cmtStatus, IDS_CHAT_CONNECTED );

	// ED2K connections aren't handled here- they are in ED2KClient
	if ( m_nProtocol == PROTOCOL_ED2K || m_nProtocol == PROTOCOL_DC )
		return TRUE;

	CConnection::OnConnected();

	m_nState		= cssRequest2;
	m_tConnected	= GetTickCount();

	// "CHAT CONNECT/0.2\r\n"
	// "Accept: text/plain,application/x-gnutella2\r\n"
	// "User-Agent: Envy\r\n"

	if ( m_bOld != TRI_FALSE )
		Write( _P("CHAT CONNECT/0.1\r\n") );
	else
		Write( _P("CHAT CONNECT/0.2\r\n") );

	Write( _P("Accept: text/plain,application/x-gnutella2\r\n") );

	Write( _P("User-Agent: ") );
	Write( Settings.SmartAgent() );
	Write( _P("\r\n") );

	if ( m_bInitiated ) SendMyAddress();
	Write( _P("\r\n") );

	LogOutgoing();

	OnWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession disconnection handler

void CChatSession::OnDropped()
{
	if ( m_nState == cssConnecting )
	{
		StatusMessage( cmtError, IDS_CHAT_CANT_CONNECT, (LPCTSTR)HostToString( &m_pHost ) );	// Was m_sAddress
		if ( m_tPushed == 0 && SendPush( TRUE ) ) return;
	}
	else if ( m_nState != cssNull )
	{
		StatusMessage( cmtError, IDS_CHAT_DROPPED, (LPCTSTR)HostToString( &m_pHost ) );
	}

	Close();
}

//////////////////////////////////////////////////////////////////////
// CChatSession run handler

BOOL CChatSession::OnRun()
{
	ProcessMessages();

	// ED2K chat sessions don't have real connections, ED2K Client just puts the packets into the buffer.
	if ( m_nProtocol == PROTOCOL_ED2K )
		return ( ReadED2K() && SendED2K() );
	if ( m_nProtocol == PROTOCOL_DC )
		return ( ReadDC() && SendDC() );

	if ( m_nState > cssNull && m_nState < cssActive )
	{
		DWORD nDelay = GetTickCount() - m_tConnected;

		if ( nDelay >= ( m_nState == cssConnecting ?
			Settings.Connection.TimeoutConnect : Settings.Connection.TimeoutHandshake ) )
		{
			theApp.Message( MSG_ERROR, IDS_HANDSHAKE_TIMEOUT, (LPCTSTR)HostToString( &m_pHost ) );	// Was m_sAddress
			Close();
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession read handler

BOOL CChatSession::OnRead()
{
	// ED2K connections aren't handled here, this shouldn't ever be called for ed2k sessions
	if ( m_nProtocol == PROTOCOL_ED2K || m_nProtocol == PROTOCOL_DC )
		return TRUE;

	CConnection::OnRead();

	switch ( m_nState )
	{
	case cssRequest1:
	case cssRequest2:
	case cssRequest3:
		return ReadHandshake();
	case cssHeaders1:
	case cssHeaders2:
	case cssHeaders3:
		return ReadHeaders();
	case cssHandshake:
	case cssActive:
//	case cssAway:
		if ( m_nProtocol == PROTOCOL_G2 )
			return ReadG2();
		else
			return ReadG1();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession handshake processor

BOOL CChatSession::ReadHandshake()
{
	CString strLine;
	if ( ! Read( strLine ) || strLine.IsEmpty() )
		return TRUE;

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"%s >> CHAT HANDSHAKE: %s", (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	if ( ::StartsWith( strLine, _P( L"CHAT CONNECT/" ) ) && m_nState == cssRequest1 )
	{
		m_bOld = ( strLine.Find( L"/0.1" ) > 0 ) ? TRI_TRUE : TRI_FALSE;

		m_nState = cssHeaders1;
		return TRUE;
	}
	else if ( ::StartsWith( strLine, _P( L"CHAT/" ) ) )
	{
		m_bOld = ( strLine.Find( L"/0.1" ) > 0 ) ? TRI_TRUE : TRI_FALSE;

		if ( _tcsistr( strLine, L"200 OK" ) )
		{
			if ( m_nState == cssRequest2 )
			{
				m_nState = cssHeaders2;
				return TRUE;
			}
			if ( m_nState == cssRequest3 )
			{
				m_nState = cssHeaders3;
				return TRUE;
			}
		}
		//else // Rejected
	}

	StatusMessage( cmtError, IDS_CHAT_PRIVATE_REFUSED, (LPCTSTR)HostToString( &m_pHost ) );	// Was m_sAddress
	Close();

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession header processor

BOOL CChatSession::OnHeaderLine(CString& strHeader, CString& strValue)
{
	ASSERT( m_nProtocol != PROTOCOL_ED2K && m_nProtocol != PROTOCOL_DC );

	if ( ! CConnection::OnHeaderLine( strHeader, strValue ) )
		return FALSE;

	if ( strHeader.CompareNoCase( L"X-Nickname" ) == 0 )
		m_sNick = strValue;

	return TRUE;
}

BOOL CChatSession::OnHeadersComplete()
{
	if ( m_nState != cssHeaders3 )
	{
		// Guessing
		if ( m_nProtocol == PROTOCOL_ANY )
			m_nProtocol = PROTOCOL_G1;
		else if ( m_nProtocol == PROTOCOL_G2 )
			m_bOld = TRI_FALSE;

		if ( m_bOld == TRI_TRUE )
			Write( _P("CHAT/0.1 200 OK\r\n") );
		else
			Write( _P("CHAT/0.2 200 OK\r\n") );

		if ( m_nProtocol == PROTOCOL_G2 )
		{
			Write( _P("Accept: application/x-gnutella2\r\n") );
			Write( _P("Content-Type: application/x-gnutella2\r\n") );
		}

		if ( MyProfile.IsValid() )
		{
			Write( _P("X-Nickname: ") );
			Write( MyProfile.GetNick().Left( 255 ) );
			Write( _P("\r\n") );
		}

		Write( _P("User-Agent: ") );
		Write( Settings.SmartAgent() );
		Write( _P("\r\n\r\n") );

		LogOutgoing();

		OnWrite();
	}

	if ( m_nState == cssHeaders1 )
	{
		// Sent second handshake
		m_nState = cssRequest3;
		return TRUE;
	}
	if ( m_nState == cssHeaders2 || m_nState == cssHeaders3 )
	{
		// Sent third handshake, or Connected
		m_nState = cssHandshake;
		return OnEstablished();
	}

	ASSERT( FALSE );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession startup

BOOL CChatSession::OnEstablished()
{
	m_tConnected = GetTickCount();

	if ( m_nProtocol == PROTOCOL_G2 )
	{
		StatusMessage( cmtStatus, IDS_CHAT_HANDSHAKE_G2 );
		Send( CG2Packet::New( G2_PACKET_PROFILE_CHALLENGE ) );
	}
	else if ( m_nProtocol == PROTOCOL_G1 )
	{
		m_nState = cssActive;
		NotifyMessage( cmtProfile, m_sNick );
		StatusMessage( cmtInfo, IDS_CHAT_HANDSHAKE_G1, ( m_bOld == TRI_TRUE ) ? L"0.1" : L"0.2" );
		//if ( m_pWndPrivate ) m_pWndPrivate->PostMessage( WM_CHAT_PROFILE_RECEIVED );	// OnProfileReceived()
		//StatusMessage( 0, 0 );
		//PostOpenWindow();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession DC++ packet interface

BOOL CChatSession::ReadDC()
{
	ASSERT( m_nProtocol == PROTOCOL_DC );
	ASSERT( IsInputExist() );

	BOOL bSuccess = TRUE;
	while ( CDCPacket* pPacket = CDCPacket::ReadBuffer( GetInput() ) )
	{
		bSuccess = OnChatMessage( pPacket );

		pPacket->Release();
		if ( ! bSuccess ) break;
	}

	return bSuccess;
}

BOOL CChatSession::SendDC()
{
	ASSERT( m_nProtocol == PROTOCOL_DC );
	ASSERT( IsOutputExist() );

	while ( CDCPacket* pPacket = CDCPacket::ReadBuffer( GetOutput() ) )
	{
		ASSERT( pPacket != NULL );

		if ( ! Send( pPacket ) )
		{
			Write( pPacket );

			pPacket->Release();
			break;
		}
	}

	return TRUE;
}

BOOL CChatSession::Send(CDCPacket* pPacket)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( pLock.Lock( 250 ) )
	{
		if ( CNeighbour* pNeighbour = Neighbours.Get( m_pHost.sin_addr ) )
		{
			MakeActive( FALSE );
			return pPacket ? pNeighbour->Send( pPacket ) : TRUE;
		}
		else if ( m_nState == cssConnecting )
		{
			// Still connecting...
			return TRUE;
		}
		else if ( Neighbours.ConnectTo( m_pHost.sin_addr, ntohs( m_pHost.sin_port ), PROTOCOL_DC ) )
		{
			m_nState = cssConnecting;
			m_tConnected = GetTickCount();
			StatusMessage( cmtStatus, IDS_CHAT_CONNECTING_TO, (LPCTSTR)HostToString( &m_pHost ) );
		}
		else
		{
			m_nState = cssNull;
			StatusMessage( cmtError, IDS_CHAT_CANT_CONNECT, (LPCTSTR)HostToString( &m_pHost ) );
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CChatSession::OnChatMessage(CDCPacket* pPacket)
{
	// Note: The message packet has already been validated by the DCClient or DCNeighbour.

	if ( *pPacket->m_pBuffer == '<' )
	{
		CString strMsg( UTF8Decode( (LPCSTR)&pPacket->m_pBuffer[ 1 ], pPacket->m_nLength - 1 - 1 ) );
		int nPos = strMsg.Find( L'>' );
		OnChatMessage( strMsg.Left( nPos ), strMsg.Mid( nPos + 2 ) );
	}
	else if ( pPacket->Compare( _P("$HubTopic ") ) )
	{
		CString strTopic( UTF8Decode( (LPCSTR)&pPacket->m_pBuffer[ 10 ], pPacket->m_nLength - 10 - 1 ) );
		NotifyMessage( cmtCaption, m_sNick, strTopic );
	}
	else if ( pPacket->Compare( _P("$HubName ") ) )
	{
		CString strTopic( UTF8Decode( (LPCSTR)&pPacket->m_pBuffer[ 9 ], pPacket->m_nLength - 9 - 1 ) );
		NotifyMessage( cmtCaption, m_sNick, strTopic );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession ED2K packet interface

// The ED2K packets are put into the input/output buffers, then handled by ReadED2K()
// and SendED2K().  Those functions are called by the OnRun(), since there isn't a valid
// connection/socket associated with an ED2K client chat session. (It's handled by the EDClient)

BOOL CChatSession::ReadED2K()
{
	ASSERT( m_nProtocol == PROTOCOL_ED2K );
	ASSERT( IsInputExist() );

	BOOL bSuccess = TRUE;

	while ( CEDPacket* pPacket = CEDPacket::ReadBuffer( GetInput() ) )	// CLockedBuffer
	{
		try
		{
			// Note: This isn't a "real" packet parser. Message packets are simply dumped into
			// the input buffer by the EDClient, so all packets should be valid ED2K chat messages.
			switch ( pPacket->m_nType )
			{
			case ED2K_C2C_MESSAGE:
				bSuccess = OnChatMessage( pPacket );
				break;
			case ED2K_C2C_CHATCAPTCHAREQ:
				bSuccess = OnCaptchaRequest( pPacket );
				break;
			case ED2K_C2C_CHATCAPTCHARES:
				bSuccess = OnCaptchaResult( pPacket );
				break;
			default:
				DEBUG_ONLY( pPacket->Debug( L"Unknown ED2K Chat packet from " + m_sAddress ) );
			}
		}
		catch ( CException* pException )
		{
			pException->Delete();
			if ( ! m_oGUID )
				bSuccess = FALSE;
		}

		pPacket->Release();
		if ( ! bSuccess ) break;
	}

	return bSuccess;
}

BOOL CChatSession::SendED2K()
{
	ASSERT( m_nProtocol == PROTOCOL_ED2K );
	ASSERT( IsOutputExist() );

	while ( CEDPacket* pPacket = CEDPacket::ReadBuffer( GetOutput() ) )		// CLockedBuffer
	{
		ASSERT( pPacket != NULL );

		// Send the message to the appropriate ED2K Client
		if ( ! Send( pPacket ) )
		{
			// Put the packet back into the buffer until we are ready to deal with it
			Write( pPacket );
			// We're done with the packet (for now), so release it.
			pPacket->Release();
			break;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession ED2K packet handlers

BOOL CChatSession::Send(CEDPacket* pPacket)
{
	// Lock the transfers while we send a message (We need the EDClient)
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	// Try to find an ed2k client
	CEDClient* pClient = EDClients.GetByIP( &m_pHost.sin_addr );

	if ( pClient && validAndEqual( pClient->m_oGUID, m_oGUID ) )	// Found a client
	{
		if ( pClient->IsOnline() )		// We found a client that's ready to go
		{
			MakeActive( FALSE );

			// Send the packet to the ed2k client, and report the packet should be removed
			pClient->Send( pPacket );

			return TRUE;
		}
		else if ( m_nState != cssConnecting && pClient->Connect() )		// We found a client we need to connect to
		{
			// Set the 'connection' state while we wait for EDClient to do it's job
			m_nState = cssConnecting;
			m_tConnected = GetTickCount();
			StatusMessage( cmtStatus, IDS_CHAT_CONNECTING_TO, (LPCTSTR)HostToString( &pClient->m_pHost ) );		// pClient->m_sAddress
			// Return false to out the packet back into the buffer until we're ready to send it
			return FALSE;
		}
		else if ( m_nState == cssConnecting )		// We found a client but couldn't start a connection, we are connecting
		{
			// Check time-out
			if ( ( GetTickCount() - m_tConnected ) >= Settings.Connection.TimeoutConnect )
			{
				// We've timed out. Display an error and drop the message
				StatusMessage( cmtError, IDS_CHAT_CANT_CONNECT, (LPCTSTR)HostToString( &pClient->m_pHost ) );			// pClient->m_sAddress
				m_nState = cssNull;
				return TRUE;
			}

			// Waiting to connect. Put the packet back into the buffer and try later.
			return FALSE;
		}
		else	// We found a client but can't connect
		{
			// There is a problem.  Inform the user and drop the message.
			StatusMessage( cmtError, IDS_CHAT_CANT_CONNECT, (LPCTSTR)HostToString( &pClient->m_pHost ) );		// pClient->m_sAddress
			m_nState = cssNull;
			return TRUE;
		}
	}
	else // We don't seem to have a client that matches.
	{
		// Make a new client/connection if we can
		if ( m_nState != cssConnecting )
		{
			// We need to connect to them, so either find or create an EDClient
			if ( m_bMustPush )
				pClient = EDClients.Connect( m_pHost.sin_addr.s_addr, ntohs( m_pHost.sin_port ), &m_pServer.sin_addr, ntohs( m_pServer.sin_port ), m_oGUID );
			else
				pClient = EDClients.Connect( m_pHost.sin_addr.s_addr, ntohs( m_pHost.sin_port ), NULL, 0, m_oGUID );
			// If we weren't able to create a client (Low-id and no server), then exit.

			if ( pClient && pClient->Connect() )
			{
				pClient->OpenChat();
				// Set the 'connection' state while we wait for EDClient to do it's job
				m_nState = cssConnecting;
				m_tConnected = GetTickCount();
				StatusMessage( cmtStatus, IDS_CHAT_NOT_CONNECTED_1 );

				// Return false to out the packet back into the buffer until we're ready to send it
				return FALSE;
			}
		}

		// Inform the user and drop the message.
		StatusMessage( cmtError, IDS_CHAT_DROPPED );
		m_nState = cssNull;
		return TRUE;
	}
}

BOOL CChatSession::OnChatMessage(CEDPacket* pPacket)
{
	// Note: The message packet has already been validated by the EDClient.

	// Read message length
	DWORD nMessageLength = pPacket->ReadShortLE();

	OnChatMessage( m_sNick, m_bUnicode ?
		pPacket->ReadStringUTF8( nMessageLength ) : pPacket->ReadStringASCII( nMessageLength ) );

	// Obsolete: Display message
	//if ( m_pWndPrivate ) m_pWndPrivate->PostMessage( WM_CHAT_REMOTE_MESSAGE, FALSE, (LPARAM)psMessage.Detach() );

	return TRUE;
}

BOOL CChatSession::OnCaptchaRequest(CEDPacket* pPacket)
{
	// Note: The message packet has already been validated by the EDClient.

	// Skip tags
	for ( BYTE nCount = pPacket->ReadByte() ; nCount && pPacket->GetRemaining() ; --nCount )
	{
		CEDTag pTag;
		pTag.Read( pPacket );
	}

	// Load bitmap
	CImageFile imgCaptcha;
	if ( imgCaptcha.LoadFromMemory( L".bmp", (LPCVOID)( pPacket->m_pBuffer + pPacket->m_nPosition ), pPacket->GetRemaining() ) &&
		 imgCaptcha.EnsureRGB() )
	{
		NotifyMessage( cmtStatus, m_sNick, LoadString( IDS_CHAT_CAPTCHA_REQUEST ), imgCaptcha.CreateBitmap() );
	//	m_pWndPrivate->PostMessage( WM_CHAT_STATUS_MESSAGE, (WPARAM)0, (LPARAM)new CString( L"Confirm CAPTCHA:" ) );
	//	m_pWndPrivate->PostMessage( WM_CHAT_BITMAP_MESSAGE, 0, (LPARAM)imgCaptcha.CreateBitmap() );
	}

	return TRUE;
}

BOOL CChatSession::OnCaptchaResult(CEDPacket* pPacket)
{
	// Note: The message packet has already been validated by the EDClient.

	BYTE nStatus = pPacket->ReadByte();

	if ( nStatus == 0 )
		NotifyMessage( cmtStatus, m_sNick, LoadString( IDS_CHAT_CAPTCHA_ACCEPTED ) );
	else
		NotifyMessage( cmtError, m_sNick, LoadString( IDS_CHAT_CAPTCHA_DENIED ) );

	//	Was m_pWndPrivate->PostMessage( WM_CHAT_STATUS_MESSAGE, (WPARAM)0, (LPARAM)new CString( L"CAPTCHA accepted." ) );
	//	Was m_pWndPrivate->PostMessage( WM_CHAT_STATUS_MESSAGE, (WPARAM)1, (LPARAM)new CString( L"CAPTCHA refused." ) );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession text interface

BOOL CChatSession::ReadG1()
{
	//ASSERT( m_nProtocol == PROTOCOL_G1 );

	CString strLine;

	while ( Read( strLine, FALSE ) )
	{
		OnChatMessage( m_sNick, strLine );
	}

	return TRUE;
}

void CChatSession::OnChatMessage(const CString& sFrom, const CString& sMessage)
{
	if ( m_bOld == TRI_TRUE )	// G1
	{
		if ( ::StartsWith( sMessage, _P( L"\001ACTION " ) ) )
			NotifyMessage( cmtAction, sFrom, sMessage.Mid( 8 ) );
		else if ( ::StartsWith( sMessage, _P( L"* " ) ) )
			NotifyMessage( cmtAction, sFrom, sMessage.Mid( 2 ) );
		else if ( ::StartsWith( sMessage, _P( L"/me " ) ) )
			NotifyMessage( cmtAction, sFrom, sMessage.Mid( 4 ) );
		else
			NotifyMessage( cmtMessage, sFrom, sMessage );

		return;
	}

	if ( ::StartsWith( sMessage, _P( L"MESSAGE " ) ) )
	{
		if ( ::StartsWith( sMessage, _P( L"MESSAGE \001ACTION " ) ) )
			NotifyMessage( cmtAction, sFrom, sMessage.Mid( 16 ) );
		else
			NotifyMessage( cmtMessage, sFrom, sMessage.Mid( 8 ) );
	}
	else if ( ::StartsWith( sMessage, _P( L"NICK " ) ) )
	{
		// Note new nick is sMessage.Mid( 5 )
	}
	else
	{
		NotifyMessage( cmtMessage, sFrom, sMessage );
	}
}

//////////////////////////////////////////////////////////////////////
// CChatSession Gnutella2 packet interface

void CChatSession::Send(CG2Packet* pPacket)
{
	ASSERT( m_nProtocol == PROTOCOL_G2 );
	ASSERT( m_nState >= cssHandshake );

	Write( pPacket );
	pPacket->Release();

	OnWrite();
}

BOOL CChatSession::ReadG2()
{
	//ASSERT( m_nProtocol == PROTOCOL_G2 );

	CLockedBuffer pInput( GetInput() );

	BOOL bSuccess = TRUE;
	for ( ; bSuccess && pInput->m_nLength ; )
	{
		BYTE nInput = *( pInput->m_pBuffer );

		if ( nInput == 0 )
		{
			pInput->Remove( 1 );
			continue;
		}

		BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
		BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
		BYTE nFlags		= ( nInput & 0x07 );

	//	if ( nLenLen == 0 )
	//	{
	//		Close();
	//		return FALSE;
	//	}

		if ( (DWORD)pInput->m_nLength < (DWORD)nLenLen + nTypeLen + 2 ) break;

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
			BYTE* pLenIn  = pInput->m_pBuffer + 1;
			BYTE* pLenOut = (BYTE*)&nLength;
			for ( BYTE nLenCnt = nLenLen ; nLenCnt-- ; ) *pLenOut++ = *pLenIn++;
		}

		if ( nLength >= Settings.Gnutella.MaximumPacket )
		{
			Close();
			return FALSE;
		}

		if ( (DWORD)pInput->m_nLength < (DWORD)nLength + nLenLen + nTypeLen + 2 ) break;

		CG2Packet* pPacket = CG2Packet::New( pInput->m_pBuffer );

		pInput->Remove( nLength + nLenLen + nTypeLen + 2 );

		try
		{
			switch ( pPacket->m_nType )
			{
			case G2_PACKET_CHAT_MESSAGE:
				return OnChatMessage( pPacket );
			case G2_PACKET_PROFILE_CHALLENGE:
				return OnProfileChallenge( pPacket );
			case G2_PACKET_PROFILE_DELIVERY:
				return OnProfileDelivery( pPacket );
			case G2_PACKET_CHAT_REQUEST:
				return OnChatRequest( pPacket );
			case G2_PACKET_CHAT_ANSWER:
				return OnChatAnswer( pPacket );
			//case G2_PACKET_CHAT_AWAY:
			//	return TRUE;	// ToDo: Away?
#ifdef _DEBUG
			default:
				{
					CString str;
					str.Format( L"Unknown G2 Chat packet from %s", HostToString( &m_pHost ) );
					pPacket->Debug( str );
				}
#endif	// Debug
			}
		}
		catch ( CException* pException )
		{
			pException->Delete();
			bSuccess = TRUE;
		}

		pPacket->Release();
	}

	if ( ! bSuccess ) Close();

	return bSuccess;
}

BOOL CChatSession::OnProfileChallenge(CG2Packet* /*pPacket*/)
{
	if ( ! MyProfile.IsValid() ) return TRUE;

	CG2Packet* pProfile = CG2Packet::New( G2_PACKET_PROFILE_DELIVERY, TRUE );
	CString strXML = MyProfile.GetPublicXML( /*m_sUserAgent*/ NULL, TRUE )->ToString( TRUE );

	pProfile->WritePacket( G2_PACKET_XML, pProfile->GetStringLen( strXML ) );
	pProfile->WriteString( strXML, FALSE );

	Send( pProfile );

	return TRUE;
}

BOOL CChatSession::OnProfileDelivery(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	delete m_pProfile;
	m_pProfile = NULL;

	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_XML )
		{
			CXMLElement* pXML = CXMLElement::FromString( pPacket->ReadString( nLength ), TRUE );

			if ( pXML != NULL )
			{
				m_pProfile = new CGProfile();

				if ( m_pProfile == NULL )
				{
					//theApp.Message( MSG_ERROR, L"Error in CChatSession::OnProfileDelivery()" );
					delete pXML;
				}
				else if ( ! m_pProfile->FromXML( pXML ) || ! m_pProfile->IsValid() )
				{
					delete pXML;
					delete m_pProfile;
					m_pProfile = NULL;
				}
			}
		}

		pPacket->m_nPosition = nOffset;
	}

	if ( m_pProfile == NULL ) return TRUE;

	m_sNick = m_pProfile->GetNick();

	m_oGUID = m_pProfile->oGUID;

	NotifyMessage( cmtProfile, m_sNick );
	//m_pWndPrivate->PostMessage( WM_CHAT_PROFILE_RECEIVED );		// OnProfileReceived()

	if ( CG2Packet* pReqPacket = CG2Packet::New( G2_PACKET_CHAT_REQUEST, TRUE ) )
	{
		pReqPacket->WritePacket( G2_PACKET_USER_GUID, 16 );
		pReqPacket->Write( m_oGUID );
		Send( pReqPacket );
	}

	return TRUE;
}

BOOL CChatSession::OnChatRequest(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	Hashes::Guid oGUID;

	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_USER_GUID && nLength >= 16 )
			pPacket->Read( oGUID );

		pPacket->m_nPosition = nOffset;
	}

	CG2Packet* pAnswer = CG2Packet::New( G2_PACKET_CHAT_ANSWER, TRUE );
	pAnswer->WritePacket( G2_PACKET_USER_GUID, 16 );
	const Hashes::Guid oMyGUID = MyProfile.oGUID;
	pAnswer->Write( oMyGUID );

	if ( validAndEqual( oGUID, oMyGUID ) )
	{
		DWORD nIdle = (DWORD)time( NULL ) - theApp.m_nLastInput;

		if ( nIdle > Settings.Community.AwayMessageIdleTime )
		{
			CString strTime;
			if ( nIdle > 86400 )
				strTime.Format( L"%u:%.2u:%.2u:%.2u", nIdle / 86400, ( nIdle / 3600 ) % 24, ( nIdle / 60 ) % 60, nIdle % 60 );
			else
				strTime.Format( L"%u:%.2u:%.2u", nIdle / 3600, ( nIdle / 60 ) % 60, nIdle % 60 );

			pAnswer->WritePacket( G2_PACKET_CHAT_AWAY, pAnswer->GetStringLen( strTime ) );
			pAnswer->WriteString( strTime, FALSE );
		}
		else
		{
			pAnswer->WritePacket( G2_PACKET_CHAT_ACCEPT, 0 );
		}

	//	PostOpenWindow();
	}
	else
	{
		pAnswer->WritePacket( G2_PACKET_CHAT_DENY, 0 );
	}

	Send( pAnswer );

	return TRUE;
}

BOOL CChatSession::OnChatAnswer(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound )
		return TRUE;

	BOOL ret = TRUE;
	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		switch ( nType )
		{
		case G2_PACKET_USER_GUID:
			if ( nLength >= 16 )
			{
				Hashes::Guid oGUID;
				pPacket->Read( oGUID );
				if ( oGUID )
					m_oGUID = oGUID;
			}
			break;

		case G2_PACKET_CHAT_ACCEPT:
			MakeActive();
			break;

		case G2_PACKET_CHAT_DENY:
			StatusMessage( cmtError, IDS_CHAT_PRIVATE_REFUSED, (LPCTSTR)m_sNick );
			ret = FALSE;	// Close connection
			break;

		case G2_PACKET_CHAT_AWAY:
			MakeActive();	// m_nState = cssAway;
			StatusMessage( cmtError, IDS_CHAT_PRIVATE_AWAY, (LPCTSTR)m_sNick, pPacket->ReadString( nLength ) );
			break;
		}
		pPacket->m_nPosition = nOffset;
	}

	return ret;
}

BOOL CChatSession::OnChatMessage(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound )
		return TRUE;

	BOOL bAction = FALSE;
	CString strBody;
	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		switch ( nType )
		{
		case G2_PACKET_BODY:
			strBody = pPacket->ReadString( nLength );
			break;

		case G2_PACKET_CHAT_ACTION:
			bAction = TRUE;
			break;
		}

		pPacket->m_nPosition = nOffset;
	}

	if ( strBody.IsEmpty() || ! m_pWndPrivate )
		return TRUE;

	// Obsolete for reference and deletion
	//CAutoPtr< CString > psBody( new CString );
	//*psBody = strBody;
	//m_pWndPrivate->PostMessage( WM_CHAT_REMOTE_MESSAGE, bAction, (LPARAM)psBody.Detach() );

	NotifyMessage( ( bAction ? cmtAction : cmtMessage ), m_sNick, strBody );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession message interface

BOOL CChatSession::SendPrivateMessage(bool bAction, const CString& strText)
{
	CSingleLock pLock( &ChatCore.m_pSection, TRUE );

	if ( m_nState < cssActive )
		return FALSE;

	//if ( m_nState == cssAway )
	//{
	//	CString strMessage;
	//	CString strNick = MyProfile.GetNick();
	//	if ( strNick.IsEmpty() ) strNick = CLIENT_NAME L" User";
	//	strMessage.Format( LoadString( IDS_CHAT_PRIVATE_AWAY ), strNick, strText );
	//}

	if ( m_nProtocol == PROTOCOL_ED2K )
	{
		// Limit outgoing ed2k messages to shorter than ED2K_MESSAGE_MAX characters, just in case
		CString strMessage = strText.Left( ED2K_MESSAGE_MAX - 50 );

		// Create an ed2k packet holding the message
		if ( CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_MESSAGE ) )
		{
			if ( m_bUnicode )
			{
				pPacket->WriteShortLE( WORD( pPacket->GetStringLenUTF8( strMessage ) ) );
				pPacket->WriteStringUTF8( strMessage, FALSE );
			}
			else
			{
				pPacket->WriteShortLE( WORD( pPacket->GetStringLen( strMessage ) ) );
				pPacket->WriteString( strMessage, FALSE );
			}

			//ASSERT( pPacket->m_nEdProtocol == ED2K_PROTOCOL_EDONKEY );

			Write( pPacket );

			pPacket->Release();
		}

		return TRUE;
	}

	if ( m_nProtocol == PROTOCOL_DC )
	{
		CSingleLock pNetworkLock( &Network.m_pSection );
		if ( pNetworkLock.Lock( 250 ) )
		{
			if ( CNeighbour* pClient = Neighbours.Get( m_pHost.sin_addr ) )
			{
				if ( pClient->m_nProtocol == PROTOCOL_DC )
				{
					if ( CDCPacket* pPacket = CDCPacket::New() )
					{
						pPacket->WriteByte( '<' );
						pPacket->WriteString( static_cast< CDCNeighbour* >( pClient )->m_sNick, FALSE );
						pPacket->WriteByte( '>' );
						pPacket->WriteByte( ' ' );
						if ( bAction ) pPacket->WriteString( L"/me ", FALSE );
						pPacket->WriteString( strText, FALSE );
						pPacket->WriteByte( '|' );

						Write( pPacket );

						pPacket->Release();
					}
					return TRUE;
				}
			}
		}

		return FALSE;
	}

	if ( GetConnectedState() != TRI_TRUE )
	{
		StatusMessage( cmtError, IDS_CHAT_NOT_CONNECTED_1 );
		Command( ID_CHAT_CONNECT );
		return FALSE;
	}

	if ( m_bOld == TRI_TRUE )
	{
		if ( bAction )
			Write( L"* " + strText + L"\r\n", CP_UTF8 );
		else
			Write( strText + L"\r\n", CP_UTF8 );
	}
	else if ( m_nProtocol == PROTOCOL_G1 )
	{
		if ( bAction )
			Write( L"MESSAGE \001ACTION " + strText + L"\r\n" );
		else
			Write( L"MESSAGE " + strText + L"\r\n" );
	}
	else if ( m_nProtocol == PROTOCOL_G2 )
	{
		if ( CG2Packet* pPacket = CG2Packet::New( G2_PACKET_CHAT_MESSAGE, TRUE ) )
		{
			if ( bAction )
				pPacket->WritePacket( G2_PACKET_CHAT_ACTION, 0 );

			pPacket->WritePacket( G2_PACKET_BODY, pPacket->GetStringLen( strText ) );
			pPacket->WriteString( strText, FALSE );

			Send( pPacket );
		}
	}

	OnWrite();

	return TRUE;
}

//bool CChatSession::SendAwayMessage(const CString& strText)
//{
//	int nOldState = m_nState;
//	m_nState = cssAway;
//	bool bResult = SendPrivateMessage( false, strText );
//	m_nState = nOldState;
//
//	return bResult;
//}

//////////////////////////////////////////////////////////////////////
// CChatSession status message

void CChatSession::StatusMessage(MessageType bType, UINT nID, ...)
{
	CString strMessage;
	CString strFormat( LoadString( nID ) );
	va_list pArgs;

	va_start( pArgs, nID );

	if ( strFormat.Find( L"%1" ) >= 0 )
	{
		LPTSTR lpszTemp = NULL;
		if ( ::FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			strFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &pArgs ) != 0 && lpszTemp != NULL )
		{
			strMessage = lpszTemp;
			LocalFree( lpszTemp );
		}
	}
	else
	{
		strMessage.FormatV( strFormat, pArgs );
	}

	va_end( pArgs );

	NotifyMessage( bType, m_sNick, strMessage );

	// Obsolete:
	//if ( ! m_pWndPrivate ) return;
	//CAutoPtr< CString > psMessage( new CString );
	//*psMessage = strMessage;
	//m_pWndPrivate->PostMessage( WM_CHAT_STATUS_MESSAGE, (WPARAM)nFlags, (LPARAM)psMessage.Detach() );
}

void CChatSession::NotifyMessage(MessageType bType, const CString& sFrom, const CString& sMessage, HBITMAP hBitmap)
{
	CQuickLock oLock( ChatCore.m_pSection );

	MSG oMsg = { NULL, WM_CHAT_MESSAGE, 0, (LPARAM)new CChatMessage( bType, sFrom, sMessage, hBitmap ) };
	m_pMessages.AddTail( oMsg );

	// Obsolete:
	//if ( m_pWndPrivate )
	//	m_pWndPrivate->PostMessage( WM_CHAT_MESSAGE, 0, (LPARAM)new CChatMessage( bType, sFrom, sMessage, hBitmap ) );
	//else if ( hBitmap )
	//	DeleteObject( hBitmap );
}

void CChatSession::AddUser(CChatUser* pUser)
{
	CQuickLock oLock( ChatCore.m_pSection );

	MSG oMsg = { NULL, WM_CHAT_ADD_USER, 0, (LPARAM)pUser };
	m_pMessages.AddTail( oMsg );
}

void CChatSession::DeleteUser(CString* pUser)
{
	CQuickLock oLock( ChatCore.m_pSection );

	MSG oMsg = { NULL, WM_CHAT_DELETE_USER, 0, (LPARAM)pUser };
	m_pMessages.AddTail( oMsg );
}

void CChatSession::Command(UINT nCommand)
{
	CQuickLock oLock( ChatCore.m_pSection );

	MSG oMsg = { NULL, WM_COMMAND, nCommand };
	m_pMessages.AddTail( oMsg );
}

void CChatSession::ProcessMessages()
{
	if ( ! m_pWndPrivate )
		return;

	CQuickLock oLock( ChatCore.m_pSection );

	if ( ! m_pWndPrivate )
		return;

	while ( ! m_pMessages.IsEmpty() )
	{
		MSG oMsg = m_pMessages.RemoveHead();
		m_pWndPrivate->PostMessage( oMsg.message, oMsg.wParam, oMsg.lParam );
	}
}

void CChatSession::ClearMessages()
{
	CQuickLock oLock( ChatCore.m_pSection );

	while ( ! m_pMessages.IsEmpty() )
	{
		MSG oMsg = m_pMessages.RemoveHead();
		switch ( oMsg.message )
		{
		case WM_CHAT_MESSAGE:
			if ( CChatMessage* pMsg = (CChatMessage*)oMsg.lParam )
			{
				if ( pMsg->m_hBitmap ) DeleteObject( pMsg->m_hBitmap );
				delete pMsg;
			}
			break;

		case WM_CHAT_ADD_USER:
			delete (CChatUser*)oMsg.lParam;
			break;

		case WM_CHAT_DELETE_USER:
			delete (CString*)oMsg.lParam;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CChatSession chat window interface

// Obsolete:
//void CChatSession::PostOpenWindow()
//{
//	CQuickLock oLock( ChatCore.m_pSection );
//	if ( m_pWndPrivate != NULL ) return;
//	PostMainWndMessage( WM_OPENCHAT, (WPARAM)this );
//}

void CChatSession::OnOpenWindow()
{
	CQuickLock oLock( ChatCore.m_pSection );

	if ( m_pWndPrivate != NULL ) return;

	if ( m_oGUID )
	{
		m_pWndPrivate = ChatWindows.FindPrivate( m_oGUID, false );
		if ( ! m_pWndPrivate )
			m_pWndPrivate = ChatWindows.FindPrivate( m_oGUID, true );
	}
	else
	{
		m_pWndPrivate = ChatWindows.FindPrivate( &m_pHost );
	}

	if ( m_pWndPrivate == NULL && m_nProtocol == PROTOCOL_ED2K )
	{
		if ( m_bMustPush )
			m_pWndPrivate = ChatWindows.FindED2KFrame( m_nClientID, &m_pServer );
		else
			m_pWndPrivate = ChatWindows.FindED2KFrame( &m_pHost );
	}

	if ( m_pWndPrivate == NULL )
		m_pWndPrivate = new CPrivateChatWnd();

	if ( ! m_pWndPrivate->Accept( this ) )
	{
		m_pWndPrivate = new CPrivateChatWnd();
		m_pWndPrivate->Accept( this );
	}

	if ( m_nProtocol == PROTOCOL_DC )
	{
		CSingleLock pLock( &Network.m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CNeighbour* pClient = Neighbours.Get( m_pHost.sin_addr ) )
			{
				if ( pClient->m_nProtocol == PROTOCOL_DC )
					static_cast< CDCNeighbour* >( pClient )->OnChatOpen( this );
			}
		}

		MakeActive( FALSE );
	}
	else
	{
		MakeActive();
	}

	// Obsolete:
	//NotifyMessage( cmtProfile, m_sNick );
	//m_pWndPrivate->PostMessage( WM_CHAT_PROFILE_RECEIVED );		// OnProfileReceived()
	//
	//StatusMessage( cmtInfo, IDS_CHAT_PRIVATE_ONLINE, (LPCTSTR)m_sNick );
	//StatusMessage( 0, 0 );
	//
	//PlaySound( L"Sound_IncomingChat", NULL, SND_APPLICATION|SND_ALIAS|SND_ASYNC );
	//m_nState = cssActive;
	//
	// Hack to open it
	//CWnd* pParent = m_pWndPrivate->GetParent();
	//if ( pParent->IsIconic() )
	//	pParent->ShowWindow( SW_SHOWNORMAL );
	//pParent->BringWindowToTop();
	//pParent->SetForegroundWindow();
}

void CChatSession::OnCloseWindow()
{
	CQuickLock oLock( ChatCore.m_pSection );

	ChatCore.Remove( this );

	ClearMessages();

	m_pWndPrivate = NULL;

	// Moved from Close()
	if ( m_nProtocol == PROTOCOL_DC )
	{
		CSingleLock pLock( &Network.m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CNeighbour* pNeighbour = Neighbours.Get( m_pHost.sin_addr ) )
			{
				pNeighbour->Close();
			}
		}
	}

	Close();
}
