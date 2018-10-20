//
// WndDownloads.h
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

#include "WndPanel.h"
#include "CtrlDownloads.h"
#include "CtrlDownloadTabBar.h"


class CDownloadsWnd : public CPanelWnd
{
	DECLARE_SERIAL(CDownloadsWnd)

public:
	CDownloadsWnd();
	virtual ~CDownloadsWnd();

public:
	void			Update();
	void			Select(CDownload* pDownload);	// For DownloadMonitor "Show"
	void			DragDownloads(CList< CDownload* >* pList, CImageList* pImage, const CPoint& ptScreen);
protected:
	void			Prepare();
	void			CancelDrag();

protected:
	CDownloadsCtrl	m_wndDownloads;
	CDownloadTabBar	m_wndTabBar;
	CCoolBarCtrl	m_wndToolBar;
	CList< CDownload* >* m_pDragList;
	CImageList*		m_pDragImage;
	CPoint			m_pDragOffs;
	HCURSOR			m_hCursMove;
	HCURSOR			m_hCursCopy;
	int				m_nMoreSourcesLimiter;
	DWORD			m_tMoreSourcesTimer;
	DWORD			m_tLastUpdate;
	bool			m_bMouseCaptured;
	DWORD			m_nSelectedDownloads;
	DWORD			m_nSelectedSources;
	DWORD			m_tSel;
	BOOL			m_bSelAny;
	BOOL			m_bSelDownload;
	BOOL			m_bSelSource;
	BOOL			m_bSelTrying;
	BOOL			m_bSelPaused;
	BOOL			m_bSelNotPausedOrMoving;
	BOOL			m_bSelNoPreview;
	BOOL			m_bSelNotCompleteAndNoPreview;
	BOOL			m_bSelCompletedAndNoPreview;
	BOOL			m_bSelStartedAndNotMoving;
	BOOL			m_bSelNotMoving;
	BOOL			m_bSelCompleted;
	BOOL			m_bSelShareState;
	BOOL			m_bSelSeeding;
	BOOL			m_bSelTorrent;
	BOOL			m_bSelIdleSource;
	BOOL			m_bSelActiveSource;
	BOOL			m_bSelBoostable;
	BOOL			m_bSelBrowse;
	BOOL			m_bSelChat;
	BOOL			m_bSelShareConsistent;
	BOOL			m_bSelMoreSourcesOK;
	BOOL			m_bSelSourceAcceptConnections;
	BOOL			m_bSelSourceExtended;
	BOOL			m_bSelRemotePreviewCapable;
	BOOL			m_bSelHasReviews;
	BOOL			m_bSelHasSize;
	BOOL			m_bSelHasHash;	// m_bSelSHA1orTTHorED2KorName
	BOOL			m_bSelHasName;
	BOOL			m_bConnectOkay;

public:
	virtual void OnSkinChange();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual HRESULT	GetGenericView(IGenericView** ppView);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateDownloadsResume(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsResume();
	afx_msg void OnUpdateDownloadsPause(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsPause();
	afx_msg void OnUpdateDownloadsClear(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsClear();
	afx_msg void OnUpdateDownloadsLaunch(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsLaunch();
	afx_msg void OnUpdateDownloadsViewReviews(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsViewReviews();
	afx_msg void OnUpdateDownloadsRemotePreview(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsRemotePreview();
	afx_msg void OnUpdateDownloadsSources(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsSources();
	afx_msg void OnDownloadsClearCompleted();
	afx_msg void OnDownloadsClearPaused();
	afx_msg void OnUpdateTransfersDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnTransfersDisconnect();
	afx_msg void OnUpdateTransfersForget(CCmdUI* pCmdUI);
	afx_msg void OnTransfersForget();
	afx_msg void OnUpdateTransfersChat(CCmdUI* pCmdUI);
	afx_msg void OnTransfersChat();
	afx_msg void OnUpdateDownloadsMergeLocal(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsMergeLocal();
	afx_msg void OnUpdateDownloadsAddSource(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsAddSource();
	afx_msg void OnUpdateDownloadsBoost(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsBoost();
	afx_msg void OnUpdateDownloadsEnqueue(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsEnqueue();
	afx_msg void OnUpdateDownloadsAutoClear(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsAutoClear();
	afx_msg void OnUpdateTransfersConnect(CCmdUI* pCmdUI);
	afx_msg void OnTransfersConnect();
	afx_msg void OnUpdateDownloadsShowSources(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsShowSources();
	afx_msg void OnUpdateBrowseLaunch(CCmdUI* pCmdUI);
	afx_msg void OnBrowseLaunch();
	afx_msg void OnUpdateDownloadsLaunchCopy(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsLaunchCopy();
	afx_msg void OnUpdateDownloadsFolder(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFolder();
	afx_msg void OnUpdateDownloadsMonitor(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsMonitor();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnUpdateDownloadsFileDelete(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFileDelete();
	afx_msg void OnUpdateDownloadsRate(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsRate();
	afx_msg void OnUpdateDownloadsMove(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsMoveUp();
	afx_msg void OnDownloadsMoveDown();
	afx_msg void OnDownloadsMoveTop();
	afx_msg void OnDownloadsMoveBottom();
	afx_msg void OnDownloadsSettings();
	afx_msg void OnUpdateDownloadsFilterAll(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterAll();
	afx_msg void OnUpdateDownloadsFilterActive(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterActive();
	afx_msg void OnUpdateDownloadsFilterPaused(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterPaused();
	afx_msg void OnUpdateDownloadsFilterQueued(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterQueued();
	afx_msg void OnUpdateDownloadsFilterSources(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterSources();
	afx_msg void OnUpdateDownloadsFilterSeeds(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterSeeds();
	afx_msg void OnUpdateDownloadsLaunchComplete(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsLaunchComplete();
	afx_msg void OnUpdateDownloadsShare(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsShare();
	afx_msg void OnUpdateDownloadsURI(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsURI();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnUpdateDownloadGroupShow(CCmdUI* pCmdUI);
	afx_msg void OnDownloadGroupShow();
	afx_msg void OnDownloadsHelp();
	afx_msg void OnDownloadsFilterMenu();
	afx_msg void OnUpdateDownloadsClearIncomplete(CCmdUI *pCmdUI);
	afx_msg void OnDownloadsClearIncomplete();
	afx_msg void OnUpdateDownloadsClearComplete(CCmdUI *pCmdUI);
	afx_msg void OnDownloadsClearComplete();
	afx_msg void OnUpdateDownloadsEdit(CCmdUI *pCmdUI);
	afx_msg void OnDownloadsEdit();
	afx_msg void OnCaptureChanged(CWnd *pWnd);

	DECLARE_MESSAGE_MAP()
};

#define IDC_DOWNLOADS	100
