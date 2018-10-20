//
// CtrlMatch.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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

#include "CtrlMatchTip.h"
#include "Schema.h"

// Set Column Order
#define MATCH_COL_NAME		0
#define MATCH_COL_TYPE		1
#define MATCH_COL_SIZE		2
#define MATCH_COL_COUNT		3
#define MATCH_COL_SPEED		4
#define MATCH_COL_RATING	5
#define MATCH_COL_STATUS	6
#define MATCH_COL_CLIENT	7
#define MATCH_COL_COUNTRY	8
#define MATCH_COL_TIME		9
#define MATCH_COL_MAX		10

#define IDC_MATCHES			100
#define IDC_MATCH_HEADER	115


class CMatchList;
class CMatchFile;
class CQueryHit;


class CMatchCtrl : public CWnd
{
public:
	CMatchCtrl();
	virtual ~CMatchCtrl();

	friend class CHitMonitorWnd;
	friend class CSearchWnd;
	friend class CBrowseFrameCtrl;

public:
	CMatchList*		m_pMatches;
	LPCTSTR			m_sType;
	CSchemaPtr		m_pSchema;
	CList< CSchemaMember* > m_pColumns;

protected:
	CHeaderCtrl		m_wndHeader;
	CMatchTipCtrl	m_wndTip;
	CImageList		m_pStars;

	DWORD			m_nTopIndex;
	DWORD			m_nHitIndex;
	DWORD			m_nBottomIndex;
	DWORD			m_nFocus;
	int				m_nPageCount;
	int				m_nCurrentWidth;
	DWORD			m_nCacheItems;
	UINT			m_nMessage;
	CString			m_sMessage;
	BOOL			m_bSearchLink;
	CBitmap			m_bmSortAsc;
	CBitmap			m_bmSortDesc;
	BOOL			m_bTips;
	CMatchFile*		m_pLastSelectedFile;
	CQueryHit*		m_pLastSelectedHit;

public:
	void	Update();
	void	DestructiveUpdate();
	void	SelectSchema(CSchemaPtr pSchema, CList< CSchemaMember* >* pColumns);
	void	SetBrowseMode();
	BOOL	HitTestHeader(const CPoint& point);
	void	SetSortColumn(int nColumn = -1, BOOL bDirection = FALSE);
	void	SetMessage(UINT nMessageID, BOOL bLink = FALSE);
	void	SetMessage(LPCTSTR pszMessage, BOOL bLink = FALSE);
	void	EnableTips(BOOL bTips);
protected:
	BOOL	LoadColumnState();
	void	SaveColumnState();
	void	InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth);
	void	UpdateScroll(DWORD nScroll = 0xFFFFFFFF);
	void	ScrollBy(int nDelta);
	void	ScrollTo(DWORD nIndex);
	void	DrawItem(CDC& dc, CRect& rc, CMatchFile* pFile, CQueryHit* pHit, BOOL bFocus);
	void	DrawStatus(CDC& dc, CRect& rcCol, CMatchFile* pFile, CQueryHit* pHit, COLORREF crBack, BOOL bSelected, BOOL bSkinned = FALSE);
	void	DrawRating(CDC& dc, CRect& rcCol, int nRating, COLORREF crBack, BOOL bSelected, BOOL bSkinned = FALSE);
	void	DrawCountry(CDC& dc, CRect& rcCol, const CString& sCountry, COLORREF crBack, BOOL bSelected, BOOL bSkinned = FALSE);
	void	DrawEmptyMessage(CDC& dc, CRect& rcClient);
	BOOL	HitTest(const CPoint& point, CMatchFile** poFile, CQueryHit** poHit, DWORD* pnIndex = NULL, CRect* pRect = NULL);
	BOOL	GetItemRect(CMatchFile* pFindFile, CQueryHit* pFindHit, CRect* pRect);
	BOOL	PixelTest(const CPoint& point);
	void	MoveFocus(int nDelta, BOOL bShift);
	void	NotifySelection();
	void	SelectAll();
	void	DoDelete();
	void	DoExpand(BOOL bExpand);

public:
	virtual BOOL Create(CMatchList* pMatches, CWnd* pParentWnd);
	virtual void OnSkinChange();

public:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnChangeHeader(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnClickHeader(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()
};
