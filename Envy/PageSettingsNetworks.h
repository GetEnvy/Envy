//
// PageSettingsNetworks.h
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


class CNetworksSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CNetworksSettingsPage)

public:
	CNetworksSettingsPage();
	virtual ~CNetworksSettingsPage();

	enum { IDD = IDD_SETTINGS_NETWORKS };

public:
	CStatic	m_wndG2Setup;
	CStatic	m_wndG1Setup;
	CStatic	m_wndEDSetup;
	CStatic	m_wndDCSetup;
	BOOL	m_bG2Enable;
	BOOL	m_bG1Enable;
	BOOL	m_bEDEnable;
	BOOL	m_bDCEnable;

	void	Update();

public:
	virtual BOOL OnSetActive();
	virtual BOOL OnApply();
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnG2Enable();
	afx_msg void OnG1Enable();
	afx_msg void OnEd2kEnable();
	afx_msg void OnDCEnable();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
