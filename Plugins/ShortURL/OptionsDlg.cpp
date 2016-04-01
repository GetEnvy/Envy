//
// OptionsDlg.cpp : Implementation of COptionsDlg
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2014 and Nikolay Raspopov 2014
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
#include "Plugin.h"
#include "OptionsDlg.h"

#ifndef PROGDLG_NOCANCEL		// IE6
#define PROGDLG_NOCANCEL		0x00000040
#define PROGDLG_MARQUEEPROGRESS	0x00000020
#endif

// COptionsDlg

COptionsDlg::COptionsDlg(CPlugin* pOwner)
	: m_pOwner	( pOwner )
{
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::LoadList( CString sURLs )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	ListView_DeleteAllItems( hWnd );

	LVITEM item = { LVIF_TEXT };
	while ( sURLs.GetLength() )
	{
		CString strURL = sURLs.SpanExcluding( L"|" );
		sURLs = sURLs.Mid( strURL.GetLength() + 1 );
		strURL.Trim();
		strURL.Replace( L"%7C", L"|" );

		if ( strURL.GetLength() > 4 )
		{
			item.pszText = const_cast< LPTSTR >( (LPCTSTR)strURL );
			ListView_InsertItem( hWnd, &item );
			++ item.iItem;
		}
	}

	UpdateInterface();
}

CString COptionsDlg::SaveList() const
{
	CString strURLs;
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	int nCount = ListView_GetItemCount( hWnd );
	for ( int i = 0; i < nCount; i++ )
	{
		CString strURL;
		ListView_GetItemText( hWnd, i, 0, strURL.GetBuffer( MAX_PATH ), MAX_PATH );
		strURL.ReleaseBuffer();
		strURL.Trim();
		strURL.Replace( L"|", L"%7C" );

		if ( strURL.GetLength() < 6 || StartsWith( strURL, LoadString( IDS_HINT ) ) )
			continue;
		if ( ! strURLs.IsEmpty() )
			strURLs += L'|';
		strURLs += strURL;
	}

	return strURLs;
}

int COptionsDlg::GetSelectedURL() const
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	int nCount = ListView_GetItemCount( hWnd );
	for ( int i = 0; i < nCount; i++ )
	{
		if ( ListView_GetItemState( hWnd, i, LVIS_SELECTED ) == LVIS_SELECTED )
			return i;
	}

	return -1;
}

void COptionsDlg::UpdateInterface()
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;
	const int nCount = ListView_GetItemCount( hWnd );
	const int nSelected = GetSelectedURL();

	GetDlgItem( IDC_TEST ).EnableWindow( nSelected >= 0 );
	GetDlgItem( IDC_UP ).EnableWindow( nSelected > 0 );
	GetDlgItem( IDC_DOWN ).EnableWindow( nSelected >= 0 && nSelected < nCount - 1 );
	GetDlgItem( IDC_NEW ).EnableWindow( nCount < 100 );
	GetDlgItem( IDC_DELETE ).EnableWindow( nSelected >= 0 );
}

void COptionsDlg::Edit()
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;
	const int nSelected = GetSelectedURL();
	if ( nSelected >= 0 )
	{
		::SetFocus( hWnd );
		ListView_EditLabel( hWnd, nSelected );
	}
}

