//
// PageSettingsSkins.h
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

#include "WndSettingsPage.h"


class CSkinsSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CSkinsSettingsPage)

public:
	CSkinsSettingsPage();
	virtual ~CSkinsSettingsPage();

	enum { IDD = IDD_SETTINGS_SKINS };

public:
	CListCtrl	m_wndList;
	CButton 	m_wndDelete;
	CStatic 	m_wndName;
	CStatic 	m_wndAuthor;
	CEdit		m_wndDesc;

	CImageList	m_gdiImageList;
	int			m_nSelected;

public:
	void	EnumerateSkins(LPCTSTR pszPath = NULL);
	BOOL	AddSkin(LPCTSTR pszPath, LPCTSTR pszName);
	void	CheckDependencies(CString sPaths);

public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedSkins(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSkinsBrowse();
	afx_msg void OnSkinsWeb();
	afx_msg void OnSkinsDelete();

	DECLARE_MESSAGE_MAP()
};
