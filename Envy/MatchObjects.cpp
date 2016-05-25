//
// MatchObjects.cpp
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
#include "MatchObjects.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "Library.h"
#include "SharedFile.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "Security.h"
#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"
#include "VendorCache.h"
#include "ShellIcons.h"
#include "RegExp.h"
#include "Colors.h"
#include "XML.h"

#include "CtrlMatch.h"
#include "LiveList.h"
#include "ResultFilters.h"
#include "CtrlSearchDetailPanel.h"
#include "WndSearch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define MAP_SIZE		256
#define BUFFER_GROW		64


//////////////////////////////////////////////////////////////////////
// CMatchList construction

CMatchList::CMatchList(CBaseMatchWnd* pParent)
	: m_pParent 		( pParent )
	, m_pSchema			( NULL )
	, m_bNew			( FALSE )
	, m_bSortDir		( 1 )
	, m_nSortColumn		( -1 )
	, m_pszFilter		( NULL )
	, m_pColumns		( NULL )
	, m_nColumns		( 0 )
	, m_pFiles			( NULL )
	, m_nFiles			( 0 )
	, m_nItems			( 0 )
	, m_nFilteredFiles	( 0 )
	, m_nFilteredHits	( 0 )
	, m_nGnutellaHits	( 0 )
	, m_nED2KHits		( 0 )
	, m_nBuffer 		( 0 )
{
	m_pResultFilters = new CResultFilters;
	m_pResultFilters->Load();

	const int nDefaultFilter = m_pResultFilters->m_nDefault;

	const BOOL bBrowsing = ( pParent && ! pParent->IsKindOf( RUNTIME_CLASS( CSearchWnd ) ) );	// No Search assume Browse

	if ( ( nDefaultFilter != NONE ) && ( (int)m_pResultFilters->m_nFilters >= nDefaultFilter + 1 ) )
	{
		m_sFilter			= m_pResultFilters->m_pFilters[nDefaultFilter]->m_sFilter;
		m_bFilterBusy		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterBusy;
		m_bFilterPush		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterPush;
		m_bFilterUnstable	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterUnstable;
		m_bFilterReject		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterReject;
		m_bFilterLocal		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterLocal;
		m_bFilterBogus		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterBogus;
		m_bFilterDRM		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterDRM;
		m_bFilterRestricted	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterRestricted;
		m_bFilterSuspicious	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterSuspicious;
		m_bFilterAdult		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterAdult;
		m_nFilterMinSize	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_nFilterMinSize;
		m_nFilterMaxSize	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_nFilterMaxSize;
		m_nFilterSources	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_nFilterSources;
		m_bRegExp			= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bRegExp;
	}
	else
	{
		m_bRegExp			= ( Settings.Search.FilterMask & ( 1 << 0 ) ) > 0;	// 0;
		m_bFilterPush		= ( Settings.Search.FilterMask & ( 1 << 1 ) ) > 0;	// 0;
		m_bFilterUnstable	= ( Settings.Search.FilterMask & ( 1 << 2 ) ) > 0;	// 0;
		m_bFilterBusy		= ( Settings.Search.FilterMask & ( 1 << 3 ) ) > 0;	// 0;
		m_bFilterLocal		= ( Settings.Search.FilterMask & ( 1 << 4 ) ) > 0;	// 0;
		m_bFilterReject		= ( Settings.Search.FilterMask & ( 1 << 5 ) ) > 0 || bBrowsing;	// 0;
		m_bFilterBogus		= ( Settings.Search.FilterMask & ( 1 << 6 ) ) > 0;	// 0;
		m_bFilterDRM		= ( Settings.Search.FilterMask & ( 1 << 7 ) ) > 0;	// 1;
		m_bFilterRestricted	= ( Settings.Search.FilterMask & ( 1 << 8 ) ) > 0;	// 0;
		m_bFilterSuspicious	= ( Settings.Search.FilterMask & ( 1 << 9 ) ) > 0;	// 1;
		m_bFilterAdult		= ( Settings.Search.FilterMask & ( 1 << 10 ) ) > 0;	// 0;
		m_nFilterMinSize	= 1;
		m_nFilterMaxSize	= 0;
		m_nFilterSources	= 1;
	}

	m_sRegexPattern.Empty();

	m_pSizeMap	= new CMatchFile*[ MAP_SIZE ];
	m_pMapSHA1	= new CMatchFile*[ MAP_SIZE ];
	m_pMapTiger	= new CMatchFile*[ MAP_SIZE ];
	m_pMapED2K	= new CMatchFile*[ MAP_SIZE ];
	m_pMapBTH	= new CMatchFile*[ MAP_SIZE ];
	m_pMapMD5	= new CMatchFile*[ MAP_SIZE ];

	ClearUpdated();

	ZeroMemory( m_pSizeMap, MAP_SIZE * sizeof *m_pSizeMap );
	ZeroMemory( m_pMapSHA1, MAP_SIZE * sizeof *m_pMapSHA1 );
	ZeroMemory( m_pMapTiger, MAP_SIZE * sizeof *m_pMapTiger );
	ZeroMemory( m_pMapED2K, MAP_SIZE * sizeof *m_pMapED2K );
	ZeroMemory( m_pMapBTH, MAP_SIZE * sizeof *m_pMapBTH );
	ZeroMemory( m_pMapMD5, MAP_SIZE * sizeof *m_pMapMD5 );

	SetSortColumn( MATCH_COL_COUNT, TRUE );
}

CMatchList::~CMatchList()
{
	Clear();

	delete [] m_pColumns;
	delete [] m_pszFilter;

	delete [] m_pMapED2K;
	delete [] m_pMapTiger;
	delete [] m_pMapSHA1;
	delete [] m_pMapBTH;
	delete [] m_pMapMD5;
	delete [] m_pSizeMap;

	delete [] m_pFiles;

	delete m_pResultFilters;
}

//////////////////////////////////////////////////////////////////////
// CMatchList update stats

