//
// ConfigDlg.cpp : Implementation of CConfigDlg
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2009 and Nikolay Raspopov 2009
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "ConfigDlg.h"
#include "dllmain.h"

// CConfigDlg

CConfigDlg::CConfigDlg()
	: m_nActive( -1 )
{
}

void CConfigDlg::Load()
{
	CAtlMap< CString, CString > oData;
	LoadData( oData );

	for ( POSITION pos = oData.GetStartPosition(); pos; )
	{
		CString strExt, strCommand;
		oData.GetNextAssoc( pos, strExt, strCommand );
		Add( strExt, strCommand );
	}
}

void CConfigDlg::Save()
{
	CAtlMap< CString, CString > oData;

	CWindow wndList = GetDlgItem( IDC_LIST );
	int nCount = ListView_GetItemCount( wndList.m_hWnd );
	for ( int i = 0; i < nCount; i++ )
	{
		TCHAR szExt[ MAX_PATH ] = {};
		ListView_GetItemText( wndList.m_hWnd, i, 0, szExt, _countof( szExt ) );
		StrTrim( szExt, L" /t" );
		if ( ! *szExt )
			continue;

		TCHAR szCommand[ MAX_PATH ] = {};
		ListView_GetItemText( wndList.m_hWnd, i, 1, szCommand, _countof( szCommand ) );
		StrTrim( szCommand, L" /t" );
		if ( ! *szCommand )
			continue;

		oData.SetAt( szExt, szCommand );
	}

	SaveData( oData );
}

int CConfigDlg::GetActive() const
{
	CWindow wndList = GetDlgItem( IDC_LIST );

	int nCount = ListView_GetItemCount( wndList.m_hWnd );
	for ( int i = 0; i < nCount; i++ )
	{
		if ( ListView_GetItemState( wndList.m_hWnd, i, LVIS_SELECTED ) == LVIS_SELECTED )
			return i;
	}

	return -1;
}

