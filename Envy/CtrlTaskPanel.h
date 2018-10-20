//
// CtrlTaskPanel.h
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

class CTaskBox;


class CTaskPanel : public CWnd
{
	DECLARE_DYNAMIC(CTaskPanel)

public:
	CTaskPanel();

protected:
	CList< CTaskBox* >	m_pBoxes;
	CTaskBox*	m_pStretch;
	CBitmap		m_bmWatermark;
	CBitmap		m_bmFooter;
//	int			m_nMargin;	// MARGIN_WIDTH
//	int			m_nCurve;
	BOOL		m_bLayout;

public:
	CTaskBox*	AddBox(CTaskBox* pBox, POSITION posBefore = NULL);
	POSITION	GetBoxIterator() const;
	CTaskBox*	GetNextBox(POSITION& pos) const;
	INT_PTR		GetBoxCount() const;
	void		RemoveBox(CTaskBox* pBox);
	void		ClearBoxes(BOOL bDelete);

	void		SetStretchBox(CTaskBox* pBox);
	void		SetWatermark(LPCTSTR szWatermark);
	void		SetFooter(LPCTSTR szWatermark);
//	void		SetMargin(int nMargin, int nCurve = 2);
	void		OnChanged();
protected:
	void		Layout(CRect& rcClient);

public:
	virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()

	friend class CTaskBox;
};


class CTaskBox : public CButton
{
	DECLARE_DYNAMIC(CTaskBox)

public:
	CTaskBox();
	virtual ~CTaskBox();

public:
	CTaskPanel*	GetPanel() const;
	void		SetCaption(LPCTSTR pszCaption);
	void		SetIcon(HICON hIcon);
	void		SetSize(int nHeight);
	void		SetPrimary(BOOL bPrimary);
	void		SetWatermark(LPCTSTR szWatermark);
	void		SetCaptionmark(LPCTSTR szWatermark);
	void		Expand(BOOL bOpen = TRUE);

protected:
	CTaskPanel*	m_pPanel;
	int			m_nHeight;
	BOOL		m_bVisible;
	BOOL		m_bOpen;
	BOOL		m_bHover;
	BOOL		m_bPrimary;
	HICON		m_hIcon;
	BOOL		m_bIconDel;
	CBitmap		m_bmWatermark;
	CBitmap		m_bmCaptionmark;
	BOOL		m_bCaptionCurve;

	int			GetOuterHeight() const;
	void		PaintBorders();
	void		InvalidateNonclient();

public:
	virtual BOOL Create(CTaskPanel* pPanel, int nHeight = 0, LPCTSTR pszCaption = NULL, UINT nIDIcon = 0, UINT nID = 0);
	virtual void DrawItem(LPDRAWITEMSTRUCT) {}
protected:
	virtual void OnExpanded(BOOL bOpen);

protected:
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()

	friend class CTaskPanel;
};
