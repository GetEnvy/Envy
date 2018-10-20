//
// PageSettingsGeneral.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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

#include "WndSettingsPage.h"


class CGeneralSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CGeneralSettingsPage)

public:
	CGeneralSettingsPage();
	virtual ~CGeneralSettingsPage();

	enum { IDD = IDD_SETTINGS_GENERAL };

public:
	BOOL	m_bAutoConnect;
	BOOL	m_bStartup;
	BOOL	m_bPromptURLs;
	BOOL	m_bUpdateCheck;
	BOOL	m_bExpandDownloads;
	BOOL	m_bSimpleBar;
	BOOL	m_bExpandMatches;
	BOOL	m_bSwitchToTransfers;
	BOOL	m_bHideSearch;
	BOOL	m_bAdultFilter;
	BOOL	m_bTipShadow;
	int		m_nCloseMode;
	int		m_bTrayMinimise;
	int		m_nRatesInBytes;
	DWORD	m_nTipDelay;
	CSpinButtonCtrl	m_wndTipSpin;
	CListCtrl	m_wndTips;
	CSliderCtrl	m_wndTipAlpha;
	CComboBox	m_wndCloseMode;
	CComboBox	m_wndTrayMinimise;


protected:
	void Add(LPCTSTR pszName, BOOL bState);

public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnDropdownCloseMode();
	afx_msg void OnDropdownTrayMinimise();

	DECLARE_MESSAGE_MAP()
};