void CMatchList::UpdateStats()
{
	m_nItems = 0;
	m_nFilteredFiles = 0;
	m_nFilteredHits = 0;
	m_nGnutellaHits = 0;
	m_nED2KHits = 0;

	for ( DWORD i = 0 ; i < m_nFiles ; ++i )
	{
		if ( DWORD nItemCount = m_pFiles[ i ]->GetItemCount() )
		{
			m_nItems += nItemCount;
			m_nFilteredFiles ++;
			m_nFilteredHits += m_pFiles[ i ]->m_nFiltered;
			for ( CQueryHit* pHit = m_pFiles[ i ]->GetHits() ; pHit ; pHit = pHit->m_pNext )
			{
				switch ( pHit->m_nProtocol )
				{
				case PROTOCOL_G1:
				case PROTOCOL_G2:
					m_nGnutellaHits ++;
					break;
				case PROTOCOL_ED2K:
					m_nED2KHits ++;
					break;
				//default:
				//	;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CMatchList add hits

void CMatchList::AddHits(const CQueryHit* pHits, const CQuerySearch* pFilter)
{
	CSingleLock pLock( &m_pSection, TRUE );
	CMatchFile** pMap = NULL;
	for ( const CQueryHit* pNext = pHits ; pNext ; pNext = pNext->m_pNext )
	{
		// Empty file names mean a hit for the currently downloading file.
		// We just clicked the search result while the search was in progress.
		// The size may be zero or match the size of the file.
		// Empty file names are caught by the next clause and deleted.

		if ( Security.IsDenied( &pNext->m_pAddress ) ||
			 Security.IsDenied( pNext ) ||
			 pNext->m_sName.IsEmpty() ||		// Empty name
			 pNext->m_nSize == 0 ||				// size is 0
			 pNext->m_nSize == SIZE_UNKNOWN )	// size is SIZE_UNKNOWN (0xFFFFFFFFFFFFFFFF)
			// NOTE: Because Gnutella without GGEP "LF" extension can only handle up to 4GB-1B size
			// (can be 2GB-1 depending on sign handling of the value), large files can have SIZE_UNKNOWN value
			// if no GGEP "LF" was on QueryHit packet. Because of current implementation of list
			// (always added to filesize maps without check) these hits have to be rejected.
		{
			// Skip bad hit
			continue;
		}

		CQueryHit* pHit = new CQueryHit( *pNext );
		if ( ! pHit )
			break;	// Out of memory

		pHit->m_bNew = m_bNew;

		if ( pFilter )
		{
			// ToDo: pHit->m_bExactMatch is broken anyway,
			// since its m_sKeywords has no punctuation marks.
			// Consider this:
			//
			// pHit->m_sKeywords = pHit->m_sName;
			// pFilter->MakeKeywords( pHit->m_sKeywords, false );
			//
			// Then we can tokenize strings to word lists by
			// using space as a separator, sort 2 lists and compare.
			//
			// Spammers abuse hits using such technique:
			// "Your keywords (bla-blah)" and also "Your keywords [bla-blah]"
			//
			// Thus, we can filter them if we get two or more
			// of the same keyword lists from one user.

			if ( _tcsistr( pFilter->m_sKeywords, pHit->m_sName ) == 0 )
				pHit->m_bExactMatch = TRUE;

			pHit->m_bMatched = pFilter->Match( pHit->m_sName,
				( pHit->m_pSchema ? pHit->m_pSchema->GetURI() : NULL ), pHit->m_pXML, pHit );

			// ToDo: Change to pHit->m_bMatched when we will be able to get folder name from hits.
			// Envy/Shareaza sends hits if folder name matches the search keywords too.
			// For now, just move such files to bogus. (Unwise?)
			if ( Settings.Search.SchemaTypes && pFilter->m_pSchema )
			{
				if ( ! pHit->m_bMatched && pFilter->m_pSchema->Equals( pHit->m_pSchema ) )
					pHit->m_bBogus = TRUE;
				else
					pHit->m_bBogus = ! pFilter->m_pSchema->FilterType( pHit->m_sName );
			}
		}
		else
		{
			pHit->m_bMatched = TRUE;
		}

		FilterHit( pHit );

		CMatchFile* pFile = NULL;
		FILESTATS Stats = {};

		if ( pHit->m_oSHA1 )
			pFile = FindFileAndAddHit( pHit, fSHA1, &Stats );
		if ( pFile == NULL && pHit->m_oTiger )
			pFile = FindFileAndAddHit( pHit, fTiger, &Stats );
		if ( pFile == NULL && pHit->m_oED2K )
			pFile = FindFileAndAddHit( pHit, fED2K, &Stats );
		if ( pFile == NULL && pHit->m_oBTH )
			pFile = FindFileAndAddHit( pHit, fBTH, &Stats );
		if ( pFile == NULL && pHit->m_oMD5 )
			pFile = FindFileAndAddHit( pHit, fMD5, &Stats );

		if ( pFile == NULL
			&& ( ( ! pHit->m_oSHA1 && ! pHit->m_oTiger && ! pHit->m_oED2K && ! pHit->m_oBTH && ! pHit->m_oMD5 )
				|| ! Settings.General.HashIntegrity ) )
		{
			pFile = FindFileAndAddHit( pHit, fSize, &Stats );
		}

		if ( pFile )	// New hit for an existing file
		{
			pMap = m_pFiles;
			for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pMap++ )
			{
				if ( *pMap == pFile )
				{
					if ( m_nSortColumn >= 0 )
					{
						UpdateRange( m_nFiles - nCount );
						MoveMemory( pMap, pMap + 1, ( nCount - 1 ) * sizeof *pMap );
						m_nFiles--;
						InsertSorted( pFile );
					}
					else
					{
						UpdateRange( m_nFiles - nCount, m_nFiles - nCount );
					}
					break;
				}
			}
		}
		else	// New file hit
		{
			pFile = new CMatchFile( this, pHit );
			if ( ! pFile )	// Out of memory
			{
				delete pHit;
				break;
			}

			if ( Security.IsDenied( pFile ) )	// Non-regex/address filters
				continue;

			pFile->m_bNew = m_bNew;

			if ( m_nFiles + 1 > m_nBuffer )
			{
				if ( CMatchFile** pFiles = new CMatchFile*[ m_nBuffer + BUFFER_GROW ] )
				{
					if ( m_pFiles )
					{
						CopyMemory( pFiles, m_pFiles, m_nFiles * sizeof( CMatchFile* ) );
						delete [] m_pFiles;
					}
					m_nBuffer += BUFFER_GROW;
					m_pFiles = pFiles;
				}
				else	// Out of memory
				{
					delete pFile;
					break;
				}
			}

			pMap = m_pSizeMap + (DWORD)( pFile->m_nSize & 0xFF );
			pFile->m_pNextSize = *pMap;
			*pMap = pFile;

			if ( m_nSortColumn >= 0 )
			{
				InsertSorted( pFile );
			}
			else
			{
				UpdateRange( m_nFiles );
				m_pFiles[ m_nFiles++ ] = pFile;
			}
		}

		if ( ! Stats.bHadSHA1 && pFile->m_oSHA1 )
		{
			pMap = m_pMapSHA1 + pFile->m_oSHA1[ 0 ];
			pFile->m_pNextSHA1 = *pMap;
			*pMap = pFile;
		}
		if ( ! Stats.bHadTiger && pFile->m_oTiger )
		{
			pMap = m_pMapTiger + pFile->m_oTiger[ 0 ];
			pFile->m_pNextTiger = *pMap;
			*pMap = pFile;
		}
		if ( ! Stats.bHadED2K && pFile->m_oED2K )
		{
			pMap = m_pMapED2K + pFile->m_oED2K[ 0 ];
			pFile->m_pNextED2K = *pMap;
			*pMap = pFile;
		}
		if ( ! Stats.bHadBTH && pFile->m_oBTH )
		{
			pMap = m_pMapBTH + pFile->m_oBTH[ 0 ];
			pFile->m_pNextBTH = *pMap;
			*pMap = pFile;
		}
		if ( ! Stats.bHadMD5 && pFile->m_oMD5 )
		{
			pMap = m_pMapMD5 + pFile->m_oMD5[ 0 ];
			pFile->m_pNextMD5 = *pMap;
			*pMap = pFile;
		}
	}

	UpdateStats();
}

CMatchFile* CMatchList::FindFileAndAddHit(CQueryHit* pHit, const findType nFindFlag, FILESTATS* Stats)
{
	CMatchFile** pMap;

	switch ( nFindFlag )
	{
	case fSHA1:
		pMap = m_pMapSHA1 + pHit->m_oSHA1[ 0 ];
		break;
	case fTiger:
		pMap = m_pMapTiger + pHit->m_oTiger[ 0 ];
		break;
	case fED2K:
		pMap = m_pMapED2K + pHit->m_oED2K[ 0 ];
		break;
	case fBTH:
		pMap = m_pMapBTH + pHit->m_oBTH[ 0 ];
		break;
	case fMD5:
		pMap = m_pMapMD5 + pHit->m_oMD5[ 0 ];
		break;
	case fSize:
		pMap = m_pSizeMap + (DWORD)( pHit->m_nSize & 0xFF );
		break;
	default:
		return NULL;
	}

	for ( CMatchFile* pSeek = *pMap ; pSeek ; )
	{
		bool bValid = false;

		switch ( nFindFlag )
		{
		case fSHA1:
			bValid = validAndEqual( pSeek->m_oSHA1, pHit->m_oSHA1 );
			break;
		case fTiger:
			bValid = validAndEqual( pSeek->m_oTiger, pHit->m_oTiger );
			break;
		case fED2K:
			bValid = validAndEqual( pSeek->m_oED2K, pHit->m_oED2K );
			break;
		case fBTH:
			bValid = validAndEqual( pSeek->m_oBTH, pHit->m_oBTH );
			break;
		case fMD5:
			bValid = validAndEqual( pSeek->m_oMD5, pHit->m_oMD5 );
			break;
		case fSize:
			bValid = pSeek->m_nSize && pHit->m_bSize && ( pSeek->m_nSize == pHit->m_nSize );
			break;
		}

		if ( bValid )
		{
			bool bHadSHA1 = pSeek->m_oSHA1;
			bool bHadTiger = pSeek->m_oTiger;
			bool bHadED2K = pSeek->m_oED2K;
			bool bHadBTH = pSeek->m_oBTH;
			bool bHadMD5 = pSeek->m_oMD5;

			if ( pSeek->Add( pHit, ( nFindFlag != fSize ) ) )
			{
				Stats->bHadSHA1	 |= bHadSHA1;
				Stats->bHadTiger |= bHadTiger;
				Stats->bHadED2K	 |= bHadED2K;
				Stats->bHadBTH	 |= bHadBTH;
				Stats->bHadMD5	 |= bHadMD5;

				return pSeek;
			}
			//else
				// ToDo: Equal hashes for different files or bad hit
		}

		if ( nFindFlag == fSize )
		{
			Stats->bHadSHA1 = false;
			Stats->bHadTiger = false;
			Stats->bHadED2K = false;
			Stats->bHadBTH = false;
			Stats->bHadMD5 = false;
		}

		switch ( nFindFlag )
		{
		case fSHA1:
			pSeek = pSeek->m_pNextSHA1;
			break;
		case fTiger:
			pSeek = pSeek->m_pNextTiger;
			break;
		case fED2K:
			pSeek = pSeek->m_pNextED2K;
			break;
		case fBTH:
			pSeek = pSeek->m_pNextBTH;
			break;
		case fMD5:
			pSeek = pSeek->m_pNextMD5;
			break;
		case fSize:
			pSeek = pSeek->m_pNextSize;
			break;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CMatchList insert to a sorted array

void CMatchList::InsertSorted(CMatchFile* pFile)
{
	int nFirst = 0;
	for ( int nLast = m_nFiles - 1 ; nLast >= nFirst ; )
	{
		DWORD nMiddle = ( nFirst + nLast ) >> 1;

		if ( pFile->Compare( m_pFiles[ nMiddle ] ) == m_bSortDir )
			nFirst = nMiddle + 1;
		else
			nLast = nMiddle - 1;
	}

	MoveMemory( m_pFiles + nFirst + 1, m_pFiles + nFirst, ( m_nFiles - nFirst ) * sizeof *m_pFiles );
	m_pFiles[ nFirst ] = pFile;
	m_nFiles++;
	UpdateRange( nFirst );
}

//////////////////////////////////////////////////////////////////////
// CMatchList searching

DWORD CMatchList::FileToItem(CMatchFile* pFile)
{
	CSingleLock pLock( &m_pSection, TRUE );

	CMatchFile** pLoop = m_pFiles;

	for ( DWORD nCount = 0 ; nCount < m_nFiles ; nCount++, pLoop++ )
	{
		if ( *pLoop == pFile ) return nCount;
	}

	return 0xFFFFFFFF;
}

//////////////////////////////////////////////////////////////////////
// CMatchList clear

void CMatchList::Clear()
{
	CSingleLock pLock( &m_pSection, TRUE );

	m_pSelectedFiles.RemoveAll();
	m_pSelectedHits.RemoveAll();

	ZeroMemory( m_pSizeMap, MAP_SIZE * sizeof *m_pSizeMap );
	ZeroMemory( m_pMapSHA1, MAP_SIZE * sizeof *m_pMapSHA1 );
	ZeroMemory( m_pMapTiger, MAP_SIZE * sizeof *m_pMapTiger );
	ZeroMemory( m_pMapED2K, MAP_SIZE * sizeof *m_pMapED2K );
	ZeroMemory( m_pMapBTH, MAP_SIZE * sizeof *m_pMapBTH );
	ZeroMemory( m_pMapMD5, MAP_SIZE * sizeof *m_pMapMD5 );

	if ( m_pFiles )
	{
		CMatchFile** pLoop = m_pFiles;

		for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pLoop++ )
		{
			if ( *pLoop ) delete (*pLoop);
		}
	}

	m_nFiles = 0;

	UpdateStats();
	UpdateRange();
}

//////////////////////////////////////////////////////////////////////
// CMatchList selection

BOOL CMatchList::Select(CMatchFile* pFile, CQueryHit* pHit, BOOL bSelected)
{
	if ( pHit != NULL )
	{
		if ( pHit->m_bSelected == bSelected ) return FALSE;
		pHit->m_bSelected = bSelected;

		if ( bSelected )
			m_pSelectedHits.AddTail( pHit );
		else
			m_pSelectedHits.RemoveAt( m_pSelectedHits.Find( pHit ) );
	}
	else if ( pFile != NULL )
	{
		if ( pFile->m_bSelected == bSelected ) return FALSE;
		pFile->m_bSelected = bSelected;

		if ( bSelected )
			m_pSelectedFiles.AddTail( pFile );
		else
			m_pSelectedFiles.RemoveAt( m_pSelectedFiles.Find( pFile ) );
	}

	if ( pFile != NULL )
	{
		DWORD nIndex = FileToItem( pFile );
		UpdateRange( nIndex, nIndex );
	}

	return TRUE;
}

CMatchFile* CMatchList::GetSelectedFile(BOOL bFromHit) const
{
	if ( m_pSelectedFiles.GetCount() != 1 )
	{
		if ( bFromHit == FALSE ) return NULL;

		CQueryHit* pHit = GetSelectedHit();
		if ( pHit == NULL ) return NULL;

		CMatchFile** pLoop = m_pFiles;

		for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pLoop++ )
		{
			if ( (*pLoop)->Check( pHit ) ) return *pLoop;
		}

		return NULL;
	}

	return m_pSelectedFiles.GetHead();
}

CQueryHit* CMatchList::GetSelectedHit() const
{
	if ( m_pSelectedHits.GetCount() != 1 )
	{
		if ( m_pSelectedFiles.GetCount() != 1 ) return NULL;
		CMatchFile* pFile = m_pSelectedFiles.GetHead();
		return ( pFile->m_nFiltered == 1 ) ? pFile->GetHits() : NULL;
	}

	return m_pSelectedHits.GetHead();
}

INT_PTR CMatchList::GetSelectedCount() const
{
	return m_pSelectedFiles.GetCount() + m_pSelectedHits.GetCount();
}

BOOL CMatchList::ClearSelection()
{
	CSingleLock pLock( &m_pSection, TRUE );

	CMatchFile** pLoop = m_pFiles;
	BOOL bChanged = FALSE;

	for ( DWORD nCount = 0 ; nCount < m_nFiles ; nCount++, pLoop++ )
	{
		bChanged = (*pLoop)->ClearSelection();
		if ( bChanged )
			UpdateRange( nCount, nCount );
	}

	m_pSelectedFiles.RemoveAll();
	m_pSelectedHits.RemoveAll();

	return bChanged;
}

CString CMatchList::CreateRegExpFilter(const CString& strPattern)
{
	m_sRegexPattern = strPattern;

	if ( strPattern.IsEmpty() )
		return m_sRegexPattern;

	CBaseMatchWnd* pParent = GetParent();
	if ( pParent && pParent->IsKindOf( RUNTIME_CLASS( CSearchWnd ) ) )
	{
		CQuerySearchPtr pQuery = static_cast< CSearchWnd* >( pParent )->GetLastSearch();
		if ( pQuery )
			m_sRegexPattern = pQuery->BuildRegExp( strPattern );
	}

	return m_sRegexPattern;
}

// Moved to QuerySearch:
//
//	CString strNewPattern;
//	bool bReplaced = false;
//
//	CSearchWnd* pParent = static_cast< CSearchWnd* >( GetParent() );
//	if ( ! pParent ) return false;
//
//	CQuerySearchPtr pQuery = pParent->GetLastSearch();
//	if ( ! pQuery ) return false;
//
//	LPCTSTR pszPattern = strPattern.GetBuffer();
//	int nTotal = 0;
//
//	while ( *pszPattern )
//	{
//		if ( *pszPattern == '<' )
//		{
//			pszPattern++;
//			bool bEnds = false;
//			bool bAll = ( *pszPattern == '%' || *pszPattern == '$' || *pszPattern == '_' || *pszPattern == '>' );
//
//			for ( ; *pszPattern ; pszPattern++ )
//			{
//				if ( *pszPattern == '>' )
//				{
//					bEnds = true;
//					break;
//				}
//			}
//
//			if ( bEnds )	// Closed '>'
//			{
//				CQuerySearch::const_iterator itWord = pQuery->begin();
//				CQuerySearch::const_iterator itWordEnd = pQuery->end();
//
//				if ( bAll )	// <%>,<$>,<_>,<>
//				{
//					// Add all keywords at the "< >" position
//					for ( ; itWord != itWordEnd ; itWord++ )
//					{
//						strNewPattern.AppendFormat( L"%s\\s*",
//							(LPCTSTR)CString( itWord->first, int(itWord->second) ) );
//					}
//					bReplaced = true;
//				}
//				else		// <1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>
//				{
//					pszPattern--;	// Go back
//					int nNumber = 0;
//
//					// Numbers from 1 to 9, no more
//					if ( _stscanf( &pszPattern[0], L"%i", &nNumber ) != 1 )
//						nNumber = ++nTotal;
//
//					// Add specified keyword at the "< >" position
//					for ( int nWord = 1 ; itWord != itWordEnd ; itWord++, nWord++ )
//					{
//						if ( nWord == nNumber )
//						{
//							strNewPattern.AppendFormat( L"%s\\s*",
//								(LPCTSTR)CString( itWord->first, int(itWord->second) ) );
//							bReplaced = true;
//							break;
//						}
//					}
//					pszPattern++;	// Return to the last position
//				}
//			}
//			else	// No closing '>'
//			{
//				return false;
//			}
//		}
//		else	// No replacing
//		{
//			strNewPattern += *pszPattern;
//		}
//		pszPattern++;
//	}

//////////////////////////////////////////////////////////////////////
// CMatchList filter

void CMatchList::Filter()
{
	CSingleLock pLock( &m_pSection, TRUE );

	Settings.Search.FilterMask = 0;

	if ( m_bRegExp )			Settings.Search.FilterMask |= ( 1 << 0 );	// 0;
	if ( m_bFilterPush )		Settings.Search.FilterMask |= ( 1 << 1 );	// 0;
	if ( m_bFilterUnstable )	Settings.Search.FilterMask |= ( 1 << 2 );	// 0;
	if ( m_bFilterBusy )		Settings.Search.FilterMask |= ( 1 << 3 );	// 0;
	if ( m_bFilterLocal )		Settings.Search.FilterMask |= ( 1 << 4 );	// 0;
	if ( m_bFilterReject )		Settings.Search.FilterMask |= ( 1 << 5 );	// 0;
	if ( m_bFilterBogus )		Settings.Search.FilterMask |= ( 1 << 6 );	// 0;
	if ( m_bFilterDRM )			Settings.Search.FilterMask |= ( 1 << 7 );	// 1;
	if ( m_bFilterRestricted )	Settings.Search.FilterMask |= ( 1 << 8 );	// 0;
	if ( m_bFilterSuspicious )	Settings.Search.FilterMask |= ( 1 << 9 );	// 1;
	if ( m_bFilterAdult )		Settings.Search.FilterMask |= ( 1 << 10 );	// 0;
//	if ( m_nFilterSources > 0 )	Settings.Search.FilterMask |= ( 1 << 12 );	// 1;
//	if ( m_nFilterMinSize > 0 )	Settings.Search.FilterMask |= ( 1 << 10 );	// 1;
//	if ( m_nFilterMaxSize > m_nFilterMinSize )	Settings.Search.FilterMask |= ( 1 << 11 );	// 0;

	delete [] m_pszFilter;
	m_pszFilter = NULL;

	if ( ! m_sFilter.IsEmpty() )
	{
		if ( m_bRegExp )
		{
			m_sFilter = CreateRegExpFilter( m_sFilter );
		}
		else
		{
			int nWordLen = 3;
			LPCTSTR pszPtr = m_sFilter;
			BOOL bQuote = FALSE;
			BOOL bNot = FALSE;

			CList< CString > pWords;

			int nStart = 0, nPos = 0;
			for ( ; *pszPtr ; nPos++, pszPtr++ )
			{
				if ( *pszPtr == '\"' || ( ! bQuote && ( *pszPtr == ' ' || *pszPtr == '\t' || *pszPtr == '-' ) ) )
				{
					if ( nStart < nPos )
					{
						pWords.AddTail( ( bNot ? '-' : '+' ) + m_sFilter.Mid( nStart, nPos - nStart ) );
						nWordLen += ( nPos - nStart ) + 2;
					}

					nStart = nPos + 1;

					if ( *pszPtr == '\"' )
						bQuote = ! bQuote;
					else if ( *pszPtr == '-' && ! bQuote )
						bNot = TRUE;

					if ( bNot && ! bQuote && *pszPtr != '-' )
						bNot = FALSE;
				}
			}

			if ( nStart < nPos )
			{
				pWords.AddTail( ( bNot ? '-' : '+' ) + m_sFilter.Mid( nStart, nPos - nStart ) );
				nWordLen += ( nPos - nStart ) + 2;
			}

			m_pszFilter = new TCHAR[ nWordLen ];
			LPTSTR pszFilter = m_pszFilter;

			for ( POSITION pos = pWords.GetHeadPosition() ; pos ; )
			{
				CString strWord = pWords.GetNext( pos );
				CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof( TCHAR ) * ( strWord.GetLength() + 1 ) );
				pszFilter += strWord.GetLength() + 1;
			}

			*pszFilter++ = 0;
			*pszFilter++ = 0;
		}
	}

	for ( DWORD i = 0 ; i < m_nFiles ; ++i )
	{
		if ( ! m_pFiles[ i ]->Filter() )
		{
			if ( m_pFiles[ i ]->m_bSelected )
				Select( m_pFiles[ i ], NULL, FALSE );
		}
	}

	UpdateStats();

	SetSortColumn( m_nSortColumn, m_bSortDir < 0 );
	UpdateRange();
}

//////////////////////////////////////////////////////////////////////
// CMatchList hit filtering

BOOL CMatchList::FilterHit(CQueryHit* pHit)
{
	pHit->m_bFiltered = FALSE;
	LPCTSTR pszName = pHit->m_sName;

	if ( pHit->m_nSpeed )
		pHit->m_sSpeed = Settings.SmartSpeed( pHit->m_nSpeed, KiloBytes );
	else
		pHit->m_sSpeed.Empty();

	CBaseMatchWnd* pParent = GetParent();
	if ( pParent && pParent->IsKindOf( RUNTIME_CLASS( CSearchWnd ) ) )
	{
		CQuerySearchPtr pQuery = static_cast< CSearchWnd* >( pParent )->GetLastSearch();
		if ( pQuery && Security.IsDenied( pQuery, pHit->m_sName ) )
			return FALSE;
	}

	if ( m_bRegExp && ! m_sRegexPattern.IsEmpty() )
	{
		if ( ! RegExp::Match( m_sRegexPattern, pHit->m_sName ) )
			return FALSE;
	}

	if ( ( pHit->m_oSHA1 && Security.IsDenied( pHit->m_oSHA1.toUrn() ) ) ||
		 ( pHit->m_oTiger && Security.IsDenied( pHit->m_oTiger.toUrn() ) ) ||
		 ( pHit->m_oED2K && Security.IsDenied( pHit->m_oED2K.toUrn() ) ) ||
		 ( pHit->m_oBTH && Security.IsDenied( pHit->m_oBTH.toUrn() ) ) ||
		 ( pHit->m_oMD5 && Security.IsDenied( pHit->m_oMD5.toUrn() ) ) )
		return FALSE;

	if ( ( m_bFilterBusy && pHit->m_bBusy == TRI_TRUE ) ||
	//	 ( m_bFilterPush && pHit->m_bPush == TRI_TRUE && pHit->m_nProtocol != PROTOCOL_ED2K ) ||
		 ( m_bFilterPush && pHit->m_bPush == TRI_TRUE ) ||
		 ( m_bFilterUnstable && pHit->m_bStable == TRI_FALSE ) ||
		 ( m_bFilterReject && pHit->m_bMatched == FALSE ) ||
		 ( m_bFilterBogus && pHit->m_bBogus ) ||
		 ( m_nFilterMinSize > 0 && pHit->m_nSize < m_nFilterMinSize ) ||
		 ( m_nFilterMaxSize > 0 && pHit->m_nSize > m_nFilterMaxSize ) )
		return FALSE;

	if ( m_pszFilter )
	{
		for ( LPCTSTR pszFilter = m_pszFilter ; *pszFilter ; )
		{
			if ( *pszFilter == '-' )
			{
				if ( _tcsistr( pszName, pszFilter + 1 ) != NULL )
					return FALSE;
			}
			else
			{
				if ( _tcsistr( pszName, pszFilter + 1 ) == NULL )
					return FALSE;
			}

			pszFilter += _tcslen( pszFilter + 1 ) + 2;
		}
	}

	// Global adult filter and Local adult filter
	if ( Settings.Search.AdultFilter || m_bFilterAdult )
	{
		if ( AdultFilter.IsHitAdult( pszName ) )
			return FALSE;
	}

	return ( pHit->m_bFiltered = TRUE );
}

//////////////////////////////////////////////////////////////////////
// CMatchList schema selection

void CMatchList::SelectSchema(CSchemaPtr pSchema, CList< CSchemaMember* >* pColumns)
{
	CSingleLock pLock( &m_pSection, TRUE );

	CMatchFile** pLoop = m_pFiles;

	for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pLoop++ )
	{
		if ( (*pLoop)->m_pColumns )
		{
			delete [] (*pLoop)->m_pColumns;
			(*pLoop)->m_pColumns = NULL;
			(*pLoop)->m_nColumns = 0;
		}
	}

	delete [] m_pColumns;
	m_pColumns	= NULL;
	m_nColumns	= 0;
	m_pSchema	= pSchema;

	if ( ! pSchema || ! pColumns ) return;

	m_pColumns = new CSchemaMember*[ pColumns->GetCount() ];

	for ( POSITION pos = pColumns->GetHeadPosition() ; pos ; )
	{
		m_pColumns[ m_nColumns++ ] = pColumns->GetNext( pos );
	}

	Filter();
}

//////////////////////////////////////////////////////////////////////
// CMatchList sort column selection

void CMatchList::SetSortColumn(int nColumn, BOOL bDirection)
{
	m_nSortColumn	= nColumn;
	m_bSortDir		= bDirection ? -1 : 1;

	if ( m_nSortColumn < 0 || ! m_nFiles ) return;

	int nFirst		= 0;
	int nLast		= m_nFiles - 1;
	DWORD nStack	= 0;

	int nStackFirst[128];
	int nStackLast[128];

	for ( ;; )
	{
		if ( nLast - nFirst <= 16 )
		{
			int nIndex;
			CMatchFile* pPrevious = m_pFiles[ nFirst ];
			CMatchFile* pCurrent;

			for ( nIndex = nFirst + 1 ; nIndex <= nLast ; ++nIndex )
			{
				pCurrent = m_pFiles[ nIndex ];

				if ( pPrevious->Compare( pCurrent ) == m_bSortDir )
				{
					int nIndex2;
					m_pFiles[ nIndex ] = pPrevious;

					for ( nIndex2 = nIndex - 1 ; nIndex2 > nFirst ; )
					{
						CMatchFile* pTemp = m_pFiles[ nIndex2 - 1 ];

						if ( pTemp->Compare( pCurrent ) == m_bSortDir )
							m_pFiles[ nIndex2-- ] = pTemp;
						else
							break;
					}
					m_pFiles[ nIndex2 ] = pCurrent;
				}
				else
				{
					pPrevious = pCurrent;
				}
			}
		}
		else
		{
			CMatchFile* pPivot;
			{
				CMatchFile* pTemp;
				DWORD nMiddle = ( nFirst + nLast ) >> 1;

				pTemp = m_pFiles[ nFirst ];
				if ( pTemp->Compare( m_pFiles[ nLast ] ) == m_bSortDir )
				{
					m_pFiles[ nFirst ] = m_pFiles[ nLast ];
					m_pFiles[ nLast ] = pTemp;
				}

				pTemp = m_pFiles[ nMiddle ];
				if ( m_pFiles[ nFirst ]->Compare( pTemp ) == m_bSortDir )
				{
					m_pFiles[ nMiddle ] = m_pFiles[ nFirst ];
					m_pFiles[ nFirst ] = pTemp;
				}

				pTemp = m_pFiles[ nLast ];
				if ( m_pFiles[ nMiddle ]->Compare( pTemp ) == m_bSortDir )
				{
					m_pFiles[ nLast ] = m_pFiles[ nMiddle ];
					m_pFiles[ nMiddle ] = pTemp;
				}
				pPivot = m_pFiles[ nMiddle ];
			}

			DWORD nUp;
			{
				DWORD nDown = nFirst;
				nUp = nLast;

				for ( ;; )
				{
					do
					{
						++nDown;
					} while ( pPivot->Compare( m_pFiles[ nDown ] ) == m_bSortDir );
					do
					{
						--nUp;
					} while ( m_pFiles[ nUp ]->Compare( pPivot ) == m_bSortDir );

					if ( nUp <= nDown )
						break;

					CMatchFile* pTemp = m_pFiles[ nDown ];
					m_pFiles[ nDown ] = m_pFiles[ nUp ];
					m_pFiles[ nUp ] = pTemp;
				}
			}

			if ( ( nUp - nFirst + 1 ) >= ( nLast - nUp ) )
			{
				nStackFirst[ nStack ]	= nFirst;
				nStackLast[ nStack++ ]	= nUp;

				nFirst = nUp + 1;
			}
			else
			{
				nStackFirst[ nStack ]	= nUp + 1;
				nStackLast[ nStack++ ]	= nLast;

				nLast = nUp;
			}

			continue;
		}

		if ( nStack <= 0 )
			break;

		nFirst = nStackFirst[ --nStack ];
		nLast = nStackLast[ nStack ];
	}

	UpdateRange();
}

//////////////////////////////////////////////////////////////////////
// CMatchList updates

void CMatchList::UpdateRange(DWORD nMin, DWORD nMax)
{
	m_nUpdateMin	= min( m_nUpdateMin, nMin );
	m_nUpdateMax	= max( m_nUpdateMax, nMax );
	m_bUpdated		= TRUE;
}

void CMatchList::ClearUpdated()
{
	m_nUpdateMin	= 0xFFFFFFFF;
	m_nUpdateMax	= 0;
	m_bUpdated		= FALSE;
}

//////////////////////////////////////////////////////////////////////
// CMatchList clear new

void CMatchList::ClearNew()
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( DWORD nFile = 0 ; nFile < m_nFiles ; nFile++ )
	{
		if ( m_pFiles[ nFile ] )
			m_pFiles[ nFile ]->ClearNew();
	}

	m_bNew = TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchList serialize

//#define MATCHLIST_SER_VERSION	1000	// 15	// MatchObjects.h
// nVersion History:
// 12 - Shareaza 2.2 (Rolandas)
// 13 - Shareaza 2.3 (ryo-oh-ki)
// 14 - Shareaza 2.4 (ryo-oh-ki)
// 15 - Added CQueryHit::m_sNick for DC++ hits (ryo-oh-ki) (Shareaza 2.5.5.0)
// 1000 - (Envy 1.0) (15)

void CMatchList::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << nVersion;

		ar << m_sFilter;
		ar << m_bFilterBusy;
		ar << m_bFilterPush;
		ar << m_bFilterUnstable;
		ar << m_bFilterReject;
		ar << m_bFilterLocal;
		ar << m_bFilterBogus;
		ar << m_bFilterDRM;
		ar << m_bFilterAdult;
		ar << m_bFilterSuspicious;
		ar << m_bRegExp;

		ar << m_nFilterMinSize;
		ar << m_nFilterMaxSize;
		ar << m_nFilterSources;
		ar << m_nSortColumn;
		ar << m_bSortDir;

		ar.WriteCount( m_nFiles );

		for ( DWORD nFile = 0 ; nFile < m_nFiles ; nFile++ )
		{
			CMatchFile* pFile = m_pFiles[ nFile ];
			pFile->Serialize( ar, nVersion );
		}
	}
	else // Loading
	{
		ar >> nVersion;
		if ( nVersion < 1000 )
			AfxThrowUserException();

		ar >> m_sFilter;
		ar >> m_bFilterBusy;
		ar >> m_bFilterPush;
		ar >> m_bFilterUnstable;
		ar >> m_bFilterReject;
		ar >> m_bFilterLocal;
		ar >> m_bFilterBogus;

		ar >> m_bFilterDRM;
		ar >> m_bFilterAdult;
		ar >> m_bFilterSuspicious;
		ar >> m_bRegExp;

		//if ( nVersion >= 10 )
		//{
			ar >> m_nFilterMinSize;
			ar >> m_nFilterMaxSize;
		//}
		//else
		//{
		//	DWORD nInt32;
		//	ar >> nInt32; m_nFilterMinSize = nInt32;
		//	ar >> nInt32; m_nFilterMaxSize = nInt32;
		//}

		ar >> m_nFilterSources;
		ar >> m_nSortColumn;
		ar >> m_bSortDir;

		m_nFiles = m_nBuffer = static_cast< DWORD >( ar.ReadCount() );
		m_pFiles = new CMatchFile*[ m_nFiles ];
		ZeroMemory( m_pFiles, sizeof( CMatchFile* ) * m_nFiles );

		for ( DWORD nFile = 0 ; nFile < m_nFiles ; nFile++ )
		{
			CMatchFile* pFile = new CMatchFile( this );
			m_pFiles[ nFile ] = pFile;
			pFile->Serialize( ar, nVersion );

			CMatchFile** pMap = m_pSizeMap + (DWORD)( pFile->m_nSize & 0xFF );
			pFile->m_pNextSize = *pMap;
			*pMap = pFile;

			if ( pFile->m_oSHA1 )
			{
				pMap = m_pMapSHA1 + pFile->m_oSHA1[ 0 ];
				pFile->m_pNextSHA1 = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_oTiger )
			{
				pMap = m_pMapTiger + pFile->m_oTiger[ 0 ];
				pFile->m_pNextTiger = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_oED2K )
			{
				pMap = m_pMapED2K + ( pFile->m_oED2K[ 0 ] );
				pFile->m_pNextED2K = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_oBTH )
			{
				pMap = m_pMapBTH + ( pFile->m_oBTH[ 0 ] );
				pFile->m_pNextBTH = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_oMD5 )
			{
				pMap = m_pMapMD5 + ( pFile->m_oMD5[ 0 ] );
				pFile->m_pNextMD5 = *pMap;
				*pMap = pFile;
			}
		}

		Filter();
	}
}

void CMatchList::SanityCheck()
{
	for ( DWORD nFile = 0 ; nFile < m_nFiles ; )
	{
		CMatchFile* pFile = m_pFiles[ nFile ];

		pFile->SanityCheck();

		// If file lost all of its hits then delete it
		if ( pFile->GetTotalHitsCount() == 0 )
		{
			// Remove file from Files array
			m_nFiles--;
			if ( nFile < m_nFiles )
			{
				MoveMemory( m_pFiles + nFile, m_pFiles + nFile + 1,
					( m_nFiles - nFile ) * sizeof *m_pFiles );
				UpdateRange( nFile );
			}
			else
				m_pFiles[ nFile ] = NULL;

			delete pFile;
		}
		else
			nFile++;
	}
}


//////////////////////////////////////////////////////////////////////
// CMatchFile construction

CMatchFile::CMatchFile(CMatchList* pList, CQueryHit* pHit)
	: m_pList			( pList )
	, m_pHits			( NULL )
	, m_pBest			( NULL )
	, m_nTotal			( 0 )
	, m_nFiltered		( 0 )
	, m_nSources		( 0 )
	, m_pNextSize		( NULL )
	, m_pNextSHA1		( NULL )
	, m_pNextTiger		( NULL )
	, m_pNextED2K		( NULL )
	, m_pNextBTH		( NULL )
	, m_pNextMD5		( NULL )
	, m_bBusy			( TRI_UNKNOWN )
	, m_bPush			( TRI_UNKNOWN )
	, m_bStable 		( TRI_UNKNOWN )
	, m_bPreview		( FALSE )
	, m_nSpeed			( 0 )
	, m_nRating 		( 0 )
	, m_nRated			( 0 )
	, m_bDRM			( FALSE )
	, m_bRestricted 	( TRI_UNKNOWN )
	, m_bSuspicious 	( FALSE )
	, m_bCollection 	( FALSE )
	, m_bTorrent		( FALSE )
	, m_bSelected		( FALSE )
	, m_bExpanded		( Settings.Search.ExpandMatches )
	, m_bExisting		( TRI_UNKNOWN )
	, m_bDownload		( FALSE )
	, m_bNew			( FALSE )
	, m_bOneValid		( FALSE )
	, m_nShellIndex		( -1 )
	, m_nColumns		( 0 )
	, m_pColumns		( NULL )
	, m_nPreview		( 0 )
	, m_pPreview 		( NULL )
	, m_pTime			( CTime::GetCurrentTime() )
{
	// ToDo: Change to SIZE_UNKNOWN without the size
	m_nSize = ( pHit && pHit->m_bSize ) ? pHit->m_nSize : 0;
	m_sSize = Settings.SmartVolume( m_nSize );

	if ( pHit ) Add( pHit );
}

CMatchFile::~CMatchFile()
{
	// Remove file from Size Map
	if ( m_pList && m_pList->m_pSizeMap )
	{
		CMatchFile** pMap = m_pList->m_pSizeMap + (DWORD)( m_nSize & 0xFF );
		CMatchFile* pPrevFile = NULL;
		for ( CMatchFile* pFile = *pMap ; pFile ; pFile = pFile->m_pNextSize )
		{
			if ( this == pFile )
			{
				if ( pPrevFile )
					pPrevFile->m_pNextSize = m_pNextSize;
				else
					*pMap = m_pNextSize;
				break;
			}
			pPrevFile = pFile;
		}
	}

	// Remove file from SHA1 Map
	if ( m_oSHA1 && m_pList && m_pList->m_pMapSHA1 )
	{
		CMatchFile** pMap = m_pList->m_pMapSHA1 + m_oSHA1[ 0 ];
		CMatchFile* pPrevFile = NULL;
		for ( CMatchFile* pFile = *pMap ; pFile ; pFile = pFile->m_pNextSHA1 )
		{
			if ( this == pFile )
			{
				if ( pPrevFile )
					pPrevFile->m_pNextSHA1 = m_pNextSHA1;
				else
					*pMap = m_pNextSHA1;
				break;
			}
			pPrevFile = pFile;
		}
	}

	// Remove file from Tiger Map
	if ( m_oTiger && m_pList && m_pList->m_pMapTiger )
	{
		CMatchFile** pMap = m_pList->m_pMapTiger + m_oTiger[ 0 ];
		CMatchFile* pPrevFile = NULL;
		for ( CMatchFile* pFile = *pMap ; pFile ; pFile = pFile->m_pNextTiger )
		{
			if ( this == pFile )
			{
				if ( pPrevFile )
					pPrevFile->m_pNextTiger = m_pNextTiger;
				else
					*pMap = m_pNextTiger;
				break;
			}
			pPrevFile = pFile;
		}
	}

	// Remove file from ED2K Map
	if ( m_oED2K && m_pList && m_pList->m_pMapED2K )
	{
		CMatchFile** pMap = m_pList->m_pMapED2K + m_oED2K[ 0 ];
		CMatchFile* pPrevFile = NULL;
		for ( CMatchFile* pFile = *pMap ; pFile ; pFile = pFile->m_pNextED2K )
		{
			if ( this == pFile )
			{
				if ( pPrevFile )
					pPrevFile->m_pNextED2K = m_pNextED2K;
				else
					*pMap = m_pNextED2K;
				break;
			}
			pPrevFile = pFile;
		}
	}

	// Remove file from BTH Map
	if ( m_oBTH && m_pList && m_pList->m_pMapBTH )
	{
		CMatchFile** pMap = m_pList->m_pMapBTH + m_oBTH[ 0 ];
		CMatchFile* pPrevFile = NULL;
		for ( CMatchFile* pFile = *pMap ; pFile ; pFile = pFile->m_pNextBTH )
		{
			if ( this == pFile )
			{
				if ( pPrevFile )
					pPrevFile->m_pNextBTH = m_pNextBTH;
				else
					*pMap = m_pNextBTH;
				break;
			}
			pPrevFile = pFile;
		}
	}

	// Remove file from MD5 Map
	if ( m_oMD5 && m_pList && m_pList->m_pMapMD5 )
	{
		CMatchFile** pMap = m_pList->m_pMapMD5 + m_oMD5[ 0 ];
		CMatchFile* pPrevFile = NULL;
		for ( CMatchFile* pFile = *pMap ; pFile ; pFile = pFile->m_pNextMD5 )
		{
			if ( this == pFile )
			{
				if ( pPrevFile )
					pPrevFile->m_pNextMD5 = m_pNextMD5;
				else
					*pMap = m_pNextMD5;
				break;
			}
			pPrevFile = pFile;
		}
	}

	// Remove all Hits
	if ( m_pHits )
	{
		for ( CQueryHit* pHit = m_pHits ; pHit ; )
		{
			CQueryHit* pNext = pHit->m_pNext;

			// Remove hit from selected hits
			if ( m_pList )
			{
				if ( POSITION pos = m_pList->m_pSelectedHits.Find( pHit ) )
					m_pList->m_pSelectedHits.RemoveAt( pos );
			}

			delete pHit;
			pHit = pNext;
		}
	}

	// Remove from selected files
	if ( m_pList )
	{
		if ( POSITION pos = m_pList->m_pSelectedFiles.Find( this ) )
			m_pList->m_pSelectedFiles.RemoveAt( pos );
	}

	delete [] m_pColumns;
	delete [] m_pPreview;
}

void CMatchFile::RefreshStatus()
{
#ifdef _DEBUG
	DWORD nTotal = 0;
#endif

	m_nRating	= 0;
	m_nRated	= 0;
	m_nFiltered	= 0;
	m_nSources	= 0;
	m_nSpeed	= 0;
	m_sSpeed.Empty();

	CMap< CString, const CString&, DWORD, DWORD > oNameRatings;
	oNameRatings.InitHashTable( GetBestHashTableSize( m_nTotal ) );

	DWORD nBestVote = 0;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_nRating )
		{
			m_nRating += pHit->m_nRating;
			m_nRated ++;
		}

		if ( pHit->m_bFiltered )
			m_nFiltered ++;
		//else
			// ToDo: Use for smarter filtered-count display?

		m_nSources = max( m_nSources, pHit->GetSources() );
		m_nSpeed += pHit->m_nSpeed;

		DWORD nVote = 0;
		oNameRatings.Lookup( pHit->m_sName, nVote );
		oNameRatings[ pHit->m_sName ] = ++nVote;
		if ( nVote > nBestVote )
		{
			nBestVote = nVote;
			m_sName = pHit->m_sName;
			m_sCompareName = m_sName;
			ToLower.Clean( m_sCompareName );
			m_sURL = pHit->m_sURL;
		}

#ifdef _DEBUG
		nTotal ++;
#endif
	}

	m_nSources = max( m_nSources, m_nTotal );

	if ( m_nFiltered && m_nSpeed )
		m_sSpeed = Settings.SmartSpeed( m_nSpeed / m_nFiltered, KiloBytes );

	ASSERT( m_nTotal == nTotal );
}

