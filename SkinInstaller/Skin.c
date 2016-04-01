//
// Skin.c
//
// This file is part of Envy (getenvy.com) © 2016
//
// Portions of this page have been previously released into the public domain.
// You are free to redistribute and modify it without any restrictions
// with the exception of the following notice:
//
// The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
// The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.

#include "Skin.h"

// Globals
int 	skinType;
TCHAR*	szName;
TCHAR*	szPath;
TCHAR*	szVersion;
TCHAR*	szAuthor;
TCHAR*	szUpdates;
TCHAR*	szXML;
TCHAR	skins_dir[MAX_PATH];

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR cmdParam, int cmdShow)
{
	int rtn = 0;

	InitCommonControls();

	// Globals
	skinType  = 0;
	szName    = NULL;
	szPath    = NULL;
	szVersion = NULL;
	szAuthor  = NULL;
	szUpdates = NULL;
	szXML     = NULL;

	if ( *cmdParam == 0 )
		MessageBox(NULL,L"Envy Skin Installer " VERSION L"\n\nDouble-click an Envy skin package file (.envy) to use this tool.",L"Envy Skin Installer",MB_OK|MB_ICONINFORMATION);
	else if ( ! _wcsicmp(cmdParam, L"/install") || ! _wcsicmp(cmdParam, L"/installsilent") )
		rtn = CreateSkinKeys();
	else if ( ! _wcsicmp(cmdParam, L"/uninstall") || ! _wcsicmp(cmdParam, L"/uninstallsilent") )
		rtn = DeleteSkinKeys();
	else
		ExtractSkinFile(cmdParam);

	// Free memory from globals
	free(szName);
	free(szPath);
	free(szVersion);
	free(szAuthor);
	free(szUpdates);
	free(szXML);

	return rtn;
}
