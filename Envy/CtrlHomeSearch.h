//
// CtrlHomeSearch.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2010
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

#include "CtrlSchemaCombo.h"
#include "CtrlIconButton.h"

class CHomeSearchCtrl : public CWnd
{
	DECLARE_DYNCREATE(CHomeSearchCtrl)

public:
	CHomeSearchCtrl();

	void	OnSkinChange(COLORREF crWindow, COLORREF crText = 0);
	void	Activate();

	virtual BOOL Create(CWnd* pParentWnd, UINT nID);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	CComboBox		m_wndText;
	CSchemaCombo	m_wndSchema;
	CIconButtonCtrl	m_wndSearch;
	CIconButtonCtrl	m_wndAdvanced;
	COLORREF		m_crWindow;
	COLORREF		m_crText;

	void	FillHistory();
	void	Search(bool bAutostart);

	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnCloseUpText();
	afx_msg void OnSelChangeText();
	afx_msg void OnSearchStart();
	afx_msg void OnSearchAdvanced();

	DECLARE_MESSAGE_MAP()
};
