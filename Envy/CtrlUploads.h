//
// CtrlUploads.h
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

#include "CtrlUploadTip.h"

class CUploadFile;
class CUploadQueue;
class CUploadDisplayData;
class CQueueDisplayData;


class CUploadsCtrl : public CWnd
{
	DECLARE_DYNAMIC(CUploadsCtrl)

public:
	CUploadsCtrl();

public:
	BOOL		Create(CWnd* pParentWnd, UINT nID);
	BOOL		Update();
protected:
	void		InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth);
	void		SaveColumnState();
	BOOL		LoadColumnState();
	void		SelectTo(int nIndex);
	void		DeselectAll(CUploadFile* pExcept = NULL);
	BOOL		HitTest(const CPoint& point, CUploadQueue** ppQueue, CUploadFile** ppFile, int* pnIndex, RECT* prcItem);
	BOOL		HitTest(int nIndex, CUploadQueue** ppQueue, CUploadFile** ppFile);
	BOOL		GetAt(int nSelect, CUploadQueue** ppQueue, CUploadFile** ppFile);
	void		PaintQueue(CDC& dc, const CRect& rcRow, const CQueueDisplayData* pQueueData, BOOL bFocus);
	void		PaintFile(CDC& dc, const CRect& rcRow, const CUploadDisplayData* pUploadData, BOOL bFocus);
	void		UpdateUploadsData(BOOL bForce = FALSE);
	int			GetExpandableColumnX() const;
	void		OnSkinChange();

	friend class CUploadsWnd;

protected:
	CHeaderCtrl			m_wndHeader;
	CUploadTipCtrl		m_wndTip;
	CUploadFile*		m_pDeselect;
	CImageList			m_gdiProtocols;
	int					m_nFocus;
	int 				m_nHover;			// Visible index

	CArray< CQueueDisplayData > m_pDisplayData;

// Queue/File Abstractation Layer
public:
	static POSITION			GetQueueIterator();
	static CUploadQueue*	GetNextQueue(POSITION& pos);
	static POSITION			GetFileIterator(CUploadQueue* pQueue);
	static CUploadFile*		GetNextFile(CUploadQueue* pQueue, POSITION& pos, int* pnPosition = NULL);

public:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnChangeHeader(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();

protected:
	DECLARE_MESSAGE_MAP()
};


#define ULF_ACTIVE		0x01
#define ULF_QUEUED		0x02
#define ULF_HISTORY		0x04
#define ULF_TORRENT		0x08

#define ULF_ALL			(ULF_ACTIVE|ULF_QUEUED|ULF_HISTORY|ULF_TORRENT)
