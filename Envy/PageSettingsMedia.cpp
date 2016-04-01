//
// PageSettingsMedia.cpp
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
#include "WndSettingsPage.h"
#include "WndSettingsSheet.h"
#include "PageSettingsMedia.h"
#include "PageSettingsPlugins.h"
#include "DlgMediaVis.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define INTERNAL_INDEX	1
#define CUSTOM_INDEX	0


IMPLEMENT_DYNCREATE(CMediaSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CMediaSettingsPage, CSettingsPage)
	ON_BN_CLICKED(IDC_MEDIA_PLAY, OnMediaPlay)
	ON_BN_CLICKED(IDC_MEDIA_ENQUEUE, OnMediaEnqueue)
	ON_BN_CLICKED(IDC_MEDIA_ADD, OnMediaAdd)
	ON_BN_CLICKED(IDC_MEDIA_REMOVE, OnMediaRemove)
	ON_BN_CLICKED(IDC_MEDIA_VIS, OnMediaVis)
	ON_CBN_EDITCHANGE(IDC_MEDIA_TYPES, OnEditChangeMediaTypes)
	ON_CBN_SELCHANGE(IDC_MEDIA_TYPES, OnSelChangeMediaTypes)
	ON_CBN_SELCHANGE(IDC_MEDIA_SERVICE, OnSelChangeMediaService)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMediaSettingsPage property page

CMediaSettingsPage::CMediaSettingsPage()
	: CSettingsPage(CMediaSettingsPage::IDD)
	, m_bEnablePlay 	( FALSE )
	, m_bEnableEnqueue	( FALSE )
{
}

CMediaSettingsPage::~CMediaSettingsPage()
{
}

void CMediaSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEDIA_SERVICE, m_wndServices);
	DDX_Control(pDX, IDC_MEDIA_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_MEDIA_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_MEDIA_TYPES, m_wndList);
	DDX_CBString(pDX, IDC_MEDIA_TYPES, m_sType);
	DDX_Check(pDX, IDC_MEDIA_PLAY, m_bEnablePlay);
	DDX_Check(pDX, IDC_MEDIA_ENQUEUE, m_bEnableEnqueue);
}

void CMediaSettingsPage::Update()
{
	UpdateData();

	const BOOL bInternal = ( m_wndServices.GetCurSel() == INTERNAL_INDEX );
	GetDlgItem( IDC_MEDIA_PLAY )->EnableWindow( bInternal );
	GetDlgItem( IDC_MEDIA_ENQUEUE )->EnableWindow( bInternal );
	GetDlgItem( IDC_MEDIA_VIS )->EnableWindow( bInternal );

	m_wndList.EnableWindow( m_bEnablePlay || m_bEnableEnqueue );
	m_wndAdd.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetWindowTextLength() > 0 );
	m_wndRemove.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetCurSel() >= 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaSettingsPage message handlers

BOOL CMediaSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bEnablePlay		= Settings.MediaPlayer.EnablePlay;
	m_bEnableEnqueue	= Settings.MediaPlayer.EnableEnqueue;

	for ( string_set::const_iterator i = Settings.MediaPlayer.FileTypes.begin() ;
		i != Settings.MediaPlayer.FileTypes.end() ; ++i )
	{
		m_wndList.AddString( *i );
	}

	m_wndServices.AddString( L"(" + LoadString( IDS_GENERAL_CUSTOM ) + L"\x2026)" );
	m_wndServices.AddString( LoadString( IDS_MEDIA_PLAYER ) );
	int nSelected = INTERNAL_INDEX;

	for ( string_set::const_iterator i = Settings.MediaPlayer.ServicePath.begin() ;
		i != Settings.MediaPlayer.ServicePath.end() ; ++i )
	{
		CString strPlayer = *i;
		BOOL bSelected = strPlayer.Right( 1 ) == L'*';	// SELECTED_PLAYER_TOKEN
		if ( bSelected ) strPlayer.TrimRight( L'*' );	// SELECTED_PLAYER_TOKEN

		int nIndex = m_wndServices.AddString( PathFindFileName( strPlayer ) );
		if ( bSelected )
			nSelected = nIndex;

		m_wndServices.SetItemDataPtr( nIndex, new CString( strPlayer ) );
	}

	m_wndServices.SetCurSel( nSelected );

	UpdateData( FALSE );

	Update();

	return TRUE;
}

void CMediaSettingsPage::OnMediaPlay()
{
	Update();
}

void CMediaSettingsPage::OnMediaEnqueue()
{
	Update();
}

void CMediaSettingsPage::OnSelChangeMediaTypes()
{
	Update();
}

void CMediaSettingsPage::OnEditChangeMediaTypes()
{
	Update();
}

void CMediaSettingsPage::OnMediaAdd()
{
	UpdateData();

	ToLower( m_sType );
	m_sType.Trim();
	if ( m_sType.IsEmpty() ) return;

	if ( m_wndList.FindStringExact( -1, m_sType ) >= 0 ) return;

	m_wndList.AddString( m_sType );
	m_sType.Empty();
	UpdateData( FALSE );
}

