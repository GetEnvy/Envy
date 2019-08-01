//
// StdAfx.h
//
// Source file for standard system include files,
// or project files that are used frequently but changed infrequently
//

#pragma once

#if !defined(_UNICODE) || !defined(UNICODE)
	#error Unicode Required
#endif

// TargetVer.h: (WINVER)
#ifdef WIN64
#define NTDDI_VERSION	0x06000000	// NTDDI_VISTA
#define _WIN32_WINNT	0x0600		// Vista
#else
#define NTDDI_VERSION	0x05010200	// NTDDI_WINXPSP2
#define _WIN32_WINNT	0x0501		// XP
#endif
#include <sdkddkver.h>

#define VC_EXTRALEAN
#define STRICT

#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#include "Resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <thumbcache.h>		// IThumbnailCache, ISharedBitmap

using namespace ATL;
