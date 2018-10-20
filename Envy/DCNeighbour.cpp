//
// DCNeighbour.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2010 and PeerProject 2010-2015
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
#include "DCNeighbour.h"
#include "DCPacket.h"
#include "DCClient.h"
#include "DCClients.h"
#include "Neighbours.h"
#include "Network.h"
#include "ChatCore.h"
#include "HostCache.h"
#include "LibraryMaps.h"
#include "LocalSearch.h"
#include "Security.h"
#include "GProfile.h"
#include "UploadQueue.h"
#include "UploadQueues.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CDCNeighbour::CDCNeighbour()
	: CNeighbour	( PROTOCOL_DC )
	, m_bExtended	( FALSE )
	, m_bNickValid	( FALSE )
{
	m_nNodeType = ntHub;

	m_oUsers.InitHashTable( GetBestHashTableSize( 3000 ) );
}

CDCNeighbour::~CDCNeighbour()
{
	ChatCore.OnDropped( this );

	RemoveAllUsers();
}

void CDCNeighbour::RemoveAllUsers()
{
	for ( POSITION pos = m_oUsers.GetStartPosition(); pos; )
	{
		CString strNick;
		CChatUser* pUser;
		m_oUsers.GetNextAssoc( pos, strNick, pUser );
		delete pUser;
	}
	m_oUsers.RemoveAll();
}

BOOL CDCNeighbour::ConnectToMe(const CString& sNick)
{
	// $ConnectToMe RemoteNick SenderIp:SenderPort|

	if ( m_nState != nrsConnected )
		return FALSE;	// Too early

	ASSERT( ! sNick.IsEmpty() );

	CString strRequest;
	strRequest.Format( L"$ConnectToMe %s %s|",
		(LPCTSTR)DCClients.CreateNick( sNick ), (LPCTSTR)HostToString( &Network.m_pHost ) );

	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->WriteString( strRequest, FALSE );

		Send( pPacket );
	}

	return TRUE;
}

CChatUser* CDCNeighbour::GetUser(const CString& sNick) const
{
	CChatUser* pUser;
	return m_oUsers.Lookup( sNick, pUser ) ? pUser : NULL;
}

void CDCNeighbour::OnChatOpen(CChatSession* pSession)
{
	// (Re)fill chat user list
	for ( POSITION pos = m_oUsers.GetStartPosition(); pos; )
	{
		CString strNick;
		CChatUser* pUser;
		m_oUsers.GetNextAssoc( pos, strNick, pUser );

		pSession->AddUser( new CChatUser( *pUser ) );
	}
}

BOOL CDCNeighbour::ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic)
{
	CString strHost( inet_ntoa( *pAddress ) );

	if ( CConnection::ConnectTo( pAddress, nPort ) )
	{
		WSAEventSelect( m_hSocket, Network.GetWakeupEvent(), FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE );

		theApp.Message( MSG_INFO, IDS_CONNECTION_ATTEMPTING, (LPCTSTR)strHost, htons( m_pHost.sin_port ) );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_CONNECTION_CONNECT_FAIL, (LPCTSTR)strHost );
		return FALSE;
	}

	m_nState = nrsConnecting;

	m_bAutomatic = bAutomatic;

	Neighbours.Add( this );

	return TRUE;
}

BOOL CDCNeighbour::OnRead()
{
	CNeighbour::OnRead();

	return ProcessPackets();
}

BOOL CDCNeighbour::ProcessPackets()
{
	CLockedBuffer pInputLocked( GetInput() );

	if ( m_pZInput && m_pZInput->m_nLength == 0 )
	{
		if ( m_bZInputEOS )
		{
			// Got "End of Stream" so turn decompression off
			m_bZInputEOS = FALSE;
			CBuffer::InflateStreamCleanup( m_pZSInput );
			delete m_pZInput;
			m_pZInput = NULL;
		}
		return TRUE;
	}

	CBuffer* pInput = m_pZInput ? m_pZInput : (CBuffer*)pInputLocked;

	return ProcessPackets( pInput );
}

BOOL CDCNeighbour::ProcessPackets(CBuffer* pInput)
{
	if ( ! pInput )
		return FALSE;

	BOOL bSuccess = TRUE;

	while ( CDCPacket* pPacket = CDCPacket::ReadBuffer( pInput ) )
	{
		try
		{
			bSuccess = OnPacket( pPacket );
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}

		pPacket->Release();
		if ( ! bSuccess )
			break;

		if ( m_pZInput && m_pZInput != pInput )
			break;		// Compression just turned on
	}

	return bSuccess;
}

