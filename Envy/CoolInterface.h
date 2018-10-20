//
// CoolInterface.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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

#include "Skin.h"

#define LVSIL_MID		24	// 24x24 icons
#define LVSIL_BIG		48	// 48x48 icons

class CCoolInterface
{
public:
	CCoolInterface();
	virtual ~CCoolInterface();

public:
	CFont		m_fntNormal;		// system.plain
	CFont		m_fntBold;			// system.bold
	CFont		m_fntItalic;		// system.plain + italic
	CFont		m_fntBoldItalic;	// system.bold + italic
	CFont		m_fntUnder;			// system.plain + underline
	CFont		m_fntCaption;		// panel.caption
	CFont		m_fntNavBar;		// navbar.caption
	CFont		m_fntNavBarActive;	// navbar.selected
	CFont		m_fntNavBarHover;	// navbar.hover
	CFont		m_fntRichDefault;	// richdoc.default
	CFont		m_fntRichHeading;	// richdoc.heading

	void		Load();
	void		Clear();
	void		NameCommand(UINT nID, LPCTSTR pszName);
	UINT		NameToID(LPCTSTR pszName) const;
	int			ImageForID(UINT nID, int nImageListType = LVSIL_SMALL) const;
	void		AddIcon(UINT nID, HICON hIcon, int nImageListType = LVSIL_SMALL);
	void		CopyIcon(UINT nFromID, UINT nToID, int nImageListType = LVSIL_SMALL);
	HICON		ExtractIcon(UINT nID, BOOL bMirrored = FALSE, int nImageListType = LVSIL_SMALL, int nSizeX = 0, int nSizeY = 0 );
	int			ExtractIconID(UINT nID, BOOL bMirrored = FALSE, int nImageListType = LVSIL_SMALL);
	// Set skinned icon to window: pWnd->SetIcon( hIcon, bBigIcon )
	void		SetIcon(UINT nID, BOOL bMirrored, BOOL bBigIcon, CWnd* pWnd);
	void		SetIcon(HICON hIcon, BOOL bMirrored, BOOL bBigIcon, CWnd* pWnd);
	int			GetImageCount(int nImageListType = LVSIL_SMALL);
	// Assign image list to CListCtrl object. Returns old image list of CListCtrl object.
	CImageList*	SetImageListTo(CListCtrl& pWnd, int nImageListType = LVSIL_SMALL);
	// Loads skinable icons specified by ID array to CImageList object
	void		LoadIconsTo(CImageList& pImageList, const UINT nID[], BOOL bMirror = FALSE, int nImageListType = LVSIL_SMALL, int nSizeX = 0, int nSizeY = 0);
	void		LoadFlagsTo(CImageList& pImageList);
	// No void	LoadProtocolIconsTo(), use LoadIconsTo( pImageList, protocolIDs );
	//BOOL		AddImagesFromToolbar(UINT nIDToolBar, COLORREF crBack = RGB(0,255,0));
	BOOL		Add(CSkin* pSkin, CXMLElement* pBase, HBITMAP hbmImage, COLORREF crMask, int nImageListType = LVSIL_SMALL);
	BOOL		Draw(CDC* pDC, UINT nID, int nSize, int nX, int nY, COLORREF crBack = CLR_NONE, BOOL bSelected = FALSE, BOOL bExclude = TRUE) const;
	BOOL		Draw(CDC* pDC, int nImage, POINT pt, UINT nStyle = ILD_NORMAL, int nImageListType = LVSIL_SMALL) const;
	BOOL		DrawEx(CDC* pDC, int nImage, POINT pt, SIZE sz = CSize( 16, 16 ), COLORREF clrBk = CLR_NONE, COLORREF clrFg = CLR_DEFAULT, UINT nStyle = ILD_NORMAL, int nImageListType = LVSIL_SMALL) const;
//	BOOL		DrawIndirect(CDC* pDC, int nImage, POINT pt, SIZE sz = CSize( 16, 16 ), COLORREF clrBk = CLR_NONE, COLORREF clrFg = CLR_DEFAULT, UINT nStyle = ILD_NORMAL, DWORD nState = ILS_ALPHA, DWORD nAlpha = 200, int nImageListType = LVSIL_SMALL) const;
	BOOL		DrawWatermark(CDC* pDC, CRect* pRect, CBitmap* pMark, BOOL bOverdraw = TRUE, int nOffX = 0, int nOffY = 0);
	void		DrawThumbnail(CDC* pDC, const CRect& rcThumb, BOOL bWaiting, BOOL bSelected, CBitmap& bmThumb, int nIcon48 = -1, int nIcon32 = -1, const CString& strLabel = CString());
	void		CreateFonts(LPCTSTR pszFace = NULL, int nSize = 0);
	CDC*		GetBuffer(CDC& dcScreen, const CSize& szItem);

	static BOOL	EnableTheme(CWnd* pWnd, BOOL bEnable = TRUE);
	static void	FixThemeControls(CWnd* pWnd, BOOL bForce = TRUE);		// Checkbox/Groupbox text colors (Remove theme if needed)

protected:
	typedef CMap< UINT, UINT, int, int > CUINTintMap;
	typedef CMap< CString, const CString&, UINT, UINT > CStringUINTMap;
	typedef CMap< HICON, HICON, HWND, HWND > CHICONHWNDMap;

	//mutable CCriticalSection m_pSection;
	CStringUINTMap	m_pNameMap;
	CUINTintMap		m_pImageMap16;		// 16px Small images (LVSIL_SMALL)
	CImageList		m_pImages16;		// 16px Small images (LVSIL_SMALL)
//	CUINTintMap		m_pImageMap24;		// 24px Medium images (LVSIL_MID custom)
//	CImageList		m_pImages24;		// 24px Medium images (LVSIL_MID custom)
	CUINTintMap		m_pImageMap32;		// 32px Normal images (LVSIL_NORMAL)
	CImageList		m_pImages32;		// 32px Normal images (LVSIL_NORMAL)
	CUINTintMap		m_pImageMap48;		// 48px Large images (LVSIL_BIG custom)
	CImageList		m_pImages48;		// 48px Large images (LVSIL_BIG custom)
	CSize			m_czBuffer;
	CDC				m_dcBuffer;
	CBitmap			m_bmBuffer;
	HBITMAP			m_bmOldBuffer;
	CHICONHWNDMap	m_pWindowIcons;

	BOOL			ConfirmImageList();

private:
	CCoolInterface(const CCoolInterface&);
	CCoolInterface& operator=(const CCoolInterface&);
};

extern CCoolInterface CoolInterface;

typedef struct
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	WORD* items() { return (WORD*)(this+1); }
} TOOLBAR_RES;
