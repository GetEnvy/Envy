//
// Skin.cpp
//
// This file is part of Envy (getenvy.com) © 2016
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
#include "Skin.h"
#include "SkinWindow.h"
#include "CtrlCoolBar.h"
#include "CoolMenu.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "Images.h"
#include "Buffer.h"
#include "Plugins.h"
#include "WndChild.h"
#include "WndSettingsPage.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CSkin Skin;

BOOL CSkin::m_bSkinChanging = FALSE;	// Static system state indicator (Active CMainWnd::OnSkinChanged)


//////////////////////////////////////////////////////////////////////
// CSkin construction

CSkin::CSkin()
{
	// Experimental values (ToDo:?)
	m_pStrings.InitHashTable( 1531 );
	m_pMenus.InitHashTable( 83 );
	m_pToolbars.InitHashTable( 61 );
	m_pDocuments.InitHashTable( 61 );
	m_pWatermarks.InitHashTable( 31 );
	m_pLists.InitHashTable( 31 );
	m_pDialogs.InitHashTable( 127 );

	CreateDefaultColors();
}

CSkin::~CSkin()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSkin apply

void CSkin::Apply()
{
	Clear();

	CreateDefault();

	ApplyRecursive( L"Languages\\" );

	Plugins.RegisterCommands();
	Plugins.InsertCommands();

	ApplyRecursive( NULL );

	CoolMenu.SetWatermark( GetWatermark( L"CCoolMenu" ) );

	Plugins.OnSkinChanged();

//#ifdef _DEBUG
//	theApp.Message( MSG_INFO, L"Icons: 16px %i, 32px %i, 48px %i",
//		CoolInterface.GetImageCount(LVSIL_SMALL), CoolInterface.GetImageCount(LVSIL_NORMAL), CoolInterface.GetImageCount(LVSIL_BIG) );
//#endif
}

//////////////////////////////////////////////////////////////////////
// CSkin default skin

void CSkin::CreateDefault()
{
	CreateDefaultColors();

	CoolInterface.CreateFonts();

	Settings.General.Language = L"en";
	Settings.General.LanguageRTL = false;
	Settings.General.LanguageDefault = true;

	// Default Skin Options:
	Settings.SetDefault( &Settings.Skin.MenubarHeight );	// 28
	Settings.SetDefault( &Settings.Skin.ToolbarHeight );	// 28
	Settings.SetDefault( &Settings.Skin.TaskbarHeight );	// 26
	Settings.SetDefault( &Settings.Skin.TaskbarTabWidth );	// 0	// 200/140 set in WndMain
	Settings.SetDefault( &Settings.Skin.GroupsbarHeight );	// 24
	Settings.SetDefault( &Settings.Skin.HeaderbarHeight );	// 64
	Settings.SetDefault( &Settings.Skin.MonitorbarWidth );	// 120
	Settings.SetDefault( &Settings.Skin.SidebarWidth ); 	// 200
	Settings.SetDefault( &Settings.Skin.SidebarPadding );	// 12
	Settings.SetDefault( &Settings.Skin.Splitter ); 		// 6
	Settings.SetDefault( &Settings.Skin.ButtonEdge );		// 4
	Settings.SetDefault( &Settings.Skin.LibIconsX );		// 220
	Settings.SetDefault( &Settings.Skin.LibIconsY );		// 56
	Settings.SetDefault( &Settings.Skin.FrameEdge );		// true
	Settings.SetDefault( &Settings.Skin.MenuBorders );		// true
	Settings.SetDefault( &Settings.Skin.MenuGripper );		// true
	Settings.SetDefault( &Settings.Skin.RoundedSelect );	// false
	Settings.SetDefault( &Settings.Skin.DropMenu ); 		// false
	Settings.SetDefault( &Settings.Skin.DropMenuLabel ); 	// true
	m_ptNavBarOffset = CPoint( 0, 0 );

	// Command Icons
	//if ( HICON hIcon = theApp.LoadIcon( IDI_CHECKMARK ) )
	//{
	//	//if ( Settings.General.LanguageRTL ) hIcon = CreateMirroredIcon( hIcon );	// Impossible?
	//	CoolInterface.AddIcon( ID_CHECKMARK, hIcon );
	//	VERIFY( DestroyIcon( hIcon ) );
	//}

	// Load Definitions
	LoadFromResource( NULL, IDR_XML_DEFINITIONS );
	LoadFromResource( NULL, IDR_XML_DEFAULT );

	// Copying
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_GUIDE );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_UPDATE );

//	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_1 );
//	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_2 );
//	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_3 );
//	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_4 );
//	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_BITPRINT );
//	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_SKINS );
}

void CSkin::CreateDefaultColors()
{
	Colors.CalculateColors( FALSE );

	// Blank NavBar Workaround (Initialize here only)
	Colors.m_crNavBarText			= CLR_NONE;
	Colors.m_crNavBarTextUp			= CLR_NONE;
	Colors.m_crNavBarTextDown		= CLR_NONE;
	Colors.m_crNavBarTextHover		= CLR_NONE;
	Colors.m_crNavBarTextChecked	= CLR_NONE;
	Colors.m_crNavBarShadow			= CLR_NONE;
	Colors.m_crNavBarShadowUp		= CLR_NONE;
	Colors.m_crNavBarShadowDown		= CLR_NONE;
	Colors.m_crNavBarShadowHover	= CLR_NONE;
	Colors.m_crNavBarShadowChecked	= CLR_NONE;
	Colors.m_crNavBarOutline		= CLR_NONE;
	Colors.m_crNavBarOutlineUp		= CLR_NONE;
	Colors.m_crNavBarOutlineDown	= CLR_NONE;
	Colors.m_crNavBarOutlineHover	= CLR_NONE;
	Colors.m_crNavBarOutlineChecked	= CLR_NONE;
}

//////////////////////////////////////////////////////////////////////
// CSkin clear

void CSkin::Clear()
{
	//CQuickLock oLock( m_pSection );

	CString strName;
	POSITION pos;

	for ( pos = m_pMenus.GetStartPosition() ; pos ; )
	{
		CMenu* pMenu;
		m_pMenus.GetNextAssoc( pos, strName, pMenu );
		delete pMenu;
	}

	for ( pos = m_pToolbars.GetStartPosition() ; pos ; )
	{
		CCoolBarCtrl* pBar;
		m_pToolbars.GetNextAssoc( pos, strName, pBar );
		delete pBar;
	}

	for ( pos = m_pDialogs.GetStartPosition() ; pos ; )
	{
		CXMLElement* pXML;
		m_pDialogs.GetNextAssoc( pos, strName, pXML );
		delete pXML;
	}

	for ( pos = m_pDocuments.GetStartPosition() ; pos ; )
	{
		CXMLElement* pXML;
		m_pDocuments.GetNextAssoc( pos, strName, pXML );
		delete pXML;
	}

	for ( pos = m_pSkins.GetHeadPosition() ; pos ; )
	{
		delete m_pSkins.GetNext( pos );
	}

	for ( pos = m_pFontPaths.GetHeadPosition() ; pos ; )
	{
		RemoveFontResourceEx( m_pFontPaths.GetNext( pos ), FR_PRIVATE, NULL );
	}

	m_pStrings.RemoveAll();
	m_pControlTips.RemoveAll();
	m_pMenus.RemoveAll();
	m_pToolbars.RemoveAll();
	m_pDocuments.RemoveAll();
	m_pWatermarks.RemoveAll();
	m_pLists.RemoveAll();
	m_pDialogs.RemoveAll();
	m_pSkins.RemoveAll();
	m_pFontPaths.RemoveAll();
	m_pImages.RemoveAll();

//	if ( m_brDialog.m_hObject ) m_brDialog.DeleteObject();
//	if ( m_brDialogPanel.m_hObject ) m_brDialogPanel.DeleteObject();
//	if ( m_brMediaSlider.m_hObject ) m_brMediaSlider.DeleteObject();

//	if ( m_bmDialog.m_hObject ) m_bmDialog.DeleteObject();
//	if ( m_bmDialogPanel.m_hObject ) m_bmDialogPanel.DeleteObject();
//	if ( m_bmToolTip.m_hObject ) m_bmToolTip.DeleteObject();
//	if ( m_bmSelected.m_hObject ) m_bmSelected.DeleteObject();

//	if ( m_bmPanelMark.m_hObject ) m_bmPanelMark.DeleteObject();
//	if ( m_bmBanner.m_hObject ) m_bmBanner.DeleteObject();

	Images.DeleteObjects();

	CoolInterface.Clear();
}

//////////////////////////////////////////////////////////////////////
// CSkin caption selector

BOOL CSkin::SelectCaption(CWnd* pWnd, int nIndex)
{
	CString strCaption;
	pWnd->GetWindowText( strCaption );

	if ( SelectCaption( strCaption, nIndex ) )
	{
		pWnd->SetWindowText( strCaption );
		return TRUE;
	}

	return FALSE;
}

