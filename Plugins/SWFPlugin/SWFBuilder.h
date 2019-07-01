//
// SWFBuilder.h : Declaration of the CSWFBuilder
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Nikolay Raspopov 2005 and PeerProject 2008-2010
//
// GFL Library, GFL SDK and XnView
// Copyright (c) 1991-2004 Pierre-E Gougelet
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
#include "SWFPlugin.h"

class ATL_NO_VTABLE CSWFBuilder :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSWFBuilder, &CLSID_SWFBuilder>,
	public ILibraryBuilderPlugin
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_SWFBUILDER)

BEGIN_COM_MAP(CSWFBuilder)
	COM_INTERFACE_ENTRY(ILibraryBuilderPlugin)
END_COM_MAP()

// ILibraryBuilderPlugin
public:
	STDMETHOD(Process)(
		/* [in] */ BSTR sFile,
		/* [in] */ ISXMLElement* pXML);
};

OBJECT_ENTRY_AUTO(__uuidof(SWFBuilder), CSWFBuilder)
