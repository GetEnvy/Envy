//
// CtrlLibraryHistoryPanel.h
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

#include "SharedFile.h"
#include "CtrlPanel.h"
#include "CtrlLibraryTip.h"

class CLibraryHistoryPanel : public CPanelCtrl
{
	DECLARE_DYNAMIC(CLibraryHistoryPanel)

public:
	CLibraryHistoryPanel();
	virtual ~CLibraryHistoryPanel();

public:
	virtual void Update();

protected:
	class Item
	{
	public:
		inline Item() throw() :
			m_pRecent( NULL ),
			m_nIndex( 0 ),
			m_nIcon16( 0 ) { ZeroMemory( &m_pTime, sizeof( m_pTime ) ); }

		CLibraryRecent*	m_pRecent;
		DWORD			m_nIndex;
		SYSTEMTIME		m_pTime;
		CString			m_sText;
		CString			m_sTime;
		int				m_nIcon16;
		CRect			m_rect;
	};

	CArray< Item* >	m_pList;
	Item*			m_pHover;
	int				m_nIndex;
	int				m_nColumns;
	CLibraryTipCtrl	m_wndTip;

	int			 GetIndex(CPoint point);
	void		 OnClickFile(DWORD nFile);

	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateLibraryClear(CCmdUI* pCmdUI);
	afx_msg void OnLibraryClear();
	afx_msg void OnLibraryClearAll();

	DECLARE_MESSAGE_MAP()
};
