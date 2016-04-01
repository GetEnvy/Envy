//
// Class.cpp : Implementation of CClass
//
// #COPYRIGHT#
// This file is part of Envy (getenvy.com) © 2010
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

STDMETHODIMP C#PROJECT#::LoadFromFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage )
{
	ATLTRACENOTIMPL ("C#PROJECT#::LoadFromFile");
}

STDMETHODIMP C#PROJECT#::LoadFromMemory (
	/* [in] */ BSTR sType,
	/* [in] */ SAFEARRAY* pMemory,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage )
{
	ATLTRACENOTIMPL ("C#PROJECT#::LoadFromMemory");
}

STDMETHODIMP C#PROJECT#::SaveToFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [in] */ SAFEARRAY* pImage)
{
	ATLTRACENOTIMPL ("C#PROJECT#::SaveToFile");
}

STDMETHODIMP C#PROJECT#::SaveToMemory (
	/* [in] */ BSTR sType,
	/* [out] */ SAFEARRAY** ppMemory,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [in] */ SAFEARRAY* pImage)
{
	ATLTRACENOTIMPL ("C#PROJECT#::SaveToMemory");
}
