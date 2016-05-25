//
// PageComment.h
//
// This file is part of Torrent Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008,2012-2014 and Shareaza 2007
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


class CCommentPage : public CWizardPage
{
	DECLARE_DYNCREATE(CCommentPage)

// Construction
public:
	CCommentPage();
	//virtual ~CCommentPage();

	enum { IDD = IDD_COMMENT_PAGE };

// Dialog Data
public:
	CString	m_sComment;
//	CString	m_sSource;
	BOOL	m_bPrivate;

// Operations
protected:
	void SaveComments();

// Overrides
protected:
	virtual BOOL OnInitDialog();
	virtual void OnReset();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();

	virtual void DoDataExchange(CDataExchange* pDX);

// Implementation
protected:
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};
