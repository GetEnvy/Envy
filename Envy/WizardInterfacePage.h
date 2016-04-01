//
// WizardInterfacePage.h
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


class CWizardInterfacePage : public CWizardPage
{
	DECLARE_DYNCREATE(CWizardInterfacePage)

public:
	CWizardInterfacePage();
	virtual ~CWizardInterfacePage();

	enum { IDD = IDD_WIZARD_INTERFACE };

public:
	CStatic	m_wndDescriptionExpert;
	CStatic	m_wndDescriptionBasic;
	CButton	m_wndInterfaceExpert;
	CButton	m_wndInterfaceBasic;
	CButton	m_wndSkinNoChange;
	CButton	m_wndSkinDefault;
	CButton	m_wndSkinDark;
	BOOL	m_bSimpleDownloadBars;
	int		m_bBasic;
	int		m_nSkin;

	void	ClearSkins(LPCTSTR pszPath = NULL);

public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};
