//
// Envy.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2008
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
#include "CoolInterface.h"
#include "BTInfo.h"
#include "BTTrackerRequest.h"
#include "BTClients.h"
#include "DCClients.h"
#include "EDClients.h"
#include "DDEServer.h"
#include "DiscoveryServices.h"
#include "DlgDeleteFile.h"
#include "DownloadGroups.h"
#include "Downloads.h"
#include "Download.h"
#include "FileExecutor.h"
#include "Flags.h"
#include "Emoticons.h"
#include "GProfile.h"
#include "HostCache.h"
#include "IEProtocol.h"
#include "ImageServices.h"
#include "ImageFile.h"	// AfxMsgBox Banners
#include "Images.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "LibraryBuilder.h"
#include "CtrlLibraryFrame.h"
#include "Network.h"
#include "Neighbours.h"
#include "Plugins.h"
#include "EnvyURL.h"
#include "QueryHashMaster.h"
#include "Registry.h"
#include "Revision.h"	//.svn
#include "Scheduler.h"
#include "SchemaCache.h"
#include "Security.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "SQLite.h"
#include "ThumbCache.h"
#include "Transfers.h"
#include "UploadQueues.h"
#include "Uploads.h"
#include "VendorCache.h"
#include "VersionChecker.h"

#include "DlgHelp.h"
#include "DlgSplash.h"
#include "DlgMessage.h"

#include "WndMain.h"
#include "WndMedia.h"
#include "WndPacket.h"
#include "WndSystem.h"
#include "WndLibrary.h"

#if !defined(XPSUPPORT) && (_MSC_VER >= 1800)		// VS2013+ for WinSDK 8.1+
#include <VersionHelpers.h>
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


/////////////////////////////////////////////////////////////////////////////
// Rare crash workarounds

