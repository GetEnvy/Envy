//
// DlgMessage.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2009 and PeerProject 2009-2012
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

class CSkinDialog;


// CMessageDlg dialog

class CMessageDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CMessageDlg)

public:
	CMessageDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_MESSAGE };

public:
	DWORD	m_nType;		// Message box type
	CString	m_sText;		// Message box text
	DWORD*	m_pnDefault;	// Message box variable
	DWORD	m_nIDHelp;		// The Help context ID for the message (0 - default)
	DWORD	m_nTimer;		// Time to auto-press default button (0 - disabled)

	virtual INT_PTR DoModal();

protected:
	CStatic m_Icon;
	CStatic m_pText;
	CStatic m_pSplit;
	CButton m_pDefault;
	CButton m_pButton1;
	CButton m_pButton2;
	CButton m_pButton3;
	BOOL	m_bRemember;	// Remember selection next time
	int		m_nDefButton;	// Default button number (1, 2 or 3)

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void UpdateTimer();
	void StopTimer();

	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
