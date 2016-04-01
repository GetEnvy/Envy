//
// PageTracker.cpp
//
// This file is part of Envy Torrent Tool (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2007
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
#include "PageTracker.h"
#include "PageWelcome.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTrackerPage, CWizardPage)

BEGIN_MESSAGE_MAP(CTrackerPage, CWizardPage)
	ON_BN_CLICKED(IDC_CLEAR_TRACKERS, &CTrackerPage::OnClearTrackers)
	ON_WM_XBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTrackerPage property page

CTrackerPage::CTrackerPage() : CWizardPage(CTrackerPage::IDD)
{
}

//CTrackerPage::~CTrackerPage()
//{
//}

void CTrackerPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRACKER, m_wndTracker);
	DDX_CBString(pDX, IDC_TRACKER, m_sTracker);
	DDX_Control(pDX, IDC_TRACKER2, m_wndTracker2);
	DDX_CBString(pDX, IDC_TRACKER2, m_sTracker2);
}

/////////////////////////////////////////////////////////////////////////////
// CTrackerPage message handlers

BOOL CTrackerPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	int nCount = theApp.GetProfileInt( L"Trackers", L"Count", 0 );

	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CString strName, strURL;
		strName.Format( L"%.3i.URL", nItem + 1 );
		strURL = theApp.GetProfileString( L"Trackers", strName );
		if ( strURL.GetLength() )
		{
			m_wndTracker.AddString( strURL );
			m_wndTracker2.AddString( strURL );
		}
	}

	m_sTracker = theApp.GetProfileString( L"Trackers", L"Last" );
	if ( m_sTracker.GetLength() < 14 )
		m_sTracker = L"udp://tracker.openbittorrent.com:80/announce";
	UpdateData( FALSE );

	return TRUE;
}

BOOL CTrackerPage::OnSetActive()
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	if ( ! theApp.m_sCommandLineTracker.IsEmpty() )
	{
		m_sTracker = theApp.m_sCommandLineTracker;
		theApp.m_sCommandLineTracker.Empty();

		UpdateData( FALSE );

		Next();
	}

	return CWizardPage::OnSetActive();
}

void CTrackerPage::OnClearTrackers()
{
	theApp.WriteProfileInt( L"Trackers", L"Count", 0 );
	m_sTracker.Empty();
	UpdateData( FALSE );
	m_wndTracker.ResetContent();
	m_wndTracker2.ResetContent();
	m_wndTracker.SetFocus();
}

LRESULT CTrackerPage::OnWizardBack()
{
	GET_PAGE( CWelcomePage, pWelcome );

	UpdateData( TRUE );

	SaveTrackers();

	return pWelcome->m_nType ? IDD_PACKAGE_PAGE : IDD_SINGLE_PAGE;
}

LRESULT CTrackerPage::OnWizardNext()
{
	UpdateData( TRUE );

	if ( m_sTracker.GetLength() < 16 ||
		( m_sTracker.Left( 6 ).CompareNoCase( L"udp://" ) != 0 &&
		  m_sTracker.Left( 7 ).CompareNoCase( L"http://" ) != 0 &&
		  m_sTracker.Left( 8 ).CompareNoCase( L"https://" ) != 0 ) )
	{
		if ( IDYES != AfxMessageBox( IDS_TRACKER_NEED_URL, MB_ICONQUESTION|MB_YESNO ) )
		{
			m_wndTracker.SetFocus();
			return -1;
		}
	}

	if ( m_sTracker.Right( 9 ).CompareNoCase( L"/announce" ) != 0 )
	{
		if ( IDYES != AfxMessageBox( IDS_TRACKER_NEED_ANNOUNCE, MB_ICONQUESTION|MB_YESNO ) )
		{
			m_wndTracker.SetFocus();
			return -1;
		}
	}

	SaveTrackers();

	return IDD_COMMENT_PAGE;
}

void CTrackerPage::SaveTrackers()
{
	if ( m_sTracker.GetLength() > 15 && m_wndTracker.FindStringExact( -1, m_sTracker ) < 0 )
	{
		m_wndTracker.AddString( m_sTracker );	// Populate Combo-box
		m_wndTracker2.AddString( m_sTracker );

		CString strName;
		int nCount = theApp.GetProfileInt( L"Trackers", L"Count", 0 );
		strName.Format( L"%.3i.URL", ++nCount );
		theApp.WriteProfileInt( L"Trackers", L"Count", nCount );
		theApp.WriteProfileString( L"Trackers", strName, m_sTracker );
	}

	if ( m_sTracker2.GetLength() > 15 && m_sTracker2.Find( L"://" ) > 0 && m_wndTracker2.FindStringExact( -1, m_sTracker2 ) < 0 )
	{
		m_wndTracker.AddString( m_sTracker2 );
		m_wndTracker2.AddString( m_sTracker2 );

		CString strName;
		int nCount = theApp.GetProfileInt( L"Trackers", L"Count", 0 );
		strName.Format( L"%.3i.URL", ++nCount );
		theApp.WriteProfileInt( L"Trackers", L"Count", nCount );
		theApp.WriteProfileString( L"Trackers", strName, m_sTracker );
	}

	theApp.WriteProfileString( L"Trackers", L"Last", m_sTracker );
}

void CTrackerPage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}
