//
// TextViewerPlugin.cpp
//
// This file is part of Envy (getenvy.com) © 2010
// TextViewer plugin is released under the Persistent Public Domain license.
//
// This code may be treated as Public Domain, provided:
// the work in all its forms and attendant uses shall remain available as
// persistently "Public Domain" until such time it naturally enters the public domain.
// History remains immutable:  Authors do not disclaim copyright, but do disclaim
// all conferred rights beyond asserting the reach and duration and spirit of this license.

// This file contains the CTextViewerPlugin class, which is the "plugin object".
// It is created by Envy when the plugin is loaded or enabled by the user,
// and destroyed when the application is closed or the plugin is disabled.
//
// This is a "general plugin", so it implements the IGeneralPlugin interface.
// General plugins are always invoked from the GUI thread.
//
// The text viewer needs to capture the "open file" event so that it can open the
// applicable text file in a viewer window.  This is achieved by implementing the
// IExecutePlugin interface, which has OnExecute() and OnEnqueue() methods that can
// override Envy's default file-opening behaviour.
//
// The ICommandPlugin interface is also implemented, which allows the text viewer to
// register its own user interface commands, and respond to them when the user invokes them.

#include "StdAfx.h"
#include "TextViewerPlugin.h"
#include "TextWindow.h"


/////////////////////////////////////////////////////////////////////////////
// CTextViewerPlugin construction

CTextViewerPlugin::CTextViewerPlugin()
{
	// We will maintain a list of open CTextWindow objects as a linked list
	m_pWindow	= NULL;

	// Load the "pointer hand" cursor from the DLL's resources
	m_hcMove	= LoadCursor( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDC_SELECT) );
}

/////////////////////////////////////////////////////////////////////////////
// CTextViewerPlugin destruction

