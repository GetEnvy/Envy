//
// Connection.cpp
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

// CConnection holds a socket used to communicate with a remote computer, and is the root of a big inheritance tree
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CConnection
// http://getenvy.com/archives/shareazawiki/Developers.Code.CConnection.html

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "Connection.h"

#include "Network.h"
#include "Security.h"
#include "Buffer.h"
#include "Statistics.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CConnection construction

// Make a new CConnection object
CConnection::CConnection(PROTOCOLID nProtocol)
	: m_bInitiated		( FALSE )
	, m_bConnected		( FALSE )
	, m_tConnected		( 0 )
	, m_hSocket 		( INVALID_SOCKET )
	, m_pInput			( NULL )
	, m_pOutput 		( NULL )
	, m_nProtocol		( nProtocol )
	, m_bClientExtended ( FALSE )
	, m_bAutoDelete		( FALSE )
	, m_nQueuedRun		( 0 )			// DoRun sets it to 0, QueueRun sets it to 2 (do)
	, m_nDelayCloseReason ( 0 )
{
	ZeroMemory( &m_pHost, sizeof( m_pHost ) );
	m_pHost.sin_family = AF_INET;
	ZeroMemory( &m_mInput, sizeof( m_mInput ) );
	ZeroMemory( &m_mOutput, sizeof( m_mOutput ) );
	m_pInputSection.reset( new CCriticalSection() );
	m_pOutputSection.reset( new CCriticalSection() );
}

//////////////////////////////////////////////////////////////////////
// CConnection attach to an existing connection

// Call to copy a connection object to this one (do)
// Takes another connection object
void CConnection::AttachTo(CConnection* pConnection)
{
	// Make sure the socket isn't valid yet
	ASSERT( ! IsValid() );								// Make sure the socket here isn't valid yet
	ASSERT( AfxIsValidAddress( pConnection, sizeof( *pConnection ) ) );
	ASSERT( pConnection->IsValid() );					// And make sure a CConnection object socket exists

	DestroyBuffers();

	CQuickLock oOutputLock( *pConnection->m_pOutputSection );
	CQuickLock oInputLock( *pConnection->m_pInputSection );

	// Copy values from the given CConnection object to this one
	m_pHost				= pConnection->m_pHost;
	m_sAddress			= pConnection->m_sAddress;
	m_sCountry			= pConnection->m_sCountry;
	m_sCountryName		= pConnection->m_sCountryName;
	m_hSocket			= pConnection->m_hSocket;
	m_bInitiated		= pConnection->m_bInitiated;
	m_bConnected		= pConnection->m_bConnected;
	m_tConnected		= pConnection->m_tConnected;
	m_pInputSection		= pConnection->m_pInputSection;
	m_pInput			= pConnection->m_pInput;
	m_pOutputSection	= pConnection->m_pOutputSection;
	m_pOutput			= pConnection->m_pOutput;
	m_sUserAgent		= pConnection->m_sUserAgent;
	m_bClientExtended	= pConnection->m_bClientExtended;
	m_nQueuedRun		= pConnection->m_nQueuedRun;
	if ( m_nProtocol <= PROTOCOL_NULL && pConnection->m_nProtocol > PROTOCOL_NULL )
		m_nProtocol		= pConnection->m_nProtocol;
	m_nDelayCloseReason	= pConnection->m_nDelayCloseReason;

	// Invalidate the socket in the given connection object so it's just here now
	pConnection->m_hSocket	= INVALID_SOCKET;

	// Null the input and output pointers
	pConnection->m_pInput	= NULL;
	pConnection->m_pOutput	= NULL;

	// Zero the memory of the input and output TCPBandwidthMeter objects
	ZeroMemory( &pConnection->m_mInput, sizeof( m_mInput ) );
	ZeroMemory( &pConnection->m_mOutput, sizeof( m_mOutput ) );

	// Record the current time in the input and output TCP bandwidth meters
	m_mInput.tLast = m_mOutput.tLast = GetTickCount();
}

