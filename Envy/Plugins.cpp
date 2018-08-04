//
// Plugins.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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
#include "Plugins.h"
#include "SharedFile.h"
#include "Application.h"
#include "CtrlCoolBar.h"
#include "WndPlugin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


CPlugins Plugins;

//////////////////////////////////////////////////////////////////////
// CPlugins construction

CPlugins::CPlugins()
	: m_nCommandID	( ID_PLUGIN_FIRST )
	, m_inCLSID		( CLSID_NULL )
{
}

BOOL CPlugins::Register(const CString& sPath)
{
	Clear();

	DWORD nSucceeded = 0, nFailed = 0;

	LPCTSTR szParam = AfxGetPerUserRegistration() ? L"/RegServerPerUser" : L"/RegServer";

	CFileFind finder;
	BOOL bWorking = finder.FindFile( sPath + L"\\*.*" );	// .DLLs +.EXEs
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();
		const CString strPath = finder.GetFilePath();
		const CString strName = finder.GetFileName();
		const CString strExt  = PathFindExtension( strName );

		if ( strExt.CompareNoCase( L".dll" ) == 0 )
		{
			if ( strName == L"WebHook.dll" && ! Settings.Downloads.WebHookEnable )
				continue;	// Skip WebHook Integration

			if ( HINSTANCE hDll = LoadLibrary( strPath ) )
			{
				HRESULT hr = S_FALSE;

				HRESULT (WINAPI *pfnDllInstall)(BOOL bInstall, LPCWSTR pszCmdLine);
				(FARPROC&)pfnDllInstall = GetProcAddress( hDll, "DllInstall" );
				if ( pfnDllInstall && AfxGetPerUserRegistration() )
				{
					hr = pfnDllInstall( TRUE, L"user" );
				}
				else	// Should never happen
				{
					HRESULT (WINAPI *pfnDllRegisterServer)(void);
					(FARPROC&)pfnDllRegisterServer = GetProcAddress( hDll, "DllRegisterServer" );
					if ( pfnDllRegisterServer )
						hr = pfnDllRegisterServer();
				}

				if ( hr == S_OK )
				{
					nSucceeded++;
					theApp.Message( MSG_NOTICE, L"Registered plugin: %s", (LPCTSTR)strName );
				}
				else if ( FAILED( hr ) )
				{
					nFailed++;
					theApp.Message( MSG_ERROR, L"Failed to register plugin: %s : 0x%08x", strName, hr );
				}

				FreeLibrary( hDll );
			}
		}
		else if ( strExt.CompareNoCase( L".exe" ) == 0 )
		{
			DWORD dwSize = GetFileVersionInfoSize( sPath, &dwSize );
			CAutoVectorPtr< BYTE > pBuffer( new BYTE[ dwSize ] );
			if ( pBuffer && GetFileVersionInfo( sPath, NULL, dwSize, pBuffer ) )
			{
				LPCWSTR pValue = NULL;
				if ( VerQueryValue( pBuffer, L"\\StringFileInfo\\000004b0\\SpecialBuild", (void**)&pValue, (UINT*)&dwSize ) &&
					 pValue && dwSize && _wcsicmp( pValue, L"plugin" ) == 0 )
				{
					SHELLEXECUTEINFO sei =
					{
						sizeof( SHELLEXECUTEINFO ),
						SEE_MASK_NOCLOSEPROCESS,
						NULL,
						NULL,
						strPath,
						szParam,
						sPath,
						SW_HIDE
					};
					DWORD dwError = ERROR_INVALID_FUNCTION;
					if ( ShellExecuteEx( &sei ) )
					{
						WaitForSingleObject( sei.hProcess, INFINITE );
						GetExitCodeProcess( sei.hProcess, &dwError );
						CloseHandle( sei.hProcess );
					}
					else
						dwError = GetLastError();

					if ( dwError == ERROR_SUCCESS )
					{
						nSucceeded++;
						theApp.Message( MSG_NOTICE, L"Registered plugin: %s", (LPCTSTR)strName );
					}
					else
					{
						nFailed++;
						theApp.Message( MSG_ERROR, L"Failed to register plugin: %s : 0x%08x", strName, dwError );
					}
				}
			}
		}
	}

	return ( nSucceeded != 0 && nFailed == 0 );
}

//////////////////////////////////////////////////////////////////////
// CPlugins enumerate

