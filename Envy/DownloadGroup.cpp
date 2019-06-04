//
// DownloadGroup.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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
#include "DownloadGroup.h"

#include "DownloadGroups.h"
#include "Downloads.h"
#include "Download.h"
#include "SchemaCache.h"
#include "ShellIcons.h"
#include "QuerySearch.h"
#include "Transfers.h"	// Locks

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CDownloadGroup construction

CDownloadGroup::CDownloadGroup(const LPCTSTR szName, const BOOL bTemporary)
	: m_sName			( szName ? szName : L"" )
	, m_nImage			( SHI_FOLDER_OPEN )
	, m_bTemporary		( bTemporary ? TRI_FALSE : TRI_UNKNOWN )
	, m_bRemoteSelected	( TRUE )
//	, m_bTorrent		( FALSE )	// Obsolete, use detected Schema
{
}

CDownloadGroup::~CDownloadGroup()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup add and remove

void CDownloadGroup::Add(CDownload* pDownload)
{
	if ( m_pDownloads.Find( pDownload ) == NULL )
	{
		m_pDownloads.AddTail( pDownload );
		DownloadGroups.IncBaseCookie();
	}
}

void CDownloadGroup::Remove(CDownload* pDownload)
{
	if ( POSITION pos = m_pDownloads.Find( pDownload ) )
	{
		m_pDownloads.RemoveAt( pos );
		DownloadGroups.IncBaseCookie();
	}
}