void CConfigDlg::Select(int nIndex)
{
	CWindow wndList = GetDlgItem( IDC_LIST );

	int nCount = ListView_GetItemCount( wndList.m_hWnd );
	for ( int i = 0; i < nCount; i++ )
	{
		if ( ListView_GetItemState( wndList.m_hWnd, i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			if ( nIndex != i )
				ListView_SetItemState( wndList.m_hWnd, i, 0, LVIS_SELECTED );
		}
		else
		{
			if ( nIndex == i )
				ListView_SetItemState( wndList.m_hWnd, i, LVIS_SELECTED, LVIS_SELECTED );
		}
	}

	Update( nIndex );
}

void CConfigDlg::Update(int nIndex)
{
	m_nActive = nIndex;

	if ( m_nActive != -1 )
	{
		CWindow wndList = GetDlgItem( IDC_LIST );

		TCHAR szExt[ MAX_PATH ] = {};
		ListView_GetItemText( wndList.m_hWnd, m_nActive, 0, szExt, _countof( szExt ) );

		TCHAR szCommand[ MAX_PATH ] = {};
		ListView_GetItemText( wndList.m_hWnd, m_nActive, 1, szCommand, _countof( szCommand ) );

		SetDlgItemText( IDC_EXT, szExt );
		GetDlgItem( IDC_EXT ).EnableWindow( TRUE );
		SetDlgItemText( IDC_COMMAND, szCommand );
		GetDlgItem( IDC_COMMAND ).EnableWindow( TRUE );
	}
	else
	{
		SetDlgItemText( IDC_EXT, L"" );
		GetDlgItem( IDC_EXT ).EnableWindow( FALSE );
		SetDlgItemText( IDC_COMMAND, L"" );
		GetDlgItem( IDC_COMMAND ).EnableWindow( FALSE );
	}

	UpdateWindow();
}

void CConfigDlg::AddNew()
{
	Select( Add( L"ext", L"\"foo.exe\" \"%1\" \"%2\"" ) );
	GetDlgItem( IDC_EXT ).SetFocus();
	GetDlgItem( IDC_EXT ).SendMessage( EM_SETSEL, 0, (LPARAM)-1 );
}

int CConfigDlg::Add(LPCTSTR szExt, LPCTSTR szCommand)
{
	CWindow wndList = GetDlgItem( IDC_LIST );

	LVITEM item = { LVIF_TEXT };
	item.pszText = const_cast< LPTSTR >( szExt );
	item.iItem = ListView_InsertItem( wndList.m_hWnd, &item );
	if ( item.iItem != -1 )
	{
		item.iSubItem = 1;
		item.pszText = const_cast< LPTSTR >( szCommand );
		ListView_SetItem( wndList.m_hWnd, &item );
	}

	return item.iItem;
}

void CConfigDlg::Delete(int nIndex)
{
	Update( - 1 );

	CWindow wndList = GetDlgItem( IDC_LIST );

	ListView_DeleteItem( wndList.m_hWnd, nIndex );
}

LRESULT CConfigDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CConfigDlg>::OnInitDialog( uMsg, wParam, lParam, bHandled );

	TCHAR szTitle[ 64 ] = {};
	LoadString( _AtlBaseModule.GetResourceInstance(), IDS_PROJNAME, szTitle, 64 );
	SetWindowText( szTitle );

	CWindow wndList = GetDlgItem( IDC_LIST );

	ListView_SetExtendedListViewStyle( wndList.m_hWnd,
		ListView_GetExtendedListViewStyle( wndList.m_hWnd ) |
		LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
		LVS_EX_ONECLICKACTIVATE );

	RECT rc;
	wndList.GetClientRect( &rc );

	LVCOLUMN col = { LVCF_FMT | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT };
	col.cx = 70;
	col.pszText = L"Extension";
	ListView_InsertColumn( wndList.m_hWnd, 0, &col );
	col.cx = rc.right - rc.left - col.cx - GetSystemMetrics( SM_CXVSCROLL ) - 2;
	col.pszText = L"Command line";
	ListView_InsertColumn( wndList.m_hWnd, 1, &col );

	Load();

	Select( -1 );

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CConfigDlg::OnClickedOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	Save();

	EndDialog( wID );

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnClickedCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	EndDialog( wID );

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnBnClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	AddNew();

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnBnClickedDel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	int nIndex = ( m_nActive != -1 ) ? m_nActive : GetActive();
	if ( nIndex != -1 )
	{
		Select( nIndex );

		TCHAR szTitle[ 64 ] = {};
		LoadString( _AtlBaseModule.GetResourceInstance(), IDS_PROJNAME, szTitle, 64 );
		if ( MessageBox( L"Delete this item?", szTitle, MB_YESNO | MB_ICONQUESTION ) == IDYES )
		{
			Delete( nIndex );
		}
	}

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnNMClickList(int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& bHandled)
{
	Select( GetActive() );

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	if ( m_nActive != -1 )
	{
		CWindow wndList = GetDlgItem( IDC_LIST );

		TCHAR szCommand[ MAX_PATH ] = {};
		GetDlgItemText( IDC_COMMAND, szCommand, _countof( szCommand ) );

		TCHAR szPath[ MAX_PATH ];
		lstrcpyn( szPath, szCommand, _countof( szPath ) );
		PathRemoveArgs( szPath );
		PathUnquoteSpaces( szPath );

		OPENFILENAME ofn =
		{
			sizeof( OPENFILENAME ),
			m_hWnd,
			NULL,
			L"Applications (*.exe)\0*.exe\0All Files (*.*)\0*.*\0\0",
			NULL,
			0,
			0,
			szPath,
			_countof( szPath ),
			NULL,
			0,
			NULL,
			NULL,
			OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST
		};

		if ( GetOpenFileName( &ofn ) )
		{
			lstrcpyn( szCommand, szPath, _countof( szCommand ) );
			PathQuoteSpaces( szCommand );

			// Detect well-known previewers
			LPCTSTR szFilename = &szPath[ ofn.nFileOffset ];
			if ( lstrcmpi( szFilename, L"DivFix++.exe" ) == 0 )
				lstrcat( szCommand, L" -i \"%1\" -o \"%2\"" );
			else // Unknown
				lstrcat( szCommand, L" \"%1\" \"%2\"" );

			SetDlgItemText( IDC_COMMAND, szCommand );
			ListView_SetItemText( wndList.m_hWnd, m_nActive, 1, szCommand );

			GetDlgItem( IDC_COMMAND ).SetFocus();
		}
	}

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnLvnItemActivateList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast< LPNMITEMACTIVATE >( pNMHDR );

	Update( pNMIA->iItem );

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnEnChangeExt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	if ( m_nActive != -1 )
	{
		CWindow wndList = GetDlgItem( IDC_LIST );

		TCHAR szExt[ MAX_PATH ] = {};
		GetDlgItemText( IDC_EXT, szExt, _countof( szExt ) );
		StrTrim( szExt, L". /t" );

		ListView_SetItemText( wndList.m_hWnd, m_nActive, 0, szExt );
	}

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnEnChangeCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	if ( m_nActive != -1 )
	{
		CWindow wndList = GetDlgItem( IDC_LIST );

		TCHAR szCommand[ MAX_PATH ] = {};
		GetDlgItemText( IDC_COMMAND, szCommand, _countof( szCommand ) );
		StrTrim( szCommand, L" /t" );

		ListView_SetItemText( wndList.m_hWnd, m_nActive, 1, szCommand );
	}

	bHandled = TRUE;
	return 0;
}

LRESULT CConfigDlg::OnBnClickedWeb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	ShellExecute( GetActiveWindow(), L"open",
		L"http://getenvy.com/wiki/plugins", NULL, NULL, SW_SHOWNORMAL );

	bHandled = TRUE;
	return 0;
}
