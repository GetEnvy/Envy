//
// HostBrowser.cpp
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "HostBrowser.h"
#include "WndBrowseHost.h"
#include "Network.h"
#include "Buffer.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "EDPacket.h"
#include "EDClient.h"
#include "EDClients.h"
#include "Downloads.h"
#include "Neighbours.h"
#include "Transfers.h"
#include "QueryHit.h"
#include "SharedFile.h"
#include "EnvyURL.h"
#include "VendorCache.h"
#include "GProfile.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CHostBrowser construction

CHostBrowser::CHostBrowser(CBrowseHostWnd* pNotify, PROTOCOLID nProtocol, IN_ADDR* pAddress, WORD nPort, BOOL bMustPush, const Hashes::Guid& oClientID, const CString& sNick)
	: m_pNotify		( pNotify )
	, m_pProfile	( NULL )
	, m_bNewBrowse	( FALSE )
	, m_nPort		( nPort )
	, m_oClientID	( oClientID )
	, m_bMustPush	( bMustPush )
	, m_bCanPush	( m_oClientID.isValid() )
	, m_tPushed		( 0ul )
	, m_bConnect	( FALSE )
	, m_nHits		( 0 )
	, m_sNick		( sNick )
	, m_pVendor		( NULL )
	, m_bCanChat	( FALSE )
	, m_bDeflate	( FALSE )
	, m_nReceived	( 0ul )
	, m_pBuffer		( new CBuffer() )
	, m_pInflate	( NULL )
{
	if ( pAddress )
		m_pAddress = *pAddress;
	else
		m_pAddress.s_addr = INADDR_NONE;

	m_nProtocol = nProtocol;
}

