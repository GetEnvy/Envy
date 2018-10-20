//
// CtrlLibraryHeaderPanel.h
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

#include "MetaList.h"

class CAlbumFolder;


class CLibraryHeaderPanel : public CWnd
{
public:
	CLibraryHeaderPanel();
	virtual ~CLibraryHeaderPanel();

protected:
	int			m_nIcon32;
	int			m_nIcon48;
	CString		m_sTitle;
	CString		m_sSubtitle;
	CMetaList	m_pMetadata;
	int			m_nMetaWidth;
	int			m_nKeyWidth;

	CSize		m_szBuffer;
	CDC			m_dcBuffer;
	CBitmap		m_bmBuffer;
	HBITMAP		m_hBuffer;
	CBitmap		m_bmWatermark;

public:
	int				Update();
	void			OnSkinChange();
protected:
	CAlbumFolder*	GetSelectedAlbum() const;
	void			DoPaint(CDC* pDC, CRect& rcClient);
	void			DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText);

public:
	virtual BOOL Create(CWnd* pParentWnd);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};

#define ID_LIBRARY_HEADER	134