//////////////////////////////////////////////////////////////////////
// CMatchFile add

BOOL CMatchFile::Add(CQueryHit* pHit, BOOL bForce)
{
	if ( m_nSize == 0 && pHit->m_bSize )
		m_nSize = pHit->m_nSize;
	else if ( m_nSize != 0 && pHit->m_bSize && m_nSize != pHit->m_nSize )
		return FALSE;

	if ( ! bForce )
	{
		if ( m_oSHA1 && ( pHit->m_oSHA1 || Settings.General.HashIntegrity ) )
		{
			if ( ! pHit->m_oSHA1 )
				return FALSE;
			if ( validAndUnequal( m_oSHA1, pHit->m_oSHA1 ) )
				return FALSE;
			bForce = TRUE;
		}
		else if ( ! m_oSHA1 && pHit->m_oSHA1 && Settings.General.HashIntegrity && m_pHits )
		{
			return FALSE;
		}
	}

	BOOL bSubstituted = FALSE;

	if ( m_pHits )
	{
		for ( CQueryHit* pOld = m_pHits ; pOld ; pOld = pOld->m_pNext )
		{
			BOOL bName = _tcsicmp( pHit->m_sName, pOld->m_sName ) == 0;

			if ( ! bForce && bName ) bForce = TRUE;

			if ( bName && pHit->m_pAddress.S_un.S_addr == pOld->m_pAddress.S_un.S_addr &&
				 pHit->m_nPort == pOld->m_nPort )
			{
				*pOld = *pHit;
				delete pHit;
				pHit = pOld;
				bForce = bSubstituted = TRUE;
				break;
			}
		}

		if ( ! bForce )
			return FALSE;
	}

	if ( ! bSubstituted )
	{
		pHit->m_pNext = m_pHits;
		m_pHits = pHit;
		m_nTotal++;
	}

	if ( ! m_oSHA1 && pHit->m_oSHA1 )
		m_oSHA1 = pHit->m_oSHA1;

	if ( ! m_oTiger && pHit->m_oTiger )
		m_oTiger = pHit->m_oTiger;

	if ( ! m_oED2K && pHit->m_oED2K )
		m_oED2K = pHit->m_oED2K;

	if ( ! m_oBTH && pHit->m_oBTH )
		m_oBTH = pHit->m_oBTH;

	if ( ! m_oMD5 && pHit->m_oMD5 )
		m_oMD5 = pHit->m_oMD5;

	if ( ! m_bDownload && GetLibraryStatus() == TRI_UNKNOWN && HasHash() )
	{
		CSingleLock pLock2( &Transfers.m_pSection );
		if ( pLock2.Lock( 50 ) )
		{
			if ( ! m_bDownload && m_oSHA1 && Downloads.FindBySHA1( m_oSHA1 ) != NULL )
				m_bDownload = TRUE;
			else if ( ! m_bDownload && m_oTiger && Downloads.FindByTiger( m_oTiger ) != NULL )
				m_bDownload = TRUE;
			else if ( ! m_bDownload && m_oED2K && Downloads.FindByED2K( m_oED2K ) != NULL )
				m_bDownload = TRUE;
			else if ( ! m_bDownload && m_oBTH && Downloads.FindByBTH( m_oBTH ) != NULL )
				m_bDownload = TRUE;
			else if ( ! m_bDownload && m_oMD5 && Downloads.FindByMD5( m_oMD5 ) != NULL )
				m_bDownload = TRUE;
		}
	}

	if ( pHit->m_bFiltered )
		Added( pHit );

	if ( ! m_bOneValid && ! pHit->m_bBogus && pHit->m_bMatched )
		m_bOneValid = TRUE;

	RefreshStatus();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile check

BOOL CMatchFile::Check(CQueryHit* pHit) const
{
	for ( CQueryHit* pOld = m_pHits ; pOld ; pOld = pOld->m_pNext )
	{
		if ( pOld == pHit )
			return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile expand/collapse

BOOL CMatchFile::Expand(BOOL bExpand)
{
	if ( m_bExpanded == bExpand ) return FALSE;

	m_pList->m_nItems -= GetItemCount();
	m_bExpanded = bExpand;
	m_pList->m_nItems += GetItemCount();

	if ( ! m_bExpanded )
	{
		for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			if ( pHit->m_bSelected )
				m_pList->Select( this, pHit, FALSE );
		}
	}

	m_pList->UpdateRange( m_pList->FileToItem( this ) );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile filter

DWORD CMatchFile::Filter()
{
	m_bBusy			= TRI_UNKNOWN;
	m_bPush			= TRI_UNKNOWN;
	m_bStable		= TRI_UNKNOWN;
	m_bPreview		= FALSE;
	m_bDRM			= FALSE;
	m_bSuspicious	= FALSE;
	m_bCollection	= FALSE;
	m_bTorrent		= FALSE;
	m_pBest			= NULL;

	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( m_pList->FilterHit( pHit ) )
			Added( pHit );
		else if ( pHit->m_bSelected )
			m_pList->Select( this, pHit, FALSE );
	}

	RefreshStatus();

	return GetItemCount();
}

//////////////////////////////////////////////////////////////////////
// CMatchFile averaging

void CMatchFile::Added(CQueryHit* pHit)
{
	m_pBest = pHit;

	if ( pHit->GetSources() > 0 )
	{
		if ( pHit->m_bPush == TRI_FALSE )
			m_bPush = TRI_FALSE;
		else if ( pHit->m_bPush == TRI_TRUE && m_bPush == TRI_UNKNOWN )
			m_bPush = TRI_TRUE;

		if ( pHit->m_bBusy == TRI_FALSE )
			m_bBusy = TRI_FALSE;
		else if ( pHit->m_bBusy == TRI_TRUE && m_bBusy == TRI_UNKNOWN )
			m_bBusy = TRI_TRUE;

		if ( pHit->m_bStable == TRI_TRUE )
			m_bStable = TRI_TRUE;
		else if ( pHit->m_bStable == TRI_FALSE && m_bStable == TRI_UNKNOWN )
			m_bStable = TRI_FALSE;

		m_bPreview |= pHit->m_bPreview;
	}

	m_bCollection |= ( pHit->m_bCollection && ! pHit->m_bBogus );

	if ( m_nShellIndex == -1 )
		m_nShellIndex = ShellIcons.Get( pHit->m_sName, 16 );

	const BOOL bSchema = m_pList->m_pSchema && m_pList->m_pSchema->Equals( pHit->m_pSchema );

	if ( bSchema || pHit->m_oSHA1 )
	{
		if ( m_pColumns == NULL )
		{
			m_nColumns = m_pList->m_nColumns;
			m_pColumns = new CString[ m_nColumns ];
		}

		CSchemaMember** pMember = m_pList->m_pColumns;
		CString strValue;

		for ( int nCount = 0 ; nCount < m_nColumns ; nCount ++, pMember ++ )
		{
			if ( _tcsicmp( (*pMember)->m_sName, L"SHA1" ) == 0 )
			{
				if ( pHit->m_oSHA1 )
					m_pColumns[ nCount ] = m_oSHA1.toString();
			}
			else if ( bSchema )
			{
				strValue = (*pMember)->GetValueFrom( pHit->m_pXML, NULL, TRUE );
				if ( ! strValue.IsEmpty() )
					m_pColumns[ nCount ] = strValue;
			}
		}
	}
	else if ( m_pColumns != NULL )
	{
		delete [] m_pColumns;
		m_pColumns = NULL;
		m_nColumns = 0;
	}

	if ( pHit->m_pXML != NULL )
	{
		if ( ! m_bDRM && ! pHit->m_pXML->GetAttributeValue( L"DRM" ).IsEmpty() )
		{
			CString strTag = pHit->m_pXML->GetAttributeValue( L"DRM" );
			ToLower( strTag );
			if ( strTag != L"0" && strTag != L"false" && strTag != L"no" )
				m_bDRM = TRUE;
		}

		if ( m_bRestricted == TRI_UNKNOWN &&
				( ! pHit->m_pXML->GetAttributeValue( L"ShareTag" ).IsEmpty() ||
				  ! pHit->m_pXML->GetAttributeValue( L"Permissive" ).IsEmpty() ) )
		{
			CString strTag = pHit->m_pXML->GetAttributeValue( L"ShareTag" );
			if ( strTag.IsEmpty() ) strTag = pHit->m_pXML->GetAttributeValue( L"Permissive" );

			ToLower( strTag );
			if ( strTag == L"1" || strTag == L"true" || strTag == L"yes" )
				m_bRestricted = TRI_FALSE;
			else if ( strTag == L"0" || strTag == L"false" || strTag == L"no" )
				m_bRestricted = TRI_TRUE;
			//else if ( strTag == L"?" || strTag == L"unknown" )
			//	m_bRestricted = TRI_UNKNOWN;
		}
		else if ( m_bRestricted == TRI_UNKNOWN &&
				! pHit->m_pXML->GetAttributeValue( L"Restricted" ).IsEmpty() )
		{
			CString strTag = pHit->m_pXML->GetAttributeValue( L"Restricted" );

			ToLower( strTag );
			if ( strTag == L"1" || strTag == L"true" || strTag == L"yes" )
				m_bRestricted = TRI_TRUE;
			else if ( strTag == L"0" || strTag == L"false" || strTag == L"no" )
				m_bRestricted = TRI_FALSE;
			//else if ( strTag == L"?" || strTag == L"unknown" )
			//	m_bPermissive = TRI_UNKNOWN;
		}
	}

	// Cross-packet spam filtering
	DWORD nBogusCount = 0;
	DWORD nTotal = 0;
	if ( ! Settings.Experimental.LAN_Mode )
	{
		for ( CQueryHit* pFileHits = m_pHits ; pFileHits ; pFileHits = pFileHits->m_pNext, nTotal++ )
		{
			if ( pFileHits->m_pNext && validAndEqual( pFileHits->m_oClientID, pFileHits->m_pNext->m_oClientID ) )
				pFileHits->m_bBogus = TRUE;
			if ( pFileHits->m_bBogus )
				nBogusCount++;
		}

		// Mark/unmark a file as suspicious depending on the percentage of the spam hits
		m_bSuspicious = (float)nBogusCount / nTotal > Settings.Search.SpamFilterThreshold / 100.0f;

		// Unshared files are suspicious. (A user is assumed to want to exclude these entirely)
		if ( CLibrary::IsBadFile( m_sName ) )
			m_bSuspicious = TRUE;
	}

	// Get extension
	if ( int nExt = pHit->m_sName.ReverseFind( L'.' ) + 1 )
	{
		LPCTSTR pszExt = (LPCTSTR)pHit->m_sName + nExt;

		// Set torrent bool
		if ( ( _tcsicmp( pszExt, L"torrent" ) == 0 ) )
			m_bTorrent = TRUE;

		// BASIC SPAM DETECTION:
		// Check if file is suspicious
		if ( ! m_bSuspicious )
		{
			// Viral or useless filetypes
			if ( ( _tcsicmp( pszExt, L"vbs" ) == 0 ) ||
				 ( _tcsicmp( pszExt, L"lnk" ) == 0 ) ||
				 ( _tcsicmp( pszExt, L"pif" ) == 0 ) )
			{
				m_bSuspicious = TRUE;
			}
			else if ( m_nSize < 90 * 1024 )
			{
				// Small filesize spam/viral
				if ( ( _tcsicmp( pszExt, L"exe" ) == 0 ) ||
					 ( _tcsicmp( pszExt, L"com" ) == 0 ) ||
					 ( _tcsicmp( pszExt, L"scr" ) == 0 ) ||
					 ( _tcsicmp( pszExt, L"avi" ) == 0 ) ||
					 ( _tcsicmp( pszExt, L"mpg" ) == 0 ) ||
					 ( _tcsicmp( pszExt, L"mov" ) == 0 ) ||
					 ( _tcsicmp( pszExt, L"wmv" ) == 0 ) ||
					 ( _tcsicmp( pszExt, L"wma" ) == 0 ) )
				{
					m_bSuspicious = TRUE;
				}
			}

			// Common exact search hit basic spam detection  (ToDo: Move most to filters + Keep updated)
			if ( ! m_bSuspicious && pHit->m_bExactMatch && m_nFiltered < 12 )
			{
				if ( _tcsicmp( pszExt, L"zip" ) == 0 )						// .zip
				{
					if ( m_nSize > 200 * 1024 && m_nSize < 501 * 1024 &&
						m_nFiltered > 1 && pHit->GetSources() > 4 && pHit->GetSources() < 10 )		// Cluster
					{
						m_bSuspicious = TRUE;
					}

					if ( ( m_nSize < 4 * 1024 )	//||
			//			( m_nSize > 210 * 1024 && m_nSize < 215 * 1024 ) ||		// 211kb
			//			( m_nSize > 501 * 1024 && m_nSize < 503 * 1024 ) ||		// 501kb
			//			( m_nSize > 648 * 1024 && m_nSize < 652 * 1024 ) ||		// 651kb
			//			( m_nSize > 697 * 1024 && m_nSize < 699 * 1024 ) ||		// 698kb
			//			( m_nSize > 735 * 1024 && m_nSize < 737 * 1024 ) ||		// 736kb
			//			( m_nSize > 3490 * 1024 && m_nSize < 3500 * 1024 ) )	// 3.41mb
						)
					{
						m_bSuspicious = TRUE;
					}
				}
			//	else if ( _tcsicmp( pszExt, L"wma" ) == 0 )					//.wma
			//	{
			//		if ( ( m_nSize > 525 * 1024 && m_nSize < 530 * 1024 ) ||	// 528kb
			//			 ( m_nSize > 1054 * 1024 && m_nSize < 1064 * 1024 ) ||	// 1.03mb
			//			 ( m_nSize > 1088 * 1024 && m_nSize < 1092 * 1024 ) ||	// 1.06mb
			//			 ( m_nSize > 4690 * 1024 && m_nSize < 4694 * 1024 ) )	// 4.58mb
			//			m_bSuspicious = TRUE;
			//	}
			//	else if ( _tcsicmp( pszExt, L"mov" ) == 0 )					//.mov
			//	{
			//		if ( ( m_nSize > 206 * 1024 && m_nSize < 208 * 1024 ) ||	// 207kb
			//			 ( m_nSize > 3333 * 1024 && m_nSize < 3344 * 1024 ) )	// 3.26mb
			//			m_bSuspicious = TRUE;
			//	}
			//	else if ( ( _tcsicmp( pszExt, L"au" ) == 0 ) ||				//.au
			//		( _tcsicmp( pszExt, L"snd" ) == 0 ) )					//.snd
			//	{
			//		if ( m_nSize > 4400 * 1024 && m_nSize < 5800 * 1024 )		// 4-5 mb
			//			m_bSuspicious = TRUE;
			//	}
			}
		}
	}

	if ( m_bDownload )
		pHit->m_bDownload = TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile clear new

void CMatchFile::ClearNew()
{
	m_bNew = FALSE;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
		pHit->m_bNew = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile compare (for sort)

int CMatchFile::Compare(CMatchFile* pFile) const
{
	LPCTSTR pszA, pszB;
	register int x, y;		// returns x = 1,0,-1

	//if ( m_bCollection != pFile->m_bCollection )
	//	return m_bCollection ? -m_pList->m_bSortDir : m_pList->m_bSortDir;

	switch ( m_pList->m_nSortColumn )
	{
	case MATCH_COL_NAME:
		x = m_sCompareName.Compare( pFile->m_sCompareName );
		break;

	case MATCH_COL_TYPE:
		pszA = _tcsrchr( m_sName, '.' );
		pszB = _tcsrchr( pFile->m_sName, '.' );
		if ( pszA && pszB )
			x = _tcsicmp( pszA, pszB );
		else
			x = ( ! pszA ? pszB ? -1 : 0 : 1 );
		break;

	case MATCH_COL_SIZE:
		x = ( m_nSize == pFile->m_nSize ) ? 0 : ( ( m_nSize > pFile->m_nSize ) ? 1 : -1 );
		break;

	case MATCH_COL_COUNT:
		x = ( m_nFiltered == pFile->m_nFiltered ) ? 0 : ( ( m_nFiltered > pFile->m_nFiltered ) ? 1 : -1 );
		// ToDo: Add secondary sorting value to m_nSources ?
		break;

	case MATCH_COL_SPEED:
		x = m_nFiltered ? m_nSpeed / m_nFiltered : 0;
		y = pFile->m_nFiltered ? pFile->m_nSpeed / pFile->m_nFiltered : 0;
		x = ( x == y ) ? 0 : ( ( x > y ) ? 1 : -1 );
		break;

	case MATCH_COL_RATING:
		x = m_nRated ? m_nRating / m_nRated : 0;
		y = pFile->m_nRated ? pFile->m_nRating / pFile->m_nRated : 0;
		x = ( x == y ) ? 0 : ( ( x > y ) ? 1 : -1 );
		break;

	case MATCH_COL_STATUS:
		x = y = 0;
		if ( m_bPush != TRI_TRUE ) x += 4;
		if ( m_bBusy != TRI_TRUE ) x += 2;
		if ( m_bStable == TRI_TRUE ) x ++;
		if ( pFile->m_bPush != TRI_TRUE ) y += 4;
		if ( pFile->m_bBusy != TRI_TRUE ) y += 2;
		if ( pFile->m_bStable == TRI_TRUE ) y ++;
		x = ( x == y ) ? 0 : ( ( x > y ) ? 1 : -1 );
		break;

	case MATCH_COL_CLIENT:
		pszA = ( m_nFiltered == 1 ) ? (LPCTSTR)m_pHits->m_pVendor->m_sName : NULL;
		pszB = ( pFile->m_nFiltered == 1 ) ? (LPCTSTR)pFile->m_pHits->m_pVendor->m_sName : NULL;
		if ( pszA && pszB )
			x = _tcsicmp( pszA, pszB );
		else
			x = ( ! pszA ? pszB ? -1 : 0 : 1 );
		break;

	case MATCH_COL_COUNTRY:
		pszA = ( m_nFiltered == 1 ) ? (LPCTSTR)m_pHits->m_sCountry : NULL;
		pszB = ( pFile->m_nFiltered == 1 ) ? (LPCTSTR)pFile->m_pHits->m_sCountry : NULL;
		if ( pszA && pszB )
			x = _tcsicmp( pszA, pszB );
		else
			x = ( ! pszA ? pszB ? -1 : 0 : 1 );
		break;

	case MATCH_COL_TIME:
		x = ( m_pTime == pFile->m_pTime ) ? 0 : ( m_pTime > pFile->m_pTime ? 1 : -1 );
		break;

	default:
		if ( m_pColumns && pFile->m_pColumns )
		{
			x = ( m_pList->m_nSortColumn - MATCH_COL_MAX >= m_nColumns );
			y = ( m_pList->m_nSortColumn - MATCH_COL_MAX >= pFile->m_nColumns );
			if ( x && y )
			{
				pszA = m_pColumns[ m_pList->m_nSortColumn - MATCH_COL_MAX ];
				pszB = pFile->m_pColumns[ m_pList->m_nSortColumn - MATCH_COL_MAX ];
				x = CLiveList::SortProc( pszA, pszB );
			}
			else
			{
				x = ( ! x ? y ? -1 : 0 : 1 );
			}
		}
		else
		{
			x = ( ( ! m_pColumns ) ? ( pFile->m_pColumns ) ? -1 : 0 : 1 );
		}

		break;
	}

	if ( x == 0 && m_pList->m_nSortColumn != MATCH_COL_NAME )
	{
		x = m_sCompareName.Compare( pFile->m_sCompareName );
		if ( x == 0 && m_pList->m_nSortColumn != MATCH_COL_COUNT )
			x = ( m_nSources == pFile->m_nSources ) ? 0 : ( ( m_nSources > pFile->m_nSources ) ? -1 : 1 );

		if ( m_pList->m_bSortDir == -1 )
			x = - x;
	}

	return ( x > 0 ) ? 1 : ( x < 0 ) ? -1 : 0;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile serialize

void CMatchFile::Serialize(CArchive& ar, int nVersion)	// MATCHLIST_SER_VERSION
{
	if ( ar.IsStoring() )
	{
		ar << m_nSize;
		ar << m_sSize;
		SerializeOut( ar, m_oSHA1 );
		SerializeOut( ar, m_oTiger );
		SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oBTH );
		SerializeOut( ar, m_oMD5 );

		ar << m_bBusy;
		ar << m_bPush;
		ar << m_bStable;
		ar << m_nSpeed;
		ar << m_sSpeed;
		ar << m_bExpanded;
		ar << m_bExisting;
		ar << m_bDownload;
		ar << m_bOneValid;

		if ( m_pPreview == NULL ) m_nPreview = 0;
		ar.WriteCount( m_nPreview );
		if ( m_nPreview > 0 ) ar.Write( m_pPreview, m_nPreview );

		ar.WriteCount( m_nTotal );
		CArray< CQueryHit* > pHits;

		CQueryHit* pHit = m_pHits;
		for ( ; pHit ; pHit = pHit->m_pNext )
		{
			pHits.Add( pHit );
		}

		for ( int nHit = m_nTotal - 1 ; nHit >= 0 ; nHit-- )
		{
			pHit = pHits.GetAt( nHit );
			pHit->Serialize( ar, nVersion );
		}

		ar << m_pTime;
	}
	else // Loading
	{
		ar >> m_nSize;
		ar >> m_sSize;
		SerializeIn( ar, m_oSHA1, nVersion );
		SerializeIn( ar, m_oTiger, nVersion );
		SerializeIn( ar, m_oED2K, nVersion );

		//if ( nVersion >= 13 )
		//{
			SerializeIn( ar, m_oBTH, nVersion );
			SerializeIn( ar, m_oMD5, nVersion );
		//}

		ar >> m_bBusy;
		ar >> m_bPush;
		ar >> m_bStable;
		ar >> m_nSpeed;
		ar >> m_sSpeed;
		ar >> m_bExpanded;
		ar >> m_bExisting;
		ar >> m_bDownload;
		ar >> m_bOneValid;

		if ( ( m_nPreview = static_cast< DWORD >( ar.ReadCount() ) ) != 0 )
		{
			m_pPreview = new BYTE[ m_nPreview ];
			ReadArchive( ar, m_pPreview, m_nPreview );
		}

		m_nTotal = static_cast< DWORD >( ar.ReadCount() );

		for ( int nCount = m_nTotal ; nCount > 0 ; nCount-- )
		{
			CQueryHit* pNext = new CQueryHit( PROTOCOL_NULL );
			pNext->m_pNext = m_pHits;
			m_pHits = pNext;
			m_pHits->Serialize( ar, nVersion );
		}

		//if ( nVersion >= 14 )
			ar >> m_pTime;

		RefreshStatus();
	}
}

CQueryHit* CMatchFile::GetHits() const
{
	return m_pHits;
}

CQueryHit*	CMatchFile::GetBest() const
{
	return m_pBest;
}

//int CMatchFile::GetRating() const
//{
//	int nRating = 0;
//	if ( m_bPush != TRI_TRUE ) nRating += 4;
//	if ( m_bBusy != TRI_TRUE ) nRating += 2;
//	if ( m_bStable == TRI_TRUE ) nRating ++;
//	return nRating;
//}

DWORD CMatchFile::GetBogusHitsCount() const
{
	DWORD nBogusCount = 0;
	for ( CQueryHit* pHits = m_pHits ; pHits ; pHits = pHits->m_pNext )
	{
		if ( pHits->m_bBogus )
			nBogusCount++;
	}
	return nBogusCount;
}

DWORD CMatchFile::GetTotalHitsCount() const
{
	return m_nTotal;
}

DWORD CMatchFile::GetTotalHitsSpeed() const
{
	return m_nSpeed;
}

// Obsolete: (r9172)
//void CMatchFile::AddHitsToDownload(CDownload* pDownload, BOOL bForce) const
//{
//	// Best goes first if forced
//	if ( bForce && m_pBest != NULL )
//		pDownload->AddSourceHit( m_pBest, TRUE );
//
//	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
//	{
//		if ( bForce )
//		{
//			// Best already added
//			if ( pHit != m_pBest )
//				pDownload->AddSourceHit( pHit, TRUE );
//		}
//		else
//		{
//			pDownload->AddSourceHit( pHit );
//		}
//
//		// Send any reviews to the download, so they can be viewed later
//		if ( pHit->IsRated() )
//			pDownload->AddReview( &pHit->m_pAddress, 2, pHit->m_nRating, pHit->m_sNick, pHit->m_sComments );
//	}
//}

void CMatchFile::AddHitsToXML(CXMLElement* pXML) const
{
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_pXML != NULL )
		{
			for ( POSITION pos = pHit->m_pXML->GetAttributeIterator() ; pos ; )
			{
				CXMLAttribute* pAttribute = pHit->m_pXML->GetNextAttribute( pos );
				pXML->AddAttribute( pAttribute->GetName(), pAttribute->GetValue() );
			}
		}
	}
}

CSchemaPtr CMatchFile::GetHitsSchema() const
{
	CSchemaPtr pSchema = NULL;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		pSchema = pHit->m_pSchema;
		if ( pSchema ) break;
	}
	return pSchema;
}

CSchemaPtr CMatchFile::AddHitsToMetadata(CMetaList& oMetadata) const
{
	CSchemaPtr pSchema = GetHitsSchema();
	if ( pSchema )
	{
		oMetadata.Setup( pSchema );

		for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			if ( pHit->m_pXML && pSchema->Equals( pHit->m_pSchema ) )
				oMetadata.Combine( pHit->m_pXML );
		}
	}
	return pSchema;
}