void AFXAPI AfxOleTermOrFreeLibSafe(BOOL bTerm, BOOL bJustRevoke)
{
	__try
	{
		AfxOleTermOrFreeLib( bTerm, bJustRevoke );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAppCommandLineInfo	(Was CEnvyCommandLineInfo)

CAppCommandLineInfo::CAppCommandLineInfo()
	: m_bTray		( FALSE )
	, m_bHelp		( FALSE )
	, m_bWait		( FALSE )
	, m_bNoSplash	( FALSE )
	, m_bNoAlphaWarning ( FALSE )
	, m_nGUIMode	( -1 )
{
}

void CAppCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if ( bFlag )
	{
		if ( _tcsicmp( pszParam, L"tray" ) == 0 )	// lstrcmpi()
		{
			m_bTray = TRUE;
			m_bNoSplash = TRUE;
			return;
		}
		if ( _tcsicmp( pszParam, L"nosplash" ) == 0 )
		{
			m_bNoSplash = TRUE;
			return;
		}
		if ( _tcsicmp( pszParam, L"nowarn" ) == 0 )
		{
			m_bNoAlphaWarning = TRUE;
			return;
		}
		if ( _tcsicmp( pszParam, L"noskin" ) == 0 )
		{
			ClearSkins();
			return;
		}
		if ( _tcsicmp( pszParam, L"basic" ) == 0 )
		{
			m_nGUIMode = GUI_BASIC;
			return;
		}
		if ( _tcsicmp( pszParam, L"tabbed" ) == 0 )
		{
			m_nGUIMode = GUI_TABBED;
			return;
		}
		if ( _tcsicmp( pszParam, L"windowed" ) == 0 )
		{
			m_nGUIMode = GUI_WINDOWED;
			return;
		}
		if ( _tcsicmp( pszParam, L"wait" ) == 0 )
		{
			m_bWait = TRUE;
			return;
		}
		if ( _tcsncicmp( pszParam, L"task", 4 ) == 0 )
		{
			m_sTask = pszParam + 4;
			return;
		}
		if ( _tcsicmp( pszParam, L"help" ) == 0 || *pszParam == L'?' )
		{
			m_bHelp = TRUE;
			return;
		}
	}
	CCommandLineInfo::ParseParam( pszParam, bFlag, bLast );
}


/////////////////////////////////////////////////////////////////////////////
// CEnvyApp

IMPLEMENT_DYNAMIC(CEnvyApp, CWinApp)

BEGIN_MESSAGE_MAP(CEnvyApp, CWinApp)
	//{{AFX_MSG_MAP(CEnvyApp)
	//}}AFX_MSG
END_MESSAGE_MAP()

// {E3481FE3-E062-4E1C-A23A-62A6D13CBFB8}
const GUID CDECL BASED_CODE _tlid =
	{ 0xE3481FE3, 0xE062, 0x4E1C, { 0xA2, 0x3A, 0x62, 0xA6, 0xD1, 0x3C, 0xBF, 0xB8 } };

CEnvyApp theApp;

OSVERSIONINFOEX	Windows = { sizeof( OSVERSIONINFOEX ) };
SYSTEM_INFO		System = {};


/////////////////////////////////////////////////////////////////////////////
// CEnvyApp construction

CEnvyApp::CEnvyApp()
	: m_pMutex					( NULL )
	, m_pSafeWnd				( NULL )
	, m_bBusy					( 0 )
	, m_bClosing				( false )
	, m_bLive					( false )
	, m_bInteractive			( false )
	, m_bIsWinXP				( false )
	, m_bLimitedConnections 	( false )
	, m_bMenuWasVisible			( FALSE )
	, m_nLastInput				( 0ul )
	, m_nWinVer					( 0ul )
	, m_nMouseWheel 			( 3 )
	, m_hHookMouse				( NULL )
	, m_hHookKbd				( NULL )
	, m_pPacketWnd				( NULL )
//	, m_nFontQuality			( DEFAULT_QUALITY )		// Obsolete, use Settings.Fonts.Quality

	, m_hCryptProv				( NULL )

	, m_dlgSplash				( NULL )

//	, m_hTheme					( NULL )
//	, m_pfnSetWindowTheme		( NULL )	// XP+
//	, m_pfnIsThemeActive		( NULL )	// XP+
//	, m_pfnOpenThemeData		( NULL )	// XP+
//	, m_pfnCloseThemeData		( NULL )	// XP+
//	, m_pfnDrawThemeBackground	( NULL )	// XP+

	, m_hShlWapi				( NULL )
	, m_pfnAssocIsDangerous 	( NULL )

	, m_hShell32				( NULL )
//	, m_pfnSHGetFolderPathW 	( NULL )	// 2K+ (XP Only)
	, m_pfnSHGetKnownFolderPath	( NULL )
	, m_pfnSHQueryUserNotificationState	( NULL )
	, m_pfnSHCreateItemFromParsingName  ( NULL )
	, m_pfnSHGetPropertyStoreFromParsingName ( NULL )
	, m_pfnSetCurrentProcessExplicitAppUserModelID ( NULL )
//	, m_pfnSHGetImageList		( NULL )	// XP+

	, m_hUser32					( NULL )
	, m_pfnChangeWindowMessageFilter ( NULL )
	, m_pfnShutdownBlockReasonCreate ( NULL )
	, m_pfnShutdownBlockReasonDestroy ( NULL )
	, m_pfnRegisterApplicationRestart ( NULL )

	, m_hLibGFL					( NULL )
	, m_hGeoIP					( NULL )
	, m_pGeoIP					( NULL )
	, m_pfnGeoIP_delete			( NULL )
	, m_pfnGeoIP_cleanup		( NULL )
	, m_pfnGeoIP_country_code_by_ipnum ( NULL )
	, m_pfnGeoIP_country_name_by_ipnum ( NULL )
{
	// Determine the version of Windows
	//GetVersionEx( (OSVERSIONINFO*)&Windows );		// Obsolete
	GetSystemInfo( (SYSTEM_INFO*)&System );

	ZeroMemory( m_nVersion, sizeof( m_nVersion ) );
	ZeroMemory( m_pBTVersion, sizeof( m_pBTVersion ) );

// BugTrap (www.intellesoft.net)
#ifdef _DEBUG
	BT_InstallSehFilter();
	BT_SetTerminate();
//	BT_SetAppName( CLIENT_NAME );		// Below
//	BT_SetAppVersion( m_sVersionLong );	// Below
	BT_SetFlags( BTF_INTERCEPTSUEF | BTF_SHOWADVANCEDUI | BTF_DESCRIBEERROR | BTF_DETAILEDMODE | BTF_ATTACHREPORT | BTF_EDITMAIL );
	BT_SetSupportURL( L"http://getenvy.com" );
	BT_SetSupportEMail( L"getenvy-reports@lists.sourceforge.net" );
//	BT_SetSupportServer( L"http://bugtrap.getenvy.com/RequestHandler.aspx", 80 );
	BT_AddRegFile( L"Settings.reg", L"HKEY_CURRENT_USER\\" REGISTRY_KEY );
#endif
}

CEnvyApp::~CEnvyApp()
{
	if ( m_pMutex != NULL )
		CloseHandle( m_pMutex );
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyApp initialization

BOOL CEnvyApp::InitInstance()
{
	CWinApp::InitInstance();

	SetRegistryKey( CLIENT_NAME );

	if ( ! ParseCommandLine() )	// Handle -flags, create mutex.
		return FALSE;

	AfxOleInit();				// Initializes OLE support for the application.
	AfxGetThread()->m_lpfnOleTermOrFreeLib = AfxOleTermOrFreeLibSafe;	// Rare crash workaround
	CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL );

	GetVersionNumber();
	Settings.Load();			// Loads settings.			Depends on GetVersionNumber()
	InitResources();			// Loads theApp settings.	Depends on Settings::Load()
	CoolInterface.Load();		// Loads colors and fonts.	Depends on InitResources()

//	m_pFontManager = new CFontManager();
//	AfxEnableControlContainer( m_pFontManager );
	AfxEnableControlContainer();	// Enable support for containment of OLE controls.

	LoadStdProfileSettings();
	EnableShellOpen();
//	RegisterShellFileTypes();
//	Register();					// Re-register Envy Type Library (In Splash)

	AfxOleRegisterTypeLib( AfxGetInstanceHandle(), _tlid );
	COleTemplateServer::RegisterAll();
	COleObjectFactory::UpdateRegistryAll( TRUE );

// Obsolete single check, moved to ParseCommandLine()
//	m_pMutex = CreateMutex( NULL, FALSE, L"Global\\Envy" );
//	if ( m_pMutex == NULL )
//		return FALSE;		// Mutex probably created in another multi-user session
//
//	if ( GetLastError() == ERROR_ALREADY_EXISTS )
//	{
//		CloseHandle( m_pMutex );
//		m_pMutex = NULL;
//
//		// Show first instance instead
//		if ( CWnd* pWnd = CWnd::FindWindow( CLIENT_HWND, NULL ) )	// L"EnvyMainWnd"
//		{
//			pWnd->SendMessage( WM_SYSCOMMAND, SC_RESTORE );
//			pWnd->ShowWindow( SW_SHOWNORMAL );
//			pWnd->BringWindowToTop();
//			pWnd->SetForegroundWindow();
//		}
//		return FALSE;
//	}
//	// else only app instance, continue.

	if ( m_pfnSetCurrentProcessExplicitAppUserModelID )
		m_pfnSetCurrentProcessExplicitAppUserModelID( CLIENT_NAME );

	if ( m_pfnRegisterApplicationRestart )
		m_pfnRegisterApplicationRestart( L"-nowarn", 0 );

	ShowStartupText();


	// *****************
	// NO PUBLIC RELEASE
	// Remove this section for final releases and public betas.

#if defined(_DEBUG) || defined(__REVISION__)		// Show for "pre-release release builds."

	// Unskinned Banner Workaround:
	Images.m_bmBanner.Attach( CImageFile::LoadBitmapFromResource( IDB_BANNER ) );
	Skin.m_nBanner = 48;


	// BETA EXPIRATION.  Remember to re-compile to update the time.

	COleDateTime tCurrent = COleDateTime::GetCurrentTime();
	COleDateTime tCompileTime;
	tCompileTime.ParseDateTime( _T(__DATE__), LOCALE_NOUSEROVERRIDE, 1033 );
#ifdef _DEBUG
	COleDateTimeSpan tTimeOut( 30, 0, 0, 0 );		// Daily debug builds
#else
	COleDateTimeSpan tTimeOut( 45, 0, 0, 0 );		// Private Betas (Non-sourceforge release)
#endif

	if ( tCurrent > tCompileTime + tTimeOut )
		MsgBox( IDS_BETA_EXPIRED, MB_ICONQUESTION|MB_OK, 0, NULL, 30 );


	// ALPHA/BETA WARNING.  Remember to remove this section for public betas.

	if ( ! m_cmdInfo.m_bNoAlphaWarning && m_cmdInfo.m_bShowSplash )
	{
		CString strVersion = L".";
#ifdef __REVISION__
		strVersion = L", r" _T(__REVISION__) L".";
#elif defined(_DEBUG)
		COleDateTime tCompileTime;
		tCompileTime.ParseDateTime( _T(__DATE__), LOCALE_NOUSEROVERRIDE, 1033 );
		strVersion = tCompileTime.Format( L", %Y.%m.%d." );
#endif

		if ( MsgBox(
#ifdef _DEBUG
			L"\nWARNING: This is a DEBUG TEST version of Envy p2p"
#else
			L"\nWARNING: This is a PRIVATE TEST version of Envy p2p"
#endif
			+ strVersion +
			L"\n\nNOT FOR GENERAL USE, it is intended for pre-release testing in\n"
			L"controlled environments.  It may stop running or display debug info.\n\n"
			L"If you wish to simply use this software, download the current\n"
			L"stable release from GetEnvy.com.  If you continue past this point,\n"
			L"you could possibly experience system instability or lose files.\n"
			L"Please be aware of recent development before using.\n\n"
			L"Do you wish to continue?", MB_ICONEXCLAMATION|MB_YESNO|MB_SETFOREGROUND, 0, NULL, 30 ) == IDNO )
				return FALSE;
	}

#endif	// _DEBUG/__REVISION__

	// END NO PUBLIC RELEASE
	// *********************


	// Go Live

	m_bInteractive = true;

	// Test and (re)register plugins first

	CComPtr< IUnknown > pTest( Plugins.GetPlugin( L"ImageService", L".png" ) );
	if ( ! pTest || Settings.Live.FirstRun )
	{
		pTest.Release();

		Plugins.Register( Settings.General.Path );
		Plugins.Register( Settings.General.Path + L"\\Plugins" );

		pTest.Attach( Plugins.GetPlugin( L"ImageService", L".png" ) );
		if ( ! pTest )
		{
			CString strPath = m_strBinaryPath.Left( m_strBinaryPath.ReverseFind( L'\\' ) );
			Plugins.Register( strPath );
			Plugins.Register( strPath + L"\\Plugins" );
		}
	}

	// Show Startup Splash Screen

	const int nSplashSteps = ( m_cmdInfo.m_bNoSplash || ! m_cmdInfo.m_bShowSplash ) ? 0 : 21;

	SplashStep( L"Up", nSplashSteps, false );
		if ( m_cmdInfo.m_nGUIMode != -1 )
			Settings.General.GUIMode = m_cmdInfo.m_nGUIMode;
		if ( Settings.General.GUIMode != GUI_WINDOWED && Settings.General.GUIMode != GUI_TABBED && Settings.General.GUIMode != GUI_BASIC )
			Settings.General.GUIMode = GUI_TABBED;

		DDEServer.Create();
		IEProtocol.Create();

		PurgeDeletes();

	SplashStep( L"Network Winsock" );
		if ( ! Network.Init() )
			return FALSE;

		// Obsolete for reference & deletion
		//WSADATA wsaData;
		//for ( int i = 1 ; i <= 2 ; i++ )
		//{
		//	if ( WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) ) return FALSE;
		//	if ( wsaData.wVersion == MAKEWORD( 1, 1 ) ) break;
		//	if ( i == 2 ) return FALSE;
		//	WSACleanup();
		//}

	SplashStep( L"Register" );
		Register();		// CEnvyURL::Register( TRUE )
	SplashStep( L"Profile" );
		MyProfile.Load();
	SplashStep( L"Vendor Data" );
		VendorCache.Load();
	SplashStep( L"Security Services" );
		Security.Load();
		AdultFilter.Load();
		MessageFilter.Load();
	SplashStep( L"Discovery Services" );
		DiscoveryServices.Load();
	SplashStep( L"Host Cache" );
		HostCache.Load();
	SplashStep( L"Query Manager" );
		QueryHashMaster.Create();
	SplashStep( L"Scheduler" );
		Scheduler.Load();
	SplashStep( L"Shell Icons" );
		ShellIcons.Clear();
		if ( ! Emoticons.Load() )
			Message( MSG_ERROR, L"Failed to load Emoticons." );
	//	if ( ! Flags.Load() )	// Moved to MainWnd OnSkinChanged
	//		Message( MSG_ERROR, L"Failed to load Flags." );
	SplashStep( L"Metadata Schemas" );
		if ( SchemaCache.Load() < 30 &&		// Expected number of .xsd files in Schemas folder
			 MsgBox( IDS_SCHEMA_LOAD_ERROR, MB_ICONWARNING|MB_OKCANCEL ) != IDOK )
		{
			SplashAbort();
			return FALSE;
		}
		if ( ! Settings.MediaPlayer.FileTypes.size() )
		{
			CString strTypeFilter;
			static const LPCTSTR szTypes[] =
			{
				CSchema::uriAudio,
				CSchema::uriVideo,
				NULL
			};
			for ( int i = 0 ; szTypes[ i ] ; ++ i )
			{
				if ( CSchemaPtr pSchema = SchemaCache.Get( szTypes[ i ] ) )
					strTypeFilter += pSchema->GetFilterSet();
			}
			CSettings::LoadSet( &Settings.MediaPlayer.FileTypes, strTypeFilter );
		}
		if ( ! Settings.Library.SafeExecute.size() )
		{
			CString strTypeFilter;
			static const LPCTSTR szTypes[] =
			{
				CSchema::uriArchive,
				CSchema::uriAudio,
				CSchema::uriVideo,
				CSchema::uriBook,
				CSchema::uriImage,
				CSchema::uriCollection,
				CSchema::uriBitTorrent,
				NULL
			};
			for ( int i = 0 ; szTypes[ i ] ; ++ i )
			{
				if ( CSchemaPtr pSchema = SchemaCache.Get( szTypes[ i ] ) )
					strTypeFilter += pSchema->GetFilterSet();
			}
			CSettings::LoadSet( &Settings.Library.SafeExecute, strTypeFilter );
		}

	//CWaitCursor pCursor;

	SplashStep( L"Custom Folders" );
		LibraryFolders.Maintain();		// Update desktop.ini's (~2s)
	SplashStep( L"Thumb Database" );
		CThumbCache::InitDatabase();	// Several seconds if large (~5s)
	SplashStep( L"Library" );
		Library.Load();					// Lengthy if very large (~20s)
	SplashStep( L"Downloads" );
		Downloads.PreLoad();			// Very lengthy if many files (~1min)
	SplashStep( L"Downloads Cleanup" );
		Downloads.PurgeFiles();
		Sleep( 50 );					// Allow some splash text visibility
	SplashStep( L"Download Manager" );
		Downloads.Load();
		Sleep( 50 );					// Allow some splash text visibility
	SplashStep( L"Upload Manager" );
		UploadQueues.Load();
		Sleep( 50 );					// Allow some splash text visibility

	// Obsolete for reference & deletion
	//if ( Settings.Connection.EnableFirewallException )
	//{
	//	SplashStep( L"Windows Firewall Setup" );
	//	CFirewall firewall;
	//	if ( firewall.Init() && firewall.AreExceptionsAllowed() )
	//	{
	//		// Add to firewall exception list if necessary
	//		// and enable UPnP Framework if disabled
	//		firewall.SetupService( NET_FW_SERVICE_UPNP );
	//		firewall.SetupProgram( m_strBinaryPath, theApp.m_pszAppName );
	//	}
	//}
	//
	//if ( Settings.Connection.EnableUPnP )
	//{
	//	SplashStep( L"Plug'n'Play Network Access, Please Wait" );
	//	// First run will do UPnP discovery in the QuickStart Wizard
	//	if ( ! Settings.Live.FirstRun )
	//	{
	//		m_pUPnPFinder.Attach( new CUPnPFinder );
	//		if ( m_pUPnPFinder->AreServicesHealthy() )
	//			m_pUPnPFinder->StartDiscovery();	// Lengthy 30s
	//	}
	//}

	//pCursor.Restore();

	SplashStep( L"GUI" );
		if ( m_cmdInfo.m_bTray )
			WriteProfileInt( L"Windows", L"CMainWnd.ShowCmd", 0 );
		TRY
		{
			m_pMainWnd = new CMainWnd();
			// Bypass CMDIFrameWnd::LoadFrame
			if ( m_pMainWnd && ((CMainWnd*)m_pMainWnd)->CFrameWnd::LoadFrame( IDR_MAINFRAME, WS_OVERLAPPEDWINDOW ) )
				m_pSafeWnd = m_pMainWnd;
		}
		CATCH_ALL(e)
		{
			// Out of resources
		}
		END_CATCH_ALL
		if ( ! m_pSafeWnd )
		{
			SplashAbort();
			AfxMessageBox( L"Failed to initialize GUI.", MB_ICONHAND | MB_OK );
			return FALSE;
		}

		CoolMenu.EnableHook();
		if ( m_cmdInfo.m_bTray )
		{
			((CMainWnd*)m_pMainWnd)->CloseToTray();
		}
		else
		{
			m_pMainWnd->ShowWindow( SW_SHOW );
			if ( m_dlgSplash )
				m_dlgSplash->Topmost();
			m_pMainWnd->UpdateWindow();
		}
		// From this point translations would be available, and LoadString returns correct strings
		Sleep( 60 );	// Allow some splash text visibility

	SplashStep( L"Upgrade Manager" );
		VersionChecker.Start();
		Sleep( 120 );

	SplashStep();

	m_bLive = true;

//	afxMemDF = allocMemDF | delayFreeMemDF | checkAlwaysMemDF;

	ProcessShellCommand( m_cmdInfo );

	Settings.Save();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyApp termination

int CEnvyApp::ExitInstance()
{
	if ( m_bInteractive )
	{
		// Continue Shutdown Splash Screen (from WndMain)

		//CWaitCursor pCursor;
		m_bInteractive = false;

		//const int nSplashSteps = 7 + ( m_bLive ? 3 : 0 );

		SplashStep( L"Disconnecting" );
		VersionChecker.Stop();
		DiscoveryServices.Stop();
		Network.Disconnect();

		SplashStep( L"Stopping Library Tasks" );
		LibraryBuilder.CloseThread();
		Library.CloseThread();

		SplashStep( L"Stopping Security Tasks" );
		ListLoader.CloseThread();

		SplashStep( L"Stopping Transfers" );
		Transfers.StopThread();
		Downloads.CloseTransfers();

		SplashStep( L"Clearing Clients" );
		Uploads.Clear( FALSE );
		BTClients.Clear();
		DCClients.Clear();
		EDClients.Clear();

		if ( m_bLive )
		{
			SplashStep( L"Saving Services" );
			Settings.Save( TRUE );
			Security.Save();
			HostCache.Save();
			UploadQueues.Save();
			DiscoveryServices.Save();
			DownloadGroups.Save();

			SplashStep( L"Saving Downloads" );
			Downloads.Save();

			SplashStep( L"Saving Library" );
			Library.Save();
		}

		// Obsolete for reference & deletion
		//if ( m_pUPnPFinder )
		//{
		//	SplashStep( L"Closing Plug'n'Play Network Access" );
		//	m_pUPnPFinder->StopAsyncFind();
		//	if ( Settings.Connection.DeleteUPnPPorts )
		//		m_pUPnPFinder->DeletePorts();
		//	m_pUPnPFinder.Free();
		//}
		//
		//if ( Settings.Connection.DeleteFirewallException )
		//{
		//	SplashStep( L"Closing Windows Firewall Access" );
		//	CFirewall firewall;
		//	if ( firewall.Init() )
		//		firewall.SetupProgram( m_strBinaryPath, theApp.m_pszAppName, TRUE );
		//}

		SplashStep( L"Closing Network" );
		Network.Clear();	// UPnP Delay

		SplashStep( L"Finalizing" );
		TrackerRequests.Clear();
		Downloads.Clear( true );
		Library.Clear();
		HostCache.Clear();
		DiscoveryServices.Clear();
		CoolMenu.Clear();
		Skin.Clear();

		DDEServer.Close();
		IEProtocol.Close();

		SchemaCache.Clear();
		Plugins.Clear();

		FreeCountry();		// Release GeoIP

		Sleep( 100 );
		SplashStep();
	}

//	if ( m_hTheme != NULL )
//		FreeLibrary( m_hTheme );	// XP+

	if ( m_hShlWapi != NULL )
		FreeLibrary( m_hShlWapi );

	if ( m_hShell32 != NULL )
		FreeLibrary( m_hShell32 );

	if ( m_hUser32 != NULL )
		FreeLibrary( m_hUser32 );

	if ( m_hLibGFL != NULL )
		FreeLibrary( m_hLibGFL );

	//delete m_pFontManager;	// Obsolete

	UnhookWindowsHookEx( m_hHookKbd );
	UnhookWindowsHookEx( m_hHookMouse );

	if ( m_hCryptProv )
		CryptReleaseContext( m_hCryptProv, 0 );

	// Moved to destructor
	//if ( m_pMutex != NULL )
	//	CloseHandle( m_pMutex );

	{
		CQuickLock pLock( m_csMessage );
		while ( ! m_oMessages.IsEmpty() )
			delete m_oMessages.RemoveHead();
	}

	return CWinApp::ExitInstance();
}

void CEnvyApp::SplashStep(LPCTSTR pszMessage, int nMax, bool bClosing)
{
	if ( ! pszMessage )
	{
		if ( m_dlgSplash )
		{
			m_dlgSplash->Hide();
			m_dlgSplash = NULL;
		}
	}
	else if ( ! m_dlgSplash && nMax )
	{
		m_dlgSplash = new CSplashDlg( nMax, bClosing );
		m_dlgSplash->Step( pszMessage );
	}
	else if ( m_dlgSplash /*&& ! nMax*/ )	// Reset m_dlgSplash->m_nPos ?
	{
		m_dlgSplash->Step( pszMessage );
	}

	TRACE( L"Step: %s\n", pszMessage ? pszMessage : L"Done" );
}

void CEnvyApp::SplashUpdate(LPCTSTR pszMessage)
{
	if ( m_dlgSplash )
		m_dlgSplash->Update( pszMessage );
}

void CEnvyApp::SplashAbort()
{
	if ( m_dlgSplash )
	{
		m_dlgSplash->Hide( TRUE );
		m_dlgSplash = NULL;
	}
}

BOOL CEnvyApp::KeepAlive()
{
	// Call in potentially lengthy loop operations to avoid Windows "Program Not Responding"
	static DWORD tKeepAlive;
	const DWORD tNow = GetTickCount();
	if ( tNow < tKeepAlive + 5000 )
		return FALSE;		// ~5 seconds minimum
	tKeepAlive = tNow;

	MSG msg = { 0 };
	while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return TRUE;
}

BOOL CEnvyApp::ParseCommandLine()
{
	CWinApp::ParseCommandLine( m_cmdInfo );

	if ( m_cmdInfo.m_bHelp )
	{
		// Unskinned Banner/Font Workaround:
		Images.m_bmBanner.Attach( CImageFile::LoadBitmapFromResource( IDB_BANNER ) );
		Skin.m_nBanner = 50;

#ifdef WIN64
		const BOOL bIsXP = FALSE;
#else // Win32 (Obsolete)
		OSVERSIONINFOEX pVersion = { sizeof( OSVERSIONINFOEX ) };
		GetVersionEx( (OSVERSIONINFO*)&pVersion );	// Deprecated
		const BOOL bIsXP = pVersion.dwMajorVersion < 6;
#endif // WIN64

		CoolInterface.m_fntNormal.CreateFont( -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, bIsXP ? L"Tahoma" : L"Segoe UI" );

		MsgBox( //IDS_COMMANDLINE,	// No translation available yet
			L"\nEnvy command-line options:\n\n"
			L" -help   -? \tDisplay this help screen\n"
			L" -tray\t\tStart quietly in system tray\n"
			L" -nosplash\tDisable startup splash screen\n"
			L" -nowarn\t\tSkip debug version warning dialog\n"
			L" -noskin\t\tDisable all skins and languages\n"
			L" -wait\t\tWait for any prior instance to finish\n"
			L" -basic\t\tStart application in Basic mode\n"
			L" -tabbed\t\tStart application in Tabbed mode\n"
			L" -windowed\tStart application in Windowed mode\n"
			L" -regserver\tRegister application components\n"
			L" -unregserver\tUn-register application components ----------\n",	// Layout workaround
			MB_ICONINFORMATION | MB_OK );

		return FALSE;
	}

	AfxSetPerUserRegistration( m_cmdInfo.m_bRegisterPerUser || ! IsRunAsAdmin() );

	if ( m_cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister ||
		 m_cmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister )
	{
		m_cmdInfo.m_bRunEmbedded = TRUE;	// Suppress dialog

		ProcessShellCommand( m_cmdInfo );

		return FALSE;
	}

	HWND hWndPrior = NULL;
	for ( ;; )	// Loop if "wait"
	{
		m_pMutex = CreateMutex( NULL, FALSE, L"Global\\Envy" );	// CLIENT_NAME

		if ( m_pMutex == NULL )
		{
			// Mutex likely created in another multi-user session
			Images.m_bmBanner.Attach( CImageFile::LoadBitmapFromResource( IDB_BANNER ) );		// Unskinned Banner Workaround
			Skin.m_nBanner = 50;
			AfxMessageBox(
				L"Envy appears to be open in another user session."
				L"\n\nPlease close it before continuing.",
				MB_ICONEXCLAMATION | MB_OK );
			return FALSE;
		}

		if ( GetLastError() == ERROR_ALREADY_EXISTS )
		{
			CloseHandle( m_pMutex );
			m_pMutex = NULL;
			hWndPrior = FindWindow( CLIENT_HWND, NULL );	// "EnvyMainWnd"
		}
		// else we are first instance

		if ( ! m_cmdInfo.m_bWait || ! hWndPrior )
			break;

		Sleep( 500 );	// Wait for first instance exit, and try again
	}

	if ( hWndPrior )
	{
		// Windows scheduling not implemented!
		//if ( ! m_cmdInfo.m_sTask.IsEmpty() )
		//{
		//	// Pass scheduler task to existing instance
		//	//CScheduler::Execute( hWndPrior, m_cmdInfo.m_sTask );
		//
		//	return FALSE;	// Don't start second instance or show first
		//}

		if ( m_cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen )
		{
			// Pass command line to first instance
			m_cmdInfo.m_strFileName.Trim( L" \t\r\n\"" );
			COPYDATASTRUCT cd =
			{
				COPYDATA_OPEN,
				m_cmdInfo.m_strFileName.GetLength() * sizeof( TCHAR ),
				(PVOID)(LPCTSTR)m_cmdInfo.m_strFileName
			};
			DWORD_PTR dwResult;
			SendMessageTimeout( hWndPrior, WM_COPYDATA, NULL, (WPARAM)&cd, SMTO_NORMAL, 250, &dwResult );
		}

		// Popup first instance
		DWORD_PTR dwResult;
		SendMessageTimeout( hWndPrior, WM_COMMAND, ID_TRAY_OPEN, 0, SMTO_NORMAL, 250, &dwResult );
		ShowWindow( hWndPrior, SW_SHOWNA );
		BringWindowToTop( hWndPrior );
		SetForegroundWindow( hWndPrior );

		return FALSE;	// Don't start second instance
	}

	if ( m_cmdInfo.m_bWait )
		Sleep( 1000 );	// Wait for other instance complete exit

	return TRUE;		// Continue Envy execution
}

BOOL CEnvyApp::Register()
{
	COleObjectFactory::UpdateRegistryAll();
	AfxOleRegisterTypeLib( AfxGetInstanceHandle(), LIBID_Envy );

	CEnvyURL::Register( TRUE, TRUE );

	// See http://msdn.microsoft.com/en-us/gg465010#_Toc243450447 for TaskBar
	if ( theApp.m_nWinVer >= WIN_7 )
	{
#if defined(_MSC_VER) && (_MSC_VER >= 1600) && (NTDDI_VERSION >= NTDDI_WIN7)
		// For VS2010+:
		CJumpList oTasks = new JumpList();
		oTasks.ClearAllDestinations();
		oTasks.AddKnownCategory( KDC_RECENT );
		oTasks.AddTask( L"envy:command:search", L"", LoadString( IDS_SEARCH_TASK ) + L"...", theApp.m_strBinaryPath, - IDR_SEARCHFRAME );
		oTasks.AddTask( L"envy:command:download", L"", LoadString( IDS_DOWNLOAD_TASK ) + L"...", theApp.m_strBinaryPath, - IDR_DOWNLOADSFRAME );
//#else
//		// For VS2008:
//		CComPtr< ICustomDestinationList > pList;
//		if ( SUCCEEDED( pList.CoCreateInstance( CLSID_DestinationList ) ) )
//		{
//			VERIFY( SUCCEEDED( pList->SetAppID( CLIENT_NAME ) ) );
//			UINT nMinSlots;
//			CComPtr< IObjectArray > pRemoved;
//			VERIFY( SUCCEEDED( pList->BeginList( &nMinSlots, IID_IObjectArray, (LPVOID*)&pRemoved ) ) );
//			VERIFY( SUCCEEDED( pList->AppendKnownCategory( KDC_RECENT ) ) );
//
//			CComPtr< IObjectCollection > pTasks;
//			if ( SUCCEEDED( pTasks.CoCreateInstance( CLSID_EnumerableObjectCollection ) ) )
//			{
//				CComPtr< IShellLink > pSearch = CreateShellLink( L"envy:command:search", L"",
//					LoadString( IDS_SEARCH_TASK ) + L"...", theApp.m_strBinaryPath, - IDR_SEARCHFRAME, L"" );
//				ASSERT( pSearch );
//				if ( pSearch )
//					VERIFY( SUCCEEDED( pTasks->AddObject( pSearch ) ) );
//
//				CComPtr< IShellLink > pDownload = CreateShellLink( L"envy:command:download", L"",
//					LoadString( IDS_DOWNLOAD_TASK ) + L"...", theApp.m_strBinaryPath, - IDR_DOWNLOADSFRAME, L"" );
//				ASSERT( pDownload );
//				if ( pDownload )
//					VERIFY( SUCCEEDED( pTasks->AddObject( pDownload ) ) );
//
//				VERIFY( SUCCEEDED( pList->AddUserTasks( pTasks ) ) );
//			}
//
//			VERIFY( SUCCEEDED( pList->CommitList() ) );
//		}
#endif
	}

	return CWinApp::Register();
}

BOOL CEnvyApp::Unregister()
{
	CEnvyURL::Register( FALSE, TRUE );

	AfxOleUnregisterTypeLib( LIBID_Envy );
	AfxOleUnregisterTypeLib( LIBID_Envy );
	AfxOleUnregisterTypeLib( LIBID_Envy );
	COleObjectFactory::UpdateRegistryAll( FALSE );
	COleObjectFactory::UpdateRegistryAll( FALSE );
	COleObjectFactory::UpdateRegistryAll( FALSE );

	return TRUE;	// Don't call CWinApp::Unregister(), it removes Envy settings
}

void CEnvyApp::WinHelp(DWORD_PTR /*dwData*/, UINT /*nCmd*/)
{
	// Suppress F1
}

void CEnvyApp::AddToRecentFileList(LPCTSTR lpszPathName)
{
	SHAddToRecentDocs( SHARD_PATHW, lpszPathName );

// For VS2008, No need VS2010+ (Confirm?)
// SHARDAPPIDINFO Requires #define NTDDI_VERSION NTDDI_WIN7 in StdAfx.h (May be NTDDI_LONGHORN fallback or NTDDI_WINXPSP2 test)
// For applicability here, need to detect VS2008 with WinSDK 7.0+ added.
#if defined(VS2008) && (NTDDI_VERSION > NTDDI_LONGHORN)
	if ( theApp.m_nWinVer >= WIN_7 && m_pfnSHCreateItemFromParsingName )
	{
		CComPtr< IShellItem > pItem;
		if ( SUCCEEDED( m_pfnSHCreateItemFromParsingName( lpszPathName, NULL, IID_IShellItem, (LPVOID*)&pItem ) ) )
		{
			SHARDAPPIDINFO info = { pItem, CLIENT_NAME };
			SHAddToRecentDocs( SHARD_APPIDINFO, &info );
		}
	}
#endif	// WinSDK 7.0+
}

BOOL CEnvyApp::DisplayFile(LPCTSTR lpszFileName)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return FALSE;

	if ( CLibraryFile* pLibFile = LibraryMaps.LookupFileByPath( lpszFileName ) )
	{
		if ( CLibraryWnd* pLibrary = CLibraryWnd::GetLibraryWindow() )
		{
			pLibrary->Display( pLibFile );
		}
		return TRUE;
	}

	return FALSE;
}

CDocument* CEnvyApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	if ( lpszFileName )
		Open( lpszFileName );
	return NULL;
}

