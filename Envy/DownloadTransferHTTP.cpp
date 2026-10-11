//
// DownloadTransferHTTP.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2016
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
#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferHTTP.h"
#include "DownloadGroups.h"
#include "FragmentedFile.h"
#include "EnvyURL.h"
#include "Network.h"
#include "Buffer.h"
#include "GProfile.h"
#include "XML.h"
#include "XmlParseValidate.h"
#include "DownloadTransferHttpValidate.h"
#include "VendorCache.h"
#include "Transfers.h"
#include "Security.h" // Vendors

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP construction

CDownloadTransferHTTP::CDownloadTransferHTTP(CDownloadSource* pSource)
    : CDownloadTransfer(pSource, PROTOCOL_HTTP)
    , m_nRequests(0)
    , m_tContent(0)
    , m_bBadResponse(FALSE)
    , m_bBusyFault(FALSE)
    , m_bRangeFault(FALSE)
    , m_bKeepAlive(FALSE)
    , m_bTigerFetch(FALSE)
    , m_bTigerIgnore(FALSE)
    , m_bTigerAbandon(FALSE)
    , m_bTigerDescriptor(FALSE)
    , m_bTigerTreeSeen(FALSE)
    , m_bTigerDimeStarted(FALSE)
    , m_bTigerDimeEnded(FALSE)
    , m_nTigerDepth(0)
    , m_bMetaFetch(FALSE)
    , m_bGotRange(FALSE)
    , m_bGotRanges(FALSE)
    , m_bQueueFlag(FALSE)
    , m_nRetryDelay(Settings.Downloads.RetryDelay)
    , m_nRetryAfter(0)
    , m_bRedirect(FALSE)
    , m_bGzip(FALSE)
    , m_bCompress(FALSE)
    , m_bDeflate(FALSE)
    , m_bChunked(FALSE)
    , m_ChunkState(Header)
    , m_nChunkLength(SIZE_UNKNOWN)
    , m_nContentLength(SIZE_UNKNOWN)
{
}

CDownloadTransferHTTP::~CDownloadTransferHTTP()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP initiate connection

BOOL CDownloadTransferHTTP::Initiate()
{
	ASSUME_LOCK( Transfers.m_pSection );

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTING,
		(LPCTSTR)CString( inet_ntoa( m_pSource->m_pAddress ) ), m_pSource->m_nPort, (LPCTSTR)m_pDownload->GetDisplayName() );

	if ( ConnectTo( &m_pSource->m_pAddress, m_pSource->m_nPort ) )
	{
		SetState( dtsConnecting );

		if ( ! m_pDownload->IsBoosted() )
			m_mInput.pLimit = m_mOutput.pLimit = &m_nBandwidth;

		return TRUE;
	}
	else
	{
		// Couldn't connect, keep the source but add to the m_pFailedSources
		// Mark it as an offline source, it might be good later...
		m_pDownload->AddFailedSource( m_pSource, true, true );
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
		Close( TRI_UNKNOWN );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP accept push

void CDownloadTransferHTTP::AttachTo(CConnection* pConnection)
{
	CDownloadTransfer::AttachTo( pConnection );

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_PUSHED, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );

	if ( ! m_pDownload->IsBoosted() )
		m_mInput.pLimit = m_mOutput.pLimit = &m_nBandwidth;

	StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP close

void CDownloadTransferHTTP::Close( TRISTATE bKeepSource, DWORD nRetryAfter )
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_pSource != NULL && m_nState == dtsDownloading && m_nPosition )
	{
		if ( m_bRecvBackwards )
			m_pSource->AddFragment( m_nOffset + m_nLength - m_nPosition, m_nPosition );
		else
			m_pSource->AddFragment( m_nOffset, m_nPosition );
	}

	CDownloadTransfer::Close( bKeepSource, nRetryAfter );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP speed controls

void CDownloadTransferHTTP::Boost()
{
	m_mInput.pLimit = m_mOutput.pLimit = NULL;
}

DWORD CDownloadTransferHTTP::GetAverageSpeed()
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_nState == dtsDownloading )
	{
		const DWORD nTime = GetTickCount() - m_tContent;
		if ( nTime > 1 )
			m_pSource->m_nSpeed = (DWORD)( ( m_nPosition * 1000 ) / nTime );
	}

	return m_AverageSpeed( m_pSource->m_nSpeed );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP connection handler

BOOL CDownloadTransferHTTP::OnConnected()
{
	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );

	m_tConnected = GetTickCount();

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP fragment allocation

BOOL CDownloadTransferHTTP::StartNextFragment()
{
	ASSUME_LOCK( Transfers.m_pSection );

	ASSERT( this != NULL );
	if ( this == NULL ) return FALSE;

	m_nOffset			= SIZE_UNKNOWN;
	m_nPosition			= 0;
	m_nContentLength = SIZE_UNKNOWN;
	m_bWantBackwards	= FALSE;
	m_bRecvBackwards	= FALSE;
	m_bTigerFetch		= FALSE;
	m_sTigerUUID.Empty();
	m_bTigerAbandon = FALSE;
	m_bTigerDescriptor = FALSE;
	m_bTigerTreeSeen = FALSE;
	m_bTigerDimeStarted = FALSE;
	m_bTigerDimeEnded = FALSE;
	m_nTigerDepth = 0;
	m_bMetaFetch		= FALSE;

	if ( ! IsInputExist() || ! IsOutputExist() )	// || m_pDownload->GetTransferCount( dtsDownloading ) >= Settings.Downloads.MaxFileTransfers )
	{
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_CLOSING_EXTRA, (LPCTSTR)m_sAddress );
		Close( TRI_TRUE );
		return FALSE;
	}

	// This needs to go for pipeline

	if ( GetInputLength() > 0 && m_nRequests > 0 )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CLOSING_OVERFLOW, (LPCTSTR)m_sAddress );
		Close( TRI_TRUE );
		return FALSE;
	}

	if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
		return SendRequest();

	if ( m_pDownload->NeedTigerTree() )
	{
		if ( m_sTigerTree.IsEmpty() && m_pDownload->m_oTiger && Settings.Downloads.VerifyTiger && ! m_bTigerIgnore )
		{
			// Converting urn containing tiger tree root to
			// "/gnutella/thex/v1?urn:tree:tiger/:{TIGER_ROOT}&depth={TIGER_HEIGHT}&ed2k={0/1}"
			// in case if "X-Thex-URI" and "X-TigerTree-Path" headers will be absent or it is a "GIV" (push) connection
			m_sTigerTree.Format( L"/gnutella/thex/v1?%s&depth=%u&ed2k=%d", (LPCTSTR)m_pDownload->m_oTiger.toUrn(),
				Settings.Library.TigerHeight, ( Settings.Downloads.VerifyED2K ? 1 : 0 ) );
		}

		if ( ! m_sTigerTree.IsEmpty() )
		{
			theApp.Message( MSG_INFO,IDS_DOWNLOAD_TIGER_REQUEST, (LPCTSTR)m_pDownload->GetDisplayName(),(LPCTSTR)m_sAddress );

			m_bTigerFetch = TRUE;
			m_bTigerIgnore = TRUE;

			return SendRequest();
		}
	}

	if ( m_pSource && ! m_pSource->m_bMetaIgnore && ! m_sMetadata.IsEmpty() )
	{
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_METADATA_REQUEST, (LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );

		m_bMetaFetch = TRUE;
		m_pSource->m_bMetaIgnore = TRUE;

		return SendRequest();
	}

	if ( m_pDownload->GetFragment( this ) )
	{
		ChunkifyRequest( &m_nOffset, &m_nLength, Settings.Downloads.ChunkSize, TRUE );

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_REQUEST,
			m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );

		return SendRequest();
	}

	if ( m_pSource != NULL )
		m_pSource->SetAvailableRanges( NULL );

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_END, (LPCTSTR)m_sAddress );
	Close( TRI_TRUE );

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP subtract pending requests