CHostBrowser::~CHostBrowser()
{
	Stop();

	delete m_pProfile;
	delete m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser browse control

BOOL CHostBrowser::Browse()
{
	CQuickLock oTransfersLock( Transfers.m_pSection );

	m_sAddress = inet_ntoa( m_pAddress );
	m_sServer = protocolAbbr[ ( ( m_nProtocol == PROTOCOL_ANY ) ? PROTOCOL_NULL : m_nProtocol ) ];
	m_pVendor = VendorCache.Lookup( m_sServer );

	switch ( m_nProtocol )
	{
	case PROTOCOL_G2:
		Settings.Gnutella2.Enabled = true;
		break;
	case PROTOCOL_G1:
		Settings.Gnutella1.Enabled = true;
		break;
	case PROTOCOL_ED2K:
		Settings.eDonkey.Enabled = true;
		break;
	case PROTOCOL_DC:
		Settings.DC.Enabled = true;
		break;
	//default:
	}

	// ED2K Clients have their connection controlled by ED2KClient.
	// (One connection used for many things)
	if ( m_nProtocol == PROTOCOL_ED2K )
	{
		// Lock this object until we are finished with it
		CQuickLock oCEDClientsLock( EDClients.m_pSection );

		SOCKADDR_IN* pServer = NULL;	// ToDo: Add push connections
		CEDClient* pClient = EDClients.Connect( m_pAddress.s_addr, m_nPort,
			( pServer ? &pServer->sin_addr : NULL ),
			( pServer ? pServer->sin_port : 0 ), m_oClientID );

		if ( pClient && pClient->m_bConnected )
		{
			// Send browse request
			if ( CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_ASKSHAREDDIRS ) )
				pClient->Send( pPacket );
		}
		else if ( ! pClient || ! pClient->Connect() )
		{
			theApp.Message( MSG_NOTICE, IDS_BROWSE_CANT_CONNECT_TO, (LPCTSTR)m_sAddress );
			return FALSE;
		}
	}
	else if ( m_nProtocol == PROTOCOL_DC )
	{
		CEnvyURL oURL;
		oURL.m_nProtocol		= PROTOCOL_DC;
		oURL.m_nAction			= CEnvyURL::uriDownload;
		oURL.m_pServerAddress	= m_pAddress;
		oURL.m_nServerPort		= m_nPort;
		oURL.m_sLogin			= m_sNick;
		oURL.m_sName.Format( L"Files of %s.xml.bz2", (LPCTSTR)SafeFilename( m_sNick ) );
		oURL.m_sURL.Format( L"dchub://%s@%s:%u/files.xml.bz2", (LPCTSTR)URLEncode( m_sNick ), (LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort );

		return ( Downloads.Add( oURL ) != NULL );
	}
	else // G2/Gunetella
	{
		if ( IsValid() )
			return FALSE;

		if ( m_bMustPush )
		{
			if ( SendPush( FALSE ) )
			{
				theApp.Message( MSG_INFO, IDS_BROWSE_PUSHED_TO, (LPCTSTR)m_sAddress );
			}
			else
			{
				theApp.Message( MSG_NOTICE, IDS_BROWSE_CANT_PUSH_TO, (LPCTSTR)m_sAddress );
				return FALSE;
			}
		}
		else
		{
			if ( ConnectTo( &m_pAddress, m_nPort ) )
			{
				theApp.Message( MSG_INFO, IDS_BROWSE_CONNECTING_TO, (LPCTSTR)m_sAddress );
			}
			else
			{
				theApp.Message( MSG_NOTICE, IDS_BROWSE_CANT_CONNECT_TO, (LPCTSTR)m_sAddress );
				return FALSE;
			}
		}
	}

	m_nState = hbsConnecting;
	m_nHits  = 0;

	delete m_pProfile;
	m_pProfile = NULL;

	// Ensure window text is updated after state has been set to "connecting"
	m_pNotify->UpdateMessages();

	return TRUE;
}

void CHostBrowser::Stop(BOOL bCompleted)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( bCompleted )
	{
		if ( m_pProfile && m_pNotify != NULL )
			m_pNotify->OnProfileReceived();

		theApp.Message( MSG_NOTICE, IDS_BROWSE_FINISHED, m_sAddress, m_nHits );
	}
	else if ( IsValid() )
	{
		theApp.Message( MSG_INFO, IDS_BROWSE_CLOSED, m_sAddress );
	}

	m_nState  = hbsNull;
	m_tPushed = 0;

	// ED2K connections aren't handled here -they are in ED2KClient
	if ( m_nProtocol != PROTOCOL_ED2K && m_nProtocol != PROTOCOL_DC )
		CTransfer::Close();

	CBuffer::InflateStreamCleanup( m_pInflate );
}

BOOL CHostBrowser::IsBrowsing() const
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	return m_nState != hbsNull;
}

float CHostBrowser::GetProgress() const
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	if ( m_nState != hbsContent || m_nLength == 0 || m_nLength == SIZE_UNKNOWN )
		return 0.0f;

	return (float)( (double)m_nReceived / (double)m_nLength );
}

void CHostBrowser::OnQueryHits(CQueryHit* pHits)
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	m_bCanPush  = TRUE;
	m_oClientID = pHits->m_oClientID;

	for ( CQueryHit* pCount = pHits ; pCount ; pCount = pCount->m_pNext )
	{
		m_nHits++;
	}

	if ( ! m_bCanChat && pHits->m_bChat )
	{
		m_bCanChat = TRUE;

		if ( m_pProfile && m_pNotify )
			m_pNotify->OnProfileReceived();
	}

	if ( m_pNotify )
		m_pNotify->OnQueryHits( pHits );

	Network.OnQueryHits( pHits );
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser event handling

BOOL CHostBrowser::OnConnected()
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	CTransfer::OnConnected();

	SendRequest();

	return TRUE;
}

