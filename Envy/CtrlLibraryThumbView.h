//
// CtrlLibraryThumbView.h
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

#include "CtrlLibraryFileView.h"
#include "Library.h"

class CLibraryThumbItem;
class CLibraryFile;


class CLibraryThumbView :
	public CLibraryFileView,
	public CThreadImpl
{
	DECLARE_DYNCREATE(CLibraryThumbView)

public:
	CLibraryThumbView();

protected:
	CLibraryThumbItem**	m_pList;
	CLibraryThumbItem*	m_pFirst;
	CLibraryThumbItem*	m_pFocus;
	CList< CLibraryThumbItem* > m_pSelThumb;
	DWORD				m_nInvalidate;
	int					m_nColumns;
	int					m_nRows;
	int					m_nCount;
	int					m_nBuffer;
	int					m_nScroll;
	int					m_nSelected;
	BOOL				m_bDrag;
	CPoint				m_ptDrag;

protected:
	void				Clear();
	int					GetThumbIndex(CLibraryThumbItem* pThumb) const;
	BOOL				Select(CLibraryThumbItem* pThumb, TRISTATE bSelect = TRI_TRUE);
	BOOL				DeselectAll(CLibraryThumbItem* pThumb = NULL);
	BOOL				SelectTo(CLibraryThumbItem* pThumb);
	void				SelectTo(int nDelta);
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nDelta);
	void				UpdateScroll();
	BOOL				GetItemRect(CLibraryThumbItem* pThumb, CRect* pRect);
	CLibraryThumbItem*	HitTest(const CPoint& point) const;
	void				StartThread();
	void				StopThread();
	void				OnRun();

	static int			SortList(LPCVOID pA, LPCVOID pB);

protected:
	virtual BOOL		Create(CWnd* pParentWnd);
	virtual void		Update();
	virtual void		SelectAll();
	virtual BOOL		Select(DWORD nObject);
	virtual DWORD_PTR	HitTestIndex(const CPoint& point) const;
	virtual HBITMAP		CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()
};


class CLibraryThumbItem
{
public:
	CLibraryThumbItem(CLibraryFile* pFile);
	virtual ~CLibraryThumbItem();

public:
	DWORD	m_nIndex;
	DWORD	m_nCookie;
	CString	m_sText;
	BOOL	m_bShared;

	BOOL	m_bSelected;
	int		m_nThumb;
	CBitmap	m_bmThumb;
	int		m_nShell;

	enum { thumbWaiting, thumbValid, thumbError };

public:
	BOOL	Update(CLibraryFile* pFile);
	void	Paint(CDC* pDC, const CRect& rcBlock);
};
