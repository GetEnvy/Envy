//
// MatchObjects.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2012
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

// Set at INTERNAL_VERSION on change:
#define MATCHLIST_SER_VERSION 1

// History:
// 12 - Shareaza 2.2 (Rolandas)
// 13 - Shareaza 2.3 (ryo-oh-ki)
// 14 - Shareaza 2.4 (ryo-oh-ki)
// 15 - Shareaza 2.5.5.0 (ryo-oh-ki)  Added CQueryHit::m_sNick for DC++ hits
// 1000 - (15)
// 1  - Envy 1.0

#include "EnvyFile.h"
#include "Schema.h"

class CSchemaMember;
class CQuerySearch;
class CQueryHit;
class CMatchFile;
class CResultFilters;
class CDownload;
class CMetaList;
class CXMLElement;
class CBaseMatchWnd;
class Review;

typedef struct
{
	bool	bHadSHA1;
	bool	bHadTiger;
	bool	bHadED2K;
	bool	bHadBTH;
	bool	bHadMD5;
} FILESTATS;

class CMatchList
{
public:
	CMatchList(CBaseMatchWnd* pParent);
	virtual ~CMatchList();

public:
	CMutex			m_pSection;
	CString			m_sFilter;
	BOOL			m_bFilterPush;
	BOOL			m_bFilterUnstable;
	BOOL			m_bFilterBusy;
	BOOL			m_bFilterLocal;
	BOOL			m_bFilterReject;
	BOOL			m_bFilterBogus;
	BOOL			m_bFilterDRM;
	BOOL			m_bFilterRestricted;
	BOOL			m_bFilterSuspicious;
	BOOL			m_bFilterAdult;
	BOOL			m_bRegExp;
	CString			m_sRegexPattern;
	QWORD			m_nFilterMinSize;
	QWORD			m_nFilterMaxSize;
	DWORD			m_nFilterSources;
	int				m_nSortColumn;
	BOOL			m_bSortDir;
	CSchemaPtr		m_pSchema;
	BOOL			m_bNew;
	CResultFilters*	m_pResultFilters;
	CMatchFile**	m_pFiles;				// File list
	DWORD			m_nFiles;				// File list size
	DWORD			m_nItems;				// Total visible items (filtered files + expanded hits)
	DWORD			m_nFilteredFiles;		// Total post-filter files
	DWORD			m_nFilteredHits;		// Total post-filter file hits
	DWORD			m_nGnutellaHits;		// Total post-filter file hits from G1/G2
	DWORD			m_nED2KHits;			// Total post-filter file hits from ED2K
	BOOL			m_bUpdated;
	DWORD			m_nUpdateMin;
	DWORD			m_nUpdateMax;
	CList< CMatchFile* > m_pSelectedFiles;
	CList< CQueryHit* > m_pSelectedHits;
protected:
	DWORD			m_nBuffer;
	CMatchFile**	m_pSizeMap;
	CMatchFile**	m_pMapSHA1;
	CMatchFile**	m_pMapTiger;
	CMatchFile**	m_pMapED2K;
	CMatchFile**	m_pMapBTH;
	CMatchFile**	m_pMapMD5;
	LPTSTR			m_pszFilter;
	CBaseMatchWnd*	m_pParent;
	CSchemaMember**	m_pColumns;
	int				m_nColumns;

	enum findType
	{
		fSHA1	= 0,
		fTiger	= 1,
		fED2K	= 2,
		fSize	= 3,
		fBTH	= 4,
		fMD5	= 5
	};

public:
	void		UpdateStats();
	void		AddHits(const CQueryHit* pHits, const CQuerySearch* pFilter = NULL);
	DWORD		FileToItem(CMatchFile* pFile);
	BOOL		Select(CMatchFile* pFile, CQueryHit* pHit, BOOL bSelected = TRUE);
	CMatchFile*	GetSelectedFile(BOOL bFromHit = FALSE) const;
	CQueryHit*	GetSelectedHit() const;
	INT_PTR		GetSelectedCount() const;
	BOOL		ClearSelection();
	void		Clear();
	void		Filter();
	CString		CreateRegExpFilter(const CString& strPattern);
	void		SelectSchema(CSchemaPtr pSchema, CList< CSchemaMember* >* pColumns);
	void		SetSortColumn(int nColumn = -1, BOOL bDirection = FALSE);
	void		UpdateRange(DWORD nMin = 0, DWORD nMax = 0xFFFFFFFF);
	void		ClearUpdated();
	void		ClearNew();
	void		SanityCheck();
	void		Serialize(CArchive& ar, int nVersion = MATCHLIST_SER_VERSION);

	CBaseMatchWnd* GetParent() const
	{
		return m_pParent;
	}

protected:
	CMatchFile* FindFileAndAddHit(CQueryHit* pHit, const findType nFindFlag, FILESTATS* Stats);
	void		InsertSorted(CMatchFile* pFile);
	BOOL		FilterHit(CQueryHit* pHit);

	friend class CMatchFile;
};