BOOL CMatchFile::AddHitsToPreviewURLs(CList<CString>& oPreviewURLs) const
{
	BOOL bCanPreview = FALSE;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_oSHA1 && pHit->m_bPush == TRI_FALSE )
		{
			if ( pHit->m_bPreview )
			{
				oPreviewURLs.AddTail( pHit->m_sPreview );
				bCanPreview = TRUE;
			}
//#ifdef _DEBUG
			else if (	_tcsistr( pHit->m_sName, L".avi" ) ||
						_tcsistr( pHit->m_sName, L".mpg" ) ||
						_tcsistr( pHit->m_sName, L".mpeg" ) ||
						_tcsistr( pHit->m_sName, L".jpg" ) ||
						_tcsistr( pHit->m_sName, L".jpeg" ) )
			{
				CString strURL;
				strURL.Format( L"http://%s:%i/gnutella/preview/v1?%s",
					(LPCTSTR)CString( inet_ntoa( pHit->m_pAddress ) ), pHit->m_nPort,
					(LPCTSTR)pHit->m_oSHA1.toUrn() );
				oPreviewURLs.AddTail( strURL );
				bCanPreview = TRUE;
			}
//#endif  // Debug?
		}
	}
	return bCanPreview;
}

void CMatchFile::AddHitsToReviews(CList < Review* >& oReviews) const
{
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->IsRated() )
		{
			oReviews.AddTail( new Review( pHit->m_oClientID,
				&pHit->m_pAddress, pHit->m_sNick, pHit->m_nRating, pHit->m_sComments ) );
		}
	}
}

