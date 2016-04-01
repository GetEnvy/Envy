//
// StdAfx.h
//
// Source file for standard system include files,
// or project files that are used frequently but changed infrequently
//
// Originally Created by: Rolandas Rudomanskis

#pragma once

#if !defined(_UNICODE) || !defined(UNICODE)
	#error Unicode Required
#endif

// TargetVer.h: (WINVER)
#define NTDDI_VERSION	0x05010200	// NTDDI_WINXPSP2
#define _WIN32_WINNT	0x0501		// XP
#include <sdkddkver.h>

#define VC_EXTRALEAN
#define STRICT

#define _CRT_SECURE_NO_DEPRECATE	// Disable VC8 deprecation warnings
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// Some CString constructors will be explicit
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_APARTMENT_THREADED
#define _ATL_ALL_WARNINGS			// Turn off ATL's hiding some common and often safely ignored warning messages
#define _HAS_EXCEPTIONS 0

#pragma warning( push, 0 )

#include "Resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <string>

#pragma warning( pop )

using namespace ATL;

#include "Globals.h"
