//
// PageProfileContact.h
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


class CContactProfilePage : public CSettingsPage
{
	DECLARE_DYNCREATE(CContactProfilePage)

public:
	CContactProfilePage();
	virtual ~CContactProfilePage();

	enum { IDD = IDD_PROFILE_CONTACT };

public:
	CString	m_sEmail;
	CString	m_sSkype;
	CString	m_sYahoo;
	CString	m_sAOL;
	CString	m_sICQ;
	CString m_sJabber;
	CString m_sTwitter;
	CString m_sFacebook;
	CString m_sGetEnvy;

	void	AddAddress(LPCTSTR pszClass, LPCTSTR pszName, LPCTSTR pszAddress);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

protected:
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
};