BOOL CHostBrowser::OnRead()
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	// ED2K connections aren't handled here- they are in ED2KClient
	if ( m_nProtocol == PROTOCOL_ED2K || m_nProtocol == PROTOCOL_DC ) return TRUE;

	if ( ! IsInputExist() || ! IsOutputExist() ) return TRUE;

	CTransfer::OnRead();

	switch ( m_nState )
	{
	case hbsRequesting:
		if ( ! ReadResponseLine() ) return FALSE;
		if ( m_nState != hbsHeaders ) break;

	case hbsHeaders:
		if ( ! ReadHeaders() ) return FALSE;
		if ( m_nState != hbsContent ) break;

	case hbsContent:
		return ReadContent();
	}

	return TRUE;
}

void CHostBrowser::OnDropped()
{
	if ( m_nProtocol != PROTOCOL_ED2K && m_nProtocol != PROTOCOL_DC )
	{
		if ( ! IsValid() ) return;

		if ( m_nState == hbsConnecting )
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_CANT_CONNECT_TO, m_sAddress );

			if ( ! m_tPushed && SendPush( TRUE ) )
				return;
		}
		else
		{
			if ( m_nLength == SIZE_UNKNOWN )
			{
				m_nLength = GetInputLength();
				ReadContent();
				return;
			}

			theApp.Message( MSG_ERROR, IDS_BROWSE_DROPPED, m_sAddress );
		}
	}

	Stop();
}

BOOL CHostBrowser::OnRun()
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	CTransfer::OnRun();

	const DWORD tNow = GetTickCount();

	switch ( m_nState )
	{
	case hbsConnecting:
		if ( tNow >= m_tConnected + Settings.Connection.TimeoutConnect * 2 )
		{
			OnDropped();
			return FALSE;
		}
		break;
	case hbsRequesting:
	case hbsHeaders:
		if ( tNow >= m_tConnected + Settings.Connection.TimeoutHandshake * 3 )
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_TIMEOUT, m_sAddress );
			Stop();
			return FALSE;
		}
		break;
	case hbsContent:
		if ( tNow >= m_mInput.tLast + Settings.Connection.TimeoutTraffic )
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_TIMEOUT, m_sAddress );
			Stop();
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser push handling

BOOL CHostBrowser::SendPush(BOOL bMessage)
{
	// ED2K connections aren't handled here- they are in ED2KClient
	if ( m_nProtocol == PROTOCOL_ED2K || m_nProtocol == PROTOCOL_DC ) return FALSE;

	if ( ! m_bCanPush ) return FALSE;

	if ( Network.SendPush( m_oClientID, 0 ) )
	{
		CTransfer::Close();
		m_tPushed = GetTickCount();

		if ( bMessage )
			theApp.Message( MSG_INFO, IDS_BROWSE_PUSHED_TO, m_sAddress );

		return TRUE;
	}

	return FALSE;
}

BOOL CHostBrowser::OnPush(const Hashes::Guid& oClientID, CConnection* pConnection)
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	// ED2K connections aren't handled here- they are in ED2KClient
	if ( m_nProtocol == PROTOCOL_ED2K || m_nProtocol == PROTOCOL_DC || m_tPushed == 0 ) return FALSE;
	if ( IsValid() || ! pConnection->IsValid() || ! validAndEqual( m_oClientID, oClientID ) ) return FALSE;

	AttachTo( pConnection );

	m_pAddress	= m_pHost.sin_addr;
	m_nPort		= htons( m_pHost.sin_port );

	SendRequest();

	return TRUE;
}

BOOL CHostBrowser::OnNewFile(const CLibraryFile* pFile)
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	if ( m_nProtocol == PROTOCOL_DC && ! m_sNick.IsEmpty() )
		return FALSE;

	CString strName;
	strName.Format( L"Files of %s.xml.bz2", (LPCTSTR)SafeFilename( m_sNick ) );
	if ( strName.CompareNoCase( pFile->m_sName ) != 0 )
		return FALSE;

	CQueryHit* pHits = NULL;

	if ( LoadDC( pFile->GetPath(), pHits ) )
	{
		if ( pHits != NULL )
			OnQueryHits( pHits );

		DeleteFileEx( pFile->GetPath(), TRUE, TRUE, TRUE );
	}

	return TRUE;
}

