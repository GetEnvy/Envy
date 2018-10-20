//
// WizardInterfacePage.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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
#include "WizardInterfacePage.h"
#include "WndMain.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CWizardInterfacePage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardInterfacePage, CWizardPage)
	ON_WM_LBUTTONDOWN()
	ON_WM_XBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardInterfacePage property page

CWizardInterfacePage::CWizardInterfacePage() : CWizardPage(CWizardInterfacePage::IDD)
{
	m_nSkin = 0;
	m_bBasic = Settings.General.GUIMode == GUI_BASIC;
	m_bSimpleDownloadBars = Settings.Downloads.SimpleBar;
}

CWizardInterfacePage::~CWizardInterfacePage()
{
}

void CWizardInterfacePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DESCRIPTION_EXPERT, m_wndDescriptionExpert);
	DDX_Control(pDX, IDC_DESCRIPTION_BASIC, m_wndDescriptionBasic);
	DDX_Control(pDX, IDC_INTERFACE_EXPERT, m_wndInterfaceExpert);
	DDX_Control(pDX, IDC_INTERFACE_BASIC, m_wndInterfaceBasic);
	DDX_Control(pDX, IDC_WIZARD_NOSKIN, m_wndSkinNoChange);
	DDX_Control(pDX, IDC_WIZARD_DEFAULTSKIN, m_wndSkinDefault);
	DDX_Control(pDX, IDC_WIZARD_DARKSKIN, m_wndSkinDark);
	DDX_Check(pDX, IDC_DOWNLOADS_SIMPLEBAR, m_bSimpleDownloadBars);
	DDX_Radio(pDX, IDC_INTERFACE_EXPERT, m_bBasic);
	DDX_Radio(pDX, IDC_WIZARD_NOSKIN, m_nSkin);
}

/////////////////////////////////////////////////////////////////////////////
// CWizardInterfacePage message handlers

BOOL CWizardInterfacePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	Skin.Apply( L"CWizardInterfacePage", this );

	m_nSkin = 0;
	m_bBasic = Settings.General.GUIMode == GUI_BASIC;
	m_bSimpleDownloadBars = Settings.Downloads.SimpleBar;

	m_wndSkinNoChange.SetCheck( TRUE );
	m_wndSkinDefault.SetCheck( FALSE );
	m_wndSkinDark.SetCheck( FALSE );

	UpdateData( FALSE );

	m_wndInterfaceBasic.SetFont( &theApp.m_gdiFontBold );
	m_wndInterfaceExpert.SetFont( &theApp.m_gdiFontBold );

	return TRUE;
}

BOOL CWizardInterfacePage::OnSetActive()
{
	// Wizard Window Caption Workaround
	CString strCaption;
	GetWindowText( strCaption );
	GetParent()->SetWindowText( strCaption );

	CoolInterface.FixThemeControls( this );		// Checkbox/Groupbox text colors (Remove theme if needed)

	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
	return CWizardPage::OnSetActive();
}

void CWizardInterfacePage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

void CWizardInterfacePage::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rc;
	ClientToScreen( &point );

	// Select mode by clicking any text
	m_wndDescriptionBasic.GetWindowRect( &rc );
	if ( rc.PtInRect( point ) )
	{
		m_wndInterfaceBasic.SetCheck( TRUE );
		m_wndInterfaceExpert.SetCheck( FALSE );
	}
	else
	{
		m_wndDescriptionExpert.GetWindowRect( &rc );
		if ( rc.PtInRect( point ) )
		{
			m_wndInterfaceExpert.SetCheck( TRUE );
			m_wndInterfaceBasic.SetCheck( FALSE );
		}
	}

	CWizardPage::OnLButtonDown( nFlags, point );
}

LRESULT CWizardInterfacePage::OnWizardNext()
{
	UpdateData( TRUE );

	Settings.Downloads.SimpleBar = m_bSimpleDownloadBars != FALSE;

	if ( ! m_bBasic && Settings.General.GUIMode == GUI_BASIC )
	{
		CWaitCursor pCursor;
		CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();

		Settings.Skin.RowSize = 17;
		Settings.General.GUIMode = GUI_TABBED;
		pMainWnd->SetGUIMode( Settings.General.GUIMode, FALSE );
	}
	else if ( m_bBasic && Settings.General.GUIMode != GUI_BASIC )
	{
		CWaitCursor pCursor;
		CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();

		Settings.Skin.RowSize = 18;
		Settings.General.GUIMode = GUI_BASIC;
		pMainWnd->SetGUIMode( Settings.General.GUIMode, FALSE );
	}

	Settings.Save();

	if ( m_nSkin )
	{
		CWaitCursor pCursor;

		ClearSkins();

		if ( m_nSkin == 2 && theApp.m_nWinVer >= WIN_10)	// Dark Mode
		{
			theApp.WriteProfileInt( L"Skins", L"Windows 10\\Windows10.Dark.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows 10\\Windows10.DarkFrames.xml", 1 );
		}
		else if ( m_nSkin == 2)	// Dark Skin
		{
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\SkinDark.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\Skin8AltFrames.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\SkinVistaRemote.xml", 1 );
		}
		else if ( theApp.m_nWinVer >= WIN_10 )
		{
			theApp.WriteProfileInt( L"Skins", L"Windows 10\\Windows10.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows 10\\Windows10.Frames.xml", 1 );
		}
		else if ( theApp.m_nWinVer >= WIN_8 )
		{
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\Skin8.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\Skin8Frames.xml", 1 );
		}
		else if ( theApp.m_nWinVer >= WIN_7 )
		{
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\Skin7.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\SkinVistaFrames.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\SkinVistaRemote.xml", 1 );
		}
		else if ( theApp.m_nWinVer < WIN_7 )
		{
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\SkinVista.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\SkinVistaFrames.xml", 1 );
			theApp.WriteProfileInt( L"Skins", L"Windows Collection\\SkinVistaRemote.xml", 1 );
		}

		if ( theApp.m_nWinVer >= WIN_7 && GetSystemMetrics( SM_CYSCREEN ) > 1050 )	// ToDo: Detect DPI
			theApp.WriteProfileInt( L"Skins", L"Flags\\Flags.xml", 1 );

		PostMainWndMessage( WM_SKINCHANGED );
		Sleep( 2500 );		// Wait a few seconds
		GetParent()->Invalidate();
	}

	return 0;
}

void CWizardInterfacePage::ClearSkins(LPCTSTR pszPath /*NULL*/)
{
	WIN32_FIND_DATA pFind;
	HANDLE hSearch;

	CString strPath;
	strPath.Format( L"%s\\Skins\\%s*.*",
		(LPCTSTR)Settings.General.Path, pszPath ? pszPath : L"" );

	hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == L'.' ) continue;

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( _tcsicmp( pFind.cFileName, L"Languages" ) == 0 ) continue;

				strPath.Format( L"%s%s\\",
					pszPath ? pszPath : L"", pFind.cFileName );

				ClearSkins( strPath );
			}
			else if ( _tcsistr( pFind.cFileName, L".xml" ) != NULL )
			{
				strPath.Format( L"%s%s",
					pszPath ? pszPath : L"", pFind.cFileName );

				if ( EndsWith( strPath, _P( L".xml" ) ) )
					theApp.WriteProfileInt( L"Skins", strPath, 0 );
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}
}