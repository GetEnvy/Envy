//
// PageSettingsRich.h
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
#include "RichViewCtrl.h"


class CRichSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CRichSettingsPage)

public:
	CRichSettingsPage(LPCTSTR pszName);
	virtual ~CRichSettingsPage();

	enum { IDD = IDD_SETTINGS_RICH };

protected:
	CRichViewCtrl	m_wndView;
	CRichDocument*	m_pDocument;

public:
	virtual void OnSkinChange();
protected:
	virtual BOOL OnInitDialog();

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnClickView(NMHDR* pNotify, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};

#define IDC_RICH_VIEW	100
