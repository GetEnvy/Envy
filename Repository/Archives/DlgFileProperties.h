//
// DlgFileProperties.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2010
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

class CFilePropertiesDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CFilePropertiesDlg)

public:
	CFilePropertiesDlg(CWnd* pParent = NULL, DWORD nIndex = 0);

public:
	CStatic	m_wndHash;
	CStatic	m_wndIcon;
	CButton	m_wndCancel;
	CButton	m_wndOK;
	CSchemaCombo	m_wndSchemas;
	CString	m_sName;
	CString	m_sSize;
	CString	m_sType;
	CString	m_sPath;
	CString	m_sIndex;
	CString	m_sSHA1;
	CString	m_sTiger;

	void	Update();

protected:
	CSchemaCtrl	m_wndSchema;
	DWORD		m_nIndex;
	BOOL		m_bHexHash;
	int			m_nWidth;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSelChangeSchemas();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnCloseUpSchemas();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};

#define IDC_METADATA		100