BOOL CEnvyApp::Open(LPCTSTR lpszFileName, BOOL bTest /*FALSE*/)		// Note: Not BOOL bDoIt
{
	CString strExt( PathFindExtension( lpszFileName ) );
	if ( strExt.IsEmpty() )
		return bTest ? PathIsDirectory( lpszFileName ) : OpenPath( lpszFileName );
	strExt = strExt.MakeLower();

	SwitchMap( Ext )
	{
		Ext[ L".torrent" ]	= 't';
		Ext[ L".co" ]		= 'c';
		Ext[ L".collection" ] = 'c';
		Ext[ L".emulecollection" ] = 'c';
		Ext[ L".pd" ]		= 'p';
		Ext[ L".sd" ]		= 'p';
		Ext[ L".bz2" ]		= 'b';
		Ext[ L".met" ]		= 'i';
		Ext[ L".dat" ]		= 'i';
		Ext[ L".url" ]		= 'u';
		Ext[ L".lnk" ]		= 'l';
	//	Ext[ L".metalink" ]	= 'm';
	//	Ext[ L".meta4" ]	= 'm';
	//	Ext[ L".magma" ]	= 'a';
	}

	switch ( Ext[ strExt ] )
	{
	case 't':	// .torrent
		return bTest || OpenTorrent( lpszFileName );
	case 'c':	// .co .collection .emulecollection
		return bTest || OpenCollection( lpszFileName );
	case 'p':	// .pd .sd
		return bTest || OpenDownload( lpszFileName );
	case 'i':	// .met .dat
		return bTest || OpenImport( lpszFileName );
//	case 'm':	// ToDo: .metalink .meta4 .magma (0.2)
//		return bTest || OpenMetalink( lpszFileName );
	case 'u':	// .url
		return bTest || OpenInternetShortcut( lpszFileName );
	case 'l':	// .lnk
		return bTest || OpenShellShortcut( lpszFileName );
	case 'b':	// .xml.bz2 (DC++)
		if ( EndsWith( lpszFileName, _P( L".xml.bz2" ) ) )	// Was ( _tcsicmp( lpszFileName + ( _tcslen( lpszFileName ) - 8 ), L".xml.bz2" ) == 0 )
		{
			if ( bTest ) return TRUE;
			if ( _tcsicmp( PathFindFileName( lpszFileName ), L"hublist.xml.bz2" ) == 0 )
				return OpenImport( lpszFileName );
			return OpenCollection( lpszFileName );
		}
		break;
	}

	// Legacy method for reference:
	//if ( nLength > 8  && _tcsicmp( lpszFileName + nLength - 8,  L".torrent" ) == 0 )
	//	return OpenTorrent( lpszFileName );
	//if ( nLength > 3  && _tcsicmp( lpszFileName + nLength - 3,  L".co" ) == 0 )
	//	return OpenCollection( lpszFileName );
	//if ( nLength > 11 && _tcsicmp( lpszFileName + nLength - 11, L".collection" ) == 0 )
	//	return OpenCollection( lpszFileName );
	//if ( nLength > 16 && _tcsicmp( lpszFileName + nLength - 16, L".emulecollection" ) == 0 )
	//	return OpenCollection( lpszFileName );
	//if ( nLength > 14 && _tcsicmp( lpszFileName + nLength - 15, L"hublist.xml.bz2" ) == 0 )
	//	return OpenImport( lpszFileName );
	//if ( nLength > 8  && _tcsicmp( lpszFileName + nLength - 8,  L".xml.bz2" ) == 0 )
	//	return OpenCollection( lpszFileName );
	//if (/*nLength > 4 &&*/ _tcsicmp( lpszFileName + nLength - 4,  L".met" ) == 0 )
	//	return OpenImport( lpszFileName );
	//if (/*nLength > 4 &&*/ _tcsicmp( lpszFileName + nLength - 4,  L".dat" ) == 0 )
	//	return OpenImport( lpszFileName );
	//if (/*nLength > 4 &&*/ _tcsicmp( lpszFileName + nLength - 4,  L".url" ) == 0 )
	//	return OpenInternetShortcut( lpszFileName );
	//if (/*nLength > 4 &&*/ _tcsicmp( lpszFileName + nLength - 4,  L".lnk" ) == 0 )
	//	return OpenShellShortcut( lpszFileName );

	return OpenURL( lpszFileName, bTest );
}

BOOL CEnvyApp::OpenImport(LPCTSTR lpszFileName)
{
	//return HostCache.Import( lpszFileName );	// Obsolete

	AddToRecentFileList( lpszFileName );

	const size_t nLen = _tcslen( lpszFileName ) + 1;
	CAutoVectorPtr< TCHAR > pszPath( new TCHAR[ nLen ] );
	if ( pszPath )
	{
		_tcscpy_s( pszPath, nLen, lpszFileName );
		if ( PostMainWndMessage( WM_IMPORT, (WPARAM)pszPath.Detach() ) )
			return TRUE;
	}

	return FALSE;
}

BOOL CEnvyApp::OpenShellShortcut(LPCTSTR lpszFileName)
{
	CString strPath( ResolveShortcut( lpszFileName ) );
	return ! strPath.IsEmpty() && Open( strPath );
}

BOOL CEnvyApp::OpenInternetShortcut(LPCTSTR lpszFileName)
{
	CString strURL;
	BOOL bResult = ( GetPrivateProfileString( L"InternetShortcut", L"URL",
		L"", strURL.GetBuffer( MAX_PATH ), MAX_PATH, lpszFileName ) > 3 );
	strURL.ReleaseBuffer();
	if ( ! bResult || strURL.IsEmpty() )
		return FALSE;

	AddToRecentFileList( lpszFileName );

	return OpenURL( strURL );
}

BOOL CEnvyApp::OpenTorrent(LPCTSTR lpszFileName)
{
	// Test torrent
	//augment::auto_ptr< CBTInfo > pTorrent( new CBTInfo() );
	//if ( ! pTorrent.get() ) return FALSE;
	//if ( ! pTorrent->LoadTorrentFile( lpszFileName ) ) return FALSE;

	if ( PathFileExists( lpszFileName ) )	// Skip temp file?
		AddToRecentFileList( lpszFileName );

	// Open torrent
	const size_t nLen = _tcslen( lpszFileName ) + 1;
	CAutoVectorPtr< TCHAR > pszPath( new TCHAR[ nLen ] );
	if ( pszPath )
	{
		_tcscpy_s( pszPath, nLen, lpszFileName );
		if ( PostMainWndMessage( WM_TORRENT, (WPARAM)pszPath.Detach() ) )
			return TRUE;
	}

	return FALSE;
}

BOOL CEnvyApp::OpenCollection(LPCTSTR lpszFileName)
{
	AddToRecentFileList( lpszFileName );

	const size_t nLen = _tcslen( lpszFileName ) + 1;
	CAutoVectorPtr< TCHAR > pszPath( new TCHAR[ nLen ] );
	if ( pszPath )
	{
		_tcscpy_s( pszPath, nLen, lpszFileName );
		if ( PostMainWndMessage( WM_COLLECTION, (WPARAM)pszPath.Detach() ) )
			return TRUE;
	}

	return FALSE;
}

//BOOL CEnvyApp::OpenMetalink(LPCTSTR lpszFileName)
//{
//	AddToRecentFileList( lpszFileName );
//
//	const size_t nLen = _tcslen( lpszFileName ) + 1;
//	CAutoVectorPtr< TCHAR > pszPath( new TCHAR[ nLen ] );
//	if ( pszPath )
//	{
//		_tcscpy_s( pszPath, nLen, lpszFileName );
//		if ( PostMainWndMessage( WM_METALINK, (WPARAM)pszPath.Detach() ) )
//			return TRUE;
//	}
//
//	return FALSE;
//}

BOOL CEnvyApp::OpenURL(LPCTSTR lpszFileName, BOOL bSilent /*0*/)
{
	if ( ! bSilent )
		theApp.Message( MSG_NOTICE, IDS_URL_RECEIVED, lpszFileName );

	CAutoPtr< CEnvyURL > pURL( new CEnvyURL() );
	if ( pURL && pURL->Parse( lpszFileName ) )
	{
		// "command:download" or "command:search" or ToDo: Others?
		if ( pURL->m_nAction == CEnvyURL::uriCommand )
		{
			if ( pURL->m_sName == L"download" )
				PostMainWndMessage( WM_COMMAND, ID_TOOLS_DOWNLOAD );
			else if ( pURL->m_sName == L"search" )
				PostMainWndMessage( WM_COMMAND, ID_NETWORK_SEARCH );
			else
				return FALSE;
		}
		else
		{
			PostMainWndMessage( WM_URL, (WPARAM)pURL.Detach() );
		}
		return TRUE;
	}

	if ( ! bSilent )
		theApp.Message( MSG_NOTICE, IDS_URL_PARSE_ERROR );

	return FALSE;
}

BOOL CEnvyApp::OpenPath(LPCTSTR lpszFileName)
{
	DWORD nAttributes = GetFileAttributes( lpszFileName );
	if ( ! ( nAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		return FALSE;

	if ( ! LibraryFolders.IsShareable( (CString)lpszFileName ) )
		return FALSE;

	//PostMainWndMessage( WM_COMMAND, ID_VIEW_LIBRARY );

	// Show existing folder in Library
	if ( LibraryFolders.IsFolderShared( (CString)lpszFileName ) )
	{
		if ( CLibraryFolder* pFolder = LibraryFolders.GetFolder( lpszFileName ) )
		{
			CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
			if ( CLibraryWnd* pLibraryWnd = (CLibraryWnd*)pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd) ) )
			{
				CLibraryFrame* pFrame = &pLibraryWnd->m_wndFrame;
				pFrame->Display( pFolder );
			}
		}

		return TRUE;
	}

	CString strMessage;
	strMessage.Format( LoadString( IDS_LIBRARY_ADD_FOLDER ), lpszFileName );
	if ( MsgBox( (LPCTSTR)strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES )
		return FALSE;

	if ( LibraryFolders.IsSubFolderShared( (CString)lpszFileName ) )
	{
		strMessage.Format( LoadString( IDS_LIBRARY_SUBFOLDER_IN_LIBRARY ), lpszFileName );
		if ( MsgBox( (LPCTSTR)strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES )
			return FALSE;
	}

	// Add new folder to Library
	if ( CLibraryFolder* pFolder = LibraryFolders.AddFolder( lpszFileName ) )
	{
		CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
		if ( CLibraryWnd* pLibraryWnd = (CLibraryWnd*)pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd) ) )
		{
			CLibraryFrame* pFrame = &pLibraryWnd->m_wndFrame;
			pFrame->Display( pFolder );
		}

		BOOL bShare = MsgBox( IDS_LIBRARY_DOWNLOADS_SHARE, MB_ICONQUESTION|MB_YESNO ) == IDYES;

		CQuickLock oLock( Library.m_pSection );
		if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
			pFolder->SetShared( bShare ? TRI_TRUE : TRI_FALSE );
		Library.Update();
	}

	PostMainWndMessage( WM_COMMAND, ID_LIBRARY_FOLDERS );

	return TRUE;
}

BOOL CEnvyApp::OpenDownload(LPCTSTR lpszFileName)
{
	CString strFileName = lpszFileName;
	GetLongPathName( lpszFileName, strFileName.GetBuffer( MAX_PATH * 2 ), MAX_PATH * 2 );
	strFileName.ReleaseBuffer();

	const CString strPDName = PathFindFileName( strFileName );

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	// Check for already loaded file
	if ( Downloads.FindByPDName( strPDName ) == NULL )
	{
		// Load a new one
		if ( CDownload* pDownload = Downloads.Load( lpszFileName ) )
		{
			// Save download to Incomplete folder
			pDownload->m_sPath = Settings.Downloads.IncompletePath + L"\\" + strPDName;
			if ( pDownload->Save( TRUE ) )
			{
				// Rename old file
				::MoveFileEx( SafePath( strFileName ), SafePath( strFileName + L".sav" ), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH );

				theApp.Message( MSG_NOTICE, L"Download file \"%s\" has been successfully loaded and saved as \"%s\".", (LPCTSTR)strFileName, (LPCTSTR)pDownload->m_sPath );
			}
			else
				theApp.Message( MSG_ERROR, L"Failed to save download file \"%s\" as \"%s\".", (LPCTSTR)strFileName, (LPCTSTR)pDownload->m_sPath );
		}
		else
			theApp.Message( MSG_ERROR, L"Failed to load download file \"%s\".", (LPCTSTR)strFileName );
	}
	else
		theApp.Message( MSG_WARNING, L"Download file already loaded \"%s\".", (LPCTSTR)strFileName );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyApp version

void CEnvyApp::GetVersionNumber()
{
	// Set Build Date
	COleDateTime tCompileTime;
	tCompileTime.ParseDateTime( _T(__DATE__), LOCALE_NOUSEROVERRIDE, 1033 );
	m_sBuildDate = tCompileTime.Format( L"%Y.%m.%d" );

	// Get .exe-file name
	GetModuleFileName( NULL, m_strBinaryPath.GetBuffer( MAX_PATH ), MAX_PATH );
	m_strBinaryPath.ReleaseBuffer( MAX_PATH );

	m_nVersion[0] = m_nVersion[1] = m_nVersion[2] = m_nVersion[3] = 0;

	// Load version from .exe-file properties
	if ( DWORD dwSize = GetFileVersionInfoSize( m_strBinaryPath, &dwSize ) )
	{
		if ( BYTE* pBuffer = new BYTE[ dwSize ] )
		{
			if ( GetFileVersionInfo( m_strBinaryPath, NULL, dwSize, pBuffer ) )
			{
				VS_FIXEDFILEINFO* pTable;

				if ( VerQueryValue( pBuffer, L"\\", (VOID**)&pTable, (UINT*)&dwSize ) )
				{
					m_nVersion[0] = (WORD)( pTable->dwFileVersionMS >> 16 );
					m_nVersion[1] = (WORD)( pTable->dwFileVersionMS & 0xFFFF );
				//	m_nVersion[2] = (WORD)( pTable->dwFileVersionLS >> 16 );
				//	m_nVersion[3] = (WORD)( pTable->dwFileVersionLS & 0xFFFF );
				}
			}

			delete [] pBuffer;
		}
	}

	// XX.0
	m_sVersion.Format( L"%u.%u",
		m_nVersion[0], m_nVersion[1] );

	// Envy XX.0
	m_sSmartAgent = CLIENT_NAME L" ";
	m_sSmartAgent += m_sVersion;

	// ENx0
	m_pBTVersion[0] = BT_ID1;
	m_pBTVersion[1] = BT_ID2;
	m_pBTVersion[2] = (BYTE)m_nVersion[0];
	m_pBTVersion[3] = (BYTE)m_nVersion[1];

	// 0XX0 (Torrent PeerID)
	m_szVersion[0] = theApp.m_nVersion[0] < 100 ? '0' : '1';	// ToDo: Future-proof if ever needed
	m_szVersion[1] = '0' + ( ( ( theApp.m_nVersion[0] % 100 ) - ( theApp.m_nVersion[0] % 10 ) ) / 10 );
	m_szVersion[2] = '0' + theApp.m_nVersion[0] % 10;
	m_szVersion[3] = '0' + theApp.m_nVersion[1];	// 0

	// "Envy XX.0  32/64-bit  (date rXXX)  Debug"
	m_sVersionLong = m_sSmartAgent +
#ifdef WIN64
	L"  64-bit  " +
#else
	L"  32-bit  " +
#endif
	L"(" + m_sBuildDate +
#ifdef __REVISION__
	L" r" _T(__REVISION__) L")" +
#else
	L")" +
#endif
#ifdef __MODAUTHOR__
	L"  " _T(__MODAUTHOR__);	// YOUR NAME (Edit in Revision.h)
#elif defined(_DEBUG)
	L"  Debug";
#else
	L"";
#endif

#ifdef _DEBUG	// BugTrap
	BT_SetAppName( CLIENT_NAME );
	BT_SetAppVersion( m_sVersionLong );
#endif


	// Determine the version of Windows
	//OSVERSIONINFOEX Windows
	//SYSTEM_INFO System

	// Get Service Pack version

	// Determine if it's a server
	//m_bIsServer = Windows.wProductType != VER_NT_WORKSTATION;	// VER_NT_SERVER

	// Many supported windows versions have network limiting
	//m_bLimitedConnections = ! m_bIsServer;

	// Get Major+Minor version (6.1 = 610)
	//	Major ver 5:	Win2000 = 0, WinXP = 1, WinXP64/Server2003 = 2
	//	Major ver 6:	Vista = 0, Server2008 = 0, Windows7 = 1, Windows8 = 2, Windows8.1 = 3
	//	Major ver 10:	Windows10 = 0 (1000)

#if defined(XPSUPPORT) || (_MSC_VER < 1800)
	GetVersionEx( (OSVERSIONINFO*)&Windows );	// Deprecated
	m_bIsWinXP = Windows.dwMajorVersion == 5;
	m_nWinVer = Windows.dwMajorVersion * 100 + Windows.dwMinorVersion * 10;
	TCHAR* sp = _tcsstr( Windows.szCSDVersion, L"Service Pack" );
	if ( sp )
	{
		if ( sp[ 13 ] == '1' )
			m_nWinVer++;
		else if ( sp[ 13 ] == '2' )
			m_nWinVer += 2;
		else if ( sp[ 13 ] == '3' )
			m_nWinVer += 3;
		else if ( sp[ 13 ] == '4' )
			m_nWinVer += 4;
	}
	m_bIsWinXP = m_nWinVer < WIN_VISTA;
#else // WinSDK8.1~

	m_nWinVer =
#ifndef XPSUPPORT
		// IsWindows10OrGreater() unsupported below Win10, IsWindowsVersionOrGreater() unsupported below Vista
		IsWindowsVersionOrGreater( 10, 0, 0 ) ? WIN_10 : // 1000
#endif
		IsWindows8Point1OrGreater() ? WIN_8_1 :			// 630
		IsWindows8OrGreater() ? WIN_8 :					// 620
		IsWindows7OrGreater() ? WIN_7 :					// 610
		IsWindowsVistaSP2OrGreater() ? WIN_VISTA_SP2 :	// 602
		IsWindowsVistaOrGreater() ? WIN_VISTA :			// 600
#ifdef XPSUPPORT
		IsWindowsXPSP2OrGreater() ? WIN_XP_SP2 :		// 512
		IsWindowsXPOrGreater() ? WIN_XP :				// 510
#endif
		0;	// Should never happen

#ifdef XPSUPPORT
	if ( m_nWinVer < WIN_VISTA )
		m_bIsWinXP = true;
#endif

#ifdef XPSUPPORT
	if ( m_nWinVer == WIN_8_1 )		// Test for higher if needed (Win10+)
	{
		OSVERSIONINFOEX osvi = { sizeof( osvi ) };
		if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION, VerSetConditionMask( 0, VER_MAJORVERSION, VER_GREATER_EQUAL ) ) )
			m_nWinVer = WIN_10;
	}
#endif // XP-support

#endif	// WinSDK8.1+

	// Verbose:
