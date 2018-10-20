//
// StdAfx.h
//
// This file is part of Torrent Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008-2015
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#if !defined(_UNICODE) || !defined(UNICODE)
	#error Unicode Required
#endif


// Generate Manifest  (Themed Controls)
//#ifdef _UNICODE
#ifdef _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


#define WINVER				0x0600
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS		0x0600
#define NTDDI_VERSION		NTDDI_LONGHORN  // Minimum build target = Vista

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN						// Exclude rarely-used things from Windows headers
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _SECURE_ATL 1

#define _ATL_NO_COM_SUPPORT					// Prevents ATL COM-related code
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// Makes certain ATL CString constructors explicit

#if defined(_MSC_VER) && ( _MSC_VER < 1900 || _MSC_FULL_VER > 190022820 )
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS		// Smaller filesize VS2012+ (VS2015 RC error)
#endif

#include <sdkddkver.h>		// Setup versioning for Windows SDK/DDK
#include <afxwin.h> 		// MFC core and standard components
#include <afxext.h> 		// MFC extensions
#include <afxcmn.h> 		// MFC support for Windows Common Controls
#include <afxmt.h>			// MFC multithreading
#include <atlbase.h>		// ATL
#include <shlobj.h> 		// Shell objects
#include <shlwapi.h>		// Shell win api

typedef unsigned __int64 QWORD;

#ifndef OFN_ENABLESIZING
  #define OFN_ENABLESIZING		0x00800000
#endif
#ifndef BIF_NEWDIALOGSTYLE
  #define BIF_NEWDIALOGSTYLE	0x00000040
#endif

#ifndef _PORTABLE
#include "..\HashLib\HashLib.h"
//#include "..\Envy\Strings.h"
//#include "..\Envy\Buffer.h"	// Using local file
//#include "..\Envy\BENode.h"	// Using local file
#endif
