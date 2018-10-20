//
// ClassFactory.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2006 and PeerProject 2008
// Originally Created by:	Rolandas Rudomanskis
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

#include "StdAfx.h"
#include "RatDVDPlugin.h"

////////////////////////////////////////////////////////////////////////
// CRatDVDClassFactory - IClassFactory Implementation
//
//  This is a fairly simple CF. We don't provide support for licensing
//  in this sample, nor aggregation. We just create and return
//  a new CRatDVDPlugin object.
//

////////////////////////////////////////////////////////////////////////
// QueryInterface
//

STDMETHODIMP CRatDVDClassFactory::QueryInterface(REFIID riid, void** ppv)
{
	ODS(L"CRatDVDClassFactory::QueryInterface\n");

	CHECK_NULL_RETURN(ppv, E_POINTER);

	if ( CLSID_RatDVDReader == riid )
	{
		*ppv = this;
		this->AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////
// AddRef
//
STDMETHODIMP_(ULONG) CRatDVDClassFactory::AddRef(void)
{
    return ++m_cRef;
}

////////////////////////////////////////////////////////////////////////
// Release
//
STDMETHODIMP_(ULONG) CRatDVDClassFactory::Release(void)
{
    if ( 0 != --m_cRef ) return m_cRef;

	ODS(L"CRatDVDClassFactory delete\n");

    LockServer(FALSE);
    return 0;
}

////////////////////////////////////////////////////////////////////////
// IClassFactory
//
////////////////////////////////////////////////////////////////////////
// CreateInstance
//
STDMETHODIMP CRatDVDClassFactory::CreateInstance(LPUNKNOWN punk, REFIID riid, void** ppv)
{
	HRESULT hr;

	ODS(L"CFileClassFactory::CreateInstance\n");

	CHECK_NULL_RETURN(ppv, E_POINTER);	*ppv = NULL;

 // This version does not support Aggregation...
	if (punk) return CLASS_E_NOAGGREGATION;

	if ( IID_ILibraryBuilderPlugin == riid || IID_IImageServicePlugin == riid )
	{
		CComObject<CRatDVDPlugin>*pRatDVDPlugin = new CComObject<CRatDVDPlugin>;

		CHECK_NULL_RETURN(pRatDVDPlugin, E_OUTOFMEMORY);
		hr = pRatDVDPlugin->QueryInterface( IID_IUnknown, ppv );
		if ( SUCCEEDED(hr) )
		{
			if ( IID_ILibraryBuilderPlugin == riid )
				*ppv = dynamic_cast<ILibraryBuilderPlugin*>(pRatDVDPlugin);
			else
				*ppv = dynamic_cast<IImageServicePlugin*>(pRatDVDPlugin);
		}
		else
			return hr;
	}
	else
		return E_NOINTERFACE;

	LockServer(TRUE);	// on success, bump up lock count

	return hr;
}

////////////////////////////////////////////////////////////////////////
// LockServer
//
STDMETHODIMP CRatDVDClassFactory::LockServer(BOOL fLock)
{
	if (fLock) DllAddRef();
	else DllRelease();
	return S_OK;
}
