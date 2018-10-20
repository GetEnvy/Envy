//
// DlgMediaVis.h
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

#include "DlgSkinDialog.h"

class CMediaFrame;


class CMediaVisDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CMediaVisDlg)

public:
	CMediaVisDlg(CMediaFrame* pFrame);
	virtual ~CMediaVisDlg();

	enum { IDD = IDD_MEDIA_VIS };

public:
	CButton 	m_wndSetup;
	CListCtrl	m_wndList;
	int 		m_nSize;

protected:
	CMediaFrame*	m_pFrame;
	DWORD			m_nIcon;
	HICON			m_hIcon;

protected:
	void	Enumerate();
	void	AddPlugin(LPCTSTR pszName, LPCTSTR pszCLSID, LPCTSTR pszPath);
	BOOL	EnumerateWrapped(LPCTSTR pszName, REFCLSID pCLSID, LPCTSTR pszCLSID);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblClkPlugins(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedPlugins(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetup();

	DECLARE_MESSAGE_MAP()
};
