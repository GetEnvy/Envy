//
// DlgHelp.h
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
#include "RichViewCtrl.h"
#include "RichDocument.h"


class CHelpDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CHelpDlg)

public:
	CHelpDlg(LPCTSTR pszName, CWnd* pParent = NULL);

	enum { IDD = IDD_HELP };

	static BOOL Show(LPCTSTR pszName, CWnd* pParent = NULL);

protected:
	CRichViewCtrl	m_wndView;
	CRichDocument	m_pDocument;
	CString			m_sDocument;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

protected:
	afx_msg void OnClickView(NMHDR* pNotify, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};

#define IDC_HELP_VIEW	100
