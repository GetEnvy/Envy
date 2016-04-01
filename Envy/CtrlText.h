//
// CtrlText.h
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

class CTextLine;


class CTextCtrl : public CWnd
{
	DECLARE_DYNCREATE(CTextCtrl)

public:
	CTextCtrl();
	virtual ~CTextCtrl();

protected:
	mutable CCriticalSection	m_pSection;

	CArray< CTextLine* > m_pLines;
	int					m_nPosition;
	int					m_nTotal;
	int					m_nHeight;
//	COLORREF			m_crBackground[4];
	COLORREF			m_crText[5];
	CFont				m_pFont;
	BOOL				m_bProcess;
	int					m_nLastClicked;			// Index of last clicked item

public:
	void	Add(const CLogMessage* pMsg);
	void	AddLine(WORD nType, const CString& strLine);
	void	Clear(BOOL bInvalidate = TRUE);
	void	CopyText() const;

protected:
	void	UpdateScroll(BOOL bFull = FALSE);
	int		HitTest(const CPoint& pt) const;

public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	void OnSkinChange();

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);

	DECLARE_MESSAGE_MAP()
};


class CTextLine
{
public:
	CTextLine(WORD nType, const CString& strText);
	~CTextLine();

public:
	CString	m_sText;
	int*	m_pLine;
	int		m_nLine;
	WORD	m_nType;
	BOOL	m_bSelected;

public:
	int		Process(CDC* pDC, int nWidth);
	void	Paint(CDC* pDC, CRect* pRect, BOOL bSkinned = FALSE);
protected:
	void	AddLine(int nLength);
};
