//
// PageProfileCertificate.h
//
// This file is part of Envy (getenvy.com) � 2016-2018
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


class CCertificateProfilePage : public CSettingsPage
{
	DECLARE_DYNCREATE(CCertificateProfilePage)

public:
	CCertificateProfilePage();
	virtual ~CCertificateProfilePage();

	enum { IDD = IDD_PROFILE_CERTIFICATE };

public:
	CString	m_sGUID;
	CString	m_sGUIDBT;
//	CString	m_sTime;

public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

protected:
	afx_msg void OnGuidCreate();

	DECLARE_MESSAGE_MAP()
};