// Delete this CConnection object
CConnection::~CConnection()
{
	m_bAutoDelete = FALSE;

	CConnection::Close();

	// Delete and mark null the input and output buffers
	DestroyBuffers();
}

void CConnection::LogOutgoing()
{
	if ( ! theApp.IsLogDisabled( MSG_DEBUG | MSG_FACILITY_OUTGOING ) )
	{
		CLockedBuffer pOutput( GetOutput() );
		if ( pOutput->m_nLength )
		{
			CStringA msg( (const char*)pOutput->m_pBuffer, pOutput->m_nLength );
			theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, L"%s << %s", (LPCTSTR)m_sAddress, (LPCTSTR)CString( msg ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CConnection connect to

// Connect this CConnection object to a remote computer on the Internet
// Takes pHost, a pointer to a SOCKADDR_IN structure, which is MFC's way of holding an IP address and port number
// Returns true if connected
BOOL CConnection::ConnectTo(const SOCKADDR_IN* pHost)
{
	// Call the next ConnectTo method, and return the result
	return ConnectTo( &pHost->sin_addr, htons( pHost->sin_port ) );
}

// Connect this CConnection object to a remote computer on the Internet
// Takes pAddress, a Windows Sockets structure that holds an IP address, and takes the port number seprately
// Returns true if connected
BOOL CConnection::ConnectTo(const IN_ADDR* pAddress, WORD nPort)
{
	// Make sure the socket isn't already connected somehow
	if ( IsValid() )
		return FALSE;

	// Make sure we have an address and a nonzero port number
	if ( pAddress == NULL || nPort == 0 )
		return FALSE;

	// S_un.S_addr is the IP as a single unsigned 4-byte long
	if ( pAddress->S_un.S_addr == 0 )
		return FALSE;

	// The IP address is in the security list of government and corporate addresses we want to avoid
	if ( Security.IsDenied( pAddress ) )
	{
		// Report that we aren't connecting to this IP address and return false
		theApp.Message( MSG_ERROR, IDS_NETWORK_SECURITY_OUTGOING, (LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
		return FALSE;
	}

	// The IN_ADDR structure we just got passed isn't the same as the one already stored in this object
	if ( pAddress != &m_pHost.sin_addr )
	{
		// Zero the memory of the entire SOCKADDR_IN structure m_pHost, and then copy in the sin_addr part
		ZeroMemory( &m_pHost, sizeof( m_pHost ) );
		m_pHost.sin_addr = *pAddress;
	}

	// Fill in more parts of the m_pHost structure
	m_pHost.sin_family	= AF_INET;							// PF_INET means just normal IPv4, not IPv6 yet
	m_pHost.sin_port	= htons( nPort );					// Copy the port number into the m_pHost structure
	m_sAddress			= inet_ntoa( m_pHost.sin_addr );	// Save the IP address as a string of text
	UpdateCountry();

	// Create a socket and store it in m_hSocket
	// Normal IPv4 not IPv6, and the two-way sequenced reliable byte streams of TCP, not the datagrams of UDP
	m_hSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( ! IsValid() )	// Now, make sure it has been created
	{
		// Second attempt
		m_hSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
		if ( ! IsValid() )
		{
			theApp.Message( MSG_ERROR, L"Failed to create socket." );
			return FALSE;
		}
	}

	// Disables the Nagle algorithm for send coalescing
	VERIFY( setsockopt( m_hSocket, IPPROTO_TCP, TCP_NODELAY, "\x01", 1 ) == 0 );

	// Allows the socket to be bound to an address that is already in use
	VERIFY( setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, "\x01", 1 ) == 0 );

	// Choose asynchronous, non-blocking reading and writing on our new socket
	DWORD dwValue = 1;
	ioctlsocket( m_hSocket, FIONBIO, &dwValue );
		// Call Windows Sockets ioctlsocket to control the input/output mode of our new socket
		// Give it our new socket
		// Select the option for blocking i/o, should the program wait on read and write calls, or keep going?
		// Nonzero, it should keep going

	// If the OutHost string in connection settings has an IP address written in it
	if ( Settings.Connection.OutHost.GetLength() )
	{
		// Read the text and copy the IP address and port into a new local MFC SOCKADDR_IN structure called pOutgoing
		SOCKADDR_IN pOutgoing;
		Network.Resolve( Settings.Connection.OutHost, 0, &pOutgoing );

		// S_addr is the IP address as a single long number, if it's not zero
		if ( pOutgoing.sin_addr.S_un.S_addr )
		{
			// Call bind in Windows Sockets to associate the local address with the socket
			bind(
				m_hSocket,					// Our socket
				(SOCKADDR*)&pOutgoing,		// The IP address this computer appears to have on the Internet (do)
				sizeof( SOCKADDR_IN ) );	// Tell bind how many bytes it can read at the pointer
		}
	}

	DestroyBuffers();

	// Try to connect to the remote computer
	if ( WSAConnect(
		m_hSocket,					// Our socket
		(SOCKADDR*)&m_pHost,		// The remote IP address and port number
		sizeof( SOCKADDR_IN ),		// How many bytes the function can read
		NULL, NULL, NULL, NULL ) )	// No advanced features
	{
		// If no error occurs, WSAConnect returns 0, so if we're here an error happened
		int nError = WSAGetLastError();		// Get the last Windows Sockets error number

		// An error of "would block" is normal because connections can't be made instantly and this is a non-blocking socket
		if ( nError != WSAEWOULDBLOCK )
		{
			CNetwork::CloseSocket( m_hSocket, true );

			if ( nError != 0 )
				Statistics.Current.Connections.Errors++;

			return FALSE;
		}
	}

	CreateBuffers();

	// Record that we initiated this connection, and when it happened
	m_bInitiated	= TRUE;
	m_tConnected	= GetTickCount();

	// Record one more outgoing connection in the statistics
	Statistics.Current.Connections.Outgoing++;

	// Connection successfully attempted
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection accept an incoming connection

// Called when a remote computer wants to connect to us
// When WSAAccept accepted the connection, it created a new socket hSocket for it and wrote the remote IP in pHost
void CConnection::AcceptFrom(SOCKET hSocket, SOCKADDR_IN* pHost)
{
	// Make sure the newly accepted socket is valid
	ASSERT( ! IsValid() );

	// Record the connection information here
	m_hSocket		= hSocket;							// Keep the socket here
	m_pHost			= *pHost;							// Copy the remote IP address into this object
	m_sAddress		= inet_ntoa( m_pHost.sin_addr );	// Store it as a string also
	UpdateCountry();

	// Make new input and output buffer objects
	ASSERT( m_pInput == NULL );
	ASSERT( m_pOutput == NULL );
	CreateBuffers();

	// Facts about the connection
	m_bInitiated	= FALSE;			// We didn't initiate this connection
	m_bConnected	= TRUE;				// We're connected right now
	m_tConnected	= GetTickCount();	// Record the time this happened

	// Choose asynchronous, non-blocking reading and writing on the new socket
	DWORD dwValue = 1;
	ioctlsocket( m_hSocket, FIONBIO, &dwValue );

	// Record one more incoming connection in the statistics
	Statistics.Current.Connections.Incoming++;
}

//////////////////////////////////////////////////////////////////////
// CConnection close

// Call to close the connection represented by this object
void CConnection::Close(UINT nError)
{
	// Make sure this object exists, and is located entirely within the program's memory space
	ASSERT( this != NULL );
	ASSERT( AfxIsValidAddress( this, sizeof( *this ) ) );

	if ( nError )
	{
		if ( nError == IDS_HANDSHAKE_REJECTED )
		{
			theApp.Message( MSG_ERROR, IDS_HANDSHAKE_REJECTED, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		}
		else
		{
			BOOL bInfo = ( nError == IDS_CONNECTION_CLOSED || nError == IDS_CONNECTION_PEERPRUNE );
			theApp.Message( bInfo ? MSG_INFO : MSG_ERROR, nError, (LPCTSTR)m_sAddress );
		}
	}

	CNetwork::CloseSocket( m_hSocket, false );

	// This connection object isn't connected any longer
	m_bConnected = FALSE;

	if ( m_bAutoDelete )
		delete this;
}

// Close the connection, but not until we've written the buffered outgoing data first
// Takes the reason we're closing the connection, or 0 by default
void CConnection::DelayClose(UINT nError)
{
	ASSERT( nError );

	// Clear input buffer
	{
		CQuickLock oInputLock( *m_pInputSection );
		m_pInput->Clear();
	}

	// Disable incoming data
	shutdown( m_hSocket, SD_RECEIVE );

	// Prolong ban (if any) for incoming connection
	if ( ! m_bInitiated )
		Security.Complain( &m_pHost.sin_addr );

	m_nDelayCloseReason = nError;

	// Have the connection object write all the outgoing data soon
	QueueRun();
}

//////////////////////////////////////////////////////////////////////
// CConnection run function

// Talk to the other computer, write the output buffer to the socket and read from the socket to the input buffer
// Return true if this worked, false if we've lost the connection
BOOL CConnection::DoRun()
{
	// ToDo: Occasionally locks for a few seconds?

	// If this socket is invalid, call OnRun and return the result (do)
	if ( ! IsValid() )
		return OnRun();

	// Setup pEvents to store the socket's internal information about network events
	WSANETWORKEVENTS pEvents = {};
	if ( WSAEnumNetworkEvents( m_hSocket, NULL, &pEvents ) != 0 )
		return FALSE;

	// If the FD_CONNECT network event has occurred
	if ( pEvents.lNetworkEvents & FD_CONNECT )
	{
		// If there is a nonzero error code for the connect operation, this connection was dropped
		if ( pEvents.iErrorCode[ FD_CONNECT_BIT ] != 0 )
		{
			Statistics.Current.Connections.Errors++;

			OnDropped();
			return FALSE;
		}

		// The socket is now connected
		m_bConnected = TRUE;
		m_tConnected = m_mInput.tLast = m_mOutput.tLast = GetTickCount();	// Store the time 3 places

		// Call CShakeNeighbour::OnConnected to start reading the handshake
		if ( ! OnConnected() )
			return FALSE;

		Network.AcquireLocalAddress( m_hSocket );
	}

	// If the FD_CLOSE network event has occurred, set bClosed to true, otherwise set it to false
	BOOL bClosed = ( pEvents.lNetworkEvents & FD_CLOSE ) ? TRUE : FALSE;

	// If the close event happened, null a pointer within the TCP bandwidth meter for input (do)
	if ( bClosed )
		m_mInput.pLimit = NULL;

	// Change the queued run state to 1 (do)
	m_nQueuedRun = 1;

	// Write the contents of the output buffer to the remote computer, and read in data it sent us
	if ( ! OnWrite() )
		return FALSE;
	if ( ! OnRead() )
		return FALSE;

	// If the close event happened
	if ( bClosed )
	{
		// theApp.Message( MSG_DEBUG, L"socket close() error %i", pEvents.iErrorCode[ FD_CLOSE_BIT ] );
		// Call OnDropped, telling it true if there is a close error
		OnDropped();	// True if there is an nonzero error code for the close bit
		return FALSE;
	}

	// Make sure the handshake doesn't take too long
	if ( ! OnRun() )
		return FALSE;

	// If the queued run state is 2 and OnWrite returns false, leave here with false also
	if ( m_nQueuedRun == 2 && ! OnWrite() )
		return FALSE;

	// Change the queued run state back to 0 and report success (do)
	m_nQueuedRun = 0;
	return TRUE;
}

// Call OnWrite if DoRun has just succeeded (do)
void CConnection::QueueRun()
{
	// If the queued run state is 1 or 2, make it 2, if it's 0, call OnWrite now (do)
	if ( m_nQueuedRun )
		m_nQueuedRun = 2;	// The queued run state is not 0, make it 2 and do nothing else
	else
		OnWrite();			// The queued run state is 0, do a write (do)
}

//////////////////////////////////////////////////////////////////////
// CConnection socket event handlers

// Objects that inherit from CConnection have OnConnected methods that do things, unlike this one
BOOL CConnection::OnConnected()
{
	return TRUE;	// Just return true
}

// Objects that inherit from CConnection have OnDropped methods that do things, unlike this one
void CConnection::OnDropped()
{
	// Do nothing here
}

// Objects that inherit from CConnection have OnRun methods that do things, unlike this one
BOOL CConnection::OnRun()
{
	if ( m_nDelayCloseReason )
	{
		CLockedBuffer pOutputLocked( GetOutput() );

		// If there is nothing to send
		if ( pOutputLocked->m_nLength == 0 )
		{
			Close( m_nDelayCloseReason );
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection read event handler

// Read data waiting in the socket into the input buffer
BOOL CConnection::OnRead()
{
	CQuickLock oInputLock( *m_pInputSection );

	// Make sure the socket is valid
	if ( ! IsValid() )
		return FALSE;

	if ( m_nDelayCloseReason )
		return TRUE;

	const DWORD tNow = GetTickCount();	// The time right now
	DWORD nLimit = ~0ul;				// Make the limit huge

	// If we need to worry about throttling bandwidth, calculate nLimit, the number of bytes we are allowed to read now
	if ( m_mInput.pLimit							// If there is a limit
		&& *m_mInput.pLimit							// And that limit isn't 0
		&& Settings.Live.BandwidthScaleIn <= 100 )	// And the bandwidth scale isn't at MAX
	{
		// Work out what the bandwidth limit is
		nLimit = m_mInput.CalculateLimit( tNow, Settings.Live.BandwidthScaleIn );
	}

	// Read from the socket and record the # bytes read
	if ( DWORD nTotal = m_pInput->Receive( m_hSocket, nLimit ) )
	{
		// Bytes were read, add # bytes to bandwidth meter
		m_mInput.Add( nTotal, tNow );

		// Add the total to statistics
		Statistics.Current.Bandwidth.Incoming += nTotal;
		Statistics.Current.Downloads.Volume += ( nTotal / 1024 );	// For Home tab display
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection write event handler

// Call to send the contents of the output buffer to the remote computer
BOOL CConnection::OnWrite()
{
	CQuickLock oOutputLock( *m_pOutputSection );

	// Make sure the socket is valid
	if ( ! IsValid() )
		return FALSE;

	// If there is nothing to send, we succeed without doing anything
	if ( m_pOutput->m_nLength == 0 )
		return TRUE;

	const DWORD tNow = GetTickCount();	// The time right now
	DWORD nLimit = ~0ul;				// Make the limit huge

	// If we need to worry about throttling bandwidth, calculate nLimit, the number of bytes we are allowed to write now
	if ( m_mOutput.pLimit							// If there is a limit
		&& *m_mOutput.pLimit						// And that limit isn't 0
		&& Settings.Live.BandwidthScaleOut < 100 )	// And the bandwidth scale isn't at MAX
	{
		// Work out what the bandwidth limit is
		nLimit = m_mOutput.CalculateLimit( tNow, Settings.Live.BandwidthScaleOut, Settings.Uploads.ThrottleMode );
	}

	// Read from the socket and record the # bytes sent
	if ( DWORD nTotal = m_pOutput->Send( m_hSocket, nLimit ) )
	{
		// Bytes were sent, add # bytes to bandwidth meter
		m_mOutput.Add( nTotal, tNow );

		// Add total to statistics
		Statistics.Current.Bandwidth.Outgoing += nTotal;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection measure

// Calculate the input and output speeds for this connection
void CConnection::Measure()
{
	// Time period for bytes
	const DWORD tCutoff = GetTickCount() - METER_PERIOD;

	// Calculate Input and Output separately
	m_mInput.nMeasure  = m_mInput.CalculateUsage( tCutoff )  / ( METER_PERIOD / METER_SECOND );
	m_mOutput.nMeasure = m_mOutput.CalculateUsage( tCutoff ) / ( METER_PERIOD / METER_SECOND );
}

// Calculate the input speed for this connection
void CConnection::MeasureIn()
{
	// Time period for bytes
	const DWORD tCutoff = GetTickCount() - METER_PERIOD;

	// Calculate input speed
	m_mInput.nMeasure  = m_mInput.CalculateUsage( tCutoff )  / ( METER_PERIOD / METER_SECOND );
}

// Calculate the output speed for this connection
void CConnection::MeasureOut()
{
	// Time period for bytes
	const DWORD tCutoff = GetTickCount() - METER_PERIOD;

	// Calculate output speed
	m_mOutput.nMeasure = m_mOutput.CalculateUsage( tCutoff ) / ( METER_PERIOD / METER_SECOND );
}

//////////////////////////////////////////////////////////////////////
// CConnection HTML header reading

// Remove the headers from the input buffer, handing each to OnHeaderLine
BOOL CConnection::ReadHeaders()
{
	// Move the first line from the m_pInput buffer to strLine and do the contents of the while loop
	CString strLine;
	while ( Read( strLine ) )	// ReadLine will return false when there are no more lines
	{
		// If the line is more than 256 KB, change it to the long line error code
		if ( strLine.GetLength() > HTTP_HEADER_MAX_LINE )
			strLine = L"#LINE_TOO_LONG#";

		// Find the first colon in the line
		int nPos = strLine.Find( L":" );

		// The line is empty, it's just a \n character
		if ( strLine.IsEmpty() )
		{
			// Empty the last header member variable (do)
			m_sLastHeader.Empty();

			// Call the OnHeadersComplete method for the most advanced class that inherits from CConnection
			return OnHeadersComplete();
		}
		else if ( _istspace( strLine.GetAt( 0 ) ) )		// Get the first character in the string, and see if its a space
		{
			// The line starts with a space

			// The last header has length
			if ( ! m_sLastHeader.IsEmpty() )
			{
				// Trim whitespace from both ends of the line, and ensure it still has length
				strLine.Trim();
				if ( strLine.IsEmpty() ) continue;

				// Give OnHeaderLine the last header and this line
				if ( ! OnHeaderLine( m_sLastHeader, strLine ) )
					return FALSE;
			}
		}
		else if ( nPos > 1 && nPos < 64 )	// ":a" is 0 and "a:a" is 1, but "aa:a" is greater than 1
		{
			// The colon is at a distance greater than 1 and less than 64

			// The line is like "header:value", copy out both parts
			CString strHeader	= strLine.Left( nPos );
			CString strValue	= strLine.Mid( nPos + 1 );
			m_sLastHeader = strHeader;

			strValue.Trim();
			if ( strValue.IsEmpty() ) continue;

			// Give OnHeaderLine this last header, and its value
			if ( ! OnHeaderLine( strHeader, strValue ) )
				return FALSE;
		}
	}

	// Send the contents of the output buffer to the remote computer
	OnWrite();
	return TRUE;
}

// Takes a header and its value
// Reads and processes popular Gnutella headers
// Returns true to have ReadHeaders keep going
BOOL CConnection::OnHeaderLine(CString& strHeader, CString& strValue)
{
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"%s >> %s: %s", (LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue );

	// It's the user agent header
	if ( strHeader.CompareNoCase( L"User-Agent" ) == 0 )
	{
		// Copy the value into the user agent member string
		m_sUserAgent = strValue;	// This tells what software the remote computer is running
		m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );
	}
	// It's the remote IP header
	else if ( strHeader.CompareNoCase( L"Remote-IP" ) == 0 )
	{
		// Add this address to our record of them
		Network.AcquireLocalAddress( strValue );
	}
	// It's the x my address, listen IP, or node header, like "X-My-Address: 10.254.0.16:6349"
	else if (  strHeader.CompareNoCase( L"X-My-Address" ) == 0
			|| strHeader.CompareNoCase( L"Listen-IP" ) == 0
			|| strHeader.CompareNoCase( L"X-Node" ) == 0
			|| strHeader.CompareNoCase( L"Node" ) == 0 )
	{
		// Find another colon in the value
		int nColon = strValue.Find( L':' );

		// If the remote computer first contacted us and the colon is there but not first
		if ( ! m_bInitiated && nColon > 0 )
		{
			// Read the number after the colon into nPort
			WORD nPort = protocolPorts[ PROTOCOL_G1 ];	// Start out nPort as the default value, 6346
			if ( _stscanf( strValue.Mid( nColon + 1 ), L"%hu", &nPort ) == 1 && nPort != 0 )		// Make sure 1 number was found, and isn't 0
			{
				// Save the found port number in m_pHost
				m_pHost.sin_port = htons( u_short( nPort ) );	// Convert Windows little endian to big for the Internet with htons
			}
		}
	}
	else if ( strHeader.CompareNoCase( L"Accept" ) == 0 )
	{
		if ( m_nProtocol != PROTOCOL_G2 )
		{
			if ( _tcsistr( strValue, L"application/x-gnutella-packets" ) )
				m_nProtocol = PROTOCOL_G1;
			if ( _tcsistr( strValue, L"application/x-gnutella2" ) ||
				 _tcsistr( strValue, L"application/x-shareaza" ) ||
				 _tcsistr( strValue, L"application/x-envy" ) )
				m_nProtocol = PROTOCOL_G2;
		}
	}

	// Have ReadHeaders keep going
	return TRUE;
}

// Classes that inherit from CConnection override this virtual method, adding code specific to them
BOOL CConnection::OnHeadersComplete()
{
	// Just return true: CShakeNeighbour::OnHeadersComplete() usually gets called instead of this method
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection header output helpers

// Compose text like "Listen-IP: 1.2.3.4:5" and prints it into the output buffer
// Returns true if we are listening on a port and the header was sent, false otherwise
BOOL CConnection::SendMyAddress()
{
	// Only do something if we are listening on a port
	if ( Network.IsListening() )
	{
		// Compose header text
		CString strHeader;
		strHeader.Format(
			L"Listen-IP: %s:%hu\r\n",								// Make it like "Listen-IP: 67.176.34.172:6346\r\n"
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),	// Insert the IP address like "67.176.34.172"
			htons( Network.m_pHost.sin_port ) );						// Our port number in big endian

		// Print the line into the bottom of the output buffer
		Write( strHeader );		// It will be sent to the remote computer on the next write

		// Report that we are listening on a port, and the header is sent
		return TRUE;
	}

	// We're not even listening on a port
	return FALSE;
}

void CConnection::UpdateCountry()
{
	m_sCountry		= theApp.GetCountryCode( m_pHost.sin_addr );
	m_sCountryName	= theApp.GetCountryName( m_pHost.sin_addr );
}

void CConnection::SendHTML(UINT nResourceID)
{
	CString strResponse;
	CString strBody = LoadRichHTML( nResourceID, strResponse );

	if ( strResponse.IsEmpty() )
		Write( _P("HTTP/1.1 200 OK\r\n") );
	else
		Write( L"HTTP/1.1 " + strResponse );

	if ( nResourceID == IDR_HTML_BUSY )
		Write( _P("Retry-After: 30\r\n") );

	Write( _P("Content-Type: text/html\r\n") );

	CStringA strBodyUTF8 = UTF8Encode( strBody );

	CString strLength;
	strLength.Format( L"Content-Length: %i\r\n\r\n", strBodyUTF8.GetLength() );
	Write( strLength );

	LogOutgoing();

	Write( (LPCSTR)strBodyUTF8, strBodyUTF8.GetLength() );
}

//////////////////////////////////////////////////////////////////////
// TCPBandwidthMeter Utility routines

// Calculate the number of bytes available for use
DWORD CConnection::TCPBandwidthMeter::CalculateLimit(DWORD tNow, DWORD nBandwidthScale, bool bMaxMode /*false*/ ) const
{
	DWORD tCutoff = tNow - METER_SECOND;			// Time period for bytes
	if ( bMaxMode )
		tCutoff += METER_MINIMUM;					// Adjust time period for Maximum mode limit (Default is Average)
	DWORD nData = CalculateUsage( tCutoff, true );	// #bytes in the time period

	// nLimit is the speed limit (bytes/second)
	DWORD nLimit = *pLimit;							// Get the speed limit

	if ( nBandwidthScale < 100 )					// The scale is turned down and we should use it
		nLimit = nLimit * nBandwidthScale / 100;	// Adjust limit based on the scale percentage
	else if ( nBandwidthScale > 100 )
		nLimit = 0xFFFFFFFF;						// Remove limit based on MAX scale

	// nLimit - nData is the number of bytes still available for this time period
	// Set nData to this, or 0 if we're over the limit
	nData >= nLimit ? nData = 0 : nData = nLimit - nData;

	// Is this running in Maximum mode (strict limit)
	if ( bMaxMode )
	{
		// Adjust limit for the time elapsed since last time
		nLimit = nLimit * ( tNow - tLastLimit ) / 1000;

		// nData = speed limit in bytes per second - bytes we read in the last second
		// nLimit = speed limit in bytes per second * elapsed time
		// Return the smaller of the two
		nLimit = min( nLimit, nData );
	}
	else // Average
	{
		// Set limit to the number of bytes still available for this time period
		nLimit = nData;
	}

	tLastLimit = tNow;	// Time of this limit calculation

	return nLimit;		// Return the new limit
}

// Count the #bytes used for a given time period (optimal for time periods more than METER_LENGTH / 2)
DWORD CConnection::TCPBandwidthMeter::CalculateUsage(DWORD tTime) const
{
	// Exit early if the last slot used is older than the time limit
	if ( tLastSlot <= tTime ) return 0;

	// Loop across the times and histories stored
	DWORD nData = 0;	// #bytes in the time period
	DWORD slot = 0;		// Start at the first slot

	// Find the first reading in the time limit
	while ( slot <= nPosition && pTimes[ slot ] <= tTime )
		slot++;

	// Add history up to the latest reading
	while ( slot <= nPosition )
		nData += pHistory[ slot++ ];

	// Did we start with a reading inside the time limit
	if ( pTimes[ 0 ] > tTime )
	{
		// Find the next reading in the time limit
		while ( slot < METER_LENGTH && pTimes[ slot ] <= tTime )
			slot++;

		// Add history up to the end of the meter
		while ( slot < METER_LENGTH )
			nData += pHistory[ slot++ ];
	}

	// Return # bytes in time period
	return nData;
}

// Count the #bytes used for a given time period (optimal for time periods less than METER_LENGTH / 2)
DWORD CConnection::TCPBandwidthMeter::CalculateUsage(DWORD tTime, bool /*bShortPeriod*/) const
{
	// Exit early if the last slot used is older than the time limit
	if ( tLastSlot <= tTime ) return 0;

	// Loop across the times and histories stored
	// Granularity is 1/10th ( METER_MINIMUM ) of a second, so at the most we only need
	// to read 12 ( METER_MINIMUM / METER_SECOND + 2 ) records instead of all of them
	DWORD nData = 0;			// #bytes in the time period
	DWORD slot = METER_LENGTH;	// Start at the last slot
	while ( slot-- )
	{
		if ( pTimes[ slot ] > tTime )	// Is this within the time period?
			nData += pHistory[ slot ];	// Add it to #bytes
		else if ( slot > nPosition )	// Or did we start with the latest reading ?
			slot = nPosition + 1;		// Jump to our latest reading and continue
		else
			break;						// We did, no need to check the rest
	}

	return nData;	// Pass #bytes in time period
}

// Add #bytes to history
void CConnection::TCPBandwidthMeter::Add(const DWORD nBytes, const DWORD tNow)
{
	if ( tNow < tLastSlot + METER_MINIMUM )
	{
		// Less than the minimum time interval
		// Use the same place in the array as before
		pHistory[ nPosition ] += nBytes;
	}
	else
	{
		// More than the minimum time interval
		// Store the time and total in a new array slot
		nPosition = ( nPosition + 1 ) % METER_LENGTH;	// Move to the next position in the array
		pHistory[ nPosition ] = nBytes; 				// Store the #bytes next to it
		pTimes[ nPosition ]   = tNow;					// Record the new time
		tLastSlot = tNow;								// We just wrote some history information
	}
	nTotal += nBytes;	// Add the #bytes to the total
	tLast = tNow;		// The time of this add
}
