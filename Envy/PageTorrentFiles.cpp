//
// PageTorrentFiles.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2007
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
#include "Settings.h"
#include "Envy.h"

#include "DlgDownloadSheet.h"
#include "PageTorrentFiles.h"
#include "LiveList.h"
#include "Transfers.h"
#include "Downloads.h"
#include "CtrlLibraryTip.h"

#include "Skin.h"
#include "Colors.h"
#include "ShellIcons.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Set column order
enum {
	COL_NAME,
	COL_SIZE,
	COL_STATUS,
	COL_INDEX,
	COL_LAST
};


IMPLEMENT_DYNCREATE(CTorrentFilesPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CTorrentFilesPage, CPropertyPageAdv)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_DBLCLK, IDC_TORRENT_FILES, &CTorrentFilesPage::OnNMDblclkTorrentFiles)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TORRENT_FILES, &CTorrentFilesPage::OnCustomDrawList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TORRENT_FILES, &CTorrentFilesPage::OnCheckbox)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnSortColumn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTorrentFilesPage property page

CTorrentFilesPage::CTorrentFilesPage()
	: CPropertyPageAdv( CTorrentFilesPage::IDD )
	, m_bLoaded		( FALSE )
{
}

CTorrentFilesPage::~CTorrentFilesPage()
{
}

void CTorrentFilesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TORRENT_FILES, m_wndFiles);
	DDX_Text(pDX, IDC_TORRENT_COUNT, m_sFilecount);
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentFilesPage message handlers

BOOL CTorrentFilesPage::OnInitDialog()
{
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;

	//ASSUME_LOCK( Transfers.m_pSection );

	unique_ptr< CLibraryTipCtrl > pTip( new CLibraryTipCtrl );
	pTip->Create( this, &Settings.Interface.TipDownloads );
	//m_wndFiles.EnableTips( pTip );	// Unused ComboListCtrl

	CRect rc;
	m_wndFiles.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	m_wndFiles.InsertColumn( COL_NAME,	L"Filename",	LVCFMT_LEFT,	rc.right - 66 - 54, -1 );
	m_wndFiles.InsertColumn( COL_SIZE,	L"Size", 		LVCFMT_RIGHT,	66, 0 );
	m_wndFiles.InsertColumn( COL_STATUS, L"Status",		LVCFMT_RIGHT,	54, 0 );
	m_wndFiles.InsertColumn( COL_INDEX,	L"Index",		LVCFMT_CENTER,	0, 0 );		// Workaround for internal use
//	m_wndFiles.InsertColumn( COL_PRIORITY, L"Priority", LVCFMT_RIGHT,	52, 0 );	// Obsolete

	m_wndFiles.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP|LVS_EX_CHECKBOXES );
	ShellIcons.AttachTo( &m_wndFiles, 16 );	// m_wndFiles.SetImageList()

	Skin.Translate( L"CTorrentFileList", m_wndFiles.GetHeaderCtrl() );

	if ( m_wndFiles.SetBkImage( Skin.GetWatermark( L"CListCtrl" ) ) )		// || m_wndFiles.SetBkImage( Images.m_bmSystemWindow.m_hObject )	"System.Windows"
	{
		m_wndFiles.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_CHECKBOXES );	// No LVS_EX_DOUBLEBUFFER
	}
	else
	{
		m_wndFiles.SetBkColor( Colors.m_crWindow );
		m_wndFiles.SetTextBkColor( Colors.m_crWindow );
	}

	m_wndFiles.SetTextColor( Colors.m_crText );

// Priority Column Combobox:	(Unused custom ComboListCtrl)
//	BEGIN_COLUMN_MAP()
//		COLUMN_MAP( CFragmentedFile::prHigh,	LoadString( IDS_PRIORITY_HIGH ) )
//		COLUMN_MAP( CFragmentedFile::prNormal,	LoadString( IDS_PRIORITY_NORMAL ) )
//		COLUMN_MAP( CFragmentedFile::prLow,		LoadString( IDS_PRIORITY_LOW ) )
//		COLUMN_MAP( CFragmentedFile::prUnwanted, LoadString( IDS_PRIORITY_OFF ) )
//	END_COLUMN_MAP( m_wndFiles, COL_PRIORITY )

	// Note list populating & timer moved to async OnShowWindow below

	return TRUE;
}

