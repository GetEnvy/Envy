//
// PageSettingsRemote.h
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


class CRemoteSettingsPage : public CSettingsPage
{
	DECLARE_DYNAMIC(CRemoteSettingsPage)

public:
	CRemoteSettingsPage();
	virtual ~CRemoteSettingsPage();

	enum { IDD = IDD_SETTINGS_REMOTE };

protected:
	BOOL		m_bEnable;
	CStatic		m_wndURL;
	CStatic		m_wndQRCode;
	CEdit		m_wndUsername;
	CEdit		m_wndPassword;
	CString		m_sUsername;
	CString		m_sPassword;

	BOOL		m_bOldEnable;
	CString		m_sOldUsername;
	CString		m_sOldPassword;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnSkinChange();
	virtual void OnCancel();

	afx_msg void OnNewPassword();
	afx_msg void OnBnClickedRemoteEnable();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};
