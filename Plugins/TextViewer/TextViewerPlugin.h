//
// TextViewerPlugin.h
//
// This file is part of Envy (getenvy.com) © 2010
// TextViewer plugin is released under the Persistent Public Domain license.
//
// This code may be treated as Public Domain, provided:
// the work in all its forms and attendant uses shall remain available as
// persistently "Public Domain" until it naturally enters the public domain.
// History remains immutable:  Authors do not disclaim copyright, but do disclaim
// all rights beyond asserting the reach and duration and spirit of this license.

#pragma once

#include "Resource.h"
#include "TextViewer.h"

class CTextWindow;


class ATL_NO_VTABLE CTextViewerPlugin :
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CTextViewerPlugin, &CLSID_TextViewerPlugin >,
	public IGeneralPlugin,
	public IExecutePlugin,
	public ICommandPlugin
{
// Construction
public:
	CTextViewerPlugin();
	virtual ~CTextViewerPlugin();

// Attributes
public:
	CComPtr<IApplication>	m_pApplication;
	CComPtr<IUserInterface>	m_pInterface;

	CTextWindow*	m_pWindow;
	HCURSOR			m_hcMove;

	UINT			m_nCmdBestFit;
	UINT			m_nCmdActualSize;
	UINT			m_nCmdRefresh;
	UINT			m_nCmdClose;

// Operations
public:
	BOOL			OpenNewWindow(LPCTSTR pszFilePath);
	void			RemoveWindow(CTextWindow* pWindow);

// Interfaces
public:
	DECLARE_REGISTRY_RESOURCEID(IDR_TEXTVIEWER)

	BEGIN_COM_MAP(CTextViewerPlugin)
		COM_INTERFACE_ENTRY(IGeneralPlugin)
		COM_INTERFACE_ENTRY(IExecutePlugin)
		COM_INTERFACE_ENTRY(ICommandPlugin)
	END_COM_MAP()

// IGeneralPlugin
protected:
	virtual HRESULT STDMETHODCALLTYPE SetApplication(
		/* [in] */ IApplication __RPC_FAR *pApplication);
	virtual HRESULT STDMETHODCALLTYPE QueryCapabilities(
		/* [in] */ DWORD __RPC_FAR *pnCaps);
	virtual HRESULT STDMETHODCALLTYPE Configure();
	virtual HRESULT STDMETHODCALLTYPE OnSkinChanged();

// IExecutePlugin
protected:
	virtual HRESULT STDMETHODCALLTYPE OnExecute(
		/* [in] */ BSTR sFilePath );
	virtual HRESULT STDMETHODCALLTYPE OnEnqueue(
		/* [in] */ BSTR sFilePath );

// ICommandPlugin
protected:
	virtual HRESULT STDMETHODCALLTYPE RegisterCommands();
	virtual HRESULT STDMETHODCALLTYPE InsertCommands();
	virtual HRESULT STDMETHODCALLTYPE OnUpdate(
		/* [in] */ UINT nCommandID,
		/* [out][in] */ TRISTATE __RPC_FAR *pbVisible,
		/* [out][in] */ TRISTATE __RPC_FAR *pbEnabled,
		/* [out][in] */ TRISTATE __RPC_FAR *pbChecked);
	virtual HRESULT STDMETHODCALLTYPE OnCommand(
		/* [in] */ UINT nCommandID);

};
