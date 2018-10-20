//
// PageProfileAvatar.h
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


class CAvatarProfilePage : public CSettingsPage
{
	DECLARE_DYNCREATE(CAvatarProfilePage)

public:
	CAvatarProfilePage();
	virtual ~CAvatarProfilePage();

	enum { IDD = IDD_PROFILE_AVATAR };

public:
	CButton	m_wndRemove;
	CStatic	m_wndPreview;

protected:
	CString		m_sAvatar;
	CBitmap		m_bmAvatar;

protected:
	void		PrepareImage();

public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnAvatarBrowse();
	afx_msg void OnAvatarRemove();

	DECLARE_MESSAGE_MAP()
};
