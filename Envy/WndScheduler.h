//
// WndScheduler.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2010 and Shareaza 2010
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

#include "WndPanel.h"

class CScheduleItem;


class CSchedulerWnd : public CPanelWnd
{
	DECLARE_SERIAL(CSchedulerWnd)

public:
	CSchedulerWnd();
	virtual ~CSchedulerWnd();

protected:
	CCoolBarCtrl	m_wndToolBar;
	CListCtrl		m_wndList;
	CImageList		m_gdiImageList;
	CLiveListSizer	m_pSizer;
	DWORD			tLastUpdate;

public:
	CScheduleTask*	GetItem(int nItem);
	void			Update(int nColumn = -1, BOOL bSort = TRUE);

	virtual void	OnSkinChange();
	virtual BOOL	PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnUpdateSchedulerAdd(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerAdd();
	afx_msg void OnUpdateSchedulerActivate(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerActivate();
	afx_msg void OnUpdateSchedulerDeactivate(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerDeactivate();
	afx_msg void OnUpdateSchedulerEdit(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerEdit();
	afx_msg void OnUpdateSchedulerRemove(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerRemove();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	afx_msg void OnUpdateSchedulerRemoveAll(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerRemoveAll();

	afx_msg void OnUpdateSchedulerExport(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerExport();

	//afx_msg void OnUpdateSchedulerImport(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerImport();

	DECLARE_MESSAGE_MAP()
};

enum {
	SCHEDULE_NO_ITEM = 1,
	SCHEDULE_ITEM_ACTIVE,
	SCHEDULE_ITEM_INACTIVE
};

#define IDC_SCHEDULE		100
