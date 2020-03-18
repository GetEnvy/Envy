//
// WizardSharePage.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2020
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
#include "WizardSharePage.h"

#include "DlgFolderScan.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CWizardSharePage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardSharePage, CWizardPage)
	ON_WM_XBUTTONDOWN()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SHARE_FOLDERS, OnItemChangedShareFolders)
	ON_NOTIFY(NM_DBLCLK, IDC_SHARE_FOLDERS, OnDoubleClick)
	ON_BN_CLICKED(IDC_SHARE_ADD, OnShareAdd)
	ON_BN_CLICKED(IDC_SHARE_REMOVE, OnShareRemove)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardSharePage property page

CWizardSharePage::CWizardSharePage() : CWizardPage(CWizardSharePage::IDD)
{
}

CWizardSharePage::~CWizardSharePage()
{
}

void CWizardSharePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHARE_FOLDERS, m_wndList);
	DDX_Control(pDX, IDC_SHARE_REMOVE, m_wndRemove);
}

/////////////////////////////////////////////////////////////////////////////
// CWizardSharePage message handlers

BOOL CWizardSharePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	Skin.Apply( L"CWizardSharePage", this );

	CRect rc;
	m_wndList.GetClientRect( &rc );
	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_TRANSPARENTBKGND|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP|LVS_EX_CHECKBOXES );
	m_wndList.InsertColumn( 0, L"Folder", LVCFMT_LEFT, rc.Width() - GetSystemMetrics( SM_CXVSCROLL ) );
	m_wndList.EnableToolTips( TRUE );
	ShellIcons.AttachTo( &m_wndList, 16 );	// .SetImageList()

	if ( m_wndList.SetBkImage( Skin.GetWatermark( L"CListCtrl" ) ) )		// || m_wndList.SetBkImage( Images.m_bmSystemWindow.m_hObject )		"System.Windows"
	{
		m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP|LVS_EX_CHECKBOXES );	// No LVS_EX_DOUBLEBUFFER
	}
	else
	{
		m_wndList.SetBkColor( Colors.m_crWindow );
		m_wndList.SetTextBkColor( Colors.m_crWindow );
	}

	m_wndList.SetTextColor( Colors.m_crText );

	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = LibraryFolders.GetFolderIterator(); pos; )
		{
			CLibraryFolder* pFolder = LibraryFolders.GetNextFolder( pos );

			m_wndList.InsertItem( LVIF_TEXT|LVIF_IMAGE, m_wndList.GetItemCount(),
				pFolder->m_sPath, 0, 0, SHI_FOLDER_OPEN, 0 );

			m_wndList.SetItemState( m_wndList.GetItemCount() - 1,
				UINT( ( pFolder->IsShared() != TRUE ? 1 : 2 ) << 12 ), LVIS_STATEIMAGEMASK );
		}

		CreateDirectory( Settings.Downloads.CompletePath );
		AddPhysicalFolder( Settings.Downloads.CompletePath );

		CreateDirectory( Settings.Downloads.CollectionPath );
		AddPhysicalFolder( Settings.Downloads.CollectionPath );

		CreateDirectory( Settings.Downloads.TorrentPath );
		AddPhysicalFolder( Settings.Downloads.TorrentPath );

		AddPhysicalFolder( (LPCTSTR)theApp.GetDownloadsFolder() );

		AddPhysicalFolder( (LPCTSTR)theApp.GetDocumentsFolder() );

		// ToDo: Check other common programs for download folder locations
		//strFolder = strPrograms + L"\\Shareaza\\Downloads";
		//AddPhysicalFolder( strFolder );
		//strFolder = strPrograms + L"\\Ares\\My Shared Folder";
		//AddPhysicalFolder( strFolder );
		//strFolder = strPrograms + L"\\Neo Mule\\Incoming";
		//AddPhysicalFolder( strFolder );
		//CString strFolder = theApp.GetProgramFilesFolder() + L"\\eMule\\Incoming";	// Takes several seconds
		AddPhysicalFolder( L"c:\\Program Files\\eMule\\Incoming" );
#ifdef WIN64
		AddPhysicalFolder( L"c:\\Program Files (x86)\\eMule\\Incoming" );
