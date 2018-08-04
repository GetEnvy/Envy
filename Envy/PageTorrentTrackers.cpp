//
// PageTorrentTrackers.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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
#include "PageTorrentTrackers.h"

#include "DlgDownloadSheet.h"
#include "BENode.h"
#include "BTInfo.h"
#include "Network.h"
#include "Transfers.h"
#include "Downloads.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CTorrentTrackersPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CTorrentTrackersPage, CPropertyPageAdv)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnCustomDrawList)
	ON_NOTIFY(NM_CLICK, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnNMClickTorrentTrackers)
	ON_NOTIFY(NM_DBLCLK, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnNMDblclkTorrentTrackers)
	ON_NOTIFY(LVN_KEYDOWN, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnLvnKeydownTorrentTrackers)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnLvnEndlabeleditTorrentTrackers)
	ON_BN_CLICKED(IDC_TORRENT_REFRESH, &CTorrentTrackersPage::OnTorrentRefresh)
	ON_BN_CLICKED(IDC_SOURCE_ADD, &CTorrentTrackersPage::OnBnClickedTorrentTrackersAdd)
	ON_BN_CLICKED(IDC_SOURCE_REMOVE, &CTorrentTrackersPage::OnBnClickedTorrentTrackersDel)
//	ON_BN_CLICKED(IDC_NAME, &CTorrentTrackersPage::OnBnClickedTorrentTrackersRename)
	ON_CBN_SELCHANGE(IDC_TORRENT_TRACKERMODE, &CTorrentTrackersPage::OnCbnSelchangeTorrentTrackermode)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage property page

CTorrentTrackersPage::CTorrentTrackersPage()
	: CPropertyPageAdv	( CTorrentTrackersPage::IDD )
	, m_nOriginalMode	( CBTInfo::tNull )
	, m_nComplete		( 0 )
	, m_nIncomplete		( 0 )
	, m_nRequest		( NULL )
{
}

CTorrentTrackersPage::~CTorrentTrackersPage()
{
}

void CTorrentTrackersPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TORRENT_TRACKER, m_wndTracker);
	DDX_Control(pDX, IDC_TORRENT_TRACKERS, m_wndTrackers);
	DDX_Control(pDX, IDC_TORRENT_COMPLETED, m_wndComplete);
	DDX_Control(pDX, IDC_TORRENT_INCOMPLETE, m_wndIncomplete);
	DDX_Control(pDX, IDC_TORRENT_TRACKERMODE, m_wndTrackerMode);
	DDX_Control(pDX, IDC_TORRENT_REFRESH, m_wndRefresh);
	DDX_Control(pDX, IDC_SOURCE_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_SOURCE_REMOVE, m_wndDel);
//	DDX_Control(pDX, IDC_NAME, m_wndRename);
}

void CTorrentTrackersPage::UpdateInterface()
{
	int nItem = m_wndTrackers.GetNextItem( -1, LVNI_SELECTED );
	m_wndDel.EnableWindow( ( nItem != -1 ) );
//	m_wndRename.EnableWindow( ( nItem != -1 ) );

	// Find item with current tracker...
	LVFINDINFO fi =
	{
		LVFI_STRING,
		m_sOriginalTracker
	};
	int nCurrentItem = m_wndTrackers.FindItem( &fi );

	// ...and mark it
	LVITEM lvi =
	{
		LVIF_PARAM | LVIF_IMAGE
	};
	int nCount = m_wndTrackers.GetItemCount();
	for ( int i = 0 ; i < nCount ; ++i )
	{
		CString strTracker = m_wndTrackers.GetItemText( i, 0 );
		const UINT nIconID = ( i == nCurrentItem ) ? ID_MEDIA_SELECT :
			( StartsWith( strTracker, _P( L"http://" ) ) || StartsWith( strTracker, _P( L"udp://" ) ) ) ? ID_DOWNLOADS_URI : ID_DISCOVERY_BLOCKED;
		lvi.iItem = i;
		lvi.iImage = CoolInterface.ImageForID( nIconID );
		lvi.lParam = ( i == nCurrentItem ) ? TRUE : FALSE;
		m_wndTrackers.SetItem( &lvi );
	}

	if ( nCount == 0 )
		m_wndTrackerMode.SetCurSel( CBTInfo::tNull );
	else if ( nCount == 1 )
		m_wndTrackerMode.SetCurSel( CBTInfo::tSingle );
	m_wndTrackerMode.EnableWindow( nCount > 1 );

	UpdateWindow();
}

