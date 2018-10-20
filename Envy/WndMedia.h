//
// WndMedia.h
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
#include "CtrlMediaFrame.h"
#include "EnvyDataSource.h"


class CMediaWnd : public CPanelWnd
{
	DECLARE_SERIAL(CMediaWnd)

public:
	CMediaWnd();
	virtual ~CMediaWnd();

protected:
	CMediaFrame	m_wndFrame;

public:
	static CMediaWnd* GetMediaWindow(BOOL bToggle = FALSE, BOOL bFocus = TRUE);		// Open Media Player window

	virtual BOOL PlayFile(LPCTSTR pszFile);
	virtual BOOL EnqueueFile(LPCTSTR pszFile);
	virtual void OnFileDelete(LPCTSTR pszFile);
	virtual BOOL IsPlaying();

public:
	virtual void OnSkinChange();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnNcActivate(BOOL bActive);

	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMediaKey(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDevModeChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEnqueueFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPlayFile(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	DECLARE_DROP()
};
