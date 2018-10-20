//
// DlgDownloadReviews.cpp
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
#include "Settings.h"
#include "Envy.h"
#include "DlgDownloadReviews.h"

#include "LiveList.h"
#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"
#include "CoolInterface.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CDownloadReviewDlg, CSkinDialog)

//BEGIN_MESSAGE_MAP(CDownloadReviewDlg, CSkinDialog)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadReviewDlg dialog

CDownloadReviewDlg::CDownloadReviewDlg(CWnd* pParent, CDownload* pDownload) : CSkinDialog( CDownloadReviewDlg::IDD, pParent )
{
	m_pDownload = pDownload;
}

CDownloadReviewDlg::~CDownloadReviewDlg()
{
}

void CDownloadReviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REVIEWS, m_wndReviews);
	DDX_Text(pDX, IDC_REVIEW_FILENAME, m_sReviewFileName);
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadReviewDlg message handlers

BOOL CDownloadReviewDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	// Set window icon, translation
	SkinMe( L"CDownloadReviewDlg", IDR_MEDIAFRAME );

	CRect rcList;
	m_wndReviews.GetClientRect( &rcList );
	rcList.right -= GetSystemMetrics( SM_CXVSCROLL );

	CoolInterface.SetImageListTo( m_wndReviews, LVSIL_SMALL );
	m_wndReviews.InsertColumn( 0, L"User", LVCFMT_LEFT, 100, -1 );
	m_wndReviews.InsertColumn( 1, L"Rating", LVCFMT_CENTER, 90, 0 );
	m_wndReviews.InsertColumn( 2, L"Comments", LVCFMT_CENTER, rcList.right - 100 - 80, 1 );
	m_wndReviews.InsertColumn( 3, L"Order", LVCFMT_CENTER, 0, 2 );

	m_wndReviews.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );
	m_wndReviews.EnableToolTips();

	Skin.Translate( L"CReviewList", m_wndReviews.GetHeaderCtrl() );

	// Sort by order added- first at the top
	CLiveList::Sort( &m_wndReviews, 3, FALSE );
	//CLiveList::Sort( &m_wndReviews, 3, FALSE );	// Repeat

	CLiveList pReviews( 4 );
	int nIndex = 1;

	// Lock while we're loading the list. (In case the download is destroyed)
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( ! m_pDownload ) return FALSE;

	m_sReviewFileName = m_pDownload->m_sName;

	CDownloadReview* pReview = m_pDownload->GetFirstReview();

	while ( pReview )
	{
		CLiveItem* pItem = pReviews.Add( pReview );

		// Client picture
		// ToDo: We don't have pictures yet.  Currently, it uses
		// a star for a G2 review, and a little person for everyone else
		switch ( pReview->m_nUserPicture )
		{
		case 0:
			pItem->SetImage( CoolInterface.ImageForID( ID_TOOLS_WIZARD ) );
			break;
		case 1:
			pItem->SetImage( CoolInterface.ImageForID( ID_TOOLS_PROFILE ) );
			break;
		case 2:
			pItem->SetImage( CoolInterface.ImageForID( ID_TOOLS_WIZARD ) );
			break;
		case 3:
			pItem->SetImage( CoolInterface.ImageForID( ID_TOOLS_PROFILE ) );
			break;
		default:
			pItem->SetImage( CoolInterface.ImageForID( ID_TOOLS_PROFILE ) );
		}

		pItem->Set( 0, pReview->m_sUserName );

		int nRating = min( pReview->m_nFileRating, 6 );
		nRating = max( nRating, 0 );
		pItem->Set( 1, LoadString( IDS_RATING_NONE + nRating ) );	// IDS_RATING_1...

		pItem->Set( 2, pReview->m_sFileComments );
		pItem->Format( 3, L"%i", nIndex );

		nIndex++;
		pReview = pReview->m_pNext;
	}

	pLock.Unlock();

	//m_wndReviews.SetFont( &theApp.m_gdiFontBold );

	pReviews.Apply( &m_wndReviews, TRUE );

	UpdateData( FALSE );

	return TRUE;
}

void CDownloadReviewDlg::OnOK()
{
	UpdateData( TRUE );

	CSkinDialog::OnOK();
}
