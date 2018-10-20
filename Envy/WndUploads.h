//
// WndUploads.h
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
#include "CtrlUploads.h"


class CUploadsWnd : public CPanelWnd
{
	DECLARE_SERIAL(CUploadsWnd)

public:
	CUploadsWnd();
	virtual ~CUploadsWnd();

public:
	CUploadsCtrl m_wndUploads;
	CCoolBarCtrl m_wndToolBar;
protected:
	DWORD		 m_tLastUpdate;
	DWORD		 m_tSel;
	BOOL		 m_bSelFile;
	BOOL		 m_bSelUpload;
	BOOL		 m_bSelActive;
	BOOL		 m_bSelQueued;
	BOOL		 m_bSelChat;
	BOOL		 m_bSelBrowse;
	BOOL		 m_bSelPartial;
	BOOL		 m_bSelSourceAcceptConnections;
	DWORD		 m_nSelected;

protected:
	void		 Prepare();
	inline BOOL	 IsSelected(const CUploadFile* pFile) const;

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnSkinChange();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUploadsHelp();
	afx_msg void OnUploadsSettings();
	afx_msg void OnUpdateUploadsDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnUploadsDisconnect();
	afx_msg void OnUpdateUploadsLaunch(CCmdUI* pCmdUI);
	afx_msg void OnUploadsLaunch();
	afx_msg void OnUpdateUploadsFolder(CCmdUI* pCmdUI);
	afx_msg void OnUploadsFolder();
	afx_msg void OnUpdateUploadsClear(CCmdUI* pCmdUI);
	afx_msg void OnUploadsClear();
	afx_msg void OnUploadsClearCompleted();
	afx_msg void OnUpdateUploadsChat(CCmdUI* pCmdUI);
	afx_msg void OnUploadsChat();
	afx_msg void OnUpdateUploadsAutoClear(CCmdUI* pCmdUI);
	afx_msg void OnUploadsAutoClear();
	afx_msg void OnUpdateSecurityBan(CCmdUI* pCmdUI);
	afx_msg void OnSecurityBan();
	afx_msg void OnUpdateBrowseLaunch(CCmdUI* pCmdUI);
	afx_msg void OnBrowseLaunch();
	afx_msg void OnUpdateUploadsStart(CCmdUI* pCmdUI);
	afx_msg void OnUploadsStart();
	afx_msg void OnUpdateUploadsPriority(CCmdUI* pCmdUI);
	afx_msg void OnUploadsPriority();
	afx_msg void OnUpdateUploadsFilterAll(CCmdUI* pCmdUI);
	afx_msg void OnUploadsFilterAll();
	afx_msg void OnUpdateUploadsFilterActive(CCmdUI* pCmdUI);
	afx_msg void OnUploadsFilterActive();
	afx_msg void OnUpdateUploadsFilterQueued(CCmdUI* pCmdUI);
	afx_msg void OnUploadsFilterQueued();
	afx_msg void OnUpdateUploadsFilterHistory(CCmdUI* pCmdUI);
	afx_msg void OnUploadsFilterHistory();
	afx_msg void OnUploadsFilterMenu();
public:
	afx_msg void OnUpdateUploadsFilterTorrent(CCmdUI *pCmdUI);
	afx_msg void OnUploadsFilterTorrent();

protected:
	DECLARE_MESSAGE_MAP()
};

#define IDC_UPLOADS		100
