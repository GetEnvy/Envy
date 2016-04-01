//
// CtrlLibraryTileView.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2007
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

#include "CtrlLibraryView.h"
#include "LibraryFolders.h"


class CLibraryTileItem
{
public:
	CLibraryTileItem(CAlbumFolder* pFolder)
		: m_pAlbum		( pFolder )
		, m_nCookie		( ~0ul )
		, m_nIcon32		( -1 )
		, m_nIcon48		( -1 )
		, m_bSelected	( false )
		, m_bCollection	( false )
	{
		Update();
	}

public:
	bool			m_bSelected;

protected:
	CAlbumFolder*	m_pAlbum;
	DWORD			m_nCookie;
	CString			m_sTitle;
	CString			m_sSubtitle1;
	CString			m_sSubtitle2;
	int				m_nIcon32;
	int				m_nIcon48;
	bool			m_bCollection;

public:
	bool			Update();
	void			Paint(CDC* pDC, const CRect& rcBlock, CDC* pMemDC, BOOL bFocus = FALSE);
	CAlbumFolder*	GetAlbum() const	{ return LibraryFolders.CheckAlbum( m_pAlbum ) ? m_pAlbum : NULL; }
	const CString&	GetTitle() const	{ return m_sTitle; }

protected:
	void			DrawText(CDC* pDC, const CRect* prcClip, int nX, int nY, const CString& strText, CRect* prcUnion = NULL, BOOL bSkinned = FALSE);
};


class CLibraryTileView : public CLibraryView
{
	DECLARE_DYNAMIC(CLibraryTileView)

public:
	CLibraryTileView();

protected:
	typedef std::list< CLibraryTileItem* > PtrList;
	typedef PtrList::iterator iterator;
	typedef PtrList::const_iterator const_iterator;

//	typedef boost::ptr_list< CLibraryTileItem > Container;
//	typedef Container::iterator iterator;
//	typedef Container::const_iterator const_iterator;
//	typedef Container::reverse_iterator reverse_iterator;
//	typedef Container::const_reverse_iterator const_reverse_iterator;

	iterator               begin()        { return m_oList.begin(); }
	const_iterator         begin()  const { return m_oList.begin(); }
	iterator               end()          { return m_oList.end(); }
	const_iterator         end()    const { return m_oList.end(); }
//	reverse_iterator       rbegin()       { return m_oList.rbegin(); }
//	const_reverse_iterator rbegin() const { return m_oList.rbegin(); }
//	reverse_iterator       rend()         { return m_oList.rend(); }
//	const_reverse_iterator rend()   const { return m_oList.rend(); }

	size_t size() const { return m_oList.size(); }
	bool empty() const { return m_oList.empty(); }
	iterator erase(iterator item) { return m_oList.erase( item ); }


	PtrList					m_oList;
	iterator				m_pFocus;
	iterator				m_pFirst;
	std::list< iterator >	m_oSelTile;

	CSize					m_szBlock;
	int 					m_nColumns;
	int 					m_nRows;
	int 					m_nScroll;
	int 					m_nSelected;
	BOOL					m_bDrag;
	CPoint					m_ptDrag;

protected:
	void					clear();
//	int 					GetTileIndex(CLibraryTileItem* pTile) const;
	BOOL					Select(iterator pTile, TRISTATE bSelect = TRI_TRUE);
	BOOL					DeselectAll();
	BOOL					DeselectAll(iterator pTile);
	BOOL					SelectTo(iterator pTile);
	void					SelectTo(int nDelta);
	void					Highlight(iterator pTile);

	virtual BOOL			Create(CWnd* pParentWnd);
	virtual BOOL			CheckAvailable(CLibraryTreeItem* pSel);
	virtual void			Update();
	virtual void			SelectAll();
	virtual BOOL			Select(DWORD nObject);
	virtual CLibraryListItem DropHitTest(const CPoint& point) const;
	virtual HBITMAP			CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle);

	// For std::list sort, note not using boost::ptr_list
	struct SortList : public std::binary_function<CLibraryTileItem, CLibraryTileItem, bool >
	{
		bool operator()(const CLibraryTileItem* lhs, const CLibraryTileItem* rhs) const
		{
			return _tcsicoll( (*lhs).GetTitle(), (*rhs).GetTitle() ) < 0;
		}
	};

	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nDelta);
	iterator			HitTest(const CPoint& point);
	const_iterator		HitTest(const CPoint& point) const;
	virtual DWORD_PTR	HitTestIndex(const CPoint& point) const;
	bool				GetItemRect(iterator pTile, CRect* pRect);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateLibraryAlbumOpen(CCmdUI* pCmdUI);
	afx_msg void OnLibraryAlbumOpen();
	afx_msg void OnUpdateLibraryAlbumDelete(CCmdUI* pCmdUI);
	afx_msg void OnLibraryAlbumDelete();
	afx_msg void OnUpdateLibraryAlbumProperties(CCmdUI* pCmdUI);
	afx_msg void OnLibraryAlbumProperties();
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()
};
