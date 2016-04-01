//
// RichViewCtrl.h
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

class CRichDocument;
class CRichElement;
class CRichFragment;

typedef struct
{
	int		nFragment;
	int		nOffset;
} RICHPOSITION;


class CRichViewCtrl : public CWnd
{
	DECLARE_DYNCREATE(CRichViewCtrl)

public:
	CRichViewCtrl();
	virtual ~CRichViewCtrl();

protected:
	CSyncObject*	m_pSyncRoot;
	BOOL			m_bSelectable;
	BOOL			m_bFollowBottom;
	BOOL			m_bDefaultLink;

	CRichDocument*	m_pDocument;
	CArray< CRichFragment* > m_pFragments;
	DWORD			m_nCookie;
	int				m_nLength;

	BOOL			m_bSelecting;
	CRichElement*	m_pHover;
	RICHPOSITION	m_pSelStart;
	RICHPOSITION	m_pSelEnd;
	RICHPOSITION	m_pSelAbsStart;
	RICHPOSITION	m_pSelAbsEnd;

	HCURSOR			m_hcHand;
	HCURSOR			m_hcText;
	CBrush			m_pBrush;

public:
	void			SetSyncObject(CSyncObject* pSyncRoot);
	void			SetSelectable(BOOL bSelectable);
	void			SetFollowBottom(BOOL bFollowBottom);
	void			SetDefaultLink(BOOL bDefaultLink);
	void			SetDocument(CRichDocument* pDocument);
	BOOL			IsModified() const;
	void			InvalidateIfModified();
	int				FullHeightMove(int nX, int nY, int nWidth, BOOL bShow = FALSE);
	BOOL			GetElementRect(CRichElement* pElement, RECT* prc) const;
	CString			GetWordFromPoint(CPoint& point, LPCTSTR szTokens) const;
protected:
	void			ClearFragments();
	void			Layout(CDC* pDC, CRect* pRect);
	void			WrapLineHelper(CList< CRichFragment* >& pLine, CPoint& pt, int& nLineHeight, int nWidth, int nAlign);
	CRichFragment*	PointToFrag(CPoint& pt) const;
	RICHPOSITION	PointToPosition(CPoint& pt) const;
	CPoint			PositionToPoint(RICHPOSITION& pt) const;
	void			CopySelection() const;
	void			UpdateSelection();

	virtual void	OnLayoutComplete() {};
	virtual void	OnPaintBegin(CDC* /*pDC*/) {};
	virtual void	OnPaintComplete(CDC* /*pDC*/) {};
	virtual void	OnVScrolled() {};

public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()

	friend class CRichFragment;
	friend class CIRCFrame; 	// For VScroll
};

typedef struct
{
	NMHDR			hdr;
	CRichElement*	pElement;
} RVN_ELEMENTEVENT;

#define RVN_CLICK		100
#define RVN_DBLCLICK	101
#define RVN_SETCURSOR	102