BOOL CDCNeighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL /*bBuffered*/)
{
	m_nOutputCount++;

	if ( m_pZOutput )
		pPacket->ToBuffer( m_pZOutput );
	else
		Write( pPacket );

	QueueRun();

	pPacket->SmartDump( &m_pHost, FALSE, TRUE, (DWORD_PTR)this );

	if ( bRelease ) pPacket->Release();

	return TRUE;
}

BOOL CDCNeighbour::SendUserInfo()
{
	// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|

	DWORD nUploadSlots = 0;
	if ( CUploadQueue* pQueue = UploadQueues.SelectQueue( PROTOCOL_DC, NULL, 0, CUploadQueue::ulqBoth, NULL ) )
	{
		nUploadSlots = pQueue->m_nMaxTransfers;
	}

	QWORD nMyVolume = 0;
	LibraryMaps.GetStatistics( NULL, &nMyVolume );

	CString strInfo;
	strInfo.Format( L"$MyINFO $ALL %s %s<%s V:%s,M:%c,H:%u/%u/%u,S:%u>$ $%.2f%c$%s$%I64u$|",
		(LPCTSTR)m_sNick,								// Registered nick
		WEB_SITE,										// Description
		CLIENT_NAME,									// Client name
		(LPCTSTR)theApp.m_sVersion,						// Client version
		( Network.IsFirewalled( CHECK_IP ) ? L'P' : L'A' ),	// User is in active(A), passive(P), or SOCKS5(5) mode
		Neighbours.GetCount( PROTOCOL_DC, nrsConnected, ntHub ),	// Number of connected hubs as regular user
		0,												// Number of connected hubs as VIP
		0,												// Number of connected hubs as operator
		nUploadSlots,									// Number of upload slots
		(float)Settings.Bandwidth.Uploads * Bytes / ( Kilobits * Kilobits ),	// Upload speed (Mbit/s)
		1,												// User status: Normal(1), Away(2,3), Server(4,5), Server Away(6,7)
		(LPCTSTR)MyProfile.GetContact( L"Email" ),	// E-mail
		nMyVolume << 10 );								// Share size (bytes)

	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->WriteString( strInfo, FALSE );

		return Send( pPacket );
	}

	return FALSE;
}

BOOL CDCNeighbour::OnConnected()
{
	if ( ! CNeighbour::OnConnected() )
		return FALSE;

	theApp.Message( MSG_INFO, IDS_CONNECTION_CONNECTED, (LPCTSTR)HostToString( &m_pHost ) );

	m_nState = nrsHandshake1;	// Waiting for $Lock

	return TRUE;
}

void CDCNeighbour::OnDropped()
{
	RemoveAllUsers();

	if ( m_nState == nrsConnecting )
		Close( IDS_CONNECTION_REFUSED );
	else
		Close( IDS_CONNECTION_DROPPED );
}

// DC++ Command handling:

