//
// DlgGraphList.h
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

#include "DlgSkinDialog.h"

class CLiveItem;
class CLineGraph;
class CGraphItem;


class CGraphListDlg : public CSkinDialog
{
public:
	CGraphListDlg(CWnd* pParent = NULL, CLineGraph* pGraph = NULL);

	enum { IDD = IDD_GRAPH_LIST };

public:
	CButton	m_wndCancel;
	CButton	m_wndOK;
	CSpinButtonCtrl	m_wndSpeed;
	CButton	m_wndRemove;
	CButton	m_wndEdit;
	CListCtrl m_wndList;
	DWORD	m_nSpeed;
	BOOL	m_bShowGrid;
	BOOL	m_bShowAxis;
	BOOL	m_bShowLegend;
	CString	m_sName;

	CLineGraph*	m_pGraph;
	CImageList	m_gdiImageList;

	CLiveItem*	PrepareItem(CGraphItem* pItem);
	void		SetModified();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnItemChangedGraphItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGraphAdd();
	afx_msg void OnGraphEdit();
	afx_msg void OnGraphRemove();
	afx_msg void OnDblClkGraphItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDrawItems(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
