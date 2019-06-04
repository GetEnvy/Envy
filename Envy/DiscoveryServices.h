//
// DiscoveryServices.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2012
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

#include "HttpRequest.h"

#define DSGnutellaTCP		"gnutella1:host:"
#define DSGnutella2TCP		"gnutella2:host:"
#define DSGnutellaUDPHC		"uhc:"
#define DSGnutella2UDPKHL	"ukhl:"

// ToDo: Remove Network locks
// ToDo: Add support for concurrent webcache requests


class CDiscoveryService
{
public:
	enum Type
	{
		dsNull,
		dsWebCache,			// 'M'/'2'/'1'
		dsGnutella,			// 'U'
		dsServerList,		// 'D'/'H'	dsServerMet/dsDCHubList
		dsBlocked			// 'X'
	};

	enum SubType
	{
		dsOldBootStrap,
		dsGnutellaTCP,		// "gnutella1:host:"
		dsGnutella2TCP,		// "gnutella2:host:"
		dsGnutellaUDPHC,	// "uhc:"
		dsGnutella2UDPKHL	// "ukhl:"
	};

	CDiscoveryService(Type nType = dsNull, LPCTSTR pszAddress = NULL, PROTOCOLID nProtocol = PROTOCOL_NULL);
	virtual ~CDiscoveryService();

public:
	Type		m_nType;
	CString		m_sAddress;
	BOOL		m_bGnutella2;			// Webcache supports G2
	BOOL		m_bGnutella1;			// Webcache supports Gnutella
	DWORD		m_tCreated;
	DWORD		m_tAccessed;
	DWORD		m_nAccesses;
	DWORD		m_tUpdated;
	DWORD		m_nUpdates;
	DWORD		m_nHosts;
	DWORD		m_nTotalHosts;
	DWORD		m_nURLs;
	DWORD		m_nTotalURLs;
	DWORD		m_nFailures;
	DWORD		m_nAccessPeriod;
	DWORD		m_nUpdatePeriod;
	IN_ADDR		m_pAddress;
	WORD		m_nPort;
	CString		m_sPong;
	SubType		m_nSubType;
	PROTOCOLID	m_nProtocolID;

	void		Remove(BOOL bCheck = TRUE);
	void		OnSuccess();
	void		OnFailure();
	void		OnCopyGiven();			// Used in Datagrams.cpp

protected:
	void		OnAccess();
	void		OnGivenHosts();
	void		OnHostAdd(int nCount = 1);
	void		OnURLAdd(int nCount = 1);
	void		Serialize(CArchive& ar, int nVersion);
	BOOL		ResolveGnutella(BOOL bForced = FALSE);

	friend class CDiscoveryServices;
};


class CDiscoveryServices : public CThreadImpl
{
public:
	CDiscoveryServices();
	virtual ~CDiscoveryServices();

	enum Mode
	{
		wcmHosts,		// Query G1/G2/Multi service for hosts
		wcmCaches,		// Query G1/G2/Multi service for caches
		wcmUpdate,		// Update web cache (includes advertising)
		wcmSubmit,		// Advertise web cache to a random web cache
		wcmServerList	// Query eDonkey/DC++ service (wcmServerMet/wcmDCHubList)
	};

protected:
	CList< CDiscoveryService* > m_pList;
	CHttpRequest		m_pRequest;
	CDiscoveryService*	m_pSubmit;
	CDiscoveryService*	m_pWebCache;
	Mode				m_nWebCache;
	PROTOCOLID			m_nLastQueryProtocol;		// Protocol that was queried most recently
	PROTOCOLID			m_nLastUpdateProtocol;		// Protocol that had a service update most recently
	BOOL				m_bFirstTime;
	DWORD				m_tExecute;					// Time the Execute() function was last run
	DWORD				m_tUpdated;					// Time a webcache was last updated
	DWORD				m_tQueried;					// Time a webcache/MET was last queried
//	DWORD				m_tMetQueried;				// Time a MET was last queried, currently using static
//	DWORD				m_tHubsQueried;				// Time a hublist was last queried, currently using static

public:
	POSITION			GetIterator() const;
	CDiscoveryService*	GetNext(POSITION& pos) const;
	BOOL				Check(const CDiscoveryService* pService, CDiscoveryService::Type nType = CDiscoveryService::dsNull) const;
	BOOL				Add(CDiscoveryService* pService);
	BOOL				Add(LPCTSTR pszAddress, CDiscoveryService::Type nType, PROTOCOLID nProtocol = PROTOCOL_NULL);
	BOOL				CheckMinimumServices();
	DWORD				LastExecute() const;
	CDiscoveryService*	GetByAddress(LPCTSTR pszAddress) const;
	CDiscoveryService*	GetByAddress(const IN_ADDR* pAddress, WORD nPort, CDiscoveryService::SubType nSubType );
	BOOL				Load();
	BOOL				Save();
	BOOL				Update();
	void				Stop();
	void				Clear();
	BOOL				Query(CDiscoveryService* pService, Mode nMode);
	BOOL				Execute(PROTOCOLID nProtocol = PROTOCOL_NULL, USHORT nForceDiscovery = FALSE);
	void				ExecuteBootstraps(PROTOCOLID nProtocol = PROTOCOL_NULL);
	void				OnResolve(PROTOCOLID nProtocol, LPCTSTR szAddress, const IN_ADDR* pAddress = NULL, WORD nPort = 0);

protected:
	void				Remove(CDiscoveryService* pService, BOOL bCheck = TRUE);
	DWORD				GetCount(CDiscoveryService::Type nType = CDiscoveryService::dsNull, PROTOCOLID nProtocol = PROTOCOL_NULL) const;
	BOOL				CheckWebCacheValid(LPCTSTR pszAddress);
	void				Serialize(CArchive& ar);
	int					ExecuteBootstraps(int nCount, BOOL bUDP = FALSE, PROTOCOLID nProtocol = PROTOCOL_NULL);
	BOOL				RequestRandomService(PROTOCOLID nProtocol);
	CDiscoveryService*  GetRandomService(PROTOCOLID nProtocol);
	CDiscoveryService*	GetRandomWebCache(PROTOCOLID nProtocol, BOOL bWorkingOnly, CDiscoveryService* pExclude = NULL, BOOL bForUpdate = FALSE);
	BOOL				RequestWebCache(BOOL bForced, CDiscoveryService* pService, Mode nMode, PROTOCOLID nProtocol = PROTOCOL_NULL);
	BOOL				RunWebCacheGet(BOOL bCache);
	BOOL				RunWebCacheUpdate();
	BOOL				RunServerList();	// RunWebCacheFile() -was RunServerMet()
	BOOL				SendWebCacheRequest(const CString& strURL);
	BOOL				EnoughServices() const;
	void				AddDefaults();
	void				MergeURLs();
	void				OnRun();

	friend class CDiscoveryService;

private:
	CDiscoveryServices(const CDiscoveryServices&);
	CDiscoveryServices& operator=(const CDiscoveryServices&);
};

extern CDiscoveryServices DiscoveryServices;
