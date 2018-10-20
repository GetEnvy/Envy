//
// DlgNewSearch.h
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
#include "CtrlSchema.h"
#include "CtrlSchemaCombo.h"

class CQuerySearch;


class CNewSearchDlg : public CSkinDialog
{
public:
	CNewSearchDlg(CWnd* pParent = NULL, CQuerySearch* pSearch = NULL,
		BOOL bLocal = FALSE, BOOL bAgain = FALSE);

	enum { IDD = IDD_NEW_SEARCH };

	CQuerySearchPtr GetSearch() const
	{
		return m_pSearch;
	}

protected:
	CButton			m_wndOK;
	CButton			m_wndCancel;
	CSchemaCombo	m_wndSchemas;
	CComboBox		m_wndSearch;

private:
	CSchemaCtrl		m_wndSchema;
	BOOL			m_bLocal;
	BOOL			m_bAgain;
	CQuerySearchPtr m_pSearch;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnCloseUpSchemas();
	afx_msg void OnChangeSearch();

	DECLARE_MESSAGE_MAP()
};

#define IDC_METADATA		100
