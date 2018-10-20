//
// Class.h : Declaration of the CClass
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008-2010
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
#include "7ZipBuilder.h"

class ATL_NO_VTABLE C7ZipBuilder :
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< C7ZipBuilder, &CLSID_SevenZipBuilder >,
	public ILibraryBuilderPlugin
{
public:
	C7ZipBuilder() throw()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CLASS)

BEGIN_COM_MAP(C7ZipBuilder)
	COM_INTERFACE_ENTRY(ILibraryBuilderPlugin)
END_COM_MAP()

// ILibraryBuilderPlugin
public:
	STDMETHOD(Process)(/*[in]*/ BSTR sFile, /*[in]*/ ISXMLElement* pXML);
};

OBJECT_ENTRY_AUTO(__uuidof(SevenZipBuilder), C7ZipBuilder)
