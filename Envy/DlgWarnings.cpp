//
// DlgWarnings.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
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

#include "StdAfx.h"
#include "Envy.h"
#include "DlgWarnings.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CWarningsDlg, CSkinDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWarningsDlg dialog

CWarningsDlg::CWarningsDlg(CWnd* pParent) : CSkinDialog(CWarningsDlg::IDD, pParent)
{
}

void CWarningsDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RESPECT, m_wndRespect);
	DDX_Control(pDX, IDC_WEB, m_wndWeb);
	DDX_Control(pDX, IDC_TITLE, m_wndTitle);
}

/////////////////////////////////////////////////////////////////////////////
// CWarningsDlg message handlers

BOOL CWarningsDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CWarningsDlg", IDR_MAINFRAME );

	return TRUE;
}

HBRUSH CWarningsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = (HBRUSH)CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndTitle || pWnd == &m_wndRespect )
	{
		pDC->SelectObject( &theApp.m_gdiFontBold );
	}
	else if ( pWnd == &m_wndWeb )
	{
		pDC->SetTextColor( Colors.m_crTextLink );
		pDC->SelectObject( &theApp.m_gdiFontLine );
	}

	return hbr;
}

BOOL CWarningsDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	CRect rc;

	GetCursorPos( &point );
	m_wndWeb.GetWindowRect( &rc );

	if ( rc.PtInRect( point ) )
	{
		SetCursor( theApp.LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	return CSkinDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CWarningsDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CSkinDialog::OnLButtonUp( nFlags, point );

	CRect rc;
	m_wndWeb.GetWindowRect( &rc );
	ScreenToClient( &rc );

	if ( rc.PtInRect( point ) )
	{
		const CString strWebSite( WEB_SITE );

		ShellExecute( GetSafeHwnd(), L"open",
			strWebSite + L"?Version=" + theApp.m_sVersion,
			NULL, NULL, SW_SHOWNORMAL );
	}
}
