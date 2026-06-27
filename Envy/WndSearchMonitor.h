//
// WndSearchMonitor.h
//
// This file is part of Envy (getenvy.com) � 2016-2018
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

class CLiveItem;

// Structure for advanced filtering criteria
struct SearchFilterCriteria
{
	CString sTextFilter;          // Text filtering
	CStringArray aProtocols;      // Protocols to include (G2, G1, ED2K, DC++)
	QWORD nMinSize;              // Minimum size (0 = no limit)
	QWORD nMaxSize;              // Maximum size (0 = no limit)
	CString sIPFilter;           // IP address filtering (with wildcard support)
	CString sSchemaFilter;       // Schema filtering
	bool bFilterEnabled;         // Whether filtering is active

	SearchFilterCriteria() : nMinSize(0), nMaxSize(0), bFilterEnabled(false) {}
};

class CSearchMonitorWnd : public CPanelWnd
{
	DECLARE_SERIAL(CSearchMonitorWnd)

public:
	CSearchMonitorWnd();
//	virtual ~CSearchMonitorWnd();

protected:
	CListCtrl			m_wndList;
	CImageList			m_gdiImageList;
	CLiveListSizer		m_pSizer;
	BOOL				m_bPaused;

	CList< CLiveItem* >	m_pQueue;
	CMutexEx			m_pSection;

	// Advanced filtering
	SearchFilterCriteria m_FilterCriteria;

protected:
	virtual void OnQuerySearch(const CQuerySearch* pSearch);
	virtual void OnSkinChange();

	// Advanced filtering methods
	bool ShouldDisplaySearch(const CQuerySearch* pSearch) const;
	bool MatchesTextFilter(const CQuerySearch* pSearch) const;
	bool MatchesProtocolFilter(const CQuerySearch* pSearch) const;
	bool MatchesSizeFilter(const CQuerySearch* pSearch) const;
	bool MatchesIPFilter(const CQuerySearch* pSearch) const;
	bool MatchesSchemaFilter(const CQuerySearch* pSearch) const;
	bool MatchesWildcard(const CString& sText, const CString& sPattern) const;
	void ShowFilterDialog();
	void ClearAllFilters();
	void UpdateFilterStatus();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateSearchMonitorPause(CCmdUI* pCmdUI);
	afx_msg void OnSearchMonitorPause();
	afx_msg void OnSearchMonitorClear();
	afx_msg void OnUpdateSearchMonitorSearch(CCmdUI* pCmdUI);
	afx_msg void OnSearchMonitorSearch();
	afx_msg void OnUpdateSecurityBan(CCmdUI* pCmdUI);
	afx_msg void OnSecurityBan();
	afx_msg void OnUpdateBrowseLaunch(CCmdUI* pCmdUI);
	afx_msg void OnBrowseLaunch();
	afx_msg void OnDblClkList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateSearchMonitorFilter(CCmdUI* pCmdUI);
	afx_msg void OnSearchMonitorFilter();
	afx_msg void OnUpdateSearchMonitorFilterRemove(CCmdUI* pCmdUI);
	afx_msg void OnSearchMonitorFilterRemove();

	DECLARE_MESSAGE_MAP()
};

#define IDC_SEARCHES	100
