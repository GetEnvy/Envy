//
// DlgAbout.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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

#include "StdAfx.h"
#include "Envy.h"
#include "DlgAbout.h"
#include "Colors.h"
#include "CoolInterface.h"
//#include "Revision.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CAboutDlg, CSkinDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog

CAboutDlg::CAboutDlg(CWnd* pParent) : CSkinDialog(CAboutDlg::IDD, pParent)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WEB, m_wndWeb);
	DDX_Control(pDX, IDC_TITLE, m_wndTitle);
	DDX_Control(pDX, IDC_LOGO, m_wndLogo);
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CAboutDlg", IDR_MAINFRAME );

	m_wndLogo.SetBitmap( Skin.LoadBitmap( IDR_LOGO ) );

	CString strCaption;
	GetWindowText( strCaption );
	strCaption += L" v";
	strCaption += theApp.m_sVersion;
	SetWindowText( strCaption );
	m_wndTitle.SetWindowText( theApp.m_sVersionLong );		// Envy 1.x.x.x 32/64-bit (date rXXXX) Debug

	DWORD dwSize = GetFileVersionInfoSize( theApp.m_strBinaryPath, &dwSize );
	BYTE* pBuffer = new BYTE[ dwSize ];
	GetFileVersionInfo( theApp.m_strBinaryPath, NULL, dwSize, pBuffer );

	CWnd* pWnd = GetDlgItem( IDC_COPYRIGHT );
	BYTE* pValue = NULL;

	if ( VerQueryValue( pBuffer, L"\\StringFileInfo\\000004b0\\LegalCopyright", (void**)&pValue, (UINT*)&dwSize ) )
		pWnd->SetWindowText( (LPCTSTR)pValue );				// Substitute manifest info

	delete [] pBuffer;

	return TRUE;
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = (HBRUSH)CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndTitle )
	{
		pDC->SelectObject( &CoolInterface.m_fntRichDefault );		// Bold size+1 CoolInterface.m_fntBold theApp.m_gdiFontBold
	}
	else if ( pWnd == &m_wndWeb )
	{
		pDC->SetTextColor( Colors.m_crTextLink );
		pDC->SelectObject( &CoolInterface.m_fntUnder );
	}

	return hbr;
}

BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
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

	return CSkinDialog::OnSetCursor( pWnd, nHitTest, message );
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CSkinDialog::OnLButtonUp( nFlags, point );

	CRect rc;
	m_wndWeb.GetWindowRect( &rc );
	ScreenToClient( &rc );

	if ( rc.PtInRect( point ) )
	{
		ShellExecute( GetSafeHwnd(), L"open",
			CString( WEB_SITE ) + L"?Version=" + theApp.m_sVersion,
			NULL, NULL, SW_SHOWNORMAL );
	}
}

void CAboutDlg::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	// Shift+Rightclick on link for BugTrap crash testing

//#ifdef _DEBUG
	if ( ! ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) )
		return;

	CRect rc;
	m_wndWeb.GetWindowRect( &rc );
	ScreenToClient( &rc );
	if ( ! rc.PtInRect( point ) )
		return;

#ifndef _DEBUG
	if ( MsgBox( L"\nDo you wish to trigger a program crash?", MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
		return;
#endif

	volatile DWORD* pNullPtr = (DWORD*)NULL;
	*pNullPtr = 0xFFFFFFFF;  //-V522		// Force program crash
//#endif
}