BOOL CSkin::SelectCaption(CString& strCaption, int nIndex)
{
	for ( strCaption += '|' ; ; nIndex-- )
	{
		CString strSection = strCaption.SpanExcluding( L"|" );
		strCaption = strCaption.Mid( strSection.GetLength() + 1 );
		if ( strSection.IsEmpty() ) break;

		if ( nIndex <= 0 )
		{
			strCaption = strSection;
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSkin recursive folder applicator

void CSkin::ApplyRecursive(LPCTSTR pszPath /*NULL*/)
{
	WIN32_FIND_DATA pFind;
	HANDLE hSearch;

	CString strPath;
	strPath.Format( L"%s\\Skins\\%s*.*", (LPCTSTR)Settings.General.Path,
		pszPath ? pszPath : L"" );

	hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == '.' ) continue;

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( pszPath == NULL && _tcsicmp( pFind.cFileName, L"languages" ) != 0 ||
					 pszPath != NULL && _tcsistr( pszPath, L"languages" ) == NULL )
				{
					strPath.Format( L"%s%s\\", pszPath ? pszPath : L"", pFind.cFileName );
					ApplyRecursive( strPath );
				}
			}
			else if ( _tcsistr( pFind.cFileName, L".xml" ) != NULL &&
					  _tcsicmp( pFind.cFileName, L"Definitions.xml" ) != 0 )
			{
				strPath.Format( L"%s%s", pszPath ? pszPath : L"", pFind.cFileName );

				if ( theApp.GetProfileInt( L"Skins", strPath, FALSE ) )
					LoadFromFile( Settings.General.Path + L"\\Skins\\" + strPath );
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}
}

//////////////////////////////////////////////////////////////////////
// CSkin root load

BOOL CSkin::LoadFromFile(LPCTSTR pszFile)
{
	TRACE( L"Loading skin file: %s\n", pszFile );

	CXMLElement* pXML = CXMLElement::FromFile( pszFile );
	if ( pXML == NULL ) return FALSE;

	CString strPath = pszFile;
	int nSlash = strPath.ReverseFind( L'\\' );
	if ( nSlash >= 0 ) strPath = strPath.Left( nSlash + 1 );

	BOOL bResult = LoadFromXML( pXML, strPath );

	delete pXML;

	return bResult;
}

BOOL CSkin::LoadFromResource(HINSTANCE hInstance, UINT nResourceID)
{
	HMODULE hModule = ( hInstance != NULL ) ? hInstance : GetModuleHandle( NULL );
	CString strBody( ::LoadHTML( hModule, nResourceID ) );
	CString strPath;
	strPath.Format( L"%p$", hModule );
	return LoadFromString( strBody, strPath );
}

BOOL CSkin::LoadFromString(const CString& strXML, const CString& strPath)
{
	CXMLElement* pXML = CXMLElement::FromString( strXML, TRUE );
	if ( pXML == NULL ) return FALSE;

	BOOL bSuccess = LoadFromXML( pXML, strPath );

	delete pXML;
	return bSuccess;
}

BOOL CSkin::LoadFromXML(CXMLElement* pXML, const CString& strPath)
{
	BOOL bSuccess = FALSE;

	if ( ! pXML->IsNamed( L"skin" ) )
	{
		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [skin] root element", pXML->ToString() );
		return bSuccess;
	}

	// XML Root Elements:
	SwitchMap( Text )
	{
		Text[ L"manifest" ]		= 'm';
		Text[ L"windows" ]		= 'w';
		Text[ L"windowskins" ]	= 'w';
		Text[ L"watermarks" ]	= 'e';
		Text[ L"images" ]		= 'e';
		Text[ L"icons" ] 		= 'i';
		Text[ L"commandimages" ] = 'i';
		Text[ L"colors" ]		= 'c';
		Text[ L"colours" ]		= 'c';
		Text[ L"colorscheme" ]	= 'c';
		Text[ L"colourscheme" ]	= 'c';
		Text[ L"toolbars" ]		= 't';
		Text[ L"menus" ]		= 'u';
		Text[ L"dialogs" ]		= 'a';
		Text[ L"documents" ] 	= 'd';
		Text[ L"listcolumns" ]	= 'l';
		Text[ L"options" ]		= 'o';
		Text[ L"navbar" ]		= 'v';	// Legacy
		Text[ L"fonts" ] 		= 'f';
		Text[ L"remote" ]		= 'r';
		Text[ L"strings" ]		= 's';
		Text[ L"commandtips" ]	= 's';
		Text[ L"controltips" ]	= 'n';
		Text[ L"commandmap" ]	= 'p';
		Text[ L"resourcemap" ]	= 'p';
		Text[ L"tipmap" ]		= 'p';
	}

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pSub = pXML->GetNextElement( pos );
		CString strElement = pSub->GetName();
		strElement.MakeLower();
		bSuccess = FALSE;

		switch ( Text[ strElement ] )
		{
		case 'w':	// windowskins, windows
			if ( ! LoadWindowSkins( pSub, strPath ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"WindowSkins" );
			break;
		case 'e':	// watermarks, images
			if ( ! LoadWatermarks( pSub, strPath ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Watermarks" );
			break;
		case 'i':	// commandimages, icons
			if ( ! LoadCommandImages( pSub, strPath ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"CommandImages" );
			break;
		case 'c':	// colorscheme, colourscheme, colors
			if ( ! LoadColorScheme( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"ColorScheme" );
			break;
		case 't':	// toolbars
			if ( ! LoadToolbars( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Toolbars" );
			break;
		case 'u':	// menus
			if ( ! LoadMenus( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Menus" );
			break;
		case 'a':	// dialogs
			if ( ! LoadDialogs( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Dialogs" );
			break;
		case 'd':	// documents
			if ( ! LoadDocuments( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Documents" );
			break;
		case 'r':	// remote
			if ( ! LoadRemoteInterface( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Remote" );
			break;
		case 's':	// strings, commandtips
			if ( ! LoadStrings( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Strings" );
			break;
		case 'n':	// controltips
			if ( ! LoadControlTips( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"ControlTips" );
			break;
		case 'p':	// commandmap, resourcemap, tipmap
			if ( ! LoadResourceMap( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"ResourceMap" );
			break;
		case 'l':	// listcolumns
			if ( ! LoadListColumns( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"ListColumns" );
			break;
		case 'f':	// fonts
			if ( ! LoadFonts( pSub, strPath ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Fonts" );
			break;
		case 'o':	// options
			if ( ! LoadOptions( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"Options" );
			break;
		case 'v':	// navbar  (Deprecated Shareaza import only)
			if ( ! LoadNavBar( pSub ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed section", L"NavBar (Deprecated)" );
			break;

		case 'm':	// manifest
			if ( pSub->GetAttributeValue( L"type" ).CompareNoCase( L"skin" ) == 0 )
			{
				CString strSkinName = pSub->GetAttributeValue( L"name", L"" );
				theApp.Message( MSG_NOTICE, IDS_SKIN_LOAD, strSkinName );
			}
			else if ( pSub->GetAttributeValue( L"type" ).CompareNoCase( L"language" ) == 0 )
			{
				Settings.General.Language = pSub->GetAttributeValue( L"language", L"en" );
				Settings.General.LanguageRTL = ( pSub->GetAttributeValue( L"dir", L"ltr" ) == L"rtl" );
				Settings.General.LanguageDefault = Settings.General.Language.Left(2) == L"en";
				TRACE( L"Loading language: %s\r\n", Settings.General.Language );
				TRACE( L"RTL: %d\r\n", Settings.General.LanguageRTL );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [type] attribute in [manifest] element", pSub->ToString() );
			}
			break;

		default:
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in root [skin] element", pSub->ToString() );
			continue;
		}

		bSuccess = TRUE;
	}

	return bSuccess;
}


//////////////////////////////////////////////////////////////////////
// CSkin custom options

BOOL CSkin::LoadOptions(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( ! pXML->IsNamed( L"option" ) )
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in skin [Options]", pXML->ToString() );
			continue;		// Failed, but keep trying
		}

		const CString strName	= pXML->GetAttributeValue( L"name" ).MakeLower();
		const CString strValue	= pXML->GetAttributeValue( L"value" ).MakeLower();
		const CString strHeight	= pXML->GetAttributeValue( L"height" );
		const CString strWidth	= pXML->GetAttributeValue( L"width" );

		// Skin Options:
		SwitchMap( Text )
		{
			Text[ L"navbar" ]		= 'n';
			Text[ L"dropmenu" ]		= 'd';
			Text[ L"dropdownmenu" ]	= 'd';
			Text[ L"submenu" ]		= 'd';
			Text[ L"menubar" ]		= 'm';
			Text[ L"menubars" ]		= 'm';
			Text[ L"menubarbevel" ]	= 'b';
			Text[ L"menuborders" ]	= 'b';
			Text[ L"menugripper" ]	= 'p';
			Text[ L"grippers" ] 	= 'p';
			Text[ L"toolbar" ]		= 't';
			Text[ L"toolbars" ]		= 't';
			Text[ L"taskbar" ]		= 'k';
			Text[ L"tabbar" ]		= 'k';
			Text[ L"sidebar" ]		= 's';
			Text[ L"sidepanel" ] 	= 's';
			Text[ L"taskpanel" ] 	= 's';
			Text[ L"taskboxpadding" ] = 'a';
			Text[ L"sidebarpadding" ] = 'a';
			Text[ L"sidebarmargin" ]  = 'a';
			Text[ L"titlebar" ]		= 'h';
			Text[ L"headerpanel" ]	= 'h';
			Text[ L"groupsbar" ] 	= 'g';
			Text[ L"downloadgroups" ] = 'g';
			Text[ L"bandwidthwidget" ] = 'o';
			Text[ L"monitorbar" ]	= 'o';
			Text[ L"dragbar" ]		= 'r';
			Text[ L"splitter" ]		= 'r';
			Text[ L"listitem" ]		= 'w';
			Text[ L"rowsize" ]		= 'w';
			Text[ L"roundedselect" ] = 'c';
			Text[ L"highlightchamfer" ] = 'c';
			Text[ L"frameedge" ]	= 'f';
			Text[ L"buttonedge" ]	= 'e';
			Text[ L"buttonmap" ] 	= 'e';
			Text[ L"icongrid" ]		= 'i';
			Text[ L"librarytiles" ]	= 'i';
			Text[ L"alticons" ]		= 'l';
		}

		switch ( Text[ strName ] )
		{
		case 'n':	// "Navbar"
			if ( ! LoadNavBar( pXML ) )
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Skin Option [Navbar] Failed", pXML->ToString() );
			break;
		case 'd':	// "DropMenu" or "SubMenu"
			if ( strValue.CompareNoCase( L"true" ) == 0 )
				Settings.Skin.DropMenu = true;
			else if ( strValue.CompareNoCase( L"false" ) == 0 )
				Settings.Skin.DropMenu = false;
			else if ( strValue == L"1" )
				Settings.Skin.DropMenu = true;
			else if ( strValue == L"0" )
				Settings.Skin.DropMenu = false;
			else if ( ! strValue.IsEmpty() && strValue.GetLength() < 3 )
				Settings.Skin.DropMenuLabel = _wtoi(strValue);
			if ( ! strWidth.IsEmpty() )
				Settings.Skin.DropMenuLabel = _wtoi(strWidth);
			if ( Settings.Skin.DropMenuLabel > 100 )
				Settings.Skin.DropMenuLabel = 0;
			else if ( Settings.Skin.DropMenuLabel > 1 )
				Settings.Skin.DropMenu = true;
			break;
		case 'b':	// "MenuBorders" or "MenubarBevel"
			Settings.Skin.MenuBorders = LoadOptionBool( strValue, Settings.Skin.MenuBorders );
			break;
		case 'p':	// "MenuGripper" or "Grippers"
			Settings.Skin.MenuGripper = LoadOptionBool( strValue, Settings.Skin.MenuGripper );
			break;
		case 'c':	// "RoundedSelect" or "HighlightChamfer"
			Settings.Skin.RoundedSelect = LoadOptionBool( strValue, Settings.Skin.RoundedSelect );
			break;
		case 'm':	// "Menubar" or "Menubars"
			if ( ! strHeight.IsEmpty() )
				Settings.Skin.MenubarHeight = _wtoi(strHeight);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.MenubarHeight = _wtoi(strValue);
			break;
		case 't':	// "Toolbar" or "Toolbars"
			if ( ! strHeight.IsEmpty() )
				Settings.Skin.ToolbarHeight = _wtoi(strHeight);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.ToolbarHeight = _wtoi(strValue);
			break;
		case 'k':	// "Taskbar" or "TabBar"
			if ( ! strWidth.IsEmpty() )
				Settings.Skin.TaskbarTabWidth = _wtoi(strWidth);
			if ( ! strHeight.IsEmpty() )
				Settings.Skin.TaskbarHeight = _wtoi(strHeight);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.TaskbarHeight = _wtoi(strValue);
			break;
		case 's':	// "Sidebar" or "SidePanel" or "TaskPanel"
			if ( ! strWidth.IsEmpty() )
				Settings.Skin.SidebarWidth = _wtoi(strWidth);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.SidebarWidth = _wtoi(strValue);
			break;
		case 'a':	// "SidebarMargin" or "SidebarPadding" or "TaskPanelPadding"
			if ( ! strWidth.IsEmpty() )
				Settings.Skin.SidebarPadding = _wtoi(strWidth);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.SidebarPadding = _wtoi(strValue);
			break;
		case 'h':	// "Titlebar" or "HeaderPanel"
			if ( ! strHeight.IsEmpty() )
				Settings.Skin.HeaderbarHeight = _wtoi(strHeight);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.HeaderbarHeight = _wtoi(strValue);
			break;
		case 'g':	// "Groupsbar" or "DownloadGroups"
			if ( ! strHeight.IsEmpty() )
				Settings.Skin.GroupsbarHeight = _wtoi(strHeight);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.GroupsbarHeight = _wtoi(strValue);
			break;
		case 'o':	// "Monitorbar" or "BandwidthWidget"
			if ( ! strWidth.IsEmpty() )
				Settings.Skin.MonitorbarWidth = _wtoi(strWidth);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.MonitorbarWidth = _wtoi(strValue);
			break;
		case 'r':	// "Dragbar" or "Splitter"
			if ( ! strWidth.IsEmpty() )
				Settings.Skin.Splitter = _wtoi(strWidth);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.Splitter = _wtoi(strValue);
			break;
		case 'e':	// "ButtonEdge" or "ButtonMap"
			if ( ! strWidth.IsEmpty() )
				Settings.Skin.ButtonEdge = _wtoi(strWidth);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.ButtonEdge = _wtoi(strValue);
			break;
		case 'f':	// "FrameEdge"
			Settings.Skin.FrameEdge = LoadOptionBool( strValue, Settings.Skin.FrameEdge );
			break;
		case 'l':	// "AltIcons"
			Settings.Skin.AltIcons = LoadOptionBool( strValue, Settings.Skin.AltIcons );
			break;
		case 'i':	// "IconGrid" or "LibraryTiles"
			if ( ! strHeight.IsEmpty() )
				Settings.Skin.LibIconsY = _wtoi(strHeight);
			if ( ! strWidth.IsEmpty() )
				Settings.Skin.LibIconsX = _wtoi(strWidth);
			else if ( ! strValue.IsEmpty() )
				Settings.Skin.LibIconsX = _wtoi(strValue);
			break;
		case 'w':	// "RowSize" or "ListItem"
		{
			int nSize;
			if ( ! strHeight.IsEmpty() )
				nSize = _wtoi(strHeight);
			else if ( ! strValue.IsEmpty() )
				nSize = _wtoi(strValue);
			else
				break;
			if ( nSize >= 16 && nSize <= 20 )
				Settings.Skin.RowSize = nSize;
		}
		break;
		}
	}

	return TRUE;
}

bool CSkin::LoadOptionBool(const CString str, bool bDefault /*false*/)
{
	if ( str.CompareNoCase( L"true" ) == 0 )
		return true;
	if ( str.CompareNoCase( L"false" ) == 0 )
		return false;
	if ( str == L"1" )
		return true;
	if ( str == L"0" )
		return false;
	if ( str == L"on" )
		return true;
	if ( str == L"off" )
		return false;
	if ( str == L"yes" )
		return true;
	if ( str == L"no" )
		return false;
	theApp.Message( MSG_DEBUG, L"Unexpected skin option value: %s", (LPCTSTR)str );
	return bDefault;
}


//////////////////////////////////////////////////////////////////////
// CSkin strings

void CSkin::AddString(const CString& strString, UINT nStringID)
{
	m_pStrings.SetAt( nStringID, strString );
}

BOOL CSkin::LoadString(CString& str, UINT nStringID) const
{
	if ( nStringID < 10 )
		return FALSE;	// Popup menus

	if ( m_pStrings.Lookup( nStringID, str ) ||
		( IS_INTRESOURCE( nStringID ) && str.LoadString( nStringID ) ) )
		return TRUE;

	HWND hWnd = (HWND)UIntToPtr( nStringID );
	if ( IsWindow( hWnd ) )
	{
		CWnd::FromHandle( hWnd )->GetWindowText( str );
		return TRUE;
	}

#ifdef _DEBUG
	theApp.Message( MSG_ERROR, L"Failed to load string %d.", nStringID );
#endif // _DEBUG

	str.Empty();
	return FALSE;
}

BOOL CSkin::LoadStrings(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"string" ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strValue = pXML->GetAttributeValue( L"text" );
				if ( strValue.IsEmpty() )
					strValue = pXML->GetAttributeValue( L"value" );	// Deprecated

				CheckExceptions( strValue );	// Brittish?

				for ( ;; )
				{
					int nPos = strValue.Find( L"\\n" );
					if ( nPos < 0 )
						nPos = strValue.Find( L"{n}" );
					if ( nPos < 0 )
						break;

					strValue = strValue.Left( nPos ) + L'\n' + strValue.Mid( nPos + ( strValue[ nPos ] == L'{' ? 3 : 2 ) );
				}

				// Hack for I64 compliance  (ToDo: Check this)
				if ( nID == IDS_DOWNLOAD_FRAGMENT_REQUEST || nID == IDS_DOWNLOAD_USEFUL_RANGE || nID == IDS_DOWNLOAD_VERIFY_DROP ||
					 nID == IDS_UPLOAD_CONTENT || nID == IDS_UPLOAD_PARTIAL_CONTENT )
					strValue.Replace( L"%lu", L"%I64i" );

				m_pStrings.SetAt( nID, strValue );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [id] attribute in [string] element", pXML->ToString() );
			}
		}
		else if ( pXML->IsNamed( L"tip" ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strMessage = pXML->GetAttributeValue( L"text" );
				if ( strMessage.IsEmpty() )
					strMessage = pXML->GetAttributeValue( L"message" );	// Deprecated
				CString strTip = pXML->GetAttributeValue( L"tip" );

				CheckExceptions( strMessage );	// Brittish?

				if ( ! strTip.IsEmpty() ) strMessage += L'\n' + strTip;
				m_pStrings.SetAt( nID, strMessage );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [id] attribute in [tip] element", pXML->ToString() );
			}
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [strings] element", pXML->ToString() );
		}
	}

	return TRUE;
}

void CSkin::CheckExceptions(CString& str, BOOL bExtensive /*FALSE*/)
{
	// Special case overrides, currently automatic Alt Brittish spelling
	if ( Settings.General.Language != L"en-uk" || str.GetLength() < 6 )
		return;

	if ( ! bExtensive )
		str.Replace( L"eighbor", L"eighbour" );		// Neighbour/neighbour only default
	else
		str.Replace( L"eighbor", L"eighbour" ) ||
		str.Replace( L"rganize", L"rganise" ) ||
		str.Replace( L"inimize", L"inimise" ) ||
		str.Replace( L"ehavior", L"ehaviour" ) ||
		str.Replace( L"Color", L"Colour" );
}

//////////////////////////////////////////////////////////////////////
// CSkin dialog control tips

BOOL CSkin::LoadControlTip(CString& str, UINT nCtrlID)
{
	if ( m_pControlTips.Lookup( nCtrlID, str ) ) return TRUE;
	str.Empty();
	return FALSE;
}

BOOL CSkin::LoadControlTips(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"tip" ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strMessage = pXML->GetAttributeValue( L"text" );
				if ( strMessage.IsEmpty() )
					strMessage = pXML->GetAttributeValue( L"message" );	// Deprecated
				strMessage.Replace( L"{n}", L"\r\n" );
				m_pControlTips.SetAt( nID, strMessage );
			}
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CSkin dialog control tips

BOOL CSkin::LoadRemoteText(CString& str, const CString& strTag)
{
	if ( m_pRemote.Lookup( strTag, str ) ) return TRUE;
	str.Empty();
	return FALSE;
}

BOOL CSkin::LoadRemoteInterface(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( pXML->IsNamed( L"tag" ) )
		{
			CString strTag = pXML->GetAttributeValue( L"id" );
			if ( strTag.IsEmpty() )
				strTag = pXML->GetAttributeValue( L"find" );		// Alt
			if ( StartsWith( strTag, _P( L"text_" ) ) )
			{
				ToLower( strTag );
				CString str = pXML->GetAttributeValue( L"text" );
				if ( str.IsEmpty() )
					str = pXML->GetAttributeValue( L"replace" );	// Alt
				//strTo.Replace( L"{n}", L"\r\n" );
				//CheckExceptions( strText, TRUE );	// No Brittish?
				if ( ! str.IsEmpty() )
					m_pRemote.SetAt( strTag, str );
			}
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CSkin menus

CMenu* CSkin::GetMenu(LPCTSTR pszName) const
{
	ASSERT( pszName != NULL );
	CString strName( pszName );

	CMenu* pMenu = NULL;
	m_pMenus.Lookup( strName + m_pszGUIMode[ Settings.General.GUIMode ], pMenu ) ||
		( Settings.General.GUIMode == GUI_BASIC && m_pMenus.Lookup( strName + m_pszGUIMode[ GUI_TABBED ], pMenu ) ) ||
		m_pMenus.Lookup( strName, pMenu );

	// Legacy method:
//	ASSERT( Settings.General.GUIMode == GUI_TABBED || Settings.General.GUIMode == GUI_BASIC || Settings.General.GUIMode == GUI_WINDOWED );
//	for ( int nModeTry = 0 ; m_pszModeSuffix[ Settings.General.GUIMode ][ nModeTry ] ; nModeTry++ )
//	{
//		if ( m_pMenus.Lookup( strName + m_pszModeSuffix[ Settings.General.GUIMode ][ nModeTry ], pMenu ) )
//			break;
//	}

	ASSERT_VALID( pMenu );
	ASSERT( pMenu->GetMenuItemCount() > 0 );

	return pMenu;
}

BOOL CSkin::LoadMenus(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( pXML->IsNamed( L"menu" ) )
		{
			if ( ! LoadMenu( pXML ) )
				return FALSE;
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [menu] element", pXML->ToString() );
		}
	}

	return TRUE;
}

BOOL CSkin::LoadMenu(CXMLElement* pXML)
{
	CString strName = pXML->GetAttributeValue( L"name" );
	if ( strName.IsEmpty() )
	{
		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"No [name] attribute in [menu] element", pXML->ToString() );
		return FALSE;
	}

	CMenu* pOldMenu = NULL;
	if ( m_pMenus.Lookup( strName, pOldMenu ) )
	{
		ASSERT_VALID( pOldMenu );
		delete pOldMenu;
		m_pMenus.RemoveKey( strName );
	}

	auto_ptr< CMenu > pMenu( new CMenu() );
	ASSERT_VALID( pMenu.get() );
	if ( ! pMenu.get() )
		return FALSE;

	if ( pXML->GetAttributeValue( L"type", L"popup" ).CompareNoCase( L"bar" ) == 0 )
	{
		if ( ! pMenu->CreateMenu() )
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Cannot create menu", pXML->ToString() );
			return FALSE;
		}
	}
	else if ( ! pMenu->CreatePopupMenu() )
	{
		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Cannot create popup menu", pXML->ToString() );
		return FALSE;
	}

	if ( ! CreateMenu( pXML, pMenu->GetSafeHmenu() ) )
		return FALSE;

	m_pMenus.SetAt( strName, pMenu.release() );

	return TRUE;
}

CMenu* CSkin::CreatePopupMenu(LPCTSTR pszName)
{
	CMenu* pMenu = new CMenu();
	if ( pMenu )
	{
		if ( pMenu->CreatePopupMenu() )
		{
			m_pMenus.SetAt( pszName, pMenu );
			return pMenu;
		}
		delete pMenu;
	}
	return NULL;
}

BOOL CSkin::CreateMenu(CXMLElement* pRoot, HMENU hMenu)
{
	if ( const UINT nID = LookupCommandID( pRoot, L"id" ) )
	{
		VERIFY( SetMenuContextHelpId( hMenu, nID ) );
	}

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML	= pRoot->GetNextElement( pos );
		CString strText		= pXML->GetAttributeValue( L"text" );

		CheckExceptions( strText, TRUE );	// Brittish?

		int nAmp = strText.Find( L'_' );
		if ( nAmp >= 0 ) strText.SetAt( nAmp, L'&' );	// First occurance

		if ( pXML->IsNamed( L"item" ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strKeys = pXML->GetAttributeValue( L"shortcut" );

				if ( ! strKeys.IsEmpty() ) strText += L'\t' + strKeys;

				VERIFY( AppendMenu( hMenu, MF_STRING, nID, strText ) );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [id] attribute in menu [item] element", pXML->ToString() );
			}
		}
		else if ( pXML->IsNamed( L"menu" ) )
		{
			HMENU hSubMenu = ::CreatePopupMenu();
			ASSERT( hSubMenu );
			if ( ! CreateMenu( pXML, hSubMenu ) )
			{
				DestroyMenu( hSubMenu );
				return FALSE;
			}

			VERIFY( AppendMenu( hMenu, MF_STRING|MF_POPUP, (UINT_PTR)hSubMenu, strText ) );
		}
		else if ( pXML->IsNamed( L"separator" ) )
		{
			VERIFY( AppendMenu( hMenu, MF_SEPARATOR, ID_SEPARATOR, NULL ) );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [menu] element", pXML->ToString() );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin toolbars

BOOL CSkin::LoadNavBar(CXMLElement* pBase)
{
	CString strValue = pBase->GetAttributeValue( L"offset" );
	if ( ! strValue.IsEmpty() )
	{
		if ( _stscanf( strValue, L"%li,%li", &m_ptNavBarOffset.x, &m_ptNavBarOffset.y ) != 2 )
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Bad [offset] attribute in [navbar] element", pBase->ToString() );
	}

	strValue = pBase->GetAttributeValue( L"mode" );
	if ( strValue.IsEmpty() )
		strValue = pBase->GetAttributeValue( L"case" );

	if ( strValue.CompareNoCase( L"upper" ) == 0 )
		m_NavBarMode = NavBarUpper;
	else if ( strValue.CompareNoCase( L"lower" ) == 0 )
		m_NavBarMode = NavBarLower;
	else
		m_NavBarMode = NavBarNormal;

	return TRUE;
}

BOOL CSkin::CreateToolBar(LPCTSTR pszName, CCoolBarCtrl* pBar)
{
	//CQuickLock oLock( m_pSection );

	ASSERT( pszName );
	ASSERT( Settings.General.GUIMode == GUI_TABBED || Settings.General.GUIMode == GUI_BASIC || Settings.General.GUIMode == GUI_WINDOWED );

	if ( pszName == NULL )
		return FALSE;

	if ( pBar->m_hWnd )
	{
		for ( CWnd* pChild = pBar->GetWindow( GW_CHILD ) ; pChild ; pChild = pChild->GetNextWindow() )
		{
			pChild->ShowWindow( SW_HIDE );
		}
	}
	pBar->SetWatermark( NULL, TRUE );
	pBar->Clear();

	CCoolBarCtrl* pBase = NULL;
	CString strClassName( pszName );

	if ( m_pToolbars.Lookup( strClassName + m_pszGUIMode[Settings.General.GUIMode], pBase ) || 
		 ( Settings.General.GUIMode == GUI_BASIC && m_pToolbars.Lookup( strClassName + m_pszGUIMode[ GUI_TABBED ], pBase ) ) || 
		 m_pToolbars.Lookup( strClassName, pBase ) )
	{
		//if ( StartsWith( strClassName, L"CLibraryHeaderBar" ) )	// Crash Workarounds
		//	; // Do nothing
		HBITMAP hBitmap = GetWatermark( strClassName + m_pszGUIMode[ Settings.General.GUIMode ] + L".Toolbar" );
		if ( ! hBitmap && Settings.General.GUIMode == GUI_BASIC )
			hBitmap = GetWatermark( strClassName + m_pszGUIMode[ GUI_TABBED ] + L".Toolbar" );
		if ( ! hBitmap )
			hBitmap = GetWatermark( strClassName + L".Toolbar" );
		if ( ! hBitmap && strClassName.Find( L'.', 5 ) > 1 )
			hBitmap = GetWatermark( strClassName.Left( strClassName.ReverseFind( L'.' ) ) + L".Toolbar" );
		if ( ! hBitmap && (HBITMAP)Images.m_bmToolbar )				// "System.Toolbars"
			hBitmap = (HBITMAP)Images.m_bmToolbar;

		if ( hBitmap )
			pBar->SetWatermark( hBitmap );

		pBar->Copy( pBase );
		return TRUE;
	}

	// Legacy method:
//	LPCTSTR* pszModeSuffix = m_pszModeSuffix[ Settings.General.GUIMode ];
//	for ( int nModeTry = 0 ; nModeTry < 3 && m_pszModeSuffix[ Settings.General.GUIMode ][ nModeTry ] ; nModeTry++ )
//	{
//		CString strName( strClassName + pszModeSuffix[ nModeTry ] );
//		if ( m_pToolbars.Lookup( strName, pBase ) )
//		{
//			//if ( StartsWith( strClassName, L"CLibraryHeaderBar" ) )	// Crash Workarounds
//			//	; // Do nothing
//			if ( HBITMAP hBitmap = GetWatermark( strName + L".Toolbar" ) )
//				pBar->SetWatermark( hBitmap );
//			else if ( HBITMAP hBitmap = GetWatermark( L"System.Toolbars." + strClassName ) )
//				pBar->SetWatermark( hBitmap );
//			else if ( (HBITMAP)Images.m_bmToolbar )		// "System.Toolbars"
//				pBar->SetWatermark( (HBITMAP)Images.m_bmToolbar, TRUE );
//
//			pBar->Copy( pBase );
//			return TRUE;
//		}
//	}

#ifdef _DEBUG
	//ASSERT( pBase == NULL );
	theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Toolbar Lookup", strClassName );
#endif
	return FALSE;
}

CCoolBarCtrl* CSkin::GetToolBar(LPCTSTR pszName) const
{
	//CQuickLock oLock( m_pSection );

	ASSERT( pszName );
	ASSERT( Settings.General.GUIMode == GUI_TABBED || Settings.General.GUIMode == GUI_BASIC || Settings.General.GUIMode == GUI_WINDOWED );

	CCoolBarCtrl* pBar = NULL;
	CString strName( pszName );

	if ( m_pToolbars.Lookup( strName + m_pszGUIMode[ Settings.General.GUIMode ], pBar ) )
		return pBar;
	if ( Settings.General.GUIMode == GUI_BASIC && m_pToolbars.Lookup( strName + m_pszGUIMode[ GUI_TABBED ], pBar ) )
		return pBar;
	if ( m_pToolbars.Lookup( strName, pBar ) )
		return pBar;

	// Legacy method:
	//LPCTSTR* pszModeSuffix = m_pszModeSuffix[ Settings.General.GUIMode ];
	//for ( int nModeTry = 0 ; m_pszModeSuffix[ Settings.General.GUIMode ][ nModeTry ] ; nModeTry++ )
	//{
	//	if ( m_pToolbars.Lookup( strName + m_pszModeSuffix[ Settings.General.GUIMode ][ nModeTry ], pBar ) )
	//		return pBar;
	//}

	return NULL;
}

BOOL CSkin::LoadToolbars(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"toolbar" ) )
		{
			if ( ! CreateToolBar( pXML ) )
				return FALSE;
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [toolbars] element", pXML->ToString() );
		}
	}

	return TRUE;
}

CCoolBarCtrl* CSkin::CreateToolBar(LPCTSTR pszName)
{
	//CQuickLock oLock( m_pSection );

	CCoolBarCtrl* pBar = new CCoolBarCtrl();
	if ( pBar )
	{
		m_pToolbars.SetAt( pszName, pBar );
		return pBar;
	}
	return NULL;
}

BOOL CSkin::CreateToolBar(CXMLElement* pBase)
{
	CCoolBarCtrl* pBar = new CCoolBarCtrl();

	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"button" ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strValue = pXML->GetAttributeValue( L"text" );
				CheckExceptions( strValue, TRUE );	// Brittish?

				CCoolBarItem* pItem = pBar->Add( nID, strValue );

				strValue = pXML->GetAttributeValue( L"color" );
				if ( strValue.IsEmpty() )
					strValue = pXML->GetAttributeValue( L"colour" );

				if ( ! strValue.IsEmpty() )
					pItem->m_crText = GetColor( strValue );

				strValue = pXML->GetAttributeValue( L"tip" );
				if ( ! strValue.IsEmpty() )
					pItem->SetTip( strValue );

				strValue = pXML->GetAttributeValue( L"visible", L"true" );
				if ( strValue.CompareNoCase( L"false" ) == 0 )
					pItem->Show( FALSE );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [id] attribute in [button] element", pXML->ToString() );
			}
		}
		else if ( pXML->IsNamed( L"separator" ) )
		{
			pBar->Add( ID_SEPARATOR );
		}
		else if ( pXML->IsNamed( L"right" ) || pXML->IsNamed( L"rightalign" ) )
		{
			pBar->Add( UINT( ID_RIGHTALIGN ) );
		}
		else if ( pXML->IsNamed( L"control" ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strTemp = pXML->GetAttributeValue( L"width" );
				UINT nWidth, nHeight = 0;

				if ( _stscanf( strTemp, L"%u", &nWidth ) == 1 )
				{
					strTemp = pXML->GetAttributeValue( L"height" );
					_stscanf( strTemp, L"%u", &nHeight );

					CCoolBarItem* pItem = pBar->Add( nID, nWidth, nHeight );
					if ( pItem )
					{
						strTemp = pXML->GetAttributeValue( L"checked", L"false" );

						if ( strTemp.CompareNoCase( L"true" ) == 0 )
						{
							pItem->m_bCheckButton = TRUE;
							pItem->m_bEnabled = FALSE;
						}

						strTemp = pXML->GetAttributeValue( L"text" );
						CheckExceptions( strTemp, TRUE );	// Brittish?
						pItem->SetText( strTemp );
					}
				}
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [id] attribute in [control] element", pXML->ToString() );
			}
		}
		else if ( pXML->IsNamed( L"label" ) )
		{
			CCoolBarItem* pItem = pBar->Add( 1, pXML->GetAttributeValue( L"text" ) );
			pItem->m_crText = 0;
			pItem->SetTip( pXML->GetAttributeValue( L"tip" ) );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [toolbar] element", pXML->ToString() );
		}
	}

	CString strName = pBase->GetAttributeValue( L"name" );

	CCoolBarCtrl* pOld = NULL;
	if ( m_pToolbars.Lookup( strName, pOld ) && pOld ) delete pOld;

	m_pToolbars.SetAt( strName, pBar );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin documents

BOOL CSkin::LoadDocuments(CXMLElement* pBase)
{
	for ( POSITION posDoc = pBase->GetElementIterator() ; posDoc ; )
	{
		CXMLElement* pDoc = pBase->GetNextElement( posDoc );

		if ( pDoc->IsNamed( L"document" ) )
		{
			CString strName = pDoc->GetAttributeValue( L"name" );

			CXMLElement* pOld = NULL;
			if ( m_pDocuments.Lookup( strName, pOld ) ) delete pOld;

			m_pDocuments.SetAt( strName, pDoc->Detach() );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [documents] element", pDoc->ToString() );
		}
	}

	return TRUE;
}

CXMLElement* CSkin::GetDocument(LPCTSTR pszName)
{
	//CQuickLock oLock( m_pSection );

	CXMLElement* pXML = NULL;

	if ( m_pDocuments.Lookup( pszName, pXML ) ) return pXML;

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSkin watermarks

HBITMAP CSkin::GetWatermark(LPCTSTR pszName)
{
	//CQuickLock oLock( m_pSection );

	CString strPath;
	if ( m_pWatermarks.Lookup( pszName, strPath ) && ! strPath.IsEmpty() )
	{
		if ( HBITMAP hBitmap = LoadBitmap( strPath ) )
			return hBitmap;

		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed to load watermark", (LPCTSTR)( CString( pszName ) + L". File: " + strPath ) );
	}
	return NULL;
}

BOOL CSkin::GetWatermark(CBitmap* pBitmap, LPCTSTR pszName)
{
	ASSERT( pBitmap != NULL );
	if ( pBitmap->m_hObject != NULL ) pBitmap->DeleteObject();

	HBITMAP hBitmap = GetWatermark( pszName );
	if ( hBitmap != NULL ) pBitmap->Attach( hBitmap );
	return ( hBitmap != NULL );
}

BOOL CSkin::LoadWatermarks(CXMLElement* pSub, const CString& strPath)
{
	for ( POSITION posMark = pSub->GetElementIterator() ; posMark ; )
	{
		CXMLElement* pMark = pSub->GetNextElement( posMark );

		if ( pMark->IsNamed( L"watermark" ) || pMark->IsNamed( L"image" ) )
		{
			CString strName = pMark->GetAttributeValue( L"target" );
			CString strFile = pMark->GetAttributeValue( L"path" );

			if ( ! strName.IsEmpty() )
			{
				if ( ! strFile.IsEmpty() )
					strFile = strPath + strFile;
				m_pWatermarks.SetAt( strName, strFile );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Missing [target] attribute in [watermark] element", pMark->ToString() );
			}
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [watermarks] element", pMark->ToString() );
		}
	}

	// Common system-wide volatile bitmaps (buttons):
	Images.Load();

// Obsolete: Moved to Images

//	if ( m_bmSelected.m_hObject ) m_bmSelected.DeleteObject();
//	if ( HBITMAP hSelected = GetWatermark( L"System.Highlight" ) )
//		m_bmSelected.Attach( hSelected );
//	else if ( HBITMAP hSelected = GetWatermark( L"CTransfers.Selected" ) )
//		m_bmSelected.Attach( hSelected );
//
//	if ( m_bmSelectedGrey.m_hObject ) m_bmSelectedGrey.DeleteObject();
//	if ( HBITMAP hSelected = GetWatermark( L"System.Highlight.Inactive" ) )
//		m_bmSelectedGrey.Attach( hSelected );
//	else if ( HBITMAP hSelected = GetWatermark( L"CTransfers.Selected.Inactive" ) )
//		m_bmSelectedGrey.Attach( hSelected );
//
//	if ( m_bmToolTip.m_hObject ) m_bmToolTip.DeleteObject();
//	if ( HBITMAP hToolTip = GetWatermark( L"System.ToolTip" ) )
//		m_bmToolTip.Attach( hToolTip );
//	else if ( HBITMAP hToolTip = GetWatermark( L"System.Tooltips" ) )
//		m_bmToolTip.Attach( hToolTip );
//
//	if ( m_bmDialog.m_hObject ) m_bmDialog.DeleteObject();
//	if ( HBITMAP hDialog = GetWatermark( L"System.Dialogs" ) )
//		m_bmDialog.Attach( hDialog );
//	else if ( HBITMAP hDialog = GetWatermark( L"CDialog" ) )
//		m_bmDialog.Attach( hDialog );
//
//	if ( m_bmDialogPanel.m_hObject ) m_bmDialogPanel.DeleteObject();
//	if ( HBITMAP hDialog = GetWatermark( L"System.DialogPanels" ) )
//		m_bmDialogPanel.Attach( hDialog );
//	else if ( HBITMAP hDialog = GetWatermark( L"CDialog.Panel" ) )
//		m_bmDialogPanel.Attach( hDialog );
//
//	if ( m_bmPanelMark.m_hObject != NULL ) m_bmPanelMark.DeleteObject();
//	if ( HBITMAP hPanelMark = GetWatermark( L"CPanelWnd.Caption" ) )
//		m_bmPanelMark.Attach( hPanelMark );
//	else if ( Colors.m_crPanelBack == RGB_DEFAULT_CASE )
//		m_bmPanelMark.LoadBitmap( IDB_PANEL_MARK );				// Default resource handling

	// Related brushes:

//	// Skinnable Dialogs  (This brush applies to text bg.  Body in DlgSkinDialog, WndSettingPage, etc.)
//	if ( Images.m_brDialog.m_hObject ) Images.m_brDialog.DeleteObject();
//	if ( Images.m_bmDialog.m_hObject )
//		Images.m_brDialog.CreatePatternBrush( & Images.m_bmDialog );	//Attach( (HBRUSH)GetStockObject( NULL_BRUSH ) );
//	else
//		Images.m_brDialog.CreateSolidBrush( Colors.m_crDialog );
//
//	if ( Images.m_brDialogPanel.m_hObject ) Images.m_brDialogPanel.DeleteObject();
//	if ( Images.m_bmDialogPanel.m_hObject )
//		Images.m_brDialogPanel.CreatePatternBrush( & Images.m_bmDialogPanel );
//	else
//		Images.m_brDialogPanel.CreateSolidBrush( Colors.m_crDialogPanel );
//
//	if ( Images.m_brMediaSlider.m_hObject ) Images.m_brMediaSlider.DeleteObject();
//	if ( HBITMAP hSlider = GetWatermark( L"CCoolbar.Control" ) )
//	{
//		CBitmap bmSlider;
//		bmSlider.Attach( hSlider );
//		Images.m_brMediaSlider.CreatePatternBrush( &bmSlider );
//	}
//	else if ( HBITMAP hSlider = GetWatermark( L"CMediaFrame.Slider" ) )
//	{
//		CBitmap bmSlider;
//		bmSlider.Attach( hSlider );
//		Images.m_brMediaSlider.CreatePatternBrush( &bmSlider );
//	}
//	else
//		Images.m_brMediaSlider.CreateSolidBrush( Colors.m_crMidtone );

	// Dialog Header:

//	m_nBanner = 0;	// Defined in Images
//	if ( m_bmBanner.m_hObject ) m_bmBanner.DeleteObject();
//	if ( HBITMAP hBanner = GetWatermark( L"System.Header" ) )
//	{
//		BITMAP bmInfo;
//		m_bmBanner.Attach( hBanner );
//		m_bmBanner.GetObject( sizeof( BITMAP ), &bmInfo );
//		m_bmBanner.SetBitmapDimension( bmInfo.bmWidth, bmInfo.bmHeight );
//		m_nBanner = bmInfo.bmHeight;
//	}
//	else if ( HBITMAP hBanner = GetWatermark( L"Banner" ) )
//	{
//		BITMAP bmInfo;
//		m_bmBanner.Attach( hBanner );
//		m_bmBanner.GetObject( sizeof( BITMAP ), &bmInfo );
//		m_bmBanner.SetBitmapDimension( bmInfo.bmWidth, bmInfo.bmHeight );
//		m_nBanner = bmInfo.bmHeight;
//	}

	// "System.Toolbars" fallback at toolbar creation
	// Button states in CImages

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin list column translations

BOOL CSkin::Translate(LPCTSTR pszName, CHeaderCtrl* pCtrl)
{
	CString strEdit;

	if ( ! m_pLists.Lookup( pszName, strEdit ) ) return FALSE;

	TCHAR szColumn[128] = {};
	HD_ITEM pColumn = {};

	if ( Settings.General.LanguageRTL )
		pCtrl->ModifyStyleEx( 0, WS_EX_LAYOUTRTL, 0 );
	pColumn.mask		= HDI_TEXT;
	pColumn.pszText		= szColumn;
	pColumn.cchTextMax	= 126;

	for ( int nItem = 0 ; nItem < pCtrl->GetItemCount() ; nItem++ )
	{
		*szColumn = L'\0';
		pCtrl->GetItem( nItem, &pColumn );

		_tcscat( szColumn, L"=" );

		LPCTSTR pszFind = _tcsistr( strEdit, szColumn );

		if ( pszFind )
		{
			pszFind += _tcslen( szColumn );

			CString strNew = pszFind;
			strNew = strNew.SpanExcluding( L"|" );

			_tcsncpy( szColumn, strNew, _countof( szColumn ) );
			pCtrl->SetItem( nItem, &pColumn );
		}
	}

	return TRUE;
}

CString CSkin::GetHeaderTranslation(LPCTSTR pszClassName, LPCTSTR pszHeaderName)
{
	CString strEdit;
	if ( ! m_pLists.Lookup( pszClassName, strEdit ) )
		return CString( pszHeaderName );

	CString strOriginal( pszHeaderName );
	strOriginal += L"=";
	LPCTSTR pszFind = _tcsistr( strEdit, strOriginal );

	if ( pszFind )
	{
		pszFind += strOriginal.GetLength();
		CString strNew = pszFind;
		strNew = strNew.SpanExcluding( L"|" );
		return strNew;
	}
	return CString( pszHeaderName );
}

BOOL CSkin::LoadListColumns(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"list" ) )
		{
			CString strName = pXML->GetAttributeValue( L"name" );
			if ( strName.IsEmpty() ) continue;

			CString strEdit;
			for ( POSITION posCol = pXML->GetElementIterator() ; posCol ; )
			{
				CXMLElement* pCol = pXML->GetNextElement( posCol );
				if ( pCol->IsNamed( L"column" ) )
				{
					CString strFrom	= pCol->GetAttributeValue( L"from" );
					CString strTo	= pCol->GetAttributeValue( L"to" );
					if ( strTo.IsEmpty() )
						strTo = pCol->GetAttributeValue( L"text" );		// Standard (unused)

					if ( strFrom.IsEmpty() || strTo.IsEmpty() ) continue;

					if ( ! strEdit.IsEmpty() ) strEdit += L'|';
					strEdit += strFrom + L'=' + strTo;
				}
				else
				{
					theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [list] element", pCol->ToString() );
				}
			}

			m_pLists.SetAt( strName, strEdit );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [listColumns] element", pXML->ToString() );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin dialogs

BOOL CSkin::Apply(LPCTSTR pszName, CDialog* pDialog, UINT nIconID, CToolTipCtrl* pWndTooltips)
{
	if ( nIconID )
		CoolInterface.SetIcon( nIconID, FALSE, FALSE, pDialog );

	CString strName;

	if ( pszName )
		strName = pszName;
	else
		strName = pDialog->GetRuntimeClass()->m_lpszClassName;

	if ( Settings.General.DialogScan )
		return Dialogscan( pDialog, strName );

	CXMLElement* pBase = NULL;
	if ( ! m_pDialogs.Lookup( strName, pBase ) )
		return FALSE;	// Naked dialog

	CString strCookie = GetDialogCookie( pDialog, pWndTooltips );	// Also parses default tips

	if ( strCookie != pBase->GetAttributeValue( L"cookie" ) )
	{
		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Invalid [cookie] attribute in [dialog] element, use: " + strCookie, pBase->ToString() );
		return FALSE;
	}

	CString strTip;
	CString strCaption = pBase->GetAttributeValue( L"caption" );
	if ( strCaption.IsEmpty() )
		strCaption = pBase->GetAttributeValue( L"title" );
	if ( strCaption.IsEmpty() )
		strCaption = pBase->GetAttributeValue( L"text" );
	if ( ! strCaption.IsEmpty() )
		pDialog->SetWindowText( strCaption );

	CWnd* pWnd = pDialog->GetWindow( GW_CHILD );

	for ( POSITION pos = pBase->GetElementIterator() ; pos && pWnd ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		TCHAR szClass[3] = { 0, 0, 0 };
		GetClassName( pWnd->GetSafeHwnd(), szClass, 3 );

		// Skip added banner
		if ( _tcsnicmp( szClass, L"St", 3 ) == 0 &&
			IDC_BANNER == pWnd->GetDlgCtrlID() )
		{
			pWnd = pWnd->GetNextWindow();
			if ( ! pWnd )
				break;
		}

		// Needed for some controls like Schema combo box
		if ( Settings.General.LanguageRTL && (CString)szClass != "Ed" )
			pWnd->ModifyStyleEx( 0, WS_EX_LAYOUTRTL|WS_EX_RTLREADING, 0 );

		if ( pXML->IsNamed( L"control" ) )
		{
			if ( pWndTooltips )
			{
				strTip = pXML->GetAttributeValue( L"tip" );
				if ( ! strTip.IsEmpty() )
					pWndTooltips->AddTool( pWnd, strTip );
			}

			strCaption = pXML->GetAttributeValue( L"text" );
			if ( strCaption.IsEmpty() )
				strCaption = pXML->GetAttributeValue( L"caption" );		// Deprecated
			if ( ! strCaption.IsEmpty() )
			{
				strCaption.Replace( L"{n}", L"\r\n" );

				if ( (CString)szClass != "Co" )
				{
					int nPos = strCaption.Find( L'_' );
					if ( nPos >= 0 ) strCaption.SetAt( nPos, L'&' );
					pWnd->SetWindowText( strCaption );
				}
				else
				{
					CComboBox* pCombo = (CComboBox*)pWnd;
					int nNum = pCombo->GetCount();

					CStringArray pItems;
					Split( strCaption, L'|', pItems, TRUE );

					if ( nNum == pItems.GetSize() )
					{
						int nCurSel = pCombo->GetCurSel();
						pCombo->ResetContent();
						for ( int i = 0 ; i < nNum ; ++i )
						{
							pCombo->AddString( pItems.GetAt( i ) );
						}
						pCombo->SetCurSel( nCurSel );
					}
				}
			}

			pWnd = pWnd->GetNextWindow();
		}
	}

	return TRUE;
}

BOOL CSkin::Dialogscan(CDialog* pDialog, CString sName /*=""*/)
{
	CStdioFile pFile;

	if ( pFile.Open( Settings.General.Path + L"\\Dialogs.xml", CFile::modeReadWrite ) )
		pFile.Seek( 0, CFile::end );
	else if ( pFile.Open( Settings.General.Path + L"\\Dialogs.xml", CFile::modeWrite|CFile::modeCreate ) )
		pFile.WriteString( L"<dialogs>\r\n" );
	else
		return FALSE;

	// Obsolete CFile method, for reference:
	//pFile.Write( "\t<dialog name=\"", 15 );
	//int nBytes = WideCharToMultiByte( CP_ACP, 0, strName, strName.GetLength(), NULL, 0, NULL, NULL );
	//LPSTR pBytes = new CHAR[nBytes];
	//WideCharToMultiByte( CP_ACP, 0, strName, strName.GetLength(), pBytes, nBytes, NULL, NULL );
	//pFile.Write( pBytes, nBytes );
	//delete [] pBytes;

	if ( sName.IsEmpty() )
		sName = pDialog->GetRuntimeClass()->m_lpszClassName;

	CString strCaption, strTip;
	pDialog->GetWindowText( strCaption );
	strCaption.Replace( L"\n", L"{n}" );
	strCaption.Replace( L"\r", L"" );
	strCaption.Replace( L"&", L"_" );
	strCaption = Escape( strCaption );

	pFile.WriteString( L"\t<dialog name=\"" );
	pFile.WriteString( sName );

	pFile.WriteString( L"\" cookie=\"" );
	pFile.WriteString( GetDialogCookie( pDialog ) );

	pFile.WriteString( L"\" caption=\"" );
	pFile.WriteString( strCaption );

	pFile.WriteString( L"\">\r\n" );

	for ( CWnd* pWnd = pDialog->GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		TCHAR szClass[64];
		GetClassName( pWnd->GetSafeHwnd(), szClass, 64 );

		// Skip added banner
		if ( _tcsnicmp( szClass, L"St", 3 ) == 0 &&
			 pWnd->GetDlgCtrlID() == IDC_BANNER )
			continue;

		strCaption.Empty();
		strTip.Empty();
		LoadControlTip( strTip, pWnd->GetDlgCtrlID() );

		if ( _tcsistr( szClass, L"Static" ) ||
			 _tcsistr( szClass, L"Button" ) )
		{
			pWnd->GetWindowText( strCaption );
		}
		else if ( _tcsistr( szClass, L"ListBox" ) )
		{
			CListBox* pListBox = static_cast< CListBox* >( pWnd );
			for ( int i = 0 ; i < pListBox->GetCount() ; ++i )
			{
				CString strTemp;
				pListBox->GetText( i, strTemp );
				if ( ! strCaption.IsEmpty() )
					strCaption += L'|';
				strCaption += strTemp;
			}
		}
		else if ( _tcsistr( szClass, L"ComboBox" ) )
		{
			CComboBox* pComboBox = static_cast< CComboBox* >( pWnd );
			for ( int i = 0 ; i < pComboBox->GetCount() ; ++i )
			{
				CString strTemp;
				pComboBox->GetLBText( i, strTemp );
				if ( ! strCaption.IsEmpty() )
					strCaption += L'|';
				strCaption += strTemp;
			}
		}

		pFile.WriteString( L"\t\t<control" );

		if ( ! strCaption.IsEmpty() )
		{
			strCaption.Replace( L"\n", L"{n}" );
			strCaption.Replace( L"\r", L"" );
			strCaption.Replace( L"&", L"_" );
			strCaption = Escape( strCaption );
			pFile.WriteString( L" caption=\"" );
			pFile.WriteString( strCaption );
			pFile.WriteString( L"\"" );
		}

		if ( ! strTip.IsEmpty() )
		{
			strTip.Replace( L"\n", L"{n}" );
			strTip.Replace( L"\r", L"" );
			pFile.WriteString( L" tip=\"" );
			pFile.WriteString( strTip );
			pFile.WriteString( L"\"" );
		}

		pFile.WriteString( L"/>\r\n" );
	}

	pFile.WriteString( L"\t</dialog>\r\n" );
	//pFile.Close();

	return TRUE;
}

CString CSkin::GetDialogCookie(CDialog* pDialog, CToolTipCtrl* pWndTooltips /*=NULL*/)
{
	CString strCookie, strTip;

	for ( CWnd* pWnd = pDialog->GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		pWnd->SetFont( &CoolInterface.m_fntNormal );

		if ( pWndTooltips )
		{
			LoadControlTip( strTip, pWnd->GetDlgCtrlID() );
			if ( ! strTip.IsEmpty() )
				pWndTooltips->AddTool( pWnd, strTip );
		}

		TCHAR szClass[3] = { 0, 0, 0 };
		GetClassName( pWnd->GetSafeHwnd(), szClass, 3 );

		// Skip added banner
		if ( _tcsnicmp( szClass, L"St", 3 ) == 0 &&
			 pWnd->GetDlgCtrlID() == IDC_BANNER )
			continue;

		// Skip settings pages
		if ( pWnd->IsKindOf( RUNTIME_CLASS( CSettingsPage ) ) )
			continue;

		strCookie += szClass;	// Cookie
	}

	return strCookie;
}

CString CSkin::GetDialogCaption(LPCTSTR pszName)
{
	//CQuickLock oLock( m_pSection );

	CXMLElement* pBase = NULL;
	CString strCaption;

	if ( m_pDialogs.Lookup( pszName, pBase ) )
	{
		strCaption = pBase->GetAttributeValue( L"text" );
		if ( strCaption.IsEmpty() )
			strCaption = pBase->GetAttributeValue( L"caption" );	// Deprecated
	}

	return strCaption;
}

BOOL CSkin::LoadDialogs(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"dialog" ) )
		{
			CString strName = pXML->GetAttributeValue( L"name" );
			CXMLElement* pOld;

			if ( m_pDialogs.Lookup( strName, pOld ) )
				delete pOld;

			pXML->Detach();
			m_pDialogs.SetAt( strName, pXML );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [dialogs] element", pXML->ToString() );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin window skins

CSkinWindow* CSkin::GetWindowSkin(LPCTSTR pszWindow, LPCTSTR pszAppend)
{
	//CQuickLock oLock( m_pSection );

	CString strWindow;
	strWindow.Format( L"|%s%s|", pszWindow, pszAppend ? pszAppend : L"" );

	for ( POSITION pos = m_pSkins.GetHeadPosition() ; pos ; )
	{
		CSkinWindow* pSkin = m_pSkins.GetNext( pos );
		if ( pSkin->m_sTargets.Find( strWindow ) >= 0 )
			return pSkin;
	}

	return NULL;
}

CSkinWindow* CSkin::GetWindowSkin(CWnd* pWnd)
{
	ASSERT( pWnd != NULL );
	ASSERT( Settings.General.GUIMode == GUI_TABBED || Settings.General.GUIMode == GUI_BASIC || Settings.General.GUIMode == GUI_WINDOWED );
	BOOL bPanel = FALSE;

	if ( pWnd->IsKindOf( RUNTIME_CLASS(CChildWnd) ) )
	{
		CChildWnd* pChild = (CChildWnd*)pWnd;
		bPanel = pChild->m_bPanelMode;
	}

#ifdef _AFXDLL	// ToDo: Clean this up?
	for ( CRuntimeClass* pClass = pWnd->GetRuntimeClass() ; pClass && pClass->m_pfnGetBaseClass ; pClass = pClass->m_pfnGetBaseClass() )
#else
	for ( CRuntimeClass* pClass = pWnd->GetRuntimeClass() ; pClass ; pClass = pClass->m_pBaseClass )
#endif
	{
		CString strClassName( pClass->m_lpszClassName );

		if ( bPanel )
			if ( CSkinWindow* pSkin = GetWindowSkin( (LPCTSTR)strClassName, L".Panel" ) )
				return pSkin;

		if ( CSkinWindow* pSkin = GetWindowSkin( (LPCTSTR)strClassName, m_pszGUIMode[ Settings.General.GUIMode ] ) )
			return pSkin;

		if ( Settings.General.GUIMode == GUI_BASIC )
			if ( CSkinWindow* pSkin = GetWindowSkin( (LPCTSTR)strClassName, m_pszGUIMode[ GUI_TABBED ] ) )
				return pSkin;
		
		if ( CSkinWindow* pSkin = GetWindowSkin( (LPCTSTR)strClassName ) )
			return pSkin;

		// Legacy method:
	//	LPCTSTR* pszModeSuffix = m_pszModeSuffix[ Settings.General.GUIMode ];
	//	for ( int nSuffix = 0 ; nSuffix < 3 && pszModeSuffix[ nSuffix ] != NULL ; nSuffix++ )
	//	{
	//		if ( pszModeSuffix[ nSuffix ][0] != 0 || ! bPanel )
	//		{
	//			CSkinWindow* pSkin = GetWindowSkin( (LPCTSTR)strClassName, pszModeSuffix[ nSuffix ] );
	//			if ( pSkin != NULL ) return pSkin;
	//		}
	//	}
	}

	return NULL;
}

BOOL CSkin::LoadWindowSkins(CXMLElement* pSub, const CString& strPath)
{
	for ( POSITION posSkin = pSub->GetElementIterator() ; posSkin ; )
	{
		CXMLElement* pSkinElement = pSub->GetNextElement( posSkin );

		if ( pSkinElement->IsNamed( L"windowSkin" ) || pSkinElement->IsNamed( L"window" ) )
		{
			CSkinWindow* pSkin = new CSkinWindow();

			if ( pSkin->Parse( pSkinElement, strPath ) )
				m_pSkins.AddHead( pSkin );
			else
				delete pSkin;
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [windowSkins] element", pSkinElement->ToString() );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin Color Scheme

BOOL CSkin::LoadColorScheme(CXMLElement* pBase)
{
	CMapStringToPtr pColors;

	pColors.SetAt( L"system.base.window", &Colors.m_crWindow );
	pColors.SetAt( L"system.base.midtone", &Colors.m_crMidtone );
	pColors.SetAt( L"system.base.text", &Colors.m_crText );
	pColors.SetAt( L"system.base.hitext", &Colors.m_crHiText );
	pColors.SetAt( L"system.base.hiborder", &Colors.m_crHiBorder );
	pColors.SetAt( L"system.base.hiborderin", &Colors.m_crHiBorderIn );
	pColors.SetAt( L"system.base.highlight", &Colors.m_crHighlight );
	pColors.SetAt( L"system.back.normal", &Colors.m_crBackNormal );
	pColors.SetAt( L"system.back.selected", &Colors.m_crBackSel );
	pColors.SetAt( L"system.back.checked", &Colors.m_crBackCheck );
	pColors.SetAt( L"system.back.checked.selected", &Colors.m_crBackCheckSel );
	pColors.SetAt( L"system.margin", &Colors.m_crMargin );
	pColors.SetAt( L"system.border", &Colors.m_crBorder );
	pColors.SetAt( L"system.shadow", &Colors.m_crShadow );
	pColors.SetAt( L"system.text", &Colors.m_crCmdText );
	pColors.SetAt( L"system.text.selected", &Colors.m_crCmdTextSel );
	pColors.SetAt( L"system.disabled", &Colors.m_crDisabled );

	pColors.SetAt( L"tooltip.back", &Colors.m_crTipBack );
	pColors.SetAt( L"tooltip.text", &Colors.m_crTipText );
	pColors.SetAt( L"tooltip.graph", &Colors.m_crTipGraph );
	pColors.SetAt( L"tooltip.grid", &Colors.m_crTipGraphGrid );
	pColors.SetAt( L"tooltip.border", &Colors.m_crTipBorder );
	pColors.SetAt( L"tooltip.warnings", &Colors.m_crTipWarnings );

	pColors.SetAt( L"taskpanel.back", &Colors.m_crTaskPanelBack );
	pColors.SetAt( L"taskbox.caption.back", &Colors.m_crTaskBoxCaptionBack );
	pColors.SetAt( L"taskbox.caption.text", &Colors.m_crTaskBoxCaptionText );
	pColors.SetAt( L"taskbox.caption.hover", &Colors.m_crTaskBoxCaptionHover );
	pColors.SetAt( L"taskbox.primary.back", &Colors.m_crTaskBoxPrimaryBack );	// Deprecate?
	pColors.SetAt( L"taskbox.primary.text", &Colors.m_crTaskBoxPrimaryText );	// Deprecate?
	pColors.SetAt( L"taskbox.client", &Colors.m_crTaskBoxClient );	// Deprecated
	pColors.SetAt( L"taskbox.back", &Colors.m_crTaskBoxClient );
	pColors.SetAt( L"taskbox.text", &Colors.m_crTaskBoxText );

	pColors.SetAt( L"dialog.back", &Colors.m_crDialog );
	pColors.SetAt( L"dialog.text", &Colors.m_crDialogText );
	pColors.SetAt( L"dialog.menu.back", &Colors.m_crDialogMenu );
	pColors.SetAt( L"dialog.menu.text", &Colors.m_crDialogMenuText );
	pColors.SetAt( L"dialog.panel.back", &Colors.m_crDialogPanel );
	pColors.SetAt( L"dialog.panel.text", &Colors.m_crDialogPanelText );
	pColors.SetAt( L"panel.caption.back", &Colors.m_crPanelBack );
	pColors.SetAt( L"panel.caption.text", &Colors.m_crPanelText );
	pColors.SetAt( L"panel.caption.border", &Colors.m_crPanelBorder );
	pColors.SetAt( L"banner.back", &Colors.m_crBannerBack );
	pColors.SetAt( L"banner.text", &Colors.m_crBannerText );
	pColors.SetAt( L"schema.row1", &Colors.m_crSchemaRow[0] );
	pColors.SetAt( L"schema.row2", &Colors.m_crSchemaRow[1] );

//	Active window color is controlled by media player plugin, thus we can not skin it?
	pColors.SetAt( L"media.window", &Colors.m_crMediaWindowBack );
	pColors.SetAt( L"media.window.back", &Colors.m_crMediaWindowBack );
	pColors.SetAt( L"media.window.text", &Colors.m_crMediaWindowText );
	pColors.SetAt( L"media.status", &Colors.m_crMediaStatusBack );
	pColors.SetAt( L"media.status.back", &Colors.m_crMediaStatusBack );
	pColors.SetAt( L"media.status.text", &Colors.m_crMediaStatusText );
	pColors.SetAt( L"media.panel", &Colors.m_crMediaPanelBack );
	pColors.SetAt( L"media.panel.back", &Colors.m_crMediaPanelBack );
	pColors.SetAt( L"media.panel.text", &Colors.m_crMediaPanelText );
	pColors.SetAt( L"media.panel.active", &Colors.m_crMediaPanelActiveBack );
	pColors.SetAt( L"media.panel.active.back", &Colors.m_crMediaPanelActiveBack );
	pColors.SetAt( L"media.panel.active.text", &Colors.m_crMediaPanelActiveText );
	pColors.SetAt( L"media.panel.caption", &Colors.m_crMediaPanelCaptionBack );
	pColors.SetAt( L"media.panel.caption.back", &Colors.m_crMediaPanelCaptionBack );
	pColors.SetAt( L"media.panel.caption.text", &Colors.m_crMediaPanelCaptionText );

	pColors.SetAt( L"traffic.window.back", &Colors.m_crTrafficWindowBack );
	pColors.SetAt( L"traffic.window.text", &Colors.m_crTrafficWindowText );
	pColors.SetAt( L"traffic.window.grid", &Colors.m_crTrafficWindowGrid );

	pColors.SetAt( L"monitor.history.back", &Colors.m_crMonitorHistoryBack );
	pColors.SetAt( L"monitor.history.back.max", &Colors.m_crMonitorHistoryBackMax );
	pColors.SetAt( L"monitor.history.text", &Colors.m_crMonitorHistoryText );
	pColors.SetAt( L"monitor.download.line", &Colors.m_crMonitorDownloadLine );
	pColors.SetAt( L"monitor.upload.line", &Colors.m_crMonitorUploadLine );
	pColors.SetAt( L"monitor.download.bar", &Colors.m_crMonitorDownloadBar );
	pColors.SetAt( L"monitor.upload.bar", &Colors.m_crMonitorUploadBar );
	pColors.SetAt( L"monitor.graph.border", &Colors.m_crMonitorGraphBorder );
	pColors.SetAt( L"monitor.graph.back", &Colors.m_crMonitorGraphBack );
	pColors.SetAt( L"monitor.graph.grid", &Colors.m_crMonitorGraphGrid );
	pColors.SetAt( L"monitor.graph.line", &Colors.m_crMonitorGraphLine );

	pColors.SetAt( L"schema.rating", &Colors.m_crRatingNull );
	pColors.SetAt( L"schema.rating0", &Colors.m_crRating0 );
	pColors.SetAt( L"schema.rating1", &Colors.m_crRating1 );
	pColors.SetAt( L"schema.rating2", &Colors.m_crRating2 );
	pColors.SetAt( L"schema.rating3", &Colors.m_crRating3 );
	pColors.SetAt( L"schema.rating4", &Colors.m_crRating4 );
	pColors.SetAt( L"schema.rating5", &Colors.m_crRating5 );

	pColors.SetAt( L"meta.row", &Colors.m_crSchemaRow[0] );
	pColors.SetAt( L"meta.row", &Colors.m_crSchemaRow[1] );
	pColors.SetAt( L"meta.row.alt", &Colors.m_crSchemaRow[1] );
	pColors.SetAt( L"meta.row.odd", &Colors.m_crSchemaRow[0] );
	pColors.SetAt( L"meta.row.even", &Colors.m_crSchemaRow[1] );
	pColors.SetAt( L"meta.rating",  &Colors.m_crRatingNull );
	pColors.SetAt( L"meta.rating0", &Colors.m_crRating0 );
	pColors.SetAt( L"meta.rating1", &Colors.m_crRating1 );
	pColors.SetAt( L"meta.rating2", &Colors.m_crRating2 );
	pColors.SetAt( L"meta.rating3", &Colors.m_crRating3 );
	pColors.SetAt( L"meta.rating4", &Colors.m_crRating4 );
	pColors.SetAt( L"meta.rating5", &Colors.m_crRating5 );

	pColors.SetAt( L"richdoc.back", &Colors.m_crRichdocBack );
	pColors.SetAt( L"richdoc.text", &Colors.m_crRichdocText );
	pColors.SetAt( L"richdoc.heading", &Colors.m_crRichdocHeading );

	pColors.SetAt( L"system.textalert", &Colors.m_crTextAlert );
	pColors.SetAt( L"system.textstatus", &Colors.m_crTextStatus );
	pColors.SetAt( L"system.textlink", &Colors.m_crTextLink );
	pColors.SetAt( L"system.textlink.selected", &Colors.m_crTextLinkHot );

	// Deprecated System.Base. first
	pColors.SetAt( L"system.base.chat.in", &Colors.m_crChatIn );
	pColors.SetAt( L"system.base.chat.out", &Colors.m_crChatOut );
	pColors.SetAt( L"system.base.chat.null", &Colors.m_crChatNull );
	pColors.SetAt( L"system.base.search.null", &Colors.m_crSearchNull );
	pColors.SetAt( L"system.base.search.exists", &Colors.m_crSearchExists );
	pColors.SetAt( L"system.base.search.exists.hit", &Colors.m_crSearchExistsHit );
	pColors.SetAt( L"system.base.search.exists.selected", &Colors.m_crSearchExistsSelected );
	pColors.SetAt( L"system.base.search.queued", &Colors.m_crSearchQueued );
	pColors.SetAt( L"system.base.search.queued.hit", &Colors.m_crSearchQueuedHit );
	pColors.SetAt( L"system.base.search.queued.selected", &Colors.m_crSearchQueuedSelected );
	pColors.SetAt( L"system.base.search.ghostrated", &Colors.m_crSearchGhostrated );
	pColors.SetAt( L"system.base.search.highrated", &Colors.m_crSearchHighrated );
	pColors.SetAt( L"system.base.search.collection", &Colors.m_crSearchCollection );
	pColors.SetAt( L"system.base.search.torrent", &Colors.m_crSearchTorrent );
	pColors.SetAt( L"system.base.transfer.source", &Colors.m_crTransferSource );
	pColors.SetAt( L"system.base.transfer.ranges", &Colors.m_crTransferRanges );
	pColors.SetAt( L"system.base.transfer.completed", &Colors.m_crTransferCompleted );
	pColors.SetAt( L"system.base.transfer.seeding", &Colors.m_crTransferVerifyPass );
	pColors.SetAt( L"system.base.transfer.failed", &Colors.m_crTransferVerifyFail );
	pColors.SetAt( L"system.base.transfer.completed.selected", &Colors.m_crTransferCompletedSelected );
	pColors.SetAt( L"system.base.transfer.seeding.selected", &Colors.m_crTransferVerifyPassSelected );
	pColors.SetAt( L"system.base.transfer.failed.selected", &Colors.m_crTransferVerifyFailSelected );
	pColors.SetAt( L"system.base.network.null", &Colors.m_crNetworkNull );
	pColors.SetAt( L"system.base.network.gnutella", &Colors.m_crNetworkG1 );
	pColors.SetAt( L"system.base.network.gnutella2", &Colors.m_crNetworkG2 );
	pColors.SetAt( L"system.base.network.g1", &Colors.m_crNetworkG1 );
	pColors.SetAt( L"system.base.network.g2", &Colors.m_crNetworkG2 );
	pColors.SetAt( L"system.base.network.ed2k", &Colors.m_crNetworkED2K );
	pColors.SetAt( L"system.base.network.dc", &Colors.m_crNetworkDC );
	pColors.SetAt( L"system.base.network.up", &Colors.m_crNetworkUp );
	pColors.SetAt( L"system.base.network.down", &Colors.m_crNetworkDown );
	pColors.SetAt( L"system.base.network.in", &Colors.m_crNetworkDown );
	pColors.SetAt( L"system.base.network.out", &Colors.m_crNetworkUp );
	pColors.SetAt( L"system.base.security.allow", &Colors.m_crSecurityAllow );
	pColors.SetAt( L"system.base.security.deny", &Colors.m_crSecurityDeny );

	// Preferred System. second
	pColors.SetAt( L"system.chat.in", &Colors.m_crChatIn );
	pColors.SetAt( L"system.chat.out", &Colors.m_crChatOut );
	pColors.SetAt( L"system.chat.null", &Colors.m_crChatNull );
	pColors.SetAt( L"system.search.null", &Colors.m_crSearchNull );
	pColors.SetAt( L"system.search.exists", &Colors.m_crSearchExists );
	pColors.SetAt( L"system.search.exists.hit", &Colors.m_crSearchExistsHit );
	pColors.SetAt( L"system.search.exists.selected", &Colors.m_crSearchExistsSelected );
	pColors.SetAt( L"system.search.queued", &Colors.m_crSearchQueued );
	pColors.SetAt( L"system.search.queued.hit", &Colors.m_crSearchQueuedHit );
	pColors.SetAt( L"system.search.queued.selected", &Colors.m_crSearchQueuedSelected );
	pColors.SetAt( L"system.search.ghostrated", &Colors.m_crSearchGhostrated );
	pColors.SetAt( L"system.search.highrated", &Colors.m_crSearchHighrated );
	pColors.SetAt( L"system.search.collection", &Colors.m_crSearchCollection );
	pColors.SetAt( L"system.search.torrent", &Colors.m_crSearchTorrent );
	pColors.SetAt( L"system.transfer.source", &Colors.m_crTransferSource );
	pColors.SetAt( L"system.transfer.ranges", &Colors.m_crTransferRanges );
	pColors.SetAt( L"system.transfer.completed", &Colors.m_crTransferCompleted );
	pColors.SetAt( L"system.transfer.seeding", &Colors.m_crTransferVerifyPass );
	pColors.SetAt( L"system.transfer.failed", &Colors.m_crTransferVerifyFail );
	pColors.SetAt( L"system.transfer.completed.selected", &Colors.m_crTransferCompletedSelected );
	pColors.SetAt( L"system.transfer.seeding.selected", &Colors.m_crTransferVerifyPassSelected );
	pColors.SetAt( L"system.transfer.failed.selected", &Colors.m_crTransferVerifyFailSelected );
	pColors.SetAt( L"system.library", &Colors.m_crLibraryShared );
	pColors.SetAt( L"system.library.shared", &Colors.m_crLibraryShared );
	pColors.SetAt( L"system.library.unshared", &Colors.m_crLibraryUnshared );
	pColors.SetAt( L"system.library.unscanned", &Colors.m_crLibraryUnscanned );
	pColors.SetAt( L"system.library.unsafe", &Colors.m_crLibraryUnsafe );
	pColors.SetAt( L"system.log.debug", &Colors.m_crLogDebug );
	pColors.SetAt( L"system.log.info", &Colors.m_crLogInfo );
	pColors.SetAt( L"system.log.notice", &Colors.m_crLogNotice );
	pColors.SetAt( L"system.log.warning", &Colors.m_crLogWarning );
	pColors.SetAt( L"system.log.error", &Colors.m_crLogError );
	pColors.SetAt( L"system.network.null", &Colors.m_crNetworkNull );
	pColors.SetAt( L"system.network.gnutella", &Colors.m_crNetworkG1 );
	pColors.SetAt( L"system.network.gnutella2", &Colors.m_crNetworkG2 );
	pColors.SetAt( L"system.network.edonkey", &Colors.m_crNetworkED2K );
	pColors.SetAt( L"system.network.g1", &Colors.m_crNetworkG1 );
	pColors.SetAt( L"system.network.g2", &Colors.m_crNetworkG2 );
	pColors.SetAt( L"system.network.ed2k", &Colors.m_crNetworkED2K );
	pColors.SetAt( L"system.network.dc", &Colors.m_crNetworkDC );
	pColors.SetAt( L"system.network.up", &Colors.m_crNetworkUp );
	pColors.SetAt( L"system.network.down", &Colors.m_crNetworkDown );
	pColors.SetAt( L"system.network.in", &Colors.m_crNetworkDown );
	pColors.SetAt( L"system.network.out", &Colors.m_crNetworkUp );
	pColors.SetAt( L"system.security.allow", &Colors.m_crSecurityAllow );
	pColors.SetAt( L"system.security.deny", &Colors.m_crSecurityDeny );

	pColors.SetAt( L"dropdownbox.back", &Colors.m_crDropdownBox );
	pColors.SetAt( L"dropdownbox.text", &Colors.m_crDropdownText );
	pColors.SetAt( L"resizebar.edge", &Colors.m_crResizebarEdge );
	pColors.SetAt( L"resizebar.face", &Colors.m_crResizebarFace );
	pColors.SetAt( L"resizebar.shadow", &Colors.m_crResizebarShadow );
	pColors.SetAt( L"resizebar.highlight", &Colors.m_crResizebarHighlight );
	pColors.SetAt( L"fragmentbar.shaded", &Colors.m_crFragmentShaded );
	pColors.SetAt( L"fragmentbar.complete", &Colors.m_crFragmentComplete );
	pColors.SetAt( L"fragmentbar.source1", &Colors.m_crFragmentSource1 );
	pColors.SetAt( L"fragmentbar.source2", &Colors.m_crFragmentSource2 );
	pColors.SetAt( L"fragmentbar.source3", &Colors.m_crFragmentSource3 );
	pColors.SetAt( L"fragmentbar.source4", &Colors.m_crFragmentSource4 );
	pColors.SetAt( L"fragmentbar.source5", &Colors.m_crFragmentSource5 );
	pColors.SetAt( L"fragmentbar.source6", &Colors.m_crFragmentSource6 );
	pColors.SetAt( L"fragmentbar.source5", &Colors.m_crFragmentSource7 );
	pColors.SetAt( L"fragmentbar.source6", &Colors.m_crFragmentSource8 );
	pColors.SetAt( L"fragmentbar.pass", &Colors.m_crFragmentPass );
	pColors.SetAt( L"fragmentbar.fail", &Colors.m_crFragmentFail );
	pColors.SetAt( L"fragmentbar.request", &Colors.m_crFragmentRequest );
	pColors.SetAt( L"fragmentbar.border", &Colors.m_crFragmentBorder );
	pColors.SetAt( L"fragmentbar.border.selected", &Colors.m_crFragmentBorderSelected );

	pColors.SetAt( L"system.environment.borders", &Colors.m_crSysDisabled );
	pColors.SetAt( L"system.environment.disabled", &Colors.m_crSysDisabled );
	pColors.SetAt( L"system.environment.window", &Colors.m_crSysWindow );
	pColors.SetAt( L"system.environment.btnface", &Colors.m_crSysBtnFace );
	pColors.SetAt( L"system.environment.3dshadow", &Colors.m_crSys3DShadow );
	pColors.SetAt( L"system.environment.3dhighlight", &Colors.m_crSys3DHighlight );
	pColors.SetAt( L"system.environment.activecaption", &Colors.m_crSysActiveCaption );
	pColors.SetAt( L"system.environment.menutext", &Colors.m_crSysMenuText );

	pColors.SetAt( L"navbar.text", &Colors.m_crNavBarText );
	pColors.SetAt( L"navbar.text.up", &Colors.m_crNavBarTextUp );
	pColors.SetAt( L"navbar.text.down", &Colors.m_crNavBarTextDown );
	pColors.SetAt( L"navbar.text.hover", &Colors.m_crNavBarTextHover );
	pColors.SetAt( L"navbar.text.checked", &Colors.m_crNavBarTextChecked );
	pColors.SetAt( L"navbar.shadow", &Colors.m_crNavBarShadow );
	pColors.SetAt( L"navbar.shadow.up", &Colors.m_crNavBarShadowUp );
	pColors.SetAt( L"navbar.shadow.down", &Colors.m_crNavBarShadowDown );
	pColors.SetAt( L"navbar.shadow.hover", &Colors.m_crNavBarShadowHover );
	pColors.SetAt( L"navbar.shadow.checked", &Colors.m_crNavBarShadowChecked );
	pColors.SetAt( L"navbar.outline", &Colors.m_crNavBarOutline );
	pColors.SetAt( L"navbar.outline.up", &Colors.m_crNavBarOutlineUp );
	pColors.SetAt( L"navbar.outline.down", &Colors.m_crNavBarOutlineDown );
	pColors.SetAt( L"navbar.outline.hover", &Colors.m_crNavBarOutlineHover );
	pColors.SetAt( L"navbar.outline.checked", &Colors.m_crNavBarOutlineChecked );

	// "system.base.xxx" colors trigger recalculating (ToDo: Needs review)
	BOOL bSystem  = FALSE;
	BOOL bNonBase = FALSE;

	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( pXML->IsNamed( L"color" ) ||
			 pXML->IsNamed( L"colour" ) )
		{
			CString strName  = pXML->GetAttributeValue( L"name" );
			CString strValue = pXML->GetAttributeValue( L"value" );
			strName.MakeLower();

			COLORREF* pColor;
			if ( pColors.Lookup( strName, (void*&)pColor ) )
			{
				if ( strValue.GetLength() > 2 )
				{
					*pColor = GetColor( strValue );

					if ( StartsWith( strName, _P( L"system." ) ) )
					{
						bSystem = TRUE;

						if ( ! bNonBase && strName.Find( L".base.", 5 ) < 0 )
						{
							bNonBase = TRUE;
							Colors.CalculateColors( TRUE );
						}
					}
				}
				else if ( strValue.IsEmpty() )
				{
					*pColor = CLR_NONE;
				}
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [name] attribute in [colorScheme] element", pXML->ToString() );
			}
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [colorScheme] element", pXML->ToString() );
		}
	}

	if ( bSystem && ! bNonBase )
		Colors.CalculateColors( TRUE );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin command lookup helper

UINT CSkin::LookupCommandID(CXMLElement* pXML, LPCTSTR pszName) const
{
	return CoolInterface.NameToID( pXML->GetAttributeValue( pszName ) );
}

//////////////////////////////////////////////////////////////////////
// CSkin command map

BOOL CSkin::LoadResourceMap(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"command" ) ||
			 pXML->IsNamed( L"control" ) ||
			 pXML->IsNamed( L"resource" ) )
		{
			CString strTemp = pXML->GetAttributeValue( L"code" );
			UINT nID;

			if ( _stscanf( strTemp, L"%u", &nID ) != 1 )
				return FALSE;

			CoolInterface.NameCommand( nID, pXML->GetAttributeValue( L"id" ) );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [...Map] element", pXML->ToString() );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin fonts

BOOL CSkin::LoadFonts(CXMLElement* pBase, const CString& strPath)
{
	bool bRichDefault = false, bRichHeading = false;

	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"font" ) )
		{
			CString strLanguage = pXML->GetAttributeValue( L"language" );

			if ( strLanguage.IsEmpty() ||
				( Settings.General.Language.CompareNoCase( strLanguage ) == 0 ) )
			{
				CString strName		= pXML->GetAttributeValue( L"name" ).MakeLower();
				CString strFace		= pXML->GetAttributeValue( L"face" );
				CString strSize		= pXML->GetAttributeValue( L"size" );
				CString strWeight	= pXML->GetAttributeValue( L"weight" );
				CString strColor	= pXML->GetAttributeValue( L"color" );
				CFont* pFont		= NULL;

				if ( strName.GetLength() < 6 ) continue;

				// Specifiable Fonts:
				SwitchMap( Font )
				{
					Font[ L"system" ]			= 'd';
					Font[ L"system.default" ]	= 'd';
					Font[ L"system.plain" ]		= 'd';
					Font[ L"system.bold" ]		= 'b';
					Font[ L"panel.caption" ]	= 'p';
					Font[ L"navbar" ]			= 'n';
					Font[ L"navbar.caption" ]	= 'n';
					Font[ L"navbar.selected" ]	= 's';
					Font[ L"navbar.active" ]	= 's';
					Font[ L"navbar.hover" ]		= 'v';
					Font[ L"richdoc.default" ]	= 'r';
					Font[ L"rich.default" ]		= 'r';
					Font[ L"richdoc.heading" ]	= 'h';
					Font[ L"rich.heading" ]		= 'h';
				}

				switch ( Font[ strName ] )
				{
				case 'd':	// system.default, system.plain, system
					pFont = &CoolInterface.m_fntNormal;
					if ( ! strColor.IsEmpty() )
						Colors.m_crText = GetColor( strColor );
					break;
				case 'b':	// system.bold
					pFont = &CoolInterface.m_fntBold;
					break;
				case 'p':	// panel.caption
					pFont = &CoolInterface.m_fntCaption;
					if ( ! strColor.IsEmpty() )
						Colors.m_crTaskBoxCaptionText = GetColor( strColor );
					break;
				case 'n':	// navbar.caption
					pFont = &CoolInterface.m_fntNavBar;
					if ( ! strColor.IsEmpty() )
						Colors.m_crNavBarText = GetColor( strColor );
					break;
				case 's':	// navbar.selected
					pFont = &CoolInterface.m_fntNavBarActive;
					if ( ! strColor.IsEmpty() )
						Colors.m_crNavBarTextChecked = GetColor( strColor );
					break;
				case 'v':	// navbar.hover
					pFont = &CoolInterface.m_fntNavBarHover;
					if ( ! strColor.IsEmpty() )
						Colors.m_crNavBarTextHover = GetColor( strColor );
					break;
				case 'r':	// richdoc.default, rich.default
					bRichDefault = true;
					pFont = &CoolInterface.m_fntRichDefault;
					if ( ! strColor.IsEmpty() )
						Colors.m_crRichdocText = GetColor( strColor );
					break;
				case 'h':	// richdoc.heading, rich.heading
					bRichHeading = true;
					pFont = &CoolInterface.m_fntRichHeading;
					if ( ! strColor.IsEmpty() )
						Colors.m_crRichdocHeading = GetColor( strColor );
					break;
				default:
					theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown font name", pXML->ToString() );
					continue;
				}

				if ( pFont->m_hObject ) pFont->DeleteObject();

				if ( strFace.IsEmpty() )
					strFace = Settings.Fonts.DefaultFont;

				if ( strWeight.IsEmpty() || strWeight.CompareNoCase( L"normal" ) == 0 )
					strWeight = L"400";
				else if ( strWeight.CompareNoCase( L"bold" ) == 0 )
					strWeight = L"700";
				else if ( strWeight.CompareNoCase( L"extrabold" ) == 0 || strWeight.CompareNoCase( L"heavy" ) == 0 )
					strWeight = L"900";

				int nFontSize = Settings.Fonts.DefaultSize;
				int nFontWeight = FW_NORMAL;

				if ( strSize.GetLength() && _stscanf( strSize, L"%i", &nFontSize ) != 1 )
					theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Bad [size] attribute in [font] element", pXML->ToString() );

				if ( strWeight.GetLength() && _stscanf( strWeight, L"%i", &nFontWeight ) != 1 )
					theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Bad [weight] attribute in [font] element", pXML->ToString() );

				pFont->CreateFont( -nFontSize, 0, 0, 0, nFontWeight, FALSE, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
					Settings.Fonts.Quality, DEFAULT_PITCH|FF_DONTCARE, strFace );

				if ( strName.CompareNoCase( L"system.plain" ) == 0 )
				{
					pFont = &CoolInterface.m_fntUnder;
					if ( pFont->m_hObject ) pFont->DeleteObject();

					pFont->CreateFont( -nFontSize, 0, 0, 0, nFontWeight, FALSE, TRUE, FALSE,
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						Settings.Fonts.Quality, DEFAULT_PITCH|FF_DONTCARE, strFace );

					pFont = &CoolInterface.m_fntItalic;
					if ( pFont->m_hObject ) pFont->DeleteObject();

					pFont->CreateFont( -nFontSize, 0, 0, 0, nFontWeight, TRUE, FALSE, FALSE,
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						Settings.Fonts.Quality, DEFAULT_PITCH|FF_DONTCARE, strFace );
				}
				else if ( strName.CompareNoCase( L"system.bold" ) == 0 )
				{
					pFont = &CoolInterface.m_fntBoldItalic;
					if ( pFont->m_hObject ) pFont->DeleteObject();

					pFont->CreateFont( -nFontSize, 0, 0, 0, nFontWeight, TRUE, FALSE, FALSE,
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						Settings.Fonts.Quality, DEFAULT_PITCH|FF_DONTCARE, strFace );
				}
				else if ( strName.CompareNoCase( L"navbar" ) == 0 )
				{
					pFont = &CoolInterface.m_fntNavBarActive;
					if ( pFont->m_hObject ) pFont->DeleteObject();
					pFont->CreateFont( -nFontSize, 0, 0, 0, nFontWeight, FALSE, FALSE, FALSE,
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						Settings.Fonts.Quality, DEFAULT_PITCH|FF_DONTCARE, strFace );

					pFont = &CoolInterface.m_fntNavBarHover;
					if ( pFont->m_hObject ) pFont->DeleteObject();
					pFont->CreateFont( -nFontSize, 0, 0, 0, nFontWeight, FALSE, FALSE, FALSE,
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						Settings.Fonts.Quality, DEFAULT_PITCH|FF_DONTCARE, strFace );
				}
			}
		}
		else if ( pXML->IsNamed( L"import" ) )
		{
			CString strFile = strPath + pXML->GetAttributeValue( L"path" );

			if ( AddFontResourceEx( strFile, FR_PRIVATE, NULL ) )
				m_pFontPaths.AddTail( strFile );
			else
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed to import font", pXML->ToString() );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [fonts] element", pXML->ToString() );
		}
	}

	// Create Rich Default font based on Normal font, if absent
	if ( ! bRichDefault )
	{
		LOGFONT lfDefault = {};
		CoolInterface.m_fntNormal.GetLogFont( &lfDefault );
		lfDefault.lfHeight -= 1;
		lfDefault.lfWeight += 300;
		if ( CoolInterface.m_fntRichDefault.m_hObject )
			CoolInterface.m_fntRichDefault.DeleteObject();
		CoolInterface.m_fntRichDefault.CreateFontIndirect( &lfDefault );
	}

	// Create Rich Heading font based on Rich Default font, if absent
	if ( ! bRichHeading )
	{
		LOGFONT lfDefault = {};
		CoolInterface.m_fntRichDefault.GetLogFont( &lfDefault );
		lfDefault.lfHeight -= 5;
		lfDefault.lfWeight += 100;
		if ( CoolInterface.m_fntRichHeading.m_hObject )
			CoolInterface.m_fntRichHeading.DeleteObject();
		CoolInterface.m_fntRichHeading.CreateFontIndirect( &lfDefault );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin command images

CString	CSkin::GetImagePath(UINT nImageID) const
{
	//CQuickLock oLock( m_pSection );

	CString strPath;
	if ( ! m_pImages.Lookup( nImageID, strPath ) )
		strPath.Format( L"\"%s\",-%u", theApp.m_strBinaryPath, nImageID );
	return strPath;
}

BOOL CSkin::LoadCommandImages(CXMLElement* pBase, const CString& strPath)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"icon" ) )
		{
			if ( ! LoadCommandIcon( pXML, strPath ) )
				return FALSE;
		}
		else if ( pXML->IsNamed( L"bitmap" ) )
		{
			if ( ! LoadCommandBitmap( pXML, strPath ) )
				return FALSE;
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [commandImages] element", pXML->ToString() );
		}
	}

	return TRUE;
}

BOOL CSkin::LoadCommandIcon(CXMLElement* pXML, const CString& strPath)
{
	// strPath is:
	// 1) when loading from resource: "module instance$" or ...
	// 2) when loading from file: "root skin path\".

	CString strFile = strPath +
		pXML->GetAttributeValue( L"res" ) +
		pXML->GetAttributeValue( L"path" );

	HINSTANCE hInstance( NULL );

	UINT nIconID = LookupCommandID( pXML, L"res" );
	if ( nIconID )
		_stscanf( strPath, L"%p", &hInstance );

	UINT nID = LookupCommandID( pXML );
	if ( nID == 0 )
	{
		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown [id] attribute in [icon] element", pXML->ToString() );
		return TRUE;	// Skip icon and load remaining skin
	}

	// Is this a RTL-enabled icon? (default: rtl="0" - no)
	const BOOL bRTL = Settings.General.LanguageRTL && LoadOptionBool( pXML->GetAttributeValue( L"rtl", L"0" ) );

	// Icon types (default: "16" - 16x16 icon only)
	CString strTypes = pXML->GetAttributeValue( L"types", L"16" );
	CString strSize;
	int curPos = 0;

	while ( ! ( strSize = strTypes.Tokenize( L",", curPos ) ).IsEmpty() )
	{
		int cx = _tstoi( strSize );
		int nType;
		switch ( cx )
		{
		case 16:
			nType = LVSIL_SMALL;
			break;
		case 32:
			nType = LVSIL_NORMAL;
			break;
		case 48:
			nType = LVSIL_BIG;
			break;
		default:
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Icon has invalid size", pXML->ToString() );
			return FALSE;
		}

		HICON hIcon = NULL;
		if ( nIconID && hInstance )
		{
			hIcon = (HICON)LoadImage( hInstance, MAKEINTRESOURCE( nIconID ), IMAGE_ICON, cx, cx, 0 );
		}
		else if ( LoadIcon( strFile,
			( ( nType == LVSIL_SMALL ) ? &hIcon : NULL ),
			( ( nType == LVSIL_NORMAL ) ? &hIcon : NULL ),
			( ( nType == LVSIL_BIG ) ? &hIcon : NULL ) ) )
		{
			m_pImages.SetAt( nID, strFile );
		}

		if ( hIcon )
		{
			if ( bRTL )
				hIcon = CreateMirroredIcon( hIcon );
			CoolInterface.AddIcon( nID, hIcon, nType );
			VERIFY( DestroyIcon( hIcon ) );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed to load icon", pXML->ToString() );
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CSkin::LoadCommandBitmap(CXMLElement* pBase, const CString& strPath)
{
	CString strFile;
	UINT nID = LookupCommandID( pBase );
	// If nID is 0 then we don't want to include it in strFile because
	// strFile must be a file system path rather than a resource path.
	if ( nID )
		strFile.Format( L"%s%lu%s", strPath, nID, pBase->GetAttributeValue( L"path" ) );
	else
		strFile.Format( L"%s%s", strPath, pBase->GetAttributeValue( L"path" ) );

	COLORREF crMask = NULL;
	CString strMask = pBase->GetAttributeValue( L"mask", L"00FF00" );
	if ( strMask.GetLength() >= 6 )
		crMask = GetColor( strMask );

	if ( crMask == NULL )
	{
		// ToDo: Auto set mask to CLR_NONE for alpha PNGs, and cull this list
		strMask.MakeLower();
		if ( strMask == L"alpha" ||
			 strMask == L"transparent" ||
			 strMask == L"clr_none" ||
			 strMask == L"none" ||
			 strMask == L"null" ||
			 strMask == L"png" )
		{
			crMask = CLR_NONE; 	// 0 Alpha (or Black)
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Image has invalid mask", pBase->ToString() );
			crMask = RGB( 0, 255, 0 );
		}
	}

	HBITMAP hBitmap;
	if ( crMask == CLR_NONE )
	{
		CImageFile pFile;
		if ( nID > 100 )
			pFile.LoadFromResource( AfxGetResourceHandle(), nID, RT_PNG );
		else
			pFile.LoadFromFile( strFile );

		//pFile.EnsureRGB();

		hBitmap = pFile.CreateBitmap();
	}
	else
		hBitmap = LoadBitmap( strFile );

	if ( ! hBitmap )
	{
		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed to load image", pBase->ToString() );
		return FALSE;
	}
	if ( Settings.General.LanguageRTL )
		hBitmap = CreateMirroredBitmap( hBitmap );

	BOOL bResult = CoolInterface.Add( this, pBase, hBitmap, crMask );
	DeleteObject( hBitmap );

	if ( ! bResult )
	{
		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Failed to add image", pBase->ToString() );
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin popup menu helper

void CSkin::TrackPopupMenu(LPCTSTR pszMenu, const CPoint& point, UINT nDefaultID, const CStringList& oFiles /*Empty*/, CWnd* pWnd /*MainWnd*/) const
{
	CMenu* pPopup = GetMenu( pszMenu );
	if ( pPopup == NULL )
		return;

	if ( nDefaultID != 0 )
		pPopup->SetDefaultItem( nDefaultID );

	// Unskinned shell menu
	if ( oFiles.GetCount() )
	{
		// Change ID_SHELL_MENU item to shell submenu
		MENUITEMINFO pInfo = {};
		pInfo.cbSize = sizeof( pInfo );
		pInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
		pInfo.fState = MFS_ENABLED;
		HMENU hSubMenu = pInfo.hSubMenu = ::CreatePopupMenu();
		ASSERT( hSubMenu );
		if ( pPopup->SetMenuItemInfo( ID_SHELL_MENU, &pInfo ) )
		{
			CoolMenu.DoExplorerMenu( pWnd->GetSafeHwnd(), oFiles,
				point, pPopup->GetSafeHmenu(), pInfo.hSubMenu,
				TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON );

			// Change ID_SHELL_MENU back
			pInfo.hSubMenu = NULL;
			VERIFY( pPopup->SetMenuItemInfo( ID_SHELL_MENU, &pInfo ) );

			return;
		}
		VERIFY( DestroyMenu( hSubMenu ) );
	}

	__try	// Fix for strange TrackPopupMenu crash inside GUI
	{
		pPopup->TrackPopupMenu(
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			point.x, point.y, pWnd );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
}

//////////////////////////////////////////////////////////////////////
// CSkin draw wrapped text

// GetTextFlowChange determines the direction of text and its change at word boundaries.
// Returns the direction of the first "words iceland" and the position
// where the next "iceland" starts at the word boundary.
// If there is no change in direction it returns 0.

int CSkin::GetTextFlowChange(LPCTSTR pszText, BOOL* bIsRTL)
{
	TRISTATE bTextIsRTL	= TRI_UNKNOWN;
	BOOL bChangeFound	= FALSE;
	LPCTSTR pszWord = pszText;
	LPCTSTR pszScan = pszText;

	int nPos;
	for ( nPos = 0 ; ; pszScan++, nPos++ )
	{
		// Get the first word with punctuation marks and whitespaces
		if ( (unsigned short)*pszScan > 32 && (unsigned short)*pszScan != 160 ) continue;

		if ( pszWord < pszScan )
		{
			int nLen = static_cast< int >( pszScan - pszWord );
			WORD* nCharType = new WORD[ nLen + 1 ];

			TCHAR* pszTestWord = new TCHAR[ nLen + 1 ];
			_tcsncpy_s( pszTestWord, nLen + 1, pszWord, nLen );
			pszTestWord[ nLen ] = 0;

			GetStringTypeEx( LOCALE_NEUTRAL, CT_CTYPE2, pszTestWord, nLen + 1, (LPWORD)nCharType );
			delete [] pszTestWord;

			for ( int i = 0 ; i < nLen ; i++ )
			{
				if ( nCharType[ i ] == C2_LEFTTORIGHT )
				{
					if ( bTextIsRTL == TRI_UNKNOWN )
					{
						bTextIsRTL = TRI_FALSE;
						*bIsRTL = FALSE;
					}
					else if ( bTextIsRTL == TRI_TRUE )
					{
						bChangeFound = TRUE;
						break;
					}
				}
				else if ( nCharType[ i ] == C2_RIGHTTOLEFT )
				{
					if ( bTextIsRTL == TRI_UNKNOWN )
					{
						bTextIsRTL = TRI_TRUE;
						*bIsRTL = TRUE;
					}
					else if ( bTextIsRTL == TRI_FALSE )
					{
						bChangeFound = TRUE;
						break;
					}
				}
			}
			BOOL bLeadingWhiteSpace = ( nCharType[ 0 ] == C2_WHITESPACE );
			delete [] nCharType;

			if ( bChangeFound ) return nPos - nLen + ( bLeadingWhiteSpace ? 1 : 0 );
			pszWord = pszScan;
		}
		if ( ! *pszScan ) break;
	}
	return 0;
}

void CSkin::DrawWrappedText(CDC* pDC, CRect* pBox, LPCTSTR pszText, CPoint ptStart, BOOL bExclude)
{
	// ToDo: Wrap mixed text in RTL and LTR layouts correctly

	if ( pszText == NULL ) return;
	if ( ptStart.x == 0 && ptStart.y == 0 ) ptStart = pBox->TopLeft();

	UINT nAlignOptionsOld = pDC->GetTextAlign();	// Backup settings
	UINT nFlags = ETO_CLIPPED | ( bExclude ? ETO_OPAQUE : 0 );

	unsigned short nLenFull = static_cast< unsigned short >( _tcslen( pszText ) );

	// Collect stats about the text from the start
	BOOL bIsRTLStart = FALSE;
	int nTestStart = GetTextFlowChange( pszText, &bIsRTLStart );

	// Guess text direction ( not always works )
	BOOL bNormalFlow = Settings.General.LanguageRTL ? bIsRTLStart : ! bIsRTLStart;

	TCHAR* pszSource = NULL;
	LPCTSTR pszWord  = NULL;
	LPCTSTR pszScan  = NULL;

	if ( nTestStart )
	{
		// Get the source string to draw and truncate initial string to pass it recursively
		pszSource = new TCHAR[ nTestStart + 1 ];
		_tcsncpy_s( pszSource, nTestStart + 1, pszText, nTestStart );
		pszSource[ nTestStart ] = 0;
		if ( ! bNormalFlow )
		{
			// Swap whitespaces
			CString str = pszSource;
			if ( pszSource[ 0 ] == ' ' || (unsigned short)pszSource[ 0 ] == 160 )
			{
				str = str + L" ";
				str = str.Right( nTestStart );
			}
			else if ( pszSource[ nTestStart - 1 ] == ' ' ||
					(unsigned short)pszSource[ nTestStart - 1 ] == 160 )
			{
				str = L" " + str;
				str = str.Left( nTestStart );
			}
			_tcsncpy_s( pszSource, nTestStart + 1, str.GetBuffer( nTestStart ), nTestStart );
		}
		nLenFull = static_cast< unsigned short >( nTestStart );
		pszText += nTestStart;
	}
	else
		pszSource = (TCHAR*)pszText;

	pszWord = pszSource;
	pszScan = pszSource;

	if ( ! bNormalFlow )
	{
		if ( ( bIsRTLStart != FALSE ) != Settings.General.LanguageRTL )
			pDC->SetTextAlign( nAlignOptionsOld ^ TA_RTLREADING );
		pszScan += nLenFull - 1;
		pszWord += nLenFull;
		for ( int nEnd = nLenFull - 1 ; nEnd >= 0 ; nEnd-- )
		{
			if ( nEnd ) pszScan--;
			if ( nEnd && (unsigned short)*pszScan > 32 && (unsigned short)*pszScan != 160 ) continue;

			if ( pszWord >= pszScan )
			{
				int nLen = static_cast< int >( pszWord - pszScan );
				CSize sz;
				GetTextExtentPoint32( pDC->m_hAttribDC, pszScan, nLen, &sz );

				if ( ptStart.x > pBox->left && ptStart.x + sz.cx > pBox->right )
				{
					ptStart.x = pBox->left;
					ptStart.y += sz.cy;
				}

				// Add extra point in x-axis; it cuts off the 1st word character otherwise
				const short nExtraPoint = ( Settings.General.LanguageRTL ) ? 1 : 0;
				CRect rc( ptStart.x, ptStart.y, ptStart.x + sz.cx + nExtraPoint, ptStart.y + sz.cy );

				pDC->ExtTextOut( ptStart.x, ptStart.y, nFlags, &rc,
					pszScan, nLen, NULL );
				if ( bExclude ) pDC->ExcludeClipRect( &rc );

				ptStart.x += sz.cx + nExtraPoint;
				pBox->top = ptStart.y + sz.cy;
			}
			pszWord = pszScan;
		}
	}
	else
	{
		for ( ; ; pszScan++ )
		{
			if ( *pszScan != NULL && (unsigned short)*pszScan > 32 &&
				 (unsigned short)*pszScan != 160 ) continue;

			if ( pszWord <= pszScan )
			{
				int nLen = static_cast< int >( pszScan - pszWord + ( *pszScan ? 1 : 0 ) );
				CSize sz = pDC->GetTextExtent( pszWord, nLen );

				if ( ptStart.x > pBox->left && ptStart.x + sz.cx > pBox->right )
				{
					ptStart.x = pBox->left;
					ptStart.y += sz.cy;
				}

				// Add extra point in x-axis; it cuts off the 1st word character otherwise
				const short nExtraPoint = ( Settings.General.LanguageRTL ) ? 1 : 0;

				CRect rc( ptStart.x, ptStart.y, ptStart.x + sz.cx + nExtraPoint, ptStart.y + sz.cy );

				pDC->ExtTextOut( ptStart.x, ptStart.y, nFlags, &rc,
					pszWord, nLen, NULL );
				if ( bExclude ) pDC->ExcludeClipRect( &rc );

				ptStart.x += sz.cx + nExtraPoint;
				pBox->top = ptStart.y + sz.cy;
			}

			pszWord = pszScan + 1;
			if ( ! *pszScan ) break;
		}
	}
	if ( nTestStart ) delete [] pszSource;
	// Reset align options back
	pDC->SetTextAlign( nAlignOptionsOld );
	if ( nTestStart ) DrawWrappedText( pDC, pBox, pszText, ptStart, bExclude );
}

//////////////////////////////////////////////////////////////////////
// CSkin hex color utility

BOOL CSkin::LoadColor(CXMLElement* pXML, LPCTSTR pszName, COLORREF* pColor)
{
	CString str = pXML->GetAttributeValue( pszName );
	if ( ! str.IsEmpty() )
	{
		*pColor = GetColor( str );
		if ( *pColor || str == L"000000" )
			return TRUE;

		theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Bad color attribute", pXML->ToString() );
	}

	return FALSE;
}

COLORREF CSkin::GetColor(CString sColor)
{
	sColor.Trim( L" #" );

	const int nLength = sColor.GetLength();
	if ( nLength > 3 && nLength < 14 )
	{
		if ( sColor == L"CLR_NONE" ||
			 nLength == 4 && sColor.CompareNoCase( L"none" ) == 0 )
			return CLR_NONE;

		int nRed = 0, nGreen = 0, nBlue = 0;

		if ( nLength == 6 &&
			 _stscanf( sColor.Mid( 0, 2 ), L"%x", &nRed ) == 1 &&
			 _stscanf( sColor.Mid( 2, 2 ), L"%x", &nGreen ) == 1 &&
			 _stscanf( sColor.Mid( 4, 2 ), L"%x", &nBlue ) == 1 )
			return RGB( nRed, nGreen, nBlue );

		if ( _stscanf( (LPCTSTR)sColor, L"%i, %i, %i", &nRed, &nGreen, &nBlue ) == 3 &&
			 nRed < 256 && nGreen < 256 && nBlue < 256 )
			return RGB( nRed, nGreen, nBlue );
	}

	if ( sColor.IsEmpty() )
		sColor = L"(Empty)";

	theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Bad value for color: " + sColor );

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSkin load bitmap helper

HBITMAP CSkin::LoadBitmap(const CString& strName)
{
	//HBITMAP hBitmap = NULL;
	//if ( m_pBitmaps.Lookup( strName, hBitmap ) )
	//	return hBitmap;

	const int nPos = strName.Find( L'$' );
	if ( nPos < 0 )
		return CImageFile::LoadBitmapFromFile( strName );

	HINSTANCE hInstance = NULL;
	if ( _stscanf( (LPCTSTR)strName, L"%p", &hInstance ) != 1 )
		return NULL;

	UINT nID = 0;
	if ( _stscanf( (LPCTSTR)strName + nPos + 1, L"%u", &nID ) != 1 )
		return NULL;

	return CImageFile::LoadBitmapFromResource( nID, hInstance );

	//if ( hBitmap )
	//	m_pBitmaps.SetAt( strName, hBitmap );
	//return hBitmap;
}

HBITMAP CSkin::LoadBitmap(UINT nID)
{
	CString strName;
	strName.Format( L"%p$%lu", (HINSTANCE)GetModuleHandle( NULL ), nID );
	return LoadBitmap( strName );
}


//////////////////////////////////////////////////////////////////////
// CSkin mode suffixes

LPCTSTR CSkin::m_pszGUIMode[3] =
{
	L".Windowed",	// 0 GUI_WINDOWED
	L".Tabbed",		// 1 GUI_TABBED
	L".Basic"		// 2 GUI_BASIC
};

// Obsolete legacy fallbacks method:
//LPCTSTR CSkin::m_pszModeSuffix[3][4] =
//{
//	{ L".Windowed", L"", NULL, NULL },			// 0 GUI_WINDOWED
//	{ L".Tabbed", L"", NULL, NULL },			// 1 GUI_TABBED
//	{ L".Basic", L".Tabbed", L"", NULL }		// 2 GUI_BASIC
//};