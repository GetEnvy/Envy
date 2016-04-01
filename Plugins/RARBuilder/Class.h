//
// Class.h : Declaration of the CClass
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2007
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
#include "RARBuilder.h"

class ATL_NO_VTABLE CRARBuilder :
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< CRARBuilder, &CLSID_RARBuilder >,
	public ILibraryBuilderPlugin
{
public:
	CRARBuilder() throw()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CLASS)

BEGIN_COM_MAP(CRARBuilder)
	COM_INTERFACE_ENTRY(ILibraryBuilderPlugin)
END_COM_MAP()

// ILibraryBuilderPlugin
public:
	STDMETHOD(Process)(/*[in]*/ BSTR sFile, /*[in]*/ ISXMLElement* pXML);
};

OBJECT_ENTRY_AUTO(__uuidof(RARBuilder), CRARBuilder)
