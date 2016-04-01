//
// KeywordTestDlg.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2012 and Shareaza 2011
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

class CKeywordTestDlg : public CDialog
{
public:
	CKeywordTestDlg(CWnd* pParent = NULL);	// Standard constructor
	enum { IDD = IDD_KEYWORDTEST_DIALOG };

protected:
	CString m_sInput;
	CString m_sSplitted;
	CListBox m_pResults;
	BOOL m_bExp;
	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEnChangeInput();
	afx_msg void OnBnClickedExp();

	DECLARE_MESSAGE_MAP()
};