class CMatchFile : public CEnvyFile
{
public:
	CMatchFile(CMatchList* pList, CQueryHit* pHit = NULL);
	virtual ~CMatchFile();

public:
	CString		m_sCompareName;			// Filename for comparison (clean lowercase)
	CMatchList*	m_pList;
	DWORD		m_nTotal;
	DWORD		m_nFiltered;
	DWORD		m_nSources;
	CMatchFile*	m_pNextSize;
	CMatchFile*	m_pNextSHA1;
	CMatchFile*	m_pNextTiger;
	CMatchFile*	m_pNextED2K;
	CMatchFile*	m_pNextBTH;
	CMatchFile*	m_pNextMD5;
	CString		m_sSize;
	TRISTATE	m_bBusy;
	TRISTATE	m_bPush;
	TRISTATE	m_bStable;
	BOOL		m_bPreview;
	DWORD		m_nSpeed;
	CString		m_sSpeed;
	int			m_nRating;				// Total value of all ratings
	int			m_nRated;				// Number of ratings received
	BOOL		m_bDRM;					// Appears to have DRM
	TRISTATE	m_bRestricted;			// Appears to be a Restricted copyright file, or known Permissive File (Inverse ShareTag)
	BOOL		m_bSuspicious;			// Appears to be a suspicious file (small exe, vbs, etc)
	BOOL		m_bCollection;			// Appears to be a collection
	BOOL		m_bTorrent;				// Appears to be a torrent
	BOOL		m_bExpanded;
	BOOL		m_bSelected;
	BOOL		m_bDownload;
	BOOL		m_bNew;
	BOOL		m_bOneValid;
	int			m_nShellIndex;
	int			m_nColumns;
	CString*	m_pColumns;
	BYTE*		m_pPreview;
	DWORD		m_nPreview;
	CTime		m_pTime;				// Found time

	BOOL		Add(CQueryHit* pHit, BOOL bForce = FALSE);
	BOOL		Check(CQueryHit* pHit) const;
	BOOL		Expand(BOOL bExpand = TRUE);
	inline int	Compare(CMatchFile* pFile) const;
	void		Serialize(CArchive& ar, int nVersion = 0);	// MATCHLIST_SER_VERSION
	void		Ban(int nBanLength);	// Ban by hashes and by hit host IPs

	inline DWORD GetFilteredCount()
	{
		return ( ! m_pList || ! m_pBest ||
			( m_pList->m_bFilterDRM && m_bDRM ) ||
			( m_pList->m_bFilterSuspicious && m_bSuspicious ) ||
		//	( m_pList->m_bFilterRestricted && m_bRestricted == TRI_TRUE ) ||
			( m_pList->m_nFilterSources > m_nSources ) ||
			( m_pList->m_bFilterLocal && GetLibraryStatus() == TRI_FALSE ) ) ?
			0 : m_nFiltered;
	}

	inline DWORD GetItemCount()
	{
		DWORD nFiltered = GetFilteredCount();
		if ( nFiltered == 0 )
			return 0;
		if ( nFiltered == 1 || ! m_bExpanded )
			return 1;

		return nFiltered + 1;
	}

//	int			GetRating() const;
	DWORD		Filter();
	void		ClearNew();

	CQueryHit*	GetHits() const;			// Access to Hits list first element.  Use with CAUTION, if Hit was changed call RefreshStatus().
	CQueryHit*	GetBest() const;			// Access to best Hit.  Use with CAUTION, if Hit was changed call RefreshStatus().
	void		RefreshStatus();			// Refresh file status with Hits list: m_sName, m_sURL, m_nRating, m_nRated, m_nFiltered, m_nSources, m_nSpeed, m_sSpeed
	DWORD		GetBogusHitsCount() const;	// Count bogus status setted Hits
	DWORD		GetTotalHitsCount() const;	// Count Hits
	DWORD		GetTotalHitsSpeed() const;	// Sum Hits speeds
	CSchemaPtr	GetHitsSchema() const;		// Get first available Hits Schema
	void		SetBogus( BOOL bBogus = TRUE );	// Change Hits bogus status
	BOOL		ClearSelection();			// Clear selection of file itself and all Hits
	BOOL		IsValid() const;			// File has hits
	DWORD		GetBestPartial() const;		// Get partial count of best Hit
	int			GetBestRating() const;		// Get rating of best Hit
	IN_ADDR		GetBestAddress() const;		// Get address of best Hit
	LPCTSTR		GetBestVendorName() const;	// Get vendor name of best Hit
	LPCTSTR		GetBestCountry() const;		// Get country code of best Hit
	CSchemaPtr	GetBestSchema() const;		// Get schema of best Hit
	TRISTATE	GetBestMeasured() const;	// Get measured of best Hit
	BOOL		GetBestBrowseHost() const;	// Get browse host flag of best Hit

	// Is this file known (Exists in Library)?
	// TRI_UNKNOWN	- Not
	// TRI_FALSE	- Yes
	// TRI_TRUE		- Yes, Ghost
	TRISTATE	GetLibraryStatus();

	// Get some data for interface
	void		GetQueueTip(CString& sPartial) const;
	void		GetPartialTip(CString& sQueue) const;
	void		GetUser(CString& sUser) const;
	void		GetStatusTip(CString& sStatus, COLORREF& crStatus);

	// Output some data
	void		AddHitsToXML(CXMLElement* pXML) const;
	CSchemaPtr	AddHitsToMetadata(CMetaList& oMetadata) const;
	BOOL		AddHitsToPreviewURLs(CList < CString > & oPreviewURLs) const;
	void		AddHitsToReviews(CList < Review* >& oReviews) const;

	void		SanityCheck();

protected:
	CQueryHit*	m_pHits;
	CQueryHit*	m_pBest;
	TRISTATE	m_bLibraryStatus;
	TRISTATE	m_bExisting;

	void		Added(CQueryHit* pHit);
};
