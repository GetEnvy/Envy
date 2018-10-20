//
// WndNeighbours.h
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

#include "WndPanel.h"
#include "CtrlTipList.h"

class CNeighbour;


class CNeighboursWnd : public CPanelWnd
{
	DECLARE_SERIAL(CNeighboursWnd)

public:
	CNeighboursWnd();

protected:
	CCoolBarCtrl		m_wndToolBar;
	CTipListCtrl		m_wndList;
	CNeighbourTipCtrl	m_wndTip;
	CLiveListSizer		m_pSizer;
	CImageList			m_gdiImageList;
//	DWORD				m_tLastUpdate;	// Using static

public:
	void		Update();
	void		OpenPacketWnd(BOOL bIncoming, BOOL bOutgoing);
	void		DrawEmptyMessage(CDC* pDC);
	CNeighbour*	GetItem(int nItem);
	UINT		GetSelectedCount() const;

public:
	virtual void OnSkinChange();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateNeighboursDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursDisconnect();
	afx_msg void OnUpdateNeighboursViewAll(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursViewAll();
	afx_msg void OnUpdateNeighboursViewIncoming(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursViewIncoming();
	afx_msg void OnUpdateNeighboursViewOutgoing(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursViewOutgoing();
	afx_msg void OnDestroy();
	afx_msg void OnUpdateNeighboursChat(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursChat();
	afx_msg void OnUpdateSecurityBan(CCmdUI* pCmdUI);
	afx_msg void OnSecurityBan();
	afx_msg void OnUpdateBrowseLaunch(CCmdUI* pCmdUI);
	afx_msg void OnBrowseLaunch();
	afx_msg void OnUpdateNeighboursCopy(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursCopy();
	afx_msg void OnNeighboursSettings();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);

	DECLARE_MESSAGE_MAP()
};

#define IDC_NEIGHBOURS	100
