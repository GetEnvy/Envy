//
// HostCache.cpp
//
// This file is part of Envy (getenvy.com)  2016-2018
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "Buffer.h"
#include "PacketLengthValidate.h"
#include "EDPacket.h"
#include "Neighbours.h"
#include "Network.h"
#include "Security.h"
#include "VendorCache.h"
#include "XML.h"
#include "BootstrapCatalog.h"
#include "KadNodesDat.h"
#include "KadBootstrapColdStart.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CHostCache HostCache;


//////////////////////////////////////////////////////////////////////
// CHostCache construction

CHostCache::CHostCache()
	: Gnutella2	( PROTOCOL_G2 )
	, Gnutella1	( PROTOCOL_G1 )
	, G1DNA 	( PROTOCOL_G1 )
	, eDonkey	( PROTOCOL_ED2K )
	, BitTorrent( PROTOCOL_BT )
	, Kademlia	( PROTOCOL_KAD )
	, DC		( PROTOCOL_DC )
//	, m_tLastPruneTime ( 0 )	// Using static
{
	m_pList.AddTail( &Gnutella1 );
	m_pList.AddTail( &Gnutella2 );
	m_pList.AddTail( &eDonkey );
	m_pList.AddTail( &G1DNA );
	m_pList.AddTail( &BitTorrent );
	m_pList.AddTail( &Kademlia );
	m_pList.AddTail( &DC );
}

//////////////////////////////////////////////////////////////////////
// CHostCache core operations

void CHostCache::Clear()
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		pCache->Clear();
	}
}

BOOL CHostCache::Load()
{
	const CString strFile = Settings.General.DataPath + L"HostCache.dat";

	BOOL bResult = FALSE;

	CFile pFile;
	if ( pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load, 262144 );	// 256 KB buffer
			try
			{
				{
					CQuickLock oLock( m_pSection );

					Clear();

					Serialize( ar );
					ar.Close();
				}

				pFile.Close();

				bResult = TRUE;	// Success
			}
			catch ( CException* pException )
			{
				ar.Abort();
				pFile.Abort();
				pException->Delete();
			}
		}
		catch ( CException* pException )
		{
			pFile.Abort();
			pException->Delete();
		}
	}

	if ( Gnutella2.IsEmpty() )	CheckMinimumServers( PROTOCOL_G2 );
	if ( Gnutella1.IsEmpty() )	CheckMinimumServers( PROTOCOL_G1 );
	if ( eDonkey.IsEmpty() )	CheckMinimumServers( PROTOCOL_ED2K );
	if ( BitTorrent.IsEmpty() )	CheckMinimumServers( PROTOCOL_BT );
	if ( Kademlia.IsEmpty() )	CheckMinimumServers( PROTOCOL_KAD );
	if ( DC.IsEmpty() )			CheckMinimumServers( PROTOCOL_DC );

	if ( ! bResult )
		theApp.Message( MSG_ERROR, L"Failed to load host cache: %s", (LPCTSTR)strFile );

	return bResult;
}

BOOL CHostCache::Save()
{
	const CString strFile = Settings.General.DataPath + L"HostCache.dat";
	const CString strTemp = Settings.General.DataPath + L"HostCache.tmp";

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save host cache: %s", (LPCTSTR)strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store, 262144 );		// 256 KB buffer
		try
		{
			CQuickLock oLock( m_pSection );

			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, L"Failed to save host cache: %s", (LPCTSTR)strTemp );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save host cache: %s", (LPCTSTR)strTemp );
		return FALSE;
	}

	if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save host cache: %s", (LPCTSTR)strFile );
		return FALSE;
	}

	return TRUE;
}


// Set at INTERNAL_VERSION on change:
#define HOSTCACHE_SER_VERSION 2

// nVersion History:
// 14 - Add m_sCountry
// 15 - Add m_bDHT and m_oBtGUID (Ryo-oh-ki)
// 16 - Add m_nUDPPort, m_oGUID and m_nKADVersion (Ryo-oh-ki)
// 17 - Add m_tConnect (Ryo-oh-ki)
// 18 - Add m_sUser and m_sPass (Ryo-oh-ki)
// 19 - Add m_sAddress (Ryo-oh-ki)
// 1000 - Remove m_bDHT
// 1 - (Envy 1.0)
// 2 - Add m_nCodePage (NMDC hub text encoding)

void CHostCache::Serialize(CArchive& ar)
{
	int nVersion = HOSTCACHE_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;
		ar.WriteCount( m_pList.GetCount() );

		for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
		{
			CHostCacheList* pCache = m_pList.GetNext( pos );
			ar << pCache->m_nProtocol;
			pCache->Serialize( ar, nVersion );
		}
	}
	else // Loading
	{
		ar >> nVersion;
		if ( nVersion > INTERNAL_VERSION && nVersion != 1000 )
			AfxThrowUserException();

		for ( DWORD_PTR nCount = ar.ReadCount(); nCount > 0; nCount-- )
		{
			PROTOCOLID nProtocol;
			ar >> nProtocol;

			for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
			{
				CHostCacheList* pCache = m_pList.GetNext( pos );
				if ( pCache->m_nProtocol == nProtocol )
				{
					pCache->Serialize( ar, nVersion );
					break;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCache prune old hosts

void CHostCache::PruneOldHosts()
{
	const DWORD tNow = static_cast< DWORD >( time( NULL ) );
	static DWORD tLastPruneTime = 0;

	if ( tNow > tLastPruneTime + 90 )		// Every minute+ (ToDo: Setting?)
	{
		for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
		{
			m_pList.GetNext( pos )->PruneOldHosts( tNow );
		}

		tLastPruneTime = tNow;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCache forwarding operations

CHostCacheHostPtr CHostCache::Find(const IN_ADDR* pAddress) const
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( CHostCacheHostPtr pHost = pCache->Find( pAddress ) )
			return pHost;
	}
	return NULL;
}

CHostCacheHostPtr CHostCache::Find(LPCTSTR szAddress) const
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( CHostCacheHostPtr pHost = pCache->Find( szAddress ) )
			return pHost;
	}
	return NULL;
}

BOOL CHostCache::Check(const CHostCacheHostPtr pHost) const
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( pCache->Check( pHost ) )
			return TRUE;
	}
	return FALSE;
}

void CHostCache::Remove(CHostCacheHostPtr pHost)
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		pCache->Remove( pHost );
	}
}

void CHostCache::SanityCheck()
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		pCache->SanityCheck();
	}
}

void CHostCache::OnResolve(PROTOCOLID nProtocol, LPCTSTR szAddress, const IN_ADDR* pAddress, WORD nPort)
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( nProtocol == PROTOCOL_NULL || nProtocol == pCache->m_nProtocol )
			pCache->OnResolve( szAddress, pAddress, nPort );
	}
}

void CHostCache::OnFailure(const IN_ADDR* pAddress, WORD nPort, PROTOCOLID nProtocol, bool bRemove)
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( nProtocol == PROTOCOL_NULL || nProtocol == pCache->m_nProtocol )
			pCache->OnFailure( pAddress, nPort, bRemove );
	}
}

void CHostCache::OnSuccess(const IN_ADDR* pAddress, WORD nPort, PROTOCOLID nProtocol, bool bUpdate)
{
	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( nProtocol == PROTOCOL_NULL || nProtocol == pCache->m_nProtocol )
			pCache->OnSuccess( pAddress, nPort, bUpdate );
	}
}


//////////////////////////////////////////////////////////////////////
// CHostCacheList construction

CHostCacheList::CHostCacheList(PROTOCOLID nProtocol)
	: m_nProtocol	( nProtocol )
	, m_nCookie		( 0 )
{
}

CHostCacheList::~CHostCacheList()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList clear