void CPlugins::Enumerate()
{
	HUSKEY hKey = NULL;
	if ( SHRegOpenUSKey( REGISTRY_KEY L"\\Plugins\\General",
		KEY_READ, NULL, &hKey, FALSE ) != ERROR_SUCCESS ) return;

	for ( DWORD nKey = 0 ; ; nKey++ )
	{
		TCHAR szName[ 128 ], szCLSID[ 64 ];
		DWORD dwType, dwName = _countof( szName ), dwCLSID = sizeof( szCLSID );

		if ( SHRegEnumUSValue( hKey, nKey, szName, &dwName, &dwType,
			(LPBYTE)szCLSID, &dwCLSID, SHREGENUM_DEFAULT ) != ERROR_SUCCESS ) break;

		if ( dwType != REG_SZ ) continue;
		szCLSID[ 38 ] = 0;

		CLSID pCLSID;
		if ( ! Hashes::fromGuid( szCLSID, &pCLSID ) ) continue;

		CQuickLock oLock( m_pSection );

		for ( POSITION pos = GetIterator() ; pos ; )
		{
			if ( GetNext( pos )->m_pCLSID == pCLSID )
			{
				pCLSID = GUID_NULL;
				break;
			}
		}

		if ( pCLSID == GUID_NULL ) continue;

		if ( CPlugin* pPlugin = new CPlugin( pCLSID, szName ) )
		{
			m_pList.AddTail( pPlugin );

			if ( LookupEnable( pCLSID ) )
				pPlugin->Start();
		}
	}

	SHRegCloseUSKey( hKey );
}

//////////////////////////////////////////////////////////////////////
// CPlugins clear

void CPlugins::Clear()
{
	CloseThread();

	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}
	m_pList.RemoveAll();
}

//void CPlugins::UnloadPlugin(REFCLSID pCLSID)
//{
//	CSingleLock oLock( &m_pSection, FALSE );
//	if ( ! oLock.Lock( 500 ) ) return;
//
//	// Delete from cache
//	CPluginPtr* pGITPlugin = NULL;
//	if ( m_pCache.Lookup( pCLSID, pGITPlugin ) )
//	{
//		m_pCache.RemoveKey( pCLSID );
//		delete pGITPlugin;
//	}
//
//	// Delete from generic plugins list
//	for ( POSITION pos = GetIterator() ; pos ; )
//	{
//		POSITION posOrig = pos;
//		CPlugin* pPlugin = GetNext( pos );
//		if ( pPlugin->m_pCLSID == pCLSID )
//		{
//			m_pList.RemoveAt( posOrig );
//			delete pPlugin;
//		}
//	}
//}

//////////////////////////////////////////////////////////////////////
// CPlugins CLSID helpers

BOOL CPlugins::LookupCLSID(LPCTSTR pszGroup, LPCTSTR pszKey, CLSID& pCLSID) const
{
	CString strCLSID = theApp.GetProfileString(
		CString( L"Plugins\\" ) + pszGroup, pszKey, L"" );
	return ! strCLSID.IsEmpty() &&
		Hashes::fromGuid( strCLSID, &pCLSID ) &&
		LookupEnable( pCLSID, pszKey );
}

BOOL CPlugins::LookupEnable(REFCLSID pCLSID, LPCTSTR pszExt) const
{
	HKEY hPlugins = NULL;

	CString strCLSID = Hashes::toGuid( pCLSID );

	if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER,
		REGISTRY_KEY L"\\Plugins", 0, KEY_ALL_ACCESS, &hPlugins ) )
	{
		DWORD nType = REG_SZ, nValue = 0;
		if ( ERROR_SUCCESS == RegQueryValueEx( hPlugins, strCLSID, NULL, &nType, NULL, &nValue ) )
		{
			// Upgrade here; Smart upgrade doesn't work
			if ( nType == REG_DWORD )
			{
				BOOL bEnabled = theApp.GetProfileInt( L"Plugins", strCLSID, TRUE );
				RegCloseKey( hPlugins );
				theApp.WriteProfileString( L"Plugins", strCLSID, bEnabled ? L"" : L"-" );
				return bEnabled;
			}
		}
		RegCloseKey( hPlugins );
	}

	CString strExtensions = theApp.GetProfileString( L"Plugins", strCLSID, L"" );

	if ( strExtensions.IsEmpty() )
		return TRUE;
	if ( strExtensions == L"-" )		// For plugins without associations
		return FALSE;
	if ( strExtensions[ 0 ] == L'-' )
		strExtensions = strExtensions.Mid( 1 );

	if ( pszExt )	// Checking only a certain extension
	{
		CString strToFind;
		strToFind.Format( L"|%s|", pszExt );
		return strExtensions.Find( strToFind ) != -1;
	}

	// For Settings page
	CStringArray oTokens;
	Split( strExtensions, L'|', oTokens );
	INT_PTR nTotal = oTokens.GetCount();
	INT_PTR nChecked = 0;

	for ( INT_PTR nToken = 0 ; nToken < nTotal ; nToken++ )
	{
		CString strToken = oTokens.GetAt( nToken );
		if ( strToken[ 0 ] != L'-' )
			nChecked++;
	}

	if ( nChecked == 0 ) return FALSE;

	return TRUE;
}

