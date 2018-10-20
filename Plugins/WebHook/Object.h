//
// Object.h : Declaration of the CWebHook
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2009 and PeerProject 2009-2010
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "Resource.h"
#include "WebHook_i.h"

// CWebHook

typedef IDispatchImpl< IWebHook, &IID_IWebHook, &LIBID_WebHookLib, /*wMajor =*/ 1, /*wMinor =*/ 0 > IWebHookDispatchImpl;

class ATL_NO_VTABLE CWebHook :
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< CWebHook, &CLSID_WebHook >,
	public IWebHookDispatchImpl,
	public IObjectWithSiteImpl< CWebHook >
{
public:
	CWebHook();

DECLARE_REGISTRY_RESOURCEID(IDR_OBJECT)

BEGIN_COM_MAP(CWebHook)
	COM_INTERFACE_ENTRY(IWebHook)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_GET_CONTROLLING_UNKNOWN()

	HRESULT FinalConstruct();
	void FinalRelease();

protected:
	CComPtr < IUnknown >	m_pUnkMarshaler;
	DWORD					m_dwCookie;
	CString					m_sURL;

	bool IsEnabled() const;
	bool IsHooked(const CString& sExt) const;
	void Connect();
	void Disconnect();
	void AddLink(const CString& sURL);

// IDispatchImpl
	STDMETHOD(Invoke)(
		/* [in] */ DISPID dispIdMember,
		/* [in] */ REFIID riid,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [out][in] */ DISPPARAMS *pDispParams,
		/* [out] */ VARIANT *pVarResult,
		/* [out] */ EXCEPINFO *pExcepInfo,
		/* [out] */ UINT *puArgErr);

// IObjectWithSite
	STDMETHOD(SetSite)(
		/* [in] */ IUnknown* pUnkSite);

// IWebHook
	STDMETHOD(AddLink)(
		/* [in] */ VARIANT oLink);
};

OBJECT_ENTRY_AUTO(__uuidof(WebHook), CWebHook)