void CTorrentFilesPage::OnShowWindow(BOOL bShow, UINT /*nStatus*/)
{
	if ( ! bShow || m_bLoaded )
		return;		// First time only

	SetTimer( 1, 500, NULL );	// Populate files
}

void CTorrentFilesPage::OnCheckbox(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	BOOL bPrevState	= (BOOL)( ( ( pNMListView->uOldState & LVIS_STATEIMAGEMASK ) >> 12 ) - 1 );
	if ( bPrevState < 0 ) return;		// No previous state at startup

	BOOL bChecked	= (BOOL)( ( ( pNMListView->uNewState & LVIS_STATEIMAGEMASK ) >> 12 ) - 1 );
	if ( bChecked < 0 ) return;			// Non-checkbox notifications

	if ( bChecked == bPrevState ) return;	// No change

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 500 ) ) return;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload ) return;			// Invalid download

	unique_ptr< CFragmentedFile > pFragFile( pDownload->GetFile() );
	if ( ! pFragFile.get() ) return;

	int nIndex = _wtoi( m_wndFiles.GetItemText( pNMListView->iItem, COL_INDEX ) );

	if ( nIndex >= 0 )
	{
		pFragFile->SetPriority( /*pNMListView->iItem*/ nIndex,
			bChecked ? CFragmentedFile::prNormal : CFragmentedFile::prUnwanted );
	}
	else if ( nIndex == -1 )	// __padding_file_ special case group
	{
		const DWORD nCount = pFragFile->GetCount();
		if ( nCount > 2 )
		{
			CString strName;
			for ( DWORD i = 0 ; i < nCount ; i++ )
			{
				strName = pFragFile->GetName( i );
				strName = strName.Mid( strName.ReverseFind( L'\\' ) + 1 );

				if ( strName.GetAt( 0 ) == L'_' && strName.Left( 18 ) == L"_____padding_file_" )
					pFragFile->SetPriority( i, bChecked ? CFragmentedFile::prNormal : CFragmentedFile::prUnwanted );
			}
		}
	}

	pLock.Unlock();

	// Multiple highlighted items group handling
	if ( m_wndFiles.GetItemState( pNMListView->iItem, LVIS_SELECTED ) )
	{
		int nItem = -1;
		while ( ( nItem = m_wndFiles.GetNextItem( nItem, LVNI_SELECTED ) ) > -1 )
		{
			if ( m_wndFiles.GetCheck( nItem ) != bChecked )
			{
				nIndex = _wtoi( m_wndFiles.GetItemText( nItem, COL_INDEX ) );
				if ( nIndex < 0 ) continue;		// __padding_file_
				pFragFile->SetPriority( nIndex, bChecked ? CFragmentedFile::prNormal : CFragmentedFile::prUnwanted );
				m_wndFiles.SetCheck( nItem, bChecked ? BST_CHECKED : BST_UNCHECKED );
			}
		}
	}

	if ( m_bLoaded )
		UpdateCount();
}

void CTorrentFilesPage::OnSortColumn(NMHDR* pNotifyStruct, LRESULT* /*pResult*/)
{
	if ( m_wndFiles.GetItemCount() < 2 ) return;

	HD_NOTIFY *pHDN = (HD_NOTIFY *)pNotifyStruct;

	if ( pHDN->iButton != 0 ) return;	// Verify header clicked with left mouse button

	CLiveList::Sort( &m_wndFiles, pHDN->iItem, FALSE );
}

void CTorrentFilesPage::OnNMDblclkTorrentFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	CPoint pt;
	GetCursorPos( &pt );
	m_wndFiles.ScreenToClient( &pt );
	if ( pt.x < 18 )	// Ignore checkbox
		return;

	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	const int nIndex = _wtoi( m_wndFiles.GetItemText( pNMItemActivate->iItem, COL_INDEX ) );

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload )
		return;		// Invalid download

	pDownload->Launch( nIndex, &oLock, FALSE );
}

BOOL CTorrentFilesPage::OnApply()
{
// Unused ComboListCtrl Priority Column:
//	CSingleLock oLock( &Transfers.m_pSection, TRUE );
//	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
//	if ( ! Downloads.Check( pDownload ) || ! pDownload->IsTorrent() ) return FALSE;
//	unique_ptr< CFragmentedFile > pFragFile = pDownload->GetFile();
//	if ( pFragFile.get() )
//		for ( DWORD i = 0 ; i < pFragFile->GetCount() ; ++i )
//			pFragFile->SetPriority( i, m_wndFiles.GetColumnData( i, COL_INDEX ) );

	return CPropertyPageAdv::OnApply();
}

