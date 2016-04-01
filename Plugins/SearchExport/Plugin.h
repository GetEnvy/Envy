//
// Plugin.h : Declaration of the CPlugin
//

#pragma once

#include "SearchExport.h"

// CPlugin

class ATL_NO_VTABLE CPlugin :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPlugin, &CLSID_Plugin>,
	public IGeneralPlugin,
	public ICommandPlugin
{
public:
	CPlugin() :
		m_nCmdCheck( 0 )
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PLUGIN)

BEGIN_COM_MAP(CPlugin)
	COM_INTERFACE_ENTRY(IGeneralPlugin)
	COM_INTERFACE_ENTRY(ICommandPlugin)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		m_pApplication.Release();
		m_pUserInterface.Release();
	}

protected:
	CComPtr< IApplication >		m_pApplication;		// Envy application
	CComPtr< IUserInterface >	m_pUserInterface;	// Envy GUI
	UINT						m_nCmdCheck;		// Command ID

	HRESULT Export(IGenericView* pGenericView, LONG nCount);

	// Insert menu item only if no item present
	void InsertCommand(LPCTSTR szTitle, const LPCWSTR* szMenu, UINT nID);

// IGeneralPlugin
public:
	STDMETHOD(SetApplication)(
		/* [in] */ IApplication __RPC_FAR *pApplication);
	STDMETHOD(QueryCapabilities)(
		/* [in] */ DWORD __RPC_FAR *pnCaps);
	STDMETHOD(Configure)();
	STDMETHOD(OnSkinChanged)();

// ICommandPlugin
public:
	STDMETHOD(RegisterCommands)();
	STDMETHOD(InsertCommands)();
	STDMETHOD(OnUpdate)(
		/* [in] */ UINT nCommandID,
		/* [out][in] */ TRISTATE __RPC_FAR *pbVisible,
		/* [out][in] */ TRISTATE __RPC_FAR *pbEnabled,
		/* [out][in] */ TRISTATE __RPC_FAR *pbChecked);
	STDMETHOD(OnCommand)(
		/* [in] */ UINT nCommandID);
};

OBJECT_ENTRY_AUTO(__uuidof(Plugin), CPlugin)
