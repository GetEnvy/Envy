//
// CtrlDownloadTabBar.h
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

class CDownload;
class CDownloadGroup;


class CDownloadTabBar : public CControlBar
{
public:
	CDownloadTabBar();
	virtual ~CDownloadTabBar();

// Item Class
public:
	class TabItem
	{
	public:
		TabItem(CDownloadGroup* pGroup, int nCookie);
		virtual ~TabItem();

	public:
		CDownloadGroup*	m_pGroup;
		CString			m_sCaption;
		CString			m_sName;
		int				m_nImage;
		int				m_nCount;
		BOOL			m_bSelected;

	public:
		BOOL			Update(int nCookie);
		BOOL			Select(BOOL bSelect);
		void			Paint(CDownloadTabBar* pBar, CDC* pDC, CRect* pRect, BOOL bHot, BOOL bTransparent);
	};

protected:
	CList< TabItem* > m_pItems;
	TabItem*		m_pHot;
	BOOL			m_bTimer;
	BOOL			m_bMenuGray;
	int				m_nCookie;

	int				m_nMaximumWidth;
	UINT			m_nMessage;
	CString			m_sMessage;
	CBitmap			m_bmImage;

public:
	void			OnSkinChange();
	void			Update(int nCookie);
	BOOL			Select(TabItem* pHit = NULL);
	TabItem*		HitTest(const CPoint& point, CRect* pItemRect = NULL) const;
	BOOL			DropObjects(CList< CDownload* >* pList, const CPoint& ptScreen);
	void			OnMouseMoveDrag(const CPoint& ptScreen);	// Was DropShowTarget(CList< CDownload* >* pList, const CPoint& ptScreen)
protected:
	void			UpdateGroups(int nCookie);
	void			UpdateStates(int nCookie);
	int				GetSelectedCount(BOOL bDownloads = FALSE);
	TabItem*		GetSelectedItem();
	CDownloadGroup*	GetSelectedGroup();
	void			GetSelectedDownloads(CList< CDownload* >* pDownloads);
	void			NotifySelection();

public:
	virtual BOOL	Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD|WS_VISIBLE|CBRS_BOTTOM, UINT nID = AFX_IDW_STATUS_BAR);
	virtual CSize	CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual INT_PTR	OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual void	OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/) {};
	virtual void	DoPaint(CDC* pDC);

protected:
	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnTimer(UINT_PTR nIDEvent);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void	OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void	OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void	OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void	OnDownloadGroupNew();
	afx_msg void	OnUpdateDownloadGroupRemove(CCmdUI* pCmdUI);
	afx_msg void	OnDownloadGroupRemove();
	afx_msg void	OnUpdateDownloadGroupMoveRight(CCmdUI* pCmdUI);
	afx_msg void	OnDownloadGroupMoveRight();
	afx_msg void	OnUpdateDownloadGroupMoveLeft(CCmdUI* pCmdUI);
	afx_msg void	OnDownloadGroupMoveLeft();
	afx_msg void	OnUpdateDownloadGroupProperties(CCmdUI* pCmdUI);
	afx_msg void	OnDownloadGroupProperties();
	afx_msg void	OnUpdateDownloadGroupResume(CCmdUI* pCmdUI);
	afx_msg void	OnDownloadGroupResume();
	afx_msg void	OnUpdateDownloadGroupPause(CCmdUI* pCmdUI);
	afx_msg void	OnDownloadGroupPause();
	afx_msg void	OnUpdateDownloadGroupClear(CCmdUI* pCmdUI);
	afx_msg void	OnDownloadGroupClear();
	afx_msg void	OnDownloadGroupOpen();

	DECLARE_MESSAGE_MAP()

	friend class TabItem;
};