void CDownloadGroup::Clear()
{
	m_pDownloads.RemoveAll();
	DownloadGroups.IncBaseCookie();
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup set selection cookie

void CDownloadGroup::SetCookie(int nCookie)
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		GetNext( pos )->m_nGroupCookie = nCookie;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup list copy

void CDownloadGroup::CopyList(CList< CDownload* >& pList)
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		pList.AddTail( GetNext( pos ) );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup conditional add

BOOL CDownloadGroup::Link(CDownload* pDownload)
{
	ASSUME_LOCK( Transfers.m_pSection );

	// Possible re-link clearing
	if ( DownloadGroups.GetSuperGroup() != this )
	{
		if ( POSITION pos = m_pDownloads.Find( pDownload ) )
		{
			m_pDownloads.RemoveAt( pos );
			DownloadGroups.IncBaseCookie();
		}
	}

	if ( pDownload->IsMultiFileTorrent() && CheckURI( m_sSchemaURI, CSchema::uriBitTorrent ) )
	{
		// Filter by BitTorrent flag  (Multifile Torrent Package addition to Schema, was m_bTorrent)
		Add( pDownload );
		return TRUE;
	}

	if ( m_pFilters.IsEmpty() )
		return FALSE;

	CString strTrackers;
	if ( pDownload->IsTorrent() )
	{
		for ( int i = 0; i < pDownload->m_pTorrent.GetTrackerCount(); i++ )
		{
			strTrackers += pDownload->m_pTorrent.GetTrackerAddress( i ).MakeLower() + L" ";
		}
	}

	for ( POSITION pos = m_pFilters.GetHeadPosition(); pos; )
	{
		CString strFilter = m_pFilters.GetNext( pos );

		if ( strFilter.GetAt( 0 ) == L'.' )
		{
			if ( EndsWith( pDownload->m_sName, (LPCTSTR)strFilter, strFilter.GetLength() ) )
			{
				// Filter by extension
				Add( pDownload );
				return TRUE;
			}
		}
		else if ( CQuerySearch::WordMatch( pDownload->m_sName, strFilter ) )
		{
			// Filter by keywords
			Add( pDownload );
			return TRUE;
		}
		else if ( ! strTrackers.IsEmpty() && strFilter.Find( L'.' ) > 2 && strTrackers.Find( strFilter.MakeLower() ) >= 0 )
		{
			// Filter by tracker
			Add( pDownload );
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup conditional add all downloads

int CDownloadGroup::LinkAll()
{
	int nCount = 0;

	ASSUME_LOCK( Transfers.m_pSection );

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		nCount += Link( Downloads.GetNext( pos ) );
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup add/remove a filter

void CDownloadGroup::AddFilter(const CString& strFilter)
{
	if ( ! strFilter.IsEmpty () )
	{
		if ( m_pFilters.Find( strFilter ) == NULL )
			m_pFilters.AddTail( strFilter );
	}
}

void CDownloadGroup::RemoveFilter(const CString& strFilter)
{
	if ( ! strFilter.IsEmpty () )
	{
		while ( POSITION pos = m_pFilters.Find( strFilter ) )
			m_pFilters.RemoveAt( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup schema

void CDownloadGroup::SetSchema(LPCTSTR pszURI, BOOL bRemoveOldFilters)
{
	if ( m_sSchemaURI != pszURI )
	{
		// Remove auto filters only
		if ( bRemoveOldFilters && ! m_pFilters.IsEmpty() )
		{
			if ( CSchemaPtr pOldSchema = SchemaCache.Get( m_sSchemaURI ) )
			{
				for ( POSITION pos = pOldSchema->GetFilterIterator(); pos; )
				{
					CString strFilter;
					BOOL bResult;
					pOldSchema->GetNextFilter( pos, strFilter, bResult );
					if ( bResult )
					{
						strFilter.Insert( 0, L'.' );
						RemoveFilter( strFilter );
					}
				}
			}
		}

		m_sSchemaURI = pszURI;
	}

	if ( CSchemaPtr pSchema = SchemaCache.Get( m_sSchemaURI ) )
	{
		m_nImage = pSchema->m_nIcon16;

		if ( ! m_sName.GetLength() || m_sName == L"DEFAULT" )	// Initial translation, avoid forced rename later
		{
			if ( ! pSchema->m_sHeaderTitle.IsEmpty() )
				m_sName = pSchema->m_sHeaderTitle;
			else
				m_sName = pSchema->m_sTitle;
		}
	}
	else // Non-schema group
	{
		m_nImage = SHI_FOLDER_OPEN;
	}
}

void CDownloadGroup::SetFolder(LPCTSTR pszFolder)
{
	m_sFolder = pszFolder;
}

void CDownloadGroup::SetDefaultFilters()
{
	if ( CSchemaPtr pSchema = SchemaCache.Get( m_sSchemaURI ) )
	{
		for ( POSITION pos = pSchema->GetFilterIterator(); pos; )
		{
			CString strFilter;
			BOOL bResult;
			pSchema->GetNextFilter( pos, strFilter, bResult );
			if ( bResult )
			{
				strFilter.Insert( 0, L'.' );
				AddFilter( strFilter );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup serialize

void CDownloadGroup::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_sSchemaURI;
		ar << m_sFolder;

		ar.WriteCount( m_pFilters.GetCount() );

		for ( POSITION pos = m_pFilters.GetHeadPosition(); pos; )
		{
			ar << m_pFilters.GetNext( pos );
		}

		ar.WriteCount( GetCount() );

		for ( POSITION pos = GetIterator(); pos; )
		{
			DWORD nDownload = GetNext( pos )->m_nSerID;
			ar << nDownload;
		}

		ASSERT( m_bTemporary == TRI_UNKNOWN || m_bTemporary == TRI_FALSE );
		ar << m_bTemporary;

		//ar << m_bTorrent;	// Obsolete: Detect Schema instead
	}
	else	// Loading
	{
		ar >> m_sName;
		ar >> m_sSchemaURI;
		ar >> m_sFolder;

		if ( nVersion > 0 )
		{
			for ( DWORD_PTR nCount = ar.ReadCount(); nCount > 0; nCount-- )
			{
				CString strFilter;
				ar >> strFilter;
				AddFilter( strFilter );
			}
		}
		else	// ToDo: Is this ever needed?
		{
			CString strFilters;
			ar >> strFilters;

			for ( strFilters += L'|'; strFilters.GetLength(); )
			{
				CString strFilter = strFilters.SpanExcluding( L" |" );
				strFilters = strFilters.Mid( strFilter.GetLength() + 1 );
				strFilter.Trim();
				if ( ! strFilter.IsEmpty() )
					AddFilter( strFilter );
			}
		}

		for ( DWORD_PTR nCount = ar.ReadCount(); nCount > 0; nCount-- )
		{
			DWORD nDownload;
			ar >> nDownload;
			if ( CDownload* pDownload = Downloads.FindBySID( nDownload ) )
				Add( pDownload );
		}

		//if ( nVersion >= 4 )
		//{
			ar >> m_bTemporary;
			ASSERT( m_bTemporary == TRI_UNKNOWN || m_bTemporary == TRI_FALSE );
		//}

		//if ( nVersion >= 7 )
			//ar >> m_bTorrent;	// Obsolete: Detect Schema instead

		// Fix collection schema (nVersion < 7)
		//if ( CheckURI( m_sSchemaURI, CSchema::uriCollectionsFolder ) )
		//{
		//	m_sSchemaURI = CSchema::uriCollection;
		//	SetDefaultFilters();
		//}

		// Restore default folder for Collections
		if ( CheckURI( m_sSchemaURI, CSchema::uriCollection ) )
		{
			if ( m_sFolder.IsEmpty() || ! PathIsDirectory( m_sFolder ) )
				m_sFolder = Settings.Downloads.CollectionPath;
		}

		// Restore default folder for Torrents
		if ( CheckURI( m_sSchemaURI, CSchema::uriBitTorrent ) )
		{
			if ( m_sFolder.IsEmpty() || ! PathIsDirectory( m_sFolder ) )
				m_sFolder = Settings.Downloads.TorrentPath;
		}

		SetSchema( m_sSchemaURI );
	}
}

BOOL CDownloadGroup::IsTemporary()
{
	if ( m_bTemporary == TRI_FALSE )
	{
		BOOL bAllCompleted = TRUE;
		for ( POSITION pos = GetIterator(); bAllCompleted && pos; )
		{
			CDownload* pDownload = GetNext( pos );
			if ( Downloads.Check( pDownload ) && ! pDownload->IsCompleted() )
				bAllCompleted = FALSE;
		}
		if ( bAllCompleted )
			m_bTemporary = TRI_TRUE;
	}
	return ( m_bTemporary == TRI_TRUE );
}
