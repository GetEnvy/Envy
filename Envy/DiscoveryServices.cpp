//
// DiscoveryServices.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
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
#include "DiscoveryServices.h"
#include "Buffer.h"
#include "Network.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "GProfile.h"
#include "G2Packet.h"
#include "G1Packet.h"
#include "Packet.h"
#include "Datagrams.h"
#include "Kademlia.h"
#include "VendorCache.h"
#include "Security.h" // Vendors

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CDiscoveryServices DiscoveryServices;

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices construction

CDiscoveryServices::CDiscoveryServices()
	: m_pWebCache			( NULL )
	, m_nWebCache			( wcmHosts )
	, m_pSubmit				( NULL )
	, m_nLastQueryProtocol	( PROTOCOL_NULL )
	, m_nLastUpdateProtocol ( PROTOCOL_NULL )
	, m_bFirstTime			( TRUE )
	, m_tUpdated			( 0 )
	, m_tExecute			( 0 )
	, m_tQueried			( 0 )
//	, m_tMetQueried			( 0 )	// Using static
//	, m_tHubsQueried		( 0 )	// Using static
{
}

CDiscoveryServices::~CDiscoveryServices()
{
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices list access

POSITION CDiscoveryServices::GetIterator() const
{
	ASSUME_LOCK( Network.m_pSection );

	return m_pList.GetHeadPosition();
}

CDiscoveryService* CDiscoveryServices::GetNext(POSITION& pos) const
{
	ASSUME_LOCK( Network.m_pSection );

	return m_pList.GetNext( pos );
}

BOOL CDiscoveryServices::Check(CDiscoveryService* pService, CDiscoveryService::Type nType) const
{
	ASSUME_LOCK( Network.m_pSection );

	if ( pService == NULL ) return FALSE;
	if ( m_pList.Find( pService ) == NULL ) return FALSE;
	return ( nType == CDiscoveryService::dsNull ) || ( pService->m_nType == nType );
}

DWORD CDiscoveryServices::GetCount(int nType, PROTOCOLID nProtocol) const
{
	ASSUME_LOCK( Network.m_pSection );

	DWORD nCount = 0;
	CDiscoveryService* ptr;

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		ptr = m_pList.GetNext( pos );
		if ( ( nType == CDiscoveryService::dsNull ) || ( ptr->m_nType == nType ) )	// If we're counting all types, or it matches
		{
			if (  nProtocol == PROTOCOL_NULL ||									// If we're counting all protocols
				( nProtocol == PROTOCOL_G1   && ptr->m_bGnutella1 ) ||			// Or we're counting G1 and it matches
				( nProtocol == PROTOCOL_G2   && ptr->m_bGnutella2 ) ||			// Or we're counting G2 and it matches
				( nProtocol == PROTOCOL_ED2K && ptr->m_nType == CDiscoveryService::dsServerList ) || 	// Or we're counting ED2K
				( nProtocol == PROTOCOL_DC   && ptr->m_nType == CDiscoveryService::dsServerList ) )		// Or we're counting DC++
			{
				nCount++;
			}
		}
	}
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices list modification

BOOL CDiscoveryServices::Add(LPCTSTR pszAddress, int nType, PROTOCOLID nProtocol)
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	CString strAddress( pszAddress );

	// Trim any excess whitespace or garbage at the end -Sometimes get "//", "./", "./." etc. (Bad caches)
	bool bEndSlash = strAddress.GetAt( strAddress.GetLength() - 1 ) == L'/';
	strAddress.Trim();
	strAddress.TrimRight( L"./" );

	if ( bEndSlash )
		strAddress.Append( L"/" );

	if ( strAddress.GetLength() < 8 )
		return FALSE;	// Reject impossibly short services

	if ( GetByAddress( strAddress ) != NULL )
		return TRUE;	// Already in list

	CDiscoveryService* pService = NULL;

	switch ( nType )
	{
	case CDiscoveryService::dsWebCache:
		if ( CheckWebCacheValid( pszAddress ) )
			pService = new CDiscoveryService( CDiscoveryService::dsWebCache, strAddress, nProtocol == PROTOCOL_NULL ? PROTOCOL_ANY : nProtocol );
		break;

	case CDiscoveryService::dsServerList:
		if ( CheckWebCacheValid( pszAddress ) )
			pService = new CDiscoveryService( CDiscoveryService::dsServerList, strAddress,
				( nProtocol == PROTOCOL_DC || strAddress.Find( L"hublist", 6 ) > 6 || strAddress.Find( L".xml.bz2", 8 ) > 8 ) ? PROTOCOL_DC : PROTOCOL_ED2K );
		break;

	case CDiscoveryService::dsGnutella:
		if ( CheckWebCacheValid( pszAddress ) )
		{
			pService = new CDiscoveryService( CDiscoveryService::dsGnutella, strAddress );

			if ( _tcsnicmp( strAddress, L"uhc:", 4 ) == 0 )
			{
				pService->m_nProtocolID = nProtocol = PROTOCOL_G1;
				pService->m_nSubType = CDiscoveryService::dsGnutellaUDPHC;
			}
			else if ( _tcsnicmp( strAddress, L"ukhl:", 5 ) == 0 )
			{
				pService->m_nProtocolID = nProtocol = PROTOCOL_G2;
				pService->m_nSubType = CDiscoveryService::dsGnutella2UDPKHL;
			}
			else if ( _tcsnicmp( strAddress, L"gnutella2:host:", 15 ) == 0 ||
				 _tcsnicmp( strAddress, L"g2:host:", 8 ) == 0 )
			{
				pService->m_nProtocolID = nProtocol = PROTOCOL_G2;
				pService->m_nSubType = CDiscoveryService::dsGnutella2TCP;
			}
			else if ( _tcsnicmp( strAddress, L"gnutella1:host:", 15 ) == 0 ||
				 _tcsnicmp( strAddress, L"gnutella:host:", 14 ) == 0 )
			{
				pService->m_nProtocolID = nProtocol = PROTOCOL_G1;
				pService->m_nSubType = CDiscoveryService::dsGnutellaTCP;
			}
		}
		break;

	case CDiscoveryService::dsBlocked:
		pService = new CDiscoveryService( CDiscoveryService::dsBlocked, strAddress );
		for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
		{
			CDiscoveryService* pItem = m_pList.GetNext( pos );

			if ( _tcsistr( pItem->m_sAddress, pService->m_sAddress ) != NULL )
			{
				if ( pItem->m_nType != CDiscoveryService::dsBlocked )
				{
					pItem->m_nType = CDiscoveryService::dsBlocked;
					delete pService;
					return FALSE;
				}
			}
		}

		break;
	}

	if ( pService == NULL )
		return FALSE;

	// Set the appropriate protocol flags
	switch ( nProtocol )
	{
	case PROTOCOL_G2:
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = FALSE;
		break;
	case PROTOCOL_G1:
		pService->m_bGnutella2 = FALSE;
		pService->m_bGnutella1 = TRUE;
		break;
	case PROTOCOL_ED2K:
	case PROTOCOL_DC:
		pService->m_bGnutella2 = FALSE;
		pService->m_bGnutella1 = FALSE;
		break;
	default:
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = TRUE;
		break;
	}

	return Add( pService );
}

BOOL CDiscoveryServices::Add(CDiscoveryService* pService)
{
	if ( pService == NULL )
		return FALSE;	// Can't add a null

	// If it's a webcache with no protocols set, assume it's for both.
	if ( pService->m_nType == CDiscoveryService::dsWebCache &&
		 pService->m_bGnutella2 == FALSE &&
		 pService->m_bGnutella1 == FALSE )
	{
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = TRUE;
	}

	CQuickLock pLock( Network.m_pSection );

	// Stop if we already have enough caches
	if ( ( pService->m_bGnutella2 && ( GetCount( pService->m_nType, PROTOCOL_G2 ) >= Settings.Discovery.CacheCount ) ) ||
		 ( pService->m_bGnutella1 && ( GetCount( pService->m_nType, PROTOCOL_G1 ) >= Settings.Discovery.CacheCount ) ) )
	{
		// Check if the service is already in the list.
		if ( m_pList.Find( pService ) == NULL )
		{
			// It's a new service, but we don't want more. We should delete it.
			theApp.Message( MSG_DEBUG, L"[DiscoveryServices] Maximum discovery service count reached, %s not added", pService->m_sAddress );
			delete pService;
			return FALSE;
		}

		// We already had this service on the list. Do nothing.
		return TRUE;
	}

	// Add the service to the list if it's not there already
	if ( m_pList.Find( pService ) == NULL )
	{
		m_pList.AddTail( pService );
		MergeURLs();
	}

	return TRUE;
}

void CDiscoveryServices::Remove(CDiscoveryService* pService, BOOL bCheck)
{
	ASSUME_LOCK( Network.m_pSection );

	if ( POSITION pos = m_pList.Find( pService ) )
		m_pList.RemoveAt( pos );
	delete pService;

	if ( bCheck )
		CheckMinimumServices();
}

BOOL CDiscoveryServices::CheckWebCacheValid(LPCTSTR pszAddress)
{
	// Check it's long enough
	if ( _tcsclen( pszAddress ) < 12 )
		return FALSE;

	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	// Check it's not blocked
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

		if ( pService->m_nType == CDiscoveryService::dsBlocked )
		{
			if ( _tcsistr( pszAddress, pService->m_sAddress ) != NULL )
				return FALSE;
		}
	}

	// Check it has a valid protocol
	if ( _tcsnicmp( pszAddress, L"http://", 7 ) == 0 )
		pszAddress += 7;
	else if ( _tcsnicmp( pszAddress, L"https://", 8 ) == 0 )
		pszAddress += 8;
	else if ( _tcsnicmp( pszAddress, L"uhc:", 4 ) == 0 ||
			  _tcsnicmp( pszAddress, L"ukhl:", 5 ) == 0 ||
			  _tcsnicmp( pszAddress, L"gnutella:host:", 14 ) == 0 ||
			  _tcsnicmp( pszAddress, L"gnutella1:host:", 15 ) == 0 ||
			  _tcsnicmp( pszAddress, L"gnutella2:host:", 15 ) == 0 ||
			  _tcsnicmp( pszAddress, L"g2:host:", 8 ) == 0 )
		return TRUE;
	else
		return FALSE;

	// Scan through http, make sure there are some "." and "/" in there.
	pszAddress = _tcschr( pszAddress, '.' );
	if ( pszAddress == NULL ) return FALSE;

	pszAddress = _tcschr( pszAddress, '/' );
	if ( pszAddress == NULL ) return FALSE;

	return TRUE;	// Probably okay
}

