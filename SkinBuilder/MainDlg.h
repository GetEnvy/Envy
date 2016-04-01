//
// MainDlg.h : interface ofCMainDlg class
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


/////////////////////////////////////////////////////////////////////////////

#pragma once

// +++ STEP 1 +++
#include "RibbonTabs.h"

// +++ STEP 2 +++
class CMainDlg :	public CRibbonTabDialogImpl<CMainDlg>,
					public CDialogResize<CMainDlg>,
					public CUpdateUI<CMainDlg>,
					public CIdleHandler
{
private:
	CStatic	m_static;
	CFont	m_font;

public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL OnIdle() {	return FALSE; }

    BEGIN_DLGRESIZE_MAP(CReminderListDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC1, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_LIST1, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP_EX(CMainDlg)
// +++ STEP 3 +++
		CHAIN_MSG_MAP(CRibbonTabDialogImpl<CMainDlg>)

		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	//	COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)

// +++ STEP 5 +++
		COMMAND_ID_HANDLER(IDS_TAB1A, OnTab1ASelected)
		COMMAND_ID_HANDLER(IDS_TAB1D, OnTab1DSelected)
		MSG_WM_COMMAND(OnCommand)

		CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// Center the dialog on the screen
		CenterWindow();

		// Set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),  IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

		// Register object for idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		DlgResize_Init();

		// Create a big, bold font...
		CLogFont lf;
		lf.SetMessageBoxFont();
		lf.lfWeight = FW_BOLD;
		lf.MakeLarger(3);
		m_font.CreateFontIndirect(&lf);

		// ...and attach it to the static control
		m_static.Attach(GetDlgItem(IDC_STATIC1));
		m_static.SendMessage(WM_SETFONT, (WPARAM) m_font.m_hFont);

// +++ STEP 4 +++
		// Add tabs: (Look how easy)
		AddTab(IDS_TAB1);
		AddSubTab(IDS_TAB1A, IDI_ICON1);
		AddSubTab(IDS_TAB1B, IDI_ICON1);
		AddSubTab(IDS_TAB1C, IDI_ICON1);
		AddSubTab(IDS_TAB1D, IDI_ICON1);
		AddTab(IDS_TAB2);
		AddSubTab(IDS_TAB2A, IDI_ICON2);
		AddSubTab(IDS_TAB2B, IDI_ICON2);
		AddSubTab(IDS_TAB2C, IDI_ICON2);
		AddSubTab(IDS_TAB2D, IDI_ICON2);
		AddTab(IDS_TAB3);
		AddSubTab(IDS_TAB3A, IDI_ICON2);
		AddSubTab(IDS_TAB3B, IDI_ICON2);
		AddSubTab(IDS_TAB3C, IDI_ICON2);
		AddSubTab(IDS_TAB3D, IDI_ICON2);
		AddTab(IDS_TAB4);
		AddSubTab(IDS_TAB4A, IDI_ICON2);
		AddSubTab(IDS_TAB4B, IDI_ICON2);
		AddSubTab(IDS_TAB4C, IDI_ICON2);
		AddTab(IDS_TAB5);
		AddSubTab(IDS_TAB5A, IDI_ICON2);
		AddSubTab(IDS_TAB5B, IDI_ICON2);
		AddSubTab(IDS_TAB5C, IDI_ICON2);
		AddTab(IDS_TAB6);
		AddSubTab(IDS_TAB6A, IDI_ICON1);
		AddSubTab(IDS_TAB6B, IDI_ICON1);
		AddSubTab(IDS_TAB6C, IDI_ICON1);

		return TRUE;
	}

	void OnCommand(UINT uNotifyCode, int nID, CWindow /*wndCtl*/)
	{
		SetMsgHandled(FALSE);

		if ( (uNotifyCode == 0 || uNotifyCode == 1) && (nID >= IDS_TAB1 && nID <= IDS_TAB6C) )
		{
			CString str;
			str.LoadString(nID);

			// Strip out the ampersands
			str.Replace(_T("&"), _T(""));

			m_static.SetWindowText(str);
		}
	}

	LRESULT OnTab1ASelected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString str(_T("Get Started"));
		m_static.SetWindowText(str);
		return 0;
	}

	LRESULT OnTab1DSelected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString str(_T("About..."));
		m_static.SetWindowText(str);

		CAboutDlg dlg;
		dlg.DoModal();

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// Unregister idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// ToDo: Add validation code
		// CloseDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}
};
