//
// CoolMenu.h
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

#include <shlobj.h>

struct __declspec(uuid("000214f4-0000-0000-c000-000000000046")) IContextMenu2;
struct __declspec(uuid("bcfce0a0-ec17-11d0-8d10-00a0c90f2719")) IContextMenu3;

// Unallocated Resource.h Ranges
#define ID_SCHEMA_MENU_MIN	35000
#define ID_SCHEMA_MENU_MAX	35100
#define ID_SHELL_MENU_MIN	50000
#define ID_SHELL_MENU_MAX	51000

class CCoolMenu
{
public:
	CCoolMenu();
	virtual ~CCoolMenu();

public:
	void		Clear();
	BOOL		AddMenu(CMenu* pMenu, BOOL bChild = FALSE);
	BOOL		ReplaceMenuText(CMenu* pMenu, int nPosition, MENUITEMINFO FAR* mii, LPCTSTR pszText);
	void		SetWatermark(HBITMAP hBitmap);
	void		OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	void		OnMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void		OnDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	LRESULT		OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	void		DoExplorerMenu(HWND hwnd, const CStringList& oFiles, POINT point, HMENU hMenu, HMENU hSubMenu, UINT nFlags);

protected:
	CComPtr< IContextMenu >		m_pContextMenuCache;
	CComPtr< IContextMenu2 >	m_pContextMenu2;
	CComPtr< IContextMenu3 >	m_pContextMenu3;

	void		OnMeasureItemInternal(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void		OnDrawItemInternal(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void		DrawMenuText(CDC* pDC, CRect* pRect, const CString& strText);
	void		DrawWatermark(CDC* pDC, CRect* pRect, int nOffX, int nOffY);

protected:
	// Note: CMap< DWORD_PTR, DWORD_PTR, ...> causes a conversion to DWORD warning within CMap.
	// DWORD_PTR& seems to solve the problem.  ToDo: this should be reinvestigated.
	// A guess: the compiler cannot distinguish between T and __w64 T with respect to overload resolution
	// or template instantiation - in that case a false warning and should be supressed.
	CMap< DWORD_PTR, DWORD_PTR&, CString, CString& >	m_pStrings;

	CBitmap		m_bmWatermark;
	CDC			m_dcWatermark;
	CSize		m_czWatermark;
	HBITMAP		m_hOldMark;

	CString		m_sFilterString;
	CString		m_sOldFilterString;

// Border Hook
public:
	void			EnableHook();
	static void		EnableHook(bool bEnable);
	static void		RegisterEdge(int nLeft, int nTop, int nLength);

protected:
	static LRESULT	CALLBACK MsgHook(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT	CALLBACK MenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static HHOOK	m_hMsgHook;
	static LPCTSTR	wpnOldProc;
	static BOOL		m_bPrinted;
	static int		m_nEdgeLeft;
	static int		m_nEdgeTop;
	static int		m_nEdgeSize;

private:
	CCoolMenu(const CCoolMenu&);
	CCoolMenu& operator=(const CCoolMenu&);
};

extern CCoolMenu CoolMenu;