BOOL CTorrentTrackersPage::ApplyTracker()
{
	CString strNewTracker;
	m_wndTracker.GetWindowText( strNewTracker );
	bool bAddressChanged = m_sOriginalTracker != strNewTracker;

	int nNewTrackerMode = m_wndTrackerMode.GetCurSel();
	bool bModeChanged = m_nOriginalMode != nNewTrackerMode;

	bool bListChanged = false;
	int nCount = m_wndTrackers.GetItemCount();
	if ( nCount == m_sOriginalTrackers.GetCount() )
	{
		int i = 0;
		for ( POSITION pos = m_sOriginalTrackers.GetHeadPosition() ; pos ; ++i )
		{
			if ( m_sOriginalTrackers.GetNext( pos ) != m_wndTrackers.GetItemText( i, 0 ) )
			{
				bListChanged = true;
				break;
			}
		}
	}
	else
		bListChanged = true;

	if ( bAddressChanged || bModeChanged || bListChanged )
	{
		CSingleLock oLock( &Transfers.m_pSection );

		// Display warning
		if ( MsgBox( IDS_BT_TRACKER_CHANGE, MB_ICONQUESTION | MB_YESNO ) != IDYES || ! oLock.Lock( 250 ) )
		{
			// Restore original settings
			m_wndTracker.SetWindowText( m_sOriginalTracker );
			m_wndTrackerMode.SetCurSel( m_nOriginalMode );
			return FALSE;
		}

		// Apply new settings
		CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
		if ( CDownload* pDownload = pSheet->GetDownload() )
		{
			CBTInfo& oInfo = pDownload->m_pTorrent;

			m_sOriginalTracker.Empty();
			m_nOriginalMode = nCount ? ( ( nCount > 1 ) ? CBTInfo::tMultiFinding : CBTInfo::tSingle ) : CBTInfo::tNull;
			m_sOriginalTrackers.RemoveAll();

			oInfo.RemoveAllTrackers();
			for ( int i = 0 ; i < nCount ; ++i )
			{
				CString strTracker = m_wndTrackers.GetItemText( i, 0 );
				if ( strTracker == strNewTracker )
				{
					m_sOriginalTracker = strNewTracker;
					m_nOriginalMode = nNewTrackerMode;
				}
				m_sOriginalTrackers.AddTail( strTracker );
				oInfo.SetTracker( strTracker );
			}

			if ( ! m_sOriginalTracker.IsEmpty() )
				oInfo.SetTracker( m_sOriginalTracker );
			oInfo.SetTrackerMode( m_nOriginalMode );

			pDownload->SetModified();
		}
	}
	return TRUE;
}

void CTorrentTrackersPage::InsertTracker()
{
	// De-select all
	int nCount = m_wndTrackers.GetItemCount();
	for ( int i = 0 ; i < nCount ; ++i )
		m_wndTrackers.SetItemState( i, 0, LVIS_SELECTED );

	LVITEM lvi =
	{
		LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT,
		nCount,
		0,
		LVIS_SELECTED,
		LVIS_SELECTED,
		(LPTSTR)(LPCTSTR)Settings.BitTorrent.DefaultTracker,
		0,
		CoolInterface.ImageForID( ID_DOWNLOADS_URI )
	};
	int nItem = m_wndTrackers.InsertItem( &lvi );
	m_wndTrackers.SetItemText( nItem, 1, LoadString( IDS_STATUS_UNKNOWN ) );
	m_wndTrackers.SetFocus();
	m_wndTrackers.EditLabel( nItem );

	if ( nCount == 1 )
		m_wndTrackerMode.SetCurSel( CBTInfo::tMultiFinding );

	UpdateInterface();
}