BOOL CDiscoveryServices::CheckMinimumServices()
{
	// Add the default services if we don't have enough
	if ( ! EnoughServices() )
	{
		AddDefaults();
		return FALSE;
	}

	return TRUE;
}

//DWORD CDiscoveryServices::MetQueried() const
//{
//	return m_tMetQueried;	// Obsolete: Unused
//}

DWORD CDiscoveryServices::LastExecute() const
{
	return m_tExecute;
}

CDiscoveryService* CDiscoveryServices::GetByAddress(LPCTSTR pszAddress) const
{
	ASSUME_LOCK( Network.m_pSection );

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

		int nLen = pService->m_sAddress.GetLength();
		if ( nLen > 45 )
		{
			// If it's a long webcache address, ignore the last few characters when checking
			if ( _tcsnicmp( pService->m_sAddress, pszAddress, nLen - 2 ) == 0 )
				return pService;
		}
		else
		{
			if ( pService->m_sAddress.CompareNoCase( pszAddress ) == 0 )
				return pService;
		}
	}

	return NULL;
}

CDiscoveryService* CDiscoveryServices::GetByAddress(const IN_ADDR* pAddress, WORD nPort, CDiscoveryService::SubType nSubType )
{
	ASSUME_LOCK( Network.m_pSection );

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

		if ( pService->m_nSubType == nSubType &&
			 pService->m_pAddress.S_un.S_addr == pAddress->S_un.S_addr &&
			 pService->m_nPort == nPort )
			return pService;
	}

	return NULL;
}

void CDiscoveryServices::Clear()
{
	CQuickLock pLock( Network.m_pSection );

	Stop();

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		delete m_pList.GetNext( pos );
	}

	m_pList.RemoveAll();
}

void CDiscoveryServices::Stop()
{
	m_pRequest.Cancel();

	CloseThread();

	m_pRequest.Clear();
}


//////////////////////////////////////////////////////////////////////
// CDiscoveryServices load and save

BOOL CDiscoveryServices::Load()
{
	const CString strFile = Settings.General.DataPath + L"Discovery.dat";

	BOOL bSuccess = FALSE;

	// Load the services from disk
	CFile pFile;
	if ( pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load, 16384 );	// 16 KB buffer
			try
			{
				CQuickLock pLock( Network.m_pSection );

				Serialize( ar );
				ar.Close();

				bSuccess = TRUE;	// Success
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

		pFile.Close();
	}

	if ( ! bSuccess )
		theApp.Message( MSG_ERROR, L"[DiscoveryServices] Failed to load discovery service list: %s", strFile );

	// Check we have the minimum number of services (in case of file corruption, etc)
	if ( ! EnoughServices() )
	{
		AddDefaults();	// Re-add the default list
		Save();			// And save it
	}

	return bSuccess;
}

BOOL CDiscoveryServices::Save()
{
	const CString strFile = Settings.General.DataPath + L"Discovery.dat";
	const CString strTemp = Settings.General.DataPath + L"Discovery.tmp";

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save discovery service list: %s", strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store, 16384 );	// 16 KB buffer
		try
		{
			CQuickLock pLock( Network.m_pSection );

			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, L"Failed to save discovery service list: %s", strTemp );
			return FALSE;
		}
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save discovery service list: %s", strTemp );
		return FALSE;
	}

	pFile.Close();

	if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save discovery service list: %s", strFile );
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices serialize

// Set at INTERNAL_VERSION on change:
#define DISCOVERY_SER_VERSION 1

// nVersion History:
// 7 - Added m_nTotalHosts, m_nURLs, m_nTotalURLs and m_sPong (BeaconCache)
// 1000 - (7)
// 1 - (Envy 1.0)