IUnknown* CPlugins::GetPlugin(LPCTSTR pszGroup, LPCTSTR pszType)
{
	CLSID pCLSID;
	if ( ! LookupCLSID( pszGroup, pszType, pCLSID ) )
		return NULL;	// Disabled

	for ( int i = 0 ; ; ++i )
	{
		{
			CSingleLock pLock( &m_pSection );
			if ( ! SafeLock( pLock ) ) return NULL;

			CComPtr< IUnknown > pPlugin;
			CPluginPtr* pGITPlugin = NULL;
			if ( m_pCache.Lookup( pCLSID, pGITPlugin ) )
			{
				if ( ! pGITPlugin )
					return NULL;

				if ( SUCCEEDED( /*hr =*/ pGITPlugin->m_pGIT.CopyTo( &pPlugin ) ) )
					return pPlugin.Detach();

				TRACE( "Invalid plugin \"%s\"-\"%s\" %s\n", (LPCSTR)CT2A( pszGroup ), (LPCSTR)CT2A( pszType ), (LPCSTR)CT2A( Hashes::toGuid( pCLSID ) ) );
			}

			if ( i == 1 )
				break;

			m_inCLSID = pCLSID;

			// Create new one
			if ( ! BeginThread( "PluginCache" ) )
				break;	// Something really bad
		}

		Wakeup();									// Start process
		WaitForSingleObject( m_pReady, INFINITE );	// Wait for result
	}

	return NULL;
}

BOOL CPlugins::ReloadPlugin(LPCTSTR pszGroup, LPCTSTR pszType)
{
	CLSID pCLSID;
	if ( ! LookupCLSID( pszGroup, pszType, pCLSID ) )
		return FALSE;	// Disabled

	{
		CQuickLock oLock( m_pSection );

		m_inCLSID = pCLSID;

		// Create new one
		if ( ! BeginThread( "PluginCache" ) )
			return FALSE;	// Something really bad
	}

	Wakeup();									// Start process
	WaitForSingleObject( m_pReady, INFINITE );	// Wait for result

	return TRUE;
}

void CPlugins::OnRun()
{
	while ( IsThreadEnabled() )
	{
		Doze( 1000 );

		if ( ! IsThreadEnabled() )
			break;

		if ( m_inCLSID == CLSID_NULL )
		{
			m_pReady.PulseEvent();
			continue;
		}

		CQuickLock oLock( m_pSection );

		// Revoke interface
		CPluginPtr* pGITPlugin = NULL;
		if ( m_pCache.Lookup( m_inCLSID, pGITPlugin ) )
		{
			delete pGITPlugin;

			TRACE( "Dropped plugin %s\n", (LPCSTR)CT2A( Hashes::toGuid( m_inCLSID ) ) );
		}

		m_pCache.SetAt( m_inCLSID, NULL );

		HINSTANCE hRes = AfxGetResourceHandle();

		pGITPlugin = new CPluginPtr;
		if ( pGITPlugin )
		{
			//HRESULT hr;

			// Create plugin, and add plugin interface to GIT	(Note: r9495 breaks GFL)
			if ( //SUCCEEDED( /*hr =*/ pGITPlugin->m_pIUnknown.CoCreateInstance( m_inCLSID, NULL, CLSCTX_LOCAL_SERVER ) ) ||
				 SUCCEEDED( /*hr =*/ pGITPlugin->m_pIUnknown.CoCreateInstance( m_inCLSID ) ) &&
				 SUCCEEDED( /*hr =*/ pGITPlugin->m_pGIT.Attach( pGITPlugin->m_pIUnknown ) ) )
			{
				m_pCache.SetAt( m_inCLSID, pGITPlugin );

				TRACE( "Created plugin %s\n", (LPCSTR)CT2A( Hashes::toGuid( m_inCLSID ) ) );
			}
			else
				delete pGITPlugin;
		}

		AfxSetResourceHandle( hRes );

		m_inCLSID = CLSID_NULL;

		m_pReady.SetEvent();
	}

	CQuickLock oLock( m_pSection );

	// Revoke all interfaces
	for ( POSITION pos = m_pCache.GetStartPosition() ; pos ; )
	{
		CLSID pCLSID;
		CPluginPtr* pGITPlugin = NULL;
		m_pCache.GetNextAssoc( pos, pCLSID, pGITPlugin );
		delete pGITPlugin;
	}
	m_pCache.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CPlugins skin changed event

void CPlugins::OnSkinChanged()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pPlugin )
			pPlugin->m_pPlugin->OnSkinChanged();
	}
}

