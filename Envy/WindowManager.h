//
// WindowManager.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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


class CWindowManager : public CWnd
{
	DECLARE_DYNCREATE(CWindowManager)

public:
	CWindowManager(CMDIFrameWnd* pParent = NULL);
	virtual ~CWindowManager();

public:
	CMDIFrameWnd*		m_pParent;
	CList< CChildWnd* >	m_pWindows;
	CRect				m_rcSize;
	BOOL				m_bIgnoreActivate;

	void		SetOwner(CMDIFrameWnd* pParent);
	CChildWnd*	GetActive() const;
	POSITION	GetIterator() const;
	CChildWnd*	GetNext(POSITION& pos) const;
	CChildWnd*	Open(CRuntimeClass* pClass, BOOL bToggle = FALSE, BOOL bFocus = TRUE);
	CChildWnd*	Find(CRuntimeClass* pClass, CChildWnd* pAfter = NULL, CChildWnd* pExcept = NULL);
	CChildWnd*	FindFromPoint(const CPoint& point) const;
	BOOL		Check(CChildWnd* pChild) const;
	BOOL		IsEmpty() const { return m_pWindows.IsEmpty(); }
	void		Close();
	void		AutoResize();
	void		Cascade(BOOL bActiveOnly = FALSE);
	void		SetGUIMode(int nMode, BOOL bSaveState = TRUE);
	void		LoadWindowStates();
	void		SaveWindowStates() const;
	BOOL		LoadSearchWindows();
	BOOL		SaveSearchWindows() const;
	BOOL		LoadBrowseHostWindows();
	BOOL		SaveBrowseHostWindows() const;
	void		OpenNewSearchWindow();
	void		PostSkinChange();
	void		PostSkinRemove();

protected:
	void		Add(CChildWnd* pChild);
	void		Remove(CChildWnd* pChild);
	void		ActivateGrouped(CChildWnd* pExcept);

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
//	afx_msg void OnStyleChanging(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
//	afx_msg void OnMDIActivate();

	DECLARE_MESSAGE_MAP()

	friend class CChildWnd;
	friend class CPluginWnd;
};
