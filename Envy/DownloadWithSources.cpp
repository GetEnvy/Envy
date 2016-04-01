//
// DownloadWithSources.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2008
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
#include "DownloadWithSources.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadGroups.h"
#include "Download.h"
#include "Downloads.h"
#include "Transfer.h"
#include "Transfers.h"
#include "MatchObjects.h"
#include "Network.h"
#include "Neighbours.h"

#include "Library.h"
#include "SharedFile.h"
#include "Security.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "EnvyURL.h"
#include "QueryHit.h"
#include "QueryHashMaster.h"
#include "VendorCache.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CDownloadWithSources construction

CDownloadWithSources::CDownloadWithSources()
	: m_nG1SourceCount	( 0 )
	, m_nG2SourceCount	( 0 )
	, m_nEdSourceCount	( 0 )
	, m_nHTTPSourceCount( 0 )
	, m_nFTPSourceCount	( 0 )
	, m_nBTSourceCount	( 0 )
	, m_nDCSourceCount	( 0 )
	, m_pXML			( NULL )
{
}

CDownloadWithSources::~CDownloadWithSources()
{
	ClearSources();

	delete m_pXML;

	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
		delete m_pFailedSources.GetNext( pos );

	m_pFailedSources.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources list access

POSITION CDownloadWithSources::GetIterator() const
{
	ASSUME_LOCK( Transfers.m_pSection );

	return m_pSources.GetHeadPosition();
}

CDownloadSource* CDownloadWithSources::GetNext(POSITION& rPosition) const
{
	ASSUME_LOCK( Transfers.m_pSection );

	return m_pSources.GetNext( rPosition );
}

INT_PTR CDownloadWithSources::GetCount() const
{
	ASSUME_LOCK( Transfers.m_pSection );

	return m_pSources.GetCount();
}

bool CDownloadWithSources::HasMetadata() const
{
	return ( m_pXML != NULL );
}

DWORD CDownloadWithSources::GetSourceCount(BOOL bNoPush, BOOL bSane) const
{
	CQuickLock pLock( Transfers.m_pSection );

	if ( ! bNoPush && ! bSane )
		return (DWORD)GetCount();

	const DWORD tNow = GetTickCount();
	DWORD nCount = 0;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( ! bNoPush || ! pSource->m_bPushOnly )
		{
			if ( ! bSane ||
			   ( ( pSource->m_tAttempt < tNow ||
				   pSource->m_tAttempt - tNow <= 900000 ) &&
				 ! pSource->m_bKeep ) )
			{
				nCount++;
			}
		}
	}

	return nCount;
}

DWORD CDownloadWithSources::GetEffectiveSourceCount() const
{
	DWORD nResult = 0;

	if ( Settings.Connection.RequireForTransfers )
	{
		if ( Settings.Gnutella2.Enabled || Settings.Gnutella1.Enabled )
			nResult += m_nHTTPSourceCount;
		if ( Settings.Gnutella1.Enabled )
			nResult += m_nG1SourceCount;
		if ( Settings.Gnutella2.Enabled )
			nResult += m_nG2SourceCount;
		if ( Settings.eDonkey.Enabled )
			nResult += m_nEdSourceCount;
		if ( Settings.DC.Enabled )
			nResult += m_nDCSourceCount;
		if ( Settings.BitTorrent.Enabled )
			nResult += m_nBTSourceCount;
		nResult += m_nFTPSourceCount;
	}
	else
	{
		nResult = m_nHTTPSourceCount + m_nG1SourceCount + m_nG2SourceCount +
			 m_nEdSourceCount + m_nBTSourceCount + m_nFTPSourceCount;
	}

	return nResult;
}

DWORD CDownloadWithSources::GetBTSourceCount(BOOL bNoPush) const
{
	CQuickLock pLock( Transfers.m_pSection );

	const DWORD tNow = GetTickCount();
	DWORD nCount = 0;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( ( pSource->m_nProtocol == PROTOCOL_BT ) &&									// Only counting BT sources
			 ( pSource->m_tAttempt < tNow || pSource->m_tAttempt - tNow <= 900000 ) &&	// Don't count dead sources
			 ( ! pSource->m_bPushOnly || ! bNoPush ) )									// Push sources might not be counted
		{
			nCount++;
		}
	}

	return nCount;
}

DWORD CDownloadWithSources::GetED2KCompleteSourceCount() const
{
	CQuickLock pLock( Transfers.m_pSection );

	const DWORD tNow = GetTickCount();
	DWORD nCount = 0;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( ( ! pSource->m_bPushOnly ) &&						// Push sources shouldn't be counted since you often cannot reach them
			 ( pSource->m_tAttempt < tNow || pSource->m_tAttempt - tNow <= 900000 ) &&	// Only count sources that are probably active
			 ( pSource->m_nProtocol == PROTOCOL_ED2K ) &&		// Only count ed2k sources
			 ( pSource->m_oAvailable.empty() && pSource->IsOnline() ) )	// Only count complete sources
		{
			nCount++;
		}
	}

	return nCount;
}

