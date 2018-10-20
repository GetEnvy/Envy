//
// CtrlBrowseTree.h
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

#include "Schema.h"

class CBrowseTreeItem;
class CG2Packet;
class CXMLElement;


class CBrowseTreeCtrl : public CWnd
{
	DECLARE_DYNAMIC(CBrowseTreeCtrl)

public:
	CBrowseTreeCtrl();
	virtual ~CBrowseTreeCtrl();

protected:
	CCriticalSection	m_csRoot;
	CBrowseTreeItem*	m_pRoot;
	int					m_nTotal;
	int					m_nVisible;
	int					m_nScroll;
	int					m_nSelected;
	CBrowseTreeItem*	m_pSelFirst;
	CBrowseTreeItem*	m_pSelLast;
	CBrowseTreeItem*	m_pFocus;
	DWORD				m_nCleanCookie;

public:
	virtual BOOL		Create(CWnd* pParentWnd);
	void				Clear(BOOL bGUI = TRUE);
	BOOL				Expand(CBrowseTreeItem* pItem, TRISTATE bExpand = TRI_TRUE, BOOL bInvalidate = TRUE);
	BOOL				Select(CBrowseTreeItem* pItem, TRISTATE bSelect = TRI_TRUE, BOOL bInvalidate = TRUE);
	BOOL				DeselectAll(CBrowseTreeItem* pExcept = NULL, CBrowseTreeItem* pParent = NULL, BOOL bInvalidate = TRUE);
	BOOL				Highlight(CBrowseTreeItem* pItem);
	BOOL				GetRect(CBrowseTreeItem* pItem, RECT* pRect);
	int					GetSelectedCount() const;
	CBrowseTreeItem*	GetFirstSelected() const;
	CBrowseTreeItem*	GetLastSelected() const;
	CBrowseTreeItem*	HitTest(const POINT& point, RECT* pRect = NULL) const;
	void				OnTreePacket(CG2Packet* pPacket);
protected:
	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nPosition);
	void				Paint(CDC& dc, CRect& rcClient, CPoint& pt, CBrowseTreeItem* pItem);
	CBrowseTreeItem*	HitTest(CRect& rcClient, CPoint& pt, CBrowseTreeItem* pItem, const POINT& point, RECT* pRect) const;
	BOOL				GetRect(CPoint& pt, CBrowseTreeItem* pItem, CBrowseTreeItem* pFind, RECT* pRect);
	BOOL				CleanItems(CBrowseTreeItem* pItem, DWORD nCookie, BOOL bVisible);
	BOOL				CollapseRecursive(CBrowseTreeItem* pItem);
	void				NotifySelection();
	void				OnTreePacket(CG2Packet* pPacket, DWORD nFinish, CBrowseTreeItem* pItem);

// Inlines
public:
	inline CSyncObject* SyncRoot()
	{
		return &m_csRoot;
	}

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg LRESULT OnUpdate(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};


class CBrowseTreeItem : public CObject
{
public:
	CBrowseTreeItem(CBrowseTreeItem* pParent = NULL);
	virtual ~CBrowseTreeItem();

public:
	CBrowseTreeItem*	m_pParent;
	CBrowseTreeItem**	m_pList;
	int					m_nCount;
	int					m_nBuffer;
	CBrowseTreeItem*	m_pSelPrev;
	CBrowseTreeItem*	m_pSelNext;
	DWORD				m_nCleanCookie;

	BOOL				m_bExpanded;
	BOOL				m_bSelected;
	BOOL				m_bContract1;
	BOOL				m_bContract2;

	DWORD				m_nCookie;
	CString				m_sText;
	BOOL				m_bBold;
	int					m_nIcon16;

	CSchemaPtr			m_pSchema;
	DWORD*				m_pFiles;
	DWORD				m_nFiles;

public:
	CBrowseTreeItem*	Add(LPCTSTR pszName);
	CBrowseTreeItem*	Add(CBrowseTreeItem* pNewItem);
	void				Clear();
	void				Delete();
	void				Delete(int nItem);
	void				Delete(CBrowseTreeItem* pItem);
	BOOL				IsVisible() const;
	int					GetChildCount() const;
	void				AddXML(const CXMLElement* pXML);
	void				Paint(CDC& dc, CRect& rc, BOOL bTarget, COLORREF crBack = CLR_NONE) const;
};

#define IDC_BROWSE_TREE 125
#define BTN_SELCHANGED	101