void CMediaSettingsPage::OnMediaRemove()
{
	UpdateData();

	int nItem = m_wndList.GetCurSel();
	if ( nItem >= 0 )
		m_wndList.DeleteString( nItem );

	m_wndRemove.EnableWindow( FALSE );
}

void CMediaSettingsPage::OnMediaVis()
{
	UpdateData();

	CMediaVisDlg dlg( NULL );
	dlg.DoModal();
}

void CMediaSettingsPage::OnDestroy()
{
	const int nCount = m_wndServices.GetCount();
	for ( int i = 0 ; i < nCount ; ++i )
	{
		delete (CString*)m_wndServices.GetItemDataPtr( i );
	}

	CSettingsPage::OnDestroy();
}

void CMediaSettingsPage::OnOK()
{
	UpdateData();

	Settings.MediaPlayer.EnablePlay		= ( m_bEnablePlay != FALSE );
	Settings.MediaPlayer.EnableEnqueue	= ( m_bEnableEnqueue != FALSE );
	Settings.MediaPlayer.ServicePath.clear();

	// Re-add previous external mediaplayers.  ToDo: Some way to clear them?

	int nSelected = m_wndServices.GetCurSel();
	if ( nSelected == CUSTOM_INDEX )
		nSelected = INTERNAL_INDEX;

	const int nCount = m_wndServices.GetCount();
	for ( int i = 0 ; i < nCount ; ++i )
	{
		CString* psPlayer = (CString*)m_wndServices.GetItemDataPtr( i );
		if ( ! psPlayer )
			continue;
		if ( i == nSelected )
			*psPlayer += L'*';	// SELECTED_PLAYER_TOKEN
		Settings.MediaPlayer.ServicePath.insert( *psPlayer );
	}

	Settings.MediaPlayer.ShortPaths = ( nSelected != INTERNAL_INDEX );	// No MAX_LENGTH issues with internal service only?

	CSettingsSheet* pSheet = GetSheet();
	for ( INT_PTR nPage = 0 ; nPage < pSheet->GetPageCount() ; nPage++ )
	{
		if ( CSettingsPage* pPage = pSheet->GetPage( nPage ) )
		{
			CString strClass( pPage->GetRuntimeClass()->m_lpszClassName );
			if ( strClass == L"CPluginsSettingsPage" )
			{
				CPluginsSettingsPage* pPluginPage = static_cast< CPluginsSettingsPage* >( pPage );
				pPluginPage->UpdateList();
				break;
			}
		}
	}

	Settings.MediaPlayer.FileTypes.clear();

	for ( int nItem = 0 ; nItem < m_wndList.GetCount() ; nItem++ )
	{
		CString str;
		m_wndList.GetLBText( nItem, str );
		if ( ! str.IsEmpty() )
			Settings.MediaPlayer.FileTypes.insert( str );
	}

	CSettingsPage::OnOK();
}

void CMediaSettingsPage::OnSelChangeMediaService()
{
	UpdateData();

	int nSelected = m_wndServices.GetCurSel();
	if ( nSelected == CUSTOM_INDEX )		// Custom Media Player selected
	{
		CFileDialog dlg( TRUE, L"exe", L"", OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
			L"Executable Files|*.exe|" + LoadString( IDS_FILES_ALL ) + L"|*.*||", this );
		//	SchemaCache.GetFilter( CSchema::uriApplicationAll ) +
		//	SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		//	L"|", this );

		if ( dlg.DoModal() != IDOK )
		{
			m_wndServices.SetCurSel( INTERNAL_INDEX );
			Update();
			return;
		}

		CString strNewPlayer = dlg.GetPathName();

		const int nCount = m_wndServices.GetCount();
		for ( int i = 0 ; i < nCount ; ++i )
		{
			CString* psPlayer = (CString*)m_wndServices.GetItemDataPtr( i );
			if ( ! psPlayer )
				continue;

			if ( psPlayer->CompareNoCase( strNewPlayer ) == 0 )	// Duplicate
			{
				m_wndServices.SetCurSel( i );
				Update();
				return;
			}
		}

		const int nIndex = m_wndServices.AddString( PathFindFileName( strNewPlayer ) );
		m_wndServices.SetItemDataPtr( nIndex, new CString( strNewPlayer ) );
		m_wndServices.SetCurSel( nIndex );

		m_bEnablePlay = m_bEnableEnqueue = FALSE;
	}
	else if ( nSelected == INTERNAL_INDEX )		// Envy Media Player selected
	{
		m_bEnablePlay = m_bEnableEnqueue = TRUE;
	}
	else	// Previous player, not Envy default or new custom
	{
		m_bEnablePlay = m_bEnableEnqueue = FALSE;
	}

	UpdateData( FALSE );
	Update();

	Invalidate();	// Clear artifacts if skinned
}
