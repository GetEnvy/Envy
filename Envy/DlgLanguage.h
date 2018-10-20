//
// DlgLanguage.h
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

#include "DlgSkinDialog.h"


class CLanguageDlg : public CSkinDialog
{
public:
	CLanguageDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_LANGUAGE };

	CString		m_sLanguage;
	bool		m_bLanguageRTL;

protected:
	CArray< CString > m_pPaths;
	CArray< CString > m_pTitles;
	CArray< CString > m_pGUIDirs;
	CArray< CString > m_pLangCodes;
	CArray< UINT > m_pPriorities;
	CImageList	m_pImages;
	int			m_nHover;
	int			m_nDown;
	BOOL		m_bKeyMode;

	CFont		m_fntNormal;
	CFont		m_fntBold;
	CFont		m_fntSmall;
	HCURSOR		m_hArrow;
	HCURSOR		m_hHand;

protected:
	void		PaintItem(int nItem, CDC* pDC, CRect* pRect);
	void		Execute(int nItem);
	void		Enumerate(LPCTSTR pszPath = NULL);
	BOOL		AddSkin(LPCTSTR pszPath, LPCTSTR pszName);
	void		AddEnglishDefault();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()
};
