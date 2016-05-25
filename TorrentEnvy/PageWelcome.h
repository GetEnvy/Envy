//
// PageWelcome.h
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

class CWelcomePage : public CWizardPage
{
	DECLARE_DYNCREATE(CWelcomePage)

// Construction
public:
	CWelcomePage();
	//virtual ~CWelcomePage();

	enum { IDD = IDD_WELCOME_PAGE };

// Dialog Data
public:
	//{{AFX_DATA(CWelcomePage)
	int		m_nType;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CWelcomePage)
	virtual void OnReset();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CWelcomePage)
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