//	OSVERSIONINFOEX osvi = { sizeof( osvi ) };
//	osvi.dwMajorVersion = 10;
//	const DWORDLONG dwlMajorMinorPack = VerSetConditionMask( VerSetConditionMask( VerSetConditionMask(
//		0, VER_MAJORVERSION, VER_GREATER_EQUAL ), VER_MINORVERSION, VER_GREATER_EQUAL ), VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );
//
//	m_nWinVer = WIN_XP;							// 510 (Minimum Supported)
//	m_bIsWinXP = false;
//
//	osvi.dwMajorVersion = 10;
//	osvi.dwMinorVersion = 0;
//	if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlMajorMinorPack ) )
//	{
//		m_nWinVer = WIN_10;						// 1000 (Windows 10+)
//	}
//	else
//	{
//		osvi.dwMajorVersion = 6;
//		osvi.dwMinorVersion = 0;
//		if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlMajorMinorPack ) )
//		{
//			m_nWinVer = WIN_VISTA;				// 600
//			osvi.dwMinorVersion = 1;
//			if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlMajorMinorPack ) )
//			{
//				m_nWinVer = WIN_7;				// 610
//				osvi.dwMinorVersion = 2;
//				if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlMajorMinorPack ) )
//				{
//					m_nWinVer = WIN_8;			// 620
//					osvi.dwMinorVersion = 3;
//					if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlMajorMinorPack ) )
//						m_nWinVer = WIN_8_1;	// 630
//				}
//			}
//			else
//			{
//				osvi.dwMinorVersion = 0;
//				osvi.wServicePackMajor = 2;
//				if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlMajorMinorPack ) )
//					m_nWinVer = WIN_VISTA_SP2;	// 602
//			}
//		}
//		else	// Should never be reachable (XP Unsupported on x64)
//		{
//			m_bIsWinXP = true;
//			osvi.dwMajorVersion = 5;
//			osvi.dwMinorVersion = 1;
//			osvi.wServicePackMajor = 2;
//			if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlMajorMinorPack ) )
//			{
//				m_nWinVer = WIN_XP_SP2;			// 512
//				osvi.dwMajorVersion = 5;
//				osvi.dwMinorVersion = 2;
//				osvi.wServicePackMajor = 0;
//				if ( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlMajorMinorPack ) )
//					m_nWinVer = WIN_XP_64;		// 520
//			}
//		}
//	}

#ifdef XPSUPPORT
	if ( Windows.dwMajorVersion == 5 )
	{
		// Windows XP
		if ( Windows.dwMinorVersion == 1 )
		{
			// No network limiting for original XP or SP1
			if ( m_nWinVer < WIN_XP_SP2 )
				m_bLimitedConnections = false;
		}
		// Windows XP64 or 2003
		else if ( Windows.dwMinorVersion == 2 )
		{
			// No network limiting for Vanilla Win2003/XP64
			//if ( ! sp )
				m_bLimitedConnections = false;
		}
	}
	else //if ( Windows.dwMajorVersion >= 6 )
#endif // XP
	{
		if ( Windows.wProductType == VER_NT_SERVER )
		{
			m_bLimitedConnections = false;
			return;
		}

		// Vista SP2+ has Registry patch (Windows 7/8/10?)
		if ( m_nWinVer >= WIN_VISTA_SP2 )
		{
			m_bLimitedConnections = false;

			HKEY hKey;
			if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
				0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
			{
				DWORD nSize = sizeof( DWORD ), nResult = 0, nType = REG_NONE;
				if ( ( RegQueryValueEx( hKey, L"EnableConnectionRateLimiting",
					NULL, &nType, (LPBYTE)&nResult, &nSize ) == ERROR_SUCCESS ) &&
					nType == REG_DWORD && nResult == 1 )
				{
					// ToDo: Request user to modify registry value?
					theApp.Message( MSG_INFO, L"Windows limited connection rate. See Registry SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\EnableConnectionRateLimiting" );
					m_bLimitedConnections = true;
				}

				RegCloseKey( hKey );
				if ( nResult == 1 )
					return;		// Don't double-check below if server
			}
		}
	}

	if ( m_bLimitedConnections && Windows.wProductType == VER_NT_SERVER && ( Windows.wSuiteMask & VER_SUITE_SMALLBUSINESS ) == 0 )
		m_bLimitedConnections = false;
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyApp resources

