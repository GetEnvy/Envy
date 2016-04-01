//
// DlgDownloadSheet.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2006
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
#include "DlgDownloadSheet.h"

#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Downloads.h"
#include "Transfers.h"

#include "PageDownloadEdit.h"
#include "PageDownloadActions.h"
#include "PageTorrentGeneral.h"
#include "PageTorrentFiles.h"
#include "PageTorrentTrackers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CDownloadSheet, CPropertySheetAdv)

BEGIN_MESSAGE_MAP(CDownloadSheet, CPropertySheetAdv)
//	ON_WM_ERASEBKGND()
//	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadSheet

CDownloadSheet::CDownloadSheet(CSingleLock& pLock, CDownload* pDownload)
	: m_pLock			( pLock )
	, m_pDownload		( pDownload )
	, m_sFilesTitle 	( L"Files" )
	, m_sTrackersTitle	( L"Trackers" )
	, m_sGeneralTitle	( L"Torrent" )
	, m_sDownloadTitle	( L"General" )
	, m_sActionsTitle	( L"Actions" )
{
	ASSUME_LOCK( Transfers.m_pSection );
}

CDownload* CDownloadSheet::GetDownload() const
{
	ASSUME_LOCK( Transfers.m_pSection );

	return ( Downloads.Check( m_pDownload ) && ! m_pDownload->IsMoving() ) ? m_pDownload : NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadSheet operations

INT_PTR CDownloadSheet::DoModal()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CTorrentFilesPage		pFiles;
	CTorrentTrackersPage	pTrackers;
	CTorrentGeneralPage		pGeneral;
	CDownloadEditPage		pDownload;
	CDownloadActionsPage	pActions;

	if ( m_pDownload->IsTorrent() )
	{
		SetTabTitle( &pFiles, m_sFilesTitle );
		AddPage( &pFiles );
		SetTabTitle( &pTrackers, m_sTrackersTitle );
		AddPage( &pTrackers );
		SetTabTitle( &pGeneral, m_sGeneralTitle );
		AddPage( &pGeneral );
	}

	if ( ! m_pDownload->IsMoving() && ! m_pDownload->IsCompleted() )
	{
		SetTabTitle( &pDownload, m_sDownloadTitle );
		AddPage( &pDownload );
		SetTabTitle( &pActions, m_sActionsTitle );
		AddPage( &pActions );
	}

	return CPropertySheetAdv::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadSheet message handlers

BOOL CDownloadSheet::OnInitDialog()
{
	BOOL bResult = CPropertySheetAdv::OnInitDialog();

	SetFont( &CoolInterface.m_fntNormal );
	SetIcon( theApp.LoadIcon( IDI_PROPERTIES ), TRUE );

	SetWindowText( LoadString( IDS_DOWNLOAD_PROPERTIES ) );

	if ( GetDlgItem( IDOK ) )
	{
		CRect rc;

		GetDlgItem( 0x3021 )->GetWindowRect( &rc );		// Apply Position for OK
		ScreenToClient( &rc );
		GetDlgItem( IDOK )->SetWindowPos( NULL, rc.left + 1, rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );

		GetDlgItem( 0x0009 )->GetWindowRect( &rc );		// Help Position for Cancel
		ScreenToClient( &rc );
		GetDlgItem( IDCANCEL )->SetWindowPos( NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );
	}

	if ( GetDlgItem( 0x3021 ) ) GetDlgItem( 0x3021 )->ShowWindow( SW_HIDE );	// No Apply
	if ( GetDlgItem( 0x0009 ) ) GetDlgItem( 0x0009 )->ShowWindow( SW_HIDE );	// No Help

	// Forcibly create all pages under protection of Transfers.m_pSection
	for ( int i = GetPageCount() - 1 ; i >= 0 ; --i )
	{
		SetActivePage( i );
	}
	m_pLock.Unlock();

	return bResult;
}

// ToDo: Override background of tab control in PagePropertyAdv, otherwise clashes with below skinning

//BOOL CDownloadSheet::OnEraseBkgnd(CDC* pDC)
//{
//	if ( m_pSkin && ! theApp.m_bClosing && m_pSkin->OnEraseBkgnd( this, pDC ) ) return TRUE;
//
//	CRect rc;
//	GetClientRect( &rc );
////	rc.top = Skin.m_nBanner;	// No banner in properties window?
//
//	if ( Images.m_bmDialog.m_hObject )
//		CoolInterface.DrawWatermark( pDC, &rc, &Images.m_bmDialog );
//	else
//		pDC->FillSolidRect( &rc, Colors.m_crDialog );
//
//	return TRUE;
//}

//HBRUSH CDownloadSheet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
//{
//	HBRUSH hbr = CWnd::OnCtlColor( pDC, pWnd, nCtlColor );
//
//	if ( Images.m_bmDialog.m_hObject )	// && nCtlColor == CTLCOLOR_BTN )	// Buttons (edge)
//	{
//			CRect rc;
//			pWnd->GetWindowRect( &rc );
//			CRect rcPos = rc;
//			pWnd->ScreenToClient( &rc );
//			ScreenToClient( &rcPos );
//		//	if ( rcPos.top > Skin.m_nBanner )
//		//		rcPos.top -= Skin.m_nBanner;
//
//			CoolInterface.DrawWatermark( pDC, &rc, &Images.m_bmDialog, FALSE, -rcPos.left, -rcPos.top );
//			hbr = (HBRUSH)GetStockObject( NULL_BRUSH );
//	}
//
//	return hbr;
//}
