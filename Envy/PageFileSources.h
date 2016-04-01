//
// PageFileSources.h
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

#include "DlgFilePropertiesPage.h"

class CSharedSource;


class CFileSourcesPage : public CFilePropertiesPage
{
	DECLARE_DYNCREATE(CFileSourcesPage)

public:
	CFileSourcesPage();
	virtual ~CFileSourcesPage();

	enum { IDD = IDD_FILE_SOURCES };

public:
	CButton	m_wndRemove;
	CButton	m_wndNew;
	CListCtrl	m_wndList;
	CString	m_sSource;

private:
	CImageList	m_gdiImageList;

	void AddSource(CSharedSource* pSource);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnItemChangedFileSources(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeFileSource();
	afx_msg void OnSourceRemove();
	afx_msg void OnSourceNew();
	afx_msg void OnDblClk(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};
