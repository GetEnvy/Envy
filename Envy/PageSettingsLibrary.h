//
// PageSettingsLibrary.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2006
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
#include "CtrlIconButton.h"


class CLibrarySettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CLibrarySettingsPage)

public:
	CLibrarySettingsPage();
	virtual ~CLibrarySettingsPage();

	enum { IDD = IDD_SETTINGS_LIBRARY };

	virtual void OnOK();

protected:
	CSpinButtonCtrl	m_wndRecentTotal;
	CSpinButtonCtrl	m_wndRecentDays;
	CButton			m_wndSafeRemove;
	CButton			m_wndSafeAdd;
	CComboBox		m_wndSafeList;
	CButton			m_wndPrivateRemove;
	CButton			m_wndPrivateAdd;
	CComboBox		m_wndPrivateList;
	BOOL			m_bWatchFolders;
	DWORD			m_nRecentDays;
	int 			m_nRecentTotal;
	BOOL			m_bBrowseFiles;
	BOOL			m_bHashWindow;
	BOOL			m_bHighPriorityHash;
	BOOL			m_bMakeGhosts;
	BOOL			m_bSmartSeries;
	CString			m_sCollectionPath;
	CIconButtonCtrl	m_wndCollectionPath;
	CEditPath		m_wndCollectionFolder;
	CButton			m_wndRecentClear;
	CButton			m_wndRecentClearGhosts;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnSelChangeSafeTypes();
	afx_msg void OnEditChangeSafeTypes();
	afx_msg void OnSafeAdd();
	afx_msg void OnSafeRemove();
	afx_msg void OnSelChangePrivateTypes();
	afx_msg void OnEditChangePrivateTypes();
	afx_msg void OnPrivateAdd();
	afx_msg void OnPrivateRemove();
	afx_msg void OnRecentClear();
	afx_msg void OnRecentClearGhosts();
	afx_msg void OnCollectionsBrowse();

	DECLARE_MESSAGE_MAP()
};
