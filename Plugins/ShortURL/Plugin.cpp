//
// Plugin.cpp : Implementation of CPlugin for ShortURL
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
//#include "..\..\Envy\Strings.h" (in StdAfx.h)

#ifndef PROGDLG_NOCANCEL
#define PROGDLG_NOCANCEL        0x00000040      // No cancel button (operation cannot be canceled, use sparingly)
#define PROGDLG_MARQUEEPROGRESS 0x00000020      // Use marquee progress (comctl32 v6 required)
#endif


void CPlugin::InsertCommand(LPCTSTR szTitle, const LPCWSTR* szMenu, UINT nID)
{
	for ( int i = 0 ; szMenu[ i ] ; ++i )
	{
		CComPtr< ISMenu > pMenu;
		if ( SUCCEEDED( m_pUserInterface->GetMenu( CComBSTR( szMenu[ i ] ), VARIANT_FALSE, &pMenu ) ) && pMenu )
		{
			LONG nPos = -1;
			CComPtr< ISMenu > pCopyMenu;
			if ( nID && SUCCEEDED( pMenu->get_Item( CComVariant( nID ), &pCopyMenu ) ) && pCopyMenu && SUCCEEDED( pCopyMenu->get_Position( &nPos ) ) )
			{
				// Check for existing menu item
				CComPtr< ISMenu > pCheckMenu;
				if ( SUCCEEDED( pMenu->get_Item( CComVariant( m_nCmdCheck ), &pCheckMenu ) ) && ! pCheckMenu )
				{
					// Insert new menu item
					CComPtr< ISMenu > pInsertMenu;
					if ( SUCCEEDED( pCopyMenu->get_Parent( &pInsertMenu ) ) && pInsertMenu )
						pInsertMenu->InsertCommand( nPos + 1, m_nCmdCheck, CComBSTR( szTitle ), NULL );
				}
			}
		}
	}
}

HRESULT CPlugin::Request(LPCWSTR szHash)
{
	ATLTRACE( "ShortURL : Request( %s )\n", (LPCSTR)CW2A( szHash ) );

	CComPtr< IProgressDialog > pProgress;
	pProgress.CoCreateInstance( CLSID_ProgressDialog );
	if ( pProgress )
	{
		pProgress->SetTitle( LoadString( IDS_PROJNAME ) );
		pProgress->SetLine( 1, LoadString( IDS_PROGRESS ), FALSE, NULL );
		pProgress->StartProgressDialog( NULL, NULL, PROGDLG_NOTIME|PROGDLG_NOCANCEL|PROGDLG_MARQUEEPROGRESS, NULL );
	}

	CStringA sShortURL;
	for ( CString sURLs = GetURLs() ; sURLs.GetLength() ; )		// Links defined in .rc
	{
		CString strURL = sURLs.SpanExcluding( L"|" );
		sURLs = sURLs.Mid( strURL.GetLength() + 1 );
		strURL.Trim();
		if ( strURL.GetLength() )
		{
			if ( pProgress )
				pProgress->SetLine( 2, strURL.Left( strURL.ReverseFind( L'/' ) ), FALSE, NULL );
			sShortURL = RequestURL( strURL + URLEncode( szHash ) );
			if ( ! sShortURL.IsEmpty() )
				break;
		}
	}

	if ( ! sShortURL.IsEmpty() )
	{
		HWND hWnd = NULL;
		m_pUserInterface->get_MainWindowHwnd( &hWnd );

		BOOL bSuccess = FALSE;
		if ( OpenClipboard( hWnd ) )
		{
			if ( HANDLE hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, sShortURL.GetLength() + 1 ) )
			{
				if ( LPVOID pMem = GlobalLock( hMem ) )
				{
					CopyMemory( pMem, (LPCSTR)sShortURL, sShortURL.GetLength() + 1 );
					GlobalUnlock( hMem );
				}

				EmptyClipboard();

				bSuccess = ( SetClipboardData( CF_TEXT, hMem ) != NULL );
			}
			CloseClipboard();
		}

		if ( pProgress )
			pProgress->SetLine( 2, LoadString( IDS_SUCCESS ), FALSE, NULL );
		m_pApplication->Message( MSG_TRAY | MSG_NOTICE | MSG_FACILITY_DEFAULT, CComBSTR( LoadString( IDS_SUCCESS ) ) );

		m_pApplication->Message( MSG_INFO | MSG_FACILITY_DEFAULT, CComBSTR( LoadString( IDS_URL_REPORT ) + CA2T( sShortURL ) ) );
	}
	else
	{
		if ( pProgress )
			pProgress->SetLine( 2, LoadString( IDS_FAILED ), FALSE, NULL );
		m_pApplication->Message( MSG_TRAY | MSG_ERROR | MSG_FACILITY_DEFAULT, CComBSTR( LoadString( IDS_FAILED ) ) );
	}

	if ( pProgress )
		pProgress->StopProgressDialog();

	return S_OK;
}

