//
// CtrlLibraryHeaderBar.h
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

#include "CtrlCoolBar.h"

class CLibraryView;
class CCoolMenu;


class CLibraryHeaderBar : public CCoolBarCtrl
{
	DECLARE_DYNAMIC(CLibraryHeaderBar)

public:
	CLibraryHeaderBar();
	virtual ~CLibraryHeaderBar();

protected:
	CLibraryView*	m_pLastView;
	int				m_nImage;
	CString			m_sTitle;

	CCoolMenu*		m_pCoolMenu;

public:
	void	Update(CLibraryView* pView);
protected:
	void	PaintHeader(CDC* pDC, CRect& rcBar, BOOL bTransparent);

protected:
	virtual void PrepareRect(CRect* pRect) const;
	virtual void DoPaint(CDC* pDC, CRect& rcBar, BOOL bTransparent);

protected:
	afx_msg void OnLibraryView();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);

	DECLARE_MESSAGE_MAP()
};
