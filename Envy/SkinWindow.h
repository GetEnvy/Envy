//
// SkinWindow.h
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

class CXMLElement;


class CSkinWindow
{
public:
	CSkinWindow();
	~CSkinWindow();

public:
	typedef CMap< CString, const CString&, CRect*, CRect* > CRectMap;

	CDC			m_dcSkin;
	CBitmap		m_bmSkin;
	CBitmap		m_bmWatermark;
	CRectMap	m_pAnchorList;	// Typedef above
	CString		m_sTargets;
	CString		m_sLanguage;
	CFont		m_fnCaption;
	COLORREF	m_crCaptionText;
	COLORREF	m_crCaptionInactive;
	COLORREF	m_crCaptionShadow;
	COLORREF	m_crCaptionOutline;

	BOOL		Parse(CXMLElement* pXML, const CString& strPath);
	void		Prepare(CDC* pDC);
	void		CalcWindowRect(RECT* pRect, BOOL bToClient = FALSE, BOOL bZoomed = FALSE);
	BOOL		GetPart(LPCTSTR pszName, CRect& rcPart);
	BOOL		GetAnchor(LPCTSTR pszName, CRect& rcAnchor);
	BOOL		GetAnchor(LPCTSTR pszName, const CRect& rcClient, CRect& rcAnchor);
	BOOL		PaintPartOnAnchor(CDC* pDC, const CRect& rcClient, LPCTSTR pszPart, LPCTSTR pszAnchor);
	BOOL		PreBlend(CBitmap* pbmTarget, const CRect& rcTarget, const CRect& rcSource);
	void		OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	void		OnSetText(CWnd* pWnd);
	void		OnSize(CWnd* pWnd);
	BOOL		OnEraseBkgnd(CWnd* pWnd, CDC* pDC);
	void		OnNcCalcSize(CWnd* pWnd, BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	UINT		OnNcHitTest(CWnd* pWnd, CPoint point, BOOL bResizable = FALSE);
	BOOL		OnNcActivate(CWnd* pWnd, BOOL bActive);
	void		OnNcPaint(CWnd* pWnd);
	void		OnNcMouseLeave(CWnd* pWnd);
	void		OnNcMouseMove(CWnd* pWnd, UINT nHitTest, CPoint point);
	BOOL		OnNcLButtonDown(CWnd* pWnd, UINT nHitTest, CPoint point);
	BOOL		OnNcLButtonUp(CWnd* pWnd, UINT nHitTest, CPoint point);
	BOOL		OnNcLButtonDblClk(CWnd* pWnd, UINT nHitTest, CPoint point);

protected:
	BOOL		m_bLoaded;		// Workaround: destructor has not been called
	HBITMAP		m_hoSkin;
	CBitmap		m_bmAlpha;
	CRectMap	m_pPartList;	// Typedef above
	BOOL*		m_bPart;
	int*		m_nPart;
	CRect*		m_rcPart;
	CRect*		m_rcAnchor;
	BOOL*		m_bAnchor;
	CSize		m_szMinSize;
	CRect		m_rcMaximise;
	CRect		m_rcResize;
	BOOL		m_bCaption;
	BOOL		m_bCaptionCaps;
	CRect		m_rcCaption;
	int			m_nCaptionAlign;
	int			m_nHoverAnchor;
	int			m_nDownAnchor;
	int			m_nMirror;
	CRect		m_rcMirror;
	CXMLElement* m_pRegionXML;

	CSize		GetRegionSize();
	void		Paint(CWnd* pWnd, TRISTATE bActive = TRI_UNKNOWN);
	BOOL		ParseRect(const CXMLElement* pXML, CRect* pRect);
	void		ResolveAnchor(const CRect& rcClient, CRect& rcAnchor, int nAnchor);
	void		SelectRegion(CWnd* pWnd);
};

enum
{
	SKINPART_TOP_LEFT, SKINPART_TOP, SKINPART_TOP_RIGHT,
	SKINPART_LEFT_TOP, SKINPART_LEFT, SKINPART_LEFT_BOTTOM,
	SKINPART_RIGHT_TOP, SKINPART_RIGHT, SKINPART_RIGHT_BOTTOM,
	SKINPART_BOTTOM_LEFT, SKINPART_BOTTOM, SKINPART_BOTTOM_RIGHT,
	SKINPART_TOP_LEFT_ALT, SKINPART_TOP_ALT, SKINPART_TOP_RIGHT_ALT,
	SKINPART_LEFT_TOP_ALT, SKINPART_LEFT_ALT, SKINPART_LEFT_BOTTOM_ALT,
	SKINPART_RIGHT_TOP_ALT, SKINPART_RIGHT_ALT, SKINPART_RIGHT_BOTTOM_ALT,
	SKINPART_BOTTOM_LEFT_ALT, SKINPART_BOTTOM_ALT, SKINPART_BOTTOM_RIGHT_ALT,

	SKINPART_MENU, SKINPART_MENU_HOT, SKINPART_MENU_DOWN,
	SKINPART_SYSTEM, SKINPART_SYSTEM_HOT, SKINPART_SYSTEM_DOWN,
	SKINPART_MINIMISE, SKINPART_MINIMISE_HOT, SKINPART_MINIMISE_DOWN,
	SKINPART_MAXIMISE, SKINPART_MAXIMISE_HOT, SKINPART_MAXIMISE_DOWN,
	SKINPART_CLOSE, SKINPART_CLOSE_HOT, SKINPART_CLOSE_DOWN,
//	SKINPART_CUSTOM, SKINPART_CUSTOM_HOT, SKINPART_CUSTOM_DOWN,

	SKINPART_COUNT
};

enum
{
	SKINANCHOR_ICON, SKINANCHOR_MENU, SKINANCHOR_SYSTEM,
	SKINANCHOR_MINIMISE, SKINANCHOR_MAXIMISE, SKINANCHOR_CLOSE, //SKINANCHOR_CUSTOM,

	SKINANCHOR_COUNT
};

enum
{
	SKINPARTMODE_TILE, SKINPARTMODE_STRETCH
};