BOOL CHostBrowser::LoadDC(LPCTSTR pszFile, CQueryHit*& pHits)
{
	CFile pFile;
	if ( ! pFile.Open( pszFile, CFile::modeRead | CFile::shareDenyWrite ) )
		return FALSE;	// File open error

	UINT nInSize = (UINT)pFile.GetLength();
	if ( ! nInSize )
		return FALSE;	// Empty file

	CBuffer pBuffer;
	if ( ! pBuffer.EnsureBuffer( nInSize ) )
		return FALSE;	// Out of memory

	if ( pFile.Read( pBuffer.GetData(), nInSize ) != nInSize )
		return FALSE;	// File read error
	pBuffer.m_nLength = nInSize;

	if ( ! pBuffer.UnBZip() )
		return FALSE;	// Decompression error

	unique_ptr< CXMLElement > pXML ( CXMLElement::FromString( pBuffer.ReadString( pBuffer.m_nLength, CP_UTF8 ), TRUE ) );
	if ( ! pXML.get() )
		return FALSE;	// XML decoding error

	// <FileListing Version="1" CID="SKCB4ZF4PZUDF7RKQ5LX6SVAARQER7QEVELZ2TY" Base="/" Generator="DC++ 0.762">

	if ( ! pXML->IsNamed( L"FileListing" ) )
		return FALSE;	// Invalid XML file format

//	CString strTitle = pXML->GetAttributeValue( L"CID" );

	return LoadDCDirectory( pXML.get(), pHits );
}

