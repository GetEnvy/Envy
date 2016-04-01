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
#define NTDDI_VERSION	0x05010200	// NTDDI_WINXPSP2
#define _WIN32_WINNT	0x0501		// XP
#include <sdkddkver.h>

#define VC_EXTRALEAN
#define STRICT

#define _WIN32_DCOM
#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_ALL_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlstr.h>

#include "LibGFL.h"
#include "Resource.h"

HRESULT SAFEgflLoadBitmap(LPCWSTR filename, GFL_BITMAP **bitmap, const GFL_LOAD_PARAMS *params, GFL_FILE_INFORMATION *info) throw();
HRESULT SAFEgflLoadBitmapFromMemory(const GFL_UINT8 * data, GFL_UINT32 data_length, GFL_BITMAP **bitmap, const GFL_LOAD_PARAMS *params, GFL_FILE_INFORMATION *info) throw();
HRESULT SAFEgflSaveBitmapIntoMemory(GFL_UINT8 ** data, GFL_UINT32 * data_length, const GFL_BITMAP *bitmap, const GFL_SAVE_PARAMS *params) throw();
HRESULT SAFEgflSaveBitmap(LPCWSTR filename, const GFL_BITMAP *bitmap, const GFL_SAVE_PARAMS *params) throw();
int GetFormatIndexByExt(LPCSTR ext);

using namespace ATL;
