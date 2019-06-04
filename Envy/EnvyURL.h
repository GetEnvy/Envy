//
// EnvyURL.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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

#pragma once

#include "DiscoveryServices.h"
#include "QuerySearch.h"

class CBTInfo;


class CEnvyURL : public CEnvyFile
{
public:
	CEnvyURL(LPCTSTR pszURL = NULL);
	CEnvyURL(CBTInfo* pTorrent);
	CEnvyURL(const CEnvyURL& pURL);
	virtual ~CEnvyURL();

public:
	enum URI_TYPE
	{
		uriNull,
		uriDownload,
		uriSource,
		uriSearch,
		uriHost,
		uriBrowse,
		uriDiscovery,
		uriCommand
	};

	PROTOCOLID		m_nProtocol;
	URI_TYPE		m_nAction;
	CString			m_sAddress;
	IN_ADDR			m_pAddress;
	WORD			m_nPort;
	IN_ADDR			m_pServerAddress;
	WORD			m_nServerPort;
	CString			m_sLogin;
	CString			m_sPassword;
	CAutoPtr< CBTInfo >	m_pTorrent;
	Hashes::BtGuid	m_oBTC;

public:
	BOOL	Parse(const CString& sText, CList< CString >& pURLs, BOOL bResolve = FALSE);	// Parse URL list
	BOOL	Parse(LPCTSTR pszURL, BOOL bResolve = TRUE);		// Parse single URL
	CQuerySearchPtr ToQuery() const;							// Construct CQuerySearch object

	CDiscoveryService::Type	GetDiscoveryService() const
	{
		return (CDiscoveryService::Type)(int)m_nSize;
	}

protected:
	void	Clear();

	BOOL	ParseRoot(LPCTSTR pszURL, BOOL bResolve = TRUE);
	BOOL	ParseHTTP(LPCTSTR pszURL, BOOL bResolve = TRUE);
	BOOL	ParseFTP(LPCTSTR pszURL, BOOL bResolve = TRUE);
	BOOL	ParseED2KFTP(LPCTSTR pszURL, BOOL bResolve = TRUE);	// ed2kftp://[client_id@]address:port/{md4_hash}/{size}/
	BOOL	ParseDCFile(LPCTSTR pszURL, BOOL bResolve = TRUE); 	// dcfile://address:port/login/TTH:tiger_hash/size/	 (Deprecated?)
	BOOL	ParseDCHub(LPCTSTR pszURL, BOOL bResolve = TRUE);  	// dchub://[login@]address:port/[filepath]  -Can be regular path or "files.xml.bz2" or "TTH:tiger_hash/size/"	 (adc:// ?)
	BOOL	ParseBTC(LPCTSTR pszURL, BOOL bResolve = TRUE);		// btc://address:port/[{node_guid}]/{btih_hash}/
	BOOL	ParseMagnet(LPCTSTR pszURL);						// magnet:?{params}
												// Host:		// envy:[//]{verb}{[user@]address[:port]}  -Where {verb} is "" (empty), "host:", "hub:", "server:", "browse:" or "btnode:"
												// WebCache:	// envy:[//]gwc:{url}[?nets={net_list}]  -Where {net_list} is "gnutella" or "gnutella2"
												// Discovery:	// envy:[//]{verb}{url}  -Where {verb} is "uhc:", "ukhl:", "gnutella1:host:" or "gnutella2:host:"
												// ServerMet:	// envy:[//]meturl:{url}
												// URL:			// envy:[//]url:{nested_url}
	BOOL	ParseEnvy(LPCTSTR pszURL);
	BOOL	ParseEnvyFile(LPCTSTR pszURL);
	BOOL	ParseEnvyHost(LPCTSTR pszURL, BOOL bBrowse = FALSE, PROTOCOLID nProtocol = PROTOCOL_G2);
	BOOL	ParseDiscovery(LPCTSTR pszURL, int nType);
	BOOL	ParseDonkey(LPCTSTR pszURL);						// ed2k://|file|{name}|{size}|{md4_hash}|/	ed2k://|server|{address}|{port}|/	ed2k://|search|{query}|/
	BOOL	ParseDonkeyFile(LPCTSTR pszURL);
	BOOL	ParseDonkeyServer(LPCTSTR pszURL);
	BOOL	ParsePiolet(LPCTSTR pszURL);						// mp2p://[|]file|{name}|{size}|{sha1_hash}/
	BOOL	ParsePioletFile(LPCTSTR pszURL);

// Registration Operations
public:
	static void	Register(BOOL bRegister = TRUE, BOOL bOnStartup = FALSE);

protected:
	static BOOL	RegisterMagnetHandler(LPCTSTR pszID, LPCTSTR pszName, LPCTSTR pszDescription, LPCTSTR pszApplication, UINT nIDIcon);
	static BOOL	RegisterShellType(LPCTSTR pszRoot, LPCTSTR pszProtocol, LPCTSTR pszName, LPCTSTR pszType, LPCTSTR pszApplication, LPCTSTR pszTopic, UINT nIDIcon, BOOL bOverwrite = TRUE);
	static BOOL	UnregisterShellType(LPCTSTR pszRoot);
};
