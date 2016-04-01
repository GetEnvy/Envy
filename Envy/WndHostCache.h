//
// WndHostCache.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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
#include "LiveList.h"

#define IDC_HOSTS		100

class CHostCacheHost;


class CHostCacheWnd : public CPanelWnd
{
	DECLARE_SERIAL(CHostCacheWnd)

public:
	CHostCacheWnd();
	//virtual ~CHostCacheWnd();

public:
	PROTOCOLID		m_nMode;
	BOOL			m_bAllowUpdates;
protected:
	CLiveListCtrl	m_wndList;
	CLiveListSizer	m_pSizer;
	CCoolBarCtrl	m_wndToolBar;
	CImageList		m_gdiImageList;
	DWORD			m_nCookie;
	DWORD			m_tLastUpdate;

public:
	void			Update(BOOL bForce = FALSE);
	CHostCacheHost*	GetItem(int nItem);
	virtual void	OnSkinChange();

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void RecalcLayout(BOOL bNotify = TRUE);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnUpdateHostCacheConnect(CCmdUI* pCmdUI);
	afx_msg void OnHostCacheConnect();
	afx_msg void OnUpdateHostCacheDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnHostCacheDisconnect();
	afx_msg void OnUpdateHostCacheRemove(CCmdUI* pCmdUI);
	afx_msg void OnHostCacheRemove();
	afx_msg void OnUpdateHostcacheG2Horizon(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheG2Horizon();
	afx_msg void OnUpdateHostcacheG2Cache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheG2Cache();
	afx_msg void OnUpdateHostcacheG1Cache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheG1Cache();
	afx_msg void OnUpdateHostcacheEd2kCache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheEd2kCache();
	afx_msg void OnUpdateHostcacheBTCache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheBTCache();
	afx_msg void OnUpdateHostcacheKADCache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheKADCache();
	afx_msg void OnUpdateHostcacheDCCache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheDCCache();
	afx_msg void OnUpdateHostcachePriority(CCmdUI* pCmdUI);
	afx_msg void OnHostcachePriority();
	afx_msg void OnUpdateNeighboursCopy(CCmdUI *pCmdUI);
	afx_msg void OnNeighboursCopy();
	afx_msg void OnHostcacheFileDownload();
	afx_msg void OnHostcacheImport();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