void CMatchFile::SetBogus( BOOL bBogus )
{
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		pHit->m_bBogus = bBogus;
	}
}

BOOL CMatchFile::ClearSelection()
{
	BOOL bChanged = FALSE;
	if ( m_bSelected )
	{
		m_bSelected = FALSE;
		bChanged = TRUE;
	}
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_bSelected )
		{
			pHit->m_bSelected = FALSE;
			bChanged = TRUE;
		}
	}
	return bChanged;
}

BOOL CMatchFile::IsValid() const
{
	return ( m_pBest != NULL );
}

DWORD CMatchFile::GetBestPartial() const
{
	return ( m_pBest ? m_pBest->m_nPartial : 0 );
}

int CMatchFile::GetBestRating() const
{
	return ( m_pBest ? m_pBest->m_nRating : 0 );
}

IN_ADDR CMatchFile::GetBestAddress() const
{
	const IN_ADDR foo = { 0xff, 0xff, 0xff, 0xff };
	return ( m_pBest ? m_pBest->m_pAddress : foo );
}

LPCTSTR CMatchFile::GetBestVendorName() const
{
	return ( ( m_pBest && m_pBest->m_pVendor ) ? m_pBest->m_pVendor->m_sName : L"" );
}

LPCTSTR CMatchFile::GetBestCountry() const
{
	return ( m_pBest ? m_pBest->m_sCountry : L"" );
}

