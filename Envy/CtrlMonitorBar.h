//
// CtrlMonitorBar.h
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

class CGraphItem;


class CMonitorBarCtrl : public CControlBar
{
public:
	CMonitorBarCtrl();
	virtual ~CMonitorBarCtrl();

public:
	CControlBar*	m_pSnapBar[2];
	CGraphItem*		m_pTxItem;
	CGraphItem*		m_pRxItem;
	DWORD			m_nMaximumIn;
	DWORD			m_nMaximumOut;
protected:
	CBitmap			m_bmWatermark;
	CRect			m_rcTrackIn;
	CRect			m_rcTrackOut;
	CRect			m_rcTabIn;
	CRect			m_rcTabOut;
	BOOL			m_bTabIn;
	BOOL			m_bTabOut;
	HICON			m_hTabIn;
	HICON			m_hTabOut;
	HICON			m_hUpDown;

public:
	BOOL			Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);
	void			OnSkinChange();

protected:
	void			PaintHistory(CDC* pDC, CRect* prc);
	void			PaintCurrent(CDC* pDC, CRect* prc, CGraphItem* pItem, DWORD nMaximum);
	void			PaintTabs(CDC* pDC);

	virtual void	DoPaint(CDC* pDC);
	virtual CSize	CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void	OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHandler*/) {};
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

protected:
	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnDestroy();
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL	OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