void CTorrentTrackersPage::EditTracker(int nItem, LPCTSTR szText)
{
	CString strEditedTracker = m_wndTrackers.GetItemText( nItem, 0 );

	if ( ! szText )
	{
		// New item
		LVFINDINFO fi =
		{
			LVFI_STRING,
			strEditedTracker
		};
		int nDuplicate = m_wndTrackers.FindItem( &fi );
		if ( nDuplicate != -1 && nDuplicate != nItem )
			m_wndTrackers.DeleteItem( nItem );	// Remove duplicate tracker
		return;
	}

	// User entered text
	CString strNewTracker = szText;
	strNewTracker.Trim();

	if ( strEditedTracker == strNewTracker )
		return;	// No changes

	if ( strNewTracker.GetLength() < 5 )
	{
		// Remove tracker
		m_wndTrackers.DeleteItem( nItem );
		return;
	}

	// Fix URL
	if ( ! StartsWith( strNewTracker, _P( L"http://" ) ) &&
		 ! StartsWith( strNewTracker, _P( L"udp://" ) ) &&
		 ! StartsWith( strNewTracker, _P( L"https://" ) ) &&
		 ! StartsWith( strNewTracker, _P( L"•" ) ) &&
		 strNewTracker.Find( L"://" ) < 3 )
		strNewTracker = L"http://" + strNewTracker;

	if ( strNewTracker.GetLength() < 22 ||
		 strNewTracker.Right( 9 ) != L"/announce" ||
		 strNewTracker.Find( L'.' ) < 6 )
	{
		if ( MsgBox( IDS_BT_ENCODING, MB_ICONQUESTION|MB_OKCANCEL ) == IDCANCEL )
		{
			m_wndTrackers.DeleteItem( nItem );
			return;
		}
	}

	if ( ! StartsWith( strNewTracker, _P( L"http://" ) ) && ! StartsWith( strNewTracker, _P( L"udp://" ) ) )
		m_wndTrackers.SetItemText( nItem, 1, LoadString( IDS_STATUS_UNSUPPORTED ) );
	else if ( m_wndTrackers.GetItemText( nItem, 1 ) == LoadString( IDS_STATUS_UNSUPPORTED ) )
		m_wndTrackers.SetItemText( nItem, 1, LoadString( IDS_STATUS_UNKNOWN ) );

	LVFINDINFO fi =
	{
		LVFI_STRING,
		strNewTracker
	};
	int nDuplicate = m_wndTrackers.FindItem( &fi );
	if ( nDuplicate == -1 || nItem == nDuplicate )
		m_wndTrackers.SetItemText( nItem, 0, strNewTracker );	// User entered unique tracker
	else
		m_wndTrackers.DeleteItem( nItem );	// Remove duplicate tracker

	UpdateInterface();
}

