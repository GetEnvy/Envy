//
// CtrlDownloads.h
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

#include "CtrlDownloadTip.h"
#include "DisplayData.h"

class CDownload;
class CDownloadSource;
class CDownloadDisplayData;


class CDownloadsCtrl : public CWnd
{
	DECLARE_DYNAMIC(CDownloadsCtrl)

public:
	CDownloadsCtrl();

public:
	BOOL		Create(CWnd* pParentWnd, UINT nID);
	BOOL		Update();
	BOOL		Update(int nGroupCookie);
	int 		GetExpandableColumnX() const;
	BOOL		DropObjects(CList< CDownload* >* pSel, const CPoint& ptScreen);
	void		OnMouseMoveDrag(const CPoint& ptScreen);	// Was DropShowTarget(CList< CDownload* >* pSel, const CPoint& ptScreen)
	void		OnSkinChange();
protected:
	void		InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth);
	void		SaveColumnState();
	BOOL		LoadColumnState();
	void		SelectTo(int nIndex);
	void		BubbleSortDownloads(int nColumn);
	void		SelectAll(CDownload* pExcept1 = NULL, CDownloadSource* pExcept2 = NULL);
	void		DeselectAll(CDownload* pExcept1 = NULL, CDownloadSource* pExcept2 = NULL);
	BOOL		GetSelectedList(CList< CDownload* >& pList, BOOL bClearing = FALSE);
	int 		GetSelectedCount();
	void		MoveSelected(int nDelta);
	void		MoveToTop();
	void		MoveToEnd();
	BOOL		HitTest(const CPoint& point, CDownload** ppDownload, CDownloadSource** ppSource, int* pnIndex, RECT* prcItem);
	BOOL		HitTest(int nIndex, CDownload** ppDownload, CDownloadSource** ppSource);
	BOOL		GetAt(int nSelect, CDownload** ppDownload, CDownloadSource** ppSource);
	BOOL		GetRect(CDownload* pSelect, RECT* prcItem);
	void		PaintDownload(CDC& dc, const CRect& rcRow, const CDownloadDisplayData* pDownloadData, BOOL bFocus = FALSE);
	void		PaintSource(CDC& dc, const CRect& rcRow, const CSourceDisplayData* pSourceData, BOOL bFocus = FALSE);
	void		UpdateDownloadsData(BOOL bForce = FALSE);
	void		OnBeginDrag(CPoint ptAction);
	CImageList*	CreateDragImage(CList< CDownload* >* pSel, const CPoint& ptMouse);	// ToDo: Remove List
public:
	static bool	IsFiltered(const CDownload* pDownload);
	static BOOL	IsExpandable(const CDownload* pDownload);

	friend class CDownloadsWnd;

protected:
	CHeaderCtrl			m_wndHeader;
	CDownloadTipCtrl	m_wndTip;
	CImageList			m_pProtocols;
	int 				m_nGroupCookie;
	int 				m_nFocus;
	int 				m_nHover;			// Visible index
//	CDownload*			m_pDragDrop;		// Use m_nHover
	BOOL				m_bCreateDragImage;
	BOOL				m_bDragStart;
	BOOL				m_bDragActive;
	CPoint				m_ptDrag;
	CDownload*			m_pDeselect1;
	CDownloadSource*	m_pDeselect2;
	BOOL*				m_pbSortAscending;
	BOOL				m_bShowSearching;
//	DWORD				m_tSwitchTimer;		// Using static

	CArray< CDownloadDisplayData > m_pDownloadsData;

public:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangeHeader(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnSortPanelItems(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnEnterKey();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();

protected:
	DECLARE_MESSAGE_MAP()
};


#define DLF_ACTIVE		0x01
#define DLF_QUEUED		0x02
#define DLF_SOURCES		0x04
#define DLF_PAUSED		0x08
#define DLF_SEED		0x10

#define DLF_ALL			(DLF_ACTIVE|DLF_QUEUED|DLF_SOURCES|DLF_PAUSED|DLF_SEED)