void CHostCacheList::Clear()
{
	CQuickLock oLock( m_pSection );

#ifndef PUBLIC_RELEASE_FIX
	// ToDo: Crash at exit, Fix properly
	for ( CHostCacheMapItr i = m_Hosts.begin(); i != m_Hosts.end(); ++i )
	{
		delete (*i).second;
	}
#endif

	m_Hosts.clear();
	m_HostsTime.clear();

	m_nCookie++;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList host add

CHostCacheHostPtr CHostCacheList::Add(LPCTSTR pszHost, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit, bool* pbAdded)
{
	CString strHost( pszHost );
	strHost.Trim();

	int nPos = strHost.ReverseFind( L' ' );
	if ( nPos < 1 )
		nPos = strHost.ReverseFind( L'\t' );
	if ( nPos > 0 )
	{
		CString strTime = strHost.Mid( nPos + 1 );
		strHost = strHost.Left( nPos );
		strHost.TrimRight();
		tSeen = TimeFromString( strTime );
		if ( ! tSeen )
			return NULL;
	}

	return Add(NULL, nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit, strHost, pbAdded);
}

CHostCacheHostPtr CHostCacheList::Add(const IN_ADDR* pAddress, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit, LPCTSTR szAddress, bool* pbAdded)
{
	ASSERT( pAddress || szAddress );
	if (pbAdded)
		*pbAdded = false;

	if ( ! nPort )
		nPort = protocolPorts[ m_nProtocol ];	// Use default port

	SOCKADDR_IN saHost;
	if ( ! pAddress )
	{
		// Try to quick resolve dotted IP address
		if ( ! Network.Resolve( szAddress, nPort, &saHost, FALSE ) )
			return NULL;	// Bad address

		pAddress = &saHost.sin_addr;
		nPort = ntohs( saHost.sin_port );
		if ( pAddress->s_addr != INADDR_ANY )		// Dotted IP address
			szAddress = NULL;
	}

	if ( pAddress->s_addr != INADDR_ANY )
	{
		if ( ! pAddress->S_un.S_un_b.s_b1 ||						// Don't add invalid address
			 Network.IsFirewalledAddress( pAddress, TRUE ) ||		// Don't add own firewalled IPs
			 Network.IsReserved( pAddress ) ||						// Check against IANA reserved address
			 Security.IsDenied( pAddress ) ||						// Check security settings, don't add blocked IPs
			 Security.IsFlood( pAddress, pszVendor, m_nProtocol ) )	// Check unwanted flood regions
			return NULL;	// Bad IP
	}

	CQuickLock oLock( m_pSection );

	// Check if we already have the host
	CHostCacheHostPtr pHost = Find( pAddress );
	if ( ! pHost && szAddress )
		pHost = Find( szAddress );
	if ( ! pHost )
	{
		// Create new host
		pHost = new CHostCacheHost( m_nProtocol );
		if ( pHost )
		{
			PruneHosts();

			pHost->m_pAddress = *pAddress;
			if ( szAddress ) pHost->m_sAddress = szAddress;
			pHost->m_sAddress = pHost->m_sAddress.SpanExcluding( L":" );

			pHost->Update( nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit );

			if ( ! pHost->m_pVendor && pHost->m_sName.IsEmpty() && pszVendor )
				pHost->m_sName = pszVendor;

			// Add host to map and index
			m_Hosts.insert( CHostCacheMapPair( pHost->m_pAddress, pHost ) );
			m_HostsTime.insert( pHost );

			m_nCookie++;
			if (pbAdded)
				*pbAdded = true;
		}
	}
	else
	{
		if ( szAddress ) pHost->m_sAddress = szAddress;
		pHost->m_sAddress = pHost->m_sAddress.SpanExcluding( L":" );

		Update( pHost, nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit );
	}

	ASSERT( m_Hosts.size() == m_HostsTime.size() );

	return pHost;
}

void CHostCacheList::Update(CHostCacheHostPtr pHost, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
{
	CQuickLock oLock( m_pSection );

	ASSERT( m_Hosts.size() == m_HostsTime.size() );

	// Update host
	if ( pHost->Update( nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit ) )
	{
		// Remove host from old and now invalid position
		m_HostsTime.erase( std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost ) );

		// Add host to new sorted position
		m_HostsTime.insert( pHost );

		ASSERT( m_Hosts.size() == m_HostsTime.size() );
	}

	m_nCookie++;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList host remove

CHostCacheMapItr CHostCacheList::Remove(CHostCacheHostPtr pHost)
{
	CQuickLock oLock( m_pSection );

	CHostCacheIterator j = std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost );
	if ( j == m_HostsTime.end() )
		return m_Hosts.end();	// Wrong cache
	m_HostsTime.erase( j );

	CHostCacheMapItr i = std::find_if( m_Hosts.begin(), m_Hosts.end(), std::bind2nd( is_host(), pHost ) );

	ASSERT( i != m_Hosts.end() );
	i = m_Hosts.erase( i );
	ASSERT( m_Hosts.size() == m_HostsTime.size() );

	delete pHost;
	m_nCookie++;

	return i;
}

CHostCacheMapItr CHostCacheList::Remove(const IN_ADDR* pAddress)
{
	CQuickLock oLock( m_pSection );

	CHostCacheMapItr i = m_Hosts.find( *pAddress );
	if ( i == m_Hosts.end() )
		return m_Hosts.end();	// Wrong cache/address

	return Remove( (*i).second );
}

void CHostCacheList::SanityCheck()
{
	CQuickLock oLock( m_pSection );

	for ( CHostCacheMapItr i = m_Hosts.begin(); i != m_Hosts.end(); )
	{
		CHostCacheHostPtr pHost = (*i).second;
		if ( Security.IsDenied( &pHost->m_pAddress ) ||
			( pHost->m_pVendor && Security.IsVendorBlocked( pHost->m_pVendor->m_sCode ) ) )
		//	|| Security.IsFlood( &pHost->m_pAddress, pHost->m_pVendor->m_sCode, pHost->m_nProtocol ) ) )	// Crash?
		{
			i = Remove( pHost );
			continue;
		}

		++i;
	}
}

void CHostCacheList::OnResolve(LPCTSTR szAddress, const IN_ADDR* pAddress, WORD nPort)
{
	if ( ! pAddress )
	{
		OnFailure( szAddress, false );
		return;
	}

	CQuickLock oLock( m_pSection );

	CHostCacheHostPtr pHost = Find( szAddress );

	if ( Network.IsFirewalledAddress( pAddress, TRUE ) ||	// Don't add own firewalled IPs
		 Network.IsReserved( pAddress ) ||				// Check against IANA Reserved address
		 Security.IsDenied( pAddress ) )				// Check security settings, don't add blocked IPs
	{
		if ( pHost )
			Remove( pHost );
		return;
	}

	if ( pHost )
	{
		// Remove from old place
		m_Hosts.erase( std::find_if( m_Hosts.begin(), m_Hosts.end(),
			std::bind2nd( is_host(), pHost ) ) );

		pHost->m_pAddress = *pAddress;
		pHost->m_nPort = nPort;
		pHost->m_sCountry = theApp.GetCountryCode( pHost->m_pAddress );

		// Add to new place
		m_Hosts.insert( CHostCacheMapPair( pHost->m_pAddress, pHost ) );

		m_nCookie++;

		ASSERT( m_Hosts.size() == m_HostsTime.size() );
	}
	else
	{
		// New host
		Add( pAddress, nPort, 0, 0, 0, 0, 0, szAddress );
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList failure processor

void CHostCacheList::OnFailure(const IN_ADDR* pAddress, WORD nPort, bool bRemove)
{
	CQuickLock oLock( m_pSection );

	CHostCacheHostPtr pHost = Find( pAddress );

	if ( pHost && ( ! nPort || pHost->m_nPort == nPort ) )
	{
		m_nCookie++;
		pHost->m_nFailures++;
		pHost->m_tFailure = static_cast< DWORD >( time( NULL ) );
		pHost->m_bCheckedLocally = TRUE;

		// Clear current IP address to re-resolve name later
		if ( ! pHost->m_sAddress.IsEmpty() )
			pHost->m_pAddress.s_addr = INADDR_ANY;

		if ( ! pHost->m_bPriority && ( bRemove || pHost->m_nFailures > Settings.Connection.FailureLimit ) )
			Remove( pHost );
	}
}

void CHostCacheList::OnFailure(LPCTSTR szAddress, bool bRemove)
{
	CQuickLock oLock( m_pSection );

	CHostCacheHostPtr pHost = Find( szAddress );

	if ( ! pHost )
		pHost = Add( szAddress );	// New host (for resolver)

	if ( pHost )
	{
		m_nCookie++;
		pHost->m_nFailures++;
		pHost->m_tFailure = static_cast< DWORD >( time( NULL ) );

		if ( ! pHost->m_bPriority && ( bRemove || pHost->m_nFailures > Settings.Connection.FailureLimit ) )
			Remove( pHost );
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList failure processor

CHostCacheHostPtr CHostCacheList::OnSuccess(const IN_ADDR* pAddress, WORD nPort, bool bUpdate)
{
	CQuickLock oLock( m_pSection );

	CHostCacheHostPtr pHost = Add( const_cast< IN_ADDR* >( pAddress ), nPort );
	if ( pHost && ( ! nPort || pHost->m_nPort == nPort ) )
	{
		m_nCookie++;
		pHost->m_tFailure = 0;
		pHost->m_nFailures = 0;
		pHost->m_bCheckedLocally = TRUE;
		if ( bUpdate )
			Update( pHost, nPort );
	}

	return pHost;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList query acknowledgment prune (G2)

//void CHostCacheList::PruneByQueryAck()
//{
//	CQuickLock oLock( m_pSection );
//	DWORD tNow = static_cast< DWORD >( time( NULL ) );
//	for ( CHostCacheMap::iterator i = m_Hosts.begin(); i != m_Hosts.end(); )
//	{
//		bool bRemoved = false;
//		CHostCacheHostPtr pHost = (*i).second;
//		if ( pHost->m_tAck && tNow - pHost->m_tAck > Settings.Gnutella2.QueryHostDeadline )
//		{
//			pHost->m_tAck = 0;
//			if ( pHost->m_nFailures++ > Settings.Connection.FailureLimit )
//			{
//				m_HostsTime.erase( std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost ) );
//				i = m_Hosts.erase( i );
//				delete pHost;
//				bRemoved = true;
//				m_nCookie++;
//			}
//		}
//		// Don't increment if host was removed
//		if ( ! bRemoved ) i++;
//	}
//}

//////////////////////////////////////////////////////////////////////
// CHostCacheList prune old hosts

void CHostCacheList::PruneOldHosts(DWORD tNow)
{
	CQuickLock oLock( m_pSection );

	for ( CHostCacheMapItr i = m_Hosts.begin(); i != m_Hosts.end(); )
	{
		CHostCacheHostPtr pHost = (*i).second;

		// Query acknowledgment prune (G2/DHT)
		switch ( pHost->m_nProtocol )
		{
		case PROTOCOL_G2:
			if ( pHost->m_tAck && tNow > pHost->m_tAck + Settings.Gnutella2.QueryHostDeadline )
			{
				pHost->m_tAck = 0;

				m_nCookie++;
				pHost->m_nFailures++;
			}
			break;

		case PROTOCOL_BT:
			if ( pHost->m_tAck && tNow > pHost->m_tAck + Settings.BitTorrent.QueryHostDeadline )
			{
				pHost->m_tAck = 0;

				m_nCookie++;
				pHost->m_nFailures++;
			}
			break;

		case PROTOCOL_ED2K:
			if ( pHost->m_tAck && tNow > pHost->m_tAck + Settings.Connection.TimeoutHandshake )
			{
				pHost->m_tAck = 0;
				pHost->m_tFailure = pHost->m_tStats;

				m_nCookie++;
				pHost->m_nFailures++;
			}
			break;

		//default:
		//	;
		}

		// Discard hosts after repeat failures
		if ( ! pHost->m_bPriority &&
			 ( pHost->m_nFailures > Settings.Connection.FailureLimit ||
			   pHost->IsExpired( tNow ) ) )
		{
			i = Remove( pHost );
		}
		else
			++i;
	}
}

//////////////////////////////////////////////////////////////////////
// Remove several oldest hosts

void CHostCacheList::PruneHosts()
{
	CQuickLock oLock( m_pSection );

	for ( CHostCacheIndex::iterator i = m_HostsTime.end();
		m_Hosts.size() > Settings.Gnutella.HostCacheSize && i != m_HostsTime.begin(); )
	{
		--i;
		CHostCacheHostPtr pHost = (*i);
		if ( ! pHost->m_bPriority )
		{
			i = m_HostsTime.erase( i );
			m_Hosts.erase( std::find_if( m_Hosts.begin(), m_Hosts.end(),
				std::bind2nd( is_host(), pHost ) ) );
			delete pHost;
			m_nCookie++;
		}
	}

	for ( CHostCacheIndex::iterator i = m_HostsTime.end();
		m_Hosts.size() > Settings.Gnutella.HostCacheSize && i != m_HostsTime.begin(); )
	{
		--i;
		CHostCacheHostPtr pHost = (*i);
		i = m_HostsTime.erase( i );
		m_Hosts.erase( std::find_if( m_Hosts.begin(), m_Hosts.end(),
			std::bind2nd( is_host(), pHost ) ) );
		delete pHost;
		m_nCookie++;
	}

	ASSERT( m_Hosts.size() == m_HostsTime.size() );
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList serialize

void CHostCacheList::Serialize(CArchive& ar, int nVersion)
{
	CQuickLock oLock( m_pSection );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( GetCount() );
		for ( CHostCacheMapItr i = m_Hosts.begin(); i != m_Hosts.end(); ++i )
		{
			CHostCacheHostPtr pHost = (*i).second;
			pHost->Serialize( ar, nVersion );
		}
	}
	else // Loading
	{
		DWORD_PTR nCount = ar.ReadCount();
		for ( DWORD_PTR nItem = 0; nItem < nCount; nItem++ )
		{
			CHostCacheHostPtr pHost = new CHostCacheHost( m_nProtocol );
			if ( pHost )
			{
				pHost->Serialize( ar, nVersion );
				if ( ! Security.IsDenied( &pHost->m_pAddress ) &&
					 ! Find( &pHost->m_pAddress ) &&
					 ! Find( pHost->m_sAddress ) )
				{
					m_Hosts.insert( CHostCacheMapPair( pHost->m_pAddress, pHost ) );
					m_HostsTime.insert( pHost );
				}
				else
				{
					// Remove bad or duplicated host
					delete pHost;
				}
			}
		}

		PruneHosts();

		m_nCookie++;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCache root import

int CHostCache::Import(LPCTSTR pszFile, BOOL bFreshOnly)
{
	const LPCTSTR szExt = PathFindExtension( pszFile );

	// Ignore old files (120 days)
	if ( bFreshOnly && ! IsFileNewerThan( pszFile, 120ull * 24 * 60 * 60 * 1000 ) )
		return 0;

	CFile pFile;
	if ( ! pFile.Open( pszFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
		return 0;

	int nImported = 0;

	if ( _tcsicmp( szExt, L".met" ) == 0 )
	{
		theApp.Message( MSG_NOTICE, L"Importing MET file: %s", pszFile );

		nImported = ImportMET( &pFile );
	}
	else if ( _tcsicmp( szExt, L".bz2" ) == 0 )		// hublist.xml.bz2
	{
		theApp.Message( MSG_NOTICE, L"Importing HubList file: %s", pszFile );

		nImported = ImportHubList( &pFile );
	}
	else if ( _tcsicmp( szExt, L".dat" ) == 0 )
	{
		theApp.Message( MSG_NOTICE, L"Importing Kademlia Nodes file: %s", pszFile );

		nImported = ImportNodes( &pFile );
	}
//	else if ( _tcsicmp( szExt, L".xml" ) == 0 || _tcsicmp( szExt, L".dat" ) == 0 ) 	// ToDo: G2/Gnutella import/export
//	{
//		theApp.Message( MSG_NOTICE, L"Importing cache file: %s", pszFile );
//
//		nImported = ImportCache( &pFile );
//	}

	Save();

	return nImported;
}

//int CHostCache::ImportCache(CFile* pFile)
//{
//	// ToDo: Import/Export G2/Gnutella .xml/.dat
//}

int CHostCache::ImportHubList(CFile* pFile)
{
	CBuffer pBuffer;
	if (!pBuffer.LoadFromBZipFile(*pFile, CBUFFER_UNBZIP_MAX))
		return 0; // Empty/oversized/read/decompress error

	CString strEncoding;
	augment::auto_ptr< CXMLElement > pHublist ( CXMLElement::FromString(
		pBuffer.ReadString( pBuffer.m_nLength ), TRUE, &strEncoding ) );
	if ( strEncoding.CompareNoCase( L"utf-8" ) == 0 )	// Reload as UTF-8
		pHublist.reset( CXMLElement::FromString(
			pBuffer.ReadString( pBuffer.m_nLength, CP_UTF8 ), TRUE ) );
	if ( ! pHublist.get() )
		return FALSE;	// XML decoding error

	if ( ! pHublist->IsNamed( L"Hublist" ) )
		return FALSE;	// Invalid XML file format

	CXMLElement* pHubs = pHublist->GetFirstElement();
	if ( ! pHubs || ! pHubs->IsNamed( L"Hubs" ) )
		return FALSE;	// Invalid XML file format

	int nHubs = 0;
	for ( POSITION pos = pHubs->GetElementIterator(); pos; )
	{
		CXMLElement* pHub = pHubs->GetNextElement( pos );
		if ( pHub->IsNamed( L"Hub" ) )
		{
			CString strAddress = pHub->GetAttributeValue( L"Address" );
			if ( _tcsnicmp( strAddress, L"dchub://", 8 ) == 0 )
				strAddress = strAddress.Mid( 8 );
			else if ( _tcsnicmp( strAddress, L"adc://", 6 ) == 0 )
				continue;	// Skip ADC-hubs
			else if ( _tcsnicmp( strAddress, L"adcs://", 7 ) == 0 )
				continue;	// Skip ADCS-hubs

			const int nUsers	= _tstoi( pHub->GetAttributeValue( L"Users" ) );
			const int nMaxusers	= _tstoi( pHub->GetAttributeValue( L"Maxusers" ) );

			CQuickLock oLock( DC.m_pSection );
			CHostCacheHostPtr pServer = DC.Add( NULL, protocolPorts[ PROTOCOL_DC ], 0,
				protocolNames[ PROTOCOL_DC ], 0, nUsers, nMaxusers, strAddress );
			if ( pServer )
			{
				pServer->m_sName = pHub->GetAttributeValue( L"Name" );
				pServer->m_sDescription = pHub->GetAttributeValue( L"Description" );
				nHubs++;
			}
		}
	}

	return nHubs;
}

int CHostCache::ImportMET(CFile* pFile)
{
	BYTE nVersion = 0;
	pFile->Read( &nVersion, sizeof( nVersion ) );
	if ( nVersion != 0xE0 &&
		 nVersion != ED2K_MET &&
		 nVersion != ED2K_MET_I64TAGS ) return 0;

	int nServers = 0;
	DWORD nCount = 0;

	pFile->Read( &nCount, sizeof( nCount ) );

	while ( nCount-- > 0 )
	{
		IN_ADDR pAddress;
		WORD nPort;
		DWORD nTags;

		if ( pFile->Read( &pAddress, sizeof( pAddress ) ) != sizeof( pAddress ) ) break;
		if ( pFile->Read( &nPort, sizeof( nPort ) ) != sizeof( nPort ) ) break;
		if ( pFile->Read( &nTags, sizeof( nTags ) ) != sizeof( nTags ) ) break;

		CQuickLock oLock( eDonkey.m_pSection );
		CHostCacheHostPtr pServer = eDonkey.Add( &pAddress, nPort );

		while ( nTags-- > 0 )
		{
			CEDTag pTag;
			if ( ! pTag.Read( pFile ) ) break;
			if ( pServer == NULL ) continue;

			if ( pTag.Check( ED2K_ST_SERVERNAME, ED2K_TAG_STRING ) )
				pServer->m_sName = pTag.m_sValue;
			else if ( pTag.Check( ED2K_ST_DESCRIPTION, ED2K_TAG_STRING ) )
				pServer->m_sDescription = pTag.m_sValue;
			else if ( pTag.Check( ED2K_ST_MAXUSERS, ED2K_TAG_INT ) )
				pServer->m_nUserLimit = (DWORD)pTag.m_nValue;
			else if ( pTag.Check( ED2K_ST_MAXFILES, ED2K_TAG_INT ) )
				pServer->m_nFileLimit = (DWORD)pTag.m_nValue;
			else if ( pTag.Check( ED2K_ST_UDPFLAGS, ED2K_TAG_INT ) )
				pServer->m_nUDPFlags = (DWORD)pTag.m_nValue;
		}

		nServers++;
	}

	return nServers;
}

int CHostCache::ImportNodes(CFile* pFile, CArray<CHostCacheHostPtr>* pAddedHosts, int* pnAddedHosts)
{
	if (!pFile)
		return 0;

	const ULONGLONG nLen64 = pFile->GetLength();
	if (nLen64 < 4 || nLen64 > KadNodesDatMaxFileBytes)
	{
		theApp.Message(MSG_NOTICE, L"Kad nodes.dat rejected (size %I64u)", nLen64);
		return 0;
	}

	const DWORD nLen = static_cast<DWORD>(nLen64);
	CAutoVectorPtr<BYTE> pBytes(new BYTE[nLen]);
	if (!pBytes)
		return 0;

	pFile->SeekToBegin();
	if (pFile->Read(pBytes, nLen) != nLen)
		return 0;

	uint8_t ownId[Hashes::Guid::byteCount];
	ZeroMemory(ownId, sizeof(ownId));
	{
		const Hashes::Guid oSelf = MyProfile.oGUID;
		CopyMemory(ownId, &oSelf[0], Hashes::Guid::byteCount);
	}

	KadNodesDatContact oParsed[KadNodesDatNormalImportCap];
	const KadNodesDatResult oResult = KadNodesDatParse(
	    pBytes, nLen, oParsed, KadNodesDatNormalImportCap, ownId);

	if (oResult.status != KadNodesDatStatus::Ok)
	{
		theApp.Message(MSG_NOTICE,
		               L"Kad nodes.dat parse failed (status %d, version %u, edition %u)",
		               static_cast<int>(oResult.status), oResult.fileVersion, oResult.edition);
		return 0;
	}

	int nServers = 0;
	CQuickLock oLock(Kademlia.m_pSection);
	for (uint32_t i = 0; i < oResult.acceptedCount; ++i)
	{
		const KadNodesDatContact& c = oParsed[i];
		IN_ADDR pAddress = {};
		pAddress.S_un.S_un_b.s_b1 = c.ip[0];
		pAddress.S_un.S_un_b.s_b2 = c.ip[1];
		pAddress.S_un.S_un_b.s_b3 = c.ip[2];
		pAddress.S_un.S_un_b.s_b4 = c.ip[3];

		Hashes::Guid oGUID;
		CopyMemory(&oGUID[0], c.id, Hashes::Guid::byteCount);
		oGUID.validate();

		bool bAdded = false;
		CHostCacheHostPtr pCache = Kademlia.Add(
		    &pAddress, c.tcpPort ? c.tcpPort : c.udpPort, 0, NULL, 0, 0, 0, NULL, &bAdded);
		if (pCache)
		{
			if (bAdded && pAddedHosts != NULL && pnAddedHosts != NULL)
				pAddedHosts->SetAt((*pnAddedHosts)++, pCache);
			pCache->m_oGUID = oGUID;
			pCache->m_sDescription = oGUID.toString();
			pCache->m_nUDPPort = c.udpPort;
			pCache->m_nKADVersion = c.contactVersion;
			++nServers;
		}
	}

	theApp.Message(MSG_DEBUG,
	               L"Kad nodes.dat imported %d of %u accepted contacts (file v%u%s)",
	               nServers, oResult.acceptedCount, oResult.fileVersion,
	               oResult.bootstrapEdition ? L", bootstrap edition" : L"");
	return nServers;
}

int CHostCache::ImportValidatedNodesDat(const BYTE* pData, DWORD nLength, KadBootstrapAcquireResult* pnFailureOut)
{
	if (pnFailureOut != NULL)
		*pnFailureOut = KadBootstrapAcquireParseFailed;

	if (pData == NULL || nLength == 0)
	{
		if (pnFailureOut != NULL)
			*pnFailureOut = KadBootstrapAcquireEmptyBody;
		return 0;
	}

	if (nLength > KadBootstrapHttpMaxBytes())
	{
		if (pnFailureOut != NULL)
			*pnFailureOut = KadBootstrapAcquireOversized;
		return 0;
	}

	KadNodesDatResult oParsed = {};
	const KadBootstrapAcquireResult nAcquire = KadBootstrapValidateDownloadedBody(pData, nLength, &oParsed);
	if (nAcquire != KadBootstrapAcquireOk)
	{
		if (pnFailureOut != NULL)
			*pnFailureOut = nAcquire;
		return 0;
	}

	const CString strFile = Settings.General.DataPath + L"nodes.dat";
	const CString strTemp = strFile + L".tmp";
	const CString strLkg = strFile + L".lkg";
	const bool bHadPriorNodesDat = (GetFileAttributes(strFile) != INVALID_FILE_ATTRIBUTES);

	CFile pOut;
	if (!pOut.Open(strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive))
	{
		DeleteFile(strTemp);
		if (pnFailureOut != NULL)
			*pnFailureOut = KadBootstrapAcquirePersistenceFailed;
		return 0;
	}

	try
	{
		pOut.Write(pData, nLength);
		pOut.Close();
	}
	catch (CException* pException)
	{
		pException->Delete();
		pOut.Abort();
		DeleteFile(strTemp);
		if (pnFailureOut != NULL)
			*pnFailureOut = KadBootstrapAcquirePersistenceFailed;
		return 0;
	}

	if (bHadPriorNodesDat)
	{
		if (!CopyFile(strFile, strLkg, FALSE))
		{
			DeleteFile(strTemp);
			if (pnFailureOut != NULL)
				*pnFailureOut = KadBootstrapAcquirePersistenceFailed;
			return 0;
		}
	}

	if (!MoveFileEx(strTemp, strFile, MOVEFILE_REPLACE_EXISTING))
	{
		DeleteFile(strTemp);
		if (pnFailureOut != NULL)
			*pnFailureOut = KadBootstrapAcquirePersistenceFailed;
		return 0;
	}

	CArray<CHostCacheHostPtr> oAddedHosts;
	oAddedHosts.SetSize(KadNodesDatNormalImportCap);
	int nAddedHosts = 0;
	int nImported = 0;
	try
	{
		CMemFile pMem;
		pMem.Write(pData, nLength);
		pMem.Seek(0, CFile::begin);
		nImported = ImportNodes(&pMem, &oAddedHosts, &nAddedHosts);
	}
	catch (CException* pException)
	{
		pException->Delete();
		while (nAddedHosts > 0)
			Kademlia.Remove(oAddedHosts[--nAddedHosts]);
		if (KadBootstrapRestoreNodesDatAfterRejectedImport(bHadPriorNodesDat, strFile, strLkg))
		{
			if (pnFailureOut != NULL)
				*pnFailureOut = KadBootstrapAcquirePersistenceFailed;
		}
		else if (pnFailureOut != NULL)
			*pnFailureOut = KadBootstrapAcquirePersistenceFailed;
		return 0;
	}

	if (nImported <= 0)
	{
		while (nAddedHosts > 0)
			Kademlia.Remove(oAddedHosts[--nAddedHosts]);
		if (KadBootstrapRestoreNodesDatAfterRejectedImport(bHadPriorNodesDat, strFile, strLkg))
		{
			if (pnFailureOut != NULL)
				*pnFailureOut = KadBootstrapAcquireNoAcceptedContacts;
		}
		else if (pnFailureOut != NULL)
			*pnFailureOut = KadBootstrapAcquirePersistenceFailed;
		return 0;
	}

	return nImported;
}

bool CHostCache::EnoughServers(PROTOCOLID nProtocol) const
{
	switch ( nProtocol )
	{
	case PROTOCOL_G1:
		return ! Settings.Gnutella1.Enabled || Gnutella1.CountHosts( TRUE ) > 20;
	case PROTOCOL_G2:
		return ! Settings.Gnutella2.Enabled || Gnutella2.CountHosts( TRUE ) > 25;
	case PROTOCOL_ED2K:
		return ! Settings.eDonkey.Enabled || eDonkey.CountHosts( TRUE ) > 0;
	case PROTOCOL_DC:
		return ! Settings.DC.Enabled || DC.CountHosts( TRUE ) > 0;
	case PROTOCOL_BT:
		return ! Settings.BitTorrent.Enabled || BitTorrent.CountHosts( TRUE ) > 0;
	case PROTOCOL_KAD:
		return ! Settings.eDonkey.EnableKad || Kademlia.CountHosts(TRUE) > 0;
	default:
		return true;	// ( ForProtocol( nProtocol )->CountHosts( TRUE ) > 0 );
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCache Check Minimum Servers

bool CHostCache::CheckMinimumServers(PROTOCOLID nProtocol)
{
	if ( nProtocol != PROTOCOL_G2 && Settings.Experimental.LAN_Mode )
		return true;

	if ( EnoughServers( nProtocol ) )
		return true;

	// Load default server list (if necessary)
	LoadDefaultServers( nProtocol );

	if (nProtocol == PROTOCOL_KAD)
	{
		Import(Settings.General.DataPath + L"nodes.dat", FALSE);

		const static LPCTSTR sNodesDatPaths[4] = {
			{ L"\\eMule\\config\\nodes.dat" },
			{ L"\\eMule\\nodes.dat" },
			{ L"\\aMule\\config\\nodes.dat" },
			{ L"\\aMule\\nodes.dat" }
		};
		CString strKadRoots[4] = {
			theApp.GetProgramFilesFolder(),
			theApp.GetProgramFilesFolder64(),
			theApp.GetLocalAppDataFolder(),
			theApp.GetAppDataFolder()
		};
		for (int i = 0; i < _countof(strKadRoots); ++i)
		{
			for (int j = 0; j < _countof(sNodesDatPaths); ++j)
				Import(strKadRoots[i] + sNodesDatPaths[j], TRUE);
		}
	}

	// Get the server list from local eMule/mods if possible
	if ( nProtocol == PROTOCOL_ED2K )
	{
		const static LPCTSTR sServerMetPaths[ 4 ] =
		{
			{ L"\\eMule\\config\\server.met" },
			{ L"\\eMule\\server.met" },
			{ L"\\aMule\\config\\server.met" },
			{ L"\\aMule\\server.met" }
		//	{ L"\\iMule\\config\\server.met" },
		//	{ L"\\iMule\\server.met" },
		//	{ L"\\hebMule\\config\\server.met" },
		//	{ L"\\hebMule\\server.met" },
		//	{ L"\\Neo Mule\\config\\server.met" },
		//	{ L"\\Neo Mule\\server.met" }
		};

		CString strRootPaths[ 4 ] =
		{
			theApp.GetProgramFilesFolder(),
			theApp.GetProgramFilesFolder64(),
			theApp.GetLocalAppDataFolder(),
			theApp.GetAppDataFolder()
		};

		for ( int i = 0; i < _countof( strRootPaths ); ++i )
			for ( int j = 0; j < _countof( sServerMetPaths ); ++j )
				Import( strRootPaths[ i ] + sServerMetPaths[ j ], TRUE );
	}

	// ToDo: Try local Shareaza Servers.dat too

	if (EnoughServers(nProtocol))
		return true;

	if (nProtocol == PROTOCOL_KAD)
	{
		if (Settings.eDonkey.EnableKad && KadBootstrapColdStartMayRequest(GetTickCount()))
			DiscoveryServices.Execute(PROTOCOL_KAD, 1);
		return EnoughServers(nProtocol);
	}

	if ( EnoughServers( nProtocol ) )
		return true;

	// Get server list from Web
	DiscoveryServices.Execute( nProtocol, TRUE );

	return false;
}

//////////////////////////////////////////////////////////////////////
// CHostCache Default servers import

int CHostCache::LoadDefaultServers(PROTOCOLID nProtocol)
{
	const CString strWritablePath = Settings.General.DataPath + L"DefaultServers.dat";
	const int nBinaryDirectory = static_cast<int>(BootstrapModuleDirectoryLength(theApp.m_strBinaryPath));
	const CString strPackagedPath = nBinaryDirectory ? theApp.m_strBinaryPath.Left(nBinaryDirectory) + L"\\Data\\DefaultServers.dat" : CString();
	const wchar_t* pszCataloguePath = BootstrapSelectCataloguePath(
	    strWritablePath, ::PathFileExists(strWritablePath),
	    strPackagedPath, ::PathFileExists(strPackagedPath));
	CString strFile = pszCataloguePath ? pszCataloguePath : L"";
	int nServers = 0;

	// Ignore old files (300 days)
	//if ( ! IsFileNewerThan( strFile, 300ull * 24 * 60 * 60 * 1000 ) )
	//	return 0;

	CStdioFile pFile; // Load default list from writable or packaged data
	BOOL bOpened = !strFile.IsEmpty() && pFile.Open(strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan);
	if (!bOpened && strFile == strWritablePath && !strPackagedPath.IsEmpty() && ::PathFileExists(strPackagedPath)) // Unreadable writable copy
	{
		theApp.Message(MSG_ERROR, L"Writable default server catalogue could not be opened; using packaged data");
		strFile = strPackagedPath;
		bOpened = pFile.Open(strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan);
	}
	if (!bOpened)
	{
		static LONG s_nMissingCatalogueLogged = 0;
		if (InterlockedCompareExchange(&s_nMissingCatalogueLogged, 1, 0) == 0)
			theApp.Message(MSG_ERROR, L"No readable default server catalogue was found in writable or packaged data");
		return 0;
	}
	ULONGLONG nCatalogueLength = 0;
	try
	{
		nCatalogueLength = pFile.GetLength();
	}
	catch (CException* pException)
	{
		pFile.Abort();
		pException->Delete();
		theApp.Message(MSG_ERROR, L"Failed to inspect the default server catalogue");
		return 0;
	}
	if (nCatalogueLength > 1024 * 1024)
	{
		pFile.Close();
		theApp.Message(MSG_ERROR, L"Default server catalogue exceeds the 1 MiB limit");
		return 0;
	}

	theApp.Message(MSG_NOTICE, L"Loading default server catalogue from %s data", strFile == strWritablePath ? L"writable" : L"packaged");
	int nInvalidRows = 0;

	// Format: PE 255.255.255.255:1024	# NameForConvenience

	for ( ;; )
	{
		CString strLine;
		try
		{
			if (!pFile.ReadString(strLine))
				break; // End of file
		}
		catch (CException* pException)
		{
			pFile.Abort();
			pException->Delete();
			theApp.Message(MSG_ERROR, L"Failed while reading the default server catalogue");
			return nServers;
		}

		strLine.Trim( L" \t\r\n" );

		wchar_t cType = 0;
		bool bParsedPriority = false;
		const wchar_t* pszHost = nullptr;
		size_t nHost = 0;
		const BootstrapParseStatus nParse = BootstrapParseServerLine(
		    strLine, static_cast<size_t>(strLine.GetLength()),
		    &cType, &bParsedPriority, &pszHost, &nHost);
		if (nParse != BootstrapParseStatus::Ok)
		{
			if (nParse == BootstrapParseStatus::Invalid)
				++nInvalidRows;
			continue;
		}

		CHostCacheList* pCache = NULL;
		switch (BootstrapClassifyServerType(cType))
		{
		case BootstrapServerClass::Gnutella1:
			pCache = &Gnutella1;
			break;
		case BootstrapServerClass::Gnutella2:
			pCache = &Gnutella2;
			break;
		case BootstrapServerClass::Ed2k:
			pCache = &eDonkey;
			break;
		case BootstrapServerClass::Dc:
			pCache = &DC;
			break;
		case BootstrapServerClass::BitTorrent:
			pCache = &BitTorrent;
			break;
		case BootstrapServerClass::KadNode:
			pCache = &Kademlia;
			break;
		default:
			break;
		}

		if ( ! pCache )
			continue;	// Unknown protocol?

		if ( nProtocol != pCache->m_nProtocol && nProtocol != PROTOCOL_ANY )
			continue;	// Unneeded protocol

		const CString strHost(pszHost, static_cast<int>(nHost));
		const BOOL bPriority = bParsedPriority ? TRUE : FALSE;

		CQuickLock oLock( pCache->m_pSection );
		if (CHostCacheHostPtr pServer = pCache->Add(strHost))
		{
			pServer->m_bPriority = bPriority;
			nServers++;
		}

	//	int nIP[5];
	//	nTest =_stscanf( strServer, L"%i.%i.%i.%i:%i", &nIP[0], &nIP[1], &nIP[2], &nIP[3], &nIP[4] );
	//	if ( nTest < 4 || nIP[0] > 255 || nIP[1] > 255 || nIP[2] > 255 || nIP[3] > 255 )
	//		continue;	// Invalid
	//
	//	IN_ADDR pAddress;
	//	pAddress.S_un.S_un_b.s_b1 = (BYTE)nIP[0];
	//	pAddress.S_un.S_un_b.s_b2 = (BYTE)nIP[1];
	//	pAddress.S_un.S_un_b.s_b3 = (BYTE)nIP[2];
	//	pAddress.S_un.S_un_b.s_b4 = (BYTE)nIP[3];
	//
	//	if ( CHostCacheHostPtr pServer = pCache->Add( &pAddress, (WORD)nIP[4] ) )
	//	{
	//		pServer->m_bPriority = bPriority;
	//		nServers++;
	//	}
	}
	pFile.Close();
	if (nInvalidRows)
		theApp.Message(MSG_ERROR, L"Skipped %d invalid row(s) in the default server catalogue", nInvalidRows);

	return nServers;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost construction

CHostCacheHost::CHostCacheHost(PROTOCOLID nProtocol)
    : m_nProtocol(nProtocol)
    , m_nPort(0)
    , m_nUDPPort(0)
    , m_pVendor(NULL)
    , m_bPriority(FALSE)
    , m_nUserCount(0)
    , m_nUserLimit(0)
    , m_nFileLimit(0)
    , m_nTCPFlags(0)
    , m_nUDPFlags(0)
    , m_tAdded(GetTickCount())
    , m_tSeen(0)
    , m_tRetryAfter(0)
    , m_tConnect(0)
    , m_tQuery(0)
    , m_tAck(0)
    , m_tStats(0)
    , m_tFailure(0)
    , m_nFailures(0)
    , m_nDailyUptime(0)
    , m_tKeyTime(0)
    , m_nKeyValue(0)
    , m_nKeyHost(0)
    , m_bCheckedLocally(FALSE)
    //	, m_bDHT		( FALSE )	// Attributes: DHT (Unused)
    , m_nKADVersion(0) // Attributes: Kademlia
    , m_nCodePage(0)   // NMDC: 0 = inherit Settings.DC.CodePage
{
	m_pAddress.s_addr = INADDR_ANY;

	// 20sec cooldown to avoid neighbor add-remove oscillation
	const DWORD tNow = static_cast< DWORD >( time( NULL ) );
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
	case PROTOCOL_G2:
		m_tConnect = tNow - Settings.Gnutella.ConnectThrottle + 20;
		break;
	case PROTOCOL_ED2K:
		m_tConnect = tNow - Settings.eDonkey.QueryThrottle + 20;
		break;
	//default:
	//	break;
	}
}

DWORD CHostCacheHost::Seen() const
{
	return m_tSeen;
}

CString CHostCacheHost::Address() const
{
	if ( m_pAddress.s_addr != INADDR_ANY )
		return CString( inet_ntoa( m_pAddress ) );

	return m_sAddress;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost serialize

void CHostCacheHost::Serialize(CArchive& ar, int nVersion) // HOSTCACHE_SER_VER
{
	if ( ar.IsStoring() )
	{
		ar.Write( &m_pAddress, sizeof( m_pAddress ) );
		ar << m_nPort;

		ar << m_tAdded;
		ar << m_tSeen;
		ar << m_tRetryAfter;

		if ( m_pVendor != NULL && m_pVendor->m_sCode.GetLength() == 4 )
		{
			ar << (CHAR)m_pVendor->m_sCode.GetAt( 0 );
			ar << (CHAR)m_pVendor->m_sCode.GetAt( 1 );
			ar << (CHAR)m_pVendor->m_sCode.GetAt( 2 );
			ar << (CHAR)m_pVendor->m_sCode.GetAt( 3 );
		}
		else
		{
			CHAR cZero = 0;
			ar << cZero;
		}

		ar << m_sName;
		if ( ! m_sName.IsEmpty() )
			ar << m_sDescription;

		ar << m_nUserCount;
		ar << m_nUserLimit;
		ar << m_bPriority;

		ar << m_nFileLimit;
		ar << m_nTCPFlags;
		ar << m_nUDPFlags;
		ar << m_tStats;

		ar << m_nKeyValue;
		if ( m_nKeyValue != 0 )
		{
			ar << m_tKeyTime;
			ar << m_nKeyHost;
		}

		ar << m_tFailure;
		ar << m_nFailures;
		ar << m_bCheckedLocally;
		ar << m_nDailyUptime;
		ar << m_sCountry;

	//	ar << m_bDHT;	// Unused
		ar.Write( &m_oBtGUID[0], m_oBtGUID.byteCount );

		ar << m_nUDPPort;
		ar.Write( &m_oGUID[0], m_oGUID.byteCount );

		ar << m_tConnect;

		ar << m_sUser;
		ar << m_sPass;

		ar << m_sAddress;

		if ( m_nProtocol == PROTOCOL_KAD )
			ar << m_nKADVersion;

		if (nVersion >= 2)
			ar << m_nCodePage;
	}
	else // Loading
	{
		const DWORD tNow = static_cast< DWORD >( time( NULL ) );

		ReadArchive( ar, &m_pAddress, sizeof( m_pAddress ) );
		ar >> m_nPort;

		ar >> m_tAdded;

		ar >> m_tSeen;
		if ( m_tSeen > tNow )
			m_tSeen = tNow;

		ar >> m_tRetryAfter;

		CHAR szaVendor[4] = { 0, 0, 0, 0 };
		ar >> szaVendor[0];

		if ( szaVendor[0] )
		{
			ReadArchive( ar, szaVendor + 1, 3 );
			TCHAR szVendor[5] = { szaVendor[0], szaVendor[1], szaVendor[2], szaVendor[3], 0 };
			m_pVendor = VendorCache.Lookup( szVendor );
		}

		ar >> m_sName;
		if ( ! m_sName.IsEmpty() )
			ar >> m_sDescription;

		ar >> m_nUserCount;
		ar >> m_nUserLimit;
		ar >> m_bPriority;

		ar >> m_nFileLimit;
		ar >> m_nTCPFlags;
		ar >> m_nUDPFlags;
		ar >> m_tStats;

		ar >> m_nKeyValue;
		if ( m_nKeyValue != 0 )
		{
			ar >> m_tKeyTime;
			ar >> m_nKeyHost;
		}

		ar >> m_tFailure;
		ar >> m_nFailures;

		ar >> m_bCheckedLocally;
		ar >> m_nDailyUptime;

		ar >> m_sCountry;

		//ar >> m_bDHT;	// Unused
		ReadArchive( ar, &m_oBtGUID[0], m_oBtGUID.byteCount );
		m_oBtGUID.validate();

		ar >> m_nUDPPort;
		ReadArchive( ar, &m_oGUID[0], m_oGUID.byteCount );
		m_oGUID.validate();

		ar >> m_tConnect;

		ar >> m_sUser;
		ar >> m_sPass;

		ar >> m_sAddress;

		if ( m_nProtocol == PROTOCOL_KAD )
			ar >> m_nKADVersion;

		// Ser v2+ stores m_nCodePage. Legacy archive 1000 predates the field
		// (nVersion 1000 >= 2 numerically) — never read it from that format.
		if (nVersion >= 2 && nVersion != 1000)
			ar >> m_nCodePage;
		else
			m_nCodePage = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost update

bool CHostCacheHost::Update(WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
{
	bool bChanged = FALSE;

	if ( nPort )
		m_nUDPPort = m_nPort = nPort;

	if ( m_nProtocol == PROTOCOL_ED2K && nPort )
		m_nUDPPort += 4;

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );
	if ( ! tSeen || tSeen > tNow )
		tSeen = tNow;

	if ( m_tSeen < tSeen )
	{
		m_tSeen = tSeen;
		bChanged = true;
	}

	if ( nUptime )
		m_nDailyUptime = nUptime;

	if ( nCurrentLeaves )
		m_nUserCount = nCurrentLeaves;

	if ( nLeafLimit )
		m_nUserLimit = nLeafLimit;

	if ( pszVendor != NULL )
	{
		CString strVendorCode( pszVendor );
		strVendorCode.Trim();
		if ( ( m_pVendor == NULL || m_pVendor->m_sCode != strVendorCode ) && ! strVendorCode.IsEmpty() )
			m_pVendor = VendorCache.Lookup( (LPCTSTR)strVendorCode );
	}

	if ( m_sCountry.IsEmpty() )
		m_sCountry = theApp.GetCountryCode( m_pAddress );

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost connection setup

bool CHostCacheHost::ConnectTo(BOOL bAutomatic)
{
	m_tConnect = static_cast< DWORD >( time( NULL ) );

	if ( m_pAddress.s_addr != INADDR_ANY )
		return Neighbours.ConnectTo( m_pAddress, m_nPort, m_nProtocol, bAutomatic ) != NULL;

	m_tConnect += 30;	// Throttle for 30 seconds
	return Network.ConnectTo( m_sAddress, m_nPort, m_nProtocol ) != FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost string

CString CHostCacheHost::ToString(const bool bLong /*true*/) const
{
	CString str;

	if ( bLong )
	{
		time_t tSeen = m_tSeen;
		tm time = {};
		if ( gmtime_s( &time, &tSeen ) == 0 )
			str.Format( L"%s:%i %.4i-%.2i-%.2iT%.2i:%.2iZ",
				(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
				time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
				time.tm_hour, time.tm_min );	// 2002-04-30T08:30Z
	}
	else
	{
		str.Format( L"%s:%i",
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort );
	}

	return str;
}

bool CHostCacheHost::IsExpired(const DWORD tNow) const
{
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		return m_tSeen && ( tNow > m_tSeen + Settings.Gnutella1.HostExpire );
	case PROTOCOL_G2:
		return m_tSeen && ( tNow > m_tSeen + Settings.Gnutella2.HostExpire );
	case PROTOCOL_BT:
		return m_tSeen && ( tNow > m_tSeen + Settings.BitTorrent.HostExpire );
	case PROTOCOL_KAD:
		return m_tSeen && ( tNow > m_tSeen + 24 * 60 * 60 );	// ToDo: Add Kademlia setting
	default:	// Never PROTOCOL_ED2K PROTOCOL_DC
		return false;
	}
}

bool CHostCacheHost::IsThrottled(const DWORD tNow) const
{
	// Don't overload network name resolver
	if ( m_pAddress.s_addr == INADDR_ANY && Network.GetResolveCount() > 3 )
		return true;

	if ( Settings.Connection.ConnectThrottle &&			// 0 default, ~250ms if limited
		 tNow < m_tConnect + Settings.Connection.ConnectThrottle / 1000 )
		return true;

	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
	case PROTOCOL_G2:
		return ( tNow < m_tConnect + Settings.Gnutella.ConnectThrottle );
	case PROTOCOL_ED2K:
		return ( tNow < m_tConnect + Settings.eDonkey.QueryThrottle );
	case PROTOCOL_BT:
		return ( tNow < m_tConnect + Settings.BitTorrent.ConnectThrottle );
	default:
		return false;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost connection test

bool CHostCacheHost::CanConnect(const DWORD tNow) const
{
	// Don't connect to self
	if ( Settings.Connection.IgnoreOwnIP && Network.IsSelfIP( m_pAddress ) ) return false;

	return
		( ! m_tFailure ||											// Let failed host rest some time
		( tNow > m_tFailure + Settings.Connection.FailurePenalty ) ) &&
		( m_nFailures <= Settings.Connection.FailureLimit ) &&		// and we haven't lost hope on this host
		( m_bPriority || ! IsExpired( tNow ) ) &&					// and host isn't expired
		( ! IsThrottled( tNow ) );									// and don't reconnect too fast
																	// now we can connect.
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost quote test

bool CHostCacheHost::CanQuote(const DWORD tNow) const
{
	// If a host isn't dead and isn't expired, we can tell others about it
	return ( m_nFailures == 0 ) && ( ! IsExpired( tNow ) );
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost query test

// Can we UDP query this host? (G2/ED2K/KAD/BT)
bool CHostCacheHost::CanQuery(const DWORD tNow) const
{
	// Retry After check, for all
	if ( m_tRetryAfter != 0 && tNow < m_tRetryAfter ) return false;

	switch ( m_nProtocol )
	{
	case PROTOCOL_G2:
		// Must support G2
		if ( ! Network.IsConnected() || ! Settings.Gnutella2.Enabled ) return false;

		// Must not be waiting for an ack
		if ( m_tAck == 0 ) return false;

		// Must be a recently seen (current) host
		if ( tNow > m_tSeen + Settings.Gnutella2.HostCurrent ) return false;

		// If haven't queried yet, its ok
		if ( m_tQuery == 0 ) return true;

		// Don't query too fast
		return tNow > m_tQuery + Settings.Gnutella2.QueryThrottle;

	case PROTOCOL_ED2K:
		// Must support ED2K
		if ( ! Network.IsConnected() || ! Settings.eDonkey.Enabled ) return false;
		if ( ! Settings.eDonkey.ServerWalk ) return false;

		// Must not be waiting for an ack
		if ( 0 != m_tAck ) return false;

		// If haven't queried yet, its ok
		if ( m_tQuery == 0 ) return true;

		// Don't query too fast
		return tNow > m_tQuery + Settings.eDonkey.QueryThrottle;

	case PROTOCOL_BT:
		// Must support BT
		if ( ! Settings.BitTorrent.Enabled ) return false;

		// If haven't queried yet, its ok
		if ( m_tQuery == 0 ) return true;

		// Don't query too fast
		return tNow > m_tQuery + 90u;

	case PROTOCOL_DC:
		if ( ! Network.IsConnected() ) return false;
		return true;

	case PROTOCOL_KAD:
		if ( ! Network.IsConnected() ) return false;

		// If haven't queried yet, its ok
		//if ( m_tQuery == 0 ) return true;
		return true;	// ToDo: Support KAD

	default:
		return false;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost query key submission

void CHostCacheHost::SetKey(const DWORD nKey, const IN_ADDR* pHost)
{
	if ( ! nKey ) return;

	m_tAck		= 0;
	m_nFailures	= 0;
	m_tFailure	= 0;
	m_nKeyValue	= nKey;
	m_tKeyTime	= static_cast< DWORD >( time( NULL ) );
	m_nKeyHost	= pHost ? pHost->S_un.S_addr : Network.m_pHost.sin_addr.S_un.S_addr;
}

//////////////////////////////////////////////////////////////////////
// CHostCache Test Kademlia Import

void CHostCache::TestKadImport()
{
	// Debug: Write to log immediately
	CString strDebugFile = Settings.General.DataPath + L"debug_kad_test.txt";
	CStdioFile debugFile;
	if (debugFile.Open(strDebugFile, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive)) {
		debugFile.WriteString(L"TestKadImport started\r\n");
		debugFile.Close();
	}

	// Create a test Kademlia nodes file
	const char* testFile = "test_kad_nodes.dat";
	FILE* file = fopen(testFile, "wb");

	if (!file) {
		theApp.Message(MSG_ERROR, L"Failed to create test file: %s", (LPCTSTR)CString(testFile));
		return;
	}

	// Write header for new format
	DWORD nCount = 0;  // 0 indicates new format
	DWORD nVersion = 1; // Version 1
	DWORD nNodes = 2;  // 2 test nodes

	fwrite(&nCount, sizeof(nCount), 1, file);
	fwrite(&nVersion, sizeof(nVersion), 1, file);
	fwrite(&nNodes, sizeof(nNodes), 1, file);

	// Node 1: Sample Kademlia node
	// GUID (16 bytes) - using a simple pattern
	BYTE guid1[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
					  0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
	// IP: 192.168.1.100 (network byte order: 0xC0A80164)
	DWORD ip1 = htonl(0xC0A80164); // 192.168.1.100 - convert to network byte order
	WORD udpPort1 = htons(4672);   // Default Kademlia UDP port
	WORD tcpPort1 = htons(4672);   // Default Kademlia TCP port
	BYTE kadVersion1 = 8;   // Sample Kademlia version

	fwrite(guid1, 16, 1, file);
	fwrite(&ip1, sizeof(ip1), 1, file);
	fwrite(&udpPort1, sizeof(udpPort1), 1, file);
	fwrite(&tcpPort1, sizeof(tcpPort1), 1, file);
	fwrite(&kadVersion1, sizeof(kadVersion1), 1, file);

	// Node 2: Another sample node
	BYTE guid2[16] = {0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE,
					  0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};
	DWORD ip2 = htonl(0xC0A80165); // 192.168.1.101
	WORD udpPort2 = htons(4673);
	WORD tcpPort2 = htons(4673);
	BYTE kadVersion2 = 9;

	fwrite(guid2, 16, 1, file);
	fwrite(&ip2, sizeof(ip2), 1, file);
	fwrite(&udpPort2, sizeof(udpPort2), 1, file);
	fwrite(&tcpPort2, sizeof(tcpPort2), 1, file);
	fwrite(&kadVersion2, sizeof(kadVersion2), 1, file);

	fclose(file);
	theApp.Message(MSG_NOTICE, L"Created test Kademlia nodes file: %s", (LPCTSTR)CString(testFile));

	// Now test the import
	int nImported = 0;
	int nKadHosts = 0;

	CFile pFile;
	if (pFile.Open(CString(testFile), CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan)) {
		try {
			nImported = ImportNodes(&pFile);
			pFile.Close();

			theApp.Message(MSG_NOTICE, L"Imported %d Kademlia nodes", nImported);

			// Check if nodes were added
			nKadHosts = Kademlia.GetCount();
			theApp.Message(MSG_NOTICE, L"Kademlia cache now has %d hosts", nKadHosts);

			// Show details of imported nodes
			if (nKadHosts > 0) {
				theApp.Message(MSG_NOTICE, L"Imported nodes:");
				for (CHostCacheIterator i = Kademlia.Begin(); i != Kademlia.End(); ++i) {
					CHostCacheHostPtr pHost = *i;
					if (pHost) {
						CString strDetails;
						strDetails.Format(L"  %s:%i (UDP:%i, KAD v%i)",
							(LPCTSTR)pHost->Address(), pHost->m_nPort,
							pHost->m_nUDPPort, (int)pHost->m_nKADVersion);
						theApp.Message(MSG_NOTICE, strDetails);
					}
				}
			}
		}
		catch (CException* pException) {
			pFile.Abort();
			pException->Delete();
			theApp.Message(MSG_ERROR, L"Import failed with exception");
		}
	} else {
		theApp.Message(MSG_ERROR, L"Failed to open test file for import");
	}

	// Clean up
	DeleteFile(CString(testFile));

	// Also write results to a log file for testing
	CString strLogFile = Settings.General.DataPath + L"kad_test_log.txt";
	CStdioFile logFile;
	if (logFile.Open(strLogFile, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive)) {
		CString strResults;
		strResults.Format(L"Kademlia Import Test Results:\r\nImported: %d nodes\r\nCache size: %d nodes\r\n",
			nImported, nKadHosts);
		logFile.WriteString(strResults);

		if (nKadHosts > 0) {
			logFile.WriteString(L"Imported nodes:\r\n");
			for (CHostCacheIterator i = Kademlia.Begin(); i != Kademlia.End(); ++i) {
				CHostCacheHostPtr pHost = *i;
				if (pHost) {
					CString strDetails;
					strDetails.Format(L"  %s:%i (UDP:%i, KAD v%i)\r\n",
						(LPCTSTR)pHost->Address(), pHost->m_nPort,
						pHost->m_nUDPPort, (int)pHost->m_nKADVersion);
					logFile.WriteString(strDetails);
				}
			}
		}

		logFile.Close();
		theApp.Message(MSG_NOTICE, L"Test results written to: %s", (LPCTSTR)strLogFile);
	}
}