BOOL CDownloadTransferHTTP::SubtractRequested(Fragments::List& ppFragments) const
{
	if ( m_nLength >= SIZE_UNKNOWN || m_nOffset >= SIZE_UNKNOWN )
		return FALSE;

	if ( m_nState == dtsRequesting || m_nState == dtsDownloading )
	{
		ppFragments.erase( Fragments::Fragment( m_nOffset, m_nOffset + m_nLength ) );
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP send request

BOOL CDownloadTransferHTTP::SendRequest()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CString strLine;

	CEnvyURL pURL;
	if ( ! pURL.Parse( m_pSource->m_sURL, FALSE ) || pURL.m_nProtocol != PROTOCOL_HTTP )
		return FALSE;

	if ( m_bTigerFetch )
	{
		pURL.m_sPath = m_sTigerTree;
		m_sTigerTree.Empty();
	}
	else if ( m_bMetaFetch )
	{
		pURL.m_sPath = m_sMetadata;
		m_sMetadata.Empty();
	}

	if ( Settings.Downloads.RequestHTTP11 )
	{
		strLine.Format( L"GET %s HTTP/1.1\r\n", (LPCTSTR)pURL.m_sPath );
		Write( strLine );

		strLine.Format( L"Host: %s\r\n", (LPCTSTR)pURL.m_sAddress );
		Write( strLine );
	}
	else
	{
		strLine.Format( L"GET %s HTTP/1.0\r\n", (LPCTSTR)m_pSource->m_sURL );
		Write( strLine );
	}

	Write( _P("Connection: Keep-Alive\r\n") );	// BearShare assumes close

	if ( Settings.Gnutella2.Enabled )
		Write( _P("X-Features: g2/1.0\r\n") );

	if ( m_bTigerFetch )
		Write( _P("Accept: application/dime, application/tigertree-breadthfirst\r\n") );
	else if ( m_bMetaFetch )
		Write( _P("Accept: text/xml\r\n") );

	if ( m_bWantBackwards && Settings.Downloads.AllowBackwards )
		Write( _P("Accept-Encoding: backwards\r\n") );

	if ( m_nOffset != SIZE_UNKNOWN && ! m_bTigerFetch && ! m_bMetaFetch )
	{
		if ( m_nLength == SIZE_UNKNOWN || m_nLength == 0 || m_nOffset + m_nLength == m_pDownload->m_nSize )
			strLine.Format( L"Range: bytes=%I64u-\r\n", m_nOffset );
		else
			strLine.Format( L"Range: bytes=%I64u-%I64u\r\n", m_nOffset, m_nOffset + m_nLength - 1 );

		Write( strLine );
	}
	else
	{
		Write( _P("Range: bytes=0-\r\n") );
	}

	strLine = Settings.SmartAgent();

	if ( ! strLine.IsEmpty() )
	{
		Write( _P("User-Agent: ") );
		Write( strLine );
		Write( _P("\r\n") );
	}

	if ( m_nRequests == 0 )
	{
		if ( m_bInitiated )
			SendMyAddress();

		strLine = MyProfile.GetNick().Left( 255 );

		if ( ! strLine.IsEmpty() )
		{
			Write( _P("X-Nick: ") );
			Write( URLEncode( strLine ) );
			Write( _P("\r\n") );
		}
	}

	if ( m_pSource->m_nPort == INTERNET_DEFAULT_HTTP_PORT )
	{
		int nSlash = m_pSource->m_sURL.ReverseFind( L'/' );
		if ( nSlash > 0 )
		{
			Write( _P("Referrer: ") );
			Write( m_pSource->m_sURL.Left( nSlash + 1 ) );
			Write( _P("\r\n") );
		}
	}

	// ToDo: Browse PrivateKey  See: http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
	//if ( ! m_sKey.IsEmpty() )
	//{
	//	Write( _P("Authorization: :") );
	//	Write( m_sKey );
	//	Write( _P("\r\n") );
	//}

	Write( _P("X-Queue: 0.1\r\n") );

	if ( ! m_bTigerFetch && ! m_bMetaFetch )
	{
		if ( m_pDownload->m_oSHA1 || m_pDownload->m_oTiger )
		{
			Write( _P("X-Content-URN: ") );
			Write( m_pDownload->GetBitprint() );
			Write( _P("\r\n") );
		}
		if ( m_pDownload->m_oED2K )
		{
			Write( _P("X-Content-URN: ") );
			Write( m_pDownload->m_oED2K.toUrn() );
			Write( _P("\r\n") );
		}
		if ( m_pDownload->m_oBTH )
		{
			Write( _P("X-Content-URN: ") );
			Write( m_pDownload->m_oBTH.toUrn() );
			Write( _P("\r\n") );
		}
		if ( m_pDownload->m_oMD5 )
		{
			Write( _P("X-Content-URN: ") );
			Write( m_pDownload->m_oMD5.toUrn() );
			Write( _P("\r\n") );
		}
		if ( m_pSource->m_bSHA1 && Settings.Library.SourceMesh )
		{
			strLine = m_pDownload->GetSourceURLs( &m_pSourcesSent, 15,
				( m_pSource->m_nGnutella < 2 ) ? PROTOCOL_G1 : PROTOCOL_HTTP, m_pSource );
			if ( ! strLine.IsEmpty() )
			{
				if ( m_pSource->m_nGnutella < 2 )
					Write( _P("X-Alt: ") );
				else
					Write( _P("Alt-Location: ") );
				Write( strLine );
				Write( _P("\r\n") );
			}

			if ( m_pDownload->IsShared() && m_pDownload->IsStarted() && Network.IsStable() )
			{
				if ( m_pSource->m_nGnutella < 2 )
				{
					strLine = Network.m_sAddress;
					Write( _P("X-Alt: ") );
				}
				else
				{
					strLine = m_pDownload->GetURL( Network.m_pHost.sin_addr,
						htons( Network.m_pHost.sin_port ) ) + L" " +
						TimeToString( time( NULL ) - 180 );
					Write( _P("Alt-Location: ") );
				}
				Write( strLine );
				Write( _P("\r\n") );

				if ( m_pSource->m_nGnutella < 2 )
				{
					strLine = m_pDownload->GetTopFailedSources( 15, PROTOCOL_G1 );
					if ( ! strLine.IsEmpty() )
					{
						Write( _P("X-NAlt: ") );
						Write( strLine );
						Write( _P("\r\n") );
					}
				}
			}
		}
	}

	LogOutgoing();

	Write( _P("\r\n") );

	SetState( dtsRequesting );
	m_tRequest			= GetTickCount();
	m_bBusyFault		= FALSE;
	m_bRangeFault		= FALSE;
	m_bKeepAlive		= FALSE;
	m_bGotRange			= FALSE;
	m_bGotRanges		= FALSE;
	m_bQueueFlag		= FALSE;
	m_bGzip				= FALSE;
	m_bCompress			= FALSE;
	m_bDeflate			= FALSE;
	m_bChunked			= FALSE;
	m_ChunkState		= Header;
	m_nChunkLength		= SIZE_UNKNOWN;
	m_nContentLength	= SIZE_UNKNOWN;
	m_sContentType.Empty();

	m_nRequests++;

	m_pSource->SetLastSeen();

	CDownloadTransfer::OnWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP run handler

BOOL CDownloadTransferHTTP::OnRun()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CDownloadTransfer::OnRun();

	const DWORD tNow = GetTickCount();

	switch ( m_nState )
	{
	case dtsConnecting:
		if ( tNow > m_tConnected + Settings.Connection.TimeoutConnect )
		{
			theApp.Message( MSG_ERROR, IDS_CONNECTION_TIMEOUT_CONNECT, (LPCTSTR)m_sAddress );
			if ( m_pSource != NULL ) m_pSource->PushRequest();
			Close( TRI_UNKNOWN );
			return FALSE;
		}
		break;

	case dtsRequesting:
	case dtsHeaders:
		if ( tNow > m_tRequest + Settings.Connection.TimeoutHandshake )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_REQUEST_TIMEOUT, (LPCTSTR)m_sAddress );
			Close( m_bBusyFault || m_bQueueFlag ? TRI_TRUE : TRI_UNKNOWN );
			return FALSE;
		}
		break;

	case dtsDownloading:
	case dtsFlushing:
	case dtsTiger:
	case dtsMetadata:
		if ( tNow > m_mInput.tLast + ( Settings.Connection.TimeoutTraffic * 2 ) )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TRAFFIC_TIMEOUT, (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
		break;

	case dtsBusy:
		if ( tNow > m_tRequest + 1000 )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
			Close( TRI_TRUE );
			return FALSE;
		}
		break;

	case dtsQueued:
		if ( tNow >= m_tRequest )
			return StartNextFragment();
		break;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read handler

BOOL CDownloadTransferHTTP::OnRead()
{
	// A header cap can leave body bytes in the socket when FD_CLOSE is already
	// pending. Drain each newly available phase before DoRun calls OnDropped;
	// stop when throttling or an incomplete frame prevents further progress.
	return ReadHttpInputUntilBlocked(
	    [this]()
	    { return ReadInput() != FALSE; },
	    [this]()
	    {
		    u_long nPending = 0;
		    return ioctlsocket(m_hSocket, FIONREAD, &nPending) == 0 ? nPending : 0ul;
	    });
}

BOOL CDownloadTransferHTTP::ReadInput()
{
	// Metadata fetches use a capped receive path for the whole response so a
	// close-delimited peer cannot grow m_pInput past XML_PEER_PARSE_CHARS_MAX
	// while headers are still being parsed (before dtsMetadata).
	if (m_bMetaFetch)
	{
		if (!ReceiveMetadataInput())
			return FALSE;

		switch (m_nState)
		{
		case dtsRequesting:
			if (!ReadResponseLine()) return FALSE;
			if (m_nState == dtsRequesting)
			{
				if (GetInputLength() >= 16u * 1024u)
				{
					theApp.Message(MSG_ERROR, L"Rejected oversized metadata headers from %s",
					               (LPCTSTR)m_sAddress);
					Close(TRI_FALSE);
					return FALSE;
				}
				break;
			}
			if (m_nState != dtsHeaders) break;

		case dtsHeaders:
			if (!ReadHeaders()) return FALSE;
			// OnHeadersComplete already ran ReadMetadata when it set dtsMetadata,
			// so a body buffered with the headers was consumed there. Running the
			// reader again in the same read would wait on the emptied buffer and
			// misreport a completed fetch as unfinished (OnDropped then fails a
			// succeeded transfer). Later reads dispatch through case dtsMetadata.
			break;

		case dtsMetadata:
			return ReadMetadata();

		case dtsFlushing:
			return ReadFlush();
		}

		return TRUE;
	}

	// THEX fetches likewise cap the whole response (headers + body) so an
	// unbounded CDownloadTransfer::OnRead cannot fill m_pInput before dtsTiger.
	if (m_bTigerFetch)
	{
		if (!ReceiveTigerInput())
			return FALSE;

		switch (m_nState)
		{
		case dtsRequesting:
			if (!ReadResponseLine()) return FALSE;
			if (m_nState == dtsRequesting)
			{
				if (GetInputLength() >= 16u * 1024u)
				{
					theApp.Message(MSG_ERROR, L"Rejected oversized THEX headers from %s",
					               (LPCTSTR)m_sAddress);
					Close(TRI_FALSE);
					return FALSE;
				}
				break;
			}
			if (m_nState != dtsHeaders) break;

		case dtsHeaders:
			if (!ReadHeaders()) return FALSE;
			// Header-phase receive is capped at 16 KiB. Fail closed only after
			// attempting to parse headers so a short header block plus a
			// prefetched body prefix is not rejected as "oversized headers".
			// OnHeadersComplete already ran ReadTiger when it set dtsTiger, so
			// never fall into the body reader in the same read; later reads
			// dispatch through case dtsTiger.
			if (m_nState != dtsTiger)
			{
				const DWORD nHeaderCap = 16u * 1024u;
				if (GetInputLength() >= nHeaderCap)
				{
					theApp.Message(MSG_ERROR, L"Rejected oversized THEX headers from %s",
					               (LPCTSTR)m_sAddress);
					Close(TRI_FALSE);
					return FALSE;
				}
			}
			break;

		case dtsTiger:
			return ReadTiger();

		case dtsFlushing:
			return ReadFlush();
		}

		return TRUE;
	}

	// Bound header-phase receive for ordinary file downloads too: the post-read
	// 16 KiB check alone cannot prevent an unbounded OnRead from growing
	// m_pInput when status/headers arrive in one large incomplete block.
	if (m_nState == dtsRequesting || m_nState == dtsHeaders)
	{
		const DWORD nHeaderCap = 16u * 1024u;
		if (!OnReadBounded(nHeaderCap))
			return FALSE;
	}
	else
	{
		CDownloadTransfer::OnRead();
	}

	switch ( m_nState )
	{
	case dtsRequesting:
		if ( ! ReadResponseLine() ) return FALSE;
		if (m_nState == dtsRequesting)
		{
			if (GetInputLength() >= 16u * 1024u)
			{
				theApp.Message(MSG_ERROR, L"Rejected oversized HTTP headers from %s",
				               (LPCTSTR)m_sAddress);
				Close(TRI_FALSE);
				return FALSE;
			}
			break;
		}
		if ( m_nState != dtsHeaders ) break;

	case dtsHeaders:
		if ( ! ReadHeaders() ) return FALSE;
		if (m_nState == dtsHeaders && GetInputLength() >= 16u * 1024u)
		{
			theApp.Message(MSG_ERROR, L"Rejected oversized HTTP headers from %s",
			               (LPCTSTR)m_sAddress);
			Close(TRI_FALSE);
			return FALSE;
		}
		if ( m_nState != dtsDownloading ) break;

	case dtsDownloading:
		return ReadContent();

	case dtsFlushing:
		return ReadFlush();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read response line

BOOL CDownloadTransferHTTP::ReadResponseLine()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CString strLine, strCode, strMessage;

	if ( ! Read( strLine ) ) return TRUE;
	if ( strLine.IsEmpty() ) return TRUE;

	if ( strLine.GetLength() > HTTP_HEADER_MAX_LINE )
		strLine = L"#LINE_TOO_LONG#";

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"%s >> DOWNLOAD RESPONSE: %s", (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	if ( strLine.GetLength() >= 12 && ::StartsWith( strLine, _P( L"HTTP/1.1 " ) ) )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
		m_bKeepAlive = TRUE;
	}
	else if ( strLine.GetLength() >= 12 && ::StartsWith( strLine, _P( L"HTTP/1.0 " ) ) )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
		m_bKeepAlive = FALSE;
	}
	else if ( strLine.GetLength() >= 8 && ::StartsWith( strLine, _P( L"HTTP" ) ) )
	{
		strCode		= strLine.Mid( 5, 3 );
		strMessage	= strLine.Mid( 8 );
		theApp.Message( MSG_DEBUG, L"HTTP with unknown version: %s", (LPCTSTR)strLine );
		m_bKeepAlive = FALSE;
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_NOHTTP, (LPCTSTR)m_sAddress );
		Close( TRI_FALSE );
		return FALSE;
	}

	if ( strCode == L"200" || strCode == L"206" )
	{
		SetState( dtsHeaders );
		m_pSource->m_nFailures = 0;
		m_pSource->m_nBusyCount = 0;
	}
	else if ( strCode == L"503" )
	{
		// 503 response without an X-Available-Ranges header means the complete file is available
		if ( _tcsistr( strMessage, L"range" ) != NULL )
		{
			m_bRangeFault = TRUE;
		}
		else
		{
			m_bBusyFault = TRUE;
			m_pSource->m_nFailures = 0;
		}

		SetState( dtsHeaders );
	}
	else if ( strCode == L"416" )
	{
		m_bRangeFault = TRUE;
		SetState( dtsHeaders );
	}
	else if ( strCode == L"301" || strCode == L"302" )
	{
		m_bRedirect = TRUE;
		SetState( dtsHeaders );
	}
	else
	{
		strMessage.TrimLeft();
		if ( strMessage.GetLength() > 128 ) strMessage = L"No Message";
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HTTPCODE, (LPCTSTR)m_sAddress, (LPCTSTR)strCode, (LPCTSTR)strMessage );
		SetState( dtsHeaders );
		m_bBadResponse = TRUE;
	}

	ClearHeaders();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read header lines

