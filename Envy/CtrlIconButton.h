//
// CtrlIconButton.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2010
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

class CIconButtonCtrl : public CWnd
{
	DECLARE_DYNAMIC(CIconButtonCtrl)

public:
	CIconButtonCtrl();

	void	SetText(LPCTSTR pszText);
	void	SetIcon(HICON hIcon, BOOL bMirrored = FALSE);
	void	SetIcon(UINT nIconID, BOOL bMirrored = FALSE);
	void	SetCoolIcon(UINT nIconID, BOOL bMirrored = FALSE);
	void	SetHandCursor(BOOL bCursor);

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nControlID, DWORD dwStyle = 0);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	CImageList	m_pImageList;
	BOOL		m_bCursor;
	BOOL		m_bCapture;
	BOOL		m_bDown;

	BOOL		RemoveStyle();

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()
};
