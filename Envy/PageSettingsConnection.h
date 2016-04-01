//
// PageSettingsConnection.h
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


class CConnectionSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CConnectionSettingsPage)

public:
	CConnectionSettingsPage();
	virtual ~CConnectionSettingsPage();

	enum { IDD = IDD_SETTINGS_CONNECTION };

public:
	CEdit		m_wndInPort;
	CComboBox	m_wndInSpeed;
	CComboBox	m_wndOutSpeed;
	CComboBox	m_wndInHost;
	CComboBox	m_wndOutHost;
	CButton		m_wndInBind;
	CSpinButtonCtrl	m_wndTimeoutHandshake;
	CSpinButtonCtrl	m_wndTimeoutConnection;
	BOOL		m_bInBind;
	int			m_nInPort;
	CString		m_sInHost;
	CString		m_sOutHost;
	CComboBox	m_wndCanAccept;
	BOOL		m_bIgnoreLocalIP;
	BOOL		m_bEnableUPnP;
	DWORD		m_nTimeoutConnection;
	DWORD		m_nTimeoutHandshake;
	CString		m_sOutSpeed;
	CString		m_sInSpeed;
	BOOL		m_bInRandom;

public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CString GetInOutHostTranslation();

	virtual BOOL OnInitDialog();
	afx_msg void OnEditChangeInboundHost();
	afx_msg void OnChangedInboundHost();
	afx_msg void OnChangeInboundPort();
	afx_msg void OnInboundRandom();
	afx_msg void OnClickedEnableUpnp();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	DECLARE_MESSAGE_MAP()
};