void CTorrentFilesPage::UpdateCount()
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 200 ) ) return;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	unique_ptr< CFragmentedFile > pFragFile( pDownload->GetFile() );
	if ( ! pFragFile.get() ) return;

	oLock.Unlock();

	if ( pFragFile->GetCount() > 1 )
	{
		DWORD nTotalCount = m_wndFiles.GetItemCount();
		DWORD nActiveCount = 0;
		QWORD nActiveSize  = 0;
		for ( int nItem = 0 ; nItem < nTotalCount ; nItem++ )
		{
			if ( ! m_wndFiles.GetCheck( nItem ) )
				continue;

			nActiveCount++;
			nActiveSize += pFragFile->GetLength( _wtoi( m_wndFiles.GetItemText( nItem, COL_INDEX ) ) );
		}

		if ( nActiveCount != 1 )
			m_sFilecount.Format( L"%u %s:   %s", nActiveCount, LoadString( IDS_FILES ), Settings.SmartVolume( nActiveSize ) );
		else
			m_sFilecount.Format( L"1 %s:   %u B", LoadString( IDS_FILE ), nActiveSize );
	}
	else
	{
		m_sFilecount.Format( L"1 %s:   %u B", LoadString( IDS_FILE ), pFragFile->GetTotal() );
	}

	UpdateData( FALSE );
	UpdateWindow();
}

void CTorrentFilesPage::Update()
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 50 ) ) return;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload ) return;		// Invalid download

	unique_ptr< CFragmentedFile > pFragFile( pDownload->GetFile() );
	if ( ! pFragFile.get() )
		return;

	CString strCompleted;
	int nIndex;
	int nPaddingItem = 0;

	int nItem = -1;
	while ( ( nItem = m_wndFiles.GetNextItem( nItem, 0 ) ) > -1 )
	{
		nIndex = _wtoi( m_wndFiles.GetItemText( nItem, COL_INDEX ) );
		if ( nIndex < 0 )	// __padding_file_ group
		{
			nPaddingItem = nItem;
			continue;
		}

		strCompleted.Format( L"%.2f%%", pFragFile->GetProgress( nIndex ) );
		m_wndFiles.SetItemText( nItem, COL_STATUS, strCompleted );
	}

	if ( nPaddingItem )		// Rare __padding_file_ group special handling
	{
		const int nCount = pFragFile->GetCount();
		int nPaddingCount = 0;
		float fPaddingStatus = 0.000;
		CString strText;

		for ( int i = 0 ; i < nCount ; i++ )
		{
			strText = pFragFile->GetName( i );
			strText = strText.Mid( strText.Find( L'\\' ) + 1 );
			if ( strText.GetAt( 0 ) != L'_' || _tcscmp( strText.Left( 18 ), L"_____padding_file_" ) != 0 )
				continue;

			fPaddingStatus += pFragFile->GetProgress( i );
			nPaddingCount++;
		}

		if ( ! nPaddingCount ) return;
		strCompleted.Format( L"%.2f%%", fPaddingStatus / nPaddingCount );
		m_wndFiles.SetItemText( nPaddingItem, COL_STATUS, strCompleted );
	}
}