void CTorrentTrackersPage::SelectTracker(int nItem)
{
	if ( nItem != -1 )
	{
		m_wndTracker.SetWindowText( m_wndTrackers.GetItemText( nItem, 0 ) );
		m_wndTrackerMode.SetCurSel( CBTInfo::tSingle );

		if ( ApplyTracker() )
			OnTorrentRefresh();

		UpdateInterface();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage message handlers

BOOL CTorrentTrackersPage::OnInitDialog()
{
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;

	m_wndAdd.SetIcon( CoolInterface.ExtractIcon( ID_MEDIA_ADD ) );
	m_wndDel.SetIcon( CoolInterface.ExtractIcon( ID_MEDIA_REMOVE ) );
//	m_wndRename.SetIcon( CoolInterface.ExtractIcon( ID_LIBRARY_RENAME ) );

	ASSUME_LOCK( Transfers.m_pSection );

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	ASSERT( pDownload && pDownload->IsTorrent() );

	CBTInfo& oInfo = pDownload->m_pTorrent;

	m_sOriginalTracker = oInfo.GetTrackerAddress();
	m_wndTracker.SetWindowText( m_sOriginalTracker );

	int nCount = oInfo.GetTrackerCount();
	m_nOriginalMode = oInfo.GetTrackerMode();
	m_wndTrackerMode.SetCurSel( m_nOriginalMode );

	// Remove invalid modes
	//if ( nCount < 2 )
	//{
	//	m_wndTrackerMode.DeleteString( CBTInfo::tMultiFound );
	//	m_wndTrackerMode.DeleteString( CBTInfo::tMultiFinding );
	//}

	CRect rc;
	m_wndTrackers.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );

	CoolInterface.SetImageListTo( m_wndTrackers, LVSIL_SMALL );
	m_wndTrackers.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );
	m_wndTrackers.InsertColumn( 0, L"Tracker", LVCFMT_LEFT, rc.right - 82, -1 );
	m_wndTrackers.InsertColumn( 1, L"Status", LVCFMT_CENTER, 82, 0 );
	m_wndTrackers.InsertColumn( 2, L"Type", LVCFMT_CENTER, 0, 0 );
	Skin.Translate( L"CTorrentTrackerList", m_wndTrackers.GetHeaderCtrl() );

	if ( m_wndTrackers.SetBkImage( Skin.GetWatermark( L"CListCtrl" ) ) )		// || m_wndTrackers.SetBkImage( Images.m_bmSystemWindow.m_hObject )		"System.Windows"
		m_wndTrackers.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP );	// No LVS_EX_DOUBLEBUFFER
	else
	{
		m_wndTrackers.SetBkColor( Colors.m_crWindow );
		m_wndTrackers.SetTextBkColor( Colors.m_crWindow );
	}

	m_wndTrackers.SetTextColor( Colors.m_crText );

	for ( int nTracker = 0 ; nTracker < nCount ; nTracker++ )
	{
		//LV_ITEM pItem = {};
		//pItem.mask	= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		//pItem.iItem	= m_wndTrackers.GetItemCount();
		//pItem.lParam	= (LPARAM)nTracker;
		//
		//if ( oInfo.GetTrackerIndex() == nTracker )
		//	pItem.iImage = CoolInterface.ImageForID( ID_MEDIA_SELECT );
		//else if ( oInfo.GetTrackerAddress( nTracker ).GetAt( 0 ) == BAD_TRACKER_TOKEN )
		//	pItem.iImage = CoolInterface.ImageForID( ID_DISCOVERY_BLOCKED );
		//else
		//	pItem.iImage = CoolInterface.ImageForID( ID_DOWNLOADS_URI );
		//
		//pItem.pszText	= (LPTSTR)(LPCTSTR)oInfo.GetTrackerAddress( nTracker );
		//pItem.iItem	= m_wndTrackers.InsertItem( &pItem );

		CString strTracker = oInfo.GetTrackerAddress( nTracker );
		m_sOriginalTrackers.AddTail( strTracker );

		// Display status
		CString strStatus;
		UINT nStatusIcon = ID_DOWNLOADS_URI;
		if ( ! StartsWith( oInfo.GetTrackerAddress( nTracker ), _P( L"http://" ) ) &&
			 ! StartsWith( oInfo.GetTrackerAddress( nTracker ), _P( L"udp://" ) ) )
		{
			// Bad format, or BAD_TRACKER_TOKEN Tagged for display only (*https:// etc.)
			LoadString( strStatus, IDS_STATUS_UNSUPPORTED );
			nStatusIcon = ID_DISCOVERY_BLOCKED;
		}
		else
		{
			switch ( oInfo.GetTrackerStatus( nTracker ) )
			{
			case TRI_TRUE:
				LoadString( strStatus, IDS_STATUS_ACTIVE );
				break;
			case TRI_FALSE:
				LoadString( strStatus, IDS_STATUS_TRACKERDOWN );
				break;
			case TRI_UNKNOWN:
				LoadString( strStatus, IDS_STATUS_UNKNOWN );
			//	break;
			}
		}

		// pItem.iItem
		int nItem = m_wndTrackers.InsertItem( m_wndTrackers.GetItemCount(), strTracker, CoolInterface.ImageForID( nStatusIcon ) );

		m_wndTrackers.SetItemText( nItem, 1, strStatus );

		// Display type
		CString strType = L"Announce";
		if ( oInfo.IsMultiTracker() )
			strType.Format( L"Tier %i", oInfo.GetTrackerTier( nTracker ) );

		m_wndTrackers.SetItemText( nItem, 2, strType );
	}

	if ( Network.IsConnected() )
		PostMessage( WM_COMMAND, MAKELONG( IDC_TORRENT_REFRESH, BN_CLICKED ), (LPARAM)m_wndRefresh.GetSafeHwnd() );

