//
// StdAfx.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2014 and Nikolay Raspopov 2014
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
// Source file for standard system include files,
// or project files that are used frequently but changed infrequently
//

#pragma once

#if !defined(_UNICODE) || !defined(UNICODE)
	#error Unicode Required
#endif

// TargetVer.h: (WINVER)
#define NTDDI_VERSION	0x05010200	// NTDDI_WINXPSP2
#define _WIN32_WINNT	0x0501		// XP
#include <sdkddkver.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define STRICT

#define _SECURE_ATL 1
#define _CRT_SECURE_NO_WARNINGS

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT

#include "Resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <atlsafe.h>
#include <atlstr.h>
#include <atlhost.h>
#include <ExDisp.h>
#include <Shlobj.h>
#include <Shellapi.h>
#include <Wininet.h>

using namespace ATL;

#include "..\..\Envy\Strings.h"

CString LoadString( UINT nID );
CString GetURLs();
BOOL SaveURLs(const CString& sURLs);
