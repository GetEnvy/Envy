//
// WizardSheet.h
//
// This file is part of Torrent Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008-2014
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

class CWizardPage;


class CWizardSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CWizardSheet)

// Construction
public:
	CWizardSheet(CWnd *pParentWnd = NULL, UINT iSelectPage = 0);
	//virtual ~CWizardSheet();

// Attributes
public:
	CRect			m_rcPage;
	CBitmap			m_bmHeader;
	int 			m_nBannerHeight;
	int				m_nMenuHeight;

// Operations
public:
	CWizardPage*	GetPage(CRuntimeClass* pClass);
	void			DoReset();

// Overrides
protected:
	virtual BOOL	OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	virtual BOOL	OnInitDialog();

// Implementation
protected:
	afx_msg void	OnPaint();
//	afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL	OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void	OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void	OnNcLButtonUp( UINT nHitTest, CPoint point );
	afx_msg void	OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};


class CWizardPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardPage)

// Construction
public:
	CWizardPage(UINT nID = 0);
	//virtual ~CWizardPage();

// Attributes
public:
	CBrush			m_brPageColor;	// PAGE_COLOR (Was m_brWhite)

// Operations
public:
	CWizardSheet*	GetSheet();
	CWizardPage*	GetPage(CRuntimeClass* pClass);
	void			SetWizardButtons(DWORD dwFlags);
	void			StaticReplace(LPCTSTR pszSearch, LPCTSTR pszReplace);
	void			Next();

// Implementation
protected:
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnPressButton(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

#define GET_PAGE(gpClass, gpVar)	gpClass * gpVar = ( gpClass * )GetPage( RUNTIME_CLASS( gpClass ) )

#define WM_PRESSBUTTON (WM_APP + 100)
