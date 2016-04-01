//
// WndChild.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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

#include "CtrlCoolBar.h"
#include "LiveListSizer.h"

class CMainWnd;
class CSkinWindow;
class CWindowManager;

class CBuffer;
class CConnection;
class CLibraryFile;
class CQueryHit;
class CQuerySearch;


class CChildWnd : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildWnd)

public:
	CChildWnd();

public:
	BOOL			m_bTabMode;
	BOOL			m_bGroupMode;
	CChildWnd*		m_pGroupParent;
	float			m_nGroupSize;
	BOOL			m_bPanelMode;
	BOOL			m_bAlert;

private:
	CMainWnd*		m_pMainWndCache;
	static CChildWnd* m_pCmdMsg;
	CString			m_sCaption;

protected:
	CSkinWindow*	m_pSkin;
	UINT			m_nResID;

public:
	CMainWnd*		GetMainWnd();
	CWindowManager*	GetManager();
	void			GetWindowText(CString& rString);
	void			SetWindowText(LPCTSTR lpszString);
	BOOL			IsActive(BOOL bFocused = FALSE);
	BOOL			IsPartiallyVisible();
	BOOL			TestPoint(const CPoint& ptScreen);
	BOOL			LoadState(LPCTSTR pszName = NULL, BOOL bDefaultMaximise = TRUE);
	BOOL			SaveState(LPCTSTR pszName = NULL);
	BOOL			SetAlert(BOOL bAlert = TRUE);
	void			SizeListAndBar(CWnd* pList, CWnd* pBar);
	void			RemoveSkin();

	// Notify window about skin change
	virtual void	OnSkinChange();
	// Notify window about arrived query search
	virtual void	OnQuerySearch(const CQuerySearch* /*pSearch*/) {}
	// Notify window about arrived query hits
	virtual BOOL	OnQueryHits(const CQueryHit* /*pHits*/) { return FALSE; }
	// Notify window about security rules changed
	virtual void	SanityCheck() {}
	// Notify window about new push connection available
	virtual BOOL	OnPush(const Hashes::Guid& /*pClientID*/, CConnection* /*pConnection*/) { return FALSE; }
	// Notify window about new library file (return TRUE to cancel event route)
	virtual BOOL	OnNewFile(CLibraryFile* /*pFile*/) { return FALSE; }

	virtual HRESULT	GetGenericView(IGenericView** ppView);
	virtual BOOL	DestroyWindow();

protected:
	virtual BOOL	Create(UINT nID, BOOL bVisible = TRUE);
	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnDestroy();
	afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void	OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void	OnNcPaint();
	afx_msg void	OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg BOOL	OnNcActivate(BOOL bActive);
	afx_msg LRESULT	OnNcHitTest(CPoint point);
	afx_msg void	OnNcMouseLeave();
	afx_msg void	OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void	OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void	OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void	OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void	OnNcRButtonUp(UINT nHitTest, CPoint point);
	afx_msg BOOL	OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT	OnSetText(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
