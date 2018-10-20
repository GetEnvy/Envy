//
// PageTracker.h
//
// This file is part of Torrent Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2008,2012-2014 and Shareaza 2007
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "WizardSheet.h"


class CTrackerPage : public CWizardPage
{
	DECLARE_DYNCREATE(CTrackerPage)

// Construction
public:
	CTrackerPage();
	//virtual ~CTrackerPage();

	enum { IDD = IDD_TRACKER_PAGE };

// Dialog Data
public:
	CComboBox	m_wndTracker;
	CComboBox	m_wndTracker2;
	CString 	m_sTracker;
	CString 	m_sTracker2;

protected:
	void	SaveTrackers();

// Overrides
protected:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual void DoDataExchange(CDataExchange* pDX);

// Implementation
protected:
	afx_msg void OnClearTrackers();
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};