void CEnvyApp::InitResources()
{
	// Get .exe-file name
	GetModuleFileName( NULL, m_strBinaryPath.GetBuffer( MAX_PATH ), MAX_PATH );
	m_strBinaryPath.ReleaseBuffer( MAX_PATH );

	// Get pointers to some functions that require Windows Vista or greater
	if ( HMODULE hKernel32 = GetModuleHandle( L"kernel32.dll" ) )
	{
		(FARPROC&)m_pfnRegisterApplicationRestart = GetProcAddress( hKernel32, "RegisterApplicationRestart" );			// Vista+	RegisterApplicationRestart() for InitInstance()
	}

	// Get pointers to some functions that require Windows XP or greater
	//if ( ( m_hTheme = LoadLibrary( L"UxTheme.dll" ) ) != NULL )
	//{
	//	(FARPROC&)m_pfnSetWindowTheme = GetProcAddress( m_hTheme, "SetWindowTheme" );
	//	(FARPROC&)m_pfnIsThemeActive  = GetProcAddress( m_hTheme, "IsThemeActive" );
	//	(FARPROC&)m_pfnOpenThemeData  = GetProcAddress( m_hTheme, "OpenThemeData" );
	//	(FARPROC&)m_pfnCloseThemeData = GetProcAddress( m_hTheme, "CloseThemeData" );
	//	(FARPROC&)m_pfnDrawThemeBackground = GetProcAddress( m_hTheme, "DrawThemeBackground" );
	//	(FARPROC&)m_pfnEnableThemeDialogTexture = GetProcAddress( m_hTheme, "EnableThemeDialogTexture" );
	//	(FARPROC&)m_pfnDrawThemeParentBackground = GetProcAddress( m_hTheme, "DrawThemeParentBackground" );
	//	(FARPROC&)m_pfnGetThemeBackgroundContentRect = GetProcAddress( m_hTheme, "GetThemeBackgroundContentRect" );
	//	(FARPROC&)m_pfnDrawThemeText = GetProcAddress( m_hTheme, "DrawThemeText" );
	//	(FARPROC&)m_pfnGetThemeSysFont = GetProcAddress( m_hTheme, "GetThemeSysFont" );
	//}

	// Get pointers to some functions that require Internet Explorer 6.01 or greater
	if ( ( m_hShlWapi = LoadLibrary( L"shlwapi.dll" ) ) != NULL )
	{
		(FARPROC&)m_pfnAssocIsDangerous = GetProcAddress( m_hShlWapi, "AssocIsDangerous" );								// XPsp1+	AssocIsDangerous() for CFileExecutor::IsSafeExecute()
	}

	// Get pointers to shell functions that require Vista or greater
	if ( ( m_hShell32 = LoadLibrary( L"shell32.dll" ) ) != NULL )
	{
	//	(FARPROC&)m_pfnSHGetFolderPathW = GetProcAddress( m_hShell32, "SHGetFolderPathW" );								// Win2K+	SHGetFolderPath()  (Use directly, XP only)
		(FARPROC&)m_pfnSHGetKnownFolderPath = GetProcAddress( m_hShell32, "SHGetKnownFolderPath" );						// Vista+	SHGetKnownFolderPath()
		(FARPROC&)m_pfnSHQueryUserNotificationState = GetProcAddress( m_hShell32, "SHQueryUserNotificationState" );		// Vista+	SHQueryUserNotificationState() for IsUserFullscreen()
		(FARPROC&)m_pfnSHCreateItemFromParsingName = GetProcAddress( m_hShell32, "SHCreateItemFromParsingName" );		// Vista+	SHCreateItemFromParsingName() for CLibraryFolders::Maintain() (Win7 Libraries)
		(FARPROC&)m_pfnSHGetPropertyStoreFromParsingName = GetProcAddress( m_hShell32, "SHGetPropertyStoreFromParsingName" );	// Vista+	SHGetPropertyStoreFromParsingName() for CLibraryBuilderInternals::ExtractProperties()
		(FARPROC&)m_pfnSetCurrentProcessExplicitAppUserModelID = GetProcAddress( m_hShell32, "SetCurrentProcessExplicitAppUserModelID" );
	//	(FARPROC&)m_pfnSHGetImageList = GetProcAddress( m_hShell32, MAKEINTRESOURCEA(727) );							// WinXP+	SHGetImageList() for CShellIcons::Get()  (Use directly)
	}

	if ( ( m_hUser32 = LoadLibrary( L"user32.dll" ) ) != NULL )
	{
		(FARPROC&)m_pfnChangeWindowMessageFilter = GetProcAddress( m_hUser32, "ChangeWindowMessageFilter" );			// Vista+	ChangeWindowMessageFilter() for below only
		(FARPROC&)m_pfnShutdownBlockReasonCreate = GetProcAddress( m_hUser32, "ShutdownBlockReasonCreate" );			// Vista+	ShutdownBlockReasonCreate()
		(FARPROC&)m_pfnShutdownBlockReasonDestroy = GetProcAddress( m_hUser32, "ShutdownBlockReasonDestroy" );			// Vista+	ShutdownBlockReasonDestroy()
	}

	// Windows Vista: Enable drag-n-drop and window control operations from application with lower security level
	if ( theApp.m_pfnChangeWindowMessageFilter )
	{
		VERIFY( theApp.m_pfnChangeWindowMessageFilter( WM_DDE_INITIATE, MSGFLT_ADD ) );
		VERIFY( theApp.m_pfnChangeWindowMessageFilter( WM_DROPFILES, MSGFLT_ADD ) );
		VERIFY( theApp.m_pfnChangeWindowMessageFilter( WM_COPYGLOBALDATA, MSGFLT_ADD ) );
		VERIFY( theApp.m_pfnChangeWindowMessageFilter( WM_COPYDATA, MSGFLT_ADD ) );
		VERIFY( theApp.m_pfnChangeWindowMessageFilter( WM_COMMAND, MSGFLT_ADD ) );
		VERIFY( theApp.m_pfnChangeWindowMessageFilter( WM_CLOSE, MSGFLT_ADD ) );
	}

	LoadCountry();	// GeoIP Initialization

	// Load LibGFL in a custom way, so Envy plugins can use this library too when not in their search path (From Plugins folder, and when running inside Visual Studio)
	m_hLibGFL = CustomLoadLibrary( L"LibGFL340.dll" );

	// DPI:
	if ( Settings.Interface.DisplayScaling < 101 || Settings.Interface.DisplayScaling > 200 )
	{
		CDC ScreenDC;
		ScreenDC.CreateIC( L"DISPLAY", NULL, NULL, NULL );
		const int nDPI = ScreenDC.GetDeviceCaps( LOGPIXELSY );
		Settings.Interface.DisplayScaling =
			nDPI < 100 ? 100 :
			nDPI > 190 ? 200 :
			(DWORD)( ( nDPI * 100 ) / 96 );
	}

	//
	// Setup default fonts:
	//

	// Set Settings.Fonts.Quality to ClearType
	if ( Settings.Fonts.Quality == 0 || Settings.Fonts.Quality == 1 || Settings.Fonts.Quality > 6 )
	{
		UINT nSmoothingType = 0;
		BOOL bFontSmoothing = FALSE;
		if ( SystemParametersInfo( SPI_GETFONTSMOOTHING, 0, &bFontSmoothing, 0 ) && bFontSmoothing &&
			 SystemParametersInfo( SPI_GETFONTSMOOTHINGTYPE, 0, &nSmoothingType, 0 ) )
		{
			Settings.Fonts.Quality =	// Was m_nFontQuality
				( nSmoothingType == FE_FONTSMOOTHINGSTANDARD ) ? ANTIALIASED_QUALITY :
				( nSmoothingType == FE_FONTSMOOTHINGCLEARTYPE ) ? CLEARTYPE_NATURAL_QUALITY :	// Note XPsp1+ ?
				DEFAULT_QUALITY;
		}
	}

	if ( Settings.Fonts.DefaultFont.IsEmpty() )
	{
		// Get font from current theme
		LOGFONT pFont = {};
		if ( GetThemeSysFont( NULL, TMT_MENUFONT, &pFont ) == S_OK )
			Settings.Fonts.DefaultFont = pFont.lfFaceName;
	}

	if ( Settings.Fonts.DefaultFont.IsEmpty() )
	{
		// Get font by legacy method
		NONCLIENTMETRICS pMetrics = { sizeof( NONCLIENTMETRICS ) };
		SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof( NONCLIENTMETRICS ), &pMetrics, 0 );
		Settings.Fonts.DefaultFont = pMetrics.lfMenuFont.lfFaceName;
	}

	if ( Settings.Fonts.SystemLogFont.IsEmpty() )
		Settings.Fonts.SystemLogFont = Settings.Fonts.DefaultFont;

	if ( Settings.Fonts.PacketDumpFont.IsEmpty() )
		Settings.Fonts.PacketDumpFont = L"Lucida Console";

	m_gdiFont.CreateFont( -(int)Settings.Fonts.DefaultSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_gdiFontBold.CreateFont( -(int)Settings.Fonts.DefaultSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_gdiFontLine.CreateFont( -(int)Settings.Fonts.DefaultSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	CryptAcquireContext( &m_hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT );

	srand( GetTickCount() );

	m_hHookKbd   = SetWindowsHookEx( WH_KEYBOARD, (HOOKPROC)KbdHook, NULL, AfxGetThread()->m_nThreadID );
	m_hHookMouse = SetWindowsHookEx( WH_MOUSE,  (HOOKPROC)MouseHook, NULL, AfxGetThread()->m_nThreadID );
	m_nLastInput = (DWORD)time( NULL );

	if ( SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &m_nMouseWheel, 0 ) )
	{
		if ( m_nMouseWheel > 20 )			// Catch WHEEL_PAGESCROLL (UINT_MAX)
			m_nMouseWheel = 20;				// ToDo: Better handling rare mouse wheel set to scroll by page?
		else if ( m_nMouseWheel < 1 )
			m_nMouseWheel = 3;				// Default lines set at initialization	ToDo: User Setting?
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyApp custom library loader

HINSTANCE CEnvyApp::CustomLoadLibrary(LPCTSTR pszFileName)
{
	HINSTANCE hLibrary = NULL;

	if ( ( hLibrary = LoadLibrary( pszFileName ) ) != NULL ||
		 ( hLibrary = LoadLibrary( Settings.General.Path + L"\\" + pszFileName ) ) != NULL )
		return hLibrary;

	TRACE( L"DLL not found: %s\r\n", pszFileName );
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyApp safe main window

CMainWnd* CEnvyApp::SafeMainWnd() const
{
	if ( m_pSafeWnd == NULL ) return NULL;
	ASSERT_KINDOF( CMainWnd, m_pSafeWnd );
	return static_cast< CMainWnd* >( IsWindow( m_pSafeWnd->m_hWnd ) ? m_pSafeWnd : NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CEnvyApp message

void CEnvyApp::SetClipboard(const CString& strText, BOOL bShowTray /*=FALSE*/)
{
	if ( ! m_pMainWnd || ! m_pMainWnd->OpenClipboard() ) return;

	EmptyClipboard();

	if ( ! strText.IsEmpty() )
	{
		CT2CW pszWide( (LPCTSTR)strText );
		const DWORD nSize = ( lstrlenW(pszWide) + 1 ) * sizeof( WCHAR );
		if ( HANDLE hMem = GlobalAlloc( GMEM_MOVEABLE|GMEM_DDESHARE, nSize ) )
		{
			if ( LPVOID pMem = GlobalLock( hMem ) )
			{
				CopyMemory( pMem, pszWide, nSize );
				GlobalUnlock( hMem );
				SetClipboardData( CF_UNICODETEXT, hMem );

				if ( bShowTray )
				{
					CString str = strText.Left( 180 );
					int nBreak = str.GetLength() > 80 ? str.Find( L':' ) : 0;
					if ( nBreak > 1 && nBreak < 20 )
						str = L"(" + strText.Left( nBreak + 1 ) + L")";
					theApp.Message( MSG_TRAY|MSG_NOTICE, LoadString( IDS_COPIED_TO_CLIPBOARD ) + L"\n" + str );
				}
			}
		}
	}

	CloseClipboard();
}

BOOL CEnvyApp::GetClipboard(CString& strText)
{
	if ( ! m_pMainWnd || ! m_pMainWnd->OpenClipboard() )
		return FALSE;

	if ( HGLOBAL hData = GetClipboardData( CF_UNICODETEXT ) )
	{
		size_t nData = GlobalSize( hData );
		LPVOID pData = GlobalLock( hData );

		LPTSTR pszData = strText.GetBuffer( (int)( nData + 1 ) / 2 + 1 );
		CopyMemory( pszData, pData, nData );
		pszData[ ( nData + 1 ) / 2 ] = 0;
		strText.ReleaseBuffer();
		GlobalUnlock( hData );

		CloseClipboard();
		return strText.GetLength() > 0;
	}

	CloseClipboard();
	return FALSE;
}

bool CEnvyApp::IsLogDisabled(WORD nType) const
{
	return ( static_cast< DWORD >( nType & MSG_SEVERITY_MASK ) > Settings.General.LogLevel ) ||		// Severity filter
		( ( nType & MSG_FACILITY_MASK ) == MSG_FACILITY_SEARCH && ! Settings.General.SearchLog );	// Facility filter
}

void CEnvyApp::ShowStartupText()
{
	CString strBody;
	LoadString( strBody, IDS_SYSTEM_MESSAGE );

	strBody.Replace( L"{version}", (LPCTSTR)theApp.m_sVersionLong );

	for ( ; strBody.GetLength() ; )
	{
		CString strLine = strBody.SpanExcluding( L"\r\n" );
		strBody = strBody.Mid( strLine.GetLength() + 1 );

		strLine.Trim();
		if ( strLine.IsEmpty() )
			strLine = L" ";

		if ( strLine[ 0 ] == L'!' )
			PrintMessage( MSG_NOTICE, (LPCTSTR)strLine + 1 );
		else
			PrintMessage( MSG_INFO, strLine );
	}

	CString strCPU;
	strCPU.Format( L"%u x CPU.  Features:", System.dwNumberOfProcessors );
	if ( Machine::SupportsMMX() )
		strCPU += L" MMX";
	if ( Machine::SupportsSSE() )
		strCPU += L" SSE";
	if ( Machine::SupportsSSE2() )
		strCPU += L" SSE2";
	if ( Machine::SupportsSSE3() )
		strCPU += L" SSE3";
	if ( Machine::SupportsSSSE3() )
		strCPU += L" SSSE3";
	if ( Machine::SupportsSSE41() )
		strCPU += L" SSE4.1";
	if ( Machine::SupportsSSE42() )
		strCPU += L" SSE4.2";
	if ( Machine::SupportsSSE4A() )
		strCPU += L" SSE4A";
	if ( Machine::SupportsSSE5() )
		strCPU += L" SSE5";
	if ( Machine::Supports3DNOW() )
		strCPU += L" 3DNow";
	if ( Machine::Supports3DNOWEXT() )
		strCPU += L" 3DNowExt";
	PrintMessage( MSG_INFO, strCPU );
	PrintMessage( MSG_DEBUG, IsRunAsAdmin() ? L"Running with administrative privileges." : L"Running without administrative privileges." );
}

void CEnvyApp::Message(WORD nType, UINT nID, ...)
{
	// Check if logging this type of message is enabled
	if ( IsLogDisabled( nType ) )
		return;

	// Initialize variable arguments list
	va_list pArgs;
	va_start( pArgs, nID );

	// Load the format string from the resource file
	CString strFormat;
	LoadString( strFormat, nID );

	// Work out the type of format string and call the appropriate function
	CString strTemp;
	if ( strFormat.Find( L"%1" ) >= 0 )
		strTemp.FormatMessageV( strFormat, &pArgs );
	else
		strTemp.FormatV( strFormat, pArgs );

	// Print the message if there still is one
	if ( ! strTemp.IsEmpty() )
		PrintMessage( nType, strTemp );

	// Null the argument list pointer
	va_end( pArgs );

	return;
}

void CEnvyApp::Message(WORD nType, LPCTSTR pszFormat, ...)
{
	// Check if logging this type of message is enabled
	if ( IsLogDisabled( nType ) )
		return;

	// Initialize variable arguments list
	va_list pArgs;
	va_start( pArgs, pszFormat );

	// Format the message
	CString strTemp;
	strTemp.FormatV( pszFormat, pArgs );

	// Print the message if there still is one
	if ( strTemp.GetLength() > 1 )
		PrintMessage( nType, strTemp );

	// Null the argument list pointer
	va_end( pArgs );

	return;
}

void CEnvyApp::PrintMessage(WORD nType, const CString& strLog)
{
	if ( Settings.General.DebugLog )
		LogMessage( strLog );

	CAutoPtr< CLogMessage > pMsg( new CLogMessage( nType, strLog ) );
	if ( ! pMsg )
		return;		// Out of memory

	CQuickLock pLock( m_csMessage );

	// Max 1000 lines	// ToDo: Setting?
	if ( m_oMessages.GetCount() >= 1000 )
		delete m_oMessages.RemoveHead();

	m_oMessages.AddTail( pMsg.Detach() );
}

void CEnvyApp::LogMessage(const CString& strLog)
{
	static const CString strPath = SafePath( Settings.General.DataPath + L"Envy.log" );		// CLIENT_NAME

	CQuickLock pLock( m_csMessage );

	CFile pFile;
	if ( pFile.Open( strPath, CFile::modeReadWrite ) )
	{
		if ( Settings.General.MaxDebugLogSize &&						// If log rotation is on
			 pFile.GetLength() > Settings.General.MaxDebugLogSize )		// and file is too long...
		{
			// Close the file
			pFile.Close();

			// Rotate the logs
			MoveFileEx( strPath, strPath + L".old", MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH );

			// Start a new log
			if ( ! pFile.Open( strPath, CFile::modeWrite|CFile::modeCreate ) ) return;
			// Unicode marker
			WORD nByteOrder = 0xFEFF;
			pFile.Write( &nByteOrder, 2 );
		}
		else
		{
			// Go to end of current file to add entries
			pFile.Seek( 0, CFile::end );
		}
	}
	else
	{
		// Start a new log
		if ( ! pFile.Open( strPath, CFile::modeWrite|CFile::modeCreate ) ) return;
		// Unicode marker
		WORD nByteOrder = 0xFEFF;
		pFile.Write( &nByteOrder, 2 );
	}

	if ( Settings.General.ShowTimestamp )
	{
		CString strTime;
		CTime pNow = CTime::GetCurrentTime();
		strTime.Format( L"%.2i:%.2i:%.2i  ", pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond() );
		pFile.Write( (LPCTSTR)strTime, sizeof( TCHAR ) * strTime.GetLength() );
	}

	pFile.Write( (LPCTSTR)strLog, static_cast< UINT >( sizeof( TCHAR ) * strLog.GetLength() ) );
	pFile.Write( L"\r\n", sizeof( TCHAR ) * 2 );

	pFile.Close();
}

CString GetErrorString(DWORD dwError)
{
	CString strMessage;
	LPTSTR MessageBuffer = NULL;

	if ( FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		 NULL, dwError, 0, (LPTSTR)&MessageBuffer, 0, NULL ) )
	{
		strMessage = MessageBuffer;
		strMessage.Trim( L" \t\r\n" );
		LocalFree( MessageBuffer );
		return strMessage;
	}

	static LPCTSTR const szModules [] =
	{
		L"netapi32.dll",
		L"netmsg.dll",
		L"wininet.dll",
		L"ntdll.dll",
		L"ntdsbmsg.dll",
		NULL
	};

	for ( int i = 0 ; szModules[ i ] ; i++ )
	{
		if ( HMODULE hModule = LoadLibraryEx( szModules[ i ], NULL, LOAD_LIBRARY_AS_DATAFILE ) )
		{
			DWORD bResult = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
				hModule, dwError, 0, (LPTSTR)&MessageBuffer, 0, NULL );
			FreeLibrary( hModule );
			if ( bResult )
			{
				strMessage = MessageBuffer;
				strMessage.Trim( L" \t\r\n" );
				LocalFree( MessageBuffer );
				return strMessage;
			}
		}
	}

	return CString();
}

//void ReportError(DWORD dwError)
//{
//	CString strError = GetErrorString( dwError );
//	theApp.Message( MSG_ERROR, L"%s", strError );
//	MsgBox( strError, MB_OK | MB_ICONEXCLAMATION );
//}

/////////////////////////////////////////////////////////////////////////////
// CEnvyApp GeoIP Countries

CString CEnvyApp::GetCountryCode(IN_ADDR pAddress) const
{
	if ( m_pfnGeoIP_country_code_by_ipnum && m_pGeoIP )
		return CString( m_pfnGeoIP_country_code_by_ipnum( m_pGeoIP, htonl( pAddress.s_addr ) ) );
	return L"";
}

CString CEnvyApp::GetCountryName(IN_ADDR pAddress) const
{
	if ( m_pfnGeoIP_country_name_by_ipnum && m_pGeoIP )
		return CString( m_pfnGeoIP_country_name_by_ipnum( m_pGeoIP, htonl( pAddress.s_addr ) ) );
	return L"";
}

void CEnvyApp::LoadCountry()
{
	if ( ( m_hGeoIP = CustomLoadLibrary( L"GeoIP.dll" ) ) != NULL )
	{
		GeoIP_newFunc pfnGeoIP_new = (GeoIP_newFunc)GetProcAddress( m_hGeoIP, "GeoIP_new" );
		m_pfnGeoIP_delete  = (GeoIP_deleteFunc)GetProcAddress( m_hGeoIP, "GeoIP_delete" );
		m_pfnGeoIP_cleanup = (GeoIP_cleanupFunc)GetProcAddress( m_hGeoIP, "GeoIP_cleanup" );
		m_pfnGeoIP_country_code_by_ipnum = (GeoIP_country_code_by_ipnumFunc)GetProcAddress( m_hGeoIP, "GeoIP_country_code_by_ipnum" );
		m_pfnGeoIP_country_name_by_ipnum = (GeoIP_country_name_by_ipnumFunc)GetProcAddress( m_hGeoIP, "GeoIP_country_name_by_ipnum" );
		if ( pfnGeoIP_new )
			m_pGeoIP = pfnGeoIP_new( GEOIP_MEMORY_CACHE );
	}
}

void CEnvyApp::FreeCountry()
{
	if ( ! m_hGeoIP )
		return;

	if ( m_pGeoIP && m_pfnGeoIP_delete )
	{
		__try
		{
			m_pfnGeoIP_delete( m_pGeoIP );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
		m_pGeoIP = NULL;
	}

	if ( m_pfnGeoIP_cleanup )
	{
		__try
		{
			m_pfnGeoIP_cleanup();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}

	FreeLibrary( m_hGeoIP );
	m_hGeoIP = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CEnvyApp process an internal URI

BOOL CEnvyApp::InternalURI(LPCTSTR pszURI)
{
	CMainWnd* pMainWnd = SafeMainWnd();
	if ( pMainWnd == NULL ) return FALSE;

	CString strURI( pszURI );
//	const int nBreak = strURI.FindOneOf( L":" ) + 1;
//	strURI = strURI.Left( 24 ).MakeLower();					// Most chars needed to determine protocol or command

	if ( ! StartsWith( strURI, _P( L"command:" ) ) )		// Assume external URL if not internal command
	{
		if ( StartsWith( strURI, _P( L"magnet:" ) ) ||
			StartsWith( strURI, _P( L"http://" ) ) ||
			StartsWith( strURI, _P( L"https://" ) ) ||
			StartsWith( strURI, _P( L"ftp://" ) ) ||
			StartsWith( strURI, _P( L"gnutella:" ) ) ||
			StartsWith( strURI, _P( L"gnutella1:" ) ) ||
			StartsWith( strURI, _P( L"gnutella2:" ) ) ||
			StartsWith( strURI, _P( L"peerproject:" ) ) ||
			StartsWith( strURI, _P( L"shareaza:" ) ) ||
			StartsWith( strURI, _P( L"envy:" ) ) ||
			StartsWith( strURI, _P( L"ed2k:" ) ) ||
			StartsWith( strURI, _P( L"g2:" ) ) ||
			StartsWith( strURI, _P( L"gwc:" ) ) ||
			StartsWith( strURI, _P( L"uhc:" ) ) ||
			StartsWith( strURI, _P( L"ukhl:" ) ) ||
			StartsWith( strURI, _P( L"gnet:" ) ) ||
			StartsWith( strURI, _P( L"peer:" ) ) ||
			StartsWith( strURI, _P( L"p2p:" ) ) ||
			StartsWith( strURI, _P( L"mp2p:" ) ) ||
			StartsWith( strURI, _P( L"foxy:" ) ) ||
			StartsWith( strURI, _P( L"btc:" ) ) ||
			StartsWith( strURI, _P( L"irc:" ) ) ||
			StartsWith( strURI, _P( L"aim:" ) ) ||
			StartsWith( strURI, _P( L"adc:" ) ) ||
			StartsWith( strURI, _P( L"dchub:" ) ) ||
			StartsWith( strURI, _P( L"dcfile:" ) ) ||
			StartsWith( strURI, _P( L"mailto:" ) ) ||
			StartsWith( strURI, _P( L"sig2dat:" ) ) )
		{
			ShellExecute( pMainWnd->GetSafeHwnd(), L"open", pszURI, NULL, NULL, SW_SHOWNORMAL );
			return TRUE;
		}

		theApp.Message( MSG_ERROR, L"Unknown link URI:  %s", pszURI );
		return FALSE;
	}

	// Specific "command:" prefixed internal utilities:

	if ( _tcsnicmp( strURI, _P( L"command:id_" ) ) == 0 )				// Common "command:ID_"
	{
		if ( UINT nCmdID = CoolInterface.NameToID( pszURI + 8 ) )
		{
			pMainWnd->PostMessage( WM_COMMAND, nCmdID );
			return TRUE;
		}
	}
	else if ( _tcsnicmp( strURI, _P( L"command:shell:" ) ) == 0 ) 		// Assume "command:shell:downloads"
	{
		ShellExecute( pMainWnd->GetSafeHwnd(), L"open",
			Settings.Downloads.CompletePath, NULL, NULL, SW_SHOWNORMAL );
		if ( strURI.Find( L":downloads", 12 ) > 1 )
			return TRUE;
	}
	else if ( _tcsnicmp( strURI, _P( L"command:update" ) ) == 0 ) 		// Version notice "command:update"
	{
		pMainWnd->PostMessage( WM_VERSIONCHECK, VC_CONFIRM );
		return TRUE;
	}
	else if ( _tcsnicmp( strURI, _P( L"command:copy:" ) ) == 0 )		// Clipboard "command:copy:<text>"
	{
		strURI = CString( pszURI + 13 );
		SetClipboard( strURI, TRUE );
		return TRUE;
	}
	//else if ( _tcsnicmp( strURI, _P( L"command:launch:" ) ) == 0 )	// Unused but useful? "command:launch:"
	//{
	//	DWORD nIndex = 0;
	//	_stscanf( (LPCTSTR)strURI + 12, L"%lu", &nIndex );
	//
	//	CSingleLock oLock( &Library.m_pSection, TRUE );
	//	if ( CLibraryFile* pFile = Library.LookupFile( nIndex ) )
	//	{
	//		if ( pFile->IsAvailable() )
	//		{
	//			CString strPath = pFile->GetPath();
	//			oLock.Unlock();
	//			CFileExecutor::Execute( strPath );
	//			return TRUE;
	//		}
	//	}
	//}
	//else if ( _tcsnicmp( strURI, _P( L"command:windowptr:" ) ) == 0 ) // Unused but useful? "command:windowptr:"
	//{
	//	CChildWnd* pChild = NULL;
	//	_stscanf( (LPCTSTR)strURI + 15, L"%lu", &pChild );
	//	if ( pMainWnd->m_pWindows.Check( pChild ) )
	//	{
	//		pChild->MDIActivate();
	//		return TRUE;
	//	}
	//}

	//else if ( _tcsnicmp( strURI, _P( L"page:" ) ) == 0 )				// "page:CSettingsPage" defined locally in PageSettingsRich

	theApp.Message( MSG_ERROR, L"Unknown internal command:  %s", pszURI );
	//ASSERT( FALSE );
	return FALSE;
}

BOOL IsRunAsAdmin()
{
	BOOL bIsRunAsAdmin = FALSE;
	PSID pAdministratorsGroup = NULL;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if ( AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup ) )
	{
		CheckTokenMembership( NULL, pAdministratorsGroup, &bIsRunAsAdmin );
		FreeSid( pAdministratorsGroup );
	}

	return bIsRunAsAdmin;
}

/////////////////////////////////////////////////////////////////////////////
// Runtime class lookup

//void AFXAPI AfxLockGlobals(int nLockType);
//void AFXAPI AfxUnlockGlobals(int nLockType);

CRuntimeClass* AfxClassForName(LPCTSTR pszClass)
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();

	AfxLockGlobals( 0 );

	for ( CRuntimeClass* pClass = pModuleState->m_classList ; pClass != NULL ; pClass = pClass->m_pNextClass )
	{
		if ( CString( pClass->m_lpszClassName ).CompareNoCase( pszClass ) == 0 )
		{
			AfxUnlockGlobals( 0 );
			return pClass;
		}
	}

	AfxUnlockGlobals( 0 );

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// String functions  (See Strings.cpp)

BOOL LoadString(CString& str, UINT nID)
{
	return Skin.LoadString( str, nID );
}

CString LoadString(UINT nID)
{
	CString str;
	LoadString( str, nID );
	return str;
}

BOOL LoadSourcesString(CString& str, DWORD num, bool bFraction)
{
	if ( bFraction )
		return Skin.LoadString( str, IDS_STATUS_SOURCES );

	if ( num == 0 )
		return Skin.LoadString( str, IDS_STATUS_NOSOURCES );

	if ( num == 1 )
		return Skin.LoadString( str, IDS_STATUS_SOURCE );

	if ( ( num % 100 ) > 10 && ( num % 100 ) < 20 )	// 11-19 exempt
		return Skin.LoadString( str, IDS_STATUS_SOURCES );

	switch ( num % 10 )
	{
	case 0:
		return Skin.LoadString( str, IDS_STATUS_SOURCES );
	case 1:
		return Skin.LoadString( str, IDS_STATUS_SOURCES_ONES );
	case 2:
	case 3:
	case 4:
		return Skin.LoadString( str, IDS_STATUS_SOURCES_FEW );
	default:
		return Skin.LoadString( str, IDS_STATUS_SOURCES );
	}
}

/////////////////////////////////////////////////////////////////////////////
// Time Management Functions (C-runtime)

DWORD TimeFromString(LPCTSTR pszTime)
{
	// 2002-04-30T08:30Z

	if ( _tcslen( pszTime ) != 17 ) return 0;
	if ( pszTime[4] != '-' || pszTime[7] != '-' ) return 0;
	if ( pszTime[10] != 'T' || pszTime[13] != ':' || pszTime[16] != 'Z' ) return 0;

	LPCTSTR psz;
	int nTemp;

	tm pTime = {};

	if ( _stscanf( pszTime, L"%i", &nTemp ) != 1 ) return 0;
	pTime.tm_year = nTemp - 1900;
	for ( psz = pszTime + 5 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, L"%i", &nTemp ) != 1 ) return 0;
	pTime.tm_mon = nTemp - 1;
	for ( psz = pszTime + 8 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, L"%i", &nTemp ) != 1 ) return 0;
	pTime.tm_mday = nTemp;
	for ( psz = pszTime + 11 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, L"%i", &nTemp ) != 1 ) return 0;
	pTime.tm_hour = nTemp;
	for ( psz = pszTime + 14 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, L"%i", &nTemp ) != 1 ) return 0;
	pTime.tm_min = nTemp;

	time_t tGMT = mktime( &pTime ), tSub;
	tm pGM = {};
	if ( tGMT == -1 ||
		gmtime_s( &pGM, &tGMT ) != 0 ||
		( tSub = mktime( &pGM ) ) == -1 )
	{
	//	theApp.Message( MSG_ERROR, L"Invalid Date/Time", pszTime );
		return 0;
	}

	return DWORD( 2 * tGMT - tSub );
}

CString TimeToString(time_t tVal)
{
	tm time = {};
	CString str;
	if ( gmtime_s( &time, &tVal ) == 0 )
	{
		str.Format( L"%.4i-%.2i-%.2iT%.2i:%.2iZ",
			time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
			time.tm_hour, time.tm_min );
	}
	return str;
}

/////////////////////////////////////////////////////////////////////////////
// Time Management Functions (FILETIME)

BOOL TimeFromString(LPCTSTR pszTime, FILETIME* pTime)
{
	// 2002-04-30T08:30Z

	if ( _tcslen( pszTime ) != 17 ) return FALSE;
	if ( pszTime[4] != '-' || pszTime[7] != '-' ) return FALSE;
	if ( pszTime[10] != 'T' || pszTime[13] != ':' || pszTime[16] != 'Z' ) return FALSE;

	LPCTSTR psz;
	int nTemp;

	SYSTEMTIME pOut = {};

	if ( _stscanf( pszTime, L"%i", &nTemp ) != 1 ) return FALSE;
	pOut.wYear = WORD( nTemp );
	for ( psz = pszTime + 5 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, L"%i", &nTemp ) != 1 ) return FALSE;
	pOut.wMonth = WORD( nTemp );
	for ( psz = pszTime + 8 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, L"%i", &nTemp ) != 1 ) return FALSE;
	pOut.wDay = WORD( nTemp );
	for ( psz = pszTime + 11 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, L"%i", &nTemp ) != 1 ) return FALSE;
	pOut.wHour = WORD( nTemp );
	for ( psz = pszTime + 14 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, L"%i", &nTemp ) != 1 ) return FALSE;
	pOut.wMinute = WORD( nTemp );

	return SystemTimeToFileTime( &pOut, pTime );
}

