//
// DlgFilePropertiesPage.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2007
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
#include "DlgFilePropertiesPage.h"
#include "DlgFilePropertiesSheet.h"
#include "Library.h"
#include "SharedFile.h"
#include "ShellIcons.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CFilePropertiesPage, CPropertyPageAdv)

//BEGIN_MESSAGE_MAP(CFilePropertiesPage, CPropertyPageAdv)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage property page

CFilePropertiesPage::CFilePropertiesPage(UINT nIDD) : CPropertyPageAdv( nIDD )
{
}

CFilePropertiesPage::~CFilePropertiesPage()
{
}

void CFilePropertiesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage helper functions

CLibraryFile* CFilePropertiesPage::GetFile()
{
	CLibraryListPtr pList( GetList() );
	if ( pList->GetCount() != 1 ) return NULL;

	CQuickLock oLock( Library.m_pSection );
	CLibraryFile* pFile = Library.LookupFile( pList->GetHead() );
	if ( pFile != NULL ) return pFile;
	PostMessage( WM_CLOSE );
	return NULL;
}

CLibraryList* CFilePropertiesPage::GetList() const
{
	CFilePropertiesSheet* pSheet = (CFilePropertiesSheet*)GetParent();
	return pSheet->m_pList;
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage message handlers

BOOL CFilePropertiesPage::OnInitDialog()
{
	CPropertyPageAdv::OnInitDialog();

	CSingleLock oLock( &Library.m_pSection, TRUE );
	if ( CLibraryFile* pFile = GetFile() )
	{
		if ( CWnd* pNameWnd = GetDlgItem( IDC_FILE_NAME ) )
		{
			if ( Settings.General.LanguageRTL )
			{
				CRect rc, rcPage;
				pNameWnd->GetWindowRect( &rc );
				GetWindowRect( &rcPage );
				pNameWnd->MoveWindow( rcPage.right - rc.right,
					rc.top - rcPage.top, rc.Width(), rc.Height(), FALSE );
				pNameWnd->ModifyStyleEx( WS_EX_RTLREADING, WS_EX_LTRREADING, 0 );
			}
			pNameWnd->SetWindowText( pFile->m_sName );
		}
		m_nIcon = ShellIcons.Get( pFile->GetPath(), 48 );

		oLock.Unlock();
	}
	else
	{
		oLock.Unlock();
		CLibraryListPtr pList( GetList() );
		if ( pList )
		{
			if ( CWnd* pNameWnd = GetDlgItem( IDC_FILE_NAME ) )
			{
				if ( Settings.General.LanguageRTL )
				{
					CRect rc, rcPage;
					pNameWnd->GetWindowRect( &rc );
					GetWindowRect( &rcPage );
					pNameWnd->MoveWindow( rcPage.right - rc.right,
						rc.top - rcPage.top, rc.Width(), rc.Height(), FALSE );
					pNameWnd->ModifyStyleEx( 0, WS_EX_RTLREADING, 0 );
				}
				CString strMessage;
				strMessage.Format( LoadString( IDS_LIBRARY_METADATA_EDIT ), pList->GetCount() );
				pNameWnd->SetWindowText( strMessage );
			}
			//m_nIcon = SHI_EXECUTABLE;
		}
	}

	return TRUE;
}