CSchemaPtr CMatchFile::GetBestSchema() const
{
	return ( m_pBest ? m_pBest->m_pSchema : NULL );
}

TRISTATE CMatchFile::GetBestMeasured() const
{
	return ( m_pBest ? m_pBest->m_bMeasured : TRI_UNKNOWN );
}

BOOL CMatchFile::GetBestBrowseHost() const
{
	return ( m_pBest && m_pBest->m_bBrowseHost );
}

void CMatchFile::GetPartialTip(CString& sPartial) const
{
	if ( m_nFiltered == 1 && m_pBest && m_pBest->m_nPartial )
		sPartial.Format( LoadString( IDS_TIP_PARTIAL ), 100.0f * (float)m_pBest->m_nPartial / (float)m_nSize );
	else
		sPartial.Empty();
}

void CMatchFile::GetQueueTip(CString& sQueue) const
{
	if ( m_nFiltered == 1 && m_pBest && m_pBest->m_nUpSlots )
		sQueue.Format( LoadString( IDS_TIP_QUEUE ), m_pBest->m_nUpSlots, max( 0, m_pBest->m_nUpQueue - m_pBest->m_nUpSlots ) );
	else
		sQueue.Empty();
}

void CMatchFile::GetUser(CString& sUser) const
{
	if ( m_nFiltered == 1 && m_pBest )
	{
		if ( ! m_pBest->m_sNick.IsEmpty() )
		{
			sUser.Format( L"%s (%s - %s)",
				(LPCTSTR)m_pBest->m_sNick,
				(LPCTSTR)CString( inet_ntoa( m_pBest->m_pAddress ) ),
				(LPCTSTR)m_pBest->m_pVendor->m_sName );
		}
		else if ( ( m_pBest->m_nProtocol == PROTOCOL_ED2K ) && ( m_pBest->m_bPush == TRI_TRUE ) )
		{
			sUser.Format( L"%lu@%s - %s", m_pBest->m_oClientID.begin()[2],
				(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)*m_pBest->m_oClientID.begin() ) ),
				(LPCTSTR)m_pBest->m_pVendor->m_sName );
		}
		else
		{
			sUser.Format( L"%s - %s",
				(LPCTSTR)CString( inet_ntoa( m_pBest->m_pAddress ) ),
				(LPCTSTR)m_pBest->m_pVendor->m_sName );
		}
	}
	else
	{
		sUser.Empty();
	}
}