CString TimeToString(FILETIME* pTime)
{
	SYSTEMTIME pOut;
	CString str;

	FileTimeToSystemTime( pTime, &pOut );

	str.Format( L"%.4i-%.2i-%.2iT%.2i:%.2iZ",
		pOut.wYear, pOut.wMonth, pOut.wDay,
		pOut.wHour, pOut.wMinute );

	return str;
}

/////////////////////////////////////////////////////////////////////////////
// Automatic dropdown list width adjustment (to fit translations)
// Use in ON_CBN_DROPDOWN events

void RecalcDropWidth(CComboBox* pWnd, int nMargin /*0*/)
{
	int nWidth = 0;

	CDC* pDC = pWnd->GetDC();
	CFont* pOldFont = pDC->SelectObject( pWnd->GetFont() );

	TEXTMETRIC tm;
	pDC->GetTextMetrics( &tm );

	CString str;
	const int nNumEntries = pWnd->GetCount();
	for ( int nEntry = 0 ; nEntry < nNumEntries ; ++nEntry )
	{
		pWnd->GetLBText( nEntry, str );
		int nLength = pDC->GetTextExtent( str ).cx;
		if ( nLength > nWidth )
			nWidth = nLength;
	}

	pDC->SelectObject( pOldFont );
	pWnd->ReleaseDC( pDC );

	// Add margin space to the calculations
	nWidth += tm.tmAveCharWidth + ::GetSystemMetrics( SM_CXVSCROLL ) + ::GetSystemMetrics( SM_CXEDGE ) * 2 + nMargin;

	pWnd->SetDroppedWidth( nWidth );
}

BOOL LoadIcon(LPCTSTR szFilename, HICON* phSmallIcon, HICON* phLargeIcon, HICON* phHugeIcon, int nIcon)
{
	CString strIcon( szFilename );

	if ( phSmallIcon ) *phSmallIcon = NULL;
	if ( phLargeIcon ) *phLargeIcon = NULL;
	if ( phHugeIcon )  *phHugeIcon  = NULL;

	int nIndex = strIcon.ReverseFind( L',' );
	if ( nIndex != -1 )
	{
		if ( _stscanf( strIcon.Mid( nIndex + 1 ), L"%i", &nIcon ) == 1 )
			strIcon = strIcon.Left( nIndex );
	}
	else
		nIndex = 0;

	if ( strIcon.GetLength() < 3 )
		return FALSE;

	if ( strIcon.GetAt( 0 ) == L'\"' &&
		 strIcon.GetAt( strIcon.GetLength() - 1 ) == L'\"' )
		strIcon = strIcon.Mid( 1, strIcon.GetLength() - 2 );

	if ( phLargeIcon || phSmallIcon )
		ExtractIconEx( strIcon, nIcon, phLargeIcon, phSmallIcon, 1 );

	if ( phHugeIcon )
	{
		UINT nLoadedID;
		PrivateExtractIcons( strIcon, nIcon, 48, 48, phHugeIcon, &nLoadedID, 1, 0 );
	}

	return ( phLargeIcon && *phLargeIcon ) ||
		   ( phSmallIcon && *phSmallIcon ) ||
		   ( phHugeIcon && *phHugeIcon );
}

//HICON LoadCLSIDIcon(LPCTSTR szCLSID)
//{
//	HKEY hKey;
//	CString strPath;
//	strPath.Format( L"CLSID\\%s\\InProcServer32", szCLSID );
//	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, strPath, 0, KEY_READ, &hKey ) != ERROR_SUCCESS )
//	{
//		strPath.Format( L"CLSID\\%s\\LocalServer32", szCLSID );
//		if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, strPath, 0, KEY_READ, &hKey ) != ERROR_SUCCESS )
//			return NULL;
//	}
//
//	DWORD dwType = REG_SZ, dwSize = MAX_PATH * sizeof( TCHAR );
//	LONG lResult = RegQueryValueEx( hKey, L"", NULL, &dwType,
//		(LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
//	strPath.ReleaseBuffer( dwSize / sizeof( TCHAR ) );
//	RegCloseKey( hKey );
//
//	if ( lResult != ERROR_SUCCESS )
//		return NULL;
//
//	strPath.Trim( L" \"" );
//
//	HICON hSmallIcon;
//	if ( ! LoadIcon( strPath, &hSmallIcon, NULL, NULL ) )
//		return NULL;
//
//	return hSmallIcon;
//}

int AddIcon(UINT nIcon, CImageList& gdiImageList)
{
	return AddIcon( theApp.LoadIcon( nIcon ), gdiImageList );
}

int AddIcon(HICON hIcon, CImageList& gdiImageList)
{
	int num = -1;
	if ( hIcon )
	{
		if ( Settings.General.LanguageRTL )
			hIcon = CreateMirroredIcon( hIcon );
		num = gdiImageList.Add( hIcon );
		VERIFY( DestroyIcon( hIcon ) );
	}
	return num;
}

HICON CreateMirroredIcon(HICON hIconOrig, BOOL bDestroyOriginal)
{
	HICON hIcon = NULL;
	HDC hdcMask = NULL;
	HDC hdcBitmap = CreateCompatibleDC( NULL );

	if ( hdcBitmap )
	{
		hdcMask = CreateCompatibleDC( NULL );
		if ( hdcMask )
		{
			SetLayout( hdcBitmap, LAYOUT_RTL );
			SetLayout( hdcMask, LAYOUT_RTL );
		}
		else
		{
			DeleteDC( hdcBitmap );
			hdcBitmap = NULL;
		}
	}

	if ( HDC hdcScreen = GetDC( NULL ) )
	{
		if ( hdcBitmap && hdcMask && hIconOrig )
		{
			HBITMAP hbm, hbmMask, hbmOld, hbmOldMask;
			BITMAP bm;
			ICONINFO ii;
			if ( GetIconInfo( hIconOrig, &ii ) && GetObject( ii.hbmColor, sizeof( BITMAP ), &bm ) )
			{
				// Do the cleanup for the bitmaps.
				DeleteObject( ii.hbmMask );
				DeleteObject( ii.hbmColor );
				ii.hbmMask = ii.hbmColor = NULL;
				hbm = CreateCompatibleBitmap( hdcScreen, bm.bmWidth, bm.bmHeight );
				if ( hbm != NULL )
				{
					hbmMask = CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL );
					if ( hbmMask != NULL )
					{
						hbmOld = (HBITMAP)SelectObject( hdcBitmap, hbm );
						hbmOldMask = (HBITMAP)SelectObject( hdcMask, hbmMask );
						DrawIconEx( hdcBitmap, 0, 0, hIconOrig, bm.bmWidth, bm.bmHeight, 0, NULL, DI_IMAGE );
						DrawIconEx( hdcMask, 0, 0, hIconOrig, bm.bmWidth, bm.bmHeight, 0, NULL, DI_MASK );
						SelectObject( hdcBitmap, hbmOld );
						SelectObject( hdcMask, hbmOldMask );
						// Create the new mirrored icon and delete bitmaps

						ii.hbmMask = hbmMask;
						ii.hbmColor = hbm;
						hIcon = CreateIconIndirect( &ii );
						DeleteObject( hbmMask );
					}
					DeleteObject( hbm );
				}
			}
		}

		ReleaseDC( NULL, hdcScreen );
	}

	if ( hdcBitmap ) DeleteDC( hdcBitmap );
	if ( hdcMask ) DeleteDC( hdcMask );
	if ( hIcon && hIconOrig && bDestroyOriginal ) VERIFY( DestroyIcon( hIconOrig ) );
	if ( ! hIcon ) hIcon = hIconOrig;
	return hIcon;
}

HBITMAP CreateMirroredBitmap(HBITMAP hbmOrig)
{
	BITMAP bm;
	HBITMAP hbm = NULL, hOld_bm1, hOld_bm2;
	if ( ! hbmOrig ) return NULL;
	if ( ! GetObject( hbmOrig, sizeof( BITMAP ), &bm ) ) return NULL;

	if ( HDC hdc = GetDC( NULL ) )
	{
		HDC hdcMem1 = CreateCompatibleDC( hdc );
		if ( ! hdcMem1 )
		{
			ReleaseDC( NULL, hdc );
			return NULL;
		}
		HDC hdcMem2 = CreateCompatibleDC( hdc );
		if ( ! hdcMem2 )
		{
			DeleteDC( hdcMem1 );
			ReleaseDC( NULL, hdc );
			return NULL;
		}
		hbm = CreateCompatibleBitmap( hdc, bm.bmWidth, bm.bmHeight );
		if ( ! hbm )
		{
			DeleteDC( hdcMem1 );
			DeleteDC( hdcMem2 );
			ReleaseDC( NULL, hdc );
			return NULL;
		}
		// Flip the bitmap.
		hOld_bm1 = (HBITMAP)SelectObject( hdcMem1, hbmOrig );
		hOld_bm2 = (HBITMAP)SelectObject( hdcMem2, hbm );
		SetLayout( hdcMem2, LAYOUT_RTL );
		BitBlt( hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCCOPY );
		SelectObject( hdcMem1, hOld_bm1 );
		SelectObject( hdcMem2, hOld_bm2 );
		DeleteDC( hdcMem1 );
		DeleteDC( hdcMem2 );
		ReleaseDC( NULL, hdc );
	}

	return hbm;
}

/////////////////////////////////////////////////////////////////////////////
// Keyboard hook: record tick count

LRESULT CALLBACK KbdHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if ( nCode == HC_ACTION )
	{
		theApp.m_nLastInput = (DWORD)time( NULL );

		BOOL bAlt = (WORD)( lParam >> 16 ) & KF_ALTDOWN;
		// BOOL bCtrl = GetAsyncKeyState( VK_CONTROL ) & 0x80000000;
		if ( bAlt )
		{
			if ( wParam == VK_DOWN )
				SendMessage( AfxGetMainWnd()->GetSafeHwnd(), WM_SETALPHA, (WPARAM)0, 0 );
			else if ( wParam == VK_UP )
				SendMessage( AfxGetMainWnd()->GetSafeHwnd(), WM_SETALPHA, (WPARAM)1, 0 );
		}
	}

	return ::CallNextHookEx( theApp.m_hHookKbd, nCode, wParam, lParam );
}

/////////////////////////////////////////////////////////////////////////////
// Mouse hook: record tick count

LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if ( nCode == HC_ACTION )
		theApp.m_nLastInput = (DWORD)time( NULL );

	return ::CallNextHookEx( theApp.m_hHookMouse, nCode, wParam, lParam );
}


/////////////////////////////////////////////////////////////////////////////
// Folder Path Methods for Windows Vista/7 or XP/2000

CString CEnvyApp::GetWindowsFolder() const
{
	CString strWindows;

	// Vista+
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		HRESULT hr = m_pfnSHGetKnownFolderPath( FOLDERID_Windows, KF_FLAG_CREATE|KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			strWindows = pPath;
			CoTaskMemFree( pPath );

			if ( SUCCEEDED( hr ) && ! strWindows.IsEmpty() )
				return strWindows;
		}
	}
#ifdef XPSUPPORT
	else	// XP
	{
		HRESULT hr = SHGetFolderPath( NULL, CSIDL_WINDOWS, NULL, NULL, strWindows.GetBuffer( MAX_PATH ) );
		strWindows.ReleaseBuffer();
		if ( SUCCEEDED( hr ) && ! strWindows.IsEmpty() )
			return strWindows;
	}
#endif

	// Legacy
	GetWindowsDirectory( strWindows.GetBuffer( MAX_PATH ), MAX_PATH );
	strWindows.ReleaseBuffer();
	return strWindows;
}

CString CEnvyApp::GetProgramFilesFolder64() const
{
	HRESULT hr;
	CString strProgramFiles;

	// 64-bit way
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		hr = m_pfnSHGetKnownFolderPath( FOLDERID_ProgramFilesX64, KF_FLAG_DONT_VERIFY, NULL, &pPath );
		if ( pPath )
		{
			strProgramFiles = pPath;
			CoTaskMemFree( pPath );
		}
		if ( SUCCEEDED( hr ) && ! strProgramFiles.IsEmpty() )
			return strProgramFiles;
	}

	// 32-bit way
	ExpandEnvironmentStrings( L"%ProgramW6432%", strProgramFiles.GetBuffer( MAX_PATH ), MAX_PATH );
	strProgramFiles.ReleaseBuffer();
	strProgramFiles.Trim();
	strProgramFiles.TrimRight( L"\\" );
	if ( ! strProgramFiles.IsEmpty() )
		return strProgramFiles;

	return GetProgramFilesFolder();
}

CString CEnvyApp::GetProgramFilesFolder() const
{
	static CString strProgramFiles;

	if ( ! strProgramFiles.IsEmpty() )
		return strProgramFiles;

	// Note: Takes several seconds!

	// Vista+
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		HRESULT hr = m_pfnSHGetKnownFolderPath( FOLDERID_ProgramFilesX86, KF_FLAG_DONT_VERIFY, NULL, &pPath );
		if ( pPath )
		{
			strProgramFiles = pPath;
			CoTaskMemFree( pPath );

			if ( SUCCEEDED( hr ) && ! strProgramFiles.IsEmpty() )
				return strProgramFiles;
		}
	}
#ifdef XPSUPPORT
	else	// XP
	{
		HRESULT hr = SHGetFolderPath( NULL, CSIDL_PROGRAM_FILES, NULL, NULL, strProgramFiles.GetBuffer( MAX_PATH ) );
		strProgramFiles.ReleaseBuffer();
		if ( SUCCEEDED( hr ) && ! strProgramFiles.IsEmpty() )
			return strProgramFiles;
	}
#endif

	// Legacy
	strProgramFiles = GetWindowsFolder().Left( 1 ) + L":\\Program Files";

	return strProgramFiles;
}

CString CEnvyApp::GetDocumentsFolder() const
{
	CString strDocuments;

	// Vista+
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		HRESULT hr = m_pfnSHGetKnownFolderPath( FOLDERID_Documents, KF_FLAG_CREATE|KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			strDocuments = pPath;
			CoTaskMemFree( pPath );

			if ( SUCCEEDED( hr ) && ! strDocuments.IsEmpty() )
				return strDocuments;
		}
	}
#ifdef XPSUPPORT
	else	// XP
	{
		HRESULT hr = SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, NULL, strDocuments.GetBuffer( MAX_PATH ) );
		strDocuments.ReleaseBuffer();
		if ( SUCCEEDED( hr ) && ! strDocuments.IsEmpty() )
			return strDocuments;
	}
#endif

	// Legacy
	strDocuments = CRegistry::GetString( L"Shell Folders", L"Personal",
		L"", L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" );

	return strDocuments;
}

CString CEnvyApp::GetDownloadsFolder() const
{
	HRESULT hr;
	CString strDownloads;

	// Vista+
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		hr = m_pfnSHGetKnownFolderPath( FOLDERID_Downloads, KF_FLAG_CREATE|KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			strDownloads = pPath;
			CoTaskMemFree( pPath );

			if ( SUCCEEDED( hr ) && ! strDownloads.IsEmpty() )
				return strDownloads + L"\\Envy";	// CLIENT_NAME
		}
	}

	// Legacy (do not use...)
	strDownloads = CRegistry::GetString( L"Shell Folders", L"{374DE290-123F-4565-9164-39C4925E467B}",
		L"", L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" );

	// XP/Legacy
	if ( strDownloads.IsEmpty() )
		strDownloads = GetDocumentsFolder() + L"\\Envy Downloads";
	else
		strDownloads = strDownloads + L"\\Envy";

	return strDownloads;
}

CString CEnvyApp::GetAppDataFolder() const
{
	CString strAppData;

	// Vista+
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		HRESULT hr = m_pfnSHGetKnownFolderPath( FOLDERID_RoamingAppData, KF_FLAG_CREATE|KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			strAppData = pPath;
			CoTaskMemFree( pPath );

			if ( SUCCEEDED( hr ) && ! strAppData.IsEmpty() )
				return strAppData;
		}
	}
#ifdef XPSUPPORT
	else	// XP
	{
		HRESULT hr = SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, NULL, strAppData.GetBuffer( MAX_PATH ) );
		strAppData.ReleaseBuffer();
		if ( SUCCEEDED( hr ) && ! strAppData.IsEmpty() )
			return strAppData;
	}
#endif

	// Legacy
	strAppData = CRegistry::GetString( L"Shell Folders", L"AppData",
		L"", L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" );

	return strAppData;
}

CString CEnvyApp::GetLocalAppDataFolder() const
{
	CString strLocalAppData;

	// Vista+
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		HRESULT hr = m_pfnSHGetKnownFolderPath( FOLDERID_LocalAppData, KF_FLAG_CREATE|KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			strLocalAppData = pPath;
			CoTaskMemFree( pPath );

			if ( SUCCEEDED( hr ) && ! strLocalAppData.IsEmpty() )
				return strLocalAppData;
		}
	}
#ifdef XPSUPPORT
	else	// XP
	{
		HRESULT hr = SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, NULL, strLocalAppData.GetBuffer( MAX_PATH ) );
		strLocalAppData.ReleaseBuffer();
		if ( SUCCEEDED( hr ) && ! strLocalAppData.IsEmpty() )
			return strLocalAppData;
	}
#endif

	// Legacy
	strLocalAppData = CRegistry::GetString( L"Shell Folders", L"Local AppData",
		L"", L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" );
	if ( ! strLocalAppData.IsEmpty() )
		return strLocalAppData;

	// Failsafe
	return GetAppDataFolder();
}

void CEnvyApp::OnRename(LPCTSTR pszSource, LPCTSTR pszTarget)
{
	LibraryBuilder.Remove( pszSource );

	Uploads.OnRename( pszSource, pszTarget );

	Downloads.OnRename( pszSource, pszTarget );

	// Notify built-in Mediaplayer
	if ( pszTarget == NULL )
	{
		if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
		{
			if ( CMediaWnd* pMediaWnd = (CMediaWnd*)pMainWnd->m_pWindows.Find( RUNTIME_CLASS( CMediaWnd ) ) )
			{
				CQuickLock otheAppLock( theApp.m_pSection );
				pMediaWnd->OnFileDelete( pszSource );
			}
		}
	}
}

CDatabase* CEnvyApp::GetDatabase(int nType /*0*/) const
{
	ASSERT( nType < DB_LAST );

	// Legacy v1.0 and earlier update (ToDo: Remove after v2.0)
	static BOOL bCheck = TRUE;
	if ( bCheck && PathFileExists( Settings.General.DataPath + L"Thumbnails.db3" ) )
	{
		::MoveFile( Settings.General.DataPath + L"Thumbnails.db3", Settings.General.DataPath + L"Thumbnails.db" );
		bCheck = FALSE;
	}

	return new CDatabase( Settings.General.DataPath +
		( nType == DB_THUMBS ? L"Thumbnails.db" :
		  nType == DB_SECURITY ? L"Security.db" :
		  nType == DB_BLACKLIST ? L"Blacklist.db" :
		/*nType == DB_DEFAULT ?*/ L"Envy.db" ) );
}

