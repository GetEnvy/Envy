//
// DlgUpdateServers.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2007
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

// Fetch .met and other hostcache lists from the web
// Was DlgDonkeyServers.h, CDonkeyServersDlg, IDD_DONKEY_SERVERS

#pragma once

#include "HttpRequest.h"
#include "DlgSkinDialog.h"


class CUpdateServersDlg : public CSkinDialog
{
public:
	CUpdateServersDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_UPDATE_SERVERS };

public:
	CString			m_sURL;

protected:
	CEdit			m_wndURL;
	CButton			m_wndOK;
	CProgressCtrl	m_wndProgress;
	CHttpRequest	m_pRequest;

	BOOL			IsValidURL();

	virtual void	DoDataExchange(CDataExchange* pDX);
	virtual BOOL	OnInitDialog();
	virtual void	OnOK();
	virtual void	OnCancel();

	afx_msg void	OnChangeURL();
	afx_msg void	OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
