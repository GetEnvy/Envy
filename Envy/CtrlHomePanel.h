//
// CtrlHomePanel.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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

#include "CtrlRichTaskBox.h"
#include "CtrlDownloadTip.h"
#include "CtrlLibraryTip.h"
#include "Neighbour.h"	// ntLast enum

class CDownload;
class CLibraryRecent;


class CHomeConnectionBox : public CRichTaskBox
{
	DECLARE_DYNAMIC(CHomeConnectionBox)

public:
	CHomeConnectionBox();
	virtual ~CHomeConnectionBox();

protected:
	CRichElement*	m_pdConnectedHours;
	CRichElement*	m_pdConnectedMinutes;
	CRichElement*	m_pdCount[PROTOCOL_LAST][ntLast];
	CString			m_sCount[PROTOCOL_LAST][ntLast];

public:
	void	OnSkinChange();
	void	Update();

protected:
	DECLARE_MESSAGE_MAP()
};


class CHomeLibraryBox : public CRichTaskBox
{
	DECLARE_DYNAMIC(CHomeLibraryBox)

public:
	CHomeLibraryBox();
	virtual ~CHomeLibraryBox();

protected:
	class Item
	{
	public:
		inline Item() throw() :
			m_pRecent( NULL ),
			m_nIndex( 0 ),
			m_nIcon16( 0 ) {}

		CLibraryRecent*	m_pRecent;
		DWORD			m_nIndex;
		CString			m_sText;
		int				m_nIcon16;
	};

	CRichElement*	m_pdLibraryFiles;
	CRichElement*	m_pdLibraryVolume;
	CRichElement*	m_pdLibraryHashRemaining;
	CArray< Item* >	m_pList;
	CFont			m_pFont;
	Item*			m_pHover;
	HCURSOR			m_hHand;
	CLibraryTipCtrl	m_wndTip;

public:
	int		m_nIndex;	// Context menu

	void	OnSkinChange();
	void	Update();
	Item*	HitTest(const CPoint& point) const;

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	DECLARE_MESSAGE_MAP()
};


class CHomeDownloadsBox : public CRichTaskBox
{
	DECLARE_DYNAMIC(CHomeDownloadsBox)

public:
	CHomeDownloadsBox();
	virtual ~CHomeDownloadsBox();

protected:
	class Item
	{
	public:
		inline Item () throw() :
			m_pDownload( NULL ),
			m_nIcon16( 0 ),
			m_nSize( 0 ),
			m_nComplete( 0 ),
			m_bPaused( FALSE ) {}

		CDownload*	m_pDownload;
		CString		m_sText;
		int			m_nIcon16;
		QWORD		m_nSize;
		QWORD		m_nComplete;
		BOOL		m_bPaused;
	};

	CRichElement*		m_pdDownloadsNone;
	CRichElement*		m_pdDownloadsOne;
	CRichElement*		m_pdDownloadsMany;
	CRichElement*		m_pdDownloadedNone;
	CRichElement*		m_pdDownloadedOne;
	CRichElement*		m_pdDownloadedMany;
	CRichElement*		m_pdDownloadedVolume;
	CString				m_sDownloadedMany;
	CString				m_sDownloadsMany;
	CArray< Item* >		m_pList;
	CFont				m_pFont;
	Item*				m_pHover;
	HCURSOR				m_hHand;
	CDownloadTipCtrl	m_wndTip;

public:
	void	OnSkinChange();
	void	Update();
	Item*	HitTest(const CPoint& point) const;
	BOOL	ExecuteDownload(CDownload* pDownload);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};


class CHomeUploadsBox : public CRichTaskBox
{
	DECLARE_DYNAMIC(CHomeUploadsBox)

public:
	CHomeUploadsBox();
	virtual ~CHomeUploadsBox();

protected:
	CRichElement*	m_pdUploadsNone;
	CRichElement*	m_pdUploadsOne;
	CRichElement*	m_pdUploadsMany;
	CRichElement*	m_pdUploadedNone;
	CRichElement*	m_pdUploadedOne;
	CRichElement*	m_pdUploadedMany;
	CRichElement*	m_pdTorrentsOne;
	CRichElement*	m_pdTorrentsMany;
	CString			m_sUploadsMany;
	CString			m_sUploadedOne;
	CString			m_sUploadedMany;
	CString			m_sTorrentsMany;

public:
	void	OnSkinChange();
	void	Update();

protected:
	DECLARE_MESSAGE_MAP()
};


class CHomePanel : public CTaskPanel
{
	DECLARE_DYNAMIC(CHomePanel)

public:
	CHomePanel();

public:
	CHomeConnectionBox	m_boxConnection;
	CHomeLibraryBox		m_boxLibrary;
	CHomeDownloadsBox	m_boxDownloads;
	CHomeUploadsBox		m_boxUploads;

	void	OnSkinChange();
	void	Update();

	virtual BOOL Create(CWnd* pParentWnd);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#define IDC_HOME_PANEL	111