BOOL CHostBrowser::LoadDCDirectory(CXMLElement* pRoot, CQueryHit*& pHits)
{
	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( L"Directory" ) )
		{
			// <Directory Name="Downloads">

			if ( ! LoadDCDirectory( pElement, pHits ) )
				return FALSE;
		}
		else if ( pElement->IsNamed( L"File" ) )
		{
			// <File Name="music.mp3" Size="100000" TTH="3A6D6T2NDRLU6BGSTSJNW3R3QWTV6A44M6AGXMA"/>

			CString strName = pElement->GetAttributeValue( L"Name" );
			QWORD nSize;
			if ( _stscanf( pElement->GetAttributeValue( L"Size" ), L"%I64i", &nSize ) != 1 )
				nSize = SIZE_UNKNOWN;
			CString strTiger = pElement->GetAttributeValue( L"TTH" );

			if ( strName.IsEmpty() || nSize == SIZE_UNKNOWN )
				return FALSE;

			if ( CQueryHit* pHit = new CQueryHit( PROTOCOL_DC ) )
			{
				pHit->m_sName		= strName;
				pHit->m_nSize		= nSize;
				pHit->m_bSize		= TRUE;
				pHit->m_bChat		= TRUE;
				pHit->m_bBrowseHost	= TRUE;
				pHit->m_oTiger.fromString( strTiger );
				pHit->m_sNick		= m_sNick;
				pHit->m_bBusy		= TRI_FALSE;
				pHit->m_bPush		= TRI_TRUE;
				pHit->m_pAddress	= m_pAddress;
				pHit->m_nPort		= m_nPort;
				pHit->m_sCountry	= theApp.GetCountryCode( m_pAddress );
				pHit->m_pVendor		= m_pVendor ? m_pVendor : VendorCache.m_pNull;

				pHit->Resolve();

				pHit->m_pNext = pHits;
				pHits = pHit;
			}
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser send request

void CHostBrowser::SendRequest()
{
	if ( m_nProtocol != PROTOCOL_ED2K && m_nProtocol != PROTOCOL_DC )
	{
		if ( ! IsValid() ) return;

		if ( m_bNewBrowse )
			Write( _P("GET /gnutella/browse/v1 HTTP/1.1\r\n") );
		else if ( Settings.Downloads.RequestHTTP11 )
			Write( _P("GET / HTTP/1.1\r\n") );
		else
			Write( _P("GET / HTTP/1.0\r\n") );

		CString strHeader = Settings.SmartAgent();

		if ( ! strHeader.IsEmpty() )
		{
			Write( _P("User-Agent: ") );
			Write( strHeader );
			Write( _P("\r\n") );
		}

		Write( _P("Accept: text/html, application/x-gnutella-packets, application/x-gnutella2\r\n") );

		Write( _P("Accept-Encoding: deflate\r\n") );
		Write( _P("Connection: close\r\n") );

		strHeader.Format( L"Host: %s:%lu\r\n\r\n", (LPCTSTR)m_sAddress, htons( m_pHost.sin_port ) );
		Write( strHeader );

		LogOutgoing();

		OnWrite();

		m_nProtocol = PROTOCOL_ANY;
	}

	m_nState	= hbsRequesting;
	m_bDeflate	= FALSE;
	m_nLength	= SIZE_UNKNOWN;
	m_bConnect	= TRUE;

	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Downloads;

	theApp.Message( MSG_INFO, IDS_BROWSE_SENT_REQUEST, m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser read response line

BOOL CHostBrowser::ReadResponseLine()
{
	// ED2K connections aren't handled here- they are in ED2KClient
	ASSERT( m_nProtocol != PROTOCOL_ED2K && m_nProtocol != PROTOCOL_DC );

	CString strLine, strCode, strMessage;

	if ( ! Read( strLine ) ) return TRUE;
	if ( strLine.IsEmpty() ) return TRUE;

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"%s >> %s", (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	if ( strLine.GetLength() > HTTP_HEADER_MAX_LINE )
		strLine = L"#LINE_TOO_LONG#";

	if ( strLine.GetLength() >= 12 && strLine.Left( 9 ) == L"HTTP/1.1 " )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
	}
	else if ( strLine.GetLength() >= 12 && strLine.Left( 9 ) == L"HTTP/1.0 " )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
	}
	else if ( strLine.GetLength() >= 8 && strLine.Left( 4 ) == L"HTTP" )
	{
		strCode		= strLine.Mid( 5, 3 );
		strMessage	= strLine.Mid( 8 );
	}
	else
	{
		theApp.Message( MSG_DEBUG, L"UNKNOWN BROWSE RESPONSE: %s: %s", m_sAddress, strLine );
		theApp.Message( MSG_ERROR, IDS_BROWSE_NOT_HTTP, m_sAddress );
		Stop();
		return FALSE;
	}

	if ( strCode == L"200" || strCode == L"206" )
	{
		m_nState = hbsHeaders;
	}
	else
	{
		strMessage.TrimLeft();
		if ( strMessage.GetLength() > 256 )
			strMessage = L"No Message";	// Should it be truncated?

		theApp.Message( MSG_ERROR, IDS_BROWSE_HTTPCODE, m_sAddress, strCode, strMessage );

		Stop();
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser read headers

BOOL CHostBrowser::OnHeaderLine(CString& strHeader, CString& strValue)
{
	//CQuickLock oTransfersLock( Transfers.m_pSection );

	// ED2K connections aren't handled here- they are in ED2KClient
	ASSERT( m_nProtocol != PROTOCOL_ED2K && m_nProtocol != PROTOCOL_DC );

	if ( ! CTransfer::OnHeaderLine( strHeader, strValue ) )
		return FALSE;

	if ( strHeader.CompareNoCase( L"Server" ) == 0 )
	{
		m_pVendor = VendorCache.LookupByName( strValue );
		m_sServer = strValue;
		if ( m_sServer.GetLength() > 64 ) m_sServer = m_sServer.Left( 64 );
	}
	else if ( strHeader.CompareNoCase( L"Content-Type" ) == 0 )
	{
		if ( strValue.CompareNoCase( L"application/x-gnutella-packets" ) == 0 )
			m_nProtocol = PROTOCOL_G1;
		else if ( strValue.CompareNoCase( L"application/x-gnutella2" ) == 0 )
			m_nProtocol = PROTOCOL_G2;
		else if ( strValue.CompareNoCase( L"application/x-shareaza" ) == 0 )
			m_nProtocol = PROTOCOL_G2;
		else if ( strValue.CompareNoCase( L"application/x-envy" ) == 0 )
			m_nProtocol = PROTOCOL_G2;
		else if ( strValue.Left(9).CompareNoCase( L"text/html" ) == 0 )
			m_nProtocol = PROTOCOL_NULL;
	}
	else if ( strHeader.CompareNoCase( L"Content-Encoding" ) == 0 )
	{
		m_bDeflate = strValue.CompareNoCase( L"deflate" ) == 0;
	}
	else if ( strHeader.CompareNoCase( L"Content-Length" ) == 0 )
	{
		_stscanf( strValue, L"%I64u", &m_nLength );
	}

	return TRUE;
}

BOOL CHostBrowser::OnHeadersComplete()
{
	CQuickLock oTransfersLock( Transfers.m_pSection );

	if ( m_nState == hbsContent )
		return TRUE;

	if ( m_nProtocol == PROTOCOL_ANY || m_nLength == 0 )
	{
		theApp.Message( MSG_ERROR, IDS_BROWSE_BAD_RESPONSE, m_sAddress );
		Stop();
		return FALSE;
	}

	m_nState		= hbsContent;
	m_nReceived		= 0ul;
	m_pBuffer->Clear();
	m_mInput.tLast	= GetTickCount();

	theApp.Message( MSG_INFO, IDS_BROWSE_DOWNLOADING_FROM,
		m_sAddress, protocolNames[ m_nProtocol == PROTOCOL_NULL ? PROTOCOL_HTTP : m_nProtocol ]);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser read content

BOOL CHostBrowser::ReadContent()
{
	CQuickLock oTransfersLock( Transfers.m_pSection );

	if ( m_nProtocol != PROTOCOL_ED2K && m_nProtocol != PROTOCOL_DC )
	{
		if ( m_nReceived < m_nLength )
		{
			CLockedBuffer pInput( GetInput() );

			DWORD nVolume = min( DWORD( m_nLength - m_nReceived ), pInput->m_nLength );
			m_nReceived += nVolume;

			if ( ! m_bDeflate )
			{
				m_pBuffer->AddBuffer( pInput, nVolume );
			}
			else if ( ! pInput->InflateStreamTo( *m_pBuffer, m_pInflate ) )		// Try to decompress the stream
			{
				Stop();			// Clean up
				return FALSE;	// Report failure
			}
		}
	}

	switch ( m_nProtocol )
	{
	case PROTOCOL_NULL:
		if ( ! StreamHTML() ) return FALSE;
		break;
	case PROTOCOL_G2:
		if ( ! StreamPacketsG2() ) return FALSE;
		break;
	case PROTOCOL_G1:
		if ( ! StreamPacketsG1() ) return FALSE;
		break;
	//case PROTOCOL_ED2K:
	//case PROTOCOL_DC:
	//	break;	// Skip
	//default:
	//	theApp.Message( MSG_ERROR, L"CHostBrowser::ReadContent(): Unknown Browse Protocol" );
	}

	if ( m_nReceived < m_nLength ) return TRUE;

	Stop( TRUE );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser packet streaming

BOOL CHostBrowser::StreamPacketsG1()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_nProtocol == PROTOCOL_G1 );

	BOOL bSuccess = TRUE;
	for ( ; bSuccess ; )
	{
		GNUTELLAPACKET* pPacket = (GNUTELLAPACKET*)m_pBuffer->m_pBuffer;
		if ( m_pBuffer->m_nLength < sizeof( *pPacket ) ) break;

		DWORD nLength = sizeof( *pPacket ) + pPacket->m_nLength;

		if ( pPacket->m_nLength < 0 || nLength >= (DWORD)Settings.Gnutella.MaximumPacket * 8 )
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_PACKET_ERROR, m_sAddress );
			Stop();
			return FALSE;
		}

		if ( m_pBuffer->m_nLength < nLength ) break;

		CG1Packet* pPacketObject = CG1Packet::New( pPacket );

		try
		{
			bSuccess = OnPacket( pPacketObject );
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}

		pPacketObject->Release();

		m_pBuffer->Remove( nLength );
	}

	if ( ! bSuccess )
		Stop();

	return bSuccess;
}

BOOL CHostBrowser::StreamPacketsG2()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_nProtocol == PROTOCOL_G2 );

	while ( CG2Packet* pPacket = CG2Packet::ReadBuffer( m_pBuffer ) )
	{
		BOOL bSuccess = FALSE;

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
		{
			Stop();
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser packet interpreter

BOOL CHostBrowser::OnPacket(CG1Packet* pPacket)
{
	if ( pPacket->m_nType != G1_PACKET_HIT || pPacket->m_nLength <= 27 )
		return TRUE;

	CQueryHit* pHits = CQueryHit::FromG1Packet( pPacket );

	if ( pHits == NULL )
	{
		theApp.Message( MSG_ERROR, IDS_BROWSE_PACKET_ERROR, m_sAddress );
		return FALSE;
	}

	OnQueryHits( pHits );

	return TRUE;
}

BOOL CHostBrowser::OnPacket(CG2Packet* pPacket)
{
	switch ( pPacket->m_nType )
	{
	case G2_PACKET_HIT:
		if ( CQueryHit* pHits = CQueryHit::FromG2Packet( pPacket ) )
		{
			OnQueryHits( pHits );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_PACKET_ERROR, m_sAddress );
			return FALSE;
		}
		break;

	case G2_PACKET_PHYSICAL_FOLDER:
		if ( m_pNotify != NULL )
			m_pNotify->OnPhysicalTree( pPacket );
		break;

	case G2_PACKET_VIRTUAL_FOLDER:
		if ( m_pNotify != NULL )
			m_pNotify->OnVirtualTree( pPacket );
		break;

	case G2_PACKET_PROFILE_DELIVERY:
		OnProfilePacket( pPacket );
		if ( m_pProfile && m_pNotify )
			m_pNotify->OnProfileReceived();
		break;

	case G2_PACKET_PROFILE_AVATAR:
		if ( m_pNotify != NULL )
			m_pNotify->OnHeadPacket( pPacket );
		break;

	case G2_PACKET_PEER_CHAT:
		if ( ! m_bCanChat )
		{
			m_bCanChat = TRUE;
			if ( m_pNotify )
				m_pNotify->OnProfileReceived();
		}
		break;

#ifdef _DEBUG
	default:
		CString str;
		str.Format( L"Unknown G2 Browse packet from %s:%u",
			(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ),
			htons( m_pHost.sin_port ) );
		pPacket->Debug( str );
#endif	// Debug
	}

	return TRUE;
}

void CHostBrowser::OnProfilePacket(CG2Packet* pPacket)
{
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
				if ( m_pProfile == NULL )
					m_pProfile = new CGProfile();
				if ( m_pProfile && ! m_pProfile->FromXML( pXML ) )
					delete pXML;
				if ( m_pProfile && ! m_pProfile->IsValid() )
				{
					delete m_pProfile;
					m_pProfile = NULL;
				}
			}
		}

		pPacket->m_nPosition = nOffset;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser HTML streaming

BOOL CHostBrowser::StreamHTML()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_nProtocol == PROTOCOL_NULL );

	CString strLine;

	CQueryHit* pHits = NULL;

	while ( m_pBuffer->ReadLine( strLine ) )
	{
		int nPosHTTP = strLine.Find( L"http://" );

		while ( nPosHTTP >= 0 && strLine.Find( L"/get/" ) > nPosHTTP )
		{
			CString strURI = strLine.Mid( nPosHTTP ).SpanExcluding( L"?&\"'<>" );
			CString strName;
			DWORD nSize = 0;

			int nPosSize = strLine.Find( L"<TD NOWRAP>" );

			if ( nPosSize >= 0 && nPosSize < nPosHTTP )
			{
				CString strSize = strLine.Mid( nPosSize + 11 ).SpanExcluding( L"</" );
				float nFloat = 0;

				if ( _stscanf( strSize, L"%f", &nFloat ) == 1 && nFloat > 0 )
				{
					if ( strSize.Find( L" GB" ) >= 0 )
						nFloat *= 1024*1024*1024;
					else if ( strSize.Find( L" MB" ) >= 0 )
						nFloat *= 1024*1024;
					else if ( strSize.Find( L" KB" ) >= 0 )
						nFloat *= 1024;

					nSize = (DWORD)ceil( nFloat );
				}
			}

			strLine = strLine.Mid( nPosHTTP + strURI.GetLength() );

			int nPosName = strLine.Find( L">" );

			if ( nPosName >= 0 )
				strName = strLine.Mid( nPosName + 1 ).SpanExcluding( L"<>" );

			if ( strName.IsEmpty() && ( nPosName = strURI.ReverseFind( L'/' ) ) > 0 )
				strName = URLDecode( strURI.Mid( nPosName + 1 ) );

			CQueryHit* pHit = new CQueryHit( PROTOCOL_NULL );

			pHit->m_pAddress	= m_pHost.sin_addr;
			pHit->m_nPort		= htons( m_pHost.sin_port );
			pHit->m_pVendor		= m_pVendor ? m_pVendor : VendorCache.m_pNull;
			pHit->m_bPush		= ( m_tPushed ) ? TRI_TRUE : TRI_FALSE;
			pHit->m_bBrowseHost	= TRUE;
			pHit->m_nSize		= nSize;
			pHit->m_bSize		= TRUE;
			pHit->m_sName		= strName;
			pHit->m_sURL		= strURI;

			if ( m_bCanPush )
				pHit->m_oClientID = m_oClientID;

			pHit->Resolve();

			pHit->m_pNext = pHits;
			pHits = pHit;

			nPosHTTP = strLine.Find( L"http://" );
		}
	}

	if ( pHits != NULL )
		OnQueryHits( pHits );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHostBrowser serialize

