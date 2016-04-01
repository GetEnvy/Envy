//
// PageComment.cpp
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
#include "PageComment.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CCommentPage, CWizardPage)

BEGIN_MESSAGE_MAP(CCommentPage, CWizardPage)
	ON_WM_XBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCommentPage property page

CCommentPage::CCommentPage() : CWizardPage(CCommentPage::IDD)
{
}

//CCommentPage::~CCommentPage()
//{
//}

void CCommentPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_COMMENT, m_sComment);
//	DDX_Text(pDX, IDC_SOURCE, m_sSource);
	DDX_Check(pDX, IDC_PRIVATE, m_bPrivate);
}

/////////////////////////////////////////////////////////////////////////////
// CCommentPage message handlers

void CCommentPage::OnReset()
{
}

BOOL CCommentPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	m_sComment = theApp.GetProfileString( L"Comments", L"Comment" );
//	m_sSource = theApp.GetProfileString( L"Comments", L"Source" );
	m_bPrivate = theApp.GetProfileInt( L"Comments", L"Private", FALSE );

	UpdateData( FALSE );

	return TRUE;
}

BOOL CCommentPage::OnSetActive()
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	if ( ! theApp.m_sCommandLineComment.IsEmpty() )
	{
		m_sComment = theApp.m_sCommandLineComment;
		theApp.m_sCommandLineComment.Empty();

		Next();
	}

	UpdateData( FALSE );

	return CWizardPage::OnSetActive();
}

LRESULT CCommentPage::OnWizardBack()
{
	SaveComments();

	return IDD_TRACKER_PAGE;
}

LRESULT CCommentPage::OnWizardNext()
{
	SaveComments();

	return IDD_OUTPUT_PAGE;
}

void CCommentPage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

void CCommentPage::SaveComments()
{
	UpdateData();

	theApp.WriteProfileString( L"Comments", L"Comment", m_sComment );
//	theApp.WriteProfileString( L"Comments", L"Source", m_sSource );
	theApp.WriteProfileInt( L"Comments", L"Private", m_bPrivate );
}
