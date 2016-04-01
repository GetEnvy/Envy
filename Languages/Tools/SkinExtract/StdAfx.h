//
// StdAfx.h
//

#pragma once

// TargetVer.h: 0x0601 Windows 7, 0x0A00 Windows 10

#ifndef WINVER
#define WINVER 0x0601
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0601
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0800
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT

#include <tchar.h>
#include <stdio.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <list>
