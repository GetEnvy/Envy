//
// EnvyURL.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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
#include "EnvyURL.h"
#include "Transfer.h"
#include "QuerySearch.h"
#include "DiscoveryServices.h"
#include "Network.h"
#include "BTInfo.h"
#include "Skin.h"
#include "Registry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define REG_NUMBER(x) REG_DWORD,((LPBYTE)&(x)),(sizeof(DWORD))
#define REG_STRING(x) REG_SZ,((LPBYTE)(LPCTSTR)(x)),((DWORD)(sizeof(TCHAR)*(_tcslen((LPCTSTR)(x))+1)))

//////////////////////////////////////////////////////////////////////
// CEnvyURL construction

CEnvyURL::CEnvyURL(LPCTSTR pszURL)
	: m_nProtocol		( PROTOCOL_NULL )
	, m_nAction			( uriNull )
	, m_pTorrent		( )
	, m_pAddress		( )
	, m_nPort			( 0 )
	, m_pServerAddress	( )
	, m_nServerPort		( 0 )
{
	if ( pszURL != NULL ) Parse( pszURL );
}

CEnvyURL::CEnvyURL(CBTInfo* pTorrent)
	: CEnvyFile( static_cast< const CEnvyFile& >( *pTorrent ) )
	, m_nProtocol		( PROTOCOL_NULL )
	, m_nAction			( uriDownload )
	, m_pTorrent		( pTorrent )
	, m_pAddress		( )
	, m_nPort			( 0 )
	, m_pServerAddress	( )
	, m_nServerPort		( 0 )
{
}

CEnvyURL::CEnvyURL(const CEnvyURL& pURL)
	: CEnvyFile( static_cast< const CEnvyFile& >( pURL ) )
	, m_nProtocol		( pURL.m_nProtocol )
	, m_nAction			( pURL.m_nAction )
	, m_pTorrent		( pURL.m_pTorrent )
	, m_sAddress		( pURL.m_sAddress )
	, m_pAddress		( pURL.m_pAddress )
	, m_nPort			( pURL.m_nPort )
	, m_pServerAddress	( pURL.m_pServerAddress )
	, m_nServerPort		( pURL.m_nServerPort )
	, m_oBTC			( pURL.m_oBTC )
	, m_sLogin			( pURL.m_sLogin )
	, m_sPassword		( pURL.m_sPassword )
{
}