#endif

		AddRegistryFolder( HKEY_CURRENT_USER, L"Software\\Shareaza\\Shareaza\\Downloads", L"CompletePath" );
		AddRegistryFolder( HKEY_CURRENT_USER, L"Software\\Kazaa\\Transfer", L"DlDir0" );
		AddRegistryFolder( HKEY_CURRENT_USER, L"Software\\Xolox", L"completedir" );
		AddRegistryFolder( HKEY_CURRENT_USER, L"Software\\Xolox", L"sharedir" );
	}

	return TRUE;
}

void CWizardSharePage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

BOOL CWizardSharePage::OnSetActive()
{
	// Wizard Window Caption Workaround
	CString strCaption;
	GetWindowText( strCaption );
	GetParent()->SetWindowText( strCaption );

	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	return CWizardPage::OnSetActive();
}

void CWizardSharePage::AddPhysicalFolder(LPCTSTR pszFolder)
{
	DWORD nFlags = GetFileAttributes( pszFolder );

	if ( nFlags == 0xFFFFFFFF ) return;
	if ( ( nFlags & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) return;

	for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
	{
		if ( m_wndList.GetItemText( nItem, 0 ).CompareNoCase( pszFolder ) == 0 ) return;
	}

	m_wndList.InsertItem( LVIF_TEXT|LVIF_IMAGE, m_wndList.GetItemCount(),
		pszFolder, 0, 0, SHI_FOLDER_OPEN, 0 );
}

void CWizardSharePage::AddRegistryFolder(HKEY hRoot, LPCTSTR pszKey, LPCTSTR pszValue)
{
	TCHAR szFolder[ MAX_PATH ];
	DWORD dwType, dwFolder;
	HKEY hKey = NULL;

	if ( RegOpenKeyEx( hRoot, pszKey, 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) return;

	dwType = REG_SZ;
	dwFolder = MAX_PATH - 1;

	if ( RegQueryValueEx( hKey, pszValue, NULL, &dwType, (LPBYTE)szFolder, &dwFolder ) != ERROR_SUCCESS ||
		 dwType != REG_SZ )
	{
		RegCloseKey( hKey );
		return;
	}

	RegCloseKey( hKey );

	szFolder[ dwFolder ] = 0;

	AddPhysicalFolder( szFolder );
}

void CWizardSharePage::OnItemChangedShareFolders(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_wndRemove.EnableWindow( m_wndList.GetSelectedCount() > 0 );
	*pResult = 0;
}

void CWizardSharePage::OnShareAdd()
{
	LibraryFolders.AddSharedFolder( m_wndList );

	m_wndList.SetItemState( m_wndList.GetItemCount() - 1, 2 << 12, LVIS_STATEIMAGEMASK );	// Checked box
}

void CWizardSharePage::OnShareRemove()
{
	for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
	{
		if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
			m_wndList.DeleteItem( nItem-- );
	}
}

void CWizardSharePage::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	// Toggle checkmarks (newly selected items at second click)
	for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
	{
		if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
			m_wndList.SetItemState( nItem, UINT( ( m_wndList.GetCheck(nItem) ? 1 : 2 ) << 12 ), LVIS_STATEIMAGEMASK );
	}
}

LRESULT CWizardSharePage::OnWizardNext()
{
	if ( m_wndList.GetItemCount() == 0 )
	{
		if ( MsgBox( IDS_WIZARD_SHARE_CONFIRM, MB_ICONQUESTION|MB_YESNO ) == IDNO )
			return -1;
	}

	CWaitCursor pCursor;

	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = LibraryFolders.GetFolderIterator(); pos; )
		{
			CLibraryFolder* pFolder = LibraryFolders.GetNextFolder( pos );

			int nItem = 0;
			for ( ; nItem < m_wndList.GetItemCount(); nItem++ )
			{
				CString strFolder = m_wndList.GetItemText( nItem, 0 );
				if ( strFolder.CompareNoCase( pFolder->m_sPath ) == 0 )
				{
					if ( m_wndList.GetCheck(nItem) && ! pFolder->IsShared() )
						pFolder->SetShared( TRI_TRUE );
					else if ( ! m_wndList.GetCheck(nItem) && pFolder->IsShared() )
						pFolder->SetShared( TRI_FALSE );
					break;
				}
			}

			if ( nItem >= m_wndList.GetItemCount() )
				LibraryFolders.RemoveFolder( pFolder );
		}

		for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
		{
			LibraryFolders.AddFolder( m_wndList.GetItemText( nItem, 0 ), m_wndList.GetCheck( nItem ) );
		}
	}

	CFolderScanDlg dlgScan;
	dlgScan.DoModal();

	return 0;
}