//	CoolInterface.FixThemeControls( this );

	UpdateInterface();

	return TRUE;
}

void CTorrentTrackersPage::OnDestroy()
{
	KillTimer( 1 );

	// Cancel unfinished request
	TrackerRequests.Cancel( m_nRequest );

	CPropertyPageAdv::OnDestroy();
}

void CTorrentTrackersPage::OnTrackerEvent(bool bSuccess, LPCTSTR /*pszReason*/, LPCTSTR /*pszTip*/, CBTTrackerRequest* pEvent)
{
	ASSUME_LOCK( Transfers.m_pSection );

	m_nRequest = 0;		// Need no cancel

	if ( bSuccess )
	{
		m_nComplete   = pEvent->GetComplete();		// ->m_nSeeders
		m_nIncomplete = pEvent->GetIncomplete();	// ->m_nLeechers
	}

	PostMessage( WM_TIMER, bSuccess ? 3 : 2 );
}

void CTorrentTrackersPage::OnTorrentRefresh()
{
	if ( m_wndRefresh.IsWindowEnabled() == FALSE )
		return;

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return;

	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return;

	m_wndRefresh.EnableWindow( FALSE );

	m_nRequest = TrackerRequests.Request( pDownload, BTE_TRACKER_SCRAPE, 0, this );

	//BeginThread( "PageTorrentTrackers" );	// Obsolete
}

void CTorrentTrackersPage::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		// Re-enable the refresh button
		m_wndRefresh.EnableWindow( TRUE );
		KillTimer( 1 );
	}
	else
	{
		// Close the scrape thread (Obsolete)
		//CloseThread();

		// Re-enable the refresh button
		SetTimer( 1, 3300, NULL );

		// Update the display
		if ( nIDEvent == 3 )
		{
			CString str;
			str.Format( L"%u", m_nComplete );
			m_wndComplete.SetWindowText( str );
			str.Format( L"%u", m_nIncomplete );
			m_wndIncomplete.SetWindowText( str );
		}
		else
		{
			m_wndComplete.SetWindowText( L"" );
			m_wndIncomplete.SetWindowText( L"" );
		}
	}
}

BOOL CTorrentTrackersPage::OnApply()
{
	return ApplyTracker() ? CPropertyPageAdv::OnApply() : FALSE;
}

void CTorrentTrackersPage::OnNMClickTorrentTrackers(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	UpdateInterface();
}

void CTorrentTrackersPage::OnNMDblclkTorrentTrackers(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	SelectTracker( m_wndTrackers.GetNextItem( -1, LVNI_SELECTED ) );
}

