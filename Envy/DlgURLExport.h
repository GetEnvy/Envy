//
// DlgURLExport.h
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

#include "DlgSkinDialog.h"
#include "EnvyFile.h"

class CURLExportDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CURLExportDlg)

public:
	CURLExportDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_URL_EXPORT };

protected:
	CProgressCtrl	m_wndProgress;
	CButton			m_wndSave;
	CButton			m_wndCopy;
	CComboBox		m_wndToken;
	CComboBox		m_wndPreset;
	CEdit			m_wndFormat;
	CStatic			m_wndMessage;
	CString			m_sFormat;

	CList< CEnvyFile >	m_pFiles;

public:
	void		Add(const CEnvyFile* pFile);
	static void	MakeURL(CEnvyFile pFile, CString& strLine);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnCloseUpUrlToken();
	afx_msg void OnSelChangeUrlPreset();
	afx_msg void OnKillFocusUrlPreset();
	afx_msg void OnSave();
	afx_msg void OnCopy();

	DECLARE_MESSAGE_MAP()
};
