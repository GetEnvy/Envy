//
// Packages.h
//
// This file is part of Envy (getenvy.com) © 2016
//
// Portions of this page have been previously released into the public domain.
// You are free to redistribute and modify it without any restrictions
// with the exception of the following notice:
//
// The Zlib  library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
// The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.

#pragma once

#include <stddef.h>
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <stddef.h>
#include <stdio.h>
#include <commctrl.h>
#include <unzip.h>
#include <errno.h>
#include "Resource.h"

#define VERSION        L"1.0"
#define MAIN_HWND      L"EnvyMainWnd"
#define SKIN_TITLE     L"Envy Skin Installer"
#define PACKAGE_TITLE  L"Envy Package Installer"

enum
{
	typeSkin,
	typeLang,
	typeSchemas,
	typePlugin,
	typeData
};

// Globals
extern int	  skinType;
extern TCHAR* szName;
extern TCHAR* szPath;
extern TCHAR* szVersion;
extern TCHAR* szAuthor;
extern TCHAR* szUpdates;
extern TCHAR* szXML;
extern TCHAR  skins_dir[MAX_PATH];	// Full path to Skin folder

// Extract.c
void ExtractSkinFile(LPCTSTR pszFile);
void GetInstallDirectory();
int GetSkinFileCount(LPTSTR pszFile);
int ValidateSkin(LPTSTR pszFile, HWND hwndDlg);
int ExtractSkin(LPTSTR pszFile, HWND hwndDlg);
LPCTSTR GetUnicodeString(char* pszString);

// Registry.c
LSTATUS CreateSkinKeys();
LSTATUS DeleteSkinKeys();

// Utils.c
void LoadManifestInfo(char *buf);
int SetSkinAsDefault();
int MakeDirectory(LPCTSTR newdir);

// Window.c
INT_PTR CALLBACK ExtractProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


// Generate Manifest  (Themed Controls)
//#ifdef _UNICODE (Required)
#ifdef _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
