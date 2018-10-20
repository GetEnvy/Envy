//
// DlgFilePropertiesSheet.cpp
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
#include "DlgFilePropertiesSheet.h"

#include "PageFileGeneral.h"
#include "PageFileMetadata.h"
#include "PageFileSources.h"
#include "PageFileComments.h"
#include "PageFileSharing.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CFilePropertiesSheet, CPropertySheetAdv)

//BEGIN_MESSAGE_MAP(CFilePropertiesSheet, CPropertySheetAdv)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesSheet

CFilePropertiesSheet::CFilePropertiesSheet(CLibraryListItem oObject)
	: m_sGeneralTitle	( L"General" )
	, m_sCommentsTitle	( L"Comments" )
	, m_sMetadataTitle	( L"Metadata" )
	, m_sSourcesTitle	( L"Sources" )
	, m_sSharingTitle	( L"Sharing" )
	, m_pList			( new CLibraryList() )
{
	if ( oObject.Type != CLibraryListItem::Empty )
		m_pList->AddTail( oObject );
}


/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesSheet operations

void CFilePropertiesSheet::Add(CLibraryListItem oObject)
{
	m_pList->CheckAndAdd( oObject );
}

void CFilePropertiesSheet::Add(const CLibraryList* pList)
{
	m_pList->Merge( pList );
}

INT_PTR CFilePropertiesSheet::DoModal(int nPage)
{
	CFileGeneralPage	pGeneral;
	CFileCommentsPage	pComments;
	CFileMetadataPage	pMetadata;
	CFileSourcesPage	pSources;
	CFileSharingPage	pSharing;

	switch ( m_pList->GetCount() )
	{
	case 0:
		return IDCANCEL;
	case 1:
		SetTabTitle( &pGeneral, m_sGeneralTitle );
		AddPage( &pGeneral );
		SetTabTitle( &pComments, m_sCommentsTitle );
		AddPage( &pComments );
		SetTabTitle( &pMetadata, m_sMetadataTitle );
		AddPage( &pMetadata );
		SetTabTitle( &pSources, m_sSourcesTitle );
		AddPage( &pSources );
		SetTabTitle( &pSharing, m_sSharingTitle );
		AddPage( &pSharing );
		break;
	default:
		SetTabTitle( &pComments, m_sCommentsTitle );
		AddPage( &pComments );
		SetTabTitle( &pMetadata, m_sMetadataTitle );
		AddPage( &pMetadata );
		SetTabTitle( &pSharing, m_sSharingTitle );
		AddPage( &pSharing );
		if ( nPage == 1 ) nPage = 0;
		else if ( nPage == 2 ) nPage = 1;
		break;
	}

	m_psh.nStartPage = nPage;
	INT_PTR nRes = CPropertySheetAdv::DoModal();

	Settings.Save();

	return nRes;
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesSheet message handlers

BOOL CFilePropertiesSheet::OnInitDialog()
{
	BOOL bResult = CPropertySheetAdv::OnInitDialog();

	SetFont( &theApp.m_gdiFont );
	SetIcon( theApp.LoadIcon( IDI_PROPERTIES ), TRUE );
	SetWindowText( LoadString( IDS_FILE_PROPERTIES ) );

	if ( GetDlgItem( IDOK ) )
	{
		CRect rc;

		GetDlgItem( 0x3021 )->GetWindowRect( &rc );		// Apply Position for OK
		ScreenToClient( &rc );
		GetDlgItem( IDOK )->SetWindowPos( NULL, rc.left + 1, rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );

		GetDlgItem( 0x0009 )->GetWindowRect( &rc );		// Help Position for Cancel
		ScreenToClient( &rc );
		GetDlgItem( IDCANCEL )->SetWindowPos( NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );
	}

	if ( GetDlgItem( 0x3021 ) ) GetDlgItem( 0x3021 )->ShowWindow( SW_HIDE );	// No Apply
	if ( GetDlgItem( 0x0009 ) ) GetDlgItem( 0x0009 )->ShowWindow( SW_HIDE );	// No Help

	return bResult;
}
