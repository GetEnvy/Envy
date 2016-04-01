//
// CtrlSharedFolder.h
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

class CLibraryFolder;


class CLibraryFolderCtrl : public CTreeCtrl
{
public:
	CLibraryFolderCtrl();
	virtual ~CLibraryFolderCtrl();

protected:
	HTREEITEM	m_hRoot;
	HTREEITEM	m_hFirstSelected;
	BOOL		m_bFirstClick;
	BOOL		m_bMultiSelect;
	BOOL		m_bSaveExpand;

public:
	void		SetMultiSelect(BOOL bMultiSelect);
	void		SetSaveExpand(BOOL bSaveExpand);
	void		Update(DWORD nUpdateCookie = 0);
	void		SetSelectedCookie(DWORD nUpdateCookie, HTREEITEM hParent = NULL, BOOL bSelect = FALSE);
	POSITION		GetSelectedFolderIterator() const;
	CLibraryFolder*	GetNextSelectedFolder(POSITION& pos) const;
	BOOL		ClearSelection(HTREEITEM hExcept = NULL, HTREEITEM hItem = NULL, BOOL bSelect = FALSE);
	BOOL		SelectAll(HTREEITEM hExcept = NULL);
	BOOL		SelectFolder(CLibraryFolder* pFolder, HTREEITEM hItem = NULL);
protected:
	void		Update(CLibraryFolder* pFolder, HTREEITEM hFolder, HTREEITEM hParent, DWORD nUpdateCookie, BOOL bRecurse);
	BOOL		SelectItems(HTREEITEM hItemFrom, HTREEITEM hItemTo);
	HTREEITEM	GetFirstSelectedItem() const;
	HTREEITEM	GetNextSelectedItem(HTREEITEM hItem) const;
	void		NotifySelectionChanged();

public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNcPaint();

	DECLARE_MESSAGE_MAP()
};
