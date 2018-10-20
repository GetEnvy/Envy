//
// DlgUpgrade.h
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

#include "DlgSkinDialog.h"


class CUpgradeDlg : public CSkinDialog
{
public:
	CUpgradeDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_UPGRADE };

public:
	BOOL	m_bCheck;
	CString	m_sMessage;

	void ParseCheckAgain();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

	//DECLARE_MESSAGE_MAP()
};
