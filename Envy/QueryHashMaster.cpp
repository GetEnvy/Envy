//
// QueryHashMaster.cpp
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

#include "StdAfx.h"
#include "Envy.h"
#include "QueryHashMaster.h"
#include "QueryHashGroup.h"
#include "Neighbour.h"
#include "Library.h"
#include "LibraryDictionary.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CQueryHashMaster QueryHashMaster;


//////////////////////////////////////////////////////////////////////
// CQueryHashMaster construction

CQueryHashMaster::CQueryHashMaster()
	: m_nPerGroup	( 0 )
	, m_bValid		( FALSE )
{
}

CQueryHashMaster::~CQueryHashMaster()
{
	ASSERT( GetCount() == 0 );
}

//////////////////////////////////////////////////////////////////////
// CQueryHashMaster create

void CQueryHashMaster::Create()
{
	CQueryHashTable::Create();

	m_nPerGroup	= 250;
	m_bValid	= FALSE;
	m_bLive		= false;
	m_nCookie	= 0;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashMaster add neighbour

void CQueryHashMaster::Add(CQueryHashTable* pTable)
{
	ASSERT( m_nPerGroup > 0 );
	ASSERT( pTable->m_nHash > 0 );
	ASSERT( pTable->m_pGroup == NULL );

	for ( POSITION pos = GetIterator(); pos; )
	{
		CQueryHashGroup* pGroup = GetNext( pos );

		if ( pGroup->m_nHash == pTable->m_nHash &&
			 pGroup->GetCount() < m_nPerGroup )
		{
			pGroup->Add( pTable );
			m_bValid = FALSE;
			return;
		}
	}

	CQueryHashGroup* pGroup = new CQueryHashGroup( pTable->m_nHash );
	m_pGroups.AddTail( pGroup );
	pGroup->Add( pTable );
	m_bValid = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashMaster remove neighbour

void CQueryHashMaster::Remove(CQueryHashTable* pTable)
{
	if ( pTable->m_pGroup == NULL ) return;

	CQueryHashGroup* pGroup = pTable->m_pGroup;
	pGroup->Remove( pTable );

	if ( pGroup->GetCount() == 0 )
	{
		POSITION pos = m_pGroups.Find( pGroup );
		ASSERT( pos != NULL );
		m_pGroups.RemoveAt( pos );
		delete pGroup;
	}

	m_bValid = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashMaster build

void CQueryHashMaster::Build()
{
	const DWORD tNow = GetTickCount();

	if ( tNow < m_nCookie + m_bValid ? 600000 : 20000 )
		return;

	CSingleLock oLibraryLock( &Library.m_pSection );
	if ( ! oLibraryLock.Lock( 500 ) )
		return;

	const CQueryHashTable* pLocalTable = LibraryDictionary.GetHashTable();
	if ( ! pLocalTable )
		return;

	Clear();
	Merge( pLocalTable );

	oLibraryLock.Unlock();

	for ( POSITION pos = GetIterator(); pos; )
	{
		CQueryHashGroup* pGroup = GetNext( pos );
		Merge( pGroup );
	}

	CSingleLock oTransfersLock( &Transfers.m_pSection );
	if ( oTransfersLock.Lock( 100 ) )
	{
		for ( POSITION pos = Downloads.GetIterator(); pos; )
		{
			const CDownload& oDownload = *Downloads.GetNext( pos );
			if ( oDownload.IsShared() )
				AddFile( oDownload );
		}
	}

	m_bValid	= TRUE;
	m_bLive		= true;
	m_nCookie	= tNow;
}
