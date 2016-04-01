//
// PageSettingsCommunity.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2007
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


class CCommunitySettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CCommunitySettingsPage)

public:
	CCommunitySettingsPage();
	virtual ~CCommunitySettingsPage();

	enum { IDD = IDD_SETTINGS_COMMUNITY };

public:
	BOOL	m_bProfileEnable;
	BOOL	m_bChatEnable;
	BOOL	m_bChatAllNetworks;
	BOOL	m_bChatFilter;
	BOOL	m_bChatCensor;

public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnEditProfile();

	DECLARE_MESSAGE_MAP()
};
