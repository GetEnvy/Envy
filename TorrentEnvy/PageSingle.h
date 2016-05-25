//
// PageSingle.h
//
// This file is part of Torrent Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008,2012 and Shareaza 2007
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "WizardSheet.h"


class CSinglePage : public CWizardPage
{
	DECLARE_DYNCREATE(CSinglePage)

// Construction
public:
	CSinglePage();
	//virtual ~CSinglePage();

	enum { IDD = IDD_SINGLE_PAGE };

// Dialog Data
public:
	//{{AFX_DATA(CSinglePage)
	CString	m_sFileName;
	CString	m_sFileSize;
	//CString	m_sMagnet;
	//}}AFX_DATA

protected:
	void Update();

// Overrides
public:
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSinglePage)
	virtual void OnReset();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSinglePage)
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBrowseFile();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
