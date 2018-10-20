//
// PageSettingsMedia.h
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


class CMediaSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CMediaSettingsPage)

public:
	CMediaSettingsPage();
	virtual ~CMediaSettingsPage();

	enum { IDD = IDD_SETTINGS_MEDIA };

public:
	CButton		m_wndRemove;
	CButton		m_wndAdd;
	CComboBox	m_wndList;
	CComboBox	m_wndServices;
	CString		m_sType;
	BOOL		m_bEnablePlay;
	BOOL		m_bEnableEnqueue;

protected:
	void Update();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnDestroy();
	afx_msg void OnMediaPlay();
	afx_msg void OnMediaEnqueue();
	afx_msg void OnMediaAdd();
	afx_msg void OnMediaRemove();
	afx_msg void OnMediaVis();
	afx_msg void OnEditChangeMediaTypes();
	afx_msg void OnSelChangeMediaTypes();
	afx_msg void OnSelChangeMediaService();

	DECLARE_MESSAGE_MAP()
};
