//
// PageFileSharing.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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
#include "Envy.h"
#include "PageFileSharing.h"

#include "Library.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "UploadQueue.h"
#include "UploadQueues.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CFileSharingPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileSharingPage, CFilePropertiesPage)
	ON_BN_CLICKED(IDC_SHARE_OVERRIDE_0, OnShareOverride0)
	ON_BN_CLICKED(IDC_SHARE_OVERRIDE_1, OnShareOverride1)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileSharingPage property page

CFileSharingPage::CFileSharingPage()
	: CFilePropertiesPage(CFileSharingPage::IDD)
	, m_bOverride	( -1 )
	, m_bShare		( FALSE )
	, m_sTags		()
{
}

CFileSharingPage::~CFileSharingPage()
{
}

void CFileSharingPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_FILE_NETWORKS, m_wndNetworks);
	DDX_Control(pDX, IDC_FILE_TAGS, m_wndTags);
	DDX_Control(pDX, IDC_FILE_SHARE, m_wndShare);
	DDX_Radio(pDX, IDC_SHARE_OVERRIDE_0, m_bOverride);
	DDX_Check(pDX, IDC_FILE_SHARE, m_bShare);
	DDX_CBString(pDX, IDC_FILE_TAGS, m_sTags);
}

/////////////////////////////////////////////////////////////////////////////
// CFileSharingPage message handlers

BOOL CFileSharingPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	m_wndTags.AddString( L"" );

	{
		CQuickLock oLock( UploadQueues.m_pSection );

		CList< CString > pAdded;

		for ( POSITION pos = UploadQueues.GetIterator() ; pos ; )
		{
			CUploadQueue* pQueue = UploadQueues.GetNext( pos );

			if ( ! pQueue->m_sShareTag.IsEmpty() )
			{
				if ( pAdded.Find( pQueue->m_sShareTag ) == NULL )
				{
					pAdded.AddTail( pQueue->m_sShareTag );
					m_wndTags.AddString( pQueue->m_sShareTag );
				}
			}
		}

		if ( pAdded.IsEmpty() )
		{
			m_wndTags.AddString( L"Release" );
			m_wndTags.AddString( L"Popular" );
		}
	}

	{
		CQuickLock oLock( Library.m_pSection );

		if ( CLibraryFile* pSingleFile = GetFile() )
		{
			m_bOverride	= pSingleFile->IsSharedOverride();
			m_bShare	= pSingleFile->IsShared();
			m_sTags		= pSingleFile->m_sShareTags;
		}
		else
		{
			CLibraryListPtr pList( GetList() );
			if ( pList )
			{
				for ( POSITION pos = pList->GetHeadPosition() ; pos ; )
				{
					if ( CLibraryFile* pFile = pList->GetNextFile( pos ) )
					{
						m_bOverride	= pFile->IsSharedOverride();
						m_bShare	= pFile->IsShared();
						m_sTags		= pFile->m_sShareTags;
					}
				}
			}
		}
	}

	UpdateData( FALSE );
	m_wndShare.EnableWindow( m_bOverride );

	return TRUE;
}

void CFileSharingPage::OnShareOverride0()
{
	UpdateData();

	m_wndShare.EnableWindow( m_bOverride );

	if ( ! m_bOverride )
	{
		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = GetFile() )
		{
			m_bShare = pFile->IsShared( true );

			oLock.Unlock();
			UpdateData( FALSE );
		}
	}
}

void CFileSharingPage::OnShareOverride1()
{
	OnShareOverride0();
}

void CFileSharingPage::OnOK()
{
	UpdateData();

	CLibraryListPtr pList( GetList() );
	if ( pList )
	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = pList->GetHeadPosition() ; pos ; )
		{
			if ( CLibraryFile* pFile = pList->GetNextFile( pos ) )
			{
				pFile->SetShared( ( m_bShare != FALSE ), ( m_bOverride != FALSE ) );
				pFile->m_sShareTags = m_sTags;
			}
		}
	}

	CFilePropertiesPage::OnOK();
}
