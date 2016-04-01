//
// PageSettingsWeb.h
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

class CWebSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CWebSettingsPage)

public:
	CWebSettingsPage();
	virtual ~CWebSettingsPage();

	enum { IDD = IDD_SETTINGS_WEB };

public:
	CButton	m_wndExtRemove;
	CButton	m_wndExtAdd;
	CComboBox	m_wndExtensions;
	BOOL	m_bWebHook;
	BOOL	m_bUriMagnet;
	BOOL	m_bUriGnutella;
	BOOL	m_bUriED2K;
	BOOL	m_bUriDC;
	BOOL	m_bUriPiolet;
	BOOL	m_bUriTorrent;
//	BOOL	m_bUriMetalink;

public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnEditChangeExtList();
	afx_msg void OnSelChangeExtList();
	afx_msg void OnExtAdd();
	afx_msg void OnExtRemove();
	afx_msg void OnWebHook();

	DECLARE_MESSAGE_MAP()
};