void CHostBrowser::Serialize(CArchive& ar, int nVersion)	// BROWSER_SER_VERSION
{
	BOOL bProfilePresent = FALSE;

	if ( ar.IsStoring() )
	{
		ar << m_bNewBrowse;
		ar << m_pAddress.S_un.S_addr;
		ar << m_nPort;
		ar << m_bMustPush;
		ar << m_bCanPush;
		SerializeOut( ar, m_oPushID );
		SerializeOut( ar, m_oClientID );
		ar << m_sNick;
		ar << m_bCanChat;
		ar << m_sServer;
		ar << m_nProtocol;
		ar << m_nHits;
		ar << (DWORD)m_nLength;
		ar << m_nReceived;

		bProfilePresent = ( m_pProfile != NULL );
		ar << bProfilePresent;
	}
	else // Loading
	{
		Stop();

		ar >> m_bNewBrowse;
		ar >> m_pAddress.S_un.S_addr;
		ar >> m_nPort;
		ar >> m_bMustPush;
		ar >> m_bCanPush;
		SerializeIn( ar, m_oPushID, 31 );
		SerializeIn( ar, m_oClientID, 31 );
		ar >> m_sNick;
		ar >> m_bCanChat;
		ar >> m_sServer;
		ar >> m_nProtocol;
		ar >> m_nHits;
		DWORD nLength = 0;
		ar >> nLength;
		m_nLength = nLength;	// To QWORD
		ar >> m_nReceived;
		ar >> bProfilePresent;

		m_pVendor = VendorCache.LookupByName( m_sServer );

		delete m_pProfile;
		m_pProfile = new CGProfile();
	}

	if ( bProfilePresent )
	{
		if ( m_pProfile == NULL )
			m_pProfile = new CGProfile();
		if ( m_pProfile )
			m_pProfile->Serialize( ar, nVersion );
	}
}