CTextViewerPlugin::~CTextViewerPlugin()
{
	// If CTextWindow windows are open, we must close them now as the plugin is being destroyed.
	// Otherwise windows will be left behind and will become unstable.
	// Simply walk through the linked list and call DestroyWindow()

	while ( m_pWindow != NULL )
	{
		CTextWindow* pNext = m_pWindow->m_pNext;
		m_pWindow->m_pPlugin = NULL;
		m_pWindow->DestroyWindow();
		m_pWindow = pNext;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTextViewerPlugin IGeneralPlugin implementation

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::SetApplication(IApplication __RPC_FAR *pApplication)
{
	// This method is invoked as soon as the plugin object has been created,
	// to pass a reference to the core Application object

	// Save the IApplication interface in a member variable (it will be AddRef'ed)
	m_pApplication = pApplication;

	// Get the user interface manager (IUserInterface) and store it in a member variable
	m_pApplication->get_UserInterface( &m_pInterface );

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::QueryCapabilities(DWORD __RPC_FAR* /*pnCaps*/)
{
	// This method is not currently used, please return S_OK and do not modify pnCaps
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::Configure()
{
	// This method invoked if a user selects the "Setup" command from Plugin Settings page.

	// Simply load a string from the string table and display it in a MessageBox
	TCHAR szMessage[1024];
	LoadString( _AtlBaseModule.GetResourceInstance(), IDS_ABOUT, szMessage, 1024 );
	MessageBox( GetActiveWindow(), szMessage, _T("Text Viewer Plugin"), MB_ICONINFORMATION );

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::OnSkinChanged()
{
	// This method is invoked to allow the plugin to handle a "skin changed" event.
	// The plugin should destroy any skin-based resources it has acquired,
	// and re-acquire them from the user interface manager.  Also invoked when
	// a new langauge is selected and after plugins have been loaded/unloaded.

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CTextViewerPlugin IExecutePlugin implementation

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::OnExecute(BSTR sFilePath)
{
	// The OnExecute method is invoked whenever Envy needs to execute (open) a file.
	// The path of the file is provided as an argument.
	// Return S_OK if you have handled the execution, or
	// S_FALSE if Envy should keep looking for someone to handle it (and potentially fall back to its internal handler).
	// Failure codes (E_*) are interpreted as an inability to open the file, and an error message will be displayed.
	// So don't return failures (E_*) unless you should be able to open the file, but can't.
	// S_FALSE is the correct code if you don't want to open this kind of file.

	// String conversion macros

	USES_CONVERSION;

	// Convert the BSTR to a LPCTSTR, and locate the file extension

	LPCTSTR pszFilePath = OLE2T( sFilePath );
	LPCTSTR pszFileType = _tcsrchr( pszFilePath, '.' );

	// If there was no file extension, this file is not for us

	if ( pszFileType == NULL ) return S_FALSE;

	// This text viewer plugin attempts to decide whether or not it should open a file
	// based on whether or not there is an ImageService plugin available for the image file type.

	// First check some common unsupported file types here, and return S_FALSE if we get a match.

	if ( lstrcmpi( pszFileType, _T(".docx") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".pdf") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".xps") ) == 0 ) return S_FALSE;

	// Assuming now that it is not an unsupported file.  The next (and primary) step is to
	// check if there is an ImageService plugin available for this file type.
	// This is done by checking a registry key:

	if ( lstrcmpi( pszFileType, _T(".partial") ) != 0 )
	{
		DWORD dwCount = 128;
		TCHAR szValue[128];
		CRegKey pReg;

		if ( pReg.Open( HKEY_CURRENT_USER,
			_T("SOFTWARE\\Envy\\Envy\\Plugins\\ImageService") ) != ERROR_SUCCESS )
			return S_FALSE;

		if ( pReg.QueryValue( pszFileType, NULL, szValue, &dwCount ) != ERROR_SUCCESS )
			return S_FALSE;

		pReg.Close();
	}

	// If we made it to this point, there is indeed an ImageService plugin for the file type, so attempt opening.
	// Delegate to our OpenNewWindow() function to select or create the image window.

	OpenNewWindow( pszFilePath );

	// Return S_OK, because we have successfully opened this file (we hope).

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::OnEnqueue(BSTR /*sFilePath*/)
{
	// The OnEnqueue method is invoked whenever Envy needs to enqueue a file ("add to playlist").
	// The path of the file is provided as an argument.  Return S_OK if you have handled the execution,
	// or S_FALSE if Envy should keep looking for someone to handle it (and potentially fall back
	// to its internal handler).  Failure codes (E_*) are interpreted as an inability to open the file,
	// and an error message will be displayed.  So don't return failures (E_*) unless you should be able
	// to open the file, but can't.  S_FALSE is the correct code if you don't want to open this kind of file.

	// The image viewer does not enqueue files, so we return S_FALSE (keep looking).

	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTextViewerPlugin ICommandPlugin implementation
//
// Custom Interface Command IDs

//	RegisterCommands
//
// The RegisterCommands() method is invoked when the UI manager is building its list of available commands.
// Envy uses a unified command architecture in which every command in the UI is assigned
// a friendly name and an ID number.  Envy has already registered its internal commands,
// and is now providing an opportunity for plugins to register their own.
// The unique name is assigned by you, and Envy will provide you with the command ID number.
//
// Plugin-Command ID numbers are not fixed, so they should be stored in a variable for later use!
// By convention name commands with PluginID, underscore, plugin name, another underscore,
// followed by command name.  No spaces or special characters (normal C++ identifier rules apply).
//
// Note that this method may be called more than once in the lifetime of the plugin (at any skin/plugin load),
// in which case you must re-register your commands and receive new command IDs.
//
// The second argument, although NULL here, can optionally provide a 16x16 icon handle:
// LoadIcon( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_ICON ) )	-but also skinned normally.

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::RegisterCommands()
{
	m_pInterface->RegisterCommand( L"PluginID_TextViewer_SystemFont", NULL, &m_nCmdSystemFont );
	m_pInterface->RegisterCommand( L"PluginID_TextViewer_MonoFont", NULL, &m_nCmdMonoFont );
	m_pInterface->RegisterCommand( L"PluginID_TextViewer_Wrap", NULL, &m_nCmdWrap );
	m_pInterface->RegisterCommand( L"PluginID_TextViewer_Close", NULL, &m_nCmdClose );

	return S_OK;
}

//	InsertCommands
//
// The InsertCommands() method is invoked when the user interface manager is building the
// user interface objects such as context menus, toolbars, etc.  At this point
// it has created its internal objects, and parsed all applicable skin files to create their objects.
//
// The plugin should use this opportunity to either create its own user interface objects,
// and/or modify existing objects to add new commands, etc.
// Most of this work is achieved through the IUserInterface interface.
//
// Via IUserInterface you can access or create menus and toolbars by name (eg "CMainWnd.Tabbed"),
// and then view their content, adding, modifying or deleting commands as desired.
//
// If you are not modifying existing user interface objects, but rather creating your own
// (as in the case of this text viewer), it is a lot easier to actually use the XML skin file system.
// This is a lot better than having to do it all here programatically.
// The IUserInterface interface provides three methods for loading and incorporating a chunk of skin XML.
//
// The best choice is often to include the XML as a resource in your DLL, which is done here.
// IUserInterface::AddFromResource allows you to load a skin XML resource directly!
//
// Note that the resource type should be 23 decimal.  See the Skin.xml file for further detail.

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::InsertCommands()
{
	m_pInterface->AddFromResource( _AtlBaseModule.GetResourceInstance(), IDR_SKIN );

	return S_OK;
}

//	OnUpdate Command
//
// The OnUpdate() method is invoked when Envy needs to update the state of a command in its user interface.
// This provides an opportunity to show or hide, enable or disable, and check or uncheck user interface commands.
// Because of unified command architecture, it doesn't matter if a command is a menu or toolbar or something else entirely.
//
// The nCommandID argument is the ID of the command being updated.  You should check this against
// a list of command IDs your plugin has registered.  If you don't get a match, return S_FALSE.
// Unless you have a really good reason, you don't want to mess with commands that you didn't
// register (for one thing, you probably won't know what their ID number is).
// The S_FALSE code tells Envy to keep looking.
//
// If you do find a match, you should modify pbVisible, pbEnabled and pbChecked.
// Each is a "tri-state" enumeration, defaulting to TSUNKNOWN.
// Set TSTRUE to activate, or TSFALSE to deactivate.  Then,
// return S_OK to indicate that you are responsible for this command, and have updated it.
//
// You must check whether pbVisible, pbEnabled and pbChecked are NULL before reading or writing to them,
// as one or more of them may be NULL if it is not required.
//
// Here we are not interested in updating any commands, so we return S_FALSE.

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::OnUpdate(UINT /*nCommandID*/, TRISTATE __RPC_FAR* /*pbVisible*/, TRISTATE __RPC_FAR* /*pbEnabled*/, TRISTATE __RPC_FAR* /*pbChecked*/)
{
	return S_FALSE;
}

//	OnCommand
//
// The OnCommand() method is invoked whenever the user invokes a command.
// This applies to ANY command in the unified architecture, which could be a built-in command,
// a command you registered, or a command registered by another plugin.
//
// Return S_OK if you are handling the command, or S_FALSE if Envy should keep looking.
// Failure codes (E_*) will also cause Envy to stop looking for a handler.
//
// Typically you would check the nCommandID argument against a list of command IDs you have registered,
// and only return S_OK if you get a match.
// If the command is not currently available, return E_UNEXPECTED.
//
// However, for a good cause, you could also check for internal commands from the base Envy UI,
// (those which did not come from plugins).  These have fixed command IDs, so they can be safely detected.
// If you return S_OK for one of these, Envy won't take its default action.
//
// Here we are not interested in handling any commands, so we return S_FALSE.

HRESULT STDMETHODCALLTYPE CTextViewerPlugin::OnCommand(UINT /*nCommandID*/)
{
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTextViewerPlugin open a new window

BOOL CTextViewerPlugin::OpenNewWindow(LPCTSTR pszFilePath)
{
	// This helper function opens a new window, or activates an existing window, for the file name it is passed.

	// First, check through the linked list of CTextWindow windows, to see if the file is already open.

	CTextWindow* pWindow;
	for ( pWindow = m_pWindow ; pWindow ; pWindow = pWindow->m_pNext )
	{
		if ( lstrcmpi( pWindow->m_pszFile, pszFilePath ) == 0 )
			break;	// Got a match, break out of the loop.
	}

	// If we did not find the window...
	if ( pWindow == NULL )
	{
		// Create a new one, and add it to the linked list.

		pWindow = new CComObject<CTextWindow>;
		pWindow->m_pNext = m_pWindow;
		m_pWindow = pWindow;

		// Invoke the Create() method, to pass a reference to this plugin object, and the filename.
		pWindow->Create( this, pszFilePath );
	}

	// Tell the window (old or new) to refresh its image
	pWindow->Refresh();

	// Show and activate the window
	pWindow->ShowWindow( SW_SHOWNORMAL );
	pWindow->BringWindowToTop();
	pWindow->Invalidate();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTextViewerPlugin remove an existing window from the list

void CTextViewerPlugin::RemoveWindow(CTextWindow* pWindow)
{
	CTextWindow** ppPrev = &m_pWindow;

	// Search through the linked list of CTextWindow objects, and remove the one being closed.

	for ( CTextWindow* pSeek = *ppPrev ; pSeek ; pSeek = pSeek->m_pNext )
	{
		if ( pWindow == pSeek )
		{
			*ppPrev = pWindow->m_pNext;
			return;
		}

		ppPrev = &pSeek->m_pNext;
	}
}
