//
// StdAfx.h
//
// Source file for standard system include files,
// or project files that are used frequently but changed infrequently
//

#pragma once

#pragma warning( disable : 4127 )	// conditional expression is constant

#if !defined(_UNICODE) || !defined(UNICODE)
	#error Unicode Required
#endif

// TargetVer.h: (WINVER)
#define NTDDI_VERSION	0x05010200	// NTDDI_WINXPSP2
#define _WIN32_WINNT	0x0501		// XP
#include <sdkddkver.h>

#define VC_EXTRALEAN
#define STRICT

#define _WIN32_DCOM
#define _CRT_SECURE_NO_DEPRECATE
#define _ATL_APARTMENT_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_ALL_WARNINGS

#include "Resource.h"

#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlstr.h>

using namespace ATL;