void CTorrentTrackersPage::OnLvnKeydownTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pNMKD = (LPNMLVKEYDOWN)pNMHDR;

	*pResult = 0;

	if ( pNMKD->wVKey == VK_DELETE )
	{
		int nItem = m_wndTrackers.GetNextItem( -1, LVNI_SELECTED );
		if ( nItem != -1 )
		{
			m_wndTrackers.DeleteItem( nItem );
			UpdateInterface();
		}
	}
	else if ( pNMKD->wVKey == VK_INSERT )
	{
		InsertTracker();
	}
	else if ( pNMKD->wVKey == VK_SPACE || pNMKD->wVKey == VK_RETURN )
	{
		SelectTracker( m_wndTrackers.GetNextItem( -1, LVNI_SELECTED ) );
	}
}

void CTorrentTrackersPage::OnCbnSelchangeTorrentTrackermode()
{
	UpdateInterface();
}

void CTorrentTrackersPage::OnBnClickedTorrentTrackersAdd()
{
	InsertTracker();
}

void CTorrentTrackersPage::OnBnClickedTorrentTrackersDel()
{
	int nItem = m_wndTrackers.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem != -1 )
	{
		m_wndTrackers.DeleteItem( nItem );
		UpdateInterface();
	}
}

//void CTorrentTrackersPage::OnBnClickedTorrentTrackersRename()
//{
//	int nItem = m_wndTrackers.GetNextItem( -1, LVNI_SELECTED );
//	if ( nItem != -1 )
//	{
//		m_wndTrackers.SetFocus();
//		m_wndTrackers.EditLabel( nItem );
//	}
//}

void CTorrentTrackersPage::OnLvnEndlabeleditTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast< NMLVDISPINFO* >( pNMHDR );

	*pResult = 0;

	EditTracker( pDispInfo->item.iItem, pDispInfo->item.pszText );
}

void CTorrentTrackersPage::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	if ( ! ::IsWindow( m_wndTrackers.GetSafeHwnd() ) ) return;

	if ( m_wndTrackers.GetBkColor() != Colors.m_crWindow ) return;	// Rarely needed (Remove this line when useful)

	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		//if ( m_wndTrackers.GetBkColor() == Colors.m_crWindow )
		//	pDraw->clrTextBk = Colors.m_crWindow;
		//pDraw->clrText = Colors.m_crText;

		*pResult = CDRF_DODEFAULT;
	}
}

// Obsolete Apply:
//BOOL CTorrentTrackersPage::OnApply()
//{
//	if ( ! UpdateData() )
//		return FALSE;
//
//	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
//
//	CSingleLock oLock( &Transfers.m_pSection );
//	if ( ! oLock.Lock( 500 ) )
//		return FALSE;
//
//	CDownload* pDownload = pSheet->GetDownload();
//	if ( ! pDownload )
//		return CPropertyPageAdv::OnApply();
//
//	CBTInfo& oInfo = pDownload->m_pTorrent;
//
//	const int nTrackerMode = m_wndTrackerMode.GetCurSel();
//	const BOOL bModeChanged = oInfo.GetTrackerMode() != nTrackerMode;
//	const BOOL bAddressChanged = oInfo.GetTrackerAddress() != m_sTracker;
//	if ( bAddressChanged || bModeChanged )
//	{
//		oLock.Unlock();
//
//		GetDlgItem( IDC_TORRENT_TRACKER )->SetFocus();
//
//		// Display warning
//		if ( MsgBox( IDS_BT_TRACKER_CHANGE, MB_ICONQUESTION | MB_YESNO ) != IDYES )
//			return FALSE;
//
//		if ( ! oLock.Lock( 250 ) )
//			return FALSE;
//
//		pDownload = pSheet->GetDownload();
//		if ( ! pDownload )
//			return CPropertyPageAdv::OnApply();
//
//		if ( bAddressChanged )
//			oInfo.SetTracker( m_sTracker );
//		if ( bModeChanged )
//			oInfo.SetTrackerMode( nTrackerMode );
//	}
//
//	return CPropertyPageAdv::OnApply();
//}