BOOL CDCNeighbour::OnPacket(CDCPacket* pPacket)
{
	pPacket->SmartDump( &m_pHost, FALSE, FALSE, (DWORD_PTR)this );

	m_nInputCount++;
	m_tLastPacket = GetTickCount();

	if ( pPacket->m_nLength < 2 )		// "|"
		return OnPing();
	if ( *pPacket->m_pBuffer == '<' )	// <Nick> Message|
		return OnChat( pPacket );
	if ( *pPacket->m_pBuffer != '$' )
		return OnUnknown( pPacket );

	if ( pPacket->Compare( _P("$Search ") ) )		// $Search SenderIP:SenderPort (F|T)?(F|T)?Size?Type?String|
		return OnQuery( pPacket );
	if ( pPacket->Compare( _P("$To: ") ) )
		return OnChatPrivate( pPacket );
	if ( pPacket->Compare( _P("$HubTopic ") ) )		// $HubTopic <topic>|
		return OnHubTopic( pPacket );
	if ( pPacket->Compare( _P("$HubName ") ) )		// $HubName Title [Version]|
		return OnHubName( pPacket );

	// Convert '|' to '\0' (make ASCIIZ)
	pPacket->m_pBuffer[ pPacket->m_nLength - 1 ] = 0;
	LPCSTR szCommand = (LPCSTR)pPacket->m_pBuffer;

	// Split off parameters
	LPSTR szParams = strchr( (LPSTR)pPacket->m_pBuffer, ' ' );
	if ( szParams )
		*szParams++ = 0;

	if ( strcmp( szCommand, "$MyINFO" ) == 0 )			// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|
		return OnUserInfo( szParams );
	if ( strcmp( szCommand, "$Hello" ) == 0 )			// $Hello Nick
		return OnHello( szParams );
	if ( strcmp( szCommand, "$Quit" ) == 0 )			// $Quit nick|
		return OnQuit( szParams );
	if ( strcmp( szCommand, "$Lock" ) == 0 )			// $Lock [EXTENDEDPROTOCOL]Challenge Pk=Vendor
		return OnLock( szParams );
	if ( strcmp( szCommand, "$Supports" ) == 0 )		// $Supports [option1]...[optionN]
		return OnSupports( szParams );
	if ( strcmp( szCommand, "$OpList" ) == 0 )			// $OpList operator1$$operator2|
		return OnOpList( szParams );
//	if ( strcmp( szCommand, "$BotList" ) == 0 )			// $BotList bot1$$bot2|
//		return OnBotList( szParams );
//	if ( strcmp( szCommand, "$NickList" ) == 0 )		// $NickList user1$$user2|
//		return OnNickList( szParams );
	if ( strcmp( szCommand, "$ConnectToMe" ) == 0 )		// $ConnectToMe SenderNick RemoteNick SenderIp:SenderPort|
		return OnConnectToMe( szParams );
	if ( strcmp( szCommand, "$RevConnectToMe" ) == 0 )	// $RevConnectToMe
		return OnRevConnectToMe( szParams );
	if ( strcmp( szCommand, "$ForceMove" ) == 0 )		// $ForceMove IP:Port|
		return OnForceMove( szParams );
	if ( strcmp( szCommand, "$ValidateDenide" ) == 0 )	// $ValidateDenide Nick|
		return OnValidateDenide();
	if ( strcmp( szCommand, "$GetPass" ) == 0 )			// $GetPass
		return OnGetPass();
	if ( strcmp( szCommand, "$UserIP" ) == 0 )			// $UserIP MyNick IP|
		return OnUserIP( szParams );
	if ( strcmp( szCommand, "$ZOn" ) == 0 ) 			// $ZOn		(Zip compression)
		return OnZOn();

	return OnUnknown( pPacket );
}

