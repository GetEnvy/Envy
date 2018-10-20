//
// PageFileSources.cpp
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
#include "PageFileSources.h"
#include "SharedFile.h"
#include "Library.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CFileSourcesPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileSourcesPage, CFilePropertiesPage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_SOURCES, OnItemChangedFileSources)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_SOURCES, OnDblClk)
	ON_BN_CLICKED(IDC_SOURCE_REMOVE, OnSourceRemove)
	ON_BN_CLICKED(IDC_SOURCE_NEW, OnSourceNew)
	ON_EN_CHANGE(IDC_FILE_SOURCE, OnChangeFileSource)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileSourcesPage property page

CFileSourcesPage::CFileSourcesPage()
	: CFilePropertiesPage(CFileSourcesPage::IDD)
{
}

CFileSourcesPage::~CFileSourcesPage()
{
}

void CFileSourcesPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOURCE_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_SOURCE_NEW, m_wndNew);
	DDX_Control(pDX, IDC_FILE_SOURCES, m_wndList);
	DDX_Text(pDX, IDC_FILE_SOURCE, m_sSource);
}

/////////////////////////////////////////////////////////////////////////////
// CFileSourcesPage message handlers

BOOL CFileSourcesPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );

	m_wndList.InsertColumn( 0, L"URL", LVCFMT_LEFT, rc.right - 128, -1 );
	m_wndList.InsertColumn( 1, L"Expires", LVCFMT_RIGHT, 128, 0 );

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 1, 1 );
	AddIcon( IDI_WEB_URL, m_gdiImageList );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );

	{
		CQuickLock oLock( Library.m_pSection );
		CLibraryFile* pFile = GetFile();
		if ( pFile == NULL ) return TRUE;

		for ( POSITION pos = pFile->m_pSources.GetHeadPosition(); pos; )
		{
			AddSource( (CSharedSource*)pFile->m_pSources.GetNext( pos ) );
		}
	}

	Skin.Translate( L"CFileSourcesPageList", m_wndList.GetHeaderCtrl() );
	m_wndNew.EnableWindow( FALSE );
	m_wndRemove.EnableWindow( FALSE );

	return TRUE;
}

void CFileSourcesPage::AddSource(CSharedSource* pSource)
{
	CString strURL = pSource->m_sURL;
	if ( Settings.General.LanguageRTL ) strURL = L"\x202A" + strURL;

	LV_ITEM pItem = {};
	pItem.mask		= LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
	pItem.pszText	= (LPTSTR)(LPCTSTR)strURL;
	pItem.iImage	= 0;
	pItem.lParam	= (LPARAM)pSource;
	pItem.iItem		= m_wndList.GetItemCount();
	pItem.iItem		= m_wndList.InsertItem( &pItem );

	FILETIME pFileTime = pSource->m_pTime;
	CString strDate, strTime;
	SYSTEMTIME pSystemTime;

	(LONGLONG&)pFileTime += (LONGLONG)Settings.Library.SourceExpire * 10000000;
	FileTimeToSystemTime( &pFileTime, &pSystemTime );
	SystemTimeToTzSpecificLocalTime( NULL, &pSystemTime, &pSystemTime );

	GetDateFormat( LOCALE_USER_DEFAULT, 0, &pSystemTime, L"yyyy-MM-dd", strDate.GetBuffer( 64 ), 64 );
	GetTimeFormat( LOCALE_USER_DEFAULT, 0, &pSystemTime, L"HH:mm", strTime.GetBuffer( 64 ), 64 );
	strDate.ReleaseBuffer();
	strTime.ReleaseBuffer();

	strDate += L' ';
	strDate += strTime;

	m_wndList.SetItemText( pItem.iItem, 1, strDate );
}

void CFileSourcesPage::OnItemChangedFileSources(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_wndRemove.EnableWindow( m_wndList.GetSelectedCount() > 0 );
	*pResult = 0;
}

void CFileSourcesPage::OnChangeFileSource()
{
	UpdateData();
	m_wndNew.EnableWindow( StartsWith( m_sSource, _P( L"http://" ) ) );
}

void CFileSourcesPage::OnSourceRemove()
{
	{
		CQuickLock oLock( Library.m_pSection );
		CLibraryFile* pFile = GetFile();
		if ( pFile == NULL ) return;

		for ( int nItem; ( nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) >= 0; )
		{
			CSharedSource* pSource = (CSharedSource*)m_wndList.GetItemData( nItem );

			if ( POSITION pos = pFile->m_pSources.Find( pSource ) )
			{
				delete pSource;
				pFile->m_pSources.RemoveAt( pos );
			}

			m_wndList.DeleteItem( nItem );
		}
		Library.Update();
	}

	m_wndRemove.EnableWindow( FALSE );
}

void CFileSourcesPage::OnSourceNew()
{
	UpdateData();

	if ( StartsWith( m_sSource, _P( L"http://" ) ) )
	{
		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = GetFile() )
		{
			CSharedSource* pSource = pFile->AddAlternateSources( m_sSource );
			m_sSource.Empty();
			Library.Update();
			oLock.Unlock();
			UpdateData( FALSE );
			if ( pSource ) AddSource( pSource );
		}
	}
}

void CFileSourcesPage::OnDblClk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		CSharedSource* pSource = (CSharedSource*)m_wndList.GetItemData( nItem );
		m_sSource = pSource->m_sURL;
	}

	UpdateData( FALSE );

	*pResult = 0;
}
