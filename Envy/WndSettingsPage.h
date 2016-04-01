//
// WndSettingsPage.h
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

class CSettingsSheet;


class CSettingsPage : public CDialog
{
	DECLARE_DYNAMIC(CSettingsPage)

public:
	CSettingsPage(UINT nIDTemplate, LPCTSTR pszName = NULL);
	virtual ~CSettingsPage();

public:
	CToolTipCtrl	m_wndToolTip;
	CString			m_sName;		// Dialog name used for skinning
	CString			m_sCaption;		// Dialog caption
	BOOL			m_bGroup;

	BOOL			Create(const CRect& rcPage, CWnd* pSheetWnd);
	BOOL			LoadDefaultCaption();
	CSettingsPage*	GetPage(CRuntimeClass* pClass) const;

	inline CSettingsSheet* GetSheet() const
	{
		return (CSettingsSheet*)GetParent();
	}

	inline LPCTSTR GetTemplateName() const
	{
		return m_lpszTemplateName;
	}

// Events
public:
	virtual void SetModified(BOOL bChanged = TRUE);
	virtual BOOL OnApply();
	virtual void OnReset();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void OnSkinChange();

protected:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
//	afx_msg LRESULT OnCtlColorStatic(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

// CEditPath
// Same functionality as CEdit has but with ability to
// run specified file or folder on mouse double click.

class CEditPath : public CEdit
{
	DECLARE_DYNAMIC(CEditPath)

protected:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
