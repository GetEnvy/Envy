//
// PageProfileFavorites.cpp
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
#include "PageProfileFavorites.h"
#include "GProfile.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CFavoritesProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CFavoritesProfilePage, CSettingsPage)
	ON_EN_CHANGE(IDC_WEB_NAME, OnChangeWebName)
	ON_EN_CHANGE(IDC_WEB_URL, OnChangeWebUrl)
	ON_BN_CLICKED(IDC_WEB_ADD, OnWebAdd)
	ON_BN_CLICKED(IDC_WEB_REMOVE, OnWebRemove)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_WEB_LIST, OnItemChangedWebList)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFavoritesProfilePage property page

CFavoritesProfilePage::CFavoritesProfilePage() : CSettingsPage( CFavoritesProfilePage::IDD )
//	, m_sTitle	( L"" )
//	, m_sURL	( L"http://" )
{
}

CFavoritesProfilePage::~CFavoritesProfilePage()
{
}

void CFavoritesProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WEB_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_WEB_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_WEB_LIST, m_wndList);
	DDX_Text(pDX, IDC_WEB_URL, m_sURL);
	DDX_Text(pDX, IDC_WEB_NAME, m_sTitle);
}

/////////////////////////////////////////////////////////////////////////////
// CFavoritesProfilePage message handlers

BOOL CFavoritesProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 1, 1 );
	AddIcon( IDI_WEB_URL, m_gdiImageList );

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
	m_wndList.InsertColumn( 0, L"Name", LVCFMT_LEFT, 160, -1 );
	m_wndList.InsertColumn( 1, L"URL", LVCFMT_LEFT, rc.right - 160, 0 );

	m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	if ( CXMLElement* pBookmarks = MyProfile.GetXML( L"bookmarks" ) )
	{
		for ( POSITION pos = pBookmarks->GetElementIterator(); pos; )
		{
			CXMLElement* pBookmark = pBookmarks->GetNextElement( pos );

			if ( pBookmark->IsNamed( L"bookmark" ) )
			{
				CString strTitle	= pBookmark->GetAttributeValue( L"title" );
				CString strURL		= pBookmark->GetAttributeValue( L"url" );
				if ( Settings.General.LanguageRTL ) strURL = L"\x202A" + strURL;

				int nItem = m_wndList.InsertItem( LVIF_TEXT|LVIF_IMAGE,
					m_wndList.GetItemCount(), strTitle, 0, 0, 0, 0 );
				m_wndList.SetItemText( nItem, 1, strURL );
			}
		}
	}

	Skin.Translate( L"CFavoritesProfileList", m_wndList.GetHeaderCtrl() );
	UpdateData( FALSE );
	m_wndAdd.EnableWindow( FALSE );
	m_wndRemove.EnableWindow( FALSE );

	return TRUE;
}

void CFavoritesProfilePage::OnChangeWebName()
{
	UpdateData();
	m_wndAdd.EnableWindow( m_sTitle.GetLength() && m_sURL.Find( L"://" ) > 3 && m_sURL.Find( L"." ) > 8 );
}

void CFavoritesProfilePage::OnChangeWebUrl()
{
	UpdateData();
	m_wndAdd.EnableWindow( m_sTitle.GetLength() && m_sURL.Find( L"://" ) > 3 && m_sURL.Find( L"." ) > 8 );
}

void CFavoritesProfilePage::OnItemChangedWebList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
	m_wndRemove.EnableWindow( m_wndList.GetSelectedCount() > 0 );
}

void CFavoritesProfilePage::OnWebAdd()
{
	UpdateData();

	int nItem = m_wndList.InsertItem( LVIF_TEXT|LVIF_IMAGE, m_wndList.GetItemCount(), m_sTitle, 0, 0, 0, 0 );
	m_wndList.SetItemText( nItem, 1, Settings.General.LanguageRTL ? L"\x202A" + m_sURL : m_sURL );

	m_sTitle.Empty();
	m_sURL = L"http://";

	UpdateData( FALSE );
	m_wndAdd.EnableWindow( FALSE );
}

void CFavoritesProfilePage::OnWebRemove()
{
	for ( int nItem = m_wndList.GetItemCount() - 1; nItem >= 0; nItem-- )
	{
		if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
			m_wndList.DeleteItem( nItem );
	}

	m_wndRemove.EnableWindow( FALSE );
}

void CFavoritesProfilePage::OnOK()
{
	UpdateData();

	if ( CXMLElement* pBookmarks = MyProfile.GetXML( L"bookmarks", TRUE ) )
	{
		pBookmarks->DeleteAllElements();

		for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
		{
			CXMLElement* pBookmark = pBookmarks->AddElement( L"bookmark" );
			pBookmark->AddAttribute( L"title", m_wndList.GetItemText( nItem, 0 ) );
			pBookmark->AddAttribute( L"url", m_wndList.GetItemText( nItem, 1 ) );
		}

		if ( pBookmarks->GetElementCount() == 0 )
			pBookmarks->Delete();
	}

	CSettingsPage::OnOK();
}