void CTorrentFilesPage::GetFiles()
{
	if ( m_wndFiles.GetItemCount() )
		return;		// First time only

	KillTimer( 1 );

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	ASSERT( pDownload && pDownload->IsTorrent() );

	unique_ptr< CFragmentedFile > pFragFile( pDownload->GetFile() );
	if ( ! pFragFile.get() ) return;

	pLock.Unlock();

	const DWORD nCount = pFragFile->GetCount();

	if ( nCount < 2 || pDownload->IsSeeding() )
		m_wndFiles.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );	// No checkboxes needed
	if ( nCount < 2 )
		m_wndFiles.DeleteColumn( COL_INDEX );
	//else if ( nCount > 1500 )
	//	theApp.Message( MSG_TRAY|MSG_NOTICE, IDS_GENERAL_PLEASEWAIT );		// Obsolete L"Verifying large torrent.\nPlease wait."

	CString strText;
	DWORD nPadding = 0;
	QWORD nPaddingSize = 0;
	BOOL bPaddingCheck = FALSE;

	for ( DWORD i = 0 ; i < nCount ; i++ )
	{
		strText = pFragFile->GetName( i );
		strText = strText.Mid( strText.Find( L'\\' ) + 1 );

		// Unwanted files
		if ( strText[0] == L'_' && StartsWith( strText, L"_____padding_file_", 18 ) )
		{
			if ( Settings.BitTorrent.SkipPaddingFiles )
			{
			//	pFragFile->SetPriority( i, CFragmentedFile::prUnwanted );	// Redundant
				if ( ! bPaddingCheck && pFragFile->GetPriority( i ) != CFragmentedFile::prUnwanted ) bPaddingCheck = TRUE;
				nPaddingSize += pFragFile->GetLength( i );
				nPadding++;
				continue;
			}

			strText = strText.Left( 20 ) + L" ...";	// Otherwise hide BitComet note
		}

		LV_ITEM pItem	= {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.lParam	= (LPARAM)pFragFile->GetAt( i );
		pItem.pszText	= (LPTSTR)(LPCTSTR)strText;
		pItem.iImage	= ShellIcons.Get( strText, 16 );
		pItem.iItem		= i;
		pItem.iItem		= m_wndFiles.InsertItem( &pItem );
		m_wndFiles.SetItemText( pItem.iItem, COL_SIZE, Settings.SmartVolume( pFragFile->GetLength( i ) ) );

		strText.Format( L"%i", i );
		m_wndFiles.SetItemText( pItem.iItem, COL_INDEX, strText );
		m_wndFiles.SetItemState( pItem.iItem,
			UINT( ( pFragFile->GetPriority( i ) == CFragmentedFile::prUnwanted ? 1 : 2 ) << 12 ), LVIS_STATEIMAGEMASK );
	//	m_wndFiles.SetColumnData( pItem.iItem, COL_PRIORITY, pFragFile->GetPriority( i ) );		// Legacy priority column, unused

	//	if ( i > 500 )		// No longer applies. Very large torrents made app non-responding in OnInitDialog (~180/sec, 10,000/minute)
	//		theApp.KeepAlive();
	}

	if ( nPadding )
	{
		if ( nPadding == 1 )
			strText = L"_____padding_file_0_...";
		else
			strText.Format( L"_____padding_file_...  (x%u)", nPadding );
		LV_ITEM pItem	= {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.lParam	= NULL;
		pItem.pszText	= (LPTSTR)(LPCTSTR)strText;
		pItem.iImage	= ShellIcons.Get( L"", 16 );
		pItem.iItem		= nCount - nPadding;
		pItem.iItem		= m_wndFiles.InsertItem( &pItem );
		m_wndFiles.SetItemText( pItem.iItem, COL_INDEX, L"-1" );
		m_wndFiles.SetItemText( pItem.iItem, COL_SIZE, Settings.SmartVolume( nPaddingSize ) );
		m_wndFiles.SetItemState( pItem.iItem, UINT( ( bPaddingCheck ? 2 : 1 ) << 12 ), LVIS_STATEIMAGEMASK );
	}

	m_bLoaded = TRUE;

	UpdateCount();

	Update();

	if ( ! pDownload->IsSeeding() )
		SetTimer( 1, nCount > 60 ? nCount > 1000 ? 330 : 120 : 50, NULL );		// Rapid refresh %  (Arbitrary 3/8/20 chances per second)
}

void CTorrentFilesPage::OnTimer(UINT_PTR nIDEvent)
{
	CPropertyPageAdv::OnTimer( nIDEvent );

	if ( static_cast< CPropertySheet* >( GetParent() )->GetActivePage() != this )
		return;

	if ( ! m_wndFiles.GetItemCount() )	// First time only ( ! m_bLoaded )
		GetFiles();
	else
		Update();
}

void CTorrentFilesPage::OnDestroy()
{
	KillTimer( 1 );

	CPropertyPageAdv::OnDestroy();
}

void CTorrentFilesPage::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	if ( ! ::IsWindow( m_wndFiles.GetSafeHwnd() ) ) return;

	if ( m_wndFiles.GetBkColor() != Colors.m_crWindow ) return;		// Rarely needed (Remove this line when useful)

	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		//if ( m_wndFiles.GetBkColor() == Colors.m_crWindow )
		//	pDraw->clrTextBk = Colors.m_crWindow;
		//pDraw->clrText = Colors.m_crText;

		*pResult = CDRF_DODEFAULT;
	}
}
