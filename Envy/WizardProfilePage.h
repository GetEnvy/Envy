//
// WizardProfilePage.h
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

#include "WizardSheet.h"

class CWorldGPS;


class CWizardProfilePage : public CWizardPage
{
	DECLARE_DYNCREATE(CWizardProfilePage)

public:
	CWizardProfilePage();
	virtual ~CWizardProfilePage();

	enum { IDD = IDD_WIZARD_PROFILE };

public:
	CString 	m_sNick;
	CWorldGPS*	m_pWorld;
	CComboBox	m_wndAge;
	CComboBox	m_wndCity;
	CComboBoxEx	m_wndCountry;
	CImageList	m_gdiFlags;
	CString		m_sLocCity;
	CString		m_sLocCountry;
	int 		m_nAge;
	int			m_nGender;
	CEdit		m_wndComments;

public:
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg void OnSelChangeCountry();
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};