BOOL CDownloadTransferHTTP::OnHeaderLine(CString& strHeader, CString& strValue)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( ! CDownloadTransfer::OnHeaderLine( strHeader, strValue ) )
		return FALSE;

	CString strCase( strHeader );
	if ( strCase.GetLength() < 4 )
		return TRUE;	// Skip bad/unknown small header

	strCase.MakeLower();

	// Expected Headers
	SwitchMap( Text )
	{
		Text[ L"server" ]					= 'S';
		Text[ L"connection" ]				= 'C';
		Text[ L"location" ]					= 'L';
		Text[ L"retry-after" ]				= 'R';
		Text[ L"content-length" ]			= 'l';
		Text[ L"content-range" ]			= 'r';
		Text[ L"content-type" ]				= 't';
		Text[ L"content-language" ]			= 'y';
		Text[ L"content-encoding" ]			= 'e';
		Text[ L"transfer-encoding" ]		= 'E';
		Text[ L"content-urn" ]				= 'u';
		Text[ L"x-content-urn" ]			= 'u';
		Text[ L"x-gnutella-content-urn" ]	= 'u';
		Text[ L"x-metadata-path" ]			= 'm';
		Text[ L"x-tigertree-path" ]			= 'g';
		Text[ L"x-thex-uri" ]				= 'h';
		Text[ L"alt-location" ]				= 'a';
		Text[ L"x-alt" ]					= 'a';
		Text[ L"x-gnutella-alternate-location" ] = 'a';
		Text[ L"x-available-ranges" ]		= 'v';
		Text[ L"x-queue" ]					= 'q';
		Text[ L"x-perhost" ]				= 'p';
		Text[ L"x-gnutella-maxslotsperhost" ] = 'p';
		Text[ L"x-delete-source" ]			= 'd';
		Text[ L"x-nick" ]					= 'n';
		Text[ L"x-name" ]					= 'n';
		Text[ L"x-username" ]				= 'n';
		Text[ L"x-features" ]				= 'f';
		Text[ L"content-disposition" ]		= 'o';
		Text[ L"content-md5" ]				= '5';

		Text[ L"x-node" ]					= 'x';
		Text[ L"x-nalt" ]					= 'x';
		Text[ L"x-palt" ]					= 'x';
		Text[ L"fp-1a" ]					= 'x';
		Text[ L"fp-auth-challenge" ] 		= 'x';
		Text[ L"accept-ranges" ] 			= 'x';
		Text[ L"x-create-time" ] 			= 'x';
	}

	switch ( Text[ strCase ] )
	{
	case 'S':		// "Server"
		m_sUserAgent = strValue;
		m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );

		if ( Security.IsAgentBlocked( m_sUserAgent ) )
		{
			Close( TRI_FALSE );
			return FALSE;
		}

		m_pSource->m_sServer = strValue;
		if ( m_pSource->m_sServer.GetLength() > 64 ) m_pSource->m_sServer = m_pSource->m_sServer.Left( 64 );

		if ( m_bClientExtended ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, L"trustyfiles" ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, L"gnucdna" ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, L"vagaa" ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, L"mxie" ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, L"adagio" ) != NULL ) m_pSource->SetGnutella( 2 );
		break;

	case 'C':		// "Connection"
		if ( strValue.CompareNoCase( L"Keep-Alive" ) == 0 ) m_bKeepAlive = TRUE;
		if ( strValue.CompareNoCase( L"close" ) == 0 ) m_bKeepAlive = FALSE;
		break;

	case 'l': // "Content-Length"
	{
		QWORD nTotal;
		// Store Content-Length including 0. Control responses (503/416/redirect)
		// may advertise an empty body; reject zero only for ordinary file
		// content in OnHeadersComplete after those paths are classified.
		if (!ParsePeerUnsignedDecimal(strValue, nTotal) || nTotal >= SIZE_UNKNOWN ||
		    (m_nContentLength != SIZE_UNKNOWN && m_nContentLength != nTotal))
		{
			theApp.Message(MSG_DEBUG, L"Rejected invalid or conflicting Content-Length");
			Close(TRI_FALSE);
			return FALSE;
		}
		m_nContentLength = nTotal;
	}
	break;

	case 'r':		// "Content-Range"
		{
			QWORD nFirst, nLast, nTotal;
			if ( ( _stscanf( strValue, L"bytes %I64u-%I64u/%I64u", &nFirst, &nLast, &nTotal ) == 3 ||
				   _stscanf( strValue, L"bytes=%I64u-%I64u/%I64u", &nFirst, &nLast, &nTotal ) == 3 ) &&
				   nFirst <= nLast && nTotal > 0 && nLast < nTotal )
			{
				if ( m_bTigerFetch || m_bMetaFetch )
				{
					m_nOffset = nFirst;
					m_nLength = nLast + 1 - nFirst;
					if ( m_nContentLength == SIZE_UNKNOWN ) m_nContentLength = m_nLength;
					return TRUE;
				}
				else if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
				{
					m_pDownload->m_nSize = nTotal;
				}
				else if ( m_pDownload->m_nSize != nTotal )
				{
					theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
					Close( TRI_FALSE );
					return FALSE;
				}

				if ( m_nOffset == SIZE_UNKNOWN && ! m_pDownload->GetFragment( this ) )
				{
					Close( TRI_TRUE );
					return FALSE;
				}

				BOOL bUseful = m_pDownload->IsPositionEmpty( nFirst );
				// BOOL bUseful = m_pDownload->IsRangeUseful( nFirst, nLast - nFirst + 1 );

				if ( nFirst == m_nOffset && nLast == m_nOffset + m_nLength - 1 && bUseful )
				{
					// Perfect match, good
				}
				else if ( nFirst >= m_nOffset && nFirst < m_nOffset + m_nLength && bUseful )
				{
					m_nOffset = nFirst;
					m_nLength = nLast - nFirst + 1;

					theApp.Message( MSG_INFO, IDS_DOWNLOAD_USEFUL_RANGE,
						(LPCTSTR)m_sAddress, m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_pDownload->GetDisplayName() );
				}
				else
				{
					theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
					Close( TRI_TRUE );

					return FALSE;
				}

				if ( m_nContentLength == SIZE_UNKNOWN )
					m_nContentLength = m_nLength;
				m_bGotRange = TRUE;
			}
		}
		break;

	case 't':		// "Content-Type"
		m_sContentType = strValue;
		break;

	case 'e':		// "Content-Encoding"
		if ( _tcsistr( strValue, L"backwards" ) )
		{
			m_bRecvBackwards = TRUE;
			if ( ! Settings.Downloads.AllowBackwards )
			{
				theApp.Message( MSG_DEBUG, L"Backwards encoding disabled" );
				Close( TRI_FALSE );
				return FALSE;
			}
		}
		if ( _tcsistr( strValue, L"gzip" ) )		// gzip or x-gzip
		{
			m_bGzip = TRUE;
			theApp.Message( MSG_DEBUG, L"Gzip encoding not supported" );
			Close( TRI_FALSE );
			return FALSE;
		}
		if ( _tcsistr( strValue, L"compress" ) )	// compress or x-compress
		{
			m_bCompress = TRUE;
			theApp.Message( MSG_DEBUG, L"Compress encoding not supported" );
			Close( TRI_FALSE );
			return FALSE;
		}
		if ( _tcsistr( strValue, L"deflate" ) )	// deflate
		{
			m_bDeflate = TRUE;
			theApp.Message( MSG_DEBUG, L"Deflate encoding not supported" );
			Close( TRI_FALSE );
			return FALSE;
		}
		break;

	case 'y':		// "Content-Language"
		// ToDo: It would be nice to show language in the future
		break;

	case 'E':		// "Transfer-Encoding"
		if ( _tcsistr( strValue, L"chunked" ) )
		{
			m_bChunked = TRUE;
			m_ChunkState = Header;
			m_nChunkLength = SIZE_UNKNOWN;
		}
		else
		{
			theApp.Message( MSG_DEBUG, L"Unknown transfer encoding: %s", (LPCTSTR)strValue );
			Close( TRI_FALSE );
			return FALSE;
		}
		break;

	case 'u':		// "Content-URN" "X-Content-URN" "X-Gnutella-Content-URN"
		{
			Hashes::Sha1Hash oSHA1;
			Hashes::TigerHash oTiger;
			Hashes::Ed2kHash oED2K;
			Hashes::BtHash oBTH;
			Hashes::Md5Hash oMD5;
			CString strURNs = strValue + L',';
			for ( int nPos = strURNs.Find( L',' ); nPos >= 0; nPos = strURNs.Find( L',' ) )
			{
				strValue = strURNs.Left( nPos ).TrimLeft();
				strURNs = strURNs.Mid( nPos + 1 );

				if (   ( !  oSHA1.fromUrn( strValue ) || m_pSource->CheckHash( oSHA1 ) )
					&& ( ! oTiger.fromUrn( strValue ) || m_pSource->CheckHash( oTiger ) )
					&& ( !  oED2K.fromUrn( strValue ) || m_pSource->CheckHash( oED2K ) )
					&& ( !   oMD5.fromUrn( strValue ) || m_pSource->CheckHash( oMD5 ) )
					&& ( !   oBTH.fromUrn( strValue ) || ( m_pSource->CheckHash( oBTH ), TRUE ) ) )
				{
					continue;
				}
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
				Close( TRI_FALSE );
				return FALSE;
			}
		}
		m_pSource->SetGnutella( 1 );
		break;

	case 'm':		// "X-Metadata-Path"
		if ( Settings.Downloads.Metadata )
			m_sMetadata = strValue;
		break;

	case 'g':		// "X-TigerTree-Path"
		if ( Settings.Downloads.VerifyTiger && ! m_bTigerIgnore )
		{
			if ( strValue.Find( L"tigertree/v1" ) < 0 &&
				 strValue.Find( L"tigertree/v2" ) < 0 )
				m_sTigerTree = strValue;
		}
		break;

	case 'h':		// "X-Thex-URI"
		if ( Settings.Downloads.VerifyTiger && ! m_bTigerIgnore )
		{
			if ( strValue[ 0 ] == L'/' )
			{
				m_sTigerTree = strValue.SpanExcluding( L"; " );
				m_sTigerTree.Replace( L"ed2k=0", L"ed2k=1" );
			}
		}
		m_pSource->SetGnutella( 1 );
		break;

	case 'a':		// "X-Gnutella-Alternate-Location" "Alt-Location" "X-Alt"
		if ( Settings.Library.SourceMesh )
			m_pDownload->AddSourceURLs( strValue );
		m_pSource->SetGnutella( 1 );
		break;

	case 'v':		// "X-Available-Ranges"
		m_bGotRanges = TRUE;
		m_pSource->SetAvailableRanges( strValue );
		m_pSource->SetGnutella( 1 );
		if ( m_pSource->m_oAvailable.empty() )
		{
			theApp.Message( MSG_DEBUG, L"Header did not include valid ranges, dropping source..." );
			Close( TRI_FALSE );
			return FALSE;
		}
		break;

	case 'q':		// "X-Queue"
		m_pSource->SetGnutella( 1 );
		m_bQueueFlag = TRUE;
		{
			ToLower( strValue );

			int nPos = strValue.Find( L"position=" );
			if ( nPos >= 0 )
				_stscanf( strValue.Mid( nPos + 9 ), L"%lu", &m_nQueuePos );

			nPos = strValue.Find( L"length=" );
			if ( nPos >= 0 )
				_stscanf( strValue.Mid( nPos + 7 ), L"%lu", &m_nQueueLen );

			DWORD nLimit = 0;

			nPos = strValue.Find( L"pollmin=" );
			if ( nPos >= 0 && _stscanf( strValue.Mid( nPos + 8 ), L"%lu", &nLimit ) == 1 )
				m_nRetryDelay = max( m_nRetryDelay, nLimit * 1000 + 3000 );

			nPos = strValue.Find( L"pollmax=" );
			if ( nPos >= 0 && _stscanf( strValue.Mid( nPos + 8 ), L"%lu", &nLimit ) == 1 )
				m_nRetryDelay = min( m_nRetryDelay, nLimit * 1000 - 8000 );

			nPos = strValue.Find( L"id=" );
			if ( nPos >= 0 )
			{
				m_sQueueName = strValue.Mid( nPos + 3 );
				m_sQueueName.TrimLeft();
				if ( m_sQueueName.Find( L'\"' ) == 0 )
					m_sQueueName = m_sQueueName.Mid( 1 ).SpanExcluding( L"\"" );
				else
					m_sQueueName = m_sQueueName.SpanExcluding( L"\" " );

				if ( m_sQueueName == L"s" )
					m_sQueueName = L"Small Queue";
				else if ( m_sQueueName == L"l" )
					m_sQueueName = L"Large Queue";
			}
		}
		break;

	case 'R':		// "Retry-After"
		{
			DWORD nLimit = 0;
			if ( _stscanf( strValue, L"%lu", &nLimit ) == 1 )
				m_nRetryAfter = nLimit;
		}
		break;

	case 'p':		// "X-PerHost" "X-Gnutella-maxSlotsPerHost"
		{
			DWORD nLimit = 0;
			if ( _stscanf( strValue, L"%lu", &nLimit ) != 1 )
				Downloads.SetPerHostLimit( &m_pHost.sin_addr, nLimit );
		}
		break;

	case 'd':		// "X-Delete-Source"
		m_bBadResponse = TRUE;
		break;

	case 'n':		// "X-Nick" "X-Name" "X-UserName"
		m_pSource->m_sNick = URLDecode( strValue );
		break;

	case 'f':		// "X-Features"
		if ( _tcsistr( strValue, L"g2/" ) != NULL )
			m_pSource->SetGnutella( 2 );
		else if ( _tcsistr( strValue, L"gnutella2/" ) != NULL )
			m_pSource->SetGnutella( 2 );
		else if ( _tcsistr( strValue, L"gnet2/" ) != NULL )
			m_pSource->SetGnutella( 2 );
		m_pSource->SetGnutella( 1 );
		break;

	case 'L':		// "Location"
		m_sRedirectionURL = strValue;
		m_pDownload->SetStableName( false );
		break;

	case 'o':		// "Content-Disposition"
		// Accept Content-Disposition only if the current display name is empty or if it likely came from Web Servers default.
		if ( ! m_pDownload->HasStableName() )
		{
			const int nPos = strValue.Find( L"filename=" );
			if ( nPos >= 0 )
			{
				// If exactly, it should follow RFC 2184 rules
				const CString strFilename = SafeFilename( URLDecode( strValue.Mid( nPos + 9 ).Trim( L" \t\r\n\"" ) ) );
				if ( m_pDownload->Rename( strFilename ) )
				{
					m_pDownload->SetStableName();
					DownloadGroups.Link( m_pDownload );
					theApp.Message( MSG_DEBUG, L"Content-Disposition set filename: %s", (LPCTSTR)strFilename);
				}
			}
		}
		break;

	case '5':		// "Content-MD5"
		{
			Hashes::Md5Hash oMD5;
			oMD5.fromString( strValue );
			theApp.Message( MSG_DEBUG, L"Content-MD5: %s", (LPCTSTR)oMD5.toString() );
		}
		break;

	case 'x':		// Unsupported Gnutella "X-Node" "X-NAlt" "X-PAlt" "FP-1a" "FP-Auth-Challenge" "Accept-Ranges" "X-Create-Time"
		m_pSource->SetGnutella( 1 );
		break;

	default:		// Unknown Headers?
		theApp.Message( MSG_DEBUG, L"Unknown header: %s", (LPCTSTR)strCase );
		break;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP end of headers

