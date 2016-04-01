//
// PageFileSharing.h
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

#include "DlgFilePropertiesPage.h"


class CFileSharingPage : public CFilePropertiesPage
{
	DECLARE_DYNCREATE(CFileSharingPage)

public:
	CFileSharingPage();
	virtual ~CFileSharingPage();

	enum { IDD = IDD_FILE_SHARING };

public:
	//CListCtrl	m_wndNetworks;
	CComboBox	m_wndTags;
	CButton	m_wndShare;
	int		m_bOverride;
	BOOL	m_bShare;
	CString	m_sTags;

public:
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnShareOverride0();
	afx_msg void OnShareOverride1();

	DECLARE_MESSAGE_MAP()
};