CStringA CPlugin::RequestURL(LPCWSTR szURL)
{
	CStringA sResponse;

	CComBSTR bstrSmartAgent;
	m_pApplication->get_SmartAgent( &bstrSmartAgent );

	if ( HINTERNET hInternet = InternetOpen( CString( bstrSmartAgent ), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 ) )
	{
		if ( HINTERNET hURL = InternetOpenUrl( hInternet, szURL, L"", 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_DONT_CACHE, NULL ) )
		{
			DWORD nLength = 4;
			DWORD nStatusCode;
			if ( HttpQueryInfo( hURL, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &nStatusCode, &nLength, 0 ) && nStatusCode >= 200 && nStatusCode < 400 )
			{
				for ( DWORD nRemaining = 0; InternetQueryDataAvailable( hURL, &nRemaining, 0, 0 ) && nRemaining > 0; )
				{
					DWORD nRead = 0;
					CStringA sBuff;
					BOOL bResult = InternetReadFile( hURL, sBuff.GetBuffer( nRemaining + 1 ), nRemaining, &nRead );
					sBuff.ReleaseBuffer( nRead );
					if ( ! bResult )
						break;
					sResponse += sBuff;
					if ( sResponse.GetLength() > 256 )
						break;
				}
				sResponse.Trim();
			}
			InternetCloseHandle( hURL );
		}
		InternetCloseHandle( hInternet );
	}

	sResponse.Trim( " \t\r\n" );
	if ( sResponse.GetLength() < 16 || sResponse.Left( 7 ).CompareNoCase( "http://" ) )
		return CStringA();

	return sResponse;
}

// IGeneralPlugin

STDMETHODIMP CPlugin::SetApplication(
	/* [in] */ IApplication __RPC_FAR *pApplication)
{
	if ( ! pApplication )
		return E_POINTER;

	m_pApplication = pApplication;
	return m_pApplication->get_UserInterface( &m_pUserInterface );
}

STDMETHODIMP CPlugin::QueryCapabilities(
	/* [in] */ DWORD __RPC_FAR *pnCaps)
{
	if ( ! pnCaps )
		return E_POINTER;

	return S_OK;
}

STDMETHODIMP CPlugin::Configure()
{
	COptionsDlg dlg( this );
	dlg.DoModal();
	return S_OK;
}

STDMETHODIMP CPlugin::OnSkinChanged()
{
	// Recreate lost menu items
	return InsertCommands();
}

// ICommandPlugin

STDMETHODIMP CPlugin::RegisterCommands()
{
	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	HRESULT hr = m_pUserInterface->RegisterCommand( CComBSTR( LoadString( IDS_COMMAND ) ),
		LoadIcon( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_ICON ) ), &m_nCmdCheck );
	if ( ! SUCCEEDED( hr ) )
		return E_FAIL;

	m_pUserInterface->AddString( m_nCmdCheck, CComBSTR( LoadString( IDS_COMMAND_TIP ) ) );
	return S_OK;
}

STDMETHODIMP CPlugin::InsertCommands()
{
	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	CString strMenuItem = LoadString( IDS_MENU_ITEM );

	// Insert before "Copy URI..." item
	{
		UINT nDownloadID = 0;
		m_pUserInterface->NameToID( CComBSTR( L"ID_DOWNLOADS_URI" ), &nDownloadID );
		const LPCWSTR szDownloadMenu[] = {  L"CDownloadsWnd.Download", L"CDownloadsWnd.Completed", L"CDownloadsWnd.Seeding", NULL };
		InsertCommand( strMenuItem, szDownloadMenu, nDownloadID );
	}

	// Insert before "Copy URI..." item
	{
		UINT nSearchID = 0;
		m_pUserInterface->NameToID( CComBSTR( L"ID_SEARCH_URI" ), &nSearchID );
		const LPCWSTR szSearchMenu[] = {  L"CSearchWnd", L"CBrowseHostWnd", L"CHitMonitorWnd", NULL };
		InsertCommand( strMenuItem, szSearchMenu, nSearchID );
	}

	// Insert before "Copy URI..." item
	{
		UINT nLibraryID = 0;
		m_pUserInterface->NameToID( CComBSTR( L"ID_LIBRARY_URI" ), &nLibraryID );
		const LPCWSTR szLibraryMenu[] = {  L"CLibraryFileView", NULL };		// Was "CLibraryFileView.Physical", "CLibraryFileView.Virtual"
		InsertCommand( strMenuItem, szLibraryMenu, nLibraryID );
	}

	return S_OK;
}

