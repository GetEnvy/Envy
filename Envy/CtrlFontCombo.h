//
// CtrlFontCombo.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2005-2007
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
//  Created by:		Rolandas
//	Date:			"$Date: 2005/08/08 03:47:40 $"
//	Revision:		"$Revision: 1.1.2.1 $"
//  Last change by:	"$Author: rolandas $"
//

#pragma once

class CFontCombo : public CComboBox
{
	DECLARE_DYNAMIC(CFontCombo)

public:
	CFontCombo();
//	virtual ~CFontCombo();

public:
	CString	m_sSelectedFont;
protected:
	CImageList m_pImages;
	CMapStringToPtr m_pFonts;
	int 	m_nFontHeight;

public:
	void	Initialize();
	void	SelectFont(const CString& strFontName);
	CString	GetSelectedFont() const;
	int 	GetFontHeight() const;
	void	SetFontHeight(int nNewHeight, BOOL bReinitialize = TRUE);

protected:
	BOOL	AddFont(const CString& strFontName);
	void	DeleteAllFonts();
	static BOOL CALLBACK EnumFontProc(LPENUMLOGFONTEX lplf, NEWTEXTMETRICEX* lpntm, DWORD dwFontType, LPVOID lpData);

public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
protected:
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnDropdown();
	afx_msg LRESULT OnOcmDrawItem(WPARAM /*wParam*/, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

void PASCAL DDX_FontCombo(CDataExchange* pDX, int nIDC, CString& strFontName);
