//
// PageProfileFiles.cpp
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
#include "Settings.h"
#include "Envy.h"
#include "PageProfileFiles.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "AlbumFolder.h"
#include "SharedFile.h"
#include "Schema.h"
#include "ShellIcons.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CFilesProfilePage, CSettingsPage)

//BEGIN_MESSAGE_MAP(CFilesProfilePage, CSettingsPage)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilesProfilePage property page

CFilesProfilePage::CFilesProfilePage() : CSettingsPage( CFilesProfilePage::IDD )
{
}

CFilesProfilePage::~CFilesProfilePage()
{
}

void CFilesProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_FILE_LIST, m_wndList );
}

/////////////////////////////////////////////////////////////////////////////
// CFilesProfilePage message handlers

BOOL CFilesProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;
	m_wndList.InsertColumn( 0, L"File", LVCFMT_LEFT, rc.right, -1 );
	ShellIcons.AttachTo( &m_wndList, 16 );	// m_wndList.SetImageList()

	{
		CQuickLock oLock( Library.m_pSection );

		CAlbumFolder* pFolder = LibraryFolders.GetAlbumTarget( CSchema::uriFavoritesFolder, L"Title", NULL );
		if ( pFolder != NULL )
		{
			for ( POSITION pos = pFolder->GetFileIterator() ; pos ; )
			{
				CLibraryFile* pFile = pFolder->GetNextFile( pos );

				if ( pFile->IsShared() )
					m_wndList.InsertItem( LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM, m_wndList.GetItemCount(),
						pFile->m_sName, 0, 0, ShellIcons.Get( pFile->GetPath(), 16 ), pFile->m_nIndex );
			}
		}
	}

	UpdateData( FALSE );

	return TRUE;
}

void CFilesProfilePage::OnOK()
{
	CSettingsPage::OnOK();
}
