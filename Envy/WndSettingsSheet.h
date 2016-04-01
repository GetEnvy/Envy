//
// WndSettingsSheet.h
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
#include "CtrlIconButton.h"

class CSettingsPage;


class CSettingsSheet : public CSkinDialog
{
	DECLARE_DYNAMIC(CSettingsSheet)

public:
	CSettingsSheet(CWnd* pParent = NULL, UINT nCaptionID = 0);
	virtual ~CSettingsSheet();

protected:
	CArray< CSettingsPage* > m_pPages;
	CSettingsPage*	m_pPage;
	CSettingsPage*	m_pFirst;
	CSize			m_szPages;

	CTreeCtrl		m_wndTree;
	CButton			m_wndOK;
	CButton			m_wndCancel;
	CButton			m_wndApply;
	BOOL			m_bModified;

	CString			m_sCaption;
	int				m_nButtonHeight;

public:
	virtual BOOL	SkinMe(LPCTSTR pszSkin = NULL, UINT nIcon = 0, BOOL bLanguage = TRUE);

	void			AddPage(CSettingsPage* pPage, LPCTSTR pszCaption = NULL);
	void			AddGroup(CSettingsPage* pPage, LPCTSTR pszCaption = NULL);
	CSettingsPage*	GetPage(INT_PTR nPage) const;
	CSettingsPage*	GetPage(CRuntimeClass* pClass) const;
	CSettingsPage*	GetPage(LPCTSTR pszClass) const;
	INT_PTR			GetPageIndex(CSettingsPage* pPage) const;
	INT_PTR			GetPageCount() const;
	CSettingsPage*	GetActivePage() const;
	BOOL			SetActivePage(CSettingsPage* pPage);
//	BOOL			SetActivePage(int nPage);
	BOOL			IsModified() const;
	void			SetModified(BOOL bChanged = TRUE);
	INT_PTR			DoModal();
protected:
	void			BuildTree();
	BOOL			CreatePage(const CRect& rc, CSettingsPage* pPage);

protected:
	virtual void	DoPaint(CDC& dc);
	virtual void	DoDataExchange(CDataExchange* pDX);
	virtual BOOL	OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL	OnInitDialog();
	virtual void	OnOK();
	virtual void	OnCancel();
	afx_msg void	OnApply();
	afx_msg void	OnPaint();
	afx_msg void	OnSelectPage(NMHDR* pNotify, LRESULT *pResult);
	//afx_msg void	OnTreeExpanding(NMHDR* pNotify, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};

#define IDC_SETTINGS_TREE	100
