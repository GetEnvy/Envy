//
// PageSettingsGnutella.h
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


class CGnutellaSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CGnutellaSettingsPage)

public:
	CGnutellaSettingsPage();
	virtual ~CGnutellaSettingsPage();

	enum { IDD = IDD_SETTINGS_GNUTELLA };

public:
	CSpinButtonCtrl	m_wndG2Peers;
	CSpinButtonCtrl	m_wndG2Leafs;
	CSpinButtonCtrl	m_wndG2Hubs;
	CSpinButtonCtrl	m_wndG1Peers;
	CSpinButtonCtrl	m_wndG1Leafs;
	CSpinButtonCtrl	m_wndG1Hubs;
	BOOL	m_bG2Today;
	BOOL	m_bG2Always;
	BOOL	m_bG1Today;
	BOOL	m_bG1Always;
	CComboBox m_wndG2ClientMode;
	CComboBox m_wndG1ClientMode;
	int		m_nG2Hubs;
	int		m_nG2Leafs;
	int		m_nG2Peers;
	int		m_nG1Hubs;
	int		m_nG1Leafs;
	int		m_nG1Peers;
	BOOL	m_bDeflateHub2Hub;
	BOOL	m_bDeflateLeaf2Hub;
	BOOL	m_bDeflateHub2Leaf;
	BOOL	m_bAgent;

public:
	virtual BOOL OnSetActive();
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnG2Today();
	afx_msg void OnG1Today();
	afx_msg void OnG2Always();

	DECLARE_MESSAGE_MAP()
};
