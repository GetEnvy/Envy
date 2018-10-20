//
// DlgShareManager.cpp
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
#include "DlgShareManager.h"
#include "DlgFolderScan.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "ShellIcons.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CShareManagerDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CShareManagerDlg, CSkinDialog)
	ON_BN_CLICKED(IDC_SHARE_ADD, OnShareAdd)
	ON_BN_CLICKED(IDC_SHARE_REMOVE, OnShareRemove)
	ON_NOTIFY(NM_DBLCLK, IDC_SHARE_FOLDERS, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SHARE_FOLDERS, OnItemChangedShareFolders)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CShareManagerDlg dialog

CShareManagerDlg::CShareManagerDlg(CWnd* pParent) : CSkinDialog( CShareManagerDlg::IDD, pParent )
{
}

void CShareManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHARE_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_SHARE_FOLDERS, m_wndList);
}

/////////////////////////////////////////////////////////////////////////////
// CShareManagerDlg message handlers

BOOL CShareManagerDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CShareManagerDlg", IDR_LIBRARYFRAME );

	CRect rc;
	m_wndList.GetClientRect( &rc );
	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_TRANSPARENTBKGND|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP|LVS_EX_CHECKBOXES );
	m_wndList.InsertColumn( 0, L"Folder", LVCFMT_LEFT, rc.right - GetSystemMetrics( SM_CXVSCROLL ) );
	m_wndList.EnableToolTips( TRUE );
	ShellIcons.AttachTo( &m_wndList, 16 );	// m_wndList.SetImageList()

	if ( m_wndList.SetBkImage( Skin.GetWatermark( L"CListCtrl" ) ) )		// || m_wndList.SetBkImage( Images.m_bmSystemWindow.m_hObject )		"System.Windows"
		m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_TRANSPARENTBKGND|LVS_EX_LABELTIP|LVS_EX_CHECKBOXES );	// No LVS_EX_DOUBLEBUFFER
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
	}

	m_wndRemove.EnableWindow( FALSE );

	return TRUE;
}

void CShareManagerDlg::OnItemChangedShareFolders(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
	m_wndRemove.EnableWindow( m_wndList.GetSelectedCount() > 0 );
}

void CShareManagerDlg::OnShareAdd()
{
	LibraryFolders.AddSharedFolder( m_wndList );

	m_wndList.SetItemState( m_wndList.GetItemCount() - 1, 2 << 12, LVIS_STATEIMAGEMASK );	// Checked box
}

void CShareManagerDlg::OnShareRemove()
{
	for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
	{
		if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
			m_wndList.DeleteItem( nItem-- );
	}
}

void CShareManagerDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	// Toggle checkmarks (newly selected items at second click)
	for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
	{
		if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
			m_wndList.SetItemState( nItem, UINT( ( m_wndList.GetCheck(nItem) ? 1 : 2 ) << 12 ), LVIS_STATEIMAGEMASK );
		//	m_wndList.SetCheck( m_wndList.GetCheck(nItem) ? BST_UNCHECKED : BST_CHECKED);
	}
}

void CShareManagerDlg::OnOK()
{
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

	CDialog::OnOK();
}