// Obsolete:
//{
//	// Custom SwitchMap( Command ) for raw non-unicode
//	static std::map < std::string, char > Command;
//	if ( Command.empty() )
//	{
//		Command[ "$Search" ]		= 's';
//		Command[ "$MyINFO" ]		= 'i';
//		Command[ "$Quit" ] 			= 'q';
//		Command[ "$Lock" ] 			= 'l';
//		Command[ "$Supports" ]		= 'r';
//		Command[ "$Hello" ]			= 'h';
//		Command[ "$HubName" ]		= 'b';
//		Command[ "$HubTopic" ]		= 'p';
//		Command[ "$OpList" ]		= 'o';
//	//	Command[ "$BotList" ]		= 'n';
//		Command[ "$NickList" ] 		= 'n';
//		Command[ "$ForceMove" ]		= 'm';
//		Command[ "$ConnectToMe" ]	= 'c';
//		Command[ "$ValidateDenide" ] = 'v';	// Not "Denied"
//		Command[ "$GetPass" ]		= 'v';
//		Command[ "$UserIP" ]		= 'u';
//		Command[ "$To:" ]			= 't';
//		Command[ "$ZOn" ]			= 'z';
//
//	//	Command[ "$HubIsFull" ]		= 'x';
//	//	Command[ "$MCTo" ]			= 'x';
//	//	Command[ "$MyPass" ]		= 'x';
//	//	Command[ "$BadPass" ]		= 'x';
//	//	Command[ "$RevConnectToMe" ] = 'x';
//	//	Command[ "$Version" ]		= 'x';
//	}
//
//	switch ( Command[ strCommand ] )
//	{
//	case 's':		// $Search SenderIP:SenderPort (F|T)?(F|T)?Size?Type?String|
//		{
//			std::string::size_type nPos = strParams.find( ' ' );
//			if ( nPos != std::string::npos )
//			{
//				std::string strAddress = strParams.substr( 0, nPos );
//				std::string strSearch  = strParams.substr( nPos + 1 );
//				nPos = strAddress.find( ':' );
//				if ( nPos != std::string::npos )
//				{
//					const DWORD nAddress = inet_addr( strAddress.substr( 0, nPos ).c_str() );
//					const int nPort = atoi( strAddress.substr( nPos + 1 ).c_str() );
//					if ( nPort > 0 && nPort <= USHRT_MAX && nAddress != INADDR_NONE &&
//						! Network.IsFirewalledAddress( (const IN_ADDR*)&nAddress ) &&
//						! Network.IsReserved( (const IN_ADDR*)&nAddress ) &&
//						! Security.IsDenied( (const IN_ADDR*)&nAddress ) )
//					{
//						OnSearch( (const IN_ADDR*)&nAddress, (WORD)nPort, strSearch );
//					}
//				}
//			}
//		}
//		return TRUE;
//	case 'i':		// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|
//		// Handle user info?
//		m_nState = nrsConnected;
//		return TRUE;
//	case 'q':		// $Quit nick|
//		// Handle user leave hub?
//		return TRUE;
//	case 'h':		// $Hello Nick
//		// User logged-in
//		m_nState = nrsConnected;
//		m_sNick = CA2CT( strParams.c_str() );
//		if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
//			pServer->m_sUser = m_sNick;
//		return OnHello();
//	case 'b':		// $HubName Title [Version]|
//		m_nState = nrsConnected;
//		m_sServerName = CA2CT( strParams.c_str() );
//		if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
//			pServer->m_sName = m_sServerName;
//		return TRUE;
//	case 't':		// $HubTopic <topic>|
//		theApp.Message( MSG_NOTICE, L"DC++ hub topic at %s:  %s", (LPCTSTR)m_sServerName, (LPCTSTR)CA2CT( strParams.c_str() ) );
//		return TRUE;
//	case 'o':		// $OpList operator1$$operator2|
//		// Handle hub operators list, etc.?
//		m_nState = nrsConnected;
//		return TRUE;
//	case 'n':		// $NickList user1$$user2|
//		// Handle hub lists?  (ToDo: Could get user count, not sent currently)
//		DEBUG_ONLY( theApp.Message( MSG_DEBUG, L"DC++ $NickList received, %i char long from %s", strParams.length(), (LPCTSTR)m_sServerName ) );
//		return TRUE;
//	case 'l':		// $Lock [EXTENDEDPROTOCOL]Challenge Pk=Vendor
//		m_bExtended = ( strParams.substr( 0, 16 ) == "EXTENDEDPROTOCOL" );
//		{
//			std::string strLock;
//			std::string::size_type nPos = strParams.find( " Pk=" );
//			if ( nPos != std::string::npos )
//			{
//				// Good way
//				strLock = strParams.substr( 0, nPos );
//				m_sUserAgent = strParams.substr( nPos + 4 ).c_str();
//			}
//			else
//			{
//				// Bad way
//				nPos = strParams.find( ' ' );
//				if ( nPos != std::string::npos )
//					strLock = strParams.substr( 0, nPos );
//				else	// Very bad way
//					strLock = strParams;
//			}
//			return OnLock( strLock );
//		}
//	case 'r':		// $Supports [option1]...[optionN]
//		m_bExtended = TRUE;
//		m_oFeatures.RemoveAll();
//		for ( CString strFeatures( strParams.c_str() ); ! strFeatures.IsEmpty(); )
//		{
//			CString strFeature = strFeatures.SpanExcluding( L" " );
//			strFeatures = strFeatures.Mid( strFeature.GetLength() + 1 ).MakeLower();
//			if ( strFeature.IsEmpty() )
//				continue;
//			if ( m_oFeatures.Find( strFeature ) == NULL )
//				m_oFeatures.AddTail( strFeature );
//		}
//		return TRUE;
//	case 'c':		// $ConnectToMe SenderNick RemoteNick SenderIp:SenderPort|
//		// Client connection request
//		{
//			std::string::size_type nPos = strParams.rfind( ' ' );
//			if ( nPos != std::string::npos )
//			{
//				std::string strAddress = strParams.substr( nPos + 1 );
//				std::string strSenderNick, strRemoteNick = strParams.substr( 0, nPos );
//				nPos = strRemoteNick.find( ' ' );
//				if ( nPos != std::string::npos )
//				{
//					strSenderNick = strRemoteNick.substr( nPos + 1 );
//					strRemoteNick = strRemoteNick.substr( 0, nPos );
//				}
//				nPos = strAddress.find( ':' );
//				if ( nPos != std::string::npos )
//				{
//					const DWORD nAddress = inet_addr( strAddress.substr( 0, nPos ).c_str() );
//					const int nPort = atoi( strAddress.substr( nPos + 1 ).c_str() );
//					if ( nPort > 0 && nPort <= USHRT_MAX && nAddress != INADDR_NONE &&
//						m_sNick == strRemoteNick.c_str() &&
//						! Network.IsFirewalledAddress( (const IN_ADDR*)&nAddress ) &&
//						! Network.IsReserved( (const IN_ADDR*)&nAddress ) )
//					{
//						// Ok
//						if ( CDCClient* pClient = new CDCClient( m_sNick ) )
//						pClient->ConnectTo( (const IN_ADDR*)&nAddress, (WORD)nPort );
//					}
//				//	else
//				//		// Wrong nick, bad IP
//				}
//			}
//		}
//		return TRUE;
//	case 'm':		// $ForceMove IP:Port|
//		// User redirection
//		{
//			CString strAddress;
//			int nPort = 0;
//			std::string::size_type nPos = strParams.rfind( ':' );
//			if ( nPos != std::string::npos )
//			{
//				strAddress = strParams.substr( 0, nPos ).c_str();
//				nPort = atoi( strParams.substr( nPos + 1 ).c_str() );
//			}
//			else
//				strAddress = strParams.c_str();
//
//			Network.ConnectTo( strAddress, nPort, PROTOCOL_DC );
//		}
//		return TRUE;
//	case 'v':		// $ValidateDenide Nick|	OR	$GetPass
//		// Bad user nick
//		m_sNick.Format( CLIENT_NAME L"%04u", GetRandomNum( 0u, 9999u ) );
//		if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
//			pServer->m_sUser = m_sNick;
//		if ( CDCPacket* pPacket = CDCPacket::New() )
//		{
//			pPacket->WriteString( L"$ValidateNick " + m_sNick + L"|", FALSE );
//			Send( pPacket );
//		}
//		return TRUE;
//	case 'u':		// $UserIP MyNick IP|
//		// User address
//		{
//			std::string::size_type nPos = strParams.find( ' ' );
//			if ( nPos != std::string::npos )
//			{
//				std::string strNick = strParams.substr( 0, nPos );
//				if ( m_sNick == strNick.c_str() )
//					Network.AcquireLocalAddress( CA2CT( strParams.substr( nPos + 1 ).c_str() ) );
//			}
//		}
//		return TRUE;
//	default:
//		if ( strCommand[ 0 ] != '$' )	// <Nick> Message|
//			return OnChat( strCommand + strParams );
//		// Unknown command
//	}
//
//	// Unknown command - ignoring
//	return TRUE;
//}