BOOL CEnvyApp::GetPropertyStoreFromParsingName(LPCWSTR pszPath, IPropertyStore**ppv)
{
	if ( m_pfnSHGetPropertyStoreFromParsingName )
	{
		__try
		{
			return SUCCEEDED( m_pfnSHGetPropertyStoreFromParsingName( pszPath, NULL, GPS_BESTEFFORT, __uuidof( IPropertyStore ), (void**)ppv ) );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}
	return FALSE;
}


#undef SafeLock

BOOL SafeLock(CSingleLock& pLock, LPCTSTR pszDebug, LPCTSTR /*pszUnused*/)
{
	//static LPCTSTR pszLast;	ToDo: Better Tracking?
	if ( pLock.IsLocked() )
	{
		pLock.Unlock();
		Sleep( 0 );
	}
	if ( pLock.Lock( Settings.General.LockTimeout ) )
		return TRUE;
#ifdef _DEBUG
	theApp.Message( MSG_INFO|MSG_TRAY, IDS_LOCK_TIMEOUT, pszDebug );
#else
	theApp.Message( MSG_DEBUG, IDS_LOCK_TIMEOUT, pszDebug );
#endif
	return FALSE;
}

CString SafeFilename(CString strName, bool bPath)
{
	// Restore spaces
	strName.Replace( L"%20", L" " );

	// Replace incompatible symbols
	for ( ;; )
	{
		const int nChar = strName.FindOneOf(
			bPath ? L"/:*?<>|\"" : L"\\/:*?<>|\"" );

		if ( nChar == -1 )
			break;

		strName.SetAt( nChar, L'_' );
	}

	// Limit maximum filepath length (Obsolete)
	//LPCTSTR szExt = PathFindExtension( strName );
	//int nExtLen = lstrlen( szExt );
	//
	//int nMaxFilenameLength = MAX_PATH - 1 - max( max(
	//	Settings.Downloads.IncompletePath.GetLength(),
	//	Settings.Downloads.CompletePath.GetLength() ),
	//	Settings.Downloads.TorrentPath.GetLength() );
	//if ( strName.GetLength() > nMaxFilenameLength )
	//	strName = strName.Left( nMaxFilenameLength - nExtLen ) + strName.Right( nExtLen );

	// Note: Use SafePath() elsewhere to prepend "\\?\" for long paths

	return strName;
}

BOOL CreateDirectory(LPCTSTR szPath)
{
	CString strDir = SafePath( szPath );
	if ( strDir.GetLength() == 2 )	//&& strDir.GetAt( 1 ) == ':'
		strDir.AppendChar( L'\\' );	// Root Drive

	DWORD dwAttr = GetFileAttributes( strDir );

	if ( dwAttr != INVALID_FILE_ATTRIBUTES && ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
		return TRUE;

	for ( int nStart = 3 ; ; )
	{
		const int nSlash = strDir.Find( L'\\', nStart );
		if ( nSlash == -1 || nSlash == strDir.GetLength() - 1 )
			break;
		CString strSubDir = SafePath( strDir.Left( nSlash + 1 ) );
		dwAttr = GetFileAttributes( strSubDir );
		if ( ( dwAttr == INVALID_FILE_ATTRIBUTES ) || ! ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			if ( ! CreateDirectory( strSubDir, NULL ) )
				return FALSE;
		}
		nStart = nSlash + 1;
	}
	return CreateDirectory( strDir, NULL );
}

void DeleteFolders(CStringList& pList)
{
	// From WndDownloads torrents
	while ( ! pList.IsEmpty() )
	{
		const CString strPath = pList.RemoveHead();
		if ( PathIsDirectoryEmpty( strPath ) )
			RemoveDirectory( strPath );
	}
}

BOOL DeleteFiles(CStringList& pList)
{
	// From WndDownloads
	while ( ! pList.IsEmpty() )
	{
		const CString strFirstPath = pList.GetHead();

		CDeleteFileDlg dlg;
		dlg.m_bAll = ( pList.GetCount() > 1 );

		{
			CQuickLock pLibraryLock( Library.m_pSection );

			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strFirstPath ) )
			{
				dlg.m_sName = pFile->m_sName;
				dlg.m_sComments = pFile->m_sComments;
				dlg.m_nRateValue = pFile->m_nRating;
			}
			else
			{
				dlg.m_sName = PathFindFileName( strFirstPath );
			}
		}

		if ( dlg.DoModal() != IDOK )
			return FALSE;

		for ( INT_PTR nProcess = dlg.m_bAll ? pList.GetCount() : 1 ; nProcess > 0 && pList.GetCount() > 0 ; nProcess-- )
		{
			const CString strPath = pList.RemoveHead();

			{
				CQuickLock pTransfersLock( Transfers.m_pSection );	// Can clear uploads and downloads
				CQuickLock pLibraryLock( Library.m_pSection );

				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath ) )
				{
					// It's a library file
					dlg.Apply( pFile );
					pFile->Delete();
					continue;
				}
			}

			// It's a wild file
			const BOOL bRecycleBin = ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 );
			DeleteFileEx( strPath, TRUE, bRecycleBin, TRUE );
		}
	}

	// Note: Cleanup empty folders for multifile torrents

	return TRUE;
}

BOOL DeleteFileEx(LPCTSTR szFileName, BOOL bShared, BOOL bToRecycleBin, BOOL bEnableDelayed)
{
	// Should be double zeroed long path
	ASSERT( szFileName && *szFileName );
	CString strFileName = SafePath( szFileName );
	if ( strFileName.GetLength() < 8 ) return FALSE;
	const int nPrefix = strFileName[2] == L'?' ? 4 : 0;		// "//?/"
	DWORD len = GetLongPathName( strFileName, NULL, 0 );
	BOOL bLong = len ? TRUE : FALSE;
	if ( ! bLong )
		len = lstrlen( strFileName );

	CAutoVectorPtr< TCHAR > szPath( new TCHAR[ len + 1 ] );
	if ( ! szPath )
		return FALSE;

	if ( bLong )
		GetLongPathName( strFileName, szPath, len );
	else
		lstrcpy( szPath, strFileName );
	szPath[ len ] = 0;

	if ( bShared )	// Stop uploads
		theApp.OnRename( szPath + nPrefix, NULL );

	DWORD dwAttr = GetFileAttributes( szPath );
	if ( ( dwAttr != INVALID_FILE_ATTRIBUTES ) &&		// Filename exist
		 ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) == 0 )	// Not a folder
	{
		if ( bToRecycleBin )
		{
			SHFILEOPSTRUCT sfo = {};
			sfo.hwnd = GetDesktopWindow();
			sfo.wFunc = FO_DELETE;
			sfo.pFrom = szPath + nPrefix;
			sfo.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NORECURSION | FOF_NO_UI;
			SHFileOperation( &sfo );
		}
		else
		{
			DeleteFile( szPath );
		}

		dwAttr = GetFileAttributes( szPath );
		if ( dwAttr != INVALID_FILE_ATTRIBUTES )
		{
			// File still exists
			if ( bEnableDelayed )
			{
				// Set delayed deletion
				CString strJob;
				strJob.Format( L"%d%d", bShared, bToRecycleBin );
				theApp.WriteProfileString( L"Delete", szPath + nPrefix, strJob );
			}
			return FALSE;
		}
	}

	// Cancel delayed deletion (if any)
	theApp.WriteProfileString( L"Delete", szPath + nPrefix, NULL );

	return TRUE;
}

void PurgeDeletes()
{
	HKEY hKey = NULL;
	LSTATUS nResult = RegOpenKeyEx( HKEY_CURRENT_USER,
		REGISTRY_KEY L"\\Delete", 0, KEY_ALL_ACCESS, &hKey );
	if ( ERROR_SUCCESS == nResult )
	{
		CList< CString > pRemove;
		for ( DWORD nIndex = 0 ; ; ++nIndex )
		{
			DWORD nPath = MAX_PATH * 2;
			TCHAR szPath[ MAX_PATH * 2 ] = {};
			DWORD nType;
			DWORD nMode = 8;
			TCHAR szMode[ 8 ] = {};
			nResult = RegEnumValue( hKey, nIndex, szPath, &nPath, 0,
				&nType, (LPBYTE)szMode, &nMode );
			if ( ERROR_SUCCESS != nResult )
				break;

			BOOL bShared = ( nType == REG_SZ ) && ( szMode[ 0 ] == '1' );
			BOOL bToRecycleBin = ( nType == REG_SZ ) && ( szMode[ 1 ] == '1' );
			if ( DeleteFileEx( szPath, bShared, bToRecycleBin, TRUE ) )
				pRemove.AddTail( szPath );
		}

		while ( ! pRemove.IsEmpty() )
		{
			nResult = RegDeleteValue( hKey, pRemove.RemoveHead() );
		}

		nResult = RegCloseKey( hKey );
	}
}

// Direct Web Browsing Page (About.htm/etc.)
CString LoadHTML(HINSTANCE hInstance, UINT nResourceID)
{
	CString strBody;
	BOOL bGZIP = FALSE;

	HRSRC hRes = FindResource( hInstance, MAKEINTRESOURCE( nResourceID ), RT_HTML );

	if ( ! hRes )	// Try *.xml.gz
	{
		hRes = FindResource( hInstance, MAKEINTRESOURCE( nResourceID ), RT_GZIP );
		if ( ! hRes ) return strBody;
		bGZIP = TRUE;
	}

	const DWORD nSize = SizeofResource( hInstance, hRes );
	HGLOBAL hMemory = LoadResource( hInstance, hRes );
	if ( ! hMemory ) return strBody;

	LPCSTR pszInput = (LPCSTR)LockResource( hMemory );
	if ( ! pszInput ) return strBody;

	if ( bGZIP )
	{
		CBuffer buf;
		buf.Add( pszInput, nSize );
		if ( buf.Ungzip() )
		{
			int nWide = MultiByteToWideChar( 0, 0, (LPCSTR)buf.m_pBuffer, buf.m_nLength, NULL, 0 );
			LPTSTR pszOutput = strBody.GetBuffer( nWide + 1 );
			MultiByteToWideChar( 0, 0, (LPCSTR)buf.m_pBuffer, buf.m_nLength, pszOutput, nWide );
			pszOutput[ nWide ] = L'\0';
			strBody.ReleaseBuffer();
		}
	}
	else // .xml
	{
		int nWide = MultiByteToWideChar( 0, 0, pszInput, nSize, NULL, 0 );
		LPTSTR pszOutput = strBody.GetBuffer( nWide + 1 );
		MultiByteToWideChar( 0, 0, pszInput, nSize, pszOutput, nWide );
		pszOutput[ nWide ] = L'\0';
		strBody.ReleaseBuffer();
	}

	FreeResource( hMemory );

	return strBody;
}