void CDiscoveryServices::Serialize(CArchive& ar)
{
	ASSUME_LOCK( Network.m_pSection );

	int nVersion = DISCOVERY_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;

		//ar << m_tMetQueried;
		//ar << m_tHubsQueried;

		ar.WriteCount( GetCount() );

		for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
		{
			m_pList.GetNext( pos )->Serialize( ar, nVersion );
		}
	}
	else // Loading
	{
		Clear();

		ar >> nVersion;
		if ( nVersion > INTERNAL_VERSION && nVersion != 1000 )
			AfxThrowUserException();

		//ar >> m_tMetQueried;
		//ar >> m_tHubsQueried;

		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			augment::auto_ptr< CDiscoveryService > pService( new CDiscoveryService() );
			try
			{
				pService->Serialize( ar, nVersion );
				m_pList.AddTail( pService.release() );
			}
			catch ( CException* pException )
			{
				pException->Delete();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices Check we have the minimum number of services
// Returns TRUE if there are enough services, or FALSE if there are not.

BOOL CDiscoveryServices::EnoughServices() const
{
	int nWebCacheCount = 0, nServerMetCount = 0, nHubListCount = 0;	// Types of services
	int nG1Count = 0, nG2Count = 0;									// Protocols

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );
		if ( pService->m_nType == CDiscoveryService::dsWebCache )
		{
			nWebCacheCount++;

			if ( pService->m_bGnutella1 ) nG1Count++;
			if ( pService->m_bGnutella2 ) nG2Count++;
		}
		else if ( pService->m_nType == CDiscoveryService::dsGnutella )
		{
			if ( pService->m_nSubType == CDiscoveryService::dsGnutellaUDPHC )	nG1Count++;
			if ( pService->m_nSubType == CDiscoveryService::dsGnutella2UDPKHL )	nG2Count++;
		}
		else if ( pService->m_nType == CDiscoveryService::dsServerList )
		{
			if ( pService->m_nProtocolID == PROTOCOL_DC ) nHubListCount++;
			else /*if ( pService->m_nProtocolID == PROTOCOL_ED2K )*/ nServerMetCount++;
		}
	}

	return ( ( nWebCacheCount	>= 1 ) &&	// At least 1 webcache
			 ( nG2Count 		>= 3 ) &&	// At least 3 G2 services
			 ( nG1Count 		>= 2 || ! Settings.Gnutella1.ShowInterface ) &&	// At least 2 G1 services, if exposed
			 ( nServerMetCount	>= 2 || ! Settings.eDonkey.ShowInterface ) &&	// At least 2 server.met, if exposed
			 ( nHubListCount	>= 2 || ! Settings.DC.ShowInterface ) );		// At least 2 hublist, if exposed
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices defaults

void CDiscoveryServices::AddDefaults()
{
	CFile pFile;
	CString strFile = Settings.General.DataPath + L"DefaultServices.dat";

	if ( pFile.Open( strFile, CFile::modeRead ) )			// Load default list from file if possible
	{
		theApp.Message( MSG_NOTICE, L"Loading default discovery service list" );

		try
		{
			CString strService;
			CString strLine;
			CBuffer pBuffer;
			TCHAR cType;

			pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
			pBuffer.m_nLength = (DWORD)pFile.GetLength();
			pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
			pFile.Close();

			while ( pBuffer.ReadLine( strLine ) )
			{
				if ( strLine.GetLength() < 7 ) continue;									// Blank comment line

				cType = strLine.GetAt( 0 );
				strService = strLine.Right( strLine.GetLength() - 2 );

				switch ( cType )		// Protocol is ignored and later detected...
				{
				case '1': Add( strService, CDiscoveryService::dsWebCache, PROTOCOL_G1 );	// G1 service
					break;
				case '2': Add( strService, CDiscoveryService::dsWebCache, PROTOCOL_G2 );	// G2 service
					break;
				case 'M': Add( strService, CDiscoveryService::dsWebCache );					// Multinetwork service
					break;
				case 'D': Add( strService, CDiscoveryService::dsServerList, PROTOCOL_ED2K ); // eDonkey service
					break;
				case 'H': Add( strService, CDiscoveryService::dsServerList, PROTOCOL_DC );	// DC++ Hublist
					break;
				case 'U': Add( strService, CDiscoveryService::dsGnutella );					// Bootstrap and UDP Discovery Service
					break;
				case 'X': Add( strService, CDiscoveryService::dsBlocked );					// Blocked service
					break;
				case '#':																	// Comment line
					break;
				}
			}
		}
		catch ( CException* pException )
		{
			if ( pFile.m_hFile != CFile::hFileNull )
				pFile.Close();	// Close file if still open
			pException->Delete();
		}
	}

	// Obsolete: If file can't be used or didn't have enough services, drop back to the built-in list
	//if ( ! EnoughServices() )
	//{
	//	theApp.Message( MSG_ERROR, L"Default discovery service load failed" );
	//
	//	//CString strServices = L"\n";
	//	//for ( strServices += '\n' ; ! strServices.IsEmpty() ; )
	//	//{
	//	//	CString strService = strServices.SpanExcluding( L"\r\n" );
	//	//	strServices = strServices.Mid( strService.GetLength() + 1 );
	//	//	if ( strService.GetLength() > 12 )
	//	//	{
	//	//		if ( _tcsistr( strService, L"//server" ) != NULL )
	//	//			Add( strService, CDiscoveryService::dsServerList, PROTOCOL_ED2K );
	//	//		else if ( _tcsistr( strService, L"hublist" ) != NULL )
	//	//			Add( strService, CDiscoveryService::dsServerList, PROTOCOL_DC );
	//	//		else
	//	//			Add( strService, CDiscoveryService::dsWebCache );
	//	//	}
	//	//}
	//}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices Merge URLs

void CDiscoveryServices::MergeURLs()
{
	ASSUME_LOCK( Network.m_pSection );

	CArray< CDiscoveryService* > G1URLs, G2URLs, MultiURLs, OtherURLs;
//	theApp.Message( MSG_DEBUG, L"CDiscoveryServices::MergeURLs(): Checking the discovery service WebCache URLs" );

	// Building the arrays...
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );
		if ( pService->m_nType == CDiscoveryService::dsWebCache )
		{
			if ( pService->m_bGnutella1 && pService->m_bGnutella2 )
				MultiURLs.Add( pService );	// Multi-network array
			else if ( pService->m_bGnutella1 )
				G1URLs.Add( pService ); 	// Gnutella array
			else if ( pService->m_bGnutella2 )
				G2URLs.Add( pService ); 	// Gnutella2 array
		}
		else
		{
			OtherURLs.Add( pService );		// Ignore anything other than a GWC and pass off into the 'otherURLs array'.
		}
	}
	if ( ! MultiURLs.IsEmpty() )
	{
		for ( int index = 0 ; index < MultiURLs.GetCount() ; index++ )
		{
			for ( int dup_index = 0 ; dup_index < MultiURLs.GetCount() ; dup_index++ )
			{
				// Checking for identical duplicate.
				if ( MultiURLs.GetAt( index )->m_sAddress == MultiURLs.GetAt( dup_index )->m_sAddress && index != dup_index )
					MultiURLs.RemoveAt( dup_index );
					//theApp.Message( MSG_NOTICE, L"CDiscoveryServices::MergeURLs(): Removed %s from the multi list", (LPCTSTR)MultiURLs.GetAt( dup_index )->m_sAddress );
			}
			if ( ! G1URLs.IsEmpty() )
			{
				// Remove G1 service if it matches that of a multi.
				for ( int index2 = 0 ; index2 < G1URLs.GetCount() ; index2++ )
				{
					if ( MultiURLs.GetAt( index )->m_sAddress == G1URLs.GetAt( index2 )->m_sAddress )
						G1URLs.RemoveAt( index2 );
						//theApp.Message( MSG_NOTICE, L"CDiscoveryServices::MergeURLs(): Removed %s from the Gnutella list", (LPCTSTR)G1URLs.GetAt( index2 )->m_sAddress );
				}
			}
			if ( ! G2URLs.IsEmpty() )
			{
				// Remove G2 service if it matches that of a multi.
				for ( int index3 = 0 ; index3 < G2URLs.GetCount() ; index3++ )
				{
					if ( MultiURLs.GetAt( index )->m_sAddress == G2URLs.GetAt( index3 )->m_sAddress )
						G2URLs.RemoveAt( index3 );
						//theApp.Message( MSG_NOTICE, L"CDiscoveryServices::MergeURLs(): Removed %s from the Gnutella2 list", (LPCTSTR)G2URLs.GetAt( index3 )->m_sAddress );
				}
			}
		}
	}
	if ( ! G1URLs.IsEmpty() )
	{
		for ( int index4 = 0 ; index4 < G1URLs.GetCount() ; index4++ )
		{
			for ( int dup_index2 = 0 ; dup_index2 < G1URLs.GetCount() ; dup_index2++ )
			{
				// Checking for identical duplicate.
				if ( G1URLs.GetAt( index4 )->m_sAddress == G1URLs.GetAt( dup_index2 )->m_sAddress && index4 != dup_index2 )
					G1URLs.RemoveAt( dup_index2 );
					//theApp.Message( MSG_NOTICE, L"CDiscoveryServices::MergeURLs(): Removed %s from the Gnutella list", (LPCTSTR)G1URLs.GetAt( dup_index2 )->m_sAddress );
			}
			if ( ! G2URLs.IsEmpty() )
			{
				// If G1 and G2 of the same URL exist, drop one and upgrade the other to multi status.
				for ( int index5 = 0 ; index5 < G2URLs.GetCount() ; index5++ )
				{
					if ( G1URLs.GetAt( index4 )->m_sAddress == G2URLs.GetAt( index5 )->m_sAddress )
					{
						CDiscoveryService* pService = G1URLs[index4];
						pService->m_bGnutella2 = true;
						G1URLs[index4] = pService;
						G2URLs.RemoveAt( index5 );
						//theApp.Message( MSG_NOTICE, L"CDiscoveryServices::MergeURLs(): Merged %s into the multi list", (LPCTSTR)G2URLs.GetAt( index5 )->m_sAddress );
					}
				}
			}
		}
	}
	if ( ! G2URLs.IsEmpty() )
	{
		for ( int index6 = 0 ; index6 < G2URLs.GetCount() ; index6++ )
		{
			for ( int dup_index3 = 0 ; dup_index3 < G2URLs.GetCount() ; dup_index3++ )
			{
				// Checking for identical duplicate
				if ( G2URLs.GetAt( index6 )->m_sAddress == G2URLs.GetAt( dup_index3 )->m_sAddress && index6 != dup_index3 )
					G2URLs.RemoveAt( dup_index3 );
					//theApp.Message( MSG_NOTICE, L"CDiscoveryServices::MergeURLs(): Removed %s from the Gnutella2 list", (LPCTSTR)G2URLs.GetAt( dup_index3 )->m_sAddress );
			}
		}
	}
	if ( ! G1URLs.IsEmpty() || ! G2URLs.IsEmpty() || ! MultiURLs.IsEmpty() || ! OtherURLs.IsEmpty() )
	{
		// Updating the list...
		m_pList.RemoveAll();
		if ( ! G1URLs.IsEmpty() )
		{
			for ( int g1_index = 0 ; g1_index < G1URLs.GetCount() ; g1_index++ )
			{
				m_pList.AddTail( G1URLs.GetAt( g1_index ) );
			}
		}
		if ( ! G2URLs.IsEmpty() )
		{
			for ( int g2_index = 0 ; g2_index < G2URLs.GetCount() ; g2_index++ )
			{
				m_pList.AddTail( G2URLs.GetAt( g2_index ) );
			}
		}
		if ( ! MultiURLs.IsEmpty() )
		{
			for ( int multi_index = 0 ; multi_index < MultiURLs.GetCount() ; multi_index++ )
			{
				m_pList.AddTail( MultiURLs.GetAt( multi_index ) );
			}
		}
		if ( ! OtherURLs.IsEmpty() )
		{
			for ( int other_index = 0 ; other_index < OtherURLs.GetCount() ; other_index++ )
			{
				m_pList.AddTail( OtherURLs.GetAt( other_index ) );
			}
		}
		// The Discovery Service list is now rebuilt with the updated listing.
		theApp.Message( MSG_DEBUG, L"CDiscoveryServices::MergeURLs(): Completed Successfully" );
	}
	else
	{
		// The Discovery Service list is empty.
		theApp.Message( MSG_ERROR, L"CDiscoveryServices::MergeURLs(): detected an empty discovery service list" );
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices update

BOOL CDiscoveryServices::Update()
{
	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	// Don't update too frequently
	if ( tNow < m_tUpdated + Settings.Discovery.UpdatePeriod )
		return FALSE;

	// Don't run concurrent request
	if ( m_pRequest.IsPending() )
		return FALSE;

	Stop();

	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	// Must be up for 1.5 hours
	if ( Network.GetStableTime() < 5400 ) return FALSE;

	// Determine which network/protocol to update
	PROTOCOLID nProtocol;
	if ( Neighbours.IsG2Hub() )						// G2 hub mode is active
	{
		if ( Neighbours.IsG1Ultrapeer() )			// G2 and G1 are active
			nProtocol = ( m_nLastUpdateProtocol == PROTOCOL_G2 ) ? PROTOCOL_G1 : PROTOCOL_G2;	// Update the one we didn't update last time
		else										// Only G2 is active
			nProtocol = PROTOCOL_G2;
	}
	else if ( Neighbours.IsG1Ultrapeer() )			// Only G1 active
		nProtocol = PROTOCOL_G1;
	else											// No protocols active- no updates
		return FALSE;

	//*** ToDo: Ultrapeer mode hasn't been updated or tested in a very long time

	//ASSERT( nProtocol == PROTOCOL_G1 || nProtocol == PROTOCOL_G2 );

	// Must have at least 4 peers
	if ( Neighbours.GetCount( nProtocol, -1, ntNode ) < 4 && ! Settings.Experimental.LAN_Mode )
		return FALSE;

	// Select a random webcache of the appropriate sort
	CDiscoveryService* pService = GetRandomWebCache(nProtocol, TRUE, NULL, TRUE );
	if ( pService == NULL ) return FALSE;

	// Make the update request
	return RequestWebCache( pService, wcmUpdate, nProtocol );
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute

// This is called when trying to connect to a network, and at intervals while connected.
// Makes sure you have hosts available to connect to. Careful not to be agressive here.
// You should never query server.met files, because of the load it would create.
// This is public, and will be called quite regularly.

BOOL CDiscoveryServices::Execute(BOOL bDiscovery, PROTOCOLID nProtocol /*PROTOCOL_NULL*/, USHORT nForceDiscovery /*0*/)
{
	//	bDiscovery:
	//		TRUE	- Want Discovery(GEC, UHC, MET)
	//		FALSE	- Want Bootstrap connection.
	//	nProtocol:
	//		PROTOCOL_NULL	- Auto Detection
	//		PROTOCOL_G1		- Execute entry for G1
	//		PROTOCOL_G2		- Execute entry for G2
	//		PROTOCOL_ED2K	- Execute entry for ED2K
	//		PROTOCOL_DC 	- Execute entry for DC++ (Verify?)
	//	nForceDiscovery:
	//		FALSE - Normal discovery. There is a time limit and a check if it is needed
	//		1 - Forced discovery. Partial time limit and withOUT check if it is needed  ( Used inside CNeighboursWithConnect::Maintain() )
	//		2 - Unlimited discovery. No time limit but there is the check if it is needed  ( Only from QuickStart Wizard )

	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	if ( bDiscovery )	// If this is a user-initiated manual query, or AutoStart with Cache empty
	{
		if ( m_pRequest.IsPending() )
			return FALSE;

		const DWORD tNow = static_cast< DWORD >( time( NULL ) );
		if ( m_tExecute != 0 && tNow < m_tExecute + 5  && nForceDiscovery < 2 ) return FALSE;
		if ( m_tQueried != 0 && tNow < m_tQueried + 60 && nForceDiscovery == 0 ) return FALSE;
		if ( nForceDiscovery > 0 && nProtocol == PROTOCOL_NULL ) return FALSE;

		m_tExecute = tNow;

		static DWORD tMetQueried = 0;		// Was m_tMetQueried
		static DWORD tHubsQueried = 0;		// Was m_tHubsQueried

		BOOL bG2Required = FALSE;
		BOOL bG1Required = FALSE;
		BOOL bEdRequired = FALSE;
		BOOL bDCRequired = FALSE;

		if ( Settings.Experimental.LAN_Mode )
		{
			bG2Required =
				( nProtocol == PROTOCOL_NULL || nProtocol == PROTOCOL_G2 ) &&
				( nForceDiscovery == 1 || HostCache.Gnutella2.CountHosts(TRUE) < 1 );
		}
		else
		{
			bG2Required = Settings.Gnutella2.Enabled &&
				( nProtocol == PROTOCOL_NULL || nProtocol == PROTOCOL_G2 ) &&
				( nForceDiscovery == 1 || HostCache.Gnutella2.CountHosts(TRUE) < 25 );

			bG1Required = Settings.Gnutella1.Enabled &&
				( nProtocol == PROTOCOL_NULL || nProtocol == PROTOCOL_G1 ) &&
				( nForceDiscovery == 1 || HostCache.Gnutella1.CountHosts(TRUE) < 20 );

			bEdRequired = Settings.eDonkey.Enabled &&
				( nProtocol == PROTOCOL_NULL || nProtocol == PROTOCOL_ED2K ) &&
				Settings.eDonkey.MetAutoQuery &&
				( tMetQueried == 0 || tNow >= tMetQueried + 60 * 60 ) &&
				( nForceDiscovery == 1 || ! HostCache.EnoughServers( PROTOCOL_ED2K ) );

			bDCRequired = Settings.DC.Enabled &&
				( nProtocol == PROTOCOL_NULL || nProtocol == PROTOCOL_DC ) &&
				Settings.DC.HubListAutoQuery &&
				( tHubsQueried == 0 || tNow >= tHubsQueried + 60 * 60 ) &&
				( nForceDiscovery == 1 || HostCache.DC.CountHosts(TRUE) < 5 );	// || tNow >= m_tHubsQueried + 60 * 60 * 240 );		// 10 days  ToDo: Settings.DC.HubListQueryPeriod

			if ( bEdRequired )
				tMetQueried = tNow; 	// Execute this maximum one time each hour only when the number of eDonkey servers is too low (Very important).
			if ( bDCRequired )
				tHubsQueried = tNow; 	// Execute this maximum one time each hour only when the number of DC hubs is too low or outdated (Very important).
		}

		//pLock.Unlock();

		// Broadcast discovery
		static bool bBroadcast = true;		// test, broadcast, cache, broadcast, cache, ...
		bBroadcast = ! bBroadcast;
		if ( bBroadcast && bG2Required )
		{
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_QUERY, L"BROADCAST" );
			SOCKADDR_IN addr = { AF_INET, Network.m_pHost.sin_port };
			addr.sin_addr.S_un.S_addr = INADDR_NONE;
			return Datagrams.Send( &addr, CG2Packet::New( G2_PACKET_DISCOVERY ), TRUE, 0, FALSE );
		}

		if ( nProtocol == PROTOCOL_NULL )	// All hosts are wanted G2/G1/ED/DC
			return  ( ! bG1Required || RequestRandomService( PROTOCOL_G1 ) ) &&
					( ! bG2Required || RequestRandomService( PROTOCOL_G2 ) ) &&
					( ! bEdRequired || RequestRandomService( PROTOCOL_ED2K ) ) &&
					( ! bDCRequired || RequestRandomService( PROTOCOL_DC ) );
		if ( bG1Required )	// Only G1
			return RequestRandomService( PROTOCOL_G1 );
		if ( bG2Required )	// Only G2
			return RequestRandomService( PROTOCOL_G2 );
		if ( bEdRequired )	// Only Ed
			return RequestRandomService( PROTOCOL_ED2K );
		if ( bDCRequired )	// Only DC
			return RequestRandomService( PROTOCOL_DC );

		return TRUE;	// No Discovery needed
	}

	// TCP bootstraps
	theApp.Message( MSG_NOTICE, IDS_DISCOVERY_BOOTSTRAP );

	if ( Settings.Gnutella1.Enabled && Settings.Gnutella2.Enabled && nProtocol == PROTOCOL_NULL )
		ExecuteBootstraps( Settings.Discovery.BootstrapCount, FALSE, PROTOCOL_NULL );
	else if ( Settings.Gnutella2.Enabled && nProtocol == PROTOCOL_G2 )
		ExecuteBootstraps( Settings.Discovery.BootstrapCount, FALSE, PROTOCOL_G2 );
	else if ( Settings.Gnutella1.Enabled && nProtocol == PROTOCOL_G1 )
		ExecuteBootstraps( Settings.Discovery.BootstrapCount, FALSE, PROTOCOL_G1 );

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices resolve N gnutella bootstraps

int CDiscoveryServices::ExecuteBootstraps(int nCount, BOOL bUDP, PROTOCOLID nProtocol)
{
	if ( nCount < 1 ) return 0;

	CArray< CDiscoveryService* > pRandom;
	int nSuccess;
	BOOL bGnutella1 = FALSE;
	BOOL bGnutella2 = FALSE;

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		bGnutella1 = TRUE;
		bGnutella2 = TRUE;
		break;
	case PROTOCOL_G1:
		bGnutella1 = TRUE;
		break;
	case PROTOCOL_G2:
		bGnutella2 = TRUE;
		break;
	}

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );
		if ( pService->m_nType == CDiscoveryService::dsGnutella &&
			( ( bGnutella1 && bGnutella2 ) || ( bGnutella1 == pService->m_bGnutella1 && bGnutella2 == pService->m_bGnutella2 ) ) &&
			( ( ( pService->m_nSubType == CDiscoveryService::dsGnutellaUDPHC || pService->m_nSubType == CDiscoveryService::dsGnutella2UDPKHL ) && bUDP && ( time( NULL ) - pService->m_tAccessed >= 300 ) ) ||
			( ( pService->m_nSubType == CDiscoveryService::dsOldBootStrap || pService->m_nSubType == CDiscoveryService::dsGnutellaTCP || pService->m_nSubType == CDiscoveryService::dsGnutella2TCP ) && ! bUDP ) ) )
				pRandom.Add( pService );
	}

	for ( nSuccess = 0 ; nCount > 0 && pRandom.GetSize() > 0 ; )
	{
		INT_PTR nRandom = GetRandomNum< INT_PTR >( 0, pRandom.GetSize() - 1 );
		CDiscoveryService* pService = pRandom.GetAt( nRandom );
		pRandom.RemoveAt( nRandom );

		if ( pService->ResolveGnutella() )
		{
			++nSuccess;
			--nCount;
		}
	}

	return nSuccess;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices RequestRandomService

// Execute a random service (of any type) for any given protocol.
// Used to find more hosts (to connect to a network).

BOOL CDiscoveryServices::RequestRandomService(PROTOCOLID nProtocol)
{
	if ( CDiscoveryService* pService = GetRandomService( nProtocol ) )
	{
		if ( pService->m_nType == CDiscoveryService::dsGnutella )
			return pService->ResolveGnutella();

		if ( pService->m_nType == CDiscoveryService::dsServerList )
			return RequestWebCache( pService, wcmServerList, nProtocol );

		return RequestWebCache( pService, wcmHosts, nProtocol );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices GetRandomService

// Selects a random discovery service of any type for any given protocol.
// Called by RequestRandomService(), and used when finding more hosts (connecting to network).

CDiscoveryService* CDiscoveryServices::GetRandomService(PROTOCOLID nProtocol)
{
	ASSUME_LOCK( Network.m_pSection );

	CArray< CDiscoveryService* > pServices;
	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	// Loops through all services
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

		switch ( nProtocol )
		{
		case PROTOCOL_G1:
			if ( Settings.Discovery.EnableG1GWC && pService->m_bGnutella1 &&		// EnableG1GWC default true (was false)
				( pService->m_nType == CDiscoveryService::dsWebCache ) &&
				( tNow > pService->m_tAccessed + pService->m_nAccessPeriod ) )
				pServices.Add( pService );
			else if ( ( pService->m_nType == CDiscoveryService::dsGnutella ) &&
				( pService->m_nSubType == CDiscoveryService::dsGnutellaUDPHC ) &&
				time( NULL ) - pService->m_tAccessed >= 300 )
				pServices.Add( pService );
			break;
		case PROTOCOL_G2:
			if ( pService->m_bGnutella2 &&
				( pService->m_nType == CDiscoveryService::dsWebCache ) &&
				( tNow > pService->m_tAccessed + pService->m_nAccessPeriod ) )
				pServices.Add( pService );
			else if ( ( pService->m_nType == CDiscoveryService::dsGnutella ) &&
				( pService->m_nSubType == CDiscoveryService::dsGnutella2UDPKHL ) &&
				time( NULL ) - pService->m_tAccessed >= 300 )
				pServices.Add( pService );
			break;
		case PROTOCOL_ED2K:
		case PROTOCOL_DC:
			if ( pService->m_nType == CDiscoveryService::dsServerList &&
				( tNow > pService->m_tAccessed + pService->m_nAccessPeriod ) )
				pServices.Add( pService );
			break;
		//default:
		//	break;
		}
	}

	// Select a random service from the list of possible ones.
	if ( pServices.GetSize() > 0 )	// If the list of possible ones isn't empty
		return pServices.GetAt( GetRandomNum< INT_PTR >( 0, pServices.GetSize() - 1 ) );	// A random service

	return NULL;		// None available
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices select a random webcache (For updates, etc)

CDiscoveryService* CDiscoveryServices::GetRandomWebCache(PROTOCOLID nProtocol, BOOL bWorkingOnly, CDiscoveryService* pExclude, BOOL bForUpdate)
{
	ASSUME_LOCK( Network.m_pSection );

	// Select a random webcache for G2 (and rarely G1)
	CArray< CDiscoveryService* > pWebCaches;
	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

		if ( pService->m_nType != CDiscoveryService::dsWebCache || pService == pExclude )
			continue;

		if ( ! bWorkingOnly || ( pService->m_nAccesses > 0 && pService->m_nFailures == 0 && ( pService->m_nHosts > 1 || Neighbours.IsG2Hub() ) ) )
		{
			if ( tNow > pService->m_tAccessed + pService->m_nAccessPeriod )
			{
				if ( ! bForUpdate || tNow > pService->m_tUpdated + pService->m_nUpdatePeriod )
				{
					switch ( nProtocol )
					{
					case PROTOCOL_G1:
						if ( pService->m_nType == CDiscoveryService::dsWebCache && pService->m_bGnutella1 )
							pWebCaches.Add( pService );
						break;
					case PROTOCOL_G2:
						if ( pService->m_nType == CDiscoveryService::dsWebCache && pService->m_bGnutella2 )
							pWebCaches.Add( pService );
						break;
					default:
					//	theApp.Message( MSG_ERROR, L"CDiscoveryServices::GetRandomWebCache() was passed an invalid protocol" );
						ASSERT( FALSE );
						return NULL;
					}
				}
			}
		}
	}

	// If there are any available web caches
	if ( pWebCaches.GetSize() > 0 )
		return pWebCaches.GetAt( GetRandomNum< INT_PTR >( 0, pWebCaches.GetSize() - 1 ) );	// Select a random one

	return NULL;	// Indicate none available
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices webcache request control

BOOL CDiscoveryServices::RequestWebCache(CDiscoveryService* pService, Mode nMode, PROTOCOLID nProtocol)
{
	Stop();

	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	const DWORD tNow = (DWORD)time( NULL );
	DWORD nHosts = 0;

	switch ( nProtocol )
	{
	case PROTOCOL_G1:
		nHosts = HostCache.Gnutella1.GetCount();
		break;
	case PROTOCOL_G2:
		nHosts = HostCache.Gnutella2.GetCount();
		break;
	case PROTOCOL_ED2K:
		nHosts = HostCache.eDonkey.GetCount();
		break;
	case PROTOCOL_DC:
		nHosts = HostCache.DC.GetCount();
		break;
	default:
		return FALSE;
	}

	if ( pService != NULL && nHosts )
	{
		if ( time( NULL ) < pService->m_tAccessed + pService->m_nAccessPeriod )
			return FALSE;
	}

	m_pWebCache	= pService;
	m_nWebCache	= nMode;

	switch ( nMode )
	{
	case wcmHosts:
	case wcmCaches:
		if ( m_pWebCache != NULL )
		{
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_QUERY, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last queried' settings
			m_tQueried = tNow;
			m_nLastQueryProtocol = nProtocol;
		}
		break;

	case wcmUpdate:
		m_pSubmit = GetRandomWebCache( nProtocol, TRUE, m_pWebCache, FALSE );
		if ( m_pWebCache != NULL )
		{
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_UPDATING, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last updated' settings
			m_tUpdated = tNow;
			m_nLastUpdateProtocol = nProtocol;
		}
		break;

	case wcmSubmit:
		m_pSubmit = m_pWebCache;
		m_pWebCache = GetRandomWebCache( nProtocol, FALSE, m_pSubmit, TRUE );
		if ( m_pWebCache != NULL )
		{
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_SUBMIT, (LPCTSTR)m_pSubmit->m_sAddress );
			// Update the 'last updated' settings
			m_tUpdated = tNow;
			m_nLastUpdateProtocol = nProtocol;
		}
		break;

	case wcmServerList:
		if ( nProtocol != PROTOCOL_ED2K && nProtocol != PROTOCOL_DC )
		{
			ASSERT( FALSE );
			return FALSE;
		}
		if ( m_pWebCache != NULL )
		{
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_QUERY, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last queried' settings
			m_tQueried = tNow;
			m_nLastQueryProtocol = nProtocol;
		}
		break;

	default:
		ASSERT( FALSE );
		return FALSE;
	}

	if ( m_pWebCache == NULL )
		return FALSE;

	m_pRequest.Clear();

	return BeginThread( "Discovery" );
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices thread run

void CDiscoveryServices::OnRun()
{
	if ( m_pRequest.IsPending() )
		return;

	BOOL bSuccess = TRUE;

	switch ( m_nWebCache )
	{
	case wcmServerList:
		bSuccess = RunServerList();
		break;
	case wcmHosts:
		bSuccess = RunWebCacheGet( FALSE );
		{
			CSingleLock pLock( &Network.m_pSection, FALSE );
			if ( pLock.Lock( 250 ) && bSuccess )
			{
				if ( m_bFirstTime || ( GetCount( CDiscoveryService::dsWebCache ) < Settings.Discovery.Lowpoint ) )
				{
					m_bFirstTime = FALSE;
					pLock.Unlock();

					bSuccess = RunWebCacheGet( TRUE );
				}
			}
		}
		break;
	case wcmCaches:
		bSuccess = RunWebCacheGet( TRUE );
		break;
	case wcmUpdate:
		bSuccess = RunWebCacheUpdate();
		break;
	case wcmSubmit:
		bSuccess = RunWebCacheUpdate();
		break;
	}

	if ( ! bSuccess )
	{
		if ( m_nWebCache == wcmUpdate )
		{
			m_tUpdated = 0;
			m_nLastUpdateProtocol = PROTOCOL_NULL;
		}
		else
		{
			m_tQueried = 0;
			m_nLastQueryProtocol = PROTOCOL_NULL;
		}

		CSingleLock pLock( &Network.m_pSection );
		if ( pLock.Lock( 250 ) && Check( m_pWebCache ) )
			m_pWebCache->OnFailure();
	}

	m_pRequest.Clear();
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute hosts request

BOOL CDiscoveryServices::RunWebCacheGet(BOOL bCaches)
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	if ( ! Check( m_pWebCache ) )
		return FALSE;

	m_pWebCache->OnAccess();
	m_pWebCache->OnGivenHosts();

	CString strURL = m_pWebCache->m_sAddress;
	if ( bCaches )
		strURL += L"?get=1&urlfile=1";
	else
		strURL += L"?get=1&hostfile=1";

	//	strURL += L"&support=1";				// GWC network and status - ToDo: Use this parameter's output to check GWCs for self-network support relay.
	//	strURL += L"&info=1";					// Maintainer Info - ToDo: Use this parameter's output to add info (about maintainer etc.) into new Discovery window columns.

	if ( m_nLastQueryProtocol == PROTOCOL_G2 )
	{
		strURL += L"&net=gnutella2&ping=1&pv=4";
		strURL += L"&client=" _T(VENDOR_CODE);	// Version number is combined with client parameter for spec2
		strURL += theApp.m_sVersion;
	}
	else
	{
		// Assume gnutella
		strURL += L"&net=gnutella";				// Some gnutella GWCs that serve spec1 will not serve right on host/url requests combined with the ping request.
		strURL += L"&client=" _T(VENDOR_CODE) L"&version="; 	// Version parameter is spec1
		strURL += theApp.m_sVersion;
	}

	pLock.Unlock();

	// Specification 2.1 additions... (cluster output disabled, as clustering concept was vague)
	strURL += L"&getleaves=1&getnetworks=1&getclusters=0&getvendors=1&getuptime=1";

	CString strOutput;
	if ( ! SendWebCacheRequest( strURL, strOutput ) )
		return FALSE;

	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	if ( ! Check( m_pWebCache ) )
		return FALSE;

	int nHosts = 0, nCaches = 0;

	// Split answer to lines
	while ( ! strOutput.IsEmpty() )
	{
		CString strLine	= strOutput.SpanExcluding( L"\r\n" );
		strOutput		= strOutput.Mid( strLine.GetLength() + 1 );
		strLine.Trim( L"\r\n \t" );
		if ( strLine.IsEmpty() )
			continue;

		//theApp.Message( MSG_DEBUG, L"GWebCache %s : %s", (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );

		// Split line to parts
		CArray< CString > oParts;
		for ( CString strTmp = strLine ; ! strTmp.IsEmpty() ; )
		{
			CString strPart = strTmp.SpanExcluding( L"|" );
			strTmp = strTmp.Mid( strPart.GetLength() + 1 );
			oParts.Add( strPart.Trim() );
		}

		if ( oParts[ 0 ].CompareNoCase( L"h" ) == 0 )
		{
			// Hosts: "h|Host:Port|Age|Cluster|CurrentLeaves|VendorCode|Uptime|LeafLimit"
			if ( oParts.GetCount() < 3 )
				return FALSE;	// Empty

			// Get host and age fields
			int nPos = oParts[ 1 ].Find( L':' );
			if ( nPos < 6 )
				return FALSE;	// Invalid format

			DWORD nAddress = inet_addr( CT2CA( (LPCTSTR)oParts[ 1 ].Left( nPos ) ) );
			if ( nAddress == INADDR_NONE || nAddress < 10 )
				return FALSE;	// Invalid format

			int nPort = 0;
			if ( _stscanf( oParts[ 1 ].Mid( nPos + 1 ), L"%i", &nPort ) != 1 || nPort < 0 || nPort > 65535 )
				return FALSE;	// Invalid format

			int nSeconds = 0;
			if ( _stscanf( oParts[ 2 ], L"%i", &nSeconds ) != 1 || nSeconds < 0 || nSeconds > 60 * 60 * 24 * 366 )		// 1yr+ ?
				return FALSE;	// Invalid format

			// Skip cluster field

			// Get current leaves field
			DWORD nCurrentLeaves = 0;
			if ( oParts.GetCount() >= 5 && ! oParts[ 4 ].IsEmpty() )
			{
				int nCurrentLeavesTmp;
				if ( _stscanf( oParts[ 4 ], L"%i", &nCurrentLeavesTmp ) == 1 &&
					nCurrentLeavesTmp >= 0 && nCurrentLeavesTmp < 2048 )
				{
					nCurrentLeaves = nCurrentLeavesTmp;
				}
				else	// Bad current leaves format
				{
					return FALSE;
				}
			}

			// Get vendor field
			CVendor* pVendor = NULL;
			if ( oParts.GetCount() >= 6 && ! oParts[ 5 ].IsEmpty() )
			{
				CString strVendor = oParts[ 5 ].Left( 4 );
				if ( Security.IsVendorBlocked( strVendor ) )
					return FALSE;	// Invalid client
				pVendor = VendorCache.Lookup( strVendor );
			}

			// Get uptime field
			DWORD tUptime = 0;
			if ( oParts.GetCount() >= 7 && ! oParts[ 6 ].IsEmpty() )
			{
				int tUptimeTmp;
				if ( _stscanf( oParts[ 6 ], L"%i", &tUptimeTmp ) == 1 &&
					tUptimeTmp > 60 && tUptimeTmp < 60 * 60 * 24 * 365 )
				{
					tUptime = tUptimeTmp;
				}
				else	// Bad uptime format
				{
					return FALSE;
				}
			}

			// Get leaf limit field
			DWORD nLeafLimit = 0;
			if ( oParts.GetCount() >= 8 && ! oParts[ 7 ].IsEmpty() )
			{
				int nLeafLimitTmp;
				if ( _stscanf( oParts[ 7 ], L"%i", &nLeafLimitTmp ) == 1 &&
					nLeafLimitTmp >= 0 && nLeafLimitTmp < 2048 )
				{
					nLeafLimit = nLeafLimitTmp;
				}
				else	// Bad uptime format
				{
					return FALSE;
				}
			}

			const DWORD tSeen = static_cast< DWORD >( time( NULL ) ) - nSeconds;

			if ( ( m_nLastQueryProtocol == PROTOCOL_G2 ) ?
				HostCache.Gnutella2.Add( (IN_ADDR*)&nAddress, (WORD)nPort,
					tSeen, ( pVendor ? (LPCTSTR)pVendor->m_sCode : NULL ),
					tUptime, nCurrentLeaves, nLeafLimit ) :
				HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, (WORD)nPort,
					tSeen, ( pVendor ? (LPCTSTR)pVendor->m_sCode : NULL ),
					tUptime, nCurrentLeaves, nLeafLimit ) )
			{
				m_pWebCache->OnHostAdd();
				nHosts++;
			}
		}
		else if ( oParts[ 0 ].CompareNoCase( L"u" ) == 0 )
		{
			// URLs: "u|URL|Age"
			if ( oParts.GetCount() < 2 )
				return FALSE;		// Empty

			if ( _tcsnicmp( oParts[ 1 ], L"http://", 7 ) == 0 )
			{
				if ( Add( oParts[ 1 ], CDiscoveryService::dsWebCache, m_nLastQueryProtocol ) )
				{
					m_pWebCache->OnURLAdd();
					nCaches++;
				}
			}
			else if ( ( _tcsnicmp( oParts[ 1 ], L"uhc://",  6 ) == 0 && m_nLastQueryProtocol != PROTOCOL_G2 ) ||
					  ( _tcsnicmp( oParts[ 1 ], L"ukhl://", 7 ) == 0 && m_nLastQueryProtocol == PROTOCOL_G2 ) )
			{
				if ( Add( oParts[ 1 ], CDiscoveryService::dsGnutella, m_nLastQueryProtocol ) )
				{
					m_pWebCache->OnURLAdd();
					nCaches++;
				}
			}
		}
		else if ( oParts[ 0 ].CompareNoCase( L"UHC" ) == 0 )
		{
			// UDP Host Cache URL (For Gnutella1 ONLY)
			if ( oParts.GetCount() < 2 )
				return FALSE;		// Empty

			if ( m_nLastQueryProtocol != PROTOCOL_G2 )
			{
				if ( Add( oParts[ 1 ], CDiscoveryService::dsGnutella, m_nLastQueryProtocol ) )
				{
					m_pWebCache->OnURLAdd();
					nCaches++;
				}
			}
		}
		else if ( oParts[ 0 ].CompareNoCase( L"UKHL" ) == 0 )
		{
			// UDP Known Hub List URL (For Gnutella2 ONLY)
			if ( oParts.GetCount() < 2 )
				return FALSE;		// Empty

			if ( m_nLastQueryProtocol == PROTOCOL_G2 )
			{
				if ( Add( oParts[ 1 ], CDiscoveryService::dsGnutella, m_nLastQueryProtocol ) )
				{
					m_pWebCache->OnURLAdd();
					nCaches++;
				}
			}
		}
		else if ( oParts[ 0 ].CompareNoCase( L"i" ) == 0 )
		{
			// Informational Response: "i|command|...."
			if ( oParts.GetCount() >= 2 )
			{
				if ( oParts[ 1 ].CompareNoCase( L"pong" ) == 0 )
				{
					// "i|pong|vendor x.x.x|networks"
					// pong v2 (Skulls-type PONG network extension usage)
					// Usage here: Used to check if cache supports requested network.
					if ( m_nLastQueryProtocol != PROTOCOL_G2 )
					{
						// Mystery pong received - possibly a hosted static web page.
						theApp.Message( MSG_ERROR, L"[DiscoveryServices] Mystery PONG received when no ping was given: GWebCache %s", (LPCTSTR)m_pWebCache->m_sAddress );
						return FALSE;
					}
					if ( oParts.GetCount() >= 3 )
					{
						m_pWebCache->m_sPong = oParts[ 2 ];

						if ( oParts.GetCount() >= 4 )
						{
							BOOL bIsNetwork = FALSE;
							for ( int i = 0 ; ; )
							{
								CString strNetwork = oParts[ 3 ].Tokenize( L"-", i );
								if ( i == -1 )
									break;
								if ( ( strNetwork.CompareNoCase( L"gnutella2" ) == 0 && m_nLastQueryProtocol == PROTOCOL_G2 ) ||
									 ( strNetwork.CompareNoCase( L"gnutella" )  == 0 && m_nLastQueryProtocol != PROTOCOL_G2 ) )
								{
									bIsNetwork = TRUE;
								}
							}
							if ( ! bIsNetwork )
								return FALSE;
						}
					}
				}
				else if ( oParts[ 1 ].CompareNoCase( L"access" ) == 0 )
				{
					// "i|access|..."
					if ( oParts.GetCount() >= 4 &&
						 oParts[ 2 ].CompareNoCase( L"period" ) == 0 )
					{
						// "i|access|period|access period"
						DWORD nAccessPeriod;
						if ( _stscanf( oParts[ 3 ], L"%lu", &nAccessPeriod ) == 1 )
							m_pWebCache->m_nAccessPeriod = nAccessPeriod;
					}
				}
				else if ( oParts[ 1 ].CompareNoCase( L"force" ) == 0 )
				{
					// "i|force|..."
					if ( oParts.GetCount() >= 3 &&
						 oParts[ 2 ].CompareNoCase( L"remove" ) == 0 )
					{
						// "i|force|remove"
						m_pWebCache->Remove();
						return FALSE;
					}
				}
				else if ( oParts[ 1 ].CompareNoCase( L"update" ) == 0 )
				{
					// "i|update|..."
					if ( oParts.GetCount() >= 4 &&
						 oParts[ 2 ].CompareNoCase( L"warning" ) == 0 &&
						 oParts[ 3 ].CompareNoCase( L"bad url" ) == 0 )
					{
						// "i|update|warning|bad url"
						m_pWebCache->Remove();
						return FALSE;
					}
				}
				else if ( oParts[ 1 ].CompareNoCase( L"networks" ) == 0 )
				{
					// Beacon Cache type output
					// Used to check if cache supports requested network.
					if ( oParts.GetCount() >= 3 )
					{
						BOOL IsNetwork = FALSE;
						for ( int i = 2 ; i < oParts.GetCount() ; i++ )
						{
							if ( ( oParts[ i ].CompareNoCase( L"gnutella2" ) == 0 && m_nLastQueryProtocol == PROTOCOL_G2 ) ||
								 ( oParts[ i ].CompareNoCase( L"gnutella" )  == 0 && m_nLastQueryProtocol != PROTOCOL_G2 ) )
							{
								IsNetwork = TRUE;
							}
						}
						if ( ! IsNetwork )
							return FALSE;
					}
				}
				else if ( oParts[ 1 ].CompareNoCase( L"nets" ) == 0 )
				{
					// Skulls type output
					// Used to check if cache supports requested network.
					if ( oParts.GetCount() >= 3 )
					{
						BOOL IsNetwork = FALSE;
						for ( int i = 0 ; ; )
						{
							if ( i == -1 )
								break;
							CString strNetwork = oParts[ 2 ].Tokenize( L"-", i );
							if ( ( strNetwork.CompareNoCase( L"gnutella2" ) == 0 && m_nLastQueryProtocol == PROTOCOL_G2 ) ||
								 ( strNetwork.CompareNoCase( L"gnutella" )  == 0 && m_nLastQueryProtocol != PROTOCOL_G2 ) )
							{
								IsNetwork = TRUE;
							}
						}
						if ( ! IsNetwork )
							return FALSE;
					}
				}
			}
		}
		else if ( _tcsnicmp( strLine, L"PONG", 4 ) == 0 )
		{
			// pong v1
			if ( m_nLastQueryProtocol != PROTOCOL_G2 )
			{
				// Mystery pong received - possibly a hosted static web page.
				theApp.Message( MSG_ERROR, L"[DiscoveryServices] Mystery PONG received: GWebCache %s", (LPCTSTR)m_pWebCache->m_sAddress );
				return FALSE;
			}
		}
		else if ( _tcsistr( strLine, L"ERROR" ) != NULL )
		{
			if ( _tcsistr( strLine, L"Something Failed" ) != NULL )
			{
				// Some Bazooka GWCs are bugged but ok.
			}
			else
			{
				// Misc error. (Often CGI limits error)
				return FALSE;
			}
		}
		else if ( HostCache.Gnutella1.Add( strLine ) )
		{
			// Plain IP, G1
			m_pWebCache->OnHostAdd();
			nHosts++;
			m_pWebCache->m_bGnutella2 = FALSE;
			m_pWebCache->m_bGnutella1 = TRUE;
		}
		else if ( Add( strLine.SpanExcluding( L" " ), CDiscoveryService::dsWebCache, PROTOCOL_G1 ) )
		{
			// Plain URL, G1
			m_pWebCache->OnURLAdd();
			nCaches++;
			m_pWebCache->m_bGnutella2 = FALSE;
			m_pWebCache->m_bGnutella1 = TRUE;
		}
		else
			return FALSE;
	}

	if ( nCaches )
		m_bFirstTime = FALSE;

	if ( nHosts || nCaches )
	{
		m_pWebCache->OnSuccess();
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute update request

BOOL CDiscoveryServices::RunWebCacheUpdate()
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	if ( ! Check( m_pWebCache, CDiscoveryService::dsWebCache ) )
		return FALSE;

	m_pWebCache->OnAccess();

	CString strURL;
	if ( m_nWebCache == wcmUpdate )
	{
		if ( ! Network.IsListening() ) return TRUE;

		strURL.Format( L"%s?ip=%s:%hu&x.leaves=%u&uptime=%u&x.max=%u",
			(LPCTSTR)m_pWebCache->m_sAddress,
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
			htons( Network.m_pHost.sin_port ),
			Neighbours.GetCount( PROTOCOL_ANY, -1, ntLeaf ),
			Network.GetStableTime(),
			( m_nLastUpdateProtocol == PROTOCOL_G2 ) ? Settings.Gnutella2.NumLeafs : Settings.Gnutella1.NumLeafs );
	}

	if ( m_pSubmit != NULL && Check( m_pSubmit, CDiscoveryService::dsWebCache ) )
	{
		if ( strURL.IsEmpty() )
			strURL.Format( L"%s?url=", (LPCTSTR)m_pWebCache->m_sAddress );
		else
			strURL += L"&url=";

		CString strSubmit( m_pSubmit->m_sAddress );

		for ( int nSubmit = 0 ; nSubmit < strSubmit.GetLength() ; nSubmit ++ )
		{
			if ( (WORD)strSubmit.GetAt( nSubmit ) > 127 )
			{
				strSubmit = strSubmit.Left( nSubmit );
				break;
			}
		}

		strURL += URLEncode( strSubmit );
	}

	strURL += L"&update=1";						// 'update' parameter required for spec2
	if ( m_nLastUpdateProtocol == PROTOCOL_G2 )
	{
		strURL += L"&net=gnutella2";
		strURL += L"&client=" _T(VENDOR_CODE);	// Version number combined with client parameter for spec2
		strURL += theApp.m_sVersion;			// "ENVY1.0"
	}
	else
	{
		// Assume gnutella
		strURL += L"&net=gnutella";				// Some gnutella GWCs that serve spec 1 will not serve right on host/url requests combined with the ping request.
		strURL += L"&client=" _T(VENDOR_CODE) L"&version="; 	// Version parameter is spec1
		strURL += theApp.m_sVersion;
	}

	pLock.Unlock();

	if ( strURL.IsEmpty() )
		return FALSE;

	CString strOutput;
	if ( ! SendWebCacheRequest( strURL, strOutput ) )
		return FALSE;

	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	if ( ! Check( m_pWebCache, CDiscoveryService::dsWebCache ) )
		return FALSE;

	// Split answer to lines
	while ( ! strOutput.IsEmpty() )
	{
		CString strLine = strOutput.SpanExcluding( L"\r\n" );
		strOutput = strOutput.Mid( strLine.GetLength() + 1 );
		strLine.Trim( L"\r\n \t" );
		if ( strLine.IsEmpty() )
			continue;

	//	theApp.Message( MSG_DEBUG, L"[DiscoveryServices] GWebCache(update) %s : %s", (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );

	//	// Split line to parts
	//	CArray< CString > oParts;
	//	for ( int i = 0 ; ; )
	//	{
	//		CString strPart = strLine.Tokenize( L"|", i ).MakeLower();
	//		if ( i == -1 )
	//			break;
	//		oParts.Add( strPart );
	//	}

		if ( _tcsstr( strLine, L"OK" ) != NULL )
		{
			m_pWebCache->m_tUpdated = (DWORD)time( NULL );
			m_pWebCache->m_nUpdates++;
			m_pWebCache->OnSuccess();
			return TRUE;
		}

		if ( _tcsistr( strLine, L"ERROR" ) != NULL )
		{
			// GhostWhiteCrab type flood warning.
			if ( _tcsistr( strLine, L"ERROR: Client returned too early" ) != NULL )
				theApp.Message( MSG_ERROR, L"GWebCache(update) Too many connection attempts" );
			//else Misc error. (Often CGI limits error)
			return FALSE;
		}

		if ( StartsWith( strLine, _P( L"i|" ) ) )
		{
			if ( strLine == L"i|force|remove" )
			{
				m_pWebCache->Remove();
				return FALSE;
			}

			if ( _tcsistr( strLine, L"i|warning|client|early" ) != NULL ||
				 _tcsistr( strLine, L"i|warning|You came back too early" ) != NULL )
				 //_tcsistr( strLine, L"WARNING: You came back too early" ) != NULL )
				return FALSE;	// Old Beacon Cache type flood warning (404s for 0.4.1+)

			if ( _tcsnicmp( strLine, L"i|access|period|", 16 ) == 0 )
				_stscanf( (LPCTSTR)strLine + 16, L"%lu", &m_pWebCache->m_nAccessPeriod );
			else if ( _tcsnicmp( strLine, L"i|update|period|", 16 ) == 0 )
				_stscanf( (LPCTSTR)strLine + 16, L"%lu", &m_pWebCache->m_nUpdatePeriod );
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices HTTP request

BOOL CDiscoveryServices::SendWebCacheRequest(CString strURL, CString& strOutput)
{
	strOutput.Empty();
//	strURL += L"&client=" _T(VENDOR_CODE) L"&version=" + theApp.m_sVersion;  // DELETE

	if ( ! m_pRequest.SetURL( strURL ) )
		return FALSE;

	theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, L"[DiscoveryServices] Request: %s", (LPCTSTR)strURL );

	if ( ! m_pRequest.Execute( false ) )
		return FALSE;

	int nStatusCode = m_pRequest.GetStatusCode();
	if ( nStatusCode < 200 || nStatusCode > 299 )
		return FALSE;

	strOutput = m_pRequest.GetResponseString();

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"[DiscoveryServices] Response: %s", (LPCTSTR)strOutput );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute server.met or hublist request, was RunServerMet()

BOOL CDiscoveryServices::RunServerList()
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	if ( ! Check( m_pWebCache, CDiscoveryService::dsServerList ) )
		return FALSE;

	m_pWebCache->OnAccess();
	m_pWebCache->OnGivenHosts();

	CString strURL = m_pWebCache->m_sAddress;

	pLock.Unlock();

	if ( ! m_pRequest.SetURL( strURL ) )
		return FALSE;

	if ( ! m_pRequest.Execute( false ) )
		return FALSE;

	const CBuffer* pBuffer = m_pRequest.GetResponseBuffer();

	CMemFile pFile;
	pFile.Write( pBuffer->m_pBuffer, pBuffer->m_nLength );
	pFile.Seek( 0, CFile::begin );

	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	if ( ! Check( m_pWebCache, CDiscoveryService::dsServerList ) )
		return FALSE;

	const int nCount = m_pWebCache->m_nProtocolID == PROTOCOL_DC ?
		HostCache.ImportHubList( &pFile ) : HostCache.ImportMET( &pFile );

	if ( ! nCount ) return FALSE;

	HostCache.Save();
	m_pWebCache->OnHostAdd( nCount );
	m_pWebCache->OnSuccess();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute

// Note: This is used by wndDiscovery only
BOOL CDiscoveryServices::Execute(CDiscoveryService* pService, Mode nMode)
{
	if ( pService->m_nType == CDiscoveryService::dsGnutella )
		return pService->ResolveGnutella();

	if ( pService->m_nType == CDiscoveryService::dsWebCache )
		return RequestWebCache( pService, nMode,
			pService->m_bGnutella2 ? PROTOCOL_G2 : PROTOCOL_G1 );

	if ( pService->m_nType == CDiscoveryService::dsServerList )
		return RequestWebCache( pService, CDiscoveryServices::wcmServerList,
			pService->m_sAddress.Find( L".xml.bz2" ) > 1 ? PROTOCOL_DC : PROTOCOL_ED2K );

	return FALSE;
}

void CDiscoveryServices::OnResolve(PROTOCOLID nProtocol, LPCTSTR szAddress, const IN_ADDR* pAddress, WORD nPort)
{
	// Code to invoke UDPHC/UDPKHL Sender, from CNetwork::OnWinsock(). (uhc:/ukhl:)
	if ( nProtocol != PROTOCOL_G2 && nProtocol != PROTOCOL_G1 )
		return;

	CString strAddress( nProtocol == PROTOCOL_G1 ? L"uhc:" : L"ukhl:" );
	strAddress += szAddress;

	CSingleLock pLock( &Network.m_pSection, TRUE );

	CDiscoveryService* pService = GetByAddress( strAddress );
	if ( pService == NULL )
	{
		strAddress.AppendFormat( L":%u", nPort );
		pService = GetByAddress( strAddress );
	}

	if ( pAddress )
	{
		if ( pService != NULL )
		{
			pService->m_pAddress = *pAddress;
			pService->m_nPort = nPort;
		}

		if ( nProtocol == PROTOCOL_G1 )
		{
			if ( CG1Packet* pPing = CG1Packet::New( G1_PACKET_PING, 1, Hashes::Guid( MyProfile.oGUID ) ) )
			{
				CGGEPBlock pBlock;
				if ( CGGEPItem* pItem = pBlock.Add( GGEP_HEADER_SUPPORT_CACHE_PONGS ) )
				{
					pItem->WriteByte( Neighbours.IsG1Ultrapeer() ? GGEP_SCP_ULTRAPEER : GGEP_SCP_LEAF );
				}
				pBlock.Write( pPing );
				Datagrams.Send( pAddress, nPort, pPing, TRUE, NULL, FALSE );
			}
		}
		else // G2
		{
			if ( CG2Packet* pKHLR = CG2Packet::New( G2_PACKET_KHL_REQ ) )
			{
				Datagrams.Send( pAddress, nPort, pKHLR, TRUE, NULL, FALSE );
			}
		}
	}
	else
	{
		if ( pService != NULL )
			pService->OnFailure();
	}
}


//////////////////////////////////////////////////////////////////////
// CDiscoveryService construction

CDiscoveryService::CDiscoveryService(Type nType, LPCTSTR pszAddress, PROTOCOLID nProtocol)
	: m_nType			( nType )
	, m_nSubType		( dsOldBootStrap )
	, m_nProtocolID		( nProtocol )	// r68
	, m_bGnutella2		( FALSE )
	, m_bGnutella1		( FALSE )
	, m_pAddress		( )
	, m_sAddress		( pszAddress ? pszAddress : L"" )
	, m_nPort			( 0 )
	, m_tAccessed		( 0 )
	, m_nAccesses		( 0 )
	, m_tUpdated		( 0 )
	, m_nUpdates		( 0 )
	, m_nHosts			( 0 )
	, m_nTotalHosts 	( 0 )
	, m_nURLs			( 0 )
	, m_nTotalURLs		( 0 )
	, m_nFailures		( 0 )
	, m_tCreated		( (DWORD)time( NULL ) )
	, m_nAccessPeriod	( max( Settings.Discovery.UpdatePeriod, 1800ul ) )
	, m_nUpdatePeriod	( Settings.Discovery.DefaultUpdate )
{
}

CDiscoveryService::~CDiscoveryService()
{
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService remove

void CDiscoveryService::Remove(BOOL bCheck)
{
	DiscoveryServices.Remove( this, bCheck );
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService serialize

void CDiscoveryService::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_nType;
		ar << m_nProtocolID;
		ar << m_sAddress;
		ar << m_bGnutella2;
		ar << m_bGnutella1;
		ar << m_tCreated;
		ar << m_tAccessed;
		ar << m_nAccesses;
		ar << m_tUpdated;
		ar << m_nUpdates;
		ar << m_nFailures;
		ar << m_nHosts;
		ar << m_nTotalHosts;
		ar << m_nURLs;
		ar << m_nTotalURLs;
		ar << m_nAccessPeriod;
		ar << m_nUpdatePeriod;
		ar << m_sPong;
	}
	else // Loading
	{
		ar >> (int&)m_nType;
		ar >> (int&)m_nProtocolID;	// No Shareaza imports
		ar >> m_sAddress;
		ar >> m_bGnutella2;
		ar >> m_bGnutella1;
		ar >> m_tCreated;
		ar >> m_tAccessed;
		ar >> m_nAccesses;
		ar >> m_tUpdated;
		ar >> m_nUpdates;
		ar >> m_nFailures;
		ar >> m_nHosts;
		ar >> m_nTotalHosts;
		ar >> m_nURLs;
		ar >> m_nTotalURLs;
		ar >> m_nAccessPeriod;
		ar >> m_nUpdatePeriod;
		ar >> m_sPong;

		// Check it has a valid protocol
		if ( _tcsnicmp( m_sAddress, L"ukhl:", 5 ) == 0 )
		{
			m_bGnutella1 = FALSE;
			m_bGnutella2 = TRUE;
			m_nSubType = dsGnutella2UDPKHL;
			m_nProtocolID = PROTOCOL_G2;	// Import?
		}
		else if ( _tcsnicmp( m_sAddress, L"uhc:", 4 ) == 0 )
		{
			m_bGnutella1 = TRUE;
			m_bGnutella2 = FALSE;
			m_nSubType = dsGnutellaUDPHC;
			m_nProtocolID = PROTOCOL_G1;
		}
		else if ( _tcsnicmp( m_sAddress, L"gnutella1:host:", 15 ) == 0 ||
				  _tcsnicmp( m_sAddress, L"gnutella:host:",  14 ) == 0 )
		{
			m_bGnutella1 = TRUE;
			m_bGnutella2 = FALSE;
			m_nSubType = dsGnutellaTCP;
			m_nProtocolID = PROTOCOL_G1;
		}
		else if ( _tcsnicmp( m_sAddress, L"gnutella2:host:", 15 ) == 0 ||
				  _tcsnicmp( m_sAddress, L"g2:host:", 8 ) == 0 )
		{
			m_bGnutella1 = FALSE;
			m_bGnutella2 = TRUE;
			m_nSubType = dsGnutella2TCP;
			m_nProtocolID = PROTOCOL_G2;
		}
	//	else if ( m_sAddress.IsEmpty() || m_bGnutella2 != 0 || m_bGnutella2 != 1 || m_bGnutella1 != 0 || m_bGnutella1 != 1 )
	//		m_nSubType = -1;	// Invalid
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService resolve a gnutella node

BOOL CDiscoveryService::ResolveGnutella()
{
	if ( ! Network.Connect( FALSE ) ) return FALSE;

	if ( time( NULL ) < m_tAccessed + 300 ) return FALSE;

	CString strHost = m_sAddress;
	int nSkip = 0;
	int nPort = 0;
	OnGivenHosts();

	// Check it has a valid protocol
	if ( _tcsnicmp( strHost, L"ukhl:", 5 ) == 0 )
	{
		m_nSubType = dsGnutella2UDPKHL;
		m_bGnutella1 = FALSE;
		m_bGnutella2 = TRUE;
		nPort = protocolPorts[ PROTOCOL_G2 ];
		nSkip = 5;
	}
	else if ( _tcsnicmp( strHost, L"uhc:", 4 ) == 0 )
	{
		m_nSubType = dsGnutellaUDPHC;
		m_bGnutella1 = TRUE;
		m_bGnutella2 = FALSE;
		nPort = 9999;
		nSkip = 4;
	}
	else if ( _tcsnicmp( strHost, L"gnutella1:host:", 15 ) == 0 ||
			  _tcsnicmp( strHost, L"gnutella:host:", 14 ) == 0 )
	{
		m_nSubType = dsGnutellaTCP;
		m_bGnutella1 = TRUE;
		m_bGnutella2 = FALSE;
		nPort = protocolPorts[ PROTOCOL_G1 ];
		nSkip = 15;
	}
	else if ( _tcsnicmp( strHost, L"gnutella2:host:", 15 ) == 0 ||
			  _tcsnicmp( strHost, L"g2:host:", 8 ) == 0)
	{
		m_nSubType = dsGnutella2TCP;
		m_bGnutella1 = FALSE;
		m_bGnutella2 = TRUE;
		nPort = protocolPorts[ PROTOCOL_G2 ];
		nSkip = 15;
	}

	if ( m_nSubType == dsOldBootStrap )
	{
		int nPos = strHost.Find( L':' );
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), L"%i", &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G1, RESOLVE_ONLY ) )
		{
			OnSuccess();
			return TRUE;
		}
	}
	else if ( m_nSubType == dsGnutellaTCP )
	{
		strHost  = strHost.Mid( nSkip );
		int nPos = strHost.Find( L':' );
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), L"%i", &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G1, RESOLVE_CONNECT_ULTRAPEER ) )
		{
			OnAccess();
			return TRUE;
		}
	}
	else if ( m_nSubType == dsGnutella2TCP )
	{
		strHost  = strHost.Mid( nSkip );
		int nPos = strHost.Find( L':' );
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), L"%i", &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G2, RESOLVE_CONNECT_ULTRAPEER ) )
		{
			OnAccess();
			return TRUE;
		}
	}
	else if ( m_nSubType == dsGnutellaUDPHC )
	{
		strHost  = strHost.Mid( nSkip );
		int nPos = strHost.Find( L':' );
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), L"%i", &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G1, RESOLVE_DISCOVERY ) )
		{
			OnAccess();
			return TRUE;
		}
	}
	else if ( m_nSubType == dsGnutella2UDPKHL )
	{
		strHost  = strHost.Mid( nSkip );
		int nPos = strHost.Find( L':' );
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), L"%i", &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G2, RESOLVE_DISCOVERY ) )
		{
			OnAccess();
			return TRUE;
		}
	}

	OnFailure();

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService events

