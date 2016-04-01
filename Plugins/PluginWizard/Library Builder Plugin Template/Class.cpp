//
// Class.cpp : Implementation of CClass
//
// This file is part of Envy (getenvy.com) © 2010
// #COPYRIGHT#
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

#include "StdAfx.h"
#include "Class.h"

STDMETHODIMP C#PROJECT#::Process (
	/* [in] */ HANDLE /*hFile*/,
	/* [in] */ BSTR sFile,
	/* [in] */ ISXMLElement* pXML)
{
/*
	if ( ! pXML )
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements( &pISXMLRootElements );
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create( CComBSTR("videos"), &pXMLRootElement );
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes( &pISXMLRootAttributes );
	if ( FAILED( hr ) )
		return hr;
	pISXMLRootAttributes->Add( CComBSTR("xmlns:xsi"),
		CComBSTR("http://www.w3.org/2001/XMLSchema-instance") );
	pISXMLRootAttributes->Add( CComBSTR("xsi:noNamespaceSchemaLocation"),
		CComBSTR("http://schemas.getenvy.com/Video.xsd") );

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements( &pISXMLElements );
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create( CComBSTR("video"), &pXMLElement );
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes( &pISXMLAttributes );
	if ( FAILED( hr ) )
		return hr;

	pISXMLAttributes->Add( CComBSTR("width"), CComBSTR("640") );
	pISXMLAttributes->Add( CComBSTR("height"), CComBSTR("480") );
*/
	ATLTRACENOTIMPL("C#PROJECT#::Process");
}
