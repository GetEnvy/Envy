//
// DlgSecureRule.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

#include "DlgSkinDialog.h"

class CSecureRule;


class CSecureRuleDlg : public CSkinDialog
{
public:
	CSecureRuleDlg(CWnd* pParent = NULL, CSecureRule* pRule = NULL);
	virtual ~CSecureRuleDlg();

	enum { IDD = IDD_SECURE_RULE };

public:
	CButton	m_wndGroupNetwork;
	CButton	m_wndGroupContent;
	CButton	m_wndGroupExternal;
	CButton	m_wndBrowse;
	CEdit	m_wndPath;
	CEdit	m_wndContent;
	CEdit	m_wndExpireM;
	CEdit	m_wndExpireH;
	CEdit	m_wndExpireD;
	CEdit	m_wndMask4;
	CEdit	m_wndMask3;
	CEdit	m_wndMask2;
	CEdit	m_wndMask1;
	CEdit	m_wndIP4;
	CEdit	m_wndIP3;
	CEdit	m_wndIP2;
	CEdit	m_wndIP1;
	int		m_nExpireD;
	int		m_nExpireH;
	int		m_nExpireM;
	int		m_nAction;
	int		m_nExpire;
	int		m_nType;	// Dropdown Box Select
	int		m_nMatch;	// Radio Button Select
	CString	m_sComment;
	CString	m_sContent;
	CString	m_sPath;

	CSecureRule*	m_pRule;
	CToolTipCtrl	m_ToolTip;	// Netmask hint
	BOOL			m_bNew;

	BOOL	GetClipboardAddress();
	void	ShowGroup(CWnd* pWnd, BOOL bShow);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeRuleExpire();
	afx_msg void OnSelChangeRuleType();
	afx_msg void OnBrowse();

	DECLARE_MESSAGE_MAP()
};