LRESULT COptionsDlg::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CAxDialogImpl< COptionsDlg >::OnInitDialog( uMsg, wParam, lParam, bHandled );

	GetDlgItem( IDC_TEST ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_TEST ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	GetDlgItem( IDC_UP ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_UP ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	GetDlgItem( IDC_DOWN ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_DOWN ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	GetDlgItem( IDC_NEW ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_NEW ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	GetDlgItem( IDC_DELETE ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_DELETE ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );

	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	ListView_SetExtendedListViewStyle( hWnd, ListView_GetExtendedListViewStyle( hWnd ) |
		LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_ONECLICKACTIVATE );

	RECT rc;
	::GetClientRect( hWnd, &rc );
	LVCOLUMN col = { LVCF_FMT | LVCF_WIDTH, LVCFMT_LEFT, rc.right - rc.left - GetSystemMetrics( SM_CXVSCROLL ) - 2 };
	ListView_InsertColumn( hWnd, 0, &col );

	LoadList( GetURLs() );

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT COptionsDlg::OnClickedOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled )
{
	SaveURLs( SaveList() );

	EndDialog( wID );
	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled )
{
	EndDialog( wID );
	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedNew( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	int nSelected = GetSelectedURL();
	if ( nSelected < 0 )
		nSelected = 0;

	ListView_SetItemState( hWnd, nSelected, 0, LVIS_SELECTED );

	LVITEM item = { LVIF_TEXT | LVIF_STATE, nSelected, 0, LVIS_SELECTED, LVIS_SELECTED };
	CString strHint = LoadString( IDS_HINT );
	item.pszText = const_cast< LPTSTR >( (LPCTSTR)strHint );
	ListView_InsertItem( hWnd, &item );

	UpdateInterface();

	Edit();

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedDelete( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	const int nCount = ListView_GetItemCount( hWnd );
	const int nSelected = GetSelectedURL();
	if ( nSelected >= 0 )
	{
		ListView_DeleteItem( hWnd, nSelected );
		if ( nCount > 1 )
			ListView_SetItemState( hWnd, nSelected, LVIS_SELECTED, LVIS_SELECTED );

		UpdateInterface();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedUp( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	const int nSelected = GetSelectedURL();
	if ( nSelected > 0 )
	{
		CString strFirst;
		ListView_GetItemText( hWnd, nSelected - 1, 0, strFirst.GetBuffer( MAX_PATH ), MAX_PATH );
		strFirst.ReleaseBuffer();

		CString strSecond;
		ListView_GetItemText( hWnd, nSelected, 0, strSecond.GetBuffer( MAX_PATH ), MAX_PATH );
		strSecond.ReleaseBuffer();

		ListView_SetItemText( hWnd, nSelected - 1, 0, const_cast< LPTSTR >( (LPCTSTR)strSecond ) );
		ListView_SetItemText( hWnd, nSelected, 0, const_cast< LPTSTR >( (LPCTSTR)strFirst ) );

		ListView_SetItemState( hWnd, nSelected - 1, LVIS_SELECTED, LVIS_SELECTED );
		ListView_SetItemState( hWnd, nSelected, 0, LVIS_SELECTED );

		UpdateInterface();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedDown( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	const int nCount = ListView_GetItemCount( hWnd );
	const int nSelected = GetSelectedURL();
	if ( nSelected >= 0 && nSelected < nCount - 1 )
	{
		CString strFirst;
		ListView_GetItemText( hWnd, nSelected + 1, 0, strFirst.GetBuffer( MAX_PATH ), MAX_PATH );
		strFirst.ReleaseBuffer();

		CString strSecond;
		ListView_GetItemText( hWnd, nSelected, 0, strSecond.GetBuffer( MAX_PATH ), MAX_PATH );
		strSecond.ReleaseBuffer();

		ListView_SetItemText( hWnd, nSelected + 1, 0, const_cast< LPTSTR >( (LPCTSTR)strSecond ) );
		ListView_SetItemText( hWnd, nSelected, 0, const_cast< LPTSTR >( (LPCTSTR)strFirst ) );

		ListView_SetItemState( hWnd, nSelected + 1, LVIS_SELECTED, LVIS_SELECTED );
		ListView_SetItemState( hWnd, nSelected, 0, LVIS_SELECTED );

		UpdateInterface();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnItemChanged( int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& bHandled )
{
	UpdateInterface();

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnItemEdited( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;
	const NMLVDISPINFO* pdi = (const NMLVDISPINFO*)pNMHDR;

	if ( pdi->item.pszText )
	{
		ListView_SetItemText( hWnd, pdi->item.iItem, 0, pdi->item.pszText );
		ListView_SetItemState( hWnd, pdi->item.iItem, LVIS_SELECTED, LVIS_SELECTED );

		UpdateInterface();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnKeyDown( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled )
{
	const bool bControl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0;
	LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)pNMHDR;

	if ( bControl )
	{
		if ( pnkd->wVKey == VK_DOWN )
			OnClickedDown( 0, 0, 0, bHandled );
		else if ( pnkd->wVKey == VK_UP )
			OnClickedUp( 0, 0, 0, bHandled );
	}
	else
	{
		if ( pnkd->wVKey == VK_INSERT )
			OnClickedNew( 0, 0, 0, bHandled );
		else if ( pnkd->wVKey == VK_DELETE )
			OnClickedDelete( 0, 0, 0, bHandled );
		else if ( pnkd->wVKey == VK_F2 )
		{
			Edit();
			bHandled = TRUE;
		}
	}

	return 0;
}

LRESULT COptionsDlg::OnDblClick( int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& bHandled )
{
	Edit();

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedDefaults( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	LoadList( LoadString( IDS_URL ) );

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedTest( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	const int nSelected = GetSelectedURL();
	if ( nSelected >= 0 )
	{
		CString strURL;
		ListView_GetItemText( hWnd, nSelected, 0, strURL.GetBuffer( MAX_PATH ), MAX_PATH );
		strURL.ReleaseBuffer();

		if ( StartsWith( strURL, LoadString( IDS_HINT ) ) )
		{
			Edit();
			return 0;
		}

		CComPtr< IProgressDialog > pProgress;
		pProgress.CoCreateInstance( CLSID_ProgressDialog );
		if ( pProgress )
		{
			pProgress->SetTitle( LoadString( IDS_PROJNAME ) );
			pProgress->SetLine( 1, LoadString( IDS_PROGRESS ), FALSE, NULL );
			pProgress->SetLine( 2, strURL.Left( strURL.ReverseFind( L'/' ) ), FALSE, NULL );
			pProgress->StartProgressDialog( NULL, NULL, PROGDLG_NOTIME | PROGDLG_NOCANCEL | PROGDLG_MARQUEEPROGRESS, NULL );
		}

		CStringA sShortURL = m_pOwner->RequestURL( strURL + URLEncode( LoadString( IDS_TEST ) ) );

		if ( pProgress )
			pProgress->StopProgressDialog();

		MessageBox( sShortURL.IsEmpty() ? LoadString( IDS_FAILED ) : ( LoadString( IDS_URL_REPORT ) + L" " + CA2T( sShortURL ) ),
			LoadString( IDS_PROJNAME ), MB_OK | ( sShortURL.IsEmpty() ? MB_ICONERROR : MB_ICONINFORMATION ) );
	}

	bHandled = TRUE;
	return 0;
}
