//
// CtrlMediaList.h
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

#include "CtrlLibraryTip.h"


class CMediaListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CMediaListCtrl)

public:
	CMediaListCtrl();
	virtual ~CMediaListCtrl();

protected:
	CLibraryTipCtrl	m_wndTip;
	CImageList*	m_pDragImage;
	int			m_nDragDrop;
	BOOL		m_bCreateDragImage;
//	UINT		m_nSelectedCount;	// Using static
//	DWORD		m_tLastUpdate;		// Using static

public:
	virtual BOOL Create(CWnd* pParentWnd, UINT nID);

	BOOL	Open(LPCTSTR pszFile);
	BOOL	Enqueue(LPCTSTR pszFile, BOOL bStart = TRUE);
	int		RecursiveEnqueue(LPCTSTR pszPath);
	void	Remove(LPCTSTR pszFile);
	BOOL	LoadTextList(LPCTSTR pszFile);
	BOOL	SaveTextList(LPCTSTR pszFile);
	int		GetCount();
	UINT	GetSelectedCount();
	int		GetCurrent();
	void	SetCurrent(int nCurrent);
	void	Clear();
	void	Reset(BOOL bNext = TRUE);
	int		GetNext(BOOL bSet = TRUE);
	CString	GetPath(int nItem);
	void	OnSkinChange();

protected:
	int		Add(LPCTSTR pszPath, int nItem = -1);
	void	Remove(int nItem);
	BOOL	AreSelectedFilesInLibrary();
	void	ShowFilePropertiesDlg(int nPage = 0 );

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMHDR* pNotify, LRESULT* pResult);
	afx_msg void OnMediaAdd();
	afx_msg void OnUpdateMediaRate(CCmdUI* pCmdUI);
	afx_msg void OnMediaRate();
	afx_msg void OnUpdateMediaProperties(CCmdUI* pCmdUI);
	afx_msg void OnMediaProperties();
	afx_msg void OnUpdateMediaRemove(CCmdUI* pCmdUI);
	afx_msg void OnMediaRemove();
	afx_msg void OnUpdateMediaClear(CCmdUI* pCmdUI);
	afx_msg void OnMediaClear();
	afx_msg void OnUpdateMediaSelect(CCmdUI* pCmdUI);
	afx_msg void OnMediaSelect();
	afx_msg void OnUpdateMediaSave(CCmdUI* pCmdUI);
	afx_msg void OnMediaSave();
	afx_msg void OnMediaOpen();
	afx_msg void OnUpdateMediaPrevious(CCmdUI* pCmdUI);
	afx_msg void OnMediaPrevious();
	afx_msg void OnUpdateMediaNext(CCmdUI* pCmdUI);
	afx_msg void OnMediaNext();
	afx_msg void OnUpdateMediaRepeat(CCmdUI* pCmdUI);
	afx_msg void OnMediaRepeat();
	afx_msg void OnUpdateMediaRandom(CCmdUI* pCmdUI);
	afx_msg void OnMediaRandom();
	afx_msg void OnUpdateMediaCollection(CCmdUI* pCmdUI);
	afx_msg void OnMediaCollection();
	afx_msg void OnMediaAddFolder();

	DECLARE_MESSAGE_MAP()
};

#define MLN_NEWCURRENT	101
