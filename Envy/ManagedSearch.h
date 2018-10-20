//
// ManagedSearch.h
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

#include "QuerySearch.h"

class CPacket;
class CNeighbour;
class CManagedSearch;

typedef CComObjectPtr< CManagedSearch > CSearchPtr;


class CManagedSearch : public CComObject
{
	DECLARE_DYNAMIC(CManagedSearch)

public:
	CManagedSearch(CQuerySearch* pSearch = NULL, int nPriority = 0);
protected:
	virtual ~CManagedSearch();

public:
	typedef CMap< DWORD, DWORD, DWORD, DWORD > CDwordDwordMap;

	enum { spHighest, spMedium, spLowest, spMax };

	inline CQuerySearchPtr GetSearch() const
	{
		return m_pSearch;
	}

	inline bool IsEqualGUID(const Hashes::Guid& oGUID) const
	{
		return m_pSearch && validAndEqual( m_pSearch->m_oGUID, oGUID );
	}

	inline CSchemaPtr GetSchema() const
	{
		return m_pSearch ? m_pSearch->m_pSchema : NULL;
	}

	inline bool IsActive() const
	{
		return ( m_bActive != FALSE );
	}

	inline void SetActive(BOOL bActive)
	{
		InterlockedExchange( (LONG*)&m_bActive, bActive );
	}

	inline void SetPriority(int nPriority)
	{
		InterlockedExchange( (LONG*)&m_nPriority, nPriority );
	}

	void	Start();
	void	Stop();
	void	Serialize(CArchive& ar);
	void	OnHostAcknowledge(DWORD nAddress);
	BOOL	Execute(int nPriorityClass);	// Run search of specified priority class
	BOOL	IsLastSearch(); 				// Check GUID of latest text search on ED2K and DC++ networks
	void	CreateGUID();

	BOOL			m_bAllowG2; 			// Gnutella2 search enabled
	BOOL			m_bAllowG1; 			// Gnutella search enabled
	BOOL			m_bAllowED2K;			// eDonkey search enabled
	BOOL			m_bAllowDC;				// DC++ search enabled
	BOOL			m_bReceive;
//	DWORD			m_nG2Hits;				// G2 hits if needed
//	DWORD			m_nG1Hits;				// G1 hits if needed
//	DWORD			m_nEDHits;				// ED2k hits if needed
//	DWORD			m_nDCHits;				// DC hits if needed
	DWORD			m_nHits;				// Total hits
	DWORD			m_nHubs;				// Number of G2 hubs searched
	DWORD			m_nLeaves;				// Number of G2 leaves searched
	DWORD			m_nQueryCount;			// Total Gnutella2 queries sent
	DWORD			m_tLastED2K;			// Time an ed2k server was last searched
	DWORD			m_tMoreResults;			// Time more results were requested from an ed2k server

protected:
	int				m_nPriority;
	BOOL			m_bActive;
	DWORD			m_tLastG1;				// Time a G1 multicast search was sent
	DWORD			m_tLastG2;				// Time a G2 hub was last searched
	DWORD			m_tExecute;				// Search execute time (ticks)
	CQuerySearchPtr m_pSearch;				// Search handler
	CDwordDwordMap	m_pNodes;				// Pair of IP and query time (s)
	CDwordDwordMap	m_pG1Nodes;				// Pair of IP and last sent packet TTL

	BOOL	ExecuteNeighbours(const DWORD tTicks, const DWORD tSecs);
	BOOL	ExecuteG1Mesh(const DWORD tTicks = 0, const DWORD tSecs = 0);
	BOOL	ExecuteG2Mesh(const DWORD tTicks, const DWORD tSecs);
	BOOL	ExecuteDonkeyMesh(const DWORD tTicks, const DWORD tSecs);

private:
	CManagedSearch(const CManagedSearch&);
	CManagedSearch& operator=(const CManagedSearch&);
};