// Obsolete thread scrape method for reference or deletion
//void CTorrentTrackersPage::OnRun()
//{
//	CSingleLock oLock( &Transfers.m_pSection );
//	if ( oLock.Lock( 250 ) )
//	{
//		CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
//		if ( CDownload* pDownload = pSheet->GetDownload() )
//		{
//			m_pRequest.Clear();
//
//			CString strURL = m_sTracker;
//			if ( strURL.Left( 4 ) == L"http" &&	// ToDo: Support UDP Tracker Scrape!
//				strURL.Replace( L"/announce", L"/scrape" ) == 1 )
//			{
//				// Fetch scrape only for the given info hash
//				strURL = strURL.TrimRight( L'&' ) +
//					( ( strURL.Find( L'?' ) != -1 ) ? L'&' : L'?' ) +
//					L"info_hash=" + CBTTrackerRequest::Escape( pDownload->m_pTorrent.m_oBTH ) +
//					L"&peer_id="  + CBTTrackerRequest::Escape( pDownload->m_pPeerID );
//
//				oLock.Unlock();
//
//				m_pRequest.SetURL( strURL );
//				m_pRequest.AddHeader( L"Accept-Encoding", L"deflate, gzip" );
//				m_pRequest.EnableCookie( false );
//				m_pRequest.SetUserAgent( Settings.SmartAgent() );
//
//				theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, L"[BT] Sending BitTorrent tracker scrape: %s", (LPCTSTR)strURL );
//
//				if ( m_pRequest.Execute( FALSE ) && m_pRequest.InflateResponse() )
//				{
//					CBuffer* pResponse = m_pRequest.GetResponseBuffer();
//					if ( pResponse != NULL && pResponse->m_pBuffer != NULL )
//					{
//						if ( CBENode* pNode = CBENode::Decode( pResponse ) )
//						{
//							theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"[BT] Received BitTorrent tracker response: %s", pNode->Encode() );
//
//							if ( oLock.Lock( 250 ) )
//							{
//								pDownload = pSheet->GetDownload();
//								if ( pDownload )
//								{
//									if ( OnTree( pDownload, pNode ) )
//									{
//										delete pNode;
//										PostMessage( WM_TIMER, 3 );
//										return;
//									}
//								}
//							}
//							delete pNode;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	m_pRequest.Clear();
//	PostMessage( WM_TIMER, 2 );
//}

//BOOL CTorrentTrackersPage::OnTree(CDownload* pDownload, CBENode* pNode)
//{
//	ASSUME_LOCK( Transfers.m_pSection );
//
//	if ( ! pNode->IsType( CBENode::beDict ) ) return FALSE;
//
//	CBENode* pFiles = pNode->GetNode( "files" );
//	if ( ! pFiles->IsType( CBENode::beDict ) ) return FALSE;
//
//	LPBYTE nKey = &pDownload->m_pTorrent.m_oBTH[ 0 ];
//
//	CBENode* pFile = pFiles->GetNode( nKey, Hashes::BtHash::byteCount );
//	if ( ! pFile->IsType( CBENode::beDict ) ) return FALSE;
//
//	m_nComplete = 0;
//	m_nIncomplete = 0;
//
//	if ( CBENode* pComplete = pFile->GetNode( "complete" ) )
//	{
//		if ( ! pComplete->IsType( CBENode::beInt ) ) return FALSE;
//		// Since we read QWORDs, make sure we won't get negative values;
//		// Some buggy trackers send very huge numbers, so leave them as the max int.
//		m_nComplete = (int)( pComplete->GetInt() & ~0xFFFF0000 );
//	}
//
//	if ( CBENode* pIncomplete = pFile->GetNode( "incomplete" ) )
//	{
//		if ( ! pIncomplete->IsType( CBENode::beInt ) ) return FALSE;
//		m_nIncomplete = (int)( pIncomplete->GetInt() & ~0xFFFF0000 );
//	}
//
//	return TRUE;
//}