BOOL CDownloadTransferHTTP::OnHeadersComplete()
{
	// Close parameters:
	// TRI_FALSE	- the source will be added to m_pFailedSources in CDownloadWithSources,
	//					removed from the sources and can be distributed in the Source Mesh as X-Nalt
	// TRI_TRUE		- keeps the source and will be distributed as X-Alt
	// TRI_UNKNOWN	- keeps the source and will be dropped after several retries, will be
	//				- added to m_pFailedSources when removed

	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_bBadResponse )
	{
		Close( TRI_FALSE );
		return FALSE;
	}

	if ( m_bRedirect )
	{
		CEnvyURL pURL( m_sRedirectionURL );
		m_pDownload->AddSourceHit( pURL, TRUE, m_pSource->m_nRedirectionCount + 1 );
		Close( TRI_FALSE );
		return FALSE;
	}

	if ( ! m_pSource->CanInitiate( TRUE, TRUE ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DISABLED, (LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		Close( m_pSource->m_bED2K ? TRI_FALSE : TRI_UNKNOWN );
		return FALSE;
	}

	if ( m_bBusyFault )
	{
		m_nOffset = SIZE_UNKNOWN;

		if ( Settings.Downloads.QueueLimit > 0 && m_nQueuePos > Settings.Downloads.QueueLimit )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_QUEUE_HUGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName(), m_nQueuePos );
			Close( TRI_FALSE );
			return FALSE;
		}
		else if ( m_bQueueFlag && m_nRetryDelay >= 600000 )
		{
			m_pSource->m_tAttempt = GetTickCount() + m_nRetryDelay;
			m_bQueueFlag = FALSE;
		}

		if ( m_bQueueFlag )
		{
			SetState( dtsFlushing );
			m_tContent = m_mInput.tLast = GetTickCount();
			return ReadFlush();
		}
		else
		{
			SetState( dtsBusy );
			m_pSource->m_nBusyCount++;
			m_tRequest = GetTickCount();
			return TRUE;
		}
	}
	else if ( ! m_bGotRanges && ! m_bTigerFetch && ! m_bMetaFetch )
	{
		m_pSource->SetAvailableRanges( NULL );
	}

	if ( m_bRangeFault )
	{
		if ( Network.IsSelfIP( m_pHost.sin_addr ) )
		{
			Close( TRI_TRUE );
			return FALSE;
		}

		m_nOffset = SIZE_UNKNOWN;
		SetState( dtsFlushing );
		m_tContent = m_mInput.tLast = GetTickCount();

		return ReadFlush();
	}
	else if ( m_bTigerFetch )
	{
		if (m_bChunked)
		{
			theApp.Message(MSG_DEBUG, L"Rejected chunked THEX response");
			Close(TRI_FALSE);
			return FALSE;
		}
		// Keep-alive unknown Content-Length has no completion signal for DIME;
		// reject it like MetaFetch. Close-delimited unknown length is finalized
		// in OnDropped.
		if (m_nContentLength == SIZE_UNKNOWN && m_bKeepAlive)
		{
			theApp.Message(MSG_ERROR, L"Rejected unbounded keep-alive THEX from %s",
			               (LPCTSTR)m_sAddress);
			Close(TRI_FALSE);
			return FALSE;
		}
		// Reject empty or oversized known lengths before buffering.
		if (m_nContentLength != SIZE_UNKNOWN &&
		    !AdmitThexBodyLength(m_nContentLength))
		{
			theApp.Message(MSG_ERROR,
			               m_nContentLength == 0
			                   ? L"Rejected empty THEX response from %s"
			                   : L"Rejected oversized THEX Content-Length from %s",
			               (LPCTSTR)m_sAddress);
			Close(TRI_FALSE);
			return FALSE;
		}
		if ( m_nContentLength == SIZE_UNKNOWN && ! m_bKeepAlive )
		{
			// This should fix the PHEX TTH problem with closed connection.
			SetState( dtsTiger );
			theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_RECV, (LPCTSTR)m_sAddress, (LPCTSTR)m_pSource->m_sServer );

			return ReadTiger(); 	// Doesn't actually read but updates timings
		}
		if ( ! m_bGotRange )
		{
			m_nOffset = 0;
			m_nLength = m_nContentLength;
		}
		else if ( m_nOffset > 0 )
		{
			theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_RANGE, (LPCTSTR)m_sAddress );
			Close( TRI_FALSE );
			return FALSE;
		}

		if ( m_sContentType.CompareNoCase( L"application/tigertree-breadthfirst" ) &&
			 m_sContentType.CompareNoCase( L"application/dime" ) &&
			 m_sContentType.CompareNoCase( L"application/binary" ) )		// Content Type used by Phex
		{
			theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_RANGE, (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}

		SetState( dtsTiger );
		m_tContent = m_mInput.tLast = GetTickCount();

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_RECV, (LPCTSTR)m_sAddress, (LPCTSTR)m_pSource->m_sServer );

		return ReadTiger();
	}
	else if ( m_bMetaFetch )
	{
		// Cap known Content-Length before buffering/materializing peer metadata XML.
		// Close-delimited unknown length is finalized in OnDropped (byte-capped).
		// Chunked metadata is rejected: dtsMetadata has no chunk decoder (unlike
		// ReadContent), so accepting Transfer-Encoding: chunked would feed framing
		// bytes to the XML parser or hang keep-alive transfers.
		if (m_bChunked)
		{
			theApp.Message(MSG_ERROR, L"Rejected chunked peer metadata from %s",
			               (LPCTSTR)m_sAddress);
			Close(TRI_FALSE);
			return FALSE;
		}
		if (m_nContentLength != SIZE_UNKNOWN &&
		    m_nContentLength > XML_PEER_PARSE_CHARS_MAX)
		{
			theApp.Message(MSG_ERROR, L"Rejected oversized peer metadata from %s",
			               (LPCTSTR)m_sAddress);
			Close(TRI_FALSE);
			return FALSE;
		}
		if (m_nContentLength == SIZE_UNKNOWN && m_bKeepAlive)
		{
			theApp.Message(MSG_ERROR, L"Rejected unbounded keep-alive peer metadata from %s",
			               (LPCTSTR)m_sAddress);
			Close(TRI_FALSE);
			return FALSE;
		}

		if ( ! m_bGotRange )
		{
			m_nOffset = 0;
			m_nLength = m_nContentLength;
		}
		else if (m_nLength != SIZE_UNKNOWN && m_nLength > XML_PEER_PARSE_CHARS_MAX)
		{
			Close(TRI_FALSE);
			return FALSE;
		}

		SetState( dtsMetadata );
		m_tContent = m_mInput.tLast = GetTickCount();

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_METADATA_RECV, (LPCTSTR)m_sAddress, (LPCTSTR)m_pSource->m_sServer );

		return ReadMetadata();
	}
	else if (m_nContentLength == 0 &&
	         RejectExplicitZeroContentLength(m_bMetaFetch != FALSE, m_bTigerFetch != FALSE,
	                                         m_bBusyFault != FALSE, m_bRangeFault != FALSE,
	                                         m_bRedirect != FALSE))
	{
		// Reject before SetSize or GetFragment can mutate an unknown download.
		// Control, metadata and THEX responses were classified above.
		theApp.Message(MSG_ERROR, L"Rejected zero Content-Length file download from %s",
		               (LPCTSTR)m_sAddress);
		Close(TRI_FALSE);
		return FALSE;
	}
	else if ( ! m_bGotRange )
	{
		if ( m_nContentLength != SIZE_UNKNOWN )
		{
			if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
			{
				m_pDownload->SetSize( m_nContentLength );
			}
			else if ( m_pDownload->m_nSize != m_nContentLength )
			{
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
				Close( TRI_FALSE );
				return FALSE;
			}
		}
		if ( m_nOffset == SIZE_UNKNOWN && ! m_pDownload->GetFragment( this ) )
		{
			Close( TRI_TRUE );
			return FALSE;
		}

		if ( ! m_pDownload->IsPositionEmpty( 0 ) )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
			Close( TRI_TRUE );
			return FALSE;
		}

		m_nOffset = 0;
		m_nLength = m_nContentLength;
	}
	else if ( CFailedSource* pBadSource = m_pDownload->LookupFailedSource( m_pSource->m_sURL ) )
	{
		// We already have it added to the list but the source was offline
		if ( pBadSource->m_bOffline )
		{
			pBadSource->m_bOffline = false;
		}
		else
		{
			// Extend the period of keeping it in the failed sources list
			pBadSource->m_nTimeAdded = GetTickCount();
			Close( TRI_FALSE );
			return FALSE;
		}
	}

	if ( m_nContentLength != m_nLength )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
		Close( TRI_FALSE );
		return FALSE;
	}

	if ( ! m_bKeepAlive ) m_pSource->m_bCloseConn = TRUE;

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONTENT, (LPCTSTR)m_sAddress, (LPCTSTR)m_pSource->m_sServer );

	SetState( dtsDownloading );
	if ( ! m_pDownload->IsBoosted() )
		m_mInput.pLimit = m_mOutput.pLimit = &m_nBandwidth;
	m_nPosition = 0;
	m_tContent = m_mInput.tLast = GetTickCount();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read content