BOOL CDownloadWithSources::CheckSource(CDownloadSource* pCheck) const
{
	CQuickLock pLock( Transfers.m_pSection );

	return ( m_pSources.Find( pCheck ) != NULL );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources clear

void CDownloadWithSources::ClearSources()
{
	CQuickLock pLock( Transfers.m_pSection );

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		pSource->Remove();
	}
	m_pSources.RemoveAll();

	m_nG1SourceCount	= 0;
	m_nG2SourceCount	= 0;
	m_nEdSourceCount	= 0;
	m_nHTTPSourceCount	= 0;
	m_nBTSourceCount	= 0;
	m_nFTPSourceCount	= 0;
	m_nDCSourceCount	= 0;

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add a query-hit source

BOOL CDownloadWithSources::AddSource(const CEnvyFile* pHit, BOOL bForce)
{
	ASSUME_LOCK( Transfers.m_pSection );

	bool bHash = false;
	bool bUpdated = false;
	const bool bHasHash = HasHash();
	const bool bHitHasName = ( ! pHit->m_sName.IsEmpty() );
	const bool bHitHasSize = ( pHit->m_nSize && pHit->m_nSize != SIZE_UNKNOWN );

	if ( ! bForce )
	{
		// We should check Tiger as well as others.
		// Some hash combinations exist with same SHA1 but different Tiger.
		// (Shareaza 2.2.0.0 installer file) (CyberBob)

		if ( m_oSHA1 && pHit->m_oSHA1 )
		{
			if ( m_oSHA1 != pHit->m_oSHA1 ) return FALSE;
			bHash = true;
		}
		if ( m_oTiger && pHit->m_oTiger )
		{
			if ( m_oTiger != pHit->m_oTiger ) return FALSE;
			bHash = true;
		}
		if ( m_oED2K && pHit->m_oED2K )
		{
			if ( m_oED2K != pHit->m_oED2K ) return FALSE;
			bHash = true;
		}
		if ( m_oMD5 && pHit->m_oMD5 )
		{
			if ( m_oMD5 != pHit->m_oMD5 ) return FALSE;
			bHash = true;
		}
		if ( ! bHash && m_oBTH && pHit->m_oBTH )
		{
			// btih check last chance
			if ( m_oBTH != pHit->m_oBTH ) return FALSE;
			bHash = true;
		}

		if ( ! bHash )
		{
			if ( Settings.General.HashIntegrity ) return FALSE;

			if ( m_sName.IsEmpty() || ! bHitHasName ) return FALSE;
			if ( m_nSize == SIZE_UNKNOWN || ! bHitHasSize ) return FALSE;

			if ( m_nSize != pHit->m_nSize ) return FALSE;
			if ( m_sName.CompareNoCase( pHit->m_sName ) ) return FALSE;
		}
	}

	if ( m_nSize != SIZE_UNKNOWN && bHasHash && bHitHasSize && m_nSize != pHit->m_nSize )
		return FALSE;

	if ( ! m_oSHA1 && pHit->m_oSHA1 )
	{
		m_oSHA1 = pHit->m_oSHA1;
		bUpdated = true;
	}
	if ( ! m_oTiger && pHit->m_oTiger )
	{
		m_oTiger = pHit->m_oTiger;
		bUpdated = true;
	}
	if ( ! m_oED2K && pHit->m_oED2K )
	{
		m_oED2K = pHit->m_oED2K;
		bUpdated = true;
	}
	if ( ! m_oBTH && pHit->m_oBTH )
	{
		m_oBTH = pHit->m_oBTH;
		bUpdated = true;
	}
	if ( ! m_oMD5 && pHit->m_oMD5 )
	{
		m_oMD5 = pHit->m_oMD5;
		bUpdated = true;
	}
	if ( ( m_sName.IsEmpty() || ! bHasHash ) && bHitHasName )
	{
		bUpdated = Rename( pHit->m_sName );
	}
	if ( ( m_nSize == SIZE_UNKNOWN || ! bHasHash ) && bHitHasSize )
	{
		bUpdated = Resize( pHit->m_nSize );
	}

	if ( bUpdated )
	{
		// Re-link
		DownloadGroups.Link( static_cast< CDownload* >( this ) );

		static_cast< CDownload* >( this )->m_bUpdateSearch = TRUE;

		QueryHashMaster.Invalidate();
	}

	return TRUE;
}

BOOL CDownloadWithSources::AddSourceHit(const CQueryHit* pHit, BOOL bForce)
{
	CQuickLock oLock( Transfers.m_pSection );

	if ( ! AddSource( pHit, bForce ) )
		return FALSE;

	if ( Settings.Downloads.Metadata && m_pXML == NULL && pHit->m_pXML && pHit->m_pSchema )
	{
		m_pXML = pHit->m_pSchema->Instantiate( TRUE );
		m_pXML->AddElement( pHit->m_pXML->Clone() );
		pHit->m_pSchema->Validate( m_pXML, TRUE );
	}

	//if ( pHit->m_nProtocol == PROTOCOL_ED2K )
	//	Neighbours.FindDonkeySources( pHit->m_oED2K,
	//		(IN_ADDR*)pHit->m_oClientID.begin(), (WORD)pHit->m_oClientID.begin()[1] );

	// No URL, stop now with success
	if ( ! pHit->m_sURL.IsEmpty() &&
		 ! AddSourceInternal( new CDownloadSource( (CDownload*)this, pHit ) ) )
		return FALSE;

	return TRUE;
}

BOOL CDownloadWithSources::AddSourceHit(const CMatchFile* pMatchFile, BOOL bForce)
{
	CQuickLock oLock( Transfers.m_pSection );

	BOOL bRet = AddSource( pMatchFile, bForce );

	// Best goes first if forced
	const CQueryHit* pBestHit = pMatchFile->GetBest();
	if ( bForce && pBestHit )
		bRet = AddSourceHit( pBestHit, TRUE ) || bRet;

	for ( const CQueryHit* pHit = pMatchFile->GetHits() ; pHit; pHit = pHit->m_pNext )
	{
		if ( bForce && pHit == pBestHit )
			continue;	// Best already added

		bRet = AddSourceHit( pHit, bForce ) || bRet;
	}

	return bRet;
}

BOOL CDownloadWithSources::AddSourceHit(const CEnvyURL& oURL, BOOL bForce, int nRedirectionCount /*0*/)
{
	CQuickLock oLock( Transfers.m_pSection );

	if ( ! AddSource( &oURL, bForce ) )
		return FALSE;

	if ( oURL.m_pTorrent )
		((CDownload*)this)->SetTorrent( oURL.m_pTorrent );

	for ( CString sURLs = oURL.m_sURL ; sURLs.GetLength() ; )
	{
		CString sURL = sURLs.SpanExcluding( L"," );
		sURLs = sURLs.Mid( sURL.GetLength() + 1 );
		sURL.Trim();
		if ( sURL.IsEmpty() )
			continue;
		if ( ! AddSourceURL( sURL, NULL, nRedirectionCount, FALSE ) )
			return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add miscellaneous sources

BOOL CDownloadWithSources::AddSourceED2K(DWORD nClientID, WORD nClientPort, DWORD nServerIP, WORD nServerPort, const Hashes::Guid& oGUID)
{
	return AddSourceInternal( new CDownloadSource( (CDownload*)this, nClientID, nClientPort, nServerIP, nServerPort, oGUID ) );
}

BOOL CDownloadWithSources::AddSourceBT(const Hashes::BtGuid& oGUID, const IN_ADDR* pAddress, WORD nPort)
{
	// Unreachable (Push) BT sources should never be added.
	if ( Network.IsFirewalledAddress( pAddress, Settings.Connection.IgnoreOwnIP ) )
		return FALSE;

	return AddSourceInternal( new CDownloadSource( (CDownload*)this, oGUID, pAddress, nPort ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add a single URL source

BOOL CDownloadWithSources::AddSourceURL(LPCTSTR pszURL, FILETIME* pLastSeen, int nRedirectionCount, BOOL bFailed)
{
	if ( pszURL == NULL || *pszURL == 0 )
		return FALSE;

	if ( nRedirectionCount > 5 )
		return FALSE;	// No more than 5 redirections

	BOOL bHashAuth = FALSE;
	CEnvyURL pURL;

	if ( *pszURL == '@' )
	{
		bHashAuth = TRUE;
		pszURL++;
	}

	if ( ! pURL.Parse( pszURL ) )
		return FALSE;	// Wrong URL

	if ( pURL.m_nAction == CEnvyURL::uriHost &&
		 pURL.m_nProtocol == PROTOCOL_DC )
	{
		// Connect to specified DC++ hub for future searches
		Network.ConnectTo( pURL.m_sName, pURL.m_nPort, PROTOCOL_DC );
		return FALSE;
	}

	if ( pURL.m_nAction != CEnvyURL::uriDownload &&
		 pURL.m_nAction != CEnvyURL::uriSource )
		return FALSE;	// Wrong URL type

	if ( pURL.m_pAddress.s_addr != INADDR_ANY && pURL.m_pAddress.s_addr != INADDR_NONE )
	{
		if ( Network.IsFirewalledAddress( &pURL.m_pAddress, TRUE ) ||
			 Network.IsReserved( &pURL.m_pAddress ) )
			return FALSE;	// Unreachable URL
	}

	CQuickLock pLock( Transfers.m_pSection );

	if ( CFailedSource* pBadSource = LookupFailedSource( pszURL ) )
	{
		// Add a positive vote, add to downloads if negative votes compose less than 2/3 of total.
		int nTotal = pBadSource->m_nPositiveVotes + pBadSource->m_nNegativeVotes + 1;
		if ( bFailed )
			pBadSource->m_nNegativeVotes++;
		else
			pBadSource->m_nPositiveVotes++;

		if ( nTotal > 30 && pBadSource->m_nNegativeVotes / nTotal > 2 / 3 )
			return FALSE;
	}
	else if ( bFailed )
	{
		AddFailedSource( pszURL, false );
		VoteSource( pszURL, false );
		return TRUE;
	}

	return	AddSource( &pURL, FALSE ) &&
			AddSourceInternal( new CDownloadSource( static_cast< const CDownload* >( this ), pszURL, bHashAuth, pLastSeen, nRedirectionCount ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add several URL sources

int CDownloadWithSources::AddSourceURLs(LPCTSTR pszURLs, BOOL bFailed)
{
	int nCount = 0;

	CMapStringToFILETIME oUrls;
	SplitStringToURLs( pszURLs, oUrls );

	for ( POSITION pos = oUrls.GetStartPosition() ; pos ; )
	{
		CString strURL;
		FILETIME tSeen = {};
		oUrls.GetNextAssoc( pos, strURL, tSeen );

		if ( AddSourceURL( strURL, ( tSeen.dwLowDateTime | tSeen.dwHighDateTime ) ? &tSeen : NULL, 0, bFailed ) )
		{
			if ( bFailed )
				theApp.Message( MSG_DEBUG, L"Adding X-NAlt: %s", (LPCTSTR)strURL );
			else
				theApp.Message( MSG_DEBUG, L"Adding X-Alt: %s", (LPCTSTR)strURL );
			nCount++;
		}
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources internal source adder

BOOL CDownloadWithSources::AddSourceInternal(CDownloadSource* pSource)
{
	if ( ! pSource )
		return FALSE;	// Out of memory

	// Check/Reject if source is invalid
	if ( ! pSource->m_bPushOnly )
	{
		// Reject invalid IPs (Sometimes ed2k sends invalid 0.x.x.x sources)
		if ( pSource->m_pAddress.S_un.S_un_b.s_b1 == 0 || pSource->m_nPort == 0 )
		{
			delete pSource;
			return FALSE;
		}

		// Reject if source is the local IP/port
		if ( Network.IsSelfIP( pSource->m_pAddress ) )
		{
			if ( Settings.Connection.IgnoreOwnIP ||
				( pSource->m_nServerPort == 0 &&
				Settings.Connection.InPort == pSource->m_nPort ) )
			{
				delete pSource;
				return FALSE;
			}
		}
	}
	else if ( pSource->m_nProtocol == PROTOCOL_ED2K )
	{
		// Reject invalid server IPs (Sometimes ed2k sends invalid 0.x.x.x sources)
		if ( pSource->m_pServerAddress.S_un.S_un_b.s_b1 == 0 )
		{
			delete pSource;
			return FALSE;
		}
	}

	CQuickLock pLock( Transfers.m_pSection );

	if ( pSource->m_nRedirectionCount == 0 )	// Don't check for existing sources if source is a redirection
	{
		bool bDeleteSource = false;
		bool bHTTPSource = pSource->IsHTTPSource();
		bool bNeedHTTPSource = ! bHTTPSource &&
			Settings.Gnutella2.Enabled &&
			VendorCache.IsExtended( pSource->m_sServer );

		// Remove unneeded sources
		for ( POSITION posSource = GetIterator() ; posSource ; )
		{
			CDownloadSource* pExisting = GetNext( posSource );

			ASSERT( pSource != pExisting );
			if ( pExisting->Equals( pSource ) ) 	// IPs and ports are equal
			{
				bool bExistingHTTPSource = pExisting->IsHTTPSource();
				pExisting->SetLastSeen();

				if ( bNeedHTTPSource && bExistingHTTPSource )
					bNeedHTTPSource = false;

				if ( ( pExisting->m_nProtocol == pSource->m_nProtocol ) ||
					 ( bExistingHTTPSource && bHTTPSource ) )
				{
					bDeleteSource = true;			// Same protocol
				}
				else if ( ! pExisting->IsIdle() )
				{
					// Already downloading so we can remove new non-HTTP source
					if ( bExistingHTTPSource && ! bHTTPSource )
						bDeleteSource = true;
				}
				else // We are not downloading
				{
					// ...So we can replace non-HTTP source with a new one
					if ( ! bExistingHTTPSource && bHTTPSource )
					{
						// Set connection delay the same as for the old source
						pSource->m_tAttempt = pExisting->m_tAttempt;
						pExisting->Remove( TRUE, FALSE );
					}
				}
			}
		}

		if ( bDeleteSource )
		{
			delete pSource;

			SetModified();

			return FALSE;
		}

		// Make additional G2 source from the existing non-HTTP Envy source
		if ( bNeedHTTPSource )
		{
			CString strURL = GetURL( pSource->m_pAddress, pSource->m_nPort );
			if ( ! strURL.IsEmpty() )
			{
				if ( CDownloadSource* pG2Source = new CDownloadSource( (CDownload*)this, strURL ) )
				{
					pG2Source->m_sServer = pSource->m_sServer;		// Copy user-agent
					pG2Source->m_tAttempt = pSource->m_tAttempt;	// Set the same connection delay
					pG2Source->m_nProtocol = PROTOCOL_HTTP;

					AddSourceInternal( pG2Source );
				}
			}
		}
	}

	// Trimming extra sources - Idle and bad or busy
	DWORD nSourceCount = GetEffectiveSourceCount();
	if ( nSourceCount >= Settings.Downloads.SourcesWanted )
	{
#ifdef _DEBUG
		FILETIME tNow;
		GetSystemTimeAsFileTime( &tNow );
#endif
		CSortedSources oIdleSources;
		for ( POSITION posSource = m_pSources.GetTailPosition() ; posSource && nSourceCount >= Settings.Downloads.SourcesWanted ; )
		{
			CDownloadSource* pExisting = m_pSources.GetPrev( posSource );

			if ( pExisting->IsIdle() )
			{
				if ( pExisting->m_nFailures || pExisting->m_nBusyCount )
				{
#ifdef _DEBUG
					theApp.Message( MSG_DEBUG, L"Removed extra source %s %s (%I64d seconds old)", (LPCTSTR)CString( inet_ntoa( pExisting->m_pAddress ) ),
						(LPCTSTR)pExisting->m_oGUID.toString< Hashes::base16Encoding >().MakeUpper(),
						( *( (LONGLONG*)&tNow ) - *( (LONGLONG*)&pExisting->m_tLastSeen ) ) / 10000000 );
#endif
					RemoveSource( pExisting, FALSE );
					delete pExisting;
					nSourceCount = GetEffectiveSourceCount();
				}
				else
				{
					oIdleSources.insert( pExisting );
				}
			}
		}
		// Remove older sources first
		for ( CSortedSources::const_iterator i = oIdleSources.begin() ; i != oIdleSources.end() && nSourceCount >= Settings.Downloads.SourcesWanted ; ++i )
		{
			CDownloadSource* pExisting = (*i);
#ifdef _DEBUG
			theApp.Message( MSG_DEBUG, L"Removed extra source %s %s (%I64d seconds old)", (LPCTSTR)CString( inet_ntoa( pExisting->m_pAddress ) ),
				(LPCTSTR)pExisting->m_oGUID.toString< Hashes::base16Encoding >().MakeUpper(),
				( *( (LONGLONG*)&tNow ) - *( (LONGLONG*)&pExisting->m_tLastSeen ) ) / 10000000 );
#endif
			RemoveSource( pExisting, FALSE );
			delete pExisting;
			nSourceCount = GetEffectiveSourceCount();
		}
		if ( nSourceCount >= Settings.Downloads.SourcesWanted )
		{
			// Still too many sources
#ifdef _DEBUG
			theApp.Message( MSG_DEBUG, L"Ignored extra source %s %s due sources limit %u", (LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ),
				(LPCTSTR)pSource->m_oGUID.toString< Hashes::base16Encoding >().MakeUpper(), Settings.Downloads.SourcesWanted );
#endif
			delete pSource;
			return FALSE;
		}
	}

	InternalAdd( pSource );

	SetModified();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources query for URLs

CString CDownloadWithSources::GetSourceURLs(CList< CString >* pState, int nMaximum, PROTOCOLID nProtocol, CDownloadSource* pExcept) const
{
	CQuickLock pLock( Transfers.m_pSection );

	CString strSources;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( pSource != pExcept && pSource->m_bPushOnly == FALSE &&
			 ( ( pSource->m_nFailures == 0 && pSource->m_bReadContent ) || nProtocol == PROTOCOL_NULL ) &&
			 ( pSource->m_bSHA1 || pSource->m_bTiger || pSource->m_bED2K || pSource->m_bBTH || pSource->m_bMD5 ) &&
			 ( pState == NULL || pState->Find( pSource->m_sURL ) == NULL ) )
		{
			// Only return appropriate sources
			if ( ( nProtocol == PROTOCOL_HTTP ) && ( pSource->m_nProtocol != PROTOCOL_HTTP ) ) continue;
			if ( ( nProtocol == PROTOCOL_G1 ) && ( pSource->m_nGnutella != 1 ) ) continue;
			//if ( bHTTP && pSource->m_nProtocol != PROTOCOL_HTTP ) continue;

			if ( pState != NULL ) pState->AddTail( pSource->m_sURL );

			if ( nProtocol == PROTOCOL_G1 )
			{
				if ( ! strSources.IsEmpty() )
					strSources += ',';
				strSources += CString( inet_ntoa( pSource->m_pAddress ) );
				CString strURL;
				strURL.Format( L"%hu", pSource->m_nPort );
				strSources += ':' + strURL;
			}
			else if ( pSource->m_sURL.Find( L"Zhttp://" ) >= 0 ||
				pSource->m_sURL.Find( L"Z%2C http://" ) >= 0 )
			{
				// Ignore buggy URLs
				TRACE( L"CDownloadWithSources::GetSourceURLs() Bad URL: %s\n", pSource->m_sURL );
			}
			else
			{
				CString strURL = pSource->m_sURL;
				strURL.Replace( L",", L"%2C" );

				if ( ! strSources.IsEmpty() ) strSources += L", ";
				strSources += strURL;
				strSources += ' ';
				strSources += TimeToString( &pSource->m_tLastSeen );
			}

			if ( nMaximum == 1 )
				break;
			else if ( nMaximum > 1 )
				nMaximum --;
		}
	}

	return strSources;
}

// Returns a string containing the most recent failed sources
CString	CDownloadWithSources::GetTopFailedSources(int nMaximum, PROTOCOLID nProtocol)
{
	// Currently we return only the string for G1, in X-NAlt format
	if ( nProtocol != PROTOCOL_G1 )
		return CString();

	CString strSources, str;
	CFailedSource* pResult = NULL;

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 150 ) )
		return str;

	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
	{
		pResult = m_pFailedSources.GetNext( pos );
		// Only return sources which we detected as failed
		if ( pResult && pResult->m_bLocal )
		{
			if ( _tcsistr( pResult->m_sURL, L"http://" ) != NULL )
			{
				int nPos = pResult->m_sURL.Find( L':', 8 );
				if ( nPos < 0 ) continue;
				str = pResult->m_sURL.Mid( 7, nPos - 7 );
				int nPosSlash = pResult->m_sURL.Find( L'/', nPos );
				if ( nPosSlash < 0 ) continue;

				if ( ! strSources.IsEmpty() )
					strSources += L',';

				strSources += str;
				str = pResult->m_sURL.Mid( nPos + 1, nPosSlash - nPos - 1 );
				strSources += L':';
				strSources += str;

				if ( nMaximum == 1 )
					break;
				if ( nMaximum > 1 )
					nMaximum--;
			}
		}
	}

	return strSources;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources query hit handler

BOOL CDownloadWithSources::OnQueryHits(const CQueryHit* pHits)
{
	for ( const CQueryHit* pHit = pHits ; pHit ; pHit = pHit->m_pNext )
	{
		AddSourceHit( pHit );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources remove overlapping sources

void CDownloadWithSources::RemoveOverlappingSources(QWORD nOffset, QWORD nLength)
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 150 ) )
		return;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( pSource->TouchedRange( nOffset, nLength ) )
		{
			if ( GetTaskType() == dtaskMergeFile )
			{
				// Merging process can produce corrupted blocks, retry connection after 30 seconds
				pSource->m_nFailures = 0;
				pSource->Close( 30 );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_VERIFY_DROP,
					(LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ),
					(LPCTSTR)pSource->m_sServer,
					(LPCTSTR)m_sName,
					nOffset, nOffset + nLength - 1 );
				pSource->Remove( TRUE, FALSE );
			}
		}
	}
}

// The function takes an URL and finds a failed source in the list.
// If bReliable is true, it checks only localy checked failed sources and those which have more than 20 votes
// from other users and negative votes compose 2/3 of the total number of votes.
CFailedSource* CDownloadWithSources::LookupFailedSource(LPCTSTR pszUrl, bool bReliable)
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 150 ) )
		return NULL;

	CFailedSource* pResult = NULL;

	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
	{
		pResult = m_pFailedSources.GetNext( pos );
		if ( pResult && pResult->m_sURL.Compare( pszUrl ) == 0 )
		{
#ifndef NDEBUG
			theApp.Message( MSG_DEBUG, L"Votes for file %s: negative - %i, positive - %i; offline status: %i",
				pszUrl, pResult->m_nNegativeVotes,
				pResult->m_nPositiveVotes,
				pResult->m_bOffline );
#endif
			if ( pResult->m_bLocal )
				break;

			if ( bReliable )	// Not used anywhere at the moment, we check that explicitly
			{
				INT_PTR nTotalVotes = pResult->m_nNegativeVotes + pResult->m_nPositiveVotes;
				if ( nTotalVotes > 20 && pResult->m_nNegativeVotes / nTotalVotes > 2 / 3 )
					break;
			}
			break;	// Temp solution to ensure same source not added more than once
					// We should check IPs which add these sources, since voting takes place, etc.
		}
		else
			pResult = NULL;
	}
	return pResult;
}

void CDownloadWithSources::AddFailedSource(const CDownloadSource* pSource, bool bLocal, bool bOffline)
{
	CString strURL;
	if ( pSource->m_nProtocol == PROTOCOL_BT && pSource->m_oGUID )
	{
		strURL.Format( L"btc://%s/%s/",
			(LPCTSTR)pSource->m_oGUID.toString(),
			(LPCTSTR)m_oBTH.toString() );
	}
	else
		strURL = pSource->m_sURL;

	AddFailedSource( (LPCTSTR)strURL, bLocal, bOffline );
}

void CDownloadWithSources::AddFailedSource(LPCTSTR pszUrl, bool bLocal, bool bOffline)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( LookupFailedSource( pszUrl ) == NULL )
	{
		if ( CFailedSource* pBadSource = new CFailedSource( pszUrl, bLocal, bOffline ) )
		{
			m_pFailedSources.AddTail( pBadSource );
			theApp.Message( MSG_DEBUG, L"Bad sources count for \"%s\": %i. URL: %s", m_sName, m_pFailedSources.GetCount(), pszUrl );
		}
	}
}

void CDownloadWithSources::VoteSource(LPCTSTR pszUrl, bool bPositively)
{
	if ( CFailedSource* pBadSource = LookupFailedSource( pszUrl ) )
	{
		if ( bPositively )
			pBadSource->m_nPositiveVotes++;
		else
			pBadSource->m_nNegativeVotes++;
	}
}

void CDownloadWithSources::ExpireFailedSources()
{
	ASSUME_LOCK( Transfers.m_pSection );

	const DWORD tNow = GetTickCount();
	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
	{
		POSITION posThis = pos;
		CFailedSource* pBadSource = m_pFailedSources.GetNext( pos );
		if ( m_pFailedSources.GetAt( posThis ) == pBadSource )
		{
			// Expire bad sources added more than 2 hours ago
			if ( tNow > pBadSource->m_nTimeAdded + ( 2 * 3600 * 1000 ) )
			{
				delete pBadSource;
				m_pFailedSources.RemoveAt( posThis );
			}
			else
				break;	// We appended to tail, so we do not need to move further
		}
	}
}

void CDownloadWithSources::ClearFailedSources()
{
	ASSUME_LOCK( Transfers.m_pSection );

	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
	{
		POSITION posThis = pos;
		CFailedSource* pBadSource = m_pFailedSources.GetNext( pos );
		if ( m_pFailedSources.GetAt( posThis ) == pBadSource )
		{
			delete pBadSource;
			m_pFailedSources.RemoveAt( posThis );
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CDownloadWithSources Internal Add/Remove Source

void CDownloadWithSources::InternalAdd(CDownloadSource* pSource)
{
	ASSUME_LOCK( Transfers.m_pSection );

	ASSERT( m_pSources.Find( pSource ) == NULL );
	m_pSources.AddTail( pSource );

	switch ( pSource->m_nProtocol )
	{
	case PROTOCOL_G1:
		m_nG1SourceCount++;
		break;
	case PROTOCOL_G2:
		m_nG2SourceCount++;
		break;
	case PROTOCOL_ED2K:
		m_nEdSourceCount++;
		break;
	case PROTOCOL_BT:
		m_nBTSourceCount++;
		break;
	case PROTOCOL_HTTP:
		m_nHTTPSourceCount++;
		break;
	case PROTOCOL_FTP:
		m_nFTPSourceCount++;
		break;
	case PROTOCOL_DC:
		m_nDCSourceCount++;
		break;
	default:
		ASSERT( FALSE );
	}
}

void CDownloadWithSources::InternalRemove(CDownloadSource* pSource)
{
	ASSUME_LOCK( Transfers.m_pSection );

	POSITION posSource = m_pSources.Find( pSource );
	ASSERT( posSource != NULL );
	m_pSources.RemoveAt( posSource );

	switch ( pSource->m_nProtocol )
	{
	case PROTOCOL_G1:
		m_nG1SourceCount--;
		break;
	case PROTOCOL_G2:
		m_nG2SourceCount--;
		break;
	case PROTOCOL_ED2K:
		m_nEdSourceCount--;
		break;
	case PROTOCOL_DC:
		m_nDCSourceCount--;
		break;
	case PROTOCOL_BT:
		m_nBTSourceCount--;
		break;
	case PROTOCOL_HTTP:
		m_nHTTPSourceCount--;
		break;
	case PROTOCOL_FTP:
		m_nFTPSourceCount--;
		break;
	//default:
	//	ASSERT( FALSE );
	}
}


//////////////////////////////////////////////////////////////////////
// CDownloadWithSources remove a source

void CDownloadWithSources::RemoveSource(CDownloadSource* pSource, BOOL bBan)
{
	ASSUME_LOCK( Transfers.m_pSection );

	InternalRemove( pSource );

	if ( bBan && ! pSource->m_sURL.IsEmpty() )
		AddFailedSource( pSource );

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources sort a source

void CDownloadWithSources::SortSource(CDownloadSource* pSource, BOOL bTop)
{
	CQuickLock pLock( Transfers.m_pSection );

	if ( m_pSources.GetCount() == 1 )
		return;		// No sorting

	POSITION posSource = m_pSources.Find( pSource );
	ASSERT( posSource );

	m_pSources.RemoveAt( posSource );

	if ( bTop )
		m_pSources.AddHead( pSource );
	else
		m_pSources.AddTail( pSource );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources sort a source by state (Downloading, etc...)

void CDownloadWithSources::SortSource(CDownloadSource* pSource)
{
	CQuickLock pLock( Transfers.m_pSection );

	if ( m_pSources.GetCount() == 1 )
		return;		// No sorting

	POSITION posSource = m_pSources.Find( pSource );
	ASSERT( posSource );

	m_pSources.RemoveAt( posSource );

	// Run through the sources to the correct position
	for ( POSITION posCompare = GetIterator() ; posCompare ; )
	{
		posSource = posCompare;
		CDownloadSource* pCompare = GetNext( posCompare );

		if ( pCompare->m_nSortOrder >= pSource->m_nSortOrder )
			break;
	}

	// Insert source in front of current compare source
	m_pSources.InsertBefore( posSource, pSource );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources source color selector

#define SRC_COLORS 8u

int CDownloadWithSources::GetSourceColor()
{
	BOOL bTaken[ SRC_COLORS ] = {};
	unsigned int nFree = SRC_COLORS;

	CSingleLock oLock( &Transfers.m_pSection );
	if ( oLock.Lock( 150 ) )
	{
		for ( POSITION posSource = GetIterator() ; posSource ; )
		{
			CDownloadSource* pSource = GetNext( posSource );

			if ( pSource->m_nColor >= 0 )
			{
				if ( bTaken[ pSource->m_nColor ] == FALSE )
				{
					bTaken[ pSource->m_nColor ] = TRUE;
					nFree--;
				}
			}
		}

		oLock.Unlock();
	}

	if ( nFree == 0 )
		return GetRandomNum( 0u, SRC_COLORS - 1 );

	nFree = GetRandomNum( 0u, nFree - 1 );

	for ( int nColor = 0 ; nColor < SRC_COLORS ; nColor++ )
	{
		if ( bTaken[ nColor ] == FALSE )
		{
			if ( nFree-- == 0 )
				return nColor;
		}
	}

	return GetRandomNum( 0u, SRC_COLORS - 1 );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources serialize

void CDownloadWithSources::Serialize(CArchive& ar, int nVersion)	// DOWNLOAD_SER_VERSION
{
	CDownloadBase::Serialize( ar, nVersion );

	CQuickLock pLock( Transfers.m_pSection );

	if ( ar.IsStoring() )
	{
		DWORD_PTR nSources = (DWORD_PTR)GetCount();
		if ( nSources > Settings.Downloads.SourcesWanted )
			nSources = (DWORD_PTR)Settings.Downloads.SourcesWanted;	// Don't save more than 500 sources
		ar.WriteCount( nSources );

		for ( POSITION posSource = GetIterator() ; posSource && nSources ; nSources-- )
		{
			CDownloadSource* pSource = GetNext( posSource );

			pSource->Serialize( ar, nVersion );
		}

		ar.WriteCount( m_pXML != NULL ? 1 : 0 );
		if ( m_pXML ) m_pXML->Serialize( ar );
	}
	else // Loading
	{
		for ( DWORD_PTR nSources = ar.ReadCount() ; nSources ; nSources-- )
		{
			// Create new source
			//CDownloadSource* pSource = new CDownloadSource( (CDownload*)this );	// Obsolete
			CAutoPtr< CDownloadSource > pSource( new CDownloadSource( static_cast< CDownload* >( this ) ) );
			if ( ! pSource )
				AfxThrowMemoryException();

			// Load details from disk
			pSource->Serialize( ar, nVersion );

			// Extract ed2k client ID from url (m_pAddress) because it wasn't saved
			if ( ! pSource->m_nPort && _tcsnicmp( pSource->m_sURL, L"ed2kftp://", 10 ) == 0 )
			{
				CString strURL = pSource->m_sURL.Mid( 10 );
				if ( ! strURL.IsEmpty() )
					_stscanf( strURL, L"%lu", &pSource->m_pAddress.S_un.S_addr );
			}

			// Add to the list no more than ~500 sources
			//if ( nSources < (DWORD_PTR)Settings.Downloads.SourcesWanted )
				InternalAdd( pSource.Detach() );
		}

		if ( ar.ReadCount() )
		{
			m_pXML = new CXMLElement();
			if ( ! m_pXML )
				AfxThrowMemoryException();

			m_pXML->Serialize( ar );
		}
	}
}

void CDownloadWithSources::MergeMetadata(const CXMLElement* pXML)
{
	CQuickLock pLock( Transfers.m_pSection );

	if ( m_pXML )
	{
		const CXMLAttribute* pAttr1 = m_pXML->GetAttribute( CXMLAttribute::schemaName );
		const CXMLAttribute* pAttr2 = pXML->GetAttribute( CXMLAttribute::schemaName );
		if ( pAttr1 && pAttr2 && pAttr1->GetValue().CompareNoCase( pAttr2->GetValue() ) == 0 )
		{
			CXMLElement* pElement1 = m_pXML->GetFirstElement();
			const CXMLElement* pElement2 = pXML->GetFirstElement();
			if ( pElement1 && pElement2 )
				pElement1->Merge( pElement2 );
		}
	}
	else
	{
		m_pXML = pXML->Clone();
	}
}
