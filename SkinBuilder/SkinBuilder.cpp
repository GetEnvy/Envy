//
// SkinBuilder.cpp : main source file for SkinBuilder.exe
//
// This file is part of Envy SkinBuilder (getenvy.com) © 2016
// Portions copyright PeerProject 2009 and Raj Pabari 2009
//
// Envy SkinBuilder is free software; you can redistribute it
// or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// SkinBuilder is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"

#include "Resource.h"

#include "AboutDlg.h"
#include "MainDlg.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);	// Free-threaded EXE Win2000+
	ATLASSERT(SUCCEEDED(hRes));

	// Resolve ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// Add flags to support other controls

	// GDI+ initialization
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	/*Gdiplus::Status gdiPlusStatus =*/ GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	// GDI+ deinitialization
	GdiplusShutdown(gdiplusToken);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