void CDiscoveryService::OnAccess()
{
	m_tAccessed = (DWORD)time( NULL );
	m_nAccesses ++;
}

void CDiscoveryService::OnGivenHosts()
{
	// Resetting Per-Request Statistics
	m_nHosts = 0;	// Should reset host count stats every time a cache is called.
	m_nURLs = 0;	// Should reset URL count stats every time a cache is called.
}

void CDiscoveryService::OnHostAdd(int nCount)
{
	// Host count tracking
	m_nHosts += nCount;
	m_nTotalHosts += nCount;
	m_nFailures = 0;
}

void CDiscoveryService::OnCopyGiven()
{
	// Used for UDP bootstrap host count
	m_nTotalHosts += m_nHosts;
}

void CDiscoveryService::OnURLAdd(int nCount)
{
	// URL count tracking
	m_nURLs += nCount;
	m_nTotalURLs += nCount;
}

void CDiscoveryService::OnSuccess()
{
	m_nFailures = 0;

	if ( m_nType == dsWebCache || m_nType == dsServerList )
		theApp.Message( MSG_INFO, IDS_DISCOVERY_WEB_SUCCESS, (LPCTSTR)m_sAddress );
}

void CDiscoveryService::OnFailure()
{
	m_nFailures++;

	theApp.Message( MSG_ERROR, IDS_DISCOVERY_FAILED, (LPCTSTR)m_sAddress, m_nFailures );

	if ( m_nFailures >= Settings.Discovery.FailureLimit )
	{
		theApp.Message( MSG_ERROR, IDS_DISCOVERY_FAIL_REMOVE, (LPCTSTR)m_sAddress, m_nFailures );
		Remove();
	}
}
