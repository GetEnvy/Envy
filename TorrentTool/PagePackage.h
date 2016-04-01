//
// PagePackage.h
//
// This file is part of Envy Torrent Tool (getenvy.com) © 2016
// Portions copyright PeerProject 2008,2012 and Shareaza 2007
//
// Envy Torrent Tool is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Torrent Tool is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "WizardSheet.h"

#define LVS_EX_LABELTIP			0x00004000
#define LVS_EX_DOUBLEBUFFER 	0x00010000

class CPackagePage : public CWizardPage
{
	DECLARE_DYNCREATE(CPackagePage)

// Construction
public:
	CPackagePage();
	//virtual ~CPackagePage();

	enum { IDD = IDD_PACKAGE_PAGE };

// Dialog Data
public:
	//{{AFX_DATA(CPackagePage)
	QWORD 		m_nTotalSize;
	CString 	m_sTotalSize;
	CString 	m_sFileCount;
	CButton 	m_wndRemove;
	CListCtrl	m_wndList;
	//}}AFX_DATA

	HIMAGELIST	m_hImageList;

// Operations
protected:
	void	AddFile(LPCTSTR pszFile);
	void	AddFolder(LPCTSTR pszPath, int nRecursive);

// Overrides
public:
	//{{AFX_VIRTUAL(CPackagePage)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual void OnReset();
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CPackagePage)
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
	afx_msg void OnItemChangedFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddFolder();
	afx_msg void OnAddFile();
	afx_msg void OnRemoveFile();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