STDMETHODIMP CPlugin::OnUpdate(
	/* [in] */ UINT nCommandID,
	/* [out][in] */ TRISTATE __RPC_FAR *pbVisible,
	/* [out][in] */ TRISTATE __RPC_FAR *pbEnabled,
	/* [out][in] */ TRISTATE __RPC_FAR *pbChecked)
{
	if ( ! pbVisible || ! pbEnabled || ! pbChecked )
		return E_POINTER;

	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	if ( nCommandID != m_nCmdCheck )
		return S_FALSE;

	*pbEnabled = TRI_FALSE;
	*pbVisible = TRI_TRUE;
	*pbChecked = TRI_UNKNOWN;

	CComPtr< IGenericView > pGenericView;
	if ( SUCCEEDED( m_pUserInterface->get_ActiveView( &pGenericView ) ) && pGenericView )
	{
		LONG nCount = 0;
		if ( SUCCEEDED( pGenericView->get_Count( &nCount ) ) && nCount == 1 )
			*pbEnabled = TRI_TRUE;
	}

	return S_OK;
}

STDMETHODIMP CPlugin::OnCommand(
	/* [in] */ UINT nCommandID)
{
	ATLTRACE( "ShortURL : OnCommand( %d )\n", nCommandID );

	if ( ! m_pUserInterface )
	{
		ATLTRACE( "ShortURL : OnCommand : No user interface.\n" );
		return E_UNEXPECTED;
	}

	if ( nCommandID != m_nCmdCheck )
		return S_FALSE;

	CComPtr< IGenericView > pGenericView;
	HRESULT hr = m_pUserInterface->get_ActiveView( &pGenericView );
	if ( SUCCEEDED( hr ) && pGenericView )
	{
		LONG nCount = 0;
		hr = pGenericView->get_Count( &nCount );
		if ( SUCCEEDED( hr ) && nCount )
		{
			for ( LONG i = 0; i < nCount; ++i )
			{
				CComVariant pItem;
				hr = pGenericView->get_Item( CComVariant( i ), &pItem );
				if ( FAILED( hr ) )
				{
					ATLTRACE( "ShortURL : OnCommand() : Get item error: 0x%08x\n", hr );
					break;
				}

				CComBSTR pMagnet;
				if ( pItem.vt == VT_I4 )
				{
					CComPtr< ILibrary > pLibrary;
					hr = m_pApplication->get_Library( &pLibrary );
					if ( SUCCEEDED( hr ) && pLibrary )
					{
						CComPtr< ILibraryFile > pLibraryFile;
						hr = pLibrary->FindByIndex( pItem.lVal, &pLibraryFile );
						if ( SUCCEEDED( hr ) && pLibraryFile )
							pLibraryFile->get_Magnet( &pMagnet );
						else
							ATLTRACE( "ShortURL : OnCommand() : Find file by index error: 0x%08x\n", hr );
					}
					else
						ATLTRACE( "ShortURL : OnCommand() : Get Library error: 0x%08x\n", hr );
				}
				else if ( pItem.vt == VT_DISPATCH )
				{
					CComQIPtr< IEnvyFile > pEnvyFile( pItem.pdispVal );
					if ( pEnvyFile )
						pEnvyFile->get_Magnet( &pMagnet );
				}
				else
					ATLTRACE( "ShortURL : OnCommand() : Unknown item data.\n" );

				if ( pMagnet.Length() )
					Request( pMagnet );
				else
					ATLTRACE( "ShortURL : OnCommand() : No compatible hashes found.\n" );
			}

			return S_OK;
		}
		else
			ATLTRACE( "ShortURL : OnCommand() : No files selected: 0x%08x\n", hr );
	}
	else
		ATLTRACE( "ShortURL : OnCommand() : Active view get error: 0x%08x\n", hr );


	return S_FALSE;
}
