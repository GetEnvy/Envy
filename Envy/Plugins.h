//
// Plugins.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2006 and PeerProject 2008-2012
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

#pragma once

#include "ThreadImpl.h"

class CPlugin;
class CChildWnd;
class CLibraryFile;


class CPlugins : public CThreadImpl
{
public:
	CPlugins();

public:
	CMutex		m_pSection;

	BOOL		Register(const CString& sPath); 					// Register all plugins in given Envy installation/plugin folder

	void		Enumerate();
	void		Clear();
	BOOL		LookupCLSID(LPCTSTR pszGroup, LPCTSTR pszKey, CLSID& pCLSID) const;
	BOOL		LookupEnable(REFCLSID pCLSID, LPCTSTR pszExt = NULL) const;

	IUnknown*	GetPlugin(LPCTSTR pszGroup, LPCTSTR pszType);		// Load non-generic plugin and save it to cache
	BOOL		ReloadPlugin(LPCTSTR pszGroup, LPCTSTR pszType);	// Reload non-generic plugin within cache
//	void		UnloadPlugin(REFCLSID pCLSID);						// Unload plugin (from cache and generic plugin list)

	UINT		GetCommandID(); 									// Retrieve next free command ID

	// IGeneralPlugin mirroring:
	void		OnSkinChanged();

	// ICommandPlugin mirroring:
	void		RegisterCommands();
	void		InsertCommands();
	BOOL		OnCommand(CChildWnd* pActiveWnd, UINT nCommandID);
	BOOL		OnUpdate(CChildWnd* pActiveWnd, CCmdUI* pCmdUI);

	// IExecutePlugin mirroring:
	BOOL		OnExecuteFile(LPCTSTR pszFile, BOOL bUseImageViewer = FALSE);
	BOOL		OnEnqueueFile(LPCTSTR pszFile);

	// ILibraryPlugin mirroring
	BOOL		OnNewFile(CLibraryFile* pFile);

	// IChatPlugin mirroring:
	BOOL		OnChatMessage(LPCTSTR pszChatID, BOOL bOutgoing, LPCTSTR pszFrom, LPCTSTR pszTo, LPCTSTR pszMessage);

	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CPlugin* GetNext(POSITION& pos) const
	{
		return m_pList.GetNext( pos );
	}

private:
	typedef struct
	{
		CComGITPtr< IUnknown >	m_pGIT;
		CComPtr< IUnknown >		m_pIUnknown;
	} CPluginPtr;
	typedef CMap< CLSID, const CLSID&, CPluginPtr*, CPluginPtr* > CPluginMap;

	CPluginMap			m_pCache;		// Non-generic plugin cache
	CList< CPlugin* >	m_pList;		// Generic plugins
	CEvent				m_pReady;		// Interface creation completed
	CLSID				m_inCLSID;		// [in] Create this interface
public:
	UINT				m_nCommandID;	// First free command ID

private:
	virtual void OnRun();

	CPlugins(const CPlugins&);
	CPlugins& operator=(const CPlugins&);
};


class CPlugin
{
public:
	CPlugin(REFCLSID pCLSID, LPCTSTR pszName);
	~CPlugin();

	CLSID						m_pCLSID;
	CString						m_sName;
	DWORD						m_nCapabilities;
	CComPtr< IGeneralPlugin >	m_pPlugin;
	CComPtr< ICommandPlugin >	m_pCommand;
	CComPtr< IExecutePlugin >	m_pExecute;
	CComPtr< ILibraryPlugin >	m_pLibrary;
	CComPtr< IChatPlugin >		m_pChat;

	BOOL		Start();
	void		Stop();
	CString		GetStringCLSID() const;
	HICON		LookupIcon() const;

private:
	CPlugin(const CPlugin&);
	CPlugin& operator=(const CPlugin&);
};

template<> AFX_INLINE UINT AFXAPI HashKey(const CLSID& key)
{
	return ( key.Data1 + MAKEDWORD( key.Data2, key.Data3 ) +
		*(UINT*)&key.Data4[0] + *(UINT*)&key.Data4[4] ) & 0xffffffff;
}

extern CPlugins Plugins;
