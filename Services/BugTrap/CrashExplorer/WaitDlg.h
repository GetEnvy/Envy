/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Wait dialog.
 * Author: Maksim Pyatkovskiy.
 */

#pragma once

/// Please wait dialog.
class CWaitDialog : public CDialogImpl<CWaitDialog>, public CWaitCursor
{
public:
	/// Dialog resource identifier.
	enum { IDD = IDD_WAIT };

	BEGIN_MSG_MAP_EX(CWaitDialog)
	END_MSG_MAP()

	/**
	* @brief Show wait dialog.
	* @param hWndParent - parent window handle.
	*/
	CWaitDialog(HWND hWndParent = NULL)
	{
		Create(hWndParent);
		CenterWindow(hWndParent);
		ShowWindow(SW_SHOW);
		UpdateWindow();
	}

	/// Destroy the object.
	~CWaitDialog()
	{
		if (m_hWnd)
			DestroyWindow();
	}
};
