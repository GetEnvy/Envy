//
// DlgFolderScan.h
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


class CFolderScanDlg : public CSkinDialog
{
public:
	CFolderScanDlg(CWnd* pParent = NULL);
	virtual ~CFolderScanDlg();

	enum { IDD = IDD_FOLDER_SCAN };

public:
	CStatic	m_wndVolume;
	CStatic	m_wndFiles;
	CStatic	m_wndFile;

public:
	static void Update(LPCTSTR pszName, DWORD nVolume);	//, BOOL bLock);
	void	InstanceUpdate(LPCTSTR pszName, DWORD nVolume);

protected:
	static	CFolderScanDlg*	m_pDialog;
	DWORD	m_nCookie;
	DWORD	m_nFiles;
	DWORD	m_nVolume;
	DWORD	m_tLastUpdate;
	BOOL	m_bActive;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