BOOL CDCNeighbour::OnUnknown(CDCPacket* pPacket)
{
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"%s >> UNKNOWN COMMAND: %s",
		(LPCTSTR)HostToString( &m_pHost ), (LPCTSTR)CString( (LPCSTR)pPacket->m_pBuffer, (int)pPacket->m_nLength ) );

	return TRUE;
}

BOOL CDCNeighbour::OnPing()
{
	// Ping
	// |

	return TRUE;
}

BOOL CDCNeighbour::OnChat(CDCPacket* pPacket)
{
	// Chat message
	// <Nick> Message|

	if ( LPCSTR szMessage = strchr( (LPCSTR)pPacket->m_pBuffer, '>' ) )
	{
		int nNickLen = szMessage - (LPCSTR)pPacket->m_pBuffer - 1;
		CString sNick( UTF8Decode( (LPCSTR)&pPacket->m_pBuffer[ 1 ], nNickLen ) );

		if ( nNickLen > 0 && m_sNick != sNick )
			ChatCore.OnMessage( this, pPacket );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnChatPrivate(CDCPacket* pPacket)
{
	// Private chat message
	// $To: my_nick From: nick$<nick> message|

	int nChat = pPacket->Find( '$', 5 );
	if ( nChat != -1 )
	{
		nChat++;
		if ( pPacket->GetAt( nChat ) == '<' )
		{
			int nMessage = pPacket->Find( '>', nChat );
			if ( nMessage != -1 )
			{
				// Cut off chat message and restore ending "|"
				pPacket->Remove( nChat );

				// ToDo: Implement DC++ private chat - for now just show privates as regular chat

				ChatCore.OnMessage( this, pPacket );
			}
		}
	}
	return TRUE;
}

BOOL CDCNeighbour::OnQuery(CDCPacket* pPacket)
{
	// Search request
	// Active user / Passive user
	// $Search SenderIP:SenderPort (T|F)?(T|F)?Size?Type?String|
	// $Search Hub:Nick (T|F)?(T|F)?Size?Type?String|

	CQuerySearchPtr pSearch = CQuerySearch::FromPacket( pPacket, NULL, TRUE );
	if ( ! pSearch || pSearch->m_bDropMe )
	{
		if ( ! pSearch )
		{
			DEBUG_ONLY( pPacket->Debug( L"Malformed Query." ) );
			theApp.Message( MSG_WARNING, IDS_PROTOCOL_BAD_QUERY, L"DC++", (LPCTSTR)m_sAddress );
		}
		m_nDropCount++;
		return TRUE;
	}

	pSearch->m_pMyHub = m_pHost;
	pSearch->m_sMyHub = m_sServerName;
	pSearch->m_sMyNick = m_sNick;

	if ( pSearch->m_bUDP )
	{
		if ( Security.IsDenied( &pSearch->m_pEndpoint.sin_addr ) )
		{
			m_nDropCount++;
			return TRUE;
		}

		Network.OnQuerySearch( new CLocalSearch( pSearch, PROTOCOL_DC ) );
	}
	else
	{
		Network.OnQuerySearch( new CLocalSearch( pSearch, this ) );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnLock(LPSTR szParams)
{
	// $Lock [EXTENDEDPROTOCOL]Challenge Pk=Vendor|

	if ( LPSTR szLock = szParams )
	{
		m_bExtended = ( strncmp( szParams, _P("EXTENDEDPROTOCOL") ) == 0 );

		if ( LPSTR szUserAgent = strstr( szParams, " Pk=" ) )
		{
			// Good way
			*szUserAgent = 0;
			szUserAgent += 4;
			m_sUserAgent = UTF8Decode( szUserAgent );
		}
		else if ( LPSTR szUserAgentAlt = strchr( szParams, ' ' ) )
		{
			// Bad way
			*szUserAgentAlt++ = 0;
			m_sUserAgent = UTF8Decode( szUserAgentAlt);
		}

		if ( m_nState < nrsHandshake2 )
			m_nState = nrsHandshake2;	// Waiting for $Hello

		if ( m_nNodeType == ntHub )
			HostCache.DC.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );

		if ( m_bExtended )
		{
			if ( CDCPacket* pPacket = CDCPacket::New() )
			{
				pPacket->Write( _P( DC_HUB_SUPPORTS ) );		// "$Supports ..."
				Send( pPacket );
			}
		}

		std::string strKey = DCClients.MakeKey( szLock );
		if ( CDCPacket* pPacket = CDCPacket::New() )
		{
			pPacket->Write( _P("$Key ") );
			pPacket->Write( strKey.c_str(), (DWORD)strKey.size() );
			pPacket->Write( _P("|") );
			Send( pPacket );
		}

		m_bNickValid = FALSE;

		if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
		{
			m_sNick = pServer->m_sUser;
		}

		m_sNick = DCClients.CreateNick( m_sNick );

		if ( CDCPacket* pPacket = CDCPacket::New() )
		{
			pPacket->Write( _P("$ValidateNick ") );
			pPacket->WriteString( m_sNick, FALSE );
			pPacket->Write( _P("|") );
			Send( pPacket );
		}
	}

	return TRUE;
}

BOOL CDCNeighbour::OnSupports(LPSTR szParams)
{
	// $Supports [option1]...[optionN]|

	m_bExtended = TRUE;

	m_oFeatures.RemoveAll();
	for ( CString strFeatures( szParams ); ! strFeatures.IsEmpty(); )
	{
		CString strFeature = strFeatures.SpanExcluding( L" " );
		strFeatures = strFeatures.Mid( strFeature.GetLength() + 1 );
		if ( strFeature.IsEmpty() )
			continue;
		if ( m_oFeatures.Find( strFeature ) == NULL )
			m_oFeatures.AddTail( strFeature );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnHello(LPSTR szNick)
{
	// User logged-in
	// $Hello Nick|

	m_nState = nrsConnected;

	m_bNickValid = TRUE;
	m_sNick = UTF8Decode( szNick );

	if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
	{
		pServer->m_sUser = m_sNick;
		HostCache.DC.m_nCookie++;
	}

	// NMDC version
	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->Write( _P("$Version 1,0091|") );
		Send( pPacket );
	}

	SendUserInfo();

	// Request nick list
	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->Write( _P("$GetNickList|") );
		Send( pPacket );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnHubName(CDCPacket *pPacket)
{
	// Name of hub
	// $HubName Title [Description]|

	CString strDescription;
	int nHubInfo = pPacket->Find( ' ', 9 );
	if ( nHubInfo != -1 )
	{
		m_sServerName  = UTF8Decode( (LPCSTR)&pPacket->m_pBuffer[ 9 ], nHubInfo - 9 );
		strDescription = UTF8Decode( (LPCSTR)&pPacket->m_pBuffer[ nHubInfo + 1 ], pPacket->m_nLength - nHubInfo - 2 ).TrimLeft( L" -" );
	}
	else
		m_sServerName = UTF8Decode( (LPCSTR)&pPacket->m_pBuffer[ 9 ], pPacket->m_nLength - 9 - 1 );

	if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
	{
		pServer->m_sName = m_sServerName;
		pServer->m_sDescription = strDescription;
		HostCache.DC.m_nCookie++;
	}

	ChatCore.OnMessage( this, pPacket );

	return TRUE;
}

BOOL CDCNeighbour::OnHubTopic(CDCPacket* pPacket)
{
	// Topic of hub
	// $HubTopic topic|

	ChatCore.OnMessage( this, pPacket );

	return TRUE;
}

BOOL CDCNeighbour::OnOpList(LPSTR /*szParams*/)
{
	// Hub operators list
	// $OpList operator1|

	// ToDo: Implement DC++ operator list

	return TRUE;
}

BOOL CDCNeighbour::OnUserInfo(LPSTR szInfo)
{
	// User info
	// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|

	if ( strncmp( szInfo, _P("$ALL ") ) == 0 )
	{
		LPSTR szNick = szInfo + 5;
		if ( LPSTR szDescription = strchr( szNick, ' ' ) )
		{
			*szDescription++ = 0;

			CString strNick( UTF8Decode( szNick ) );

			CChatUser* pUser;
			if ( ! m_oUsers.Lookup( strNick, pUser ) )
			{
				pUser = new CChatUser;
				m_oUsers.SetAt( strNick, pUser );
			}
			pUser->m_bType = ( strNick == m_sNick ) ? cutMe : cutUser;
			pUser->m_sNick = strNick;

			if ( LPSTR szConnection = strchr( szDescription, '$' ) )
			{
				*szConnection++ = 0;

				if ( LPSTR szVendor = strchr( szDescription, '<' ) )
				{
					if ( *(szConnection - 2) == '>' )
					{
						*szVendor++ = 0;
						*(szConnection - 2) = 0;

						CStringA sVersion;
						if ( LPSTR szTags = strchr( szVendor, ' ' ) )
						{
							*szTags++ = 0;

							for ( CStringA sTags( szTags ); ! sTags.IsEmpty(); )
							{
								CStringA sTag = sTags.SpanExcluding( "," );
								sTags = sTags.Mid( sTag.GetLength() + 1 );
								if ( sTag.IsEmpty() )
									continue;
								int nPos = sTag.Find( ':' );
								if ( nPos > 0 )
								{
									CStringA strTagName = sTag.Left( nPos );
									sTag = sTag.Mid( nPos + 1 );

									if ( strTagName == "V" )		// Version
										sVersion = sTag;
									//else if ( strTagName == "M" )	// Mode
									//	;
									//else if ( strTagName == "H" )	// Hubs
									//	;
									//else if ( strTagName == "S" )	// Slots
									//	;
								}
							}
						}

						pUser->m_sUserAgent = UTF8Decode( szVendor );
						if ( ! sVersion.IsEmpty() )
							pUser->m_sUserAgent += L" " + UTF8Decode( sVersion );
					}
				}
			}

			pUser->m_sDescription = UTF8Decode( szDescription );

			if ( m_nNodeType == ntHub )
				HostCache.DC.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ), 0, 0, 0, GetUserCount() );

			// Notify chat window
			ChatCore.OnAddUser( this, new CChatUser( *pUser ) );
		}
	}

	return TRUE;
}

BOOL CDCNeighbour::OnUserIP(LPSTR szIP)
{
	// User address
	// $UserIP MyNick IP|

	if ( LPSTR szMyNick = szIP )
	{
		if ( LPSTR szAddress = strchr( szMyNick, ' ' ) )
		{
			*szAddress++ = 0;

			CString strNick( UTF8Decode( szMyNick ) );

			if ( m_bNickValid && m_sNick == strNick )
			{
				IN_ADDR nAddress;
				nAddress.s_addr = inet_addr( szAddress );
				Network.AcquireLocalAddress( nAddress );
			}
		}
	}

	return TRUE;
}

BOOL CDCNeighbour::OnQuit(LPSTR szNick)
{
	// User leave hub
	// $Quit nick|

	if ( szNick )
	{
		CString strNick = UTF8Decode( szNick );
		CChatUser* pUser;
		if ( m_oUsers.Lookup( strNick, pUser ) )
		{
			m_oUsers.RemoveKey( strNick );
			delete pUser;
		}

		if ( m_nNodeType == ntHub )
			HostCache.DC.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ), 0, 0, 0, GetUserCount() );

		// Notify chat window
		ChatCore.OnDeleteUser( this, new CString( strNick ) );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnConnectToMe(LPSTR szParams)
{
	// Client connection request
	// $ConnectToMe MyNick SenderIp:SenderPort|
	// $ConnectToMe SenderNick MyNick SenderIp:SenderPort|

	if ( LPSTR szSenderNick = szParams )
	{
		if ( LPSTR szMyNick = strchr( szSenderNick, ' ' ) )
		{
			*szMyNick++ = 0;
			LPSTR szAddress = strchr( szMyNick, ' ' );
			if ( szAddress )
			{
				*szAddress++ = 0;
			}
			else
			{
				szAddress = szMyNick;
				szMyNick  = szSenderNick;
				szSenderNick = "";
			}

			CString strMyNick( UTF8Decode( szMyNick ) );
			CString strSenderNick( UTF8Decode( szSenderNick ) );

			if ( LPSTR szPort = strchr( szAddress, ':' ) )
			{
				*szPort++ = 0;
				int nPort = atoi( szPort );
				IN_ADDR nAddress;
				nAddress.s_addr = inet_addr( szAddress );
				if ( m_sNick == strMyNick )		// Ok
					DCClients.ConnectTo( &nAddress, (WORD)nPort, this, strSenderNick );
				//else
					// Wrong nick, bad IP
			}
		}
	}

	return TRUE;
}

BOOL CDCNeighbour::OnRevConnectToMe(LPSTR szParams)
{
	// Callback connection request
	// $RevConnectToMe RemoteNick MyNick|

	if ( LPSTR szRemoteNick = szParams )
	{
		if ( LPSTR szMyNick = strchr( szRemoteNick, ' ' ) )
		{
			*szMyNick++ = 0;

			CString strNick( UTF8Decode( szMyNick ) );
			CString strRemoteNick( UTF8Decode( szRemoteNick ) );

			if ( m_bNickValid && m_sNick == strNick )
				ConnectToMe( strRemoteNick );
		}
	}

	return TRUE;
}

BOOL CDCNeighbour::OnForceMove(LPSTR szParams)
{
	// User redirection
	// $ForceMove IP:Port|

	if ( LPSTR szAddress = szParams )
	{
		int nPort = protocolPorts[ PROTOCOL_DC ];
		if ( LPSTR szPort = strchr( szAddress, ':' ) )
		{
			*szPort++ = 0;
			nPort = atoi( szPort );
		}

		Network.ConnectTo( UTF8Decode( szAddress ), nPort, PROTOCOL_DC );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnValidateDenide()
{
	// Bad user nick  (Not "Denied")
	// $ValidateDenide[ Nick]|

	m_bNickValid = FALSE;
	m_sNick.Format( CLIENT_NAME L"%04u", GetRandomNum( 0u, 9999u ) );

	if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
	{
		pServer->m_sUser = m_sNick;
	}

	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->Write( _P("$ValidateNick ") );
		pPacket->WriteString( m_sNick, FALSE );
		pPacket->Write( _P("|") );
		Send( pPacket );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnGetPass()
{
	// Password request
	// $GetPass|

	// ToDo: Implement DC++ registered user support - for now just change nick

	return OnValidateDenide();
}

BOOL CDCNeighbour::OnZOn()
{
	// ZLib stream compression enabled
	// $ZOn|

	ASSERT( m_pZInput == NULL );
	m_pZInput = new CBuffer();

	return TRUE;
}