void CPlugins::InsertCommands()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pCommand )
			pPlugin->m_pCommand->InsertCommands();
	}
}

//////////////////////////////////////////////////////////////////////
// CPlugins command ID registration

void CPlugins::RegisterCommands()
{
	m_nCommandID = ID_PLUGIN_FIRST;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );
		if ( pPlugin->m_pCommand )
			pPlugin->m_pCommand->RegisterCommands();
	}
}

UINT CPlugins::GetCommandID()
{
	return m_nCommandID++;
}

//////////////////////////////////////////////////////////////////////
// CPlugins command handling

BOOL CPlugins::OnUpdate(CChildWnd* pActiveWnd, CCmdUI* pCmdUI)
{
	UINT nCommandID		= pCmdUI->m_nID;
	TRISTATE bVisible	= TRI_TRUE;
	TRISTATE bEnabled	= TRI_TRUE;
	TRISTATE bChecked	= TRI_UNKNOWN;

	CCoolBarItem* pCoolUI = CCoolBarItem::FromCmdUI( pCmdUI );

	if ( pActiveWnd != NULL && pActiveWnd->IsKindOf( RUNTIME_CLASS(CPluginWnd) ) )
	{
		CPluginWnd* pPluginWnd = (CPluginWnd*)pActiveWnd;

		if ( pPluginWnd->m_pOwner )
		{
			if ( pPluginWnd->m_pOwner->OnUpdate( nCommandID, &bVisible, &bEnabled, &bChecked ) == S_OK )
			{
				if ( bVisible != TRI_UNKNOWN && pCoolUI != NULL )
					pCoolUI->Show( bVisible == TRI_TRUE );
				if ( bEnabled != TRI_UNKNOWN )
					pCmdUI->Enable( bEnabled == TRI_TRUE );
				if ( bChecked != TRI_UNKNOWN )
					pCmdUI->SetCheck( bChecked == TRI_TRUE );

				return TRUE;
			}
		}
	}

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pCommand )
		{
			if ( pPlugin->m_pCommand->OnUpdate( nCommandID, &bVisible, &bEnabled, &bChecked ) == S_OK )
			{
				if ( bVisible != TRI_UNKNOWN && pCoolUI != NULL )
					pCoolUI->Show( bVisible == TRI_TRUE );
				if ( bEnabled != TRI_UNKNOWN )
					pCmdUI->Enable( bEnabled == TRI_TRUE );
				if ( bChecked != TRI_UNKNOWN )
					pCmdUI->SetCheck( bChecked == TRI_TRUE );

				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CPlugins::OnCommand(CChildWnd* pActiveWnd, UINT nCommandID)
{
	if ( pActiveWnd != NULL && pActiveWnd->IsKindOf( RUNTIME_CLASS(CPluginWnd) ) )
	{
		CPluginWnd* pPluginWnd = (CPluginWnd*)pActiveWnd;

		if ( pPluginWnd->m_pOwner )
		{
			if ( pPluginWnd->m_pOwner->OnCommand( nCommandID ) == S_OK )
				return TRUE;
		}
	}

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pCommand )
		{
			if ( pPlugin->m_pCommand->OnCommand( nCommandID ) == S_OK )
				return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CPlugins file execution events

BOOL CPlugins::OnExecuteFile(LPCTSTR pszFile, BOOL bUseImageViewer)
{
	CPlugin* pImageViewer = NULL;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pExecute )
		{
			if ( pPlugin->m_sName == L"Envy Image Viewer" )
			{
				pImageViewer = pPlugin;
				continue;
			}
			if ( pPlugin->m_pExecute->OnExecute( CComBSTR( pszFile ) ) == S_OK )
				return TRUE;
		}
	}

	if ( bUseImageViewer && pImageViewer )
		return ( pImageViewer->m_pExecute->OnExecute( CComBSTR( pszFile ) ) == S_OK );

	return FALSE;
}

BOOL CPlugins::OnEnqueueFile(LPCTSTR pszFile)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pExecute )
		{
			if ( pPlugin->m_pExecute->OnEnqueue( CComBSTR( pszFile ) ) == S_OK )
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CPlugins::OnChatMessage(LPCTSTR pszChatID, BOOL bOutgoing, LPCTSTR pszFrom, LPCTSTR pszTo, LPCTSTR pszMessage)
{
	// IChatPlugin capturing (IRC/direct)

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pChat )
		{
			pPlugin->m_pChat->OnChatMessage(
				CComBSTR( pszChatID ),
				( bOutgoing ? VARIANT_TRUE : VARIANT_FALSE ),
				CComBSTR( pszFrom ),
				CComBSTR( pszTo ),
				CComBSTR( pszMessage ) );
		}
	}

	return TRUE;
}

