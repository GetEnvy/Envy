//
// Builder.cpp : Implementation of CBuilder
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2008-2014 and Nikolay Raspopov 2005
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
#include "Builder.h"

HRESULT CBuilder::FinalConstruct() throw()
{
	return CoCreateFreeThreadedMarshaler( GetControllingUnknown(), &m_pUnkMarshaler.p );
}

void CBuilder::FinalRelease() throw()
{
	m_pUnkMarshaler.Release();
}

STDMETHODIMP CBuilder::Process(
	/* [in] */ BSTR sFile,
	/* [in] */ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements( &pISXMLRootElements );
	if ( FAILED(hr) )
		return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create( CComBSTR("images"), &pXMLRootElement );
	if ( FAILED(hr) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes( &pISXMLRootAttributes );
	if ( FAILED(hr) )
		return hr;
	pISXMLRootAttributes->Add( CComBSTR("xmlns:xsi"),
		CComBSTR("http://www.w3.org/2001/XMLSchema-instance") );
	pISXMLRootAttributes->Add( CComBSTR("xsi:noNamespaceSchemaLocation"),
		CComBSTR("http://schemas.getenvy.com/Image.xsd") );

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements( &pISXMLElements );
	if ( FAILED(hr) )
		return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create( CComBSTR("image"), &pXMLElement );
	if ( FAILED(hr) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes( &pISXMLAttributes );
	if ( FAILED(hr) )
		return hr;

	GFL_FILE_INFORMATION inf = {};
	GFL_ERROR err = gflGetFileInformationW( (LPCWSTR)sFile, -1, &inf );

	if ( err != GFL_NO_ERROR )
	{
		WCHAR pszPath[ MAX_PATH * 2 ] = {};
		if ( GetShortPathNameW( (LPCWSTR)sFile, pszPath, MAX_PATH * 2 ) )
			err = gflGetFileInformationW( pszPath, -1, &inf );
		//else
		//	err = GFL_ERROR_FILE_OPEN;

		if ( err != GFL_NO_ERROR )
			return E_FAIL;
	}

	if ( inf.Height > 0 )
	{
		CString height;
		height.Format( _T("%d"), inf.Height );
		pISXMLAttributes->Add( CComBSTR("height"), CComBSTR(height) );
	}

	if ( inf.Width > 0 )
	{
		CString width;
		width.Format( _T("%d"), inf.Width );
		pISXMLAttributes->Add( CComBSTR("width"), CComBSTR(width) );
	}

	if ( *inf.Description )
		pISXMLAttributes->Add( CComBSTR("description"), CComBSTR(inf.Description) );

	CString colors;
	GFL_UINT16 bits = inf.ComponentsPerPixel * inf.BitsPerComponent;
	if ( inf.ColorModel == GFL_CM_GREY )
		colors = L"Greyscale";
	else if ( bits == 0 )
		; // No bits
	else if ( bits == 1 )
		colors = L"2";
	else if ( bits == 2 )
		colors = L"4";
	else if ( bits <= 4 )
		colors = L"16";
	else if ( bits <= 8 )
		colors = L"256";
	else if ( bits <= 16 )
		colors = L"64K";
	else if ( bits <= 24 )
		colors = L"16.7M";
	else
		colors = L"16.7M+Alpha";

	if ( colors.GetLength() )
		pISXMLAttributes->Add( CComBSTR("colors"), CComBSTR(colors) );

	return hr;
}
