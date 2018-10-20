//
// UploadFile.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2010
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
#include "UploadFile.h"
#include "UploadTransfer.h"
#include "FragmentedFile.h"
#include "Statistics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CUploadFile construction

CUploadFile::CUploadFile(CUploadTransfer* pUpload, const Hashes::Sha1Hash& oSHA1, LPCTSTR pszName, LPCTSTR pszPath, QWORD nSize)
	: m_pAddress	( pUpload->m_pHost.sin_addr )
	, m_oFragments	( nSize )
	, m_nRequests	( 0 )
	, m_bSelected	( FALSE )
{
	m_sName = pszName;
	m_sPath = pszPath;
	m_nSize = nSize;
	m_oSHA1 = oSHA1;
	m_pTransfers.AddTail( pUpload );
}

CUploadFile::~CUploadFile()
{
}

//////////////////////////////////////////////////////////////////////
// CUploadFile transfer operations

void CUploadFile::Add(CUploadTransfer* pUpload)
{
	if ( m_pTransfers.Find( pUpload ) == NULL )
		m_pTransfers.AddTail( pUpload );
}

BOOL CUploadFile::Remove(CUploadTransfer* pUpload)
{
	POSITION pos = m_pTransfers.Find( pUpload );
	if ( pos == NULL ) return FALSE;

	m_pTransfers.RemoveAt( pos );

	return IsEmpty();
}

CUploadTransfer* CUploadFile::GetActive() const
{
	if ( IsEmpty() ) return NULL;

	for ( POSITION pos = m_pTransfers.GetHeadPosition(); pos; )
	{
		CUploadTransfer* pUpload = m_pTransfers.GetNext( pos );
		if ( pUpload->m_nState != upsNull )
			return pUpload;
	}

	return m_pTransfers.GetTail();
}

void CUploadFile::Remove()
{
	for ( POSITION pos = m_pTransfers.GetHeadPosition(); pos; )
	{
		CUploadTransfer* pUpload = m_pTransfers.GetNext( pos );
		pUpload->Remove();
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadFile fragments

void CUploadFile::AddFragment(QWORD nOffset, QWORD nLength)
{
	if ( m_oFragments.empty() )
		Statistics.Current.Uploads.Files++;

	m_oFragments.insert( Fragments::Fragment( nOffset, nOffset + nLength ) );
}
