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
#ifndef _SECURE_ATL
	#define _SECURE_ATL	1
#endif

#define ISOLATION_AWARE_ENABLED 1

#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>	// CAtlMap
#include <atlstr.h>

#pragma pack(push)

#include "Unrar.h"

#pragma pack(pop)

typedef HANDLE (PASCAL *tRAROpenArchiveEx)(struct RAROpenArchiveDataEx *ArchiveData);
typedef int    (PASCAL *tRARCloseArchive)(HANDLE hArcData);
typedef int    (PASCAL *tRARReadHeaderEx)(HANDLE hArcData,struct RARHeaderDataEx *HeaderData);
typedef int    (PASCAL *tRARProcessFileW)(HANDLE hArcData,int Operation,wchar_t *DestPath,wchar_t *DestName);

extern tRAROpenArchiveEx fnRAROpenArchiveEx;
extern tRARCloseArchive fnRARCloseArchive;
extern tRARReadHeaderEx fnRARReadHeaderEx;
extern tRARProcessFileW fnRARProcessFileW;

using namespace ATL;

// RAROpenArchiveDataEx::Flags
#define RAR_HEAD_VOLUME_ATTR	0x0001	// Volume attribute (archive volume)
#define RAR_HEAD_COMMENT		0x0002	// Archive comment present
#define RAR_HEAD_LOCKED			0x0004	// Archive lock attribute
#define RAR_HEAD_SOLID			0x0008	// Solid attribute (solid archive)
#define RAR_HEAD_NEW_NAMING		0x0010	// New volume naming scheme (‘volname.partN.rar’)
#define RAR_HEAD_AUTH			0x0020	// Authenticity information present
#define RAR_HEAD_RECOVERY		0x0040	// Recovery record present
#define RAR_HEAD_ENCRYPTED		0x0080	// Block headers are encrypted
#define RAR_HEAD_FIRST_VOLUME	0x0100	// First volume (set by RAR 3.0+)

// RARHeaderDataEx::Flags
#define RAR_FILE_PREVIOUS		0x01	// File continued from previous volume
#define RAR_FILE_NEXT			0x02	// File continued on next volume
#define RAR_FILE_ENCRYPTED		0x04	// File encrypted with password
#define RAR_FILE_COMMENT		0x08	// File comment present
#define RAR_FILE_SOLID			0x10	// Compression of previous files is used (solid flag)
#define RAR_FILE_DIRECTORY		0xe0	// File is directory
