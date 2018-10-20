//
// DownloadBase.cpp
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
//#include "Settings.h"
#include "Envy.h"
#include "DownloadBase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CDownloadBase, CEnvyFile)

//////////////////////////////////////////////////////////////////////
// CDownloadBase construction

CDownloadBase::CDownloadBase()
	: m_bSHA1Trusted	( false )
	, m_bTigerTrusted	( false )
	, m_bED2KTrusted	( false )
	, m_bBTHTrusted		( false )
	, m_bMD5Trusted		( false )
	, m_nCookie			( 1 )
	, m_nSaveCookie		( 0 )
{
}

CDownloadBase::~CDownloadBase()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase modified

bool CDownloadBase::IsModified() const
{
	return ( m_nCookie != m_nSaveCookie );
}

void CDownloadBase::SetModified()
{
	++m_nCookie;
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase serialize

void CDownloadBase::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_nSize;
		ar << m_tDate;
	//	CString strSearchKeyword;
	//	ar << strSearchKeyword;	// ToDo: ShareazaPlus feature?
		SerializeOut( ar, m_oSHA1 );
		ar << (uint32)m_bSHA1Trusted;
		SerializeOut( ar, m_oTiger );
		ar << (uint32)m_bTigerTrusted;
		SerializeOut( ar, m_oMD5 );
		ar << (uint32)m_bMD5Trusted;
		SerializeOut( ar, m_oED2K );
		ar << (uint32)m_bED2KTrusted;
		SerializeOut( ar, m_oBTH );
		ar << (uint32)m_bBTHTrusted;
	}
	else // Loading
	{
		ar >> m_sName;

		//if ( nVersion > 32 && nVersion < 1000 )
		//{
		//	CString strSearchKeyword;
		//	ar >> strSearchKeyword;		// Shareaza compatibility for ShareazaPlus
		//}

		ar >> m_nSize;

		if ( nVersion < 30 || nVersion == 1000 )	// ToDo: Update this before version collision
			ar >> m_tDate;
		else	// Shareaza import
			m_tDate = CTime::GetCurrentTime();

	//	CString strSearchKeyword;
	//	ar >> strSearchKeyword;			// ToDo: ShareazaPlus feature?

		uint32 b;
		SerializeIn( ar, m_oSHA1, nVersion );
		ar >> b;
		m_bSHA1Trusted = b != 0;

		SerializeIn( ar, m_oTiger, nVersion );
		ar >> b;
		m_bTigerTrusted = b != 0;

		SerializeIn( ar, m_oMD5, nVersion );
		ar >> b;
		m_bMD5Trusted = b != 0;

		SerializeIn( ar, m_oED2K, nVersion );
		ar >> b;
		m_bED2KTrusted = b != 0;

		SerializeIn( ar, m_oBTH, nVersion );
		ar >> b;
		m_bBTHTrusted = b != 0;
	}
}
