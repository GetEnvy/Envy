//
// TorrentEnvy.cpp
//
// This file is part of Torrent Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2007
//
// Envy is free software; you can redistribute it
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
#include "TorrentEnvy.h"
#include "WizardSheet.h"
#include "CmdLine.h"

#include "PageWelcome.h"
#include "PageExpert.h"
#include "PageSingle.h"
#include "PagePackage.h"
#include "PageTracker.h"
#include "PageComment.h"
#include "PageOutput.h"
#include "PageFinished.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CTorrentEnvyApp, CWinApp)
	ON_COMMAND(ID_HELP, CTorrentEnvyApp::OnHelp)
END_MESSAGE_MAP()

CTorrentEnvyApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CTorrentEnvyApp construction

CTorrentEnvyApp::CTorrentEnvyApp()
{
}

void CTorrentEnvyApp::OnHelp()
{
//	CWinApp::OnHelp();
	ShellExecute( NULL, NULL, L"http://getenvy.com/wiki/TorrentEnvy/",
	//	+ static_cast< CWizardPage* >( m_pSheet->GetActivePage() )->GetWindowText(),
		NULL, NULL, SW_SHOWNORMAL );
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentEnvyApp initialization

BOOL CTorrentEnvyApp::InitInstance()
{
	CCommandLineInfoEx cmdInfo;
	ParseCommandLine( cmdInfo );

	cmdInfo.GetOption( L"sourcefile", m_sCommandLineSourceFile );
	cmdInfo.GetOption( L"destination", m_sCommandLineDestination );
	cmdInfo.GetOption( L"tracker", m_sCommandLineTracker );
	cmdInfo.GetOption( L"comment", m_sCommandLineComment );

	if ( ! m_sCommandLineSourceFile.IsEmpty() &&
		 ! m_sCommandLineDestination.IsEmpty() &&
		 ! m_sCommandLineTracker.IsEmpty() )
	{
		if ( m_sCommandLineComment.IsEmpty() )
			m_sCommandLineComment = L"http://getenvy.com/";
	}
	else
	{
		// Test prior app instance for non-commandline
		HANDLE pMutex = CreateMutex( NULL, FALSE, L"Global\\TorrentEnvy" );
		if ( GetLastError() == ERROR_ALREADY_EXISTS )
		{
			// Show first instance
			//if ( CWnd* pWnd = CWnd::FindWindow( L"TorrentEnvy", NULL ) )
			//{
			//	pWnd->SendMessage( WM_SYSCOMMAND, SC_RESTORE );
			//	pWnd->ShowWindow( SW_SHOWNORMAL );
			//	pWnd->BringWindowToTop();
			//	pWnd->SetForegroundWindow();
			//}

			if ( MessageBox( NULL,
				(LPCWSTR)L"TorrentEnvy is currently running.\nDo you wish to open a new window?",
				(LPCWSTR)L"Envy TorrentEnvy",
				MB_ICONQUESTION | MB_OKCANCEL | MB_SETFOREGROUND ) == IDCANCEL )
			{
				CloseHandle( pMutex );
				return FALSE;
			}
		}
		// else Continue...
	}

	SetRegistryKey( L"Envy" );

	InitEnvironment();
	InitResources();

	CWizardSheet	pSheet;
	CWelcomePage	pWelcome;
	CExpertPage 	pExpert;
	CSinglePage 	pSingle;
	CPackagePage	pPackage;
	CTrackerPage	pTracker;
	CCommentPage	pComment;
	COutputPage 	pOutput;
	CFinishedPage	pFinished;

	m_pSheet = &pSheet;

	pSheet.AddPage( &pWelcome );
	pSheet.AddPage( &pExpert );
	pSheet.AddPage( &pSingle );
	pSheet.AddPage( &pPackage );
	pSheet.AddPage( &pTracker );
	pSheet.AddPage( &pComment );
	pSheet.AddPage( &pOutput );
	pSheet.AddPage( &pFinished );

	if ( cmdInfo.m_sPaths.GetCount() )
	{
		if ( m_sCommandLineSourceFile.IsEmpty() && cmdInfo.m_sPaths.GetCount() == 1 )
			m_sCommandLineSourceFile = cmdInfo.m_sPaths.GetHead();

		for ( int i = (int)cmdInfo.m_sPaths.GetCount() ; i ; i-- )
			m_sCommandLinePaths.AddTail( cmdInfo.m_sPaths.RemoveHead() );
	}

	pSheet.DoModal();

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentEnvyApp environment

void CTorrentEnvyApp::InitEnvironment()
{
	TCHAR szPath[260];
	DWORD dwSize = 0;

	m_nVersion[0] = m_nVersion[1] = m_nVersion[2] = 0;

	if ( GetModuleFileName( NULL, szPath, 260 ) )
	{
		m_sPath	= szPath;
		dwSize	= GetFileVersionInfoSize( szPath, &dwSize );
	}

	if ( dwSize > 0 )
	{
		BYTE* pBuffer = new BYTE[ dwSize ];

		if ( GetFileVersionInfo( szPath, NULL, dwSize, pBuffer ) )
		{
			VS_FIXEDFILEINFO* pTable;

			if ( VerQueryValue( pBuffer, L"\\", (VOID**)&pTable, (UINT*)&dwSize ) )
			{
				m_nVersion[0] = (WORD)( pTable->dwFileVersionMS >> 16 );
				m_nVersion[1] = (WORD)( pTable->dwFileVersionMS & 0xFFFF );
				m_nVersion[2] = (WORD)( pTable->dwFileVersionLS >> 16 );
			//	m_nVersion[3] = (WORD)( pTable->dwFileVersionLS & 0xFFFF );
			}
		}

		delete [] pBuffer;
	}

	m_sVersion.Format( L"%i.%i.%i",
		m_nVersion[0], m_nVersion[1], m_nVersion[2] );

	// Obsolete:
	//OSVERSIONINFO pVersion;
	//pVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	//GetVersionEx( &pVersion );
	//
	//m_bNT = ( pVersion.dwPlatformId == VER_PLATFORM_WIN32_NT );
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentEnvyApp resources

void CTorrentEnvyApp::InitResources()
{
	//OSVERSIONINFO pVersion;
	//pVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	//GetVersionEx( &pVersion );	// Deprecated
	//LPCTSTR pszFont = pVersion.dwMajorVersion > 5 ? L"Segoe UI" : L"Tahoma";
	LPCTSTR pszFont = L"Segoe UI";

	HDC hDC = GetDC( NULL );
	const int nSize = -MulDiv( 11, GetDeviceCaps( hDC, LOGPIXELSY ), 96 );	// DPI-aware
	ReleaseDC( NULL, hDC );

	m_fntNormal.CreateFont( nSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFont );

	m_fntBold.CreateFont( nSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFont );

	m_fntLine.CreateFont( nSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFont );

	m_fntTiny.CreateFont( nSize + 3, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFont );

	srand( GetTickCount() );
}

/////////////////////////////////////////////////////////////////////////////
// Utilities

CString SmartSize(QWORD nVolume)
{
	LPCTSTR pszUnit = L"B";
	CString strVolume;

	if ( nVolume < 1024 )
	{
		strVolume.Format( L"%lu %s", (DWORD)nVolume, pszUnit );
		return strVolume;
	}

	nVolume /= 1024;

	if ( nVolume < 1024 )
		strVolume.Format( L"%lu K%s", (DWORD)nVolume, pszUnit );
	else if ( nVolume < 1024*1024 )
		strVolume.Format( L"%.2lf M%s", (double)(__int64)nVolume / 1024, pszUnit );
	else if ( nVolume < 1024*1024*1024 )
		strVolume.Format( L"%.3lf G%s", (double)(__int64)nVolume / (1024*1024), pszUnit );
	else
		strVolume.Format( L"%.3lf T%s", (double)(__int64)nVolume / (1024*1024*1024), pszUnit );

	return strVolume;
}