CEnvyURL::~CEnvyURL()
{
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL clear

void CEnvyURL::Clear()
{
	// CEnvyFile
	m_nSize	= SIZE_UNKNOWN;
	m_sName.Empty();
	m_oSHA1.clear();
	m_oTiger.clear();
	m_oMD5.clear();
	m_oED2K.clear();
	m_oBTH.clear();
	m_sURL.Empty();
	m_sPath.Empty();

	// CEnvyURL
	m_nProtocol				= PROTOCOL_NULL;
	m_nAction				= uriNull;
	m_pTorrent.Free();
	m_sAddress.Empty();
	m_pAddress.s_addr		= 0;
	m_nPort					= 0;	// protocolPorts[ PROTOCOL_G2 ]
	m_pServerAddress.s_addr = 0;
	m_nServerPort			= 0;
	m_sLogin.Empty();
	m_sPassword.Empty();
	m_oBTC.clear();
}

//////////////////////////////////////////////////////////////////////
// Parse URL list

BOOL CEnvyURL::Parse(const CString& sText, CList< CString >& pURLs, BOOL bResolve)
{
	pURLs.RemoveAll();

	// Split text to reverse string list
	CList< CString > oReverse;

	int curPos = 0;
	CString strPart;
	while ( ( strPart = sText.Tokenize( L"\n", curPos ) ).GetLength() )
	{
		oReverse.AddHead( strPart.Trim( L"\r\n\t >< " ) );		// Second space is #160
	}

	CString strBuf;
	for ( POSITION pos = oReverse.GetHeadPosition() ; pos ; )
	{
		CString strLine( oReverse.GetNext( pos ) );
		if ( strLine.IsEmpty() )
		{
			// Empty strings breaks URL
			strBuf.Empty();
		}
		else
		{
			// Append new line to current URL and parse
			strBuf.Insert( 0, strLine );
			if ( Parse( strBuf, bResolve ) )
			{
				// OK, new URL found
				pURLs.AddTail( strBuf );
				strBuf.Empty();
			}
		}
	}

	return ! pURLs.IsEmpty();
}

//////////////////////////////////////////////////////////////////////
// Parse single URL

BOOL CEnvyURL::Parse(LPCTSTR pszURL, BOOL bResolve)
{
	// Parse "good" URL, and retry "bad" URL
	return ( ParseRoot( pszURL, bResolve ) || ParseRoot( URLDecode( pszURL ), bResolve ) );
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL root parser

BOOL CEnvyURL::ParseRoot(LPCTSTR pszURL, BOOL bResolve)
{
	CString strRoot( pszURL );
	const int nRoot = strRoot.Find( L":" ) + 1;
	if ( nRoot < 3 ) return FALSE;
	strRoot = strRoot.Left( nRoot ).MakeLower();

	//CString strPart = SkipSlashes( pszURL, nRoot );
	//if ( strPart.GetLength() < 4 ) return FALSE;

	SwitchMap( Root )
	{
		Root[ L"http:" ] 		= 'h';
		Root[ L"https:" ]		= 'h';
		Root[ L"ftp:" ]			= 'f';
		Root[ L"btc:" ]			= 'b';
		Root[ L"magnet:" ]		= 'm';
		Root[ L"ed2k:" ]		= 'e';
		Root[ L"ed2kftp:" ]		= 'k';
		Root[ L"envy:" ]		= 'g';
		Root[ L"peer:" ]		= 'g';
		Root[ L"peerproject:" ]	= 'g';
		Root[ L"shareaza:" ]	= 'g';
		Root[ L"gnutella:" ]	= 'g';
		Root[ L"gnet:" ] 		= 'g';
		Root[ L"g2:" ]			= '2';
		Root[ L"gwc:" ]			= 'u';
		Root[ L"uhc:" ]			= 'u';
		Root[ L"ukhl:" ]		= 'u';
		Root[ L"gnutella1:" ]	= 'u';
		Root[ L"gnutella2:" ]	= 'u';
		Root[ L"mp2p:" ]		= 'p';
		Root[ L"adc:" ] 		= 'd';
		Root[ L"dchub:" ]		= 'd';
		Root[ L"dcfile:" ]		= 'c';
		Root[ L"foxy:" ]		= 'x';
		Root[ L"irc:" ]			= 'i';
	}

	switch ( Root[ strRoot ] )
	{
	case 'h':	// http:// https://
		return ParseHTTP( pszURL, bResolve );
	case 'f':	// ftp://
		return ParseFTP( pszURL, bResolve );
	case 'm':	// magnet:?
		if ( _tcsnicmp( pszURL, L"magnet:?", 8 ) == 0 )
		{
			pszURL += 8;
			return ParseMagnet( pszURL );
		}
		return ParseEnvy( SkipSlashes( pszURL, nRoot ) );
	case 'g':	// envy: peer: peerproject: shareaza: gnutella: gnet:
		return ParseEnvy( SkipSlashes( pszURL, nRoot ) );
	case '2':	// g2:
		pszURL = SkipSlashes( pszURL, nRoot );
		if ( _tcsnicmp( pszURL, L"//", 2 ) == 0 )	// Check malformed assumption
			pszURL = SkipSlashes( pszURL, 2 );
		if ( _tcsnicmp( pszURL, L"browse:", 7 ) == 0 ||
			 _tcsnicmp( pszURL, L"host:", 5 ) == 0 ||
			 _tcsnicmp( pszURL, L"chat:", 5 ) == 0 ||
			 _tcsnicmp( pszURL, L"search:", 7 ) == 0 ||
			 ! ParseMagnet( pszURL ) )
			return ParseEnvy( pszURL );
		return true;	// Magnet succeeded (?)
	case 'u':	// gwc: uhc: ukhl: gnutella1:host: gnutella2:host:
		return ParseEnvy( pszURL );
	case 'k':	// ed2kftp://
		return ParseED2KFTP( pszURL, bResolve );
	case 'e':	// ed2k:
		return ParseDonkey( SkipSlashes( pszURL, nRoot ) );
	case 'b':	// btc://
		return ParseBTC( pszURL, bResolve );
	case 'p':	// mp2p:
		return ParsePiolet( SkipSlashes( pszURL, nRoot ) );
	case 'd':	// dchub://1.2.3.4:411	(adc:// ?)
		return ParseDCHub( pszURL, bResolve );
	case 'c':	// dcfile:// (Deprecated?)
		return ParseDCFile( pszURL, FALSE );
	case 'x':	// foxy://download?
		pszURL = SkipSlashes( pszURL, nRoot );
		if ( _tcsnicmp( pszURL, L"download?", 9 ) == 0 )	// Original
			return ParseMagnet( pszURL + 9 );
		if ( _tcsnicmp( pszURL, L"download/?", 10 ) == 0 )	// "Fixed" by IE
			return ParseMagnet( pszURL + 10 );
		//return FALSE;		// Other?
	case 'i':	// irc://irc.server:port/channel?key
	//	SkipSlashes( pszURL, nRoot );
		return FALSE;	// ToDo: IRC link support
	default:
		if ( IPStringToDWORD( CString( pszURL ).Trim( L" \t\r\n/" ), FALSE ) )
			return ParseEnvy( pszURL );
		//return FALSE;		// Unknown? See http://en.wikipedia.org/wiki/URI_scheme
	}

// Obsolete method, for reference and deletion
//	if ( ! _tcsnicmp( pszURL, L"http://", 7 ) ||
//		 ! _tcsnicmp( pszURL, L"https://", 8 ) )
//		return ParseHTTP( pszURL, bResolve );
//	if ( ! _tcsnicmp( pszURL, L"ftp://", 6 ) )
//		return ParseFTP( pszURL, bResolve );
//	if ( ! _tcsnicmp( pszURL, L"ed2kftp://", 10 ) )
//		return ParseED2KFTP( pszURL, bResolve );
//	if ( ! _tcsnicmp( pszURL, L"btc://", 6 ) )
//		return ParseBTC( pszURL, bResolve );
//
//	if ( _tcsnicmp( pszURL, L"magnet:?", 8 ) == 0 )
//	{
//		pszURL += 8;
//		return ParseMagnet( pszURL );
//	}
//	else if ( _tcsnicmp( pszURL, L"magnet:", 7 ) == 0 )
//	{
//		pszURL += 7;
//		return ParseEnvy( pszURL );
//	}
//	else if ( _tcsnicmp( pszURL, L"ed2k:", 5 ) == 0 )
//	{
//		return ParseDonkey( SkipSlashes( pszURL, 5 ) );
//	}
//	else if ( _tcsnicmp( pszURL, L"shareaza:", 9 ) == 0 ||
//			  _tcsnicmp( pszURL, L"gnutella:", 9 ) == 0 )
//	{
//		return ParseEnvy( SkipSlashes( pszURL, 9 ) );
//	}
//	else if ( _tcsnicmp( pszURL, L"envy:", 5 ) == 0 )	// APP_LENGTH LETTERCOUNT
//	{
//		return ParseEnvy( SkipSlashes( pszURL, 5 ) );
//	}
//	else if ( _tcsnicmp( pszURL, L"gwc:", 4 ) == 0 )
//	{
//		CString strTemp;
//		strTemp.Format( L"envy:%s", pszURL );
//		pszURL = strTemp;
//		pszURL = SkipSlashes( pszURL, 5 );			// APP_LENGTH LETTERCOUNT
//		return ParseEnvy( pszURL );
//	}
//	else if ( _tcsnicmp( pszURL, L"uhc:", 4 ) == 0 ||
//			  _tcsnicmp( pszURL, L"ukhl:", 5 ) == 0 ||
//			  _tcsnicmp( pszURL, L"gnutella1:", 10 ) == 0 ||
//			  _tcsnicmp( pszURL, L"gnutella2:", 10 ) == 0 )
//	{
//		return ParseEnvy( pszURL );
//	}
//	else if ( _tcsnicmp( pszURL, L"g2:", 3 ) == 0 )
//	{
//		pszURL = SkipSlashes( pszURL, 3 );
//		if ( _tcsnicmp( pszURL, L"browse:", 7 ) == 0 || ! ParseMagnet( pszURL ) )
//			return ParseEnvy( pszURL );
//		return TRUE;
//	}
//	else if ( _tcsnicmp( pszURL, L"gnet:", 5 ) == 0 )
//	{
//		pszURL = SkipSlashes( pszURL, 5 );
//		return ParseEnvy( pszURL );
//	}
//	else if ( _tcsnicmp( pszURL, L"mp2p:", 5 ) == 0 )
//	{
//		pszURL = SkipSlashes( pszURL, 5 );
//		return ParsePiolet( pszURL );
//	}
//	else if ( _tcsnicmp( pszURL, L"foxy:", 5 ) == 0 )			// Foxy
//	{
//		pszURL += 5;
//		if ( ! _tcsnicmp( pszURL, L"//download?", 11 ) )		// Original
//		{
//			pszURL += 11;
//			return ParseMagnet( pszURL );
//		}
//		else if ( ! _tcsnicmp( pszURL, L"//download/?", 12 ) )	// "Fixed" by IE
//		{
//			pszURL += 12;
//			return ParseMagnet( pszURL );
//		}
//	}
//	else if ( ! _tcsnicmp( pszURL, L"dchub://", 8 ) )			// dchub://1.2.3.4:411
//	{
//		pszURL = SkipSlashes( pszURL, 8 );
//	//	m_nProtocol	= PROTOCOL_DC;
//	//	m_nPort = protocolPorts[ PROTOCOL_DC ];
//		return ParseEnvyHost( pszURL, FALSE, PROTOCOL_DC );
//	}
//	else if ( ! _tcsnicmp( pszURL, L"dcfile://", 8 ) )
//	{
//		return ParseDC( pszURL, FALSE );
//	}

	Clear();

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL HTTP

BOOL CEnvyURL::ParseHTTP(LPCTSTR pszURL, BOOL bResolve)
{
	CString strURL = pszURL;
	if ( ! _tcsnicmp( pszURL, L"http://", 7 ) )
		strURL = pszURL + 7;
	else if ( ! _tcsncmp( pszURL, L"https://", 8 ) )		// HTTPS unsupported, but try such links
		strURL = pszURL + 8;
	//else
	//	return FALSE;

	Clear();

	const int nSlash = strURL.Find( L'/' );
	if ( nSlash >= 0 )
	{
		m_sAddress	= strURL.Left( nSlash );
		m_sPath 	= strURL.Mid( nSlash );
	}
	else
	{
		m_sAddress	= strURL;
		m_sPath 	= L"/";
	}

	// Detect mistaken IP:port
	if ( m_sPath == L"/" && m_sAddress.Find( L':' ) > 8 && IPStringToDWORD( m_sAddress, FALSE ) )
		return ParseEnvy( m_sAddress );

	const int nAt = m_sAddress.Find( L'@' );
	if ( nAt >= 0 ) m_sAddress = m_sAddress.Mid( nAt + 1 );

	if ( m_sAddress.IsEmpty() ) return FALSE;

	if ( _tcsnicmp( m_sPath, L"/uri-res/N2R?", 13 ) == 0 )
	{
		strURL = m_sPath.Mid( 13 );
		if ( m_oSHA1.fromUrn( strURL ) ) ;
		else if ( m_oTiger.fromUrn( strURL ) ) ;
		else if ( m_oED2K.fromUrn( strURL ) ) ;
		else if ( m_oBTH.fromUrn( strURL ) ) ;
		else if ( m_oBTH.fromUrn< Hashes::base16Encoding >( strURL ) ) ;
		else if ( m_oMD5.fromUrn( strURL ) ) ;
		else return FALSE;

		m_nAction = uriSource;
	}
	else
	{
		m_nAction = uriDownload;

		CString strName( URLDecode( m_sPath.Mid( m_sPath.ReverseFind( L'/' ) + 1 ).SpanExcluding( L"?" ) ) );
		if ( ! strName.IsEmpty() )
			m_sName = strName;
	}

	SOCKADDR_IN saHost;

	const BOOL bResult = Network.Resolve( m_sAddress, INTERNET_DEFAULT_HTTP_PORT, &saHost, bResolve );

	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );

	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_HTTP;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL FTP

BOOL CEnvyURL::ParseFTP(LPCTSTR pszURL, BOOL bResolve)
{
	// URI format:	ftp://[user[:password]@]host[:port][/path]

	//if ( _tcsncmp( pszURL, L"ftp://", 6 ) != 0 ) return FALSE;

	Clear();

	CString strURL ( pszURL + 6 );

	const int nSlash = strURL.Find( L'/' );
	if ( nSlash >= 0 )
	{
		m_sAddress	= strURL.Left( nSlash );
		m_sPath		= strURL.Mid( nSlash );
	}
	else
	{
		m_sAddress = strURL;
		m_sPath = L"/";
	}

	const int nAt = m_sAddress.Find( L'@' );
	if ( nAt >= 0 )
	{
		m_sLogin = m_sAddress.Left( nAt );
		m_sAddress = m_sAddress.Mid( nAt + 1 );

		const int nColon = m_sLogin.Find( L':' );
		if ( nColon >= 0 )
		{
			m_sPassword = m_sLogin.Mid( nColon + 1 );
			m_sLogin = m_sLogin.Left( nColon );
		}
	}
	else
	{
		m_sLogin = L"anonymous";
		m_sPassword = L"guest@getenvy.com";
	}

	if ( m_sAddress.IsEmpty() || m_sLogin.IsEmpty() )
		return FALSE;

	// Add fix set name
	const int nPos = m_sPath.ReverseFind( L'/' );
	if ( m_sName.IsEmpty() && nPos >= 0 )
	{
		const CString sName( URLDecode( m_sPath.Mid( nPos + 1 ).SpanExcluding( L"?" ) ) );
		if ( ! sName.IsEmpty() )
			m_sName = sName;
	}

	SOCKADDR_IN saHost;

	BOOL bResult = Network.Resolve( m_sAddress, INTERNET_DEFAULT_FTP_PORT, &saHost, bResolve );

	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );

	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_FTP;
	m_nAction	= uriDownload;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL ED2KFTP

BOOL CEnvyURL::ParseED2KFTP(LPCTSTR pszURL, BOOL bResolve)
{
	//if ( _tcsnicmp( pszURL, L"ed2kftp://", 10 ) != 0 ) return FALSE;

	Clear();

	CString strURL = pszURL + 10;
	BOOL bPush = FALSE;

	int nSlash = strURL.Find( L'/' );
	if ( nSlash < 7 ) return FALSE;

	m_sAddress	= strURL.Left( nSlash );
	strURL		= strURL.Mid( nSlash + 1 );

	nSlash = strURL.Find( L'/' );
	if ( nSlash != 32 ) return FALSE;

	CString strHash	= strURL.Left( 32 );
	strURL			= strURL.Mid( 33 );

	if ( ! m_oED2K.fromString( strHash ) ) return FALSE;

	QWORD nSize = 0;
	if ( _stscanf( strURL, L"%I64u", &nSize ) != 1 || ! nSize ) return FALSE;
	m_nSize = nSize;

	nSlash = m_sAddress.Find( L'@' );

	if ( nSlash > 0 )
	{
		strHash = m_sAddress.Left( nSlash );
		m_sAddress = m_sAddress.Mid( nSlash + 1 );
		if ( _stscanf( strHash, L"%lu", &m_pAddress.S_un.S_addr ) != 1 ) return FALSE;
		bPush = TRUE;
	}

	SOCKADDR_IN saHost;
	BOOL bResult = Network.Resolve( m_sAddress, protocolPorts[ PROTOCOL_ED2K ], &saHost, bResolve );

	if ( bPush )
	{
		m_pServerAddress	= saHost.sin_addr;
		m_nServerPort		= htons( saHost.sin_port );
		m_nPort				= 0;
	}
	else
	{
		m_pAddress	= saHost.sin_addr;
		m_nPort		= htons( saHost.sin_port );
	}

	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_ED2K;
	m_nAction	= uriDownload;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL DC

BOOL CEnvyURL::ParseDCHub(LPCTSTR pszURL, BOOL bResolve)
{
	Clear();

	// dchub://[login@]address:port/[filepath]	-Can be regular path or "files.xml.bz2" or "TTH:tiger_hash/size/"

	if ( pszURL[0] == L'a' )
		pszURL = SkipSlashes( pszURL, 6 );	// "adc://"
	else
		pszURL = SkipSlashes( pszURL, 8 );	// "dchub://"

	CString strURL = pszURL;

	int nSlash = strURL.Find( L'/' );

	// Short version (hub address only)
	if ( nSlash == -1 || nSlash == strURL.GetLength() - 1 )		// || strURL.IsEmpty()
	{
		m_sAddress.Empty();
	//	m_nProtocol = PROTOCOL_DC;
	//	m_nPort = protocolPorts[ PROTOCOL_DC ];
		return ParseEnvyHost( pszURL, FALSE, PROTOCOL_DC );
	}

	// Full version (file URL)

	m_sAddress	= strURL.Left( nSlash );
	strURL		= strURL.Mid( nSlash + 1 ).TrimLeft( L"/" );

	int nAt = m_sAddress.Find( L'@' );
	if ( nAt > 0 )
	{
		m_sLogin = URLDecode( m_sAddress.Left( nAt ) );
		m_sAddress = m_sAddress.Mid( nAt + 1 );
	}

	int nHash = strURL.Find( L"TTH:" );
	if ( nHash != -1 )
	{
		CString strHash = strURL.Mid( nHash + 4, 39 );
		strURL = strURL.Mid( nHash + 4 + 39 );

		if ( ! m_oTiger.fromString( strHash ) )
			return FALSE;

		QWORD nSize = 0;
		if ( _stscanf( strURL, L"/%I64u", &nSize ) == 1 && nSize )
			m_nSize = nSize;
	}
	else
	{
		m_sName = URLDecode( strURL );
	}

	SOCKADDR_IN saHost = {};
	BOOL bResult = Network.Resolve( m_sAddress, protocolPorts[ PROTOCOL_DC ], &saHost, bResolve );

	m_pServerAddress	= saHost.sin_addr;
	m_nServerPort		= htons( saHost.sin_port );
	m_sURL.Format( L"dchub://%s", pszURL );
	m_nProtocol			= PROTOCOL_DC;
	m_nAction			= uriDownload;

	return bResult;
}

BOOL CEnvyURL::ParseDCFile(LPCTSTR pszURL, BOOL bResolve)
{
	Clear();

	// dcfile://address:port/login/TTH:tiger_hash/size/	(Deprecated?)

	//if ( _tcsnicmp( pszURL, L"dcfile://", 6 ) != 0 ) return FALSE;

	CString strURL = pszURL + 9;		// "dcfile://"

	int nSlash = strURL.Find( L'/' );
	if ( nSlash < 7 ) return FALSE;

	m_sAddress	= strURL.Left( nSlash );
	strURL		= strURL.Mid( nSlash + 1 );

	nSlash = strURL.Find( L'/' );
	if ( nSlash < 3 ) return FALSE;

	m_sLogin	= URLDecode( strURL.Left( nSlash ) );
	strURL		= strURL.Mid( nSlash + 1 );

	nSlash = strURL.Find( L'/' );
	if ( nSlash != 4 + 39 ) return FALSE;

	CString strHash	= strURL.Left( nSlash );
	strURL			= strURL.Mid( nSlash + 1 );
	strURL.TrimRight( L"//" );

	if ( strHash.Left( 4 ) != L"TTH:" ) return FALSE;

	if ( ! m_oTiger.fromString( strHash.Mid( 4 ) ) ) return FALSE;

	QWORD nSize = 0;
	if ( _stscanf( strURL, L"%I64u", &nSize ) != 1 || ! nSize ) return FALSE;
	m_nSize = nSize;

	SOCKADDR_IN saHost = {};
	BOOL bResult = Network.Resolve( m_sAddress, protocolPorts[ PROTOCOL_DC ], &saHost, bResolve );

	m_pServerAddress	= saHost.sin_addr;
	m_nServerPort		= htons( saHost.sin_port );
	m_sURL				= pszURL;
	m_nProtocol			= PROTOCOL_DC;
	m_nAction			= uriDownload;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL BTC

BOOL CEnvyURL::ParseBTC(LPCTSTR pszURL, BOOL bResolve)
{
	//if ( _tcsnicmp( pszURL, L"btc://", 6 ) != 0 ) return FALSE;

	Clear();

	CString strURL = pszURL + 6;

	int nSlash = strURL.Find( L'/' );
	if ( nSlash < 7 ) return FALSE;

	m_sAddress	= strURL.Left( nSlash );
	strURL		= strURL.Mid( nSlash + 1 );

	nSlash = strURL.Find( L'/' );
	m_oBTC.clear();

	if ( nSlash < 0 ) return FALSE;

	if ( nSlash == 32 )
	{
		CString strGUID	= strURL.Left( 32 );
		m_oBTC.fromString( strGUID );
	}

	strURL = strURL.Mid( nSlash + 1 );

	if ( ! m_oBTH.fromString( strURL ) ) return FALSE;

	SOCKADDR_IN saHost;
	BOOL bResult = Network.Resolve( m_sAddress, protocolPorts[ PROTOCOL_BT ], &saHost, bResolve );

	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );

	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_BT;
	m_nAction	= uriDownload;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse "magnet:" URLs

BOOL CEnvyURL::ParseMagnet(LPCTSTR pszURL)
{
	Clear();

	while ( *pszURL == L'?' )
		++pszURL;	// "magnet:?"

	CString strURL( pszURL );
	CAutoPtr< CBTInfo > pTorrent( new CBTInfo() );		// Was CBTInfo* pTorrent = new CBTInfo();

	// http://en.wikipedia.org/wiki/Magnet_URI_scheme

	for ( strURL += L'&' ; ! strURL.IsEmpty() ; )
	{
		const CString strPart = strURL.SpanExcluding( L"&" );
		strURL = strURL.Mid( strPart.GetLength() + 1 );

		const int nEquals = strPart.Find( L'=' );
		if ( nEquals < 0 ) continue;

		CString strKey   = URLDecode( strPart.Left( nEquals ) );
		CString strValue = URLDecode( strPart.Mid( nEquals + 1 ) );

		SafeString( strKey );
		SafeString( strValue );

		if ( strKey.IsEmpty() || strValue.IsEmpty() ) continue;

		// Trim key number, "xt.1" "tr.1"	(ToDo: Support xt.# multiple files)
		if ( strKey.GetLength() > 3 &&
			 strKey[ 2 ] == L'.' &&
			 _istdigit( strKey[ 3 ] ) )
			strKey = strKey.Left( 2 );

		strKey.MakeLower();

		if ( strKey == L"xt" ||		// "Exact Topic"  (URN containing file hash)
			 strKey == L"xs" ||		// "Exact Source" (p2p link)
			 strKey == L"as" ||		// "Acceptable Source" (web link)
			 strKey == L"ws" ||		// "Web Seed" (BitTorrent HTTP source, BEP 19)
			 strKey == L"mt" ||		//
			 strKey == L"tr" )		// "Tracker address" (BitTorrent tracker URL)
		{
			if ( StartsWith( strValue, _P( L"urn:" ) ) ||
				 StartsWith( strValue, _P( L"sha1:" ) ) ||
				 StartsWith( strValue, _P( L"bitprint:" ) ) ||
				 StartsWith( strValue, _P( L"btih:" ) ) ||
				 StartsWith( strValue, _P( L"ed2k:" ) ) ||
				 StartsWith( strValue, _P( L"md5:" ) ) ||
				 StartsWith( strValue, _P( L"tree:tiger" ) ) )		// tree:tiger: tree:tiger/: tree:tiger/1024:
			{
				if ( ! m_oSHA1 ) m_oSHA1.fromUrn( strValue );
				if ( ! m_oTiger ) m_oTiger.fromUrn( strValue );
				if ( ! m_oED2K ) m_oED2K.fromUrn( strValue );
				if ( ! m_oMD5 ) m_oMD5.fromUrn( strValue );
				if ( ! m_oBTH ) m_oBTH.fromUrn( strValue );
				if ( ! m_oBTH ) m_oBTH.fromUrn< Hashes::base16Encoding >( strValue );
			}
			else if ( StartsWith( strValue, _P( L"http://" ) ) ||
					  StartsWith( strValue, _P( L"https://" ) ) ||
					  StartsWith( strValue, _P( L"http%3A//" ) ) ||
					  StartsWith( strValue, _P( L"udp://" ) ) ||
					  StartsWith( strValue, _P( L"udp%3A//" ) ) ||
					  StartsWith( strValue, _P( L"ftp://" ) ) ||
					  StartsWith( strValue, _P( L"ftp%3A//" ) ) ||
					  StartsWith( strValue, _P( L"dchub://" ) ) ||
					  StartsWith( strValue, _P( L"dchub%3A//" ) ) )
			{
				strValue.Replace( L" ", L"%20" );
				strValue.Replace( L"%3A//", L"://" );

				if ( strKey == L"xt" )		// Compatibility hack: "&xt.{any}="
				{
					CString strURL = L"@" + strValue;

					if ( ! m_sURL.IsEmpty() )
						m_sURL = strURL + L", " + m_sURL;
					else
						m_sURL = strURL;
				}
				else if ( strKey == L"tr" )	// Compatibility hack: "&tr{any}="
				{
					pTorrent->SetTracker( strValue );
				}
				else
				{
					if ( ! m_sURL.IsEmpty() )
						m_sURL += L", ";
					m_sURL += strValue;
				}
			}
		}
		else if ( strKey == L"dn" )		// "Display Name" (filename)
		{
			m_sName = strValue;
		}
		else if ( strKey == L"kt" )		// "Keyword Topic" (key words for search)
		{
			m_sName = strValue;
			m_oSHA1.clear();
			m_oTiger.clear();
			m_oED2K.clear();
			m_oMD5.clear();
			m_oBTH.clear();
		}
		else if ( strKey == L"xl" ||	// "Exact Length" (size in bytes)
				  strKey == L"sz" ||	// "Size" Non-standard (very old Shareaza)
				  strKey == L"fs" ) 	// "Filesize" Non-standard (Foxy)
		{
			QWORD nSize = 0;
			if ( m_nSize == SIZE_UNKNOWN && _stscanf( strValue, L"%I64u", &nSize ) == 1 && nSize )
				m_nSize = nSize;
		}
		else if ( strKey == L"bn" )		// "Bittorrent Node" (node address for DHT bootstrapping)
		{
			// Nodes are common
			pTorrent->SetNode( strValue );
		}
#ifdef _DEBUG
		else	// Unknown key
		{
			theApp.Message( MSG_INFO, strKey + L": Unknown Magnet Key" );
		}
#endif // Debug
	}

	if ( m_sName.GetLength() > 3 )
		m_sName = URLDecode( (LPCTSTR)m_sName );

	if ( m_oBTH && ! m_pTorrent )
	{
		pTorrent->SetTrackerMode( pTorrent->GetTrackerCount() == 1 ?
			CBTInfo::tSingle : CBTInfo::tMultiFinding );

		m_pTorrent = pTorrent;
		m_pTorrent->m_nSize		= m_nSize;
		m_pTorrent->m_sName		= m_sName;
		m_pTorrent->m_oSHA1		= m_oSHA1;
		m_pTorrent->m_oTiger	= m_oTiger;
		m_pTorrent->m_oED2K		= m_oED2K;
		m_pTorrent->m_oMD5		= m_oMD5;
		m_pTorrent->m_oBTH		= m_oBTH;
	}

	//delete pTorrent;	// By CAutoPtr

	if ( HasHash() || ! m_sURL.IsEmpty() )
	{
		m_nAction = uriDownload;
		return TRUE;
	}

	if ( ! m_sName.IsEmpty() )
	{
		m_nAction = uriSearch;
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse "envy:" URLs

BOOL CEnvyURL::ParseEnvy(LPCTSTR pszURL)
{
	Clear();

	int nIP[4];
	if ( _stscanf( pszURL, L"%i.%i.%i.%i", &nIP[0], &nIP[1], &nIP[2], &nIP[3] ) == 4 )
		return ParseEnvyHost( pszURL, FALSE );

	if ( ! _tcsnicmp( pszURL, L"host:", 5 ) || ! _tcsnicmp( pszURL, L"node:", 5 ) )
		return ParseEnvyHost( pszURL + 5, FALSE );
	if ( ! _tcsnicmp( pszURL, L"hub:", 4 ) )
		return ParseEnvyHost( pszURL + 4, FALSE );
	if ( ! _tcsnicmp( pszURL, L"server:", 7 ) )
		return ParseEnvyHost( pszURL + 7, FALSE );
	if ( ! _tcsnicmp( pszURL, L"browse:", 7 ) )
		return ParseEnvyHost( pszURL + 7, TRUE );
	if ( ! _tcsnicmp( pszURL, L"chat:", 5 ) )
		return ParseEnvyHost( pszURL + 5, TRUE );
	if ( ! _tcsnicmp( pszURL, L"http://", 7 ) )
		return ParseEnvyHost( pszURL + 7, TRUE );
	if ( ! _tcsnicmp( pszURL, L"gwc:", 4 ) )
		return ParseDiscovery( pszURL + 4, CDiscoveryService::dsWebCache );
	if ( ! _tcsnicmp( pszURL, L"meturl:", 7 ) )
		return ParseDiscovery( pszURL + 7, CDiscoveryService::dsServerList );
	if ( ! _tcsnicmp( pszURL, L"url:", 4 ) )
		return Parse( pszURL + 4 );
	if ( ! _tcsnicmp( pszURL, L"uhc:", 4 ) ||
		 ! _tcsnicmp( pszURL, L"ukhl:", 5 ) ||
		 ! _tcsnicmp( pszURL, L"gnutella1:host:", 15 ) ||
		 ! _tcsnicmp( pszURL, L"gnutella2:host:", 15 ) )
		return ParseDiscovery( pszURL, CDiscoveryService::dsGnutella );
	if ( _tcsnicmp( pszURL, L"btnode:", 7 ) == 0 )
	{
		if ( ParseEnvyHost( SkipSlashes( pszURL, 7 ), FALSE, PROTOCOL_BT ) )
		{
			m_sAddress.Format( L"%s:%u", m_sName, m_nPort );
		//	m_nPort = protocolPorts[ PROTOCOL_BT ];
		//	m_nProtocol = PROTOCOL_BT;
		//	m_nAction = uriHost;
			return TRUE;
		}
		return FALSE;
	}
	if ( _tcsnicmp( pszURL, L"search:", 7 ) == 0 )
	{
		// ToDo: Formalize search parameters (Handle size/schema/etc?)
		m_sName = URLDecode( pszURL + 7 );
		m_nAction = uriSearch;
		m_oSHA1.clear();
		m_oTiger.clear();
		m_oED2K.clear();
		m_oMD5.clear();
		m_oBTH.clear();
		return TRUE;
	}
	if ( _tcsnicmp( pszURL, L"command:", 8 ) == 0 )
	{
		m_sName = pszURL + 8;
		if ( m_sName.IsEmpty() )
			return FALSE;
		m_nAction = uriCommand;
		return TRUE;
	}

	return ParseEnvyFile( pszURL );
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse Envy host URL

BOOL CEnvyURL::ParseEnvyHost(LPCTSTR pszURL, BOOL bBrowse, PROTOCOLID nProtocol /*PROTOCOL_G2*/)
{
	m_nAction = bBrowse ? uriBrowse : uriHost;

	m_sName = pszURL;
	m_sName = m_sName.SpanExcluding( L"/\\" );
	m_nPort = protocolPorts[ nProtocol ];
	m_nProtocol = nProtocol;

	int nPos = m_sName.Find( L':' );
	if ( nPos >= 0 )
	{
		_stscanf( m_sName.Mid( nPos + 1 ), L"%hu", &m_nPort );
		m_sName = m_sName.Left( nPos );
	}

	int nAt = m_sName.Find( L'@' );
	if ( nAt >= 0 )
	{
		if ( nAt > 0 )
			m_sLogin = URLDecode( m_sName.Left( nAt ) );
		m_sName = m_sName.Mid( nAt + 1 );
	}

	m_sName.Trim();

	return ! m_sName.IsEmpty();
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse Envy file URL

BOOL CEnvyURL::ParseEnvyFile(LPCTSTR pszURL)
{
	CString strURL( pszURL );

	for ( strURL += L'/' ; ! strURL.IsEmpty() ; )
	{
		CString strPart = strURL.SpanExcluding( L"/|" );
		strURL = strURL.Mid( strPart.GetLength() + 1 );

		strPart.Trim();
		if ( strPart.IsEmpty() ) continue;

		if ( StartsWith( strPart, _P( L"urn:" ) ) ||
			 StartsWith( strPart, _P( L"sha1:" ) ) ||
			 StartsWith( strPart, _P( L"bitprint:" ) ) ||
			 StartsWith( strPart, _P( L"btih:" ) ) ||
			 StartsWith( strPart, _P( L"ed2k:" ) ) ||
			 StartsWith( strPart, _P( L"md5:" ) ) ||
			 StartsWith( strPart, _P( L"tree:tiger" ) ) )		// tree:tiger: tree:tiger/: tree:tiger/1024:
		{
			if ( ! m_oSHA1 ) m_oSHA1.fromUrn( strPart );
			if ( ! m_oTiger ) m_oTiger.fromUrn( strPart );
			if ( ! m_oMD5 ) m_oMD5.fromUrn( strPart );
			if ( ! m_oED2K ) m_oED2K.fromUrn( strPart );
			if ( ! m_oBTH ) m_oBTH.fromUrn( strPart );
			if ( ! m_oBTH ) m_oBTH.fromUrn< Hashes::base16Encoding >( strPart );
		}
		else if ( _tcsnicmp( strPart, L"source:", 7 ) == 0 )
		{
			CString strSource = URLDecode( strPart.Mid( 7 ) );
			SafeString( strSource );

			if ( ! m_sURL.IsEmpty() ) m_sURL += ',';
			m_sURL += L"http://";
			m_sURL += URLEncode( strSource );
			m_sURL += L"/(^name^)";
		}
		else if ( _tcsnicmp( strPart, L"name:", 5 ) == 0 ||
				  _tcsnicmp( strPart, L"file:", 5 ) == 0 )
		{
			m_sName = URLDecode( strPart.Mid( 5 ) );
			SafeString( m_sName );
		}
		else if ( _tcschr( strPart, ':' ) == NULL )
		{
			m_sName = URLDecode( strPart );
			SafeString( m_sName );
		}
	}

	if ( ! m_sURL.IsEmpty() )
	{
		if ( ! m_sName.IsEmpty() )
		{
			m_sURL.Replace( L"(^name^)", URLEncode( m_sName ) );
			m_sURL.Replace( L"\\", L"/" );
		}
		else
		{
			m_sURL.Empty();
		}
	}

	if ( HasHash() || ! m_sURL.IsEmpty() )
	{
		m_nAction = uriDownload;
		return TRUE;
	}
	if ( ! m_sName.IsEmpty() )
	{
		m_nAction = uriSearch;
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse "ed2k:" URLs

BOOL CEnvyURL::ParseDonkey(LPCTSTR pszURL)
{
	Clear();

	if ( ! _tcsnicmp( pszURL, L"|file|", 6 ) )
		return ParseDonkeyFile( pszURL + 6 );
	if ( ! _tcsnicmp( pszURL, L"|server|", 8 ) )
		return ParseDonkeyServer( pszURL + 8 );
	if ( ! _tcsnicmp( pszURL, L"|meturl|", 8 ) )
		return ParseDiscovery( pszURL + 8, CDiscoveryService::dsServerList );
	if ( ! _tcsnicmp( pszURL, L"|serverlist|", 12 ) )
		return ParseDiscovery( pszURL + 12, CDiscoveryService::dsServerList );
	if ( ! _tcsnicmp( pszURL, L"|search|", 8 ) )
	{
		// ed2k://|search|text_to_find|/
		CString strURL( pszURL + 8 );

		const int nSep = strURL.Find( L'|' );
		if ( nSep <= 0 ) return FALSE;

		m_sName = URLDecode( strURL.Mid( 0, nSep ) ).Trim();
		if ( m_sName.IsEmpty() ) return FALSE;

		m_nAction = uriSearch;

		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse eDonkey2000 file URL
//
// ed2k://|file|Shareaza_2.1.0.0.exe|3304944|A63D221505E99043B7E7308C67F81986|h=XY5VGKFVGJFYWMOAR5XS44YCEPXSL2JZ|/|sources,1.2.3.4:5555|/

BOOL CEnvyURL::ParseDonkeyFile(LPCTSTR pszURL)
{
	CString strURL( pszURL ), strPart;
	int nSep;

	// ed2k://|file| part should not be passed

	// Name
	nSep = strURL.Find( L'|' );
	if ( nSep < 1 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	m_sName = URLDecode( strPart );
	SafeString( m_sName );
	if ( m_sName.IsEmpty() ) return FALSE;

	// Size
	nSep = strURL.Find( L'|' );
	if ( nSep < 0 ) return FALSE;

	if ( nSep == 0 )	// No size
	{
		strURL = strURL.Mid( 1 );
	}
	else
	{
		strPart = strURL.Left( nSep );
		strURL = strURL.Mid( nSep + 1 );

		QWORD nSize = 0;
		if ( _stscanf( strPart, L"%I64u", &nSize ) != 1 )
			return FALSE;

		m_nSize = nSize;
	}

	// Hash
	nSep = strURL.Find( L'|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	m_oED2K.fromString( strPart );

	// URL is valid
	m_nAction = uriDownload;

	// AICH hash (h), HTTP source (s) and/or hash set (p)
	nSep = strURL.Find( L'|' );
	if ( nSep < 0 ) return TRUE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );
	while ( strPart != L"/" )
	{
		if ( _tcsncmp( strPart, L"h=", 2 ) == 0 )
		{
			// AICH hash
			//theApp.Message( MSG_INFO, L"AICH" );
			strPart = strPart.Mid( 2 );
		}
		else if ( _tcsncmp( strPart, L"s=", 2 ) == 0 )
		{
			// HTTP source
			//theApp.Message( MSG_INFO, L"HTTP" );
			strPart = strPart.Mid( 2 );

			if ( ! m_sURL.IsEmpty() ) m_sURL += L", ";
			SafeString( strPart );
			m_sURL += strPart;
		}
		else if ( _tcsncmp( strPart, L"p=", 2 ) == 0 )
		{
			// Hash set
			//theApp.Message( MSG_INFO, L"hash set" );
			strPart = strPart.Mid( 2 );
		}

		// Read in next chunk
		nSep = strURL.Find( L'|' );
		if ( nSep < 0 ) return TRUE;
		strPart	= strURL.Left( nSep );
		strURL	= strURL.Mid( nSep + 1 );
	}

	while ( strURL.GetLength() > 8 )
	{
		// Source (Starts with |/|sources)
		nSep = strURL.Find( L',' );
		if ( nSep < 0 ) return TRUE;
		strPart	= strURL.Left( nSep );
		strURL	= strURL.Mid( nSep + 1 );

		if ( _tcsncmp( strPart, L"sources", 7 ) != 0 ) return TRUE;

		nSep = strURL.Find( L'|' );
		if ( nSep < 0 ) return TRUE;
		strPart	= strURL.Left( nSep );
		strURL	= strURL.Mid( nSep + 1 );

		// Now we have the source in x.x.x.x:port format.
		CString strEDFTP;
		strEDFTP.Format( L"ed2kftp://%s/%s/%I64u/", strPart, (LPCTSTR)m_oED2K.toString(), m_nSize );
		SafeString( strEDFTP );
		if ( ! m_sURL.IsEmpty() )
			m_sURL += L", ";
		m_sURL += strEDFTP;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse eDonkey2000 server URL
//
// ed2k://|server|1.2.3.4|4661|/

BOOL CEnvyURL::ParseDonkeyServer(LPCTSTR pszURL)
{
	LPCTSTR pszPort = _tcschr( pszURL, '|' );
	if ( pszPort == NULL ) return FALSE;

	if ( _stscanf( pszPort + 1, L"%hu", &m_nPort ) != 1 ) return FALSE;

	m_sName = pszURL;
	m_sName = m_sName.Left( static_cast< int >( pszPort - pszURL ) );

	m_sName.Trim();
	if ( m_sName.IsEmpty() ) return FALSE;

	m_sAddress.Format( L"%s:%hu", m_sName, m_nPort );

	m_nProtocol = PROTOCOL_ED2K;
	m_nAction	= uriHost;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse "mp2p:" URLs

BOOL CEnvyURL::ParsePiolet(LPCTSTR pszURL)
{
	Clear();

	if ( _tcsnicmp( pszURL, L"file|", 5 ) == 0 )
		return ParsePioletFile( pszURL + 5 );
	if ( _tcsnicmp( pszURL, L"|file|", 6 ) == 0 )
		return ParsePioletFile( pszURL + 6 );

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse Piolet file URL
//
// mp2p://file|Shareaza1600.exe|789544|3fb626ed1a9f4cb9921107f510148370/

BOOL CEnvyURL::ParsePioletFile(LPCTSTR pszURL)
{
	CString strURL( pszURL ), strPart;
	int nSep;

	nSep = strURL.Find( L'|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	m_sName = URLDecode( strPart );
	SafeString( m_sName );
	if ( m_sName.IsEmpty() ) return FALSE;

	nSep = strURL.Find( L'|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	QWORD nSize = 0;
	if ( _stscanf( strPart, L"%I64u", &nSize ) != 1 || ! nSize ) return FALSE;
		m_nSize = nSize;

	strPart = strURL.SpanExcluding( L" |/" );
	m_oSHA1.fromString( strPart );

	m_nAction = uriDownload;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEnvyURL parse discovery service URL

BOOL CEnvyURL::ParseDiscovery(LPCTSTR pszURL, int nType)
{
	if ( _tcsncmp( pszURL, L"http://", 7 ) != 0 &&
		 _tcsncmp( pszURL, L"https://", 8 ) != 0 &&
		 _tcsncmp( pszURL, L"uhc:", 4 ) != 0 &&
		 _tcsncmp( pszURL, L"ukhl:", 5 ) != 0 &&
		 _tcsncmp( pszURL, L"gnutella1:host:", 15 ) != 0 &&
		 _tcsncmp( pszURL, L"gnutella2:host:", 15 ) != 0 &&
		 _tcsncmp( pszURL, L"g2:host:", 8 ) != 0 )
		return FALSE;

	CString strURL, strNets, strTemp( pszURL );
	m_nProtocol = PROTOCOL_NULL;

	int nPos = strTemp.Find( L'?' );

	strURL = nPos < 0 ? strTemp : strTemp.Left( nPos );
	strNets = strTemp.Mid( nPos + 1 );

	if ( ! _tcsnicmp( strNets, L"nets=", 5 ) )
	{
		BOOL bG1 = FALSE, bG2 = FALSE;

		if ( _tcsistr( strNets, (LPCTSTR)L"gnutella2" ) )
		{
			bG2 = TRUE;
			strNets.Replace( L"gnutella2", L"" );
		}

		if ( _tcsistr( strNets, (LPCTSTR)L"gnutella" ) )
			bG1 = TRUE;

		if ( bG1 && bG2 )
			;	// Do nothing
		else if ( bG2 )
			m_nProtocol = PROTOCOL_G2;
		else if ( bG1 && Settings.Discovery.EnableG1GWC )
			m_nProtocol = PROTOCOL_G1;
		else
			return FALSE;
	}

	nPos = strURL.Find( L'|' );
	if ( nPos >= 0 )
		strURL = strURL.Left( nPos );

	m_nAction	= uriDiscovery;
	m_sURL		= strURL;
	m_nSize		= nType;

	return TRUE;
}

// Note SkipSlashes/SafeString moved to Strings

/////////////////////////////////////////////////////////////////////////////
// CEnvyURL query constructor

CQuerySearchPtr CEnvyURL::ToQuery() const
{
	if ( m_nAction != uriDownload && m_nAction != uriSearch )
		return CQuerySearchPtr();

	CQuerySearchPtr pSearch = new CQuerySearch();

	if ( ! m_sName.IsEmpty() )
		pSearch->m_sSearch = m_sName;

	if ( m_oSHA1 )
		pSearch->m_oSHA1 = m_oSHA1;

	if ( m_oTiger )
		pSearch->m_oTiger = m_oTiger;

	if ( m_oED2K )
		pSearch->m_oED2K = m_oED2K;

	if ( m_oBTH )
		pSearch->m_oBTH = m_oBTH;

	if ( m_oMD5 )
		pSearch->m_oMD5 = m_oMD5;

	return pSearch;
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyURL shell registration

void CEnvyURL::Register(BOOL bRegister, BOOL bOnStartup)
{
	if ( bRegister )
	{
		RegisterShellType( NULL, L"envy", L"URL:Envy P2P", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterMagnetHandler( L"Envy", L"Envy P2P", L"Envy can automatically search and download selected content on peer-to-peer networks.", L"Envy", IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( L"envy" );
		UnregisterShellType( L"Applications\\Envy.exe" );		// CLIENT_NAME
	}

	if ( CRegistry::GetString( L"Software\\Shareaza\\Shareaza", L"Path" ).IsEmpty() )
	{
		if ( bRegister )
			RegisterShellType( NULL, L"shareaza", L"URL:Shareaza P2P", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		else
			UnregisterShellType( L"shareaza" );
	}

	if ( bRegister && Settings.Web.Magnet )
		RegisterShellType( NULL, L"magnet", L"URL:Magnet Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
	else
		UnregisterShellType( L"magnet" );

	if ( bRegister && Settings.Web.Gnutella )
	{
		RegisterShellType( NULL, L"gnutella", L"URL:Gnutella Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"gnutella1", L"URL:Gnutella1 Bootstrap", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"gnutella2", L"URL:Gnutella2 Bootstrap", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"gwc", L"URL:GWC Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"g2", L"URL:G2 Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"gnet", L"URL:Gnutella Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"uhc", L"URL:Gnutella1 UDP Host Cache", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"ukhl", L"URL:Gnutella2 UDP known Hub Cache", NULL, L"Envy", L"URL", IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( L"gnutella" );
		UnregisterShellType( L"gnutella1" );
		UnregisterShellType( L"gnutella2" );
		UnregisterShellType( L"gwc" );
		UnregisterShellType( L"g2" );
		UnregisterShellType( L"gnet" );
		UnregisterShellType( L"uhc" );
		UnregisterShellType( L"ukhl" );
	}

	if ( bRegister && Settings.Web.ED2K )
		RegisterShellType( NULL, L"ed2k", L"URL:eDonkey2000 Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
	else
		UnregisterShellType( L"ed2k" );

	if ( bRegister && Settings.Web.DC )
	{
		// ToDo: Support "adc:" hubs
		RegisterShellType( NULL, L"adc",    L"URL:DirectConnect Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"dchub",  L"URL:DirectConnect Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
		RegisterShellType( NULL, L"dcfile", L"URL:DirectConnect Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( L"adc" );
		UnregisterShellType( L"dchub" );
		UnregisterShellType( L"dcfile" );
	}

	if ( bRegister && Settings.Web.Piolet )
		RegisterShellType( NULL, L"mp2p", L"URL:Piolet Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
	else
		UnregisterShellType( L"mp2p" );

	if ( bRegister && Settings.Web.Foxy )
		RegisterShellType( NULL, L"foxy", L"URL:Foxy Protocol", NULL, L"Envy", L"URL", IDR_MAINFRAME );
	else
		UnregisterShellType( L"foxy" );

	if ( ! Settings.Live.FirstRun || ! bOnStartup )
	{
		if ( bRegister && Settings.Web.Torrent )
		{
			RegisterShellType( NULL, L"BitTorrent", L"Torrent File", L".torrent",
				L"Envy", L"ENVYFORMAT", IDR_MAINFRAME );
			RegisterShellType( L"Applications\\Envy.exe", NULL, L"Torrent File", L".torrent",
				L"Envy", L"ENVYFORMAT", IDR_MAINFRAME );
		}
		else
		{
			UnregisterShellType( L"BitTorrent" );
			UnregisterShellType( L"Applications\\Envy.exe" );
		}

		// ToDo: .metalink files
	}

	if ( bRegister )
	{
		RegisterShellType( NULL, CLIENT_NAME L".Download", CLIENT_NAME L" Partial Data",
			L".pd", CLIENT_NAME, L"ENVYFORMAT", IDR_MAINFRAME );
		RegisterShellType( L"Applications\\" CLIENT_NAME L".exe", NULL, CLIENT_NAME L" Partial Data",
			L".pd", CLIENT_NAME, L"ENVYFORMAT", IDR_MAINFRAME );

		RegisterShellType( NULL, CLIENT_NAME L".Download", CLIENT_NAME L" Partial Data",
			L".sd", CLIENT_NAME, L"RAZAFORMAT", IDR_MAINFRAME );
		RegisterShellType( L"Applications\\" CLIENT_NAME L".exe", NULL, CLIENT_NAME L" Partial Data",
			L".sd", CLIENT_NAME, L"RAZAFORMAT", IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( L".pd" );
		UnregisterShellType( L".sd" );
		UnregisterShellType( CLIENT_NAME L".Download" );
	}

	if ( bRegister )
	{
		RegisterShellType( NULL, L"Envy.Collection", L"Envy Collection File",
			L".co", L"Envy", L"ENVYFORMAT", IDI_COLLECTION );
		RegisterShellType( L"Applications\\Envy.exe", NULL, L"Envy Collection File",
			L".co", L"Envy", L"ENVYFORMAT", IDI_COLLECTION );

		RegisterShellType( NULL, L"Envy.Collection", L"Envy Collection File",
			L".collection", L"Envy", L"ENVYFORMAT", IDI_COLLECTION );
		RegisterShellType( L"Applications\\Envy.exe", NULL, L"Envy Collection File",
			L".collection", L"Envy", L"ENVYFORMAT", IDI_COLLECTION );

		RegisterShellType( NULL, L"eMule.Collection", L"eMule Collection File",
			L".emulecollection", L"Envy", L"ENVYFORMAT", IDI_COLLECTION );
		RegisterShellType( L"Applications\\Envy.exe", NULL, L"eMule Collection File",
			L".emulecollection", L"Envy", L"ENVYFORMAT", IDI_COLLECTION );
	}
	else
	{
		UnregisterShellType( L".co" );
		UnregisterShellType( L".collection" );
		UnregisterShellType( L".emulecollection" );
		UnregisterShellType( L"eMule.Collection" );
		UnregisterShellType( L"Envy.Collection" );	// CLIENT_NAME
	}

	if ( ! bOnStartup )
		SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyURL shell registration helper

BOOL CEnvyURL::RegisterShellType(LPCTSTR pszRoot, LPCTSTR pszProtocol, LPCTSTR pszName, LPCTSTR pszType, LPCTSTR pszApplication, LPCTSTR pszTopic, UINT nIDIcon, BOOL bOverwrite)
{
	HKEY hRootKey = AfxGetPerUserRegistration() ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
	LPCTSTR szRootKey = L"Software\\Classes";

	HKEY hKey, hSub1, hSub2, hSub3, hSub4;
	CString strValue;
	DWORD nDisposition;

	CString strSubKey = szRootKey;
	if ( pszRoot )
	{
		if ( ! strSubKey.IsEmpty() )
			strSubKey += L"\\";
		strSubKey += pszRoot;
	}
	if ( pszProtocol )
	{
		if ( ! strSubKey.IsEmpty() )
			strSubKey += L"\\";
		strSubKey += pszProtocol;
	}

	if ( RegCreateKeyEx( hRootKey, (LPCTSTR)strSubKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &nDisposition ) != ERROR_SUCCESS )
		return FALSE;

	if ( nDisposition == REG_OPENED_EXISTING_KEY && ! bOverwrite )
	{
		RegCloseKey( hKey );
		return FALSE;
	}

	const BOOL bProtocol = _tcsncmp( pszName, L"URL:", 4 ) == 0;
	const BOOL bApplication = pszRoot && _tcsicmp( pszRoot, L"Applications\\Envy.exe" ) == 0;		// CLIENT_NAME

	// Register protocol to "Default Programs"
	if ( bProtocol )
	{
		CString strUrlAssociations = L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\";
		strUrlAssociations += pszProtocol;
		SHSetValue( hRootKey, strUrlAssociations + L"\\UserChoice", L"Progid", REG_STRING( CLIENT_NAME ) );
	}

	if ( ! bApplication )
	{
		RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)pszName,
			static_cast< DWORD >( sizeof( TCHAR ) * ( _tcslen( pszName ) + 1 ) ) );

		if ( bProtocol )
			RegSetValueEx( hKey, L"URL Protocol", 0, REG_SZ, (LPBYTE)(LPCTSTR)L"", sizeof( TCHAR ) );

		if ( RegCreateKey( hKey, L"DefaultIcon", &hSub1 ) == ERROR_SUCCESS )
		{
			strValue = Skin.GetImagePath( nIDIcon );
			RegSetValueEx( hSub1, NULL, 0, REG_SZ,
				(LPBYTE)(LPCTSTR)strValue, sizeof( TCHAR ) * ( strValue.GetLength() + 1 ) );
			RegCloseKey( hSub1 );
		}
	}
	else if ( pszType != NULL )
	{
		HKEY hKeySupported;
		if ( RegCreateKey( hKey, L"SupportedTypes", &hKeySupported ) == ERROR_SUCCESS )
		{
			RegSetValueEx( hKeySupported, pszType, 0, REG_NONE, NULL, 0 );
			RegCloseKey( hKeySupported );
		}

		RegSetValueEx( hKey, L"FriendlyAppName", 0, REG_STRING( CLIENT_NAME ) );
	}

	if ( RegCreateKey( hKey, L"shell", &hSub1 ) == ERROR_SUCCESS )
	{
		if ( RegCreateKey( hSub1, L"open", &hSub2 ) == ERROR_SUCCESS )
		{
			if ( RegCreateKey( hSub2, L"command", &hSub3 ) == ERROR_SUCCESS )
			{
				strValue.Format( L"\"%s\" \"%%%c\"", theApp.m_strBinaryPath, bProtocol ? 'L' : '1' );
				RegSetValueEx( hSub3, NULL, 0, REG_SZ, (LPBYTE)(LPCTSTR)strValue, sizeof( TCHAR ) * ( strValue.GetLength() + 1 ) );
				RegCloseKey( hSub3 );
			}

			if ( RegCreateKey( hSub2, L"ddeexec", &hSub3 ) == ERROR_SUCCESS )
			{
				RegSetValueEx( hSub3, NULL, 0, REG_SZ, (LPBYTE)L"%1", sizeof( L"%1" ) );

				if ( RegCreateKey( hSub3, L"Application", &hSub4 ) == ERROR_SUCCESS )
				{
					RegSetValueEx( hSub4, NULL, 0, REG_SZ, (LPBYTE)pszApplication,
						static_cast< DWORD >( sizeof( TCHAR ) * ( _tcslen( pszApplication ) + 1 ) ) );
					RegCloseKey( hSub4 );
				}

				if ( RegCreateKey( hSub3, L"Topic", &hSub4 ) == ERROR_SUCCESS )
				{
					RegSetValueEx( hSub4, NULL, 0, REG_SZ, (LPBYTE)pszTopic,
						static_cast< DWORD >( sizeof( TCHAR ) * ( _tcslen( pszTopic ) + 1 ) ) );
					RegCloseKey( hSub4 );
				}

				if ( RegCreateKey( hSub3, L"IfExec", &hSub4 ) == ERROR_SUCCESS )
				{
					RegSetValueEx( hSub4, NULL, 0, REG_STRING( L"*" ) );
					RegCloseKey( hSub4 );
				}

				RegSetValueEx( hSub3, L"WindowClassName", 0, REG_STRING( CLIENT_HWND ) );	// "EnvyMainWnd"

				RegCloseKey( hSub3 );
			}

			RegCloseKey( hSub2 );
		}

		RegCloseKey( hSub1 );
	}

	if ( ! bApplication )
	{
		if ( pszType && *pszType == L'.' )
		{
			DWORD dwData = /*FTA_OpenIsSafe*/ 0x00010000;	// FILETYPEATTRIBUTEFLAGS
			RegSetValueEx( hKey, L"EditFlags", 0, REG_NUMBER( dwData ) );
			RegSetValueEx( hKey, L"AppUserModelID", 0, REG_STRING( CLIENT_NAME ) );
		}
		else if ( bProtocol )
		{
			DWORD dwData = /*FTA_Show*/ 0x00000002;			// FILETYPEATTRIBUTEFLAGS
			RegSetValueEx( hKey, L"EditFlags", 0, REG_NUMBER( dwData ) );
			RegSetValueEx( hKey, L"AppUserModelID", 0, REG_STRING( CLIENT_NAME ) );
		}
	}

	RegCloseKey( hKey );

	if ( pszType && pszProtocol )
	{
		strSubKey = szRootKey;
		if ( pszRoot )
		{
			if ( ! strSubKey.IsEmpty() )
				strSubKey += L"\\";
			strSubKey += pszRoot;
		}
		if ( ! strSubKey.IsEmpty() )
			strSubKey += L"\\";
		strSubKey += pszType;

		if ( RegCreateKeyEx( hRootKey, (LPCTSTR)strSubKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &nDisposition ) == ERROR_SUCCESS )
		{
			RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)pszProtocol,
				static_cast< DWORD >( sizeof( TCHAR ) * ( _tcslen( pszProtocol ) + 1 ) ) );

		//	if ( RegCreateKey( hKey, L"OpenWithProgids", &hSub1 ) == ERROR_SUCCESS )
		//	{
		//		RegSetValueEx( hSub1, pszProtocol, 0, REG_NONE, NULL, 0 );
		//		RegCloseKey( hSub1 );
		//	}

			RegCloseKey( hKey );
		}
	}

	return TRUE;
}

BOOL CEnvyURL::UnregisterShellType(LPCTSTR pszRoot)
{
	HKEY hKey, hRootKey = AfxGetPerUserRegistration() ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
	LPCTSTR szRootKey = L"Software\\Classes";

	BOOL bRegisteredUser = FALSE;

	CString strSubKey = szRootKey, strOldKey;
	if ( pszRoot )
	{
		if ( ! strSubKey.IsEmpty() )
			strSubKey += L"\\";
		strSubKey += pszRoot;
	}

	// Obsolete method:
	//if ( ! bRegisteredUser )
	//{
	//	CString strSubKey;
	//	strSubKey.Format( L"Software\\Classes\\%s", pszProtocol );
	//	CString strPath = CRegistry::GetString( L"shell\\open\\command", NULL, NULL, strSubKey );
	//	if ( _tcsistr( strPath, theApp.m_strBinaryPath ) != NULL ||
	//		CRegistry::GetString( L"shell\\open\\ddeexec\\Application", NULL, NULL, strSubKey ) == L"Envy" )
	//	{
	//		CRegistry::DeleteKey( HKEY_CURRENT_USER, (LPCTSTR)strSubKey );
	//	}
	//}

	// Delete protocol from "Default Programs"
	if ( pszRoot && *pszRoot != L'.' )
	{
		CString strProgID;
		CString strUrlAssociations = L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\";
		strUrlAssociations += pszRoot;
		DWORD nType, nSize = MAX_PATH * sizeof( TCHAR );
		LRESULT ret = SHGetValue( hRootKey, strUrlAssociations + L"\\UserChoice", L"Progid", &nType, strProgID.GetBuffer( MAX_PATH ), &nSize );
		strProgID.ReleaseBuffer();
		if ( ret == ERROR_SUCCESS && strProgID.CompareNoCase( CLIENT_NAME ) == 0 )
			SHDeleteKey( hRootKey, strUrlAssociations );
	}
	else if ( pszRoot && *pszRoot == L'.' )
	{
		// Get real key for file extension
		if ( RegOpenKeyEx( hRootKey, strSubKey, 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
		{
			CString strPath;
			DWORD dwType;
			DWORD dwSize = MAX_PATH * sizeof( TCHAR );
			RegQueryValueEx( hKey, NULL, NULL, &dwType, (LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
			strPath.ReleaseBuffer();
			if ( ! strPath.IsEmpty() )
			{
				strOldKey = strSubKey;
				strSubKey = szRootKey;
				if ( ! strSubKey.IsEmpty() )
					strSubKey += L"\\";
				strSubKey += strPath;
			}
			RegCloseKey( hKey );
		}
	}

	if ( RegOpenKeyEx( hRootKey, strSubKey + L"\\shell\\open\\command", 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		CString strPath;
		DWORD dwType, dwSize = MAX_PATH * sizeof( TCHAR );
		RegQueryValueEx( hKey, NULL, NULL, &dwType, (LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
		strPath.ReleaseBuffer();
		if ( _tcsistr( strPath, theApp.m_strBinaryPath ) != NULL )
			bRegisteredUser = TRUE;

		RegCloseKey( hKey );
	}

	if ( ! bRegisteredUser )
	{
		if ( RegOpenKeyEx( hRootKey, strSubKey + L"\\shell\\open\\ddeexec\\Application", 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
		{
			CString strPath;
			DWORD dwType, dwSize = MAX_PATH * sizeof( TCHAR );
			RegQueryValueEx( hKey, NULL, NULL, &dwType, (LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
			strPath.ReleaseBuffer();
			if ( _tcsistr( strPath, CLIENT_NAME ) != NULL )
				bRegisteredUser = TRUE;

			RegCloseKey( hKey );
		}
	}

	if ( bRegisteredUser )
		SHDeleteKey( hRootKey, strOldKey.IsEmpty() ? (LPCTSTR)strSubKey : (LPCTSTR)strOldKey );

	return bRegisteredUser;
}

// Note recursive delete moved to CRegistry::DeleteKey(HKEY, LPCTSTR), SHDeleteKey is used

/////////////////////////////////////////////////////////////////////////////
// CEnvyURL magnet registration helper

BOOL CEnvyURL::RegisterMagnetHandler(LPCTSTR pszID, LPCTSTR pszName, LPCTSTR pszDescription, LPCTSTR pszApplication, UINT nIDIcon)
{
	HKEY hSoftware, hMagnetRoot, hHandlers, hHandler;
	DWORD dwDisposition;
	LONG lResult;

	lResult = RegOpenKeyEx( HKEY_CURRENT_USER, L"Software",
		0, KEY_ALL_ACCESS, &hSoftware );

	if ( lResult != ERROR_SUCCESS ) return FALSE;

	lResult = RegCreateKeyEx( hSoftware, L"Magnet",
		0, NULL, 0, KEY_ALL_ACCESS, NULL, &hMagnetRoot, &dwDisposition );

	if ( lResult != ERROR_SUCCESS )
	{
		RegCloseKey( hSoftware );
		return FALSE;
	}

	lResult = RegCreateKeyEx( hMagnetRoot, L"Handlers",
		0, NULL, 0, KEY_ALL_ACCESS, NULL, &hHandlers, &dwDisposition );

	if ( lResult != ERROR_SUCCESS )
	{
		RegCloseKey( hMagnetRoot );
		RegCloseKey( hSoftware );
		return FALSE;
	}

	lResult = RegCreateKeyEx( hHandlers, pszID,
		0, NULL, 0, KEY_ALL_ACCESS, NULL, &hHandler, &dwDisposition );

	if ( lResult != ERROR_SUCCESS )
	{
		RegCloseKey( hHandler );
		RegCloseKey( hMagnetRoot );
		RegCloseKey( hSoftware );
		return FALSE;
	}

	CString strCommand;
	CString strIcon( Skin.GetImagePath( nIDIcon ) );
	strCommand.Format( L"\"%s\" \"%%URL\"", theApp.m_strBinaryPath );

	RegSetValueEx( hHandler, L"", 0, REG_SZ,
		(LPBYTE)pszName, static_cast< DWORD >( sizeof( TCHAR ) * ( _tcslen( pszName ) + 1 ) ) );

	RegSetValueEx( hHandler, L"Description", 0, REG_SZ,
		(LPBYTE)pszDescription, static_cast< DWORD >( sizeof( TCHAR ) * ( _tcslen( pszDescription ) + 1 ) ) );

	RegSetValueEx( hHandler, L"DefaultIcon", 0, REG_SZ,
		(LPBYTE)(LPCTSTR)strIcon, sizeof( TCHAR ) * ( strIcon.GetLength() + 1 ) );

	RegSetValueEx( hHandler, L"ShellExecute", 0, REG_SZ,
		(LPBYTE)(LPCTSTR)strCommand, sizeof( TCHAR ) * ( strCommand.GetLength() + 1 ) );

	RegSetValueEx( hHandler, L"DdeApplication", 0, REG_SZ,
		(LPBYTE)pszApplication, static_cast< DWORD >( sizeof( TCHAR ) * ( _tcslen( pszApplication ) + 1 ) ) );

	RegSetValueEx( hHandler, L"DdeTopic", 0, REG_SZ,
		(LPBYTE)L"URL", sizeof( TCHAR ) * 4 );

	RegCloseKey( hHandler );
	RegCloseKey( hHandlers );
	RegCloseKey( hMagnetRoot );
	RegCloseKey( hSoftware );

	return TRUE;
}