CString LoadRichHTML(UINT nResourceID, CString& strResponse, CEnvyFile* pFile)
{
	CString strBody = LoadHTML( GetModuleHandle( NULL ), nResourceID );

	bool bWindowsEOL = true;
	int nBreak = strBody.Find( L"\r\n" );
	if ( nBreak == -1 )
	{
		nBreak = strBody.Find( L"\n" );
		bWindowsEOL = false;
	}
	strResponse	= strBody.Left( nBreak + ( bWindowsEOL ? 2 : 1 ) );
	strBody		= strBody.Mid(  nBreak + ( bWindowsEOL ? 2 : 1 ) );

	for ( ;; )
	{
		int nStart = strBody.Find( L"<%" );
		if ( nStart < 0 ) break;

		int nEnd = strBody.Find( L"%>" );
		if ( nEnd < nStart ) break;

		CString strReplace = strBody.Mid( nStart + 2, nEnd - nStart - 2 );

		strReplace.TrimLeft();
		strReplace.TrimRight();

		if ( strReplace.CompareNoCase( L"Client" ) == 0 )
			strReplace = CLIENT_NAME;
		else if ( strReplace.CompareNoCase( L"SmartAgent" ) == 0 )
			strReplace = theApp.m_sSmartAgent;
		else if ( strReplace.CompareNoCase( L"Name" ) == 0 )
			strReplace = pFile ? pFile->m_sName : L"";
		else if ( strReplace.CompareNoCase( L"SHA1" ) == 0 )
			strReplace = pFile ? pFile->m_oSHA1.toString() : L"";
		else if ( strReplace.CompareNoCase( L"URN" ) == 0 )
			strReplace = pFile ? pFile->m_oSHA1.toUrn() : L"";
		else if ( strReplace.CompareNoCase( L"Version" ) == 0 )
			strReplace = theApp.m_sVersion;
		else if ( strReplace.Find( L"Neighbours" ) == 0 )
			strReplace = Neighbours.GetNeighbourList( strReplace.Right( strReplace.GetLength() - 11 ) );
		else if ( strReplace.CompareNoCase( L"ListenIP" ) == 0 )
		{
			if ( Network.IsListening() )
			{
				strReplace.Format( L"%s:%i",
					(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
					htons( Network.m_pHost.sin_port ) );
			}
			else
				strReplace.Empty();
		}

		strBody = strBody.Left( nStart ) + strReplace + strBody.Mid( nEnd + 2 );
	}

	return strBody;
}

// Remote web browsing virtual resources, used below only
const struct
{
	LPCTSTR szPath;
	UINT	nID;
	LPCTSTR szType;
	LPCTSTR szContentType;
} WebResources [] =
{
	{ L"/remote/header.png",		IDR_HOME_HEADER,		RT_PNG,			L"image/png" },
	{ L"/remote/header_repeat.png",	IDR_HOME_HEADER_REPEAT,	RT_PNG,			L"image/png" },
	{ L"/favicon.ico",				IDI_FAVICON,			RT_GROUP_ICON,	L"image/x-icon" },
	{ NULL, NULL, NULL, NULL }
};

bool ResourceRequest(const CString& strPath, CBuffer& pResponse, CString& sHeader)
{
	bool ret = false;

	for ( int i = 0 ; WebResources[ i ].szPath ; i++ )
	{
		if ( strPath.Compare( WebResources[ i ].szPath ) != 0 )
			continue;

		HMODULE hModule = AfxGetResourceHandle();
		if ( HRSRC hRes = FindResource( hModule,
			MAKEINTRESOURCE( WebResources[ i ].nID ), WebResources[ i ].szType ) )
		{
			if ( HGLOBAL hMemory = LoadResource( hModule, hRes ) )
			{
				DWORD nSize = SizeofResource( hModule, hRes );
				if ( BYTE* pSource = (BYTE*)LockResource( hMemory ) )
				{
					if ( WebResources[ i ].szType == RT_GROUP_ICON )
					{
						// Save main header
						ICONDIR* piDir = (ICONDIR*)pSource;
						DWORD dwTotalSize = sizeof( ICONDIR ) + sizeof( ICONDIRENTRY ) * piDir->idCount;
						pResponse.EnsureBuffer( dwTotalSize );
						CopyMemory( pResponse.m_pBuffer, piDir, sizeof( ICONDIR ) );

						GRPICONDIRENTRY* piDirEntry = (GRPICONDIRENTRY*)( pSource + sizeof( ICONDIR ) );

						// Find all subicons
						for ( WORD j = 0 ; j < piDir->idCount ; j++ )
						{
							// pResponse.m_pBuffer may be changed
							ICONDIRENTRY* piEntry = (ICONDIRENTRY*)( pResponse.m_pBuffer + sizeof( ICONDIR ) );

							// Load subicon
							if ( HRSRC hResIcon = FindResource( hModule, MAKEINTRESOURCE( piDirEntry[ j ].nID ), RT_ICON ) )
							{
								if ( HGLOBAL hMemoryIcon = LoadResource( hModule, hResIcon ) )
								{
									DWORD nSizeIcon = SizeofResource( hModule, hResIcon );

									BITMAPINFOHEADER* piImage = (BITMAPINFOHEADER*)LockResource( hMemoryIcon );

									// Fill subicon header
									piEntry[ j ].bWidth = piDirEntry[ j ].bWidth;
									piEntry[ j ].bHeight = piDirEntry[ j ].bHeight;
									piEntry[ j ].wPlanes = piDirEntry[ j ].wPlanes;
									piEntry[ j ].bColorCount = piDirEntry[ j ].bColorCount;
									piEntry[ j ].bReserved = 0;
									piEntry[ j ].wBitCount = piDirEntry[ j ].wBitCount;
									piEntry[ j ].dwBytesInRes = nSizeIcon;
									piEntry[ j ].dwImageOffset = dwTotalSize;

									// Save subicon
									pResponse.EnsureBuffer( dwTotalSize + nSizeIcon );
									CopyMemory( pResponse.m_pBuffer + dwTotalSize, piImage, nSizeIcon );
									dwTotalSize += nSizeIcon;

									FreeResource( hMemoryIcon );
								}
							}
						}
						pResponse.m_nLength = dwTotalSize;
					}
					else
					{
						pResponse.EnsureBuffer( nSize );
						CopyMemory( pResponse.m_pBuffer, pSource, nSize );
						pResponse.m_nLength = nSize;
					}

					sHeader.Format( L"Content-Type: %s\r\n", WebResources[ i ].szContentType );
					ret = true;
				}
				FreeResource( hMemory );
			}
		}
		break;
	}

	return ret;
}

BOOL SaveIcon(HICON hIcon, CBuffer& oBuffer, int colors)
{
	ASSERT( hIcon );
	ASSERT( colors == -1 || colors == 1 || colors == 4 || colors == 8 || colors == 16 || colors == 24 || colors == 32 );

	ICONINFO ii = {};
	if ( ! GetIconInfo( hIcon, &ii ) )
		return FALSE;

	BITMAP biColor = {};
	if ( GetObject( ii.hbmColor, sizeof( BITMAP ), &biColor ) != sizeof( BITMAP ) )
		return FALSE;

	int cx = biColor.bmWidth;
	if ( colors == -1 )
		colors = biColor.bmBitsPixel;
	int palette = ( colors == 8 ) ? 256 : ( ( colors == 4 ) ? 16 : ( ( colors == 1 ) ? 2 : 0 ) );

	CAutoVectorPtr< char >pHeader( new char[ sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * 256 ] );
	if ( ! pHeader )
		return FALSE;	// Out of memory

	ZeroMemory( (char*)pHeader, sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * 256 );
	BITMAPINFO& bih = *(BITMAPINFO*)(char*)pHeader;
	bih.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bih.bmiHeader.biWidth = bih.bmiHeader.biHeight = cx;
	bih.bmiHeader.biPlanes = 1;

	HDC hDC = GetDC( NULL );
	if ( ! hDC )
		return FALSE;

	// Calculate mask size
	bih.bmiHeader.biBitCount = 1;
	if ( ! GetDIBits( hDC, ii.hbmMask, 0, cx, NULL, &bih, DIB_RGB_COLORS ) )
		return FALSE;
	DWORD nImageSize = bih.bmiHeader.biSizeImage;

	// Calculate image size
	bih.bmiHeader.biBitCount = (WORD)colors;
	if ( ! GetDIBits( hDC, ii.hbmColor, 0, cx, NULL, &bih, DIB_RGB_COLORS ) )
		return FALSE;
	nImageSize += bih.bmiHeader.biSizeImage;

	CAutoVectorPtr< char >pBuffer( new char[ nImageSize ] );
	if ( ! pBuffer )
		return FALSE;	// Out of memory
	ZeroMemory( (char*)pBuffer, nImageSize );

	// Get mask bits
	bih.bmiHeader.biBitCount = 1;
	if ( GetDIBits( hDC, ii.hbmMask, 0, cx, (char*)pBuffer + bih.bmiHeader.biSizeImage, &bih, DIB_RGB_COLORS ) != cx )
		return FALSE;

	// Get image bits
	bih.bmiHeader.biBitCount = (WORD)colors;
	if ( GetDIBits( hDC, ii.hbmColor, 0, cx, (char*)pBuffer, &bih, DIB_RGB_COLORS ) != cx )
		return FALSE;

	VERIFY( ReleaseDC( NULL, hDC ) == 1 );

	// Fill icon file header
	bih.bmiHeader.biHeight = cx * 2;
	bih.bmiHeader.biSizeImage = nImageSize;
	bih.bmiHeader.biClrUsed = bih.bmiHeader.biClrImportant = palette;
	ICONDIR id =
	{
		0,
		1,
		1
	};
	ICONDIRENTRY ide =
	{
		(BYTE)( ( cx < 256 ) ? cx : 0 ),
		(BYTE)( ( cx < 256 ) ? cx : 0 ),
		(BYTE)( ( colors < 8 ) ? ( 1 << colors ) : 0 ),
		0,
		1,
		(WORD)colors,
		sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * palette + nImageSize,
		sizeof( ICONDIR ) + sizeof( ICONDIRENTRY )
	};

	oBuffer.Add( &id, sizeof( ICONDIR ) );
	oBuffer.Add( &ide, sizeof( ICONDIRENTRY ) );
	oBuffer.Add( &bih, sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * palette );
	oBuffer.Add( pBuffer, nImageSize );

	return TRUE;
}

bool MarkFileAsDownload(const CString& sFilename)
{
	if ( ! Settings.Library.MarkFileAsDownload )
		return false;

	LPCTSTR pszExt = PathFindExtension( (LPCTSTR)sFilename );
	if ( ! pszExt ) return false;

	CString strFilename = SafePath( sFilename );

	// ToDo: pFile->m_bVerify and IDS_LIBRARY_VERIFY_FIX warning features
	// could be merged with this function, because they resemble the security warning.
	// Shouldn't we unblock files from the application without forcing user to do that manually?
	if ( CFileExecutor::IsSafeExecute( pszExt ) )
		return false;

	// Temporary clear R/O attribute
	BOOL bChanged = FALSE;
	DWORD dwOrigAttr = GetFileAttributes( strFilename );
	if ( dwOrigAttr != INVALID_FILE_ATTRIBUTES && ( dwOrigAttr & FILE_ATTRIBUTE_READONLY ) )
		bChanged = SetFileAttributes( strFilename, dwOrigAttr & ~FILE_ATTRIBUTE_READONLY );

	HANDLE hStream = CreateFile( strFilename + L":Zone.Identifier",
		GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hStream == INVALID_HANDLE_VALUE )
	{
		HANDLE hFile = CreateFile( strFilename,
			GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL );

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			hStream = CreateFile( strFilename + L":Zone.Identifier",
				GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			CloseHandle( hFile );
		}
	}

	bool bSuccess = false;

	if ( hStream != INVALID_HANDLE_VALUE )
	{
		DWORD dwWritten = 0;
		bSuccess = ( WriteFile( hStream, "[ZoneTransfer]\r\nZoneID=3\r\n", 26, &dwWritten, NULL )
			&& dwWritten == 26 );
		CloseHandle( hStream );
	}
	else
	{
		TRACE( "MarkFileAsDownload() : CreateFile \"%s\" error %d\n", (LPCSTR)CT2A( strFilename ), GetLastError() );
	}

	if ( bChanged )
		SetFileAttributes( strFilename, dwOrigAttr );

	return bSuccess;
}

bool LoadGUID(const CString& sFilename, Hashes::Guid& oGUID)
{
	if ( ! Settings.Library.UseFolderGUID )
		return false;

	bool bSuccess = false;

	HANDLE hFile = CreateFile( SafePath( sFilename + L":Envy.GUID" ),
		GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		Hashes::Guid oTmpGUID;
		DWORD dwReaded = 0;
		bSuccess = ( ReadFile( hFile, &*oTmpGUID.begin(), oTmpGUID.byteCount, &dwReaded, NULL )
			&& dwReaded == oTmpGUID.byteCount );
		if ( bSuccess )
		{
			oTmpGUID.validate();
			oGUID = oTmpGUID;
		}
		CloseHandle( hFile );
	}

	return bSuccess;
}

bool SaveGUID(const CString& sFilename, const Hashes::Guid& oGUID)
{
	if ( ! Settings.Library.UseFolderGUID )
		return false;

	CString strFilename = SafePath( sFilename );

	Hashes::Guid oCurrentGUID;
	if ( LoadGUID( strFilename, oCurrentGUID ) && oCurrentGUID == oGUID )
		return true;

	// Temporary clear R/O attribute
	BOOL bChanged = FALSE;
	DWORD dwOrigAttr = GetFileAttributes( strFilename );
	if ( dwOrigAttr != 0xffffffff && ( dwOrigAttr & FILE_ATTRIBUTE_READONLY ) )
		bChanged = SetFileAttributes( strFilename, dwOrigAttr & ~FILE_ATTRIBUTE_READONLY );

	HANDLE hStream = CreateFile( strFilename + L":Envy.GUID",
		GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hStream == INVALID_HANDLE_VALUE )
	{
		HANDLE hFile = CreateFile( strFilename,
			GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL );

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			hStream = CreateFile( strFilename + L":Envy.GUID",
				GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			CloseHandle( hFile );
		}
	}

	bool bSuccess = false;

	if ( hStream != INVALID_HANDLE_VALUE )
	{
		DWORD dwWritten = 0;
		bSuccess = ( WriteFile( hStream, &*oGUID.begin(), oGUID.byteCount, &dwWritten, NULL ) &&
			dwWritten == oGUID.byteCount );
		CloseHandle( hStream );
	}
	//else
	//	TRACE( "SaveGUID() : CreateFile \"%s\" error %d\n", strFilename, GetLastError() );

	if ( bChanged )
		SetFileAttributes( strFilename, dwOrigAttr );

	return bSuccess;
}

CString ResolveShortcut(LPCTSTR lpszFileName)
{
	CComPtr< IShellLink > pIShellLink;
	if ( SUCCEEDED( pIShellLink.CoCreateInstance( CLSID_ShellLink ) ) )
	{
		CComPtr< IPersistFile > pIPersistFile;
		pIPersistFile = pIShellLink;
		if ( pIPersistFile &&
			SUCCEEDED( pIPersistFile->Load( CComBSTR( lpszFileName ), STGM_READ ) ) &&
			SUCCEEDED( pIShellLink->Resolve( AfxGetMainWnd()->GetSafeHwnd(),
			SLR_NO_UI | SLR_NOUPDATE | SLR_NOSEARCH | SLR_NOTRACK | SLR_NOLINKINFO ) ) )
		{
			CString strPath;
			BOOL bResult = SUCCEEDED( pIShellLink->GetPath( strPath.GetBuffer( MAX_PATH ), MAX_PATH, NULL, 0 ) );
			strPath.ReleaseBuffer();
			if ( bResult )
				return strPath;
		}
	}
	return CString();
}

// BrowseCallbackProc - BrowseForFolder callback function
static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch ( uMsg )
	{
	case BFFM_INITIALIZED:
		{
			// Remove context help button from dialog caption
			SetWindowLongPtr( hWnd, GWL_STYLE,
				GetWindowLongPtr( hWnd, GWL_STYLE ) & ~DS_CONTEXTHELP );
			SetWindowLongPtr( hWnd, GWL_EXSTYLE,
				GetWindowLongPtr( hWnd, GWL_EXSTYLE ) & ~WS_EX_CONTEXTHELP );

			// Set initial directory
			SendMessage( hWnd, BFFM_SETSELECTION, TRUE, lpData );
		}
		break;

	case BFFM_SELCHANGED:
		{
			// Fail if non-filesystem
			TCHAR szDir[ MAX_PATH ] = {};
			BOOL bResult = SHGetPathFromIDList( (LPITEMIDLIST)lParam, szDir );
			if ( bResult )
			{
				// Fail if folder not accessible
				bResult = ( _taccess( szDir, 4 ) == 0 );
				if ( bResult )
				{
					// Fail if pidl is a link
					SHFILEINFO sfi = {};
					bResult = ( SHGetFileInfo( (LPCTSTR)lParam, 0, &sfi, sizeof( sfi ),
						SHGFI_PIDL | SHGFI_ATTRIBUTES ) &&
						( sfi.dwAttributes & SFGAO_LINK ) == 0 );
				}
			}
			SendMessage( hWnd, BFFM_ENABLEOK, 0, bResult );
		}
		break;
	}
	return 0;
}

// Displays a dialog box enabling the user to select a Shell folder
CString BrowseForFolder(UINT nTitle, LPCTSTR szInitialPath, HWND hWnd)
{
	return BrowseForFolder( LoadString( nTitle ), szInitialPath, hWnd );
}

// Displays a dialog box enabling the user to select a Shell folder
CString BrowseForFolder(LPCTSTR szTitle, LPCTSTR szInitialPath, HWND hWnd)
{
	// Get last used folder
	static TCHAR szDefaultPath[ MAX_PATH ] = {};
	if ( ! szInitialPath || ! *szInitialPath )
	{
		if ( ! *szDefaultPath )
			_tcsncpy_s( szDefaultPath, MAX_PATH, (LPCTSTR)theApp.GetDocumentsFolder(), MAX_PATH - 1 );
		szInitialPath = szDefaultPath;
	}

	TCHAR szDisplayName[ MAX_PATH ] = {};
	BROWSEINFO pBI = {};
	pBI.hwndOwner = hWnd ? hWnd : AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName = szDisplayName;
	pBI.lpszTitle = szTitle;
	pBI.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
	pBI.lpfn = BrowseCallbackProc;
	pBI.lParam = (LPARAM)szInitialPath;
	LPITEMIDLIST pPath = SHBrowseForFolder( &pBI );
	if ( pPath == NULL )
		return CString();

	TCHAR szPath[ MAX_PATH ] = {};
	BOOL bResult = SHGetPathFromIDList( pPath, szPath );

	CComPtr< IMalloc > pMalloc;
	if ( SUCCEEDED( SHGetMalloc( &pMalloc ) ) )
		pMalloc->Free( pPath );

	if ( ! bResult )
		return CString();

	// Save last used folder
	_tcsncpy_s( szDefaultPath, MAX_PATH, szPath, MAX_PATH - 1 );

	return CString( szPath );
}

BOOL PostMainWndMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if ( CMainWnd* pWnd = theApp.SafeMainWnd() )
		return pWnd->PostMessage( Msg, wParam, lParam );

	return FALSE;
}

void SafeMessageLoop()
{
	InterlockedIncrement( &theApp.m_bBusy );

	__try
	{
		MSG msg;
		while ( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		if ( GetWindowThreadProcessId( AfxGetMainWnd()->GetSafeHwnd(), NULL ) == GetCurrentThreadId() )
		{
			LONG lIdle = 0;
			while ( AfxGetApp()->OnIdle( lIdle++ ) );
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}

	InterlockedDecrement( &theApp.m_bBusy );
}

BOOL IsServiceHealthy(LPCTSTR szService)
{
	BOOL bResult = TRUE;	// Ok or unknown state

	// Open a handle to the Service Control Manager database
	SC_HANDLE schSCManager = OpenSCManager(
		NULL,				// Local machine
		NULL,				// ServicesActive database
		GENERIC_READ );		// Enumeration and status lookup
	if ( schSCManager )
	{
		SC_HANDLE schServiceRead = OpenService( schSCManager, szService, GENERIC_READ );
		if ( schServiceRead )
		{
			SERVICE_STATUS_PROCESS ssStatus = {};
			DWORD nBytesNeeded = 0;
			if ( QueryServiceStatusEx( schServiceRead, SC_STATUS_PROCESS_INFO,
				(LPBYTE)&ssStatus, sizeof( SERVICE_STATUS_PROCESS ), &nBytesNeeded ) )
			{
				bResult = ( ssStatus.dwCurrentState == SERVICE_RUNNING );
			}
			CloseServiceHandle( schServiceRead );

			if ( ! bResult )
			{
				SC_HANDLE schServiceStart = OpenService( schSCManager, szService, SERVICE_START );
				if ( schServiceStart )
				{
					// Power users have only right to start service, thus try to start it here
					bResult = StartService( schServiceStart, 0, NULL );

					CloseServiceHandle( schServiceStart );
				}
			}
		}
		CloseServiceHandle( schSCManager );
	}

	return bResult;
}

BOOL IsUserFullscreen()
{
	// Detect system message availability

	// Vista+ (XP-safe: SHQueryUserNotificationState breaks WinXP)
	if ( theApp.m_pfnSHQueryUserNotificationState )
	{
		QUERY_USER_NOTIFICATION_STATE state;
		if ( theApp.m_pfnSHQueryUserNotificationState( &state ) == S_OK )
			return state != QUNS_ACCEPTS_NOTIFICATIONS;
	}

	// XP external fullscreen
	const HWND hActive = GetForegroundWindow();
	if ( hActive && hActive != AfxGetMainWnd()->GetSafeHwnd() && ( GetWindowLong( hActive, GWL_EXSTYLE ) & WS_EX_TOPMOST ) )
	{
		RECT rcWindow;
		GetWindowRect( hActive, &rcWindow );
		return rcWindow.top == 0 && rcWindow.left == 0 &&
			rcWindow.right  == GetSystemMetrics(SM_CXSCREEN) &&
			rcWindow.bottom == GetSystemMetrics(SM_CYSCREEN);
	}

	return FALSE;
}

// Unused, ToDo: r9146 Win7 jumplists
IShellLink* CreateShellLink(LPCWSTR szTargetExecutablePath, LPCWSTR szCommandLineArgs, LPCWSTR szTitle, LPCWSTR szIconPath, int nIconIndex, LPCWSTR szDescription)
{
	CComPtr< IShellLink > pLink;
	if ( SUCCEEDED( pLink.CoCreateInstance( CLSID_ShellLink ) ) )
	{
		pLink->SetPath( szTargetExecutablePath );
		pLink->SetArguments( szCommandLineArgs );
		pLink->SetIconLocation( szIconPath, nIconIndex );
		pLink->SetDescription( szDescription );

#ifdef _INC_PROPKEY		// <propkey.h> <propvarutil.h> Req. WinSDK 7.0+ XPsp2 (for InitPropVariantFromString + PKEY_Title)
		CComQIPtr< IPropertyStore > pProp( pLink );
		if ( pProp )
		{
			PROPVARIANT var;
			if ( SUCCEEDED( InitPropVariantFromString( szTitle, &var ) ) )
			{
				if ( SUCCEEDED( pProp->SetValue( PKEY_Title, var ) ) )
					pProp->Commit();
				PropVariantClear( &var );
			}
		}
#else
	UNUSED_ALWAYS( szTitle );
#endif
	}
	return pLink.Detach();
}

void ClearSkins()
{
	// Commandline "-noskin" registry reset
	CRegistry::DeleteKey( HKEY_CURRENT_USER, REGISTRY_KEY L"\\Skins" );
	CRegistry::DeleteKey( HKEY_CURRENT_USER, REGISTRY_KEY L"\\Settings" );
	CRegistry::DeleteKey( HKEY_CURRENT_USER, REGISTRY_KEY L"\\Toolbars" );
	CRegistry::DeleteKey( HKEY_CURRENT_USER, REGISTRY_KEY L"\\Windows" );
	CRegistry::DeleteKey( HKEY_CURRENT_USER, REGISTRY_KEY L"\\ListStates" );

	// Reskin if not startup
	//PostMainWndMessage( WM_SKINCHANGED );
}

// AfxMessageBox() Replacement:  (DlgMessage)
INT_PTR MsgBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp, DWORD* pnDefault, DWORD nTimer)
{
	CMessageDlg dlg;
	dlg.m_nType = nType;
	dlg.m_nIDHelp = nIDHelp;
	dlg.m_sText = lpszText;
	dlg.m_pnDefault = pnDefault;
	dlg.m_nTimer = nTimer;
	return dlg.DoModal();
}

INT_PTR MsgBox(UINT nIDPrompt, UINT nType, UINT nIDHelp, DWORD* pnDefault, DWORD nTimer)
{
	return MsgBox( (LPCTSTR)LoadString( nIDPrompt ), nType, nIDHelp, pnDefault, nTimer );
}


/////////////////////////////////////////////////////////////////////////////
// CProgressDialog

CProgressDialog::CProgressDialog(LPCTSTR szTitle, DWORD dwFlags)
{
	if ( SUCCEEDED( CoCreateInstance( CLSID_ProgressDialog ) ) )
	{
		p->SetTitle( CLIENT_NAME );
		p->SetLine( 1, szTitle, FALSE, NULL );
		p->StartProgressDialog( theApp.SafeMainWnd() ? theApp.SafeMainWnd()->GetSafeHwnd() : GetDesktopWindow(), NULL, dwFlags, NULL );
	}
}

CProgressDialog::~CProgressDialog()
{
	if ( p )
		p->StopProgressDialog();
}

void CProgressDialog::Progress(LPCTSTR szText, QWORD nCompleted, QWORD nTotal)
{
	if ( p )
	{
		p->SetLine( 2, szText, TRUE, NULL );
		if ( nTotal || nCompleted )
			p->SetProgress64( nCompleted, nTotal );
	}
}


template <>
__int8 GetRandomNum<__int8>(const __int8& min, const __int8& max)
{
	return (__int8)GetRandomNum<unsigned __int8>( min, max );
}

template <>
__int16 GetRandomNum<__int16>(const __int16& min, const __int16& max)
{
	return (__int16)GetRandomNum<unsigned __int16>( min, max );
}

template <>
__int32 GetRandomNum<__int32>(const __int32& min, const __int32& max)
{
	return (__int32)GetRandomNum<unsigned __int32>( min, max );
}

template <>
__int64 GetRandomNum<__int64>(const __int64& min, const __int64& max)
{
	return (__int64)GetRandomNum<unsigned __int64>( min, max );
}
