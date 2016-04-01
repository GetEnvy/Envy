//
// PageSettingsDownloads.h
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
#include "CtrlIconButton.h"


class CDownloadsSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CDownloadsSettingsPage)

public:
	CDownloadsSettingsPage();
//	virtual ~CDownloadsSettingsPage();

	enum { IDD = IDD_SETTINGS_DOWNLOADS };

protected:
	CSpinButtonCtrl	m_wndMaxDownTransfers;
	CSpinButtonCtrl	m_wndMaxFileTransfers;
	CSpinButtonCtrl	m_wndMaxDownFiles;
	CIconButtonCtrl	m_wndIncompletePath;
	CIconButtonCtrl	m_wndDownloadsPath;
	CComboBox		m_wndBandwidthLimit;
	CComboBox		m_wndQueueLimit;
	CComboBox		m_wndAntiVirus;
	CString			m_sDownloadsPath;
	CString			m_sIncompletePath;
	int				m_nMaxDownFiles;
	int				m_nMaxFileTransfers;
	int				m_nMaxDownTransfers;
	CString			m_sBandwidthLimit;
	CString			m_sQueueLimit;
	BOOL			m_bRequireConnect;
	BOOL			m_bDownloadsChanged;
	CEditPath		m_wndDownloadsFolder;
	CEditPath		m_wndIncompleteFolder;

	bool			IsLimited(CString& strText) const;

public:
	virtual void OnOK();
	virtual BOOL OnKillActive();

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnDownloadsBrowse();
	afx_msg void OnIncompleteBrowse();
	afx_msg void OnCbnDropdownAntivirus();

	DECLARE_MESSAGE_MAP()
};
