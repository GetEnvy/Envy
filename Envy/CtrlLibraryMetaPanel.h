//
// CtrlLibraryMetaPanel.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2012
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

#include "CtrlPanel.h"


class CSchema;
class CLibraryFile;


class CLibraryMetaPanel :
	public CPanelCtrl,
	public CThreadImpl
{
	DECLARE_DYNCREATE(CLibraryMetaPanel)

public:
	CLibraryMetaPanel();
	virtual ~CLibraryMetaPanel();

public:
	virtual void Update();

	BOOL		SetServicePanel(CMetaList* pPanel);
	CMetaList*	GetServicePanel();

protected:
	int			m_nSelected;
	DWORD		m_nIndex;
	CString		m_sName;
	CString		m_sPath;		// Current file path
	CString		m_sFolder;
	CString		m_sType;
	CString		m_sSize;
	int			m_nIcon32;
	int			m_nIcon48;
	int			m_nRating;
	CSchemaPtr	m_pSchema;
	CMetaList*	m_pMetadata;
	CMetaList*	m_pServiceData;
	CCriticalSection m_pSection;
	CRect		m_rcFolder;
	CRect		m_rcRating;
	BOOL		m_bRedraw;
	BOOL		m_bForceUpdate;
//	CString		m_sThumbnailURL;	// Use this URL to load thumbnail instead
	CString		m_sThumb;			// Loaded thumbnail file path or URL
	CBitmap		m_bmThumb;

	CLibraryList*	GetViewSelection() const;

	void		DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText, RECT* pRect = NULL, int nMaxWidth = -1);

	void		OnRun();

	afx_msg void OnPaint();
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};