void CMatchFile::GetStatusTip(CString& sStatus, COLORREF& crStatus)
{
	sStatus.Empty();

	if ( GetLibraryStatus() != TRI_UNKNOWN )
	{
		CLibraryFile* pExisting = NULL;

		CQuickLock oLock( Library.m_pSection );
		if ( pExisting == NULL && m_oSHA1 )
			pExisting = LibraryMaps.LookupFileBySHA1( m_oSHA1 );
		if ( pExisting == NULL && m_oTiger )
			pExisting = LibraryMaps.LookupFileByTiger( m_oTiger );
		if ( pExisting == NULL && m_oED2K )
			pExisting = LibraryMaps.LookupFileByED2K( m_oED2K );
		if ( pExisting == NULL && m_oBTH )
			pExisting = LibraryMaps.LookupFileByBTH( m_oBTH );
		if ( pExisting == NULL && m_oMD5 )
			pExisting = LibraryMaps.LookupFileByMD5( m_oMD5 );

		if ( pExisting != NULL )
		{
			if ( pExisting->IsAvailable() )
			{
				LoadString( sStatus, IDS_TIP_EXISTS_LIBRARY );
				crStatus = Colors.m_crTextStatus;
			}
			else
			{
				LoadString( sStatus, IDS_TIP_EXISTS_DELETED );
				crStatus = Colors.m_crTextAlert;

				if ( ! pExisting->m_sComments.IsEmpty() )
				{
					sStatus += L" (";
					sStatus += pExisting->m_sComments;
					sStatus.Replace( L"\r\n", L"; " );

					int nLen = sStatus.GetLength();
					if ( nLen > 150 )
					{
						// Truncate string including the last word
						// but no more than 150 characters plus punctuation
						CString str( sStatus.Left( 150 ) );
						if ( IsCharacter( sStatus.GetAt( 151 ) ) )
						{
							nLen = str.ReverseFind( L' ' );
							sStatus = nLen == -1 ? str : str.Left( nLen );
						}
						sStatus += L"\x2026)";
					}
					else
						sStatus.Append( L")" );
				}
			}
		}
	}
	else if ( m_bDownload || ( m_pBest && m_pBest->m_bDownload ) )
	{
		LoadString( sStatus, IDS_TIP_EXISTS_DOWNLOAD );
		crStatus = Colors.m_crSearchQueued;
	}
	else if ( ! m_bOneValid || ( m_pBest && m_pBest->m_bBogus ) )
	{
		LoadString( sStatus, IDS_TIP_BOGUS );
		crStatus = Colors.m_crTextAlert;
	}
}

