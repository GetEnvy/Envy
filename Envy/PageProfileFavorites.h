//
// PageProfileFavorites.h
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


class CFavoritesProfilePage : public CSettingsPage
{
	DECLARE_DYNCREATE(CFavoritesProfilePage)

public:
	CFavoritesProfilePage();
	virtual ~CFavoritesProfilePage();

	enum { IDD = IDD_PROFILE_FAVORITES };

public:
	CButton		m_wndRemove;
	CButton		m_wndAdd;
	CString		m_sURL;
	CString		m_sTitle;
	CListCtrl	m_wndList;
	CImageList	m_gdiImageList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

	virtual BOOL OnInitDialog();
	afx_msg void OnWebAdd();
	afx_msg void OnWebRemove();
	afx_msg void OnChangeWebName();
	afx_msg void OnChangeWebUrl();
	afx_msg void OnItemChangedWebList(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
