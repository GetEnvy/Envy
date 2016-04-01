//
// SWFReader.h : Declaration of the CSWFReader
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008 and Nikolay Raspopov 2005
//
// Envy is free software; you can redistribute it and/or
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

#pragma once

#include "Resource.h"
#include "SWFPlugin.h"

class ATL_NO_VTABLE CSWFReader :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSWFReader, &CLSID_SWFReader>,
	public IImageServicePlugin
{
public:
	CSWFReader() throw()
	{
		m_pUnkMarshaler = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SWFREADER)

BEGIN_COM_MAP(CSWFReader)
	COM_INTERFACE_ENTRY(IImageServicePlugin)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_GET_CONTROLLING_UNKNOWN()

	CComPtr<IUnknown> m_pUnkMarshaler;

	HRESULT FinalConstruct () throw();
	void FinalRelease () throw();

// IImageServicePlugin
public:
	STDMETHOD(LoadFromFile)(
		/* [in] */ BSTR sFile,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [out] */ SAFEARRAY** ppImage );
	STDMETHOD(LoadFromMemory)(
		/* [in] */ BSTR sType,
		/* [in] */ SAFEARRAY* pMemory,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [out] */ SAFEARRAY** ppImage );
	STDMETHOD(SaveToFile)(
		/* [in] */ BSTR sFile,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [in] */ SAFEARRAY* pImage);
	STDMETHOD(SaveToMemory)(
		/* [in] */ BSTR sType,
		/* [out] */ SAFEARRAY** ppMemory,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [in] */ SAFEARRAY* pImage);
};

OBJECT_ENTRY_AUTO(__uuidof(SWFReader), CSWFReader)