TRISTATE CMatchFile::GetLibraryStatus()
{
	// ToDo: Is some cache needed?

	CSingleLock pLock( &Library.m_pSection );
	if ( pLock.Lock( 100 ) )
	{
		CLibraryFile* pExisting;

		if ( m_oSHA1 )
			pExisting = LibraryMaps.LookupFileBySHA1( m_oSHA1 );
		else if ( m_oTiger )
			pExisting = LibraryMaps.LookupFileByTiger( m_oTiger );
		else if ( m_oED2K )
			pExisting = LibraryMaps.LookupFileByED2K( m_oED2K );
		else if ( m_oMD5 )
			pExisting = LibraryMaps.LookupFileByMD5( m_oMD5 );
		else if ( m_oBTH )
			pExisting = LibraryMaps.LookupFileByBTH( m_oBTH );
		else
			pExisting = NULL;

		if ( pExisting )
			m_bExisting = pExisting->IsAvailable() ? TRI_FALSE : TRI_TRUE;
		else if ( m_bExisting == TRI_FALSE )
			m_bExisting = TRI_TRUE;

		pLock.Unlock();
	}
	return m_bExisting;
}

void CMatchFile::SanityCheck()
{
	const BOOL bBadFile = Security.IsDenied( this );	// Hash/etc. more efficient but undercounted here
	int nBadCount = 0;

	CQueryHit* pHitPrev = NULL;
	for ( CQueryHit* pHit = m_pHits ; pHit ; )
	{
		CQueryHit* pNext = pHit->m_pNext;

		if ( Security.IsDenied( &pHit->m_pAddress ) || bBadFile )
		{
			if ( bBadFile && ++nBadCount > 1 )
				Security.IsDenied( this );				// Duplicate check for hitcount

			// Exclude from hits list
			if ( pHitPrev )
				pHitPrev->m_pNext = pNext;
			else
				m_pHits = pNext;
			m_nTotal--;

			// Remove hit from selected hits
			if ( m_pList )
			{
				if ( POSITION pos = m_pList->m_pSelectedHits.Find( pHit ) )
					m_pList->m_pSelectedHits.RemoveAt( pos );
			}

			// Remove from best hit
			if ( m_pBest == pHit )
				m_pBest = m_pHits;

			delete pHit;
		}
		else
			pHitPrev = pHit;

		pHit = pNext;
	}

	RefreshStatus();
}

void CMatchFile::Ban(int nBanLength)
{
	// Ban by hit host IPs
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
		pHit->Ban( nBanLength );

	// Ban hashes
	Security.Ban( this, nBanLength );
}
