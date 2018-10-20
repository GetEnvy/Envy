//
// KeywordTestDlg.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2011 and PeerProject 2012
//
// Envy is free software; you can redistribute it and/or
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

#include "StdAfx.h"
#include "KeywordTest.h"
#include "KeywordTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CKeywordTestDlg::CKeywordTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog( CKeywordTestDlg::IDD, pParent )
	, m_bExp( FALSE )
	, m_sInput( _T("Alphas.s01e01.Pilot.HDTVRip.Rus.Eng.[Gravi-TV] proper.avi") )
	, m_hIcon( AfxGetApp()->LoadIcon( IDR_MAINFRAME ) )
{
}

void CKeywordTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_INPUT, m_sInput);
	DDX_Control(pDX, IDC_RESULTS, m_pResults);
	DDX_Text(pDX, IDC_SPLITTED, m_sSplitted);
	DDX_Check(pDX, IDC_EXP, m_bExp);
}

BEGIN_MESSAGE_MAP(CKeywordTestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_INPUT, &CKeywordTestDlg::OnEnChangeInput)
	ON_BN_CLICKED(IDC_EXP, &CKeywordTestDlg::OnBnClickedExp)
END_MESSAGE_MAP()

BOOL CKeywordTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon( m_hIcon, TRUE );			// Set big icon
	SetIcon( m_hIcon, FALSE );		// Set small icon

	UpdateData( FALSE );

	OnOK();

	return TRUE;	// return TRUE unless you set the focus to a control
}

void CKeywordTestDlg::OnPaint()
{
	if ( IsIconic() )
	{
		CPaintDC dc( this );	// Device context for painting

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0 );

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon( x, y, m_hIcon );
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CKeywordTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKeywordTestDlg::OnEnChangeInput()
{
	OnOK();
}

void CKeywordTestDlg::OnBnClickedExp()
{
	OnOK();
}

void CKeywordTestDlg::OnOK()
{
	CWaitCursor wc;

	if ( ! UpdateData() )
		return;

	m_sSplitted = MakeKeywords( m_sInput, m_bExp != FALSE );

	WordTable oWords, oNegWords;
	BuildWordTable( m_sSplitted, oWords, oNegWords );

	m_pResults.ResetContent();

	for ( WordTable::const_iterator i = oWords.begin(); i != oWords.end(); ++i )
	{
		CString sWord( (*i).first, (*i).second );
		m_pResults.AddString( sWord + _T(" (+)") );
	}

	for ( WordTable::const_iterator i = oNegWords.begin(); i != oNegWords.end(); ++i )
	{
		CString sNegWord( (*i).first, (*i).second );
		m_pResults.AddString( sNegWord + _T(" (-)") );
	}

	UpdateData( FALSE );
}
