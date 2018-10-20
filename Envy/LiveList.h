//
// LiveList.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2012
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

class CLiveItem;
class CLiveList;
class CLiveListCtrl;


class CLiveItem : public CObject
{
	DECLARE_DYNAMIC( CLiveItem )

public:
	CLiveItem(int nColumns, DWORD_PTR nParam);
	virtual ~CLiveItem();

public:
#ifdef _DEBUG
	virtual void AssertValid() const
	{
		CObject::AssertValid();
		m_nImage.AssertValid();
		m_pColumn.AssertValid();
	}
#endif

	void	Set(int nColumn, const CString& sText);
	void	SetImage(int nColumn, int nImage);
	void	SetImage(int nImage);	// Default column 0
	void	SetMaskOverlay(UINT nMaskOverlay);
	void	Format(int nColumn, LPCTSTR pszFormat, ...);
	int		Add(CListCtrl* pCtrl, int nItem, int nColumns);
	BOOL	Update(CListCtrl* pCtrl, int nItem, int nColumns);
//	BOOL	SetImage(CListCtrl* pCtrl, LPARAM nParam, int nColumn, int nImageIndex);

public:
	CArray< CString >	m_pColumn;	// Text for each column
	CArray< int >		m_nImage;	// Image for each column (-1 = no image, 0+ = index)
	DWORD_PTR	m_nParam;
	UINT		m_nMaskOverlay;
	UINT		m_nMaskState;
	UINT		m_nModified;		// Modified columns (bitmask)
	bool		m_bModified;		// Is data modified?
	bool		m_bOld;				// Is item old? (marked for deletion)

	friend class CLiveList;
	friend class CLiveListCtrl;
};

typedef CLiveItem* CLiveItemPtr;


class CLiveList : public CObject
{
	DECLARE_DYNAMIC( CLiveList )

public:
	CLiveList(int nColumns, UINT nHash = 0);
	virtual ~CLiveList();

public:
	static CBitmap		m_bmSortAsc;
	static CBitmap		m_bmSortDesc;

#ifdef _DEBUG
	virtual void		AssertValid() const
	{
		CObject::AssertValid();
		ASSERT( m_nColumns > 0 && m_nColumns < 100 );
		ASSERT_VALID( &m_pItems );
		ASSERT_VALID( &m_pSection );
		ASSERT_VALID( &m_bmSortAsc );
		ASSERT_VALID( &m_bmSortDesc );
	}
#endif

protected:
	typedef CMap< DWORD_PTR, const DWORD_PTR&, CLiveItem*, CLiveItem*& > CLiveItemMap;

	CLiveItemMap		m_pItems;
	CCriticalSection	m_pSection;
	int					m_nColumns;

public:
	CLiveItem*			Add(DWORD_PTR nParam);
	CLiveItem*			Add(LPVOID pParam);
	void				Apply(CListCtrl* pCtrl, BOOL bSort = FALSE);

// Sort Helpers
	static void			Sort(CListCtrl* pCtrl, int nColumn = -1, BOOL bGraphic = TRUE);
	static int			SortProc(LPCTSTR sA, LPCTSTR sB, BOOL bNumeric = FALSE);
	static int CALLBACK	SortCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static bool			Less(const CLiveItemPtr& _Left, const CLiveItemPtr& _Right, int nSortColumn);
	static inline BOOL	IsNumber(LPCTSTR pszString);

// Drag Helpers
	static HBITMAP		CreateDragImage(CListCtrl* pList, const CPoint& ptMouse, CPoint& ptMiddle);
	static CImageList*	CreateDragImage(CListCtrl* pList, const CPoint& ptMouse);

protected:
	void				Clear();
};


// Prior to Win98/IE4 (Very Obsolete):

//#ifndef CDRF_NOTIFYSUBITEMDRAW
//
//#define CDRF_NOTIFYSUBITEMDRAW 0x00000020
//#define CDDS_SUBITEM			0x00020000
//
//#define LVS_EX_DOUBLEBUFFER 	0x00010000
//#define LVS_EX_NOHSCROLL		0x10000000
//#define LVS_EX_FLATSB			0x00000100
//#define LVS_EX_REGIONAL		0x00000200
//#define LVS_EX_INFOTIP		0x00000400
//#define LVS_EX_LABELTIP		0x00004000
//#define LVS_EX_UNDERLINEHOT	0x00000800
//#define LVS_EX_UNDERLINECOLD	0x00001000
//#define LVS_EX_MULTIWORKAREAS	0x00002000
//
//#define LVM_GETSUBITEMRECT      (LVM_FIRST + 56)
//#define ListView_GetSubItemRect(hwnd, iItem, iSubItem, code, prc) \
//	(BOOL)SNDMSG((hwnd), LVM_GETSUBITEMRECT, (WPARAM)(int)(iItem),  \
//	((prc) ? ((((LPRECT)(prc))->top = iSubItem), (((LPRECT)(prc))->left = code), (LPARAM)(prc)) : (LPARAM)(LPRECT)NULL))
//
//#endif


class CLiveListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CLiveListCtrl)

public:
	CLiveListCtrl();
	virtual ~CLiveListCtrl();

protected:
	typedef std::map< DWORD_PTR, CLiveItemPtr >		CLiveMap;
	typedef std::pair< DWORD_PTR, CLiveItemPtr >	CLiveMapPair;
	typedef std::vector< CLiveItemPtr >				CLiveIndex;

	CLiveMap		m_pItems;
	CLiveIndex		m_pIndex;
	int				m_nColumns;

public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, int nColumns);

	void Apply();
	void Sort(int nColumn = -1);
	void ClearSelection();

	CLiveItemPtr Add(DWORD_PTR nParam);
	CLiveItemPtr Add(LPVOID pParam);

	DWORD_PTR GetItemData(int nItem) const;
	UINT GetItemOverlayMask(int nItem) const;

protected:
	void OnLvnGetDispInfo(NMHDR *pNMHDR, BOOL bWide = TRUE);
	afx_msg void OnLvnGetDispInfoW(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetDispInfoA(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnOdFindItem(NMHDR *pNMHDR, LRESULT *pResult);		// OnLvnOdFindItemW/OnLvnOdFindItemA
	afx_msg void OnLvnOdCacheHint(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};