BOOL CPlugins::OnNewFile(CLibraryFile* pFile)
{
	// ILibraryFile capturing (New file notification)

	ILibraryFile* pIFile = (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pLibrary && pPlugin->m_pLibrary->OnNewFile( pIFile ) == S_OK )
			return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CPlugin construction

CPlugin::CPlugin(REFCLSID pCLSID, LPCTSTR pszName)
	: m_pCLSID	( pCLSID )
	, m_sName	( pszName )
	, m_nCapabilities ( 0 )
{
}

CPlugin::~CPlugin()
{
	Stop();
}

//////////////////////////////////////////////////////////////////////
// CPlugin start / stop operations

BOOL CPlugin::Start()
{
	HRESULT hr;

	if ( m_pPlugin )
		return FALSE;	// Already initialized

	CComPtr< IApplication > pApplication;
	hr = CApplication::GetApp( &pApplication );
	if ( FAILED( hr ) )
		return FALSE;	// Something very bad

	// Note: r9495 breaks GFL
//	hr = m_pPlugin.CoCreateInstance( m_pCLSID, NULL, CLSCTX_LOCAL_SERVER );
//	if ( FAILED( hr ) )
	{
		hr = m_pPlugin.CoCreateInstance( m_pCLSID );
		if ( FAILED( hr ) )
		{
			m_pPlugin.Release();
			return FALSE;
		}
	}

	/*hr =*/ m_pPlugin->SetApplication( pApplication );

	m_nCapabilities = 0;
	/*hr =*/ m_pPlugin->QueryCapabilities( &m_nCapabilities );

	ASSERT( ! m_pCommand );
	ASSERT( ! m_pExecute );
	ASSERT( ! m_pLibrary );
	ASSERT( ! m_pChat );

	/*hr =*/ m_pPlugin->QueryInterface( IID_ICommandPlugin, (void**)&m_pCommand );
	/*hr =*/ m_pPlugin->QueryInterface( IID_IExecutePlugin, (void**)&m_pExecute );
	/*hr =*/ m_pPlugin->QueryInterface( IID_ILibraryPlugin, (void**)&m_pLibrary );
	/*hr =*/ m_pPlugin->QueryInterface( IID_IChatPlugin, (void**)&m_pChat );

	return TRUE;
}

void CPlugin::Stop()
{
	__try
	{
		m_pLibrary.Release();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	__try
	{
		m_pChat.Release();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	__try
	{
		m_pExecute.Release();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	__try
	{
		m_pCommand.Release();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	__try
	{
		m_pPlugin.Release();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
}

//////////////////////////////////////////////////////////////////////
// CPlugin CLSID helper

CString CPlugin::GetStringCLSID() const
{
	return Hashes::toGuid( m_pCLSID );
}

//////////////////////////////////////////////////////////////////////
// CPlugin icon helper

HICON CPlugin::LookupIcon() const
{
	CString strName;
	strName.Format( L"CLSID\\%s\\InprocServer32", (LPCTSTR)GetStringCLSID() );

	HKEY hKey;
	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, strName, 0, KEY_QUERY_VALUE, &hKey ) )
		return NULL;

	DWORD dwType = REG_SZ, dwSize = 256 * sizeof( TCHAR );
	LONG lResult = RegQueryValueEx( hKey, L"", NULL, &dwType, (LPBYTE)strName.GetBuffer( 256 ), &dwSize );
	strName.ReleaseBuffer( dwSize / sizeof( TCHAR ) );
	RegCloseKey( hKey );

	if ( lResult != ERROR_SUCCESS ) return NULL;

	HICON hIcon = NULL;
	ExtractIconEx( strName, 0, NULL, &hIcon, 1 );
	return hIcon;
}