BOOL CDownloadTransferHTTP::ReadContent()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CLockedBuffer pInput( GetInput() );

	while ( pInput->m_nLength > 0 )
	{
		m_pSource->SetValid();

		size_t nLength	= min( pInput->m_nLength, (DWORD)( m_nLength - m_nPosition ) );
		BOOL bSubmit	= FALSE;

		if ( m_bChunked )
		{
			BOOL bBreak = FALSE;
			switch ( m_ChunkState )
			{
			case Header:
				if ( pInput->m_nLength >= 3 )
				{
					// Looking for "Length<CR><LF>"
					DWORD i = 1;
					for ( ; i < pInput->m_nLength - 1; i++ )
					{
						if ( pInput->m_pBuffer[ i ] == 0x0d &&
							 pInput->m_pBuffer[ i + 1 ] == 0x0a )
						{
							break;
						}
					}
					if ( i < pInput->m_nLength - 1 )
					{
						if ( sscanf_s( (LPCSTR)pInput->m_pBuffer, "%I64x",
							&m_nChunkLength ) == 1 )
						{
							if ( m_nChunkLength == 0 )
							{
								// Got last chunk "0<CR><LF>"
								m_ChunkState = Footer;

								// Now file size is known
								if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
								{
									m_pDownload->SetSize( m_nDownloaded );
									m_pDownload->MakeComplete();
								}
							}
							else
								m_ChunkState = Body;

							// Cut header
							pInput->Remove( i + 2 );

							// Process rest of data
							continue;
						}
						else
						{
							// Bad format
							pInput->Clear();
							Close( TRI_FALSE );
							return FALSE;
						}
					}
				}
				bBreak = TRUE;
				break;

			case Body:
				ASSERT( m_nChunkLength != 0 );
				ASSERT( m_nChunkLength != SIZE_UNKNOWN );

				if ( nLength > m_nChunkLength )
				{
					// m_nChunkLength must be smaller than nLength so this cast is safe
					nLength = static_cast< size_t >( m_nChunkLength );
				}
				m_nChunkLength -= nLength;
				if ( m_nChunkLength == 0 )
				{
					// Whole chunk readed
					m_ChunkState = BodyEnd;
					m_nChunkLength = SIZE_UNKNOWN;
				}
				break;

			case BodyEnd:
				ASSERT( m_nChunkLength == SIZE_UNKNOWN );

				// Looking for "<CR><LF>"
				if ( pInput->m_nLength >= 2 )
				{
					if ( pInput->m_pBuffer[ 0 ] == 0x0d &&
						 pInput->m_pBuffer[ 1 ] == 0x0a )
					{
						m_ChunkState = Header;

						// Cut <CR><LF>
						pInput->Remove( 2 );

						continue;	// Process rest of data
					}
					else
					{
						// Bad Format
						pInput->Clear();
						Close( TRI_FALSE );
						return FALSE;
					}
				}
				bBreak = TRUE;
				break;

			case Footer:
				ASSERT( m_nChunkLength == 0 );

				// Bypass footer
				pInput->Clear();
				bBreak = TRUE;
				break;
			}
			if ( bBreak )
				break;
		}

		if ( m_bRecvBackwards )
		{
			//BYTE* pBuffer = new BYTE[ nLength ];	// Obsolete
			CAutoVectorPtr< BYTE >pBuffer( new BYTE[ nLength ] );
			if ( ! pBuffer )
			{
				Close( TRI_TRUE );
				return FALSE;	// Out of memory
			}
			CBuffer::ReverseBuffer( pInput->m_pBuffer, pBuffer, nLength );
			bSubmit = m_pDownload->SubmitData( m_nOffset + m_nLength - m_nPosition - nLength, pBuffer, nLength );
		}
		else
		{
			bSubmit = m_pDownload->SubmitData(
				m_nOffset + m_nPosition, pInput->m_pBuffer, nLength );
		}

		if ( m_bChunked )
			pInput->Remove( nLength );
		else
			pInput->Clear();

		m_nPosition += nLength;
		m_nDownloaded += nLength;

		if ( ! bSubmit )
		{
			BOOL bUseful = m_pDownload->IsRangeUsefulEnough( this,
				m_bRecvBackwards ? m_nOffset : m_nOffset + m_nPosition,
				m_nLength - m_nPosition );
			if ( ! bUseful /*|| m_bInitiated*/ )
			{
				theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_OVERLAP, (LPCTSTR)m_sAddress );
				Close( TRI_TRUE );
				return FALSE;
			}
		}
	}

	if ( m_nPosition >= m_nLength )
	{
		m_pSource->AddFragment( m_nOffset, m_nLength );
		return StartNextFragment();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP receive THEX (byte-capped)

BOOL CDownloadTransferHTTP::ReceiveTigerInput()
{
	// Header phase: bound receive at 16 KiB (no tree slack). Do not fail closed
	// here - OnRead parses headers first; a single read may include a valid
	// body prefix after short headers. Body phase: XML + tree slack, fail closed.
	const DWORD nHeaderCap = 16u * 1024u;
	if (m_nState == dtsTiger)
	{
		const DWORD nBodyCap = XML_PEER_THEX_BODY_CAP;
		// Admit exactly nBodyCap; read one extra byte so overflow is detectable.
		const DWORD nOverflowCap = nBodyCap + 1u;
		if (!OnReadBounded(nOverflowCap))
			return FALSE;
		if (GetInputLength() > nBodyCap)
		{
			theApp.Message(MSG_ERROR, L"Rejected oversized THEX/DIME payload from %s",
			               (LPCTSTR)m_sAddress);
			Close(TRI_FALSE);
			return FALSE;
		}
		return TRUE;
	}

	return OnReadBounded(nHeaderCap);
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP receive Metadata (byte-capped)

BOOL CDownloadTransferHTTP::ReceiveMetadataInput()
{
	// Body budget is XML_PEER_PARSE_CHARS_MAX (+1 overflow detect). While the
	// status/headers are still in m_pInput (before dtsMetadata), allow a small
	// header slack so a Content-Length of exactly the body max is not rejected
	// merely because header bytes share the buffer.
	const DWORD nHeaderSlack = (m_nState == dtsMetadata) ? 0u : (16u * 1024u);
	const DWORD nHardCap = XML_PEER_PARSE_CHARS_MAX + 1u + nHeaderSlack;
	if (!OnReadBounded(nHardCap))
		return FALSE;

	// Incomplete headers at the hard cap: ReadHeaders() returns TRUE when no
	// complete line is available, which would otherwise spin forever.
	if (m_nState != dtsMetadata && GetInputLength() >= nHardCap)
	{
		theApp.Message(MSG_ERROR, L"Rejected oversized peer metadata headers from %s",
		               (LPCTSTR)m_sAddress);
		Close(TRI_FALSE);
		return FALSE;
	}

	// Enforce the body-only cap once headers have been consumed.
	if (m_nState == dtsMetadata && GetInputLength() > XML_PEER_PARSE_CHARS_MAX)
	{
		theApp.Message(MSG_ERROR, L"Rejected oversized peer metadata from %s",
		               (LPCTSTR)m_sAddress);
		Close(TRI_FALSE);
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read Metadata

BOOL CDownloadTransferHTTP::ReadMetadata(bool bDropped)
{
	CLockedBuffer pInput( GetInput() );

	// Cap while buffering so close-delimited bodies cannot grow without bound.
	if (pInput->m_nLength > XML_PEER_PARSE_CHARS_MAX)
	{
		Close(TRI_FALSE);
		return FALSE;
	}

	QWORD nBody = m_nLength;
	if (nBody == SIZE_UNKNOWN)
	{
		// Close-delimited responses are complete only when the connection drops.
		// OnDropped sets m_nLength / m_nContentLength from the final buffer.
		// Keep-alive unknown length is rejected at response accept time.
		return TRUE;
	}
	else if (nBody > XML_PEER_PARSE_CHARS_MAX)
	{
		Close(TRI_FALSE);
		return FALSE;
	}

	if (pInput->m_nLength < nBody) return TRUE;

	CString strXML = pInput->ReadString((DWORD)nBody, CP_UTF8);

	if (CXMLElement* pXML = CXMLElement::FromPeerString(strXML, TRUE))
	{
		m_pDownload->MergeMetadata( pXML );
		delete pXML;
	}

	pInput->Remove((DWORD)nBody);

	// On a dropped connection OnDropped closes (and deletes) the transfer right
	// after this call; StartNextFragment can Close first, so never run it here.
	if (bDropped)
		return TRUE;

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read tiger tree

BOOL CDownloadTransferHTTP::ReadTiger(bool bDropped)
{
	// It is a fix for very slow DIME uploads, they get dropped while downloading (ex LimeWire).
	m_tContent = m_mInput.tLast = GetTickCount();

	// Fix for PHEX TTH which never tell content length for DIME block to get into DIME decoding
	// until the connection drops, if no content length specified and not keep-alive.
	if ( ! m_bKeepAlive && m_nContentLength == SIZE_UNKNOWN ) return TRUE;

	CLockedBuffer pInput( GetInput() );

	if ( m_sContentType.CompareNoCase( L"application/tigertree-breadthfirst" ) == 0 )
	{
		// m_nLength is the unconsumed remainder. Zero means a prior ReadTiger
		// already consumed the body; OnDropped may re-enter without re-applying
		// SetTigerTree. A positive remainder still outstanding must not be
		// overwritten with GetInputLength() on drop.
		if (m_nLength != 0)
		{
			if (pInput->m_nLength < m_nLength) return TRUE;

			if (m_nLength > MAXDWORD ||
			    !ThexTreeMatchesDepth(m_pDownload->m_nSize, (DWORD)m_nLength) ||
			    !m_pDownload->SetTigerTree(pInput->m_pBuffer, (DWORD)m_nLength))
			{
				if (bDropped) return FALSE;
				Close(TRI_FALSE);
				return FALSE;
			}
			pInput->Remove((DWORD)m_nLength);
			m_nLength = 0;
		}
	}
	else if ( m_sContentType.CompareNoCase( L"application/dime" ) == 0 ||
			  m_sContentType.CompareNoCase( L"application/binary" ) == 0 )
	{
		CString strID, strType;
		DWORD nFlags, nBody, nHeader;

		// Process DIME records as their headers become visible so oversized
		// text/xml bodies are rejected before ReadDIME waits for the full body.
		// m_nLength is the unconsumed Content-Length remaining across ReadTiger
		// calls (decremented when a record is fully consumed or abandoned) so
		// multi-record responses split across reads can complete under
		// keep-alive. An abandoned response stops parsing and drains below.
		while (!m_bTigerAbandon && pInput->PeekDIME(&nFlags, &strID, &strType, &nBody, &nHeader))
		{
			theApp.Message( MSG_DEBUG, L"THEX DIME: %u, '%s', '%s', %u", nFlags, (LPCTSTR)strID, (LPCTSTR)strType, nBody );

			if (strType.CompareNoCase(L"text/xml") == 0 &&
			    (nBody == 0 || nBody > XML_PEER_PARSE_CHARS_MAX))
			{
				theApp.Message(MSG_DEBUG, L"THEX DIME: rejecting oversized text/xml body (%u)", nBody);
				// Close() on the drop path would delete the transfer while
				// OnDropped still uses it; report failure so OnDropped
				// fail-closes instead of resuming the source.
				if (bDropped) return FALSE;
				Close(TRI_FALSE);
				return FALSE;
			}

			// (nBody + 3) & ~3 must not wrap a DWORD. Values near MAXDWORD
			// (e.g. 0xfffffffe) make nPaddedBody 0 so the availability check
			// passes with only the header present; ReadDIME then hands the
			// huge declared length to SetTigerTree/SetHashset.
			if (nBody > MAXDWORD - 3u)
			{
				theApp.Message(MSG_DEBUG, L"THEX DIME: rejecting unpaddable body length (%u)", nBody);
				if (bDropped) return FALSE;
				Close(TRI_FALSE);
				return FALSE;
			}

			const DWORD nPaddedBody = (nBody + 3) & ~3;
			const QWORD nRecordBytes = (QWORD)nHeader + nPaddedBody;
			// Check framing from the header, before waiting for an impossible body.
			// Chunked DIME records are unsupported; MB/ME delimit one HTTP response.
			if (nRecordBytes > MAXDWORD ||
			    (m_nLength != SIZE_UNKNOWN && nRecordBytes > m_nLength) ||
			    (pInput->m_pBuffer[0] & 1) || m_bTigerDimeEnded ||
			    (((nFlags & 1) != 0) == (m_bTigerDimeStarted != FALSE)) ||
			    (m_nLength != SIZE_UNKNOWN && ((nFlags & 2) != 0) != (nRecordBytes == m_nLength)))
			{
				theApp.Message(MSG_DEBUG, L"THEX DIME: invalid message framing");
				if (bDropped) return FALSE;
				Close(TRI_FALSE);
				return FALSE;
			}
			if (pInput->m_nLength < nRecordBytes) break;
			m_bTigerDimeStarted = TRUE;
			m_bTigerDimeEnded = (nFlags & 2) != 0;

			if (!pInput->ReadDIME(&nFlags, &strID, &strType, &nBody))
				break;

			if (strType.CompareNoCase(L"text/xml") == 0)
			{
				BOOL bSize = FALSE, bDigest = FALSE, bEncoding = FALSE;
				QWORD nDepth = 0;

				CString strXML = pInput->ReadString( nBody, CP_UTF8 );

				if (CXMLElement* pXML = CXMLElement::FromPeerString(strXML))
				{
					if ( pXML->IsNamed( L"hashtree" ) )
					{
						if ( CXMLElement* pxFile = pXML->GetElementByName( L"file" ) )
						{
							QWORD nSize = 0;
							QWORD nSegment = 0;
							bSize = ParsePeerUnsignedDecimal(pxFile->GetAttributeValue(L"size"), nSize) &&
							        nSize != 0 && nSize == m_pDownload->m_nSize &&
							        ParsePeerUnsignedDecimal(pxFile->GetAttributeValue(L"segmentsize"), nSegment) &&
							        nSegment == 1024;
						}
						if ( CXMLElement* pxDigest = pXML->GetElementByName( L"digest" ) )
						{
							if ( pxDigest->GetAttributeValue( L"algorithm" ).CompareNoCase( L"http://open-content.net/spec/digest/tiger" ) == 0 )
								bDigest = ( pxDigest->GetAttributeValue( L"outputsize" ) == L"24" );
						}
						if ( CXMLElement* pxTree = pXML->GetElementByName( L"serializedtree" ) )
						{
							bEncoding = pxTree->GetAttributeValue(L"type").CompareNoCase(L"http://open-content.net/spec/thex/breadthfirst") == 0 &&
							            ParsePeerUnsignedDecimal(pxTree->GetAttributeValue(L"depth"), nDepth) && nDepth < 64;
							m_sTigerUUID = pxTree->GetAttributeValue(L"uri");
						}
					}
					delete pXML;
				}

				theApp.Message( MSG_DEBUG, L"THEX XML: size=%i, digest=%i, encoding=%i", bSize, bDigest, bEncoding );

				if (!(nFlags & 1) || m_bTigerDescriptor || !bSize || !bDigest || !bEncoding)
				{
					// ReadDIME consumed only this record's header, so the
					// abandoned record must be accounted here (drop its body
					// and padding, stop counting its bytes) or the wait below
					// stalls a fully received keep-alive body. The rest of the
					// response is often still undelivered, so drain it below
					// instead of clearing it now: leftover Content-Length
					// bytes would otherwise be parsed as the next response
					// and desynchronize a keep-alive connection.
					pInput->Remove(nPaddedBody);
					if (m_nLength != SIZE_UNKNOWN)
						m_nLength -= nRecordBytes;
					m_bTigerAbandon = TRUE;
					break;
				}
				m_bTigerDescriptor = TRUE;
				m_nTigerDepth = (DWORD)nDepth;
			}
			else if (strType.CompareNoCase(L"http://open-content.net/spec/thex/breadthfirst") == 0)
			{
				// An empty DIME ID is a Shareaza tolerance after valid XML. A missing
				// descriptor URI is tolerable only with a pre-existing root to compare;
				// an empty UUID alone must never authorize adoption of a peer's tree.
				const bool bIdentity = m_bTigerDescriptor &&
				                       (m_sTigerUUID.IsEmpty() ? !!m_pDownload->m_oTiger : (strID == m_sTigerUUID || strID.IsEmpty()));
				if (bIdentity)
				{
					if (m_bTigerTreeSeen ||
					    !ThexTreeMatchesDepth(m_pDownload->m_nSize, nBody, m_nTigerDepth) ||
					    !m_pDownload->SetTigerTree(pInput->m_pBuffer, nBody))
						m_bTigerAbandon = TRUE;
					m_bTigerTreeSeen = TRUE;
				}
			}
			else if ( strType.CompareNoCase( L"http://edonkey2000.com/spec/md4-hashset" ) == 0 )
			{
				m_pDownload->SetHashset( pInput->m_pBuffer, nBody );
			}

			pInput->Remove(nPaddedBody);
			if (m_nLength != SIZE_UNKNOWN)
				m_nLength -= nRecordBytes;
			// A trailing MD4 hashset is allowed inside the same DIME message.
		}

		// Wait when Content-Length bytes remain (or close-delimited unknown
		// length; keep-alive unknown is rejected at accept). Do not Clear a
		// partial DIME buffer. m_nLength is persisted across ReadTiger calls.
		// A complete buffered body that PeekDIME cannot parse is malformed:
		// waiting on it alone would spin forever because no further bytes
		// will arrive, so fail closed instead. An abandoned response reaches
		// the same completion with an unprocessed remainder; drain it so the
		// next request starts on a synchronized keep-alive connection.
		if (!bDropped &&
		    (m_nLength == SIZE_UNKNOWN ||
		     (m_nLength > 0 && pInput->m_nLength < m_nLength)))
			return TRUE;

		if (!m_bTigerAbandon && m_nLength != SIZE_UNKNOWN && m_nLength > 0)
		{
			theApp.Message(MSG_DEBUG, L"THEX DIME: malformed header on complete buffered response");
			// OnDropped already owns Close/delete; do not Close here on the
			// dropped path or the second Close uses a freed transfer.
			if (bDropped) return FALSE;
			Close(TRI_FALSE);
			return FALSE;
		}

		m_bTigerAbandon = FALSE;
		pInput->Clear();
	}
	else if (pInput->m_nLength < m_nLength)
	{
		return TRUE;
	}

	// m_bKeepAlive == FALSE means that it was not keep-alive, so should just get disconnected after reading DIME message.
	// ToDo: This might be better returning FALSE because it is not keep alive connection: need to disconnect after business.
	if ( bDropped || ! m_bKeepAlive )
		return TRUE;

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read flushing

BOOL CDownloadTransferHTTP::ReadFlush()
{
	CLockedBuffer pInput( GetInput() );

	if ( m_nContentLength == SIZE_UNKNOWN ) m_nContentLength = 0;

	const DWORD nRemove = (DWORD)min( pInput->m_nLength, (DWORD)m_nContentLength );
	m_nContentLength -= nRemove;

	pInput->Remove( nRemove );

	if ( m_nContentLength != 0 )
		return TRUE;

	if ( m_bQueueFlag )
	{
		SetState( dtsQueued );
		if ( ! m_pDownload->IsBoosted() )
			m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;
		m_tRequest = GetTickCount() + m_nRetryDelay;

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_QUEUED, (LPCTSTR)m_sAddress, m_nQueuePos, m_nQueueLen, (LPCTSTR)m_sQueueName );
		return TRUE;
	}

	if ( m_bRangeFault )
	{
		if ( ! m_bGotRanges )
		{
			// A "requested range unavailable" error but the source doesn't
			// advertise available ranges: don't guess, try again later
			theApp.Message( MSG_INFO, IDS_DOWNLOAD_416_WITHOUT_RANGE, (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}

		if ( m_nRequests > 1 )
		{
			// Made two requests already and the source does advertise available ranges,
			// but we still managed to request a wrong one.  ToDo: Determine if/why this is still happening
			theApp.Message( MSG_ERROR, L"BUG: Envy requested a fragment from host %s, although it knew that the host doesn't have that fragment", (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
	}

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP dropped connection handler

void CDownloadTransferHTTP::OnDropped()
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_nState == dtsConnecting )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
		if ( m_pSource != NULL ) m_pSource->PushRequest();
		Close( TRI_UNKNOWN );
	}
	else if ( m_nState == dtsBusy )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
		Close( TRI_TRUE, m_nRetryAfter );
	}
	else if ( m_nState == dtsTiger )
	{
		// This is basically for PHEX DIME download
		theApp.Message( MSG_DEBUG, L"Reading THEX from the closed connection..." );
		if (m_nContentLength != SIZE_UNKNOWN)
		{
			// Known Content-Length: m_nLength is the unconsumed remainder. Do
			// not overwrite it with GetInputLength() -- a keep-alive drop with
			// an empty/short body would look complete and resume the source.
			// Missing remainder tracking also fail-closes: without m_nLength we
			// cannot prove the declared body arrived intact.
			if (m_nLength == SIZE_UNKNOWN || GetInputLength() < m_nLength)
			{
				theApp.Message(MSG_ERROR, L"Incomplete THEX from %s",
				               (LPCTSTR)m_sAddress);
				Close(TRI_FALSE);
				return;
			}
		}
		else
		{
			// Close-delimited unknown length: finalize from the buffer when
			// the connection is cut (legacy PHEX). Empty body fails closed.
			m_nLength = m_nContentLength = GetInputLength();
			if (m_nLength == 0)
			{
				theApp.Message(MSG_ERROR, L"Incomplete THEX from %s",
				               (LPCTSTR)m_sAddress);
				Close(TRI_FALSE);
				return;
			}
		}
		if (!ReadTiger(true))
		{
			// ReadTiger rejected the DIME response; fail closed like the live
			// path instead of resuming the source.
			Close(TRI_FALSE);
			return; // Closed (and deleted) the transfer
		}
		// CDownloadTransfer::Close will resume the closed connection
		if (m_pSource)
			m_pSource->m_bCloseConn = TRUE;
		Close(TRI_TRUE);
	}
	else if (m_nState == dtsMetadata)
	{
		// Close-delimited (unknown Content-Length): body is complete on drop.
		// Known Content-Length with Connection: close: finish when the full
		// body is already buffered (including Content-Length: 0); fail only
		// when the buffered length is short.
		if (m_nContentLength != SIZE_UNKNOWN)
		{
			if (GetInputLength() < m_nContentLength)
			{
				theApp.Message(MSG_ERROR, L"Incomplete peer metadata from %s",
				               (LPCTSTR)m_sAddress);
				Close(TRI_FALSE);
				return;
			}
			m_nLength = m_nContentLength;
		}
		else
		{
			m_nLength = m_nContentLength = GetInputLength();
			if (m_nLength == 0 || m_nLength > XML_PEER_PARSE_CHARS_MAX)
			{
				Close(TRI_FALSE);
				return;
			}
		}
		if (!ReadMetadata(true))
			return; // ReadMetadata closed (and deleted) the transfer
		if (m_pSource)
			m_pSource->m_bCloseConn = TRUE;
		Close(TRI_TRUE);
	}
	else if ( m_bBusyFault || m_bQueueFlag )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
		Close( TRI_TRUE, m_nRetryAfter );
	}
	else if ( m_nState == dtsDownloading && m_nContentLength == SIZE_UNKNOWN &&
		m_pDownload->m_nSize == SIZE_UNKNOWN )
	{
		// Set file size as is
		m_pDownload->SetSize( m_nDownloaded );
		m_pDownload->MakeComplete();
		Close( TRI_TRUE );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DROPPED, (LPCTSTR)m_sAddress );
		Close( m_nState >= dtsDownloading ? TRI_TRUE : TRI_UNKNOWN );
	}
}
