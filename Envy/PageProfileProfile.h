//
// PageProfileProfile.h
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

#include "WndSettingsPage.h"

class CWorldGPS;


class CProfileProfilePage : public CSettingsPage
{
	DECLARE_DYNCREATE(CProfileProfilePage)

public:
	CProfileProfilePage();
	virtual ~CProfileProfilePage();

	enum { IDD = IDD_PROFILE_PROFILE };

public:
	CButton 	m_wndInterestAdd;
	CButton 	m_wndInterestRemove;
	CListBox	m_wndInterestList;
	CComboBox	m_wndInterestAll;
	CComboBox	m_wndAge;
	CComboBox	m_wndCity;
	CComboBoxEx	m_wndCountry;
	CImageList	m_gdiFlags;
	CString 	m_sLocCity;
	CString 	m_sLocCountry;
	CString 	m_sLocLatitude;
	CString 	m_sLocLongitude;
	CString 	m_sAge;
	CString 	m_sGender;

	CWorldGPS*	m_pWorld;

private:
	int 		LoadDefaultInterests();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();
	virtual BOOL OnInitDialog();

protected:
	afx_msg void OnSelChangeCountry();
	afx_msg void OnSelChangeCity();
	afx_msg void OnSelChangeInterestList();
	afx_msg void OnSelChangeInterestAll();
	afx_msg void OnEditChangeInterestAll();
	afx_msg void OnInterestAdd();
	afx_msg void OnInterestRemove();

	DECLARE_MESSAGE_MAP()
};
