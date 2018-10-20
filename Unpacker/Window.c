//
// Window.c
//
// This file is part of Envy (getenvy.com) © 2016-2018
//
// Portions of this page have been previously released into the public domain.
// You are free to redistribute and modify it without any restrictions
// with the exception of the following notice:
//
// The Zlib  library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
// The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.

#include "Unpacker.h"

// EXPORT BEGIN
INT_PTR CALLBACK ExtractProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hBannerBmp = NULL;
	static TCHAR* szFile = NULL;
	static int maxPos = 1;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			HWND hBanner;

			EnableWindow(GetDlgItem(hwndDlg,IDC_CONFIG), FALSE);

			szFile = (LPTSTR)lParam;
			maxPos = GetSkinFileCount( szFile );
			if ( !maxPos ) maxPos = 1;
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ENVY)));
			hBannerBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BANNER),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
			hBanner = CreateWindow(L"STATIC", NULL, WS_VISIBLE|WS_CHILD|SS_BITMAP, 0, 0, 293, 172, hwndDlg, NULL, GetModuleHandle(NULL), NULL);
			SendMessage(hBanner, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBannerBmp);
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0,maxPos));
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETSTEP, 1, 0);

			if ( !ValidateSkin(szFile, hwndDlg) )
			{
				SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
				SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), L"Please verify this file is a valid Envy Package and try again.");
				EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_INSTALL), FALSE);
			}
			else if ( !skinType )	// Default typeSkin
			{
				SetWindowText(hwndDlg, SKIN_TITLE);
			}
			else
			{
				SetWindowText(hwndDlg, PACKAGE_TITLE);
				if ( skinType == typeLang )
					SetWindowText(GetDlgItem(hwndDlg, IDC_CONFIG), L"Configure &Language...");
			}

			{
				LOGFONT lf;
				HFONT hFont;
				hFont=(HFONT)SendDlgItemMessage(hwndDlg, IDC_NAME, WM_GETFONT, 0, 0);
				GetObject(hFont,sizeof(lf),&lf);
				lf.lfWeight=FW_BOLD;
				hFont=CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_NAME, WM_SETFONT, (WPARAM)hFont, 0);
			}
			if ( szPath )
			{
				TCHAR buf[MAX_PATH], tbuf[MAX_PATH];
				_snwprintf(buf, MAX_PATH, L"%s %s", szName, szVersion ? szVersion : L"");
				_snwprintf(tbuf, MAX_PATH, L"%s - %s", szName, skinType ? PACKAGE_TITLE : SKIN_TITLE);
				SetDlgItemText(hwndDlg, IDC_NAME, buf);
				SetWindowText(hwndDlg, tbuf);
			}
			if ( szAuthor )
			{
				TCHAR buf[MAX_PATH];
				_snwprintf(buf, MAX_PATH, L"By %s", szAuthor);
				SetDlgItemText(hwndDlg, IDC_AUTH, buf);
			}
			if ( szUpdates && wcscmp(szAuthor, szUpdates) != 0 )
			{
				TCHAR buf[MAX_PATH];
				if ( szAuthor )
				{
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
					GetDlgItemText(hwndDlg, IDC_AUTH, buf, MAX_PATH);
					size_t len = wcslen(buf);
					_snwprintf(buf + len, MAX_PATH - len, L",  Updated by %s", szUpdates);
#else	// VS2012~
					TCHAR upbuf[MAX_PATH];
					_snwprintf(upbuf, MAX_PATH, L",  Updated by %s", szUpdates);
					GetDlgItemText(hwndDlg, IDC_AUTH, buf, MAX_PATH);
					wcsncat(buf, upbuf, MAX_PATH - wcslen(buf));
#endif
				}
				else
				{
					_snwprintf(buf, MAX_PATH, L"Updated by %s", szUpdates);
				}
				SetDlgItemText(hwndDlg, IDC_AUTH, buf);
			}
			if ( szPath )
			{
				TCHAR buf[MAX_PATH], updbuf[MAX_PATH];
				if ( szAuthor )
				{
					_snwprintf(updbuf, MAX_PATH, L"        (%s Folder)", szPath);
					GetDlgItemText(hwndDlg, IDC_AUTH, buf, 256);
					wcsncat(buf, updbuf, MAX_PATH - wcslen(buf));
				}
				else
					_snwprintf(buf, MAX_PATH, L"(%s Folder)", szPath);
				SetDlgItemText(hwndDlg, IDC_AUTH, buf);
			}
			SetWindowLongPtr( GetDlgItem(hwndDlg,IDC_WHITERECT), GWL_STYLE, WS_VISIBLE|WS_CHILD|SS_LEFT|SS_OWNERDRAW );
			SetWindowLongPtr( GetDlgItem(hwndDlg,IDC_NAME), GWL_STYLE, WS_VISIBLE|WS_CHILD|SS_LEFT|SS_OWNERDRAW );
			SetWindowLongPtr( GetDlgItem(hwndDlg,IDC_AUTH), GWL_STYLE, WS_VISIBLE|WS_CHILD|SS_LEFT|SS_OWNERDRAW );
			SetWindowLongPtr( GetDlgItem(hwndDlg,IDC_STATUS), GWL_STYLE, WS_VISIBLE|WS_CHILD|SS_LEFT|SS_OWNERDRAW );
			break;
		}
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	case WM_DESTROY:
		DeleteObject(hBannerBmp);
		break;
	case WM_DRAWITEM:
		if ( (UINT)wParam == IDC_WHITERECT ||
			 (UINT)wParam == IDC_NAME ||
			 (UINT)wParam == IDC_AUTH ||
			 (UINT)wParam == IDC_STATUS )
		{
			TCHAR buf[MAX_PATH];
			LPDRAWITEMSTRUCT lpDrawItemStruct;
			lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
			FillRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, IntToPtr(WHITE_BRUSH+1));
			ExtFloodFill(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.top, lpDrawItemStruct->rcItem.left, RGB(0, 0, 0), FLOODFILLBORDER);
			SetBkMode(lpDrawItemStruct->hDC, TRANSPARENT);
			SetTextColor(lpDrawItemStruct->hDC, RGB(0, 0, 0));
			GetDlgItemText(hwndDlg, (UINT)wParam, buf, MAX_PATH);
			DrawText(lpDrawItemStruct->hDC, buf, (int)wcslen(buf), &lpDrawItemStruct->rcItem, DT_LEFT);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, 0);
			break;
		case IDC_INSTALL:
			{
				GetInstallDirectory();
				if ( !ExtractSkin(szFile, hwndDlg) )
				{
					SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
					SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), !skinType ?
						L"An error occured while extracting the skin.  Please try again." :
						L"An error occured while extracting the package.  Please try again." );
					EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
					break;
				}
				SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
				SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS),
					skinType == typeSkin ? L"Skin successfully installed." :
					skinType == typeLang ? L"Language successfully installed." :
					skinType == typePlugin ? L"Plugin successfully installed." :
				//	skinType == typeData ? L"Data successfully installed." :
					L"Package successfully installed.");
				EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_INSTALL), FALSE);
				if ( FindWindow(MAIN_HWND,NULL) )
					EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIG), TRUE);
			}
			break;
		case IDC_CONFIG:
			{
				HWND app = FindWindow(MAIN_HWND,NULL);
				if ( app )
				{
					if ( !IsZoomed(app) )
						PostMessage(app,WM_SYSCOMMAND,SC_RESTORE,0);
						PostMessage(app,WM_COMMAND,32879,0);
					SetFocus(app);
					if ( skinType == typeLang )
					{
						PostMessage(app,WM_COMMAND,32974,0);
					}
					else if ( SetSkinAsDefault() )
					{
						PostMessage(app,WM_COMMAND,32959,0);
						PostMessage(app,WM_COMMAND,32965,0);
					}
					EndDialog(hwndDlg, 0);
				}
			}
			break;
		}
		break;
	}

	return FALSE;
}
// EXPORT END
