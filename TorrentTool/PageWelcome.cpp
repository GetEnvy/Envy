//
// PageWelcome.cpp
//
// This file is part of Envy Torrent Tool (getenvy.com) © 2016
// Portions copyright PeerProject 2008,2012-2014 and Shareaza 2007
//
// Envy Torrent Tool is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Torrent Tool is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "TorrentTool.h"
#include "PageWelcome.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CWelcomePage, CWizardPage)

BEGIN_MESSAGE_MAP(CWelcomePage, CWizardPage)
	//ON_BN_CLICKED(IDC_EXPERT_MODE, OnExpertMode)
	ON_WM_XBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWelcomePage property page

CWelcomePage::CWelcomePage() : CWizardPage(CWelcomePage::IDD)
{
	m_nType = theApp.GetProfileInt( L"", L"Mode", 0 );
}

//CWelcomePage::~CWelcomePage()
//{
//}

void CWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_TYPE_SINGLE, m_nType);
}

/////////////////////////////////////////////////////////////////////////////
// CWelcomePage message handlers

void CWelcomePage::OnReset()
{
	m_nType = theApp.GetProfileInt( L"", L"Mode", 0 );
	UpdateData( FALSE );
}

BOOL CWelcomePage::OnSetActive()
{
	SetWizardButtons( PSWIZB_NEXT );

//	GetSheet()->GetDlgItem( 2 )->EnableWindow( TRUE );

	if ( ! theApp.m_sCommandLineSourceFile.IsEmpty() && m_nType != 2 )
	{
		m_nType = PathIsDirectory( theApp.m_sCommandLineSourceFile ) ? 1 : 0;
		Next();
	}

	UpdateData( FALSE );

	if ( m_nType == 2 && theApp.GetProfileInt( L"", L"Expert", FALSE ) == TRUE )
		Next();

	return CWizardPage::OnSetActive();
}

void CWelcomePage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

LRESULT CWelcomePage::OnWizardNext()
{
	UpdateData();

	theApp.WriteProfileInt( L"", L"Mode", m_nType );
	if ( m_nType == 2 )
		theApp.WriteProfileInt( L"", L"Expert", TRUE );

	if ( m_nType == 0 )
		return IDD_SINGLE_PAGE;
	if ( m_nType == 1 )
		return IDD_PACKAGE_PAGE;
	if ( m_nType == 2 )
		return IDD_EXPERT_PAGE;

	return -1;
}
