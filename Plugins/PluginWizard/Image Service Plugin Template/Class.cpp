//
// Class.cpp : Implementation of CClass
//
// #COPYRIGHT#
// This file is part of Envy (getenvy.com) © 2018
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
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
