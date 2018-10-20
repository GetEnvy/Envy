//
// DlgDonkeyImport.h
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
#include "EDPartImporter.h"


class CDonkeyImportDlg : public CSkinDialog
{
public:
	CDonkeyImportDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_DONKEY_IMPORT };

public:
	CButton	m_wndClose;
	CButton	m_wndCancel;
	CButton	m_wndImport;
	CEdit	m_wndLog;

	CString	m_sCancel;
	CEDPartImporter	m_pImporter;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnImport();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
