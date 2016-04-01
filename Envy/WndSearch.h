//
// WndSearch.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2008
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

#include "WndBaseMatch.h"
#include "CtrlSearchPanel.h"
#include "CtrlSearchDetailPanel.h"
#include "ManagedSearch.h"
#include "MatchObjects.h"


typedef CLocked< CMatchList*, CMutex* > CLockedMatchList;


class CSearchWnd : public CBaseMatchWnd
{
	DECLARE_DYNCREATE(CSearchWnd)

public:
	CSearchWnd(CQuerySearch* pSearch = NULL);
	virtual ~CSearchWnd();

public:
	BOOL IsWaitMore() const
	{
		return m_bWaitMore;
	}

	CString GetCaption() const
	{
		return m_sCaption;
	}

	CLockedMatchList GetMatches()
	{
		return CLockedMatchList( m_pMatches, &m_pMatches->m_pSection );
	}

	void			Serialize(CArchive& ar);
	CQuerySearchPtr	GetLastSearch() const;

protected:
	typedef std::list< CSearchPtr > List;
	typedef List::iterator iterator;
	typedef List::const_iterator const_iterator;
	typedef List::reverse_iterator reverse_iterator;
	typedef List::const_reverse_iterator const_reverse_iterator;

	BOOL				m_bPanel;
	BOOL				m_bSetFocus;
	CSearchPanel		m_wndPanel;
	CSearchDetailPanel	m_wndDetails;
	BOOL				m_bDetails;
	int					m_nDetails;
	List				m_oSearches;
	DWORD				m_tSearch;
	DWORD				m_nCacheHits;
	DWORD				m_nCacheHubs;
	DWORD				m_nCacheLeaves;
	CString				m_sCaption;
	BOOL				m_bWaitMore;
	DWORD				m_nMaxResults;
	DWORD				m_nMaxED2KResults;
	DWORD				m_nMaxQueryCount;

	iterator               begin()        { return m_oSearches.begin(); }
	const_iterator         begin()  const { return m_oSearches.begin(); }
	iterator               end()          { return m_oSearches.end(); }
	const_iterator         end()    const { return m_oSearches.end(); }
	reverse_iterator       rbegin()       { return m_oSearches.rbegin(); }
	const_reverse_iterator rbegin() const { return m_oSearches.rbegin(); }
	reverse_iterator       rend()         { return m_oSearches.rend(); }
	const_reverse_iterator rend()   const { return m_oSearches.rend(); }

	size_t size() const
	{
		return m_oSearches.size();
	}

	bool empty() const
	{
		return m_oSearches.empty();
	}

	void			ExecuteSearch();
	virtual void	OnSkinChange();
	virtual BOOL	OnQueryHits(const CQueryHit* pHits);
	void			UpdateMessages();
	BOOL			DoSizeDetails();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSelChangeMatches();
	afx_msg void OnUpdateSearchSearch(CCmdUI* pCmdUI);
	afx_msg void OnSearchSearch();
	afx_msg void OnSearchClear();
	afx_msg void OnUpdateSearchStop(CCmdUI* pCmdUI);
	afx_msg void OnSearchStop();
	afx_msg void OnUpdateSearchPanel(CCmdUI* pCmdUI);
	afx_msg void OnSearchPanel();
	afx_msg void OnUpdateSearchClear(CCmdUI* pCmdUI);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnUpdateSearchDetails(CCmdUI* pCmdUI);
	afx_msg void OnSearchDetails();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnUpdateFilters(CCmdUI* pCmdUI);
	afx_msg void OnFilters(UINT nID);

	DECLARE_MESSAGE_MAP()
};
