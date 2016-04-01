//
// WndTraffic.h
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

#include "WndChild.h"

class CGraphBase;


class CTrafficWnd : public CChildWnd
{
	DECLARE_SERIAL(CTrafficWnd)

public:
	CTrafficWnd(DWORD nUnique = 0);
	virtual ~CTrafficWnd();

public:
	DWORD		m_nUnique;
	CString		m_sName;
	CGraphBase*	m_pGraph;

protected:
	void	FindFreeUnique();
	BOOL	Serialize(BOOL bSave);
	void	SetUpdateRate();
	void	UpdateCaption();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnUpdateTrafficGrid(CCmdUI* pCmdUI);
	afx_msg void OnTrafficGrid();
	afx_msg void OnUpdateTrafficAxis(CCmdUI* pCmdUI);
	afx_msg void OnTrafficAxis();
	afx_msg void OnUpdateTrafficLegend(CCmdUI* pCmdUI);
	afx_msg void OnTrafficLegend();
	afx_msg void OnTrafficSetup();
	afx_msg void OnTrafficClear();
	afx_msg void OnTrafficWindow();

	DECLARE_MESSAGE_MAP()
};
