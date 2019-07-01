//
// SWFBuilder.cpp : Implementation of CSWFBuilder
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Nikolay Raspopov 2005 and PeerProject 2008-2014
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

#include "StdAfx.h"
#include "SWFBuilder.h"

STDMETHODIMP CSWFBuilder::Process(
	/*[in]*/ BSTR /*sFile*/,
	/*[in]*/ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements( &pISXMLRootElements );
	if ( FAILED(hr) ) return hr;

	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create( CComBSTR("videos"), &pXMLRootElement );
	if ( FAILED(hr) ) return hr;

	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes( &pISXMLRootAttributes );
	if ( FAILED(hr) ) return hr;

	pISXMLRootAttributes->Add( CComBSTR("xmlns:xsi"),
		CComBSTR("http://www.w3.org/2001/XMLSchema-instance") );
	pISXMLRootAttributes->Add( CComBSTR("xsi:noNamespaceSchemaLocation"),
		CComBSTR("http://schemas.getenvy.com/Video.xsd") );

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements( &pISXMLElements );
	if ( FAILED(hr) ) return hr;

	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create( CComBSTR("video"), &pXMLElement );
	if ( FAILED(hr) ) return hr;

	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes( &pISXMLAttributes );
	if ( FAILED(hr) ) return hr;

	pISXMLAttributes->Add( CComBSTR("type"), CComBSTR("Shockwave Flash") );
	pISXMLAttributes->Add( CComBSTR("codec"), CComBSTR("SWF") );
	CString tmp;
	tmp.Format( L"%lu", cx );
	pISXMLAttributes->Add( CComBSTR("width"), CComBSTR(tmp) );
	tmp.Format( L"%lu", cy );
	pISXMLAttributes->Add( CComBSTR("height"), CComBSTR(tmp) );

	return hr;
}
