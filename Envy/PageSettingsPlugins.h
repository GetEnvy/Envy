//
// PageSettingsPlugins.h
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


class CPluginsSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CPluginsSettingsPage)

public:
	CPluginsSettingsPage();
	virtual ~CPluginsSettingsPage();

	enum { IDD = IDD_SETTINGS_PLUGINS };

public:
	CButton	m_wndSetup;
	CEdit	m_wndDesc;
	CStatic	m_wndName;
	CListCtrl	m_wndList;

protected:
	CImageList	m_gdiImageList;
	BOOL		m_bRunning;
protected:
	void		InsertPlugin(LPCTSTR pszCLSID, LPCTSTR pszName, int nImage, TRISTATE bEnabled,
							 LPVOID pPlugin = NULL, LPCTSTR pszExtension = NULL);
	void		EnumerateGenericPlugins();
	void		EnumerateMiscPlugins();
	void		EnumerateMiscPlugins(LPCTSTR pszType, HKEY hRoot);
	void		AddMiscPlugin(LPCTSTR pszType, LPCTSTR pszCLSID, LPCTSTR pszExtension = NULL);
	CString		GetPluginComments(LPCTSTR pszCLSID) const;

public:
	void		UpdateList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnItemChangingPlugins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemChangedPlugins(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkPlugins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCustomDrawPlugins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPluginsSetup();
	afx_msg void OnPluginsWeb();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
