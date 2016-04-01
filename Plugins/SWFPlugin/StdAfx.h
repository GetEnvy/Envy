//
// StdAfx.h
//
// Source file for standard system include files,
// or project files that are used frequently but changed infrequently
//

#pragma once

#pragma warning ( disable : 4505 )	// unreferenced local function has been removed

#if !defined(_UNICODE) || !defined(UNICODE)
	#error Unicode Required
#endif

//#if defined(_DEBUG) && (_MSC_VER >= 1800)
// ToDo: Fix Install Assert  (Workaround use VS2012 profile)

// TargetVer.h: (WINVER)
#define NTDDI_VERSION	0x05010200	// NTDDI_WINXPSP2
#define _WIN32_WINNT	0x0501		// XP
#include <sdkddkver.h>

#define VC_EXTRALEAN
#define STRICT

#define _WIN32_DCOM
#define _CRT_SECURE_NO_DEPRECATE
//#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_FREE_THREADED
#define _ATL_ALL_WARNINGS

#pragma warning( push, 0 )

#include "Resource.h"

#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlstr.h>

//using namespace ATL;

const long cx = 256;
const long cy = 256;

typedef struct {
	HBITMAP				hBitmap;
	BITMAPINFOHEADER	bmiHeader;
} MY_DATA;

extern MY_DATA*			_Data;
extern CRITICAL_SECTION	_CS;

#pragma warning( pop )
