//
// WndLibrary.h
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

#include "WndPanel.h"
#include "CtrlLibraryFrame.h"

class CLibraryFile;
class CAlbumFolder;


class CLibraryWnd : public CPanelWnd
{
	DECLARE_SERIAL(CLibraryWnd)

public:
	CLibraryWnd();
	virtual ~CLibraryWnd();

public:
	CLibraryFrame	m_wndFrame;
	DWORD			m_tLast;

public:
	static CLibraryWnd*	GetLibraryWindow(BOOL bToggle = FALSE, BOOL bFocus = TRUE);		// Open Library window

	BOOL	Display(const CLibraryFile* pFile);
	BOOL	Display(const CAlbumFolder* pFolder);
	BOOL	OnCollection(LPCTSTR pszPath);

public:
	virtual HRESULT	GetGenericView(IGenericView** ppView);
	virtual void OnSkinChange();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);

	DECLARE_MESSAGE_MAP()
};
