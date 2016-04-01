//
// ImageViewerPlugin.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012
//
// Original author Michael Stokes released portions into the public domain.
// You are free to redistribute and modify this page without any restrictions.
//

#pragma once

#include "Resource.h"
#include "ImageViewer.h"

class CImageWindow;


class ATL_NO_VTABLE CImageViewerPlugin :
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CImageViewerPlugin, &CLSID_ImageViewerPlugin >,
	public IGeneralPlugin,
	public IExecutePlugin,
	public ICommandPlugin
{
// Construction
public:
	CImageViewerPlugin();
	virtual ~CImageViewerPlugin();

public:

// Attributes
	CComPtr<IApplication>	m_pApplication;
	CComPtr<IUserInterface>	m_pInterface;

	CImageWindow*	m_pWindow;
	HCURSOR			m_hcMove;

	UINT			m_nCmdBestFit;
	UINT			m_nCmdActualSize;
	UINT			m_nCmdRefresh;
	UINT			m_nCmdDelete;
	UINT			m_nCmdClose;
	UINT			m_nCmdNext;
	UINT			m_nCmdPrior;
	UINT			m_nCmdFirst;
	UINT			m_nCmdLast;

// Operations
	BOOL			OpenNewWindow(LPCTSTR pszFilePath);
	void			RemoveWindow(CImageWindow* pWindow);

// Interfaces
	DECLARE_REGISTRY_RESOURCEID(IDR_IMAGEVIEWER)

	BEGIN_COM_MAP(CImageViewerPlugin)
		COM_INTERFACE_ENTRY(IGeneralPlugin)
		COM_INTERFACE_ENTRY(IExecutePlugin)
		COM_INTERFACE_ENTRY(ICommandPlugin)
	END_COM_MAP()

protected:

// IGeneralPlugin
	virtual HRESULT STDMETHODCALLTYPE SetApplication(
		/* [in] */ IApplication __RPC_FAR *pApplication);
	virtual HRESULT STDMETHODCALLTYPE QueryCapabilities(
		/* [in] */ DWORD __RPC_FAR *pnCaps);
	virtual HRESULT STDMETHODCALLTYPE Configure();
	virtual HRESULT STDMETHODCALLTYPE OnSkinChanged();

// IExecutePlugin
	virtual HRESULT STDMETHODCALLTYPE OnExecute(
		/* [in] */ BSTR sFilePath );
	virtual HRESULT STDMETHODCALLTYPE OnEnqueue(
		/* [in] */ BSTR sFilePath );

// ICommandPlugin
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

OBJECT_ENTRY_AUTO(__uuidof(ImageViewerPlugin), CImageViewerPlugin)
