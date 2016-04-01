// stdafx.h : include file for standard system include files,
//            or project specific include files that are used frequently, but are changed infrequently

#pragma once

// Change these values to use different versions
#define WINVER			0x0600 // 0x0501 Use Vista; 0x0501 is for Windows XP
#define _WIN32_WINDOWS	0x0600 // 0x0501 for XP
#define _WIN32_WINNT	0x0600 // 0x0500 for 2000
#define _WIN32_IE		0x0501 // 0x0700 for IE7; 0x0501 is for Internet Explorer 5.01
#define _RICHEDIT_VER	0x0300 // 0x0100

#define _WTL_NO_WTYPES      // WTL shouldn't define CRect/CPoint/CSize
#define _WTL_NO_CSTRING     // WTL shouldn't define CString (i.e., using ATLString)

//#define _WTL_USE_CSTRING
//#define _CRT_SECURE_NO_DEPRECATE

#include <atlbase.h>
#include <atlstr.h>     // shared CString
#include <atltypes.h>   // shared CRect/CPoint/CSize

#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlimage.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlddx.h>

#include <atltheme.h>

#include <ATLComTime.h>	// Date and Time classes

#include <atlprint.h>

#include <gdiplus.h>
using namespace Gdiplus;

#ifdef WIN64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
