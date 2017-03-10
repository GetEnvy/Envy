//
// WndHostCache.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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
#include "WndHostCache.h"
#include "HostCache.h"
#include "HubHorizon.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Network.h"
#include "DlgUpdateServers.h"
#include "LiveList.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Skin.h"
#include "Flags.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Set Column Order
enum {
	COL_ADDRESS,
	COL_PORT,
	COL_SEEN,
	COL_FAILURES,
	COL_USERS,
	COL_MAXUSERS,
	COL_NAME,
	COL_INFO,
	COL_CLIENT,
	COL_COUNTRY,
#ifdef _DEBUG
	COL_DBG_KEY,
	COL_DBG_QUERY,
	COL_DBG_ACK,
#endif
	COL_LAST	// Column Count
};


IMPLEMENT_SERIAL(CHostCacheWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CHostCacheWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_NCMOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_HOSTS, OnCustomDrawList)
	ON_NOTIFY(NM_DBLCLK, IDC_HOSTS, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_HOSTS, OnSortList)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_CONNECT, OnUpdateHostCacheConnect)
	ON_COMMAND(ID_HOSTCACHE_CONNECT, OnHostCacheConnect)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_DISCONNECT, OnUpdateHostCacheDisconnect)
	ON_COMMAND(ID_HOSTCACHE_DISCONNECT, OnHostCacheDisconnect)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_REMOVE, OnUpdateHostCacheRemove)
	ON_COMMAND(ID_HOSTCACHE_REMOVE, OnHostCacheRemove)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_G2_HORIZON, OnUpdateHostcacheG2Horizon)
	ON_COMMAND(ID_HOSTCACHE_G2_HORIZON, OnHostcacheG2Horizon)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_G2_CACHE, OnUpdateHostcacheG2Cache)
	ON_COMMAND(ID_HOSTCACHE_G2_CACHE, OnHostcacheG2Cache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_G1_CACHE, OnUpdateHostcacheG1Cache)
	ON_COMMAND(ID_HOSTCACHE_G1_CACHE, OnHostcacheG1Cache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_ED2K_CACHE, OnUpdateHostcacheEd2kCache)
	ON_COMMAND(ID_HOSTCACHE_ED2K_CACHE, OnHostcacheEd2kCache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_BT_CACHE, OnUpdateHostcacheBTCache)
	ON_COMMAND(ID_HOSTCACHE_BT_CACHE, OnHostcacheBTCache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_KAD_CACHE, OnUpdateHostcacheKADCache)
	ON_COMMAND(ID_HOSTCACHE_KAD_CACHE, OnHostcacheKADCache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_DC_CACHE, OnUpdateHostcacheDCCache)
	ON_COMMAND(ID_HOSTCACHE_DC_CACHE, OnHostcacheDCCache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_PRIORITY, OnUpdateHostcachePriority)
	ON_COMMAND(ID_HOSTCACHE_PRIORITY, OnHostcachePriority)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_URI, OnUpdateNeighboursCopy)
	ON_COMMAND(ID_NEIGHBOURS_URI, OnNeighboursCopy)
	ON_COMMAND(ID_HOSTCACHE_FILE_DOWNLOAD, OnHostcacheFileDownload)
	ON_COMMAND(ID_HOSTCACHE_IMPORT, OnHostcacheImport)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHostCacheWnd construction

CHostCacheWnd::CHostCacheWnd()
	: m_nMode			( PROTOCOLID( Settings.Gnutella.HostCacheView ) )
	, m_bAllowUpdates	( TRUE )
	, m_nCookie			( 0 )
	, m_tLastUpdate		( 0 )
{
	Create( IDR_HOSTCACHEFRAME );
}

//CHostCacheWnd::~CHostCacheWnd()
//{
//}

/////////////////////////////////////////////////////////////////////////////
// CHostCacheWnd create

int CHostCacheWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_HOSTS, COL_LAST );		// Ensure enum includes 3 additional debug columns or not

	m_pSizer.Attach( &m_wndList );

	// Merge protocols and flags in one image list (OnSkin)
//	CoolInterface.LoadIconsTo( m_gdiImageList, protocolIDs );
//	CoolInterface.LoadFlagsTo( m_gdiImageList );
//	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );
	m_wndList.SetFont( &theApp.m_gdiFont );

	m_wndList.InsertColumn( COL_ADDRESS,	L"Address",	LVCFMT_LEFT,	140 );
	m_wndList.InsertColumn( COL_PORT,		L"Port", 	LVCFMT_CENTER,	 60 );
	m_wndList.InsertColumn( COL_SEEN,		L"Last Seen", LVCFMT_CENTER,	128 );
	m_wndList.InsertColumn( COL_FAILURES,	L"Failures",	LVCFMT_CENTER,	 60 );
	m_wndList.InsertColumn( COL_USERS,		L"CurUsers",	LVCFMT_CENTER,	 60 );
	m_wndList.InsertColumn( COL_MAXUSERS,	L"MaxUsers",	LVCFMT_CENTER,	 60 );
	m_wndList.InsertColumn( COL_NAME,		L"Name", 	LVCFMT_LEFT,	140 );
	m_wndList.InsertColumn( COL_INFO,		L"Description", LVCFMT_LEFT,	140 );
	m_wndList.InsertColumn( COL_CLIENT, 	L"Client",	LVCFMT_CENTER,	100 );
	m_wndList.InsertColumn( COL_COUNTRY,	L"Country",	LVCFMT_LEFT,	 60 );
#ifdef _DEBUG
	m_wndList.InsertColumn( COL_DBG_KEY,	L"Key",		LVCFMT_RIGHT, 0 );
	m_wndList.InsertColumn( COL_DBG_QUERY,	L"Query",	LVCFMT_RIGHT, 0 );
	m_wndList.InsertColumn( COL_DBG_ACK,	L"Ack",		LVCFMT_RIGHT, 0 );
#endif

	LoadState( L"CHostCacheWnd", TRUE );

	CWaitCursor pCursor;
	m_bAllowUpdates = TRUE;
	Update( TRUE );

	return 0;
}

void CHostCacheWnd::OnDestroy()
{
	HostCache.Save();

	Settings.SaveList( L"CHostCacheWnd", &m_wndList );
	SaveState( L"CHostCacheWnd" );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CHostCacheWnd operations

void CHostCacheWnd::Update(BOOL bForce)
{
	if ( ! bForce && ! m_bAllowUpdates ) return;

	CHostCacheList* pCache = HostCache.ForProtocol( m_nMode ? m_nMode : PROTOCOL_G2 );

	CSingleLock oLock( &pCache->m_pSection, FALSE );
	if ( ! oLock.Lock( 100 ) ) return;

	m_nCookie = pCache->m_nCookie;

	for ( CHostCacheIterator i = pCache->Begin() ; i != pCache->End() ; ++i )
	{
		CHostCacheHostPtr pHost = (*i);

		if ( m_nMode == PROTOCOL_NULL )
		{
			if ( HubHorizonPool.Find( &pHost->m_pAddress ) == NULL ) continue;
		}

		CLiveItem* pItem = m_wndList.Add( pHost );

		pItem->SetImage( pHost->m_nProtocol );
		pItem->SetMaskOverlay( pHost->m_bPriority );

		CString strAddress;
		if ( pHost->m_sAddress.IsEmpty() || pHost->m_pAddress.s_addr != INADDR_ANY )
			strAddress = inet_ntoa( pHost->m_pAddress );

		if ( pHost->m_sAddress.IsEmpty() )
			pItem->Set( COL_ADDRESS, L" " + strAddress );
		else if ( pHost->m_pAddress.s_addr == INADDR_ANY )
			pItem->Set( COL_ADDRESS, L" " + pHost->m_sAddress );
		else
			pItem->Set( COL_ADDRESS, L" " + pHost->m_sAddress + L"  (" + strAddress + L")" );

		pItem->Format( COL_PORT, L"%hu", pHost->m_nPort );

		if ( pHost->m_pVendor )
			pItem->Set( COL_CLIENT, pHost->m_pVendor->m_sName );
		else	// Unknown Vendor Code
			pItem->Set( COL_CLIENT, CString( L"(" ) + protocolNames[ pHost->m_nProtocol ] + L")" );

		CTime pTime( (time_t)pHost->Seen() );
		pItem->Set( COL_SEEN, pTime.Format( L"%Y-%m-%d %H:%M:%S" ) );

		// Display workaround  (ToDo: Fix properly elsewhere)
		CString strName = pHost->m_sName;
		if ( pHost->m_nProtocol == PROTOCOL_ED2K && strName.IsEmpty() )
			strName = Neighbours.GetServerName( pHost->m_sAddress.IsEmpty() ? strAddress : pHost->m_sAddress );

		pItem->Set( COL_NAME, strName );
		pItem->Set( COL_INFO, pHost->m_sDescription );
		if ( pHost->m_nDailyUptime )	// Only G1?
		{
			pTime = (time_t)pHost->m_nDailyUptime;
			pItem->Set( COL_INFO, pTime.Format( L"%H:%M:%S" ) );
		}
		//else	// ToDo: Use COL_INFO for G2?

		if ( pHost->m_nFailures )  pItem->Format( COL_FAILURES, L"%u", pHost->m_nFailures );
		if ( pHost->m_nUserCount ) pItem->Format( COL_USERS,    L"%u", pHost->m_nUserCount );
		if ( pHost->m_nUserLimit ) pItem->Format( COL_MAXUSERS, L"%u", pHost->m_nUserLimit );
		if ( pHost->m_sCountry )
		{
			pItem->Set( COL_COUNTRY, pHost->m_sCountry );
			const int nFlagIndex = Flags.GetFlagIndex( pHost->m_sCountry );
			if ( nFlagIndex >= 0 )
				pItem->SetImage( COL_COUNTRY, PROTOCOL_LAST + nFlagIndex );
		}
#ifdef _DEBUG
		if ( pHost->m_nKeyValue ) pItem->Format( COL_DBG_KEY, L"%u", pHost->m_nKeyValue);
		if ( pHost->m_tQuery ) pItem->Format( COL_DBG_QUERY, L"%u", pHost->m_tQuery );
		if ( pHost->m_tAck ) pItem->Format( COL_DBG_ACK, L"%u", pHost->m_tAck);
#endif
	}

	m_wndList.Apply();

	m_tLastUpdate = GetTickCount();			// Update timer
}

CHostCacheHostPtr CHostCacheWnd::GetItem(int nItem)
{
	if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
	{
		CHostCacheHostPtr pHost = (CHostCacheHostPtr)m_wndList.GetItemData( nItem );
		if ( HostCache.Check( pHost ) ) return pHost;
	}

	return NULL;
}

void CHostCacheWnd::OnSkinChange()
{
	OnSize( 0, 0, 0 );
	CPanelWnd::OnSkinChange();

	Settings.LoadList( L"CHostCacheWnd", &m_wndList );
	Skin.CreateToolBar( L"CHostCacheWnd", &m_wndToolBar );

	CoolInterface.LoadIconsTo( m_gdiImageList, protocolIDs, 0, LVSIL_SMALL, Flags.Width, (Settings.Skin.RowSize > 17 ? (int)Settings.Skin.RowSize - 1 : 16) );
	CoolInterface.LoadFlagsTo( m_gdiImageList );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.SetTextColor( Colors.m_crText );
	m_wndList.SetTextBkColor( Colors.m_crWindow );
	m_wndList.SetBkColor( Colors.m_crWindow );

	//if ( Settings.General.GUIMode == GUI_BASIC )
	//	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_G2;
}

/////////////////////////////////////////////////////////////////////////////
// CHostCacheWnd message handlers

void CHostCacheWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );
	m_pSizer.Resize( cx );
	SizeListAndBar( &m_wndList, &m_wndToolBar );
}

void CHostCacheWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 && IsPartiallyVisible() )
	{
		PROTOCOLID nEffective = m_nMode ? m_nMode : PROTOCOL_G2;

		if ( nEffective != PROTOCOL_G2 &&
			 nEffective != PROTOCOL_G1 &&
			 nEffective != PROTOCOL_ED2K &&
			 nEffective != PROTOCOL_BT &&
			 nEffective != PROTOCOL_DC &&
			 nEffective != PROTOCOL_KAD )
			nEffective = PROTOCOL_G2;

		CHostCacheList* pCache = HostCache.ForProtocol( nEffective );
		DWORD tTicks = GetTickCount();

		// Wait 10 seconds before refreshing; do not force updates
		if ( pCache->m_nCookie != m_nCookie && ( tTicks - m_tLastUpdate ) > 10000 )
			Update();
	}
}

void CHostCacheWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( m_wndList.GetItemOverlayMask( (int)pDraw->nmcd.dwItemSpec ) )
			pDraw->clrText = Colors.m_crSysActiveCaption;

		*pResult = CDRF_DODEFAULT;
	}
}

void CHostCacheWnd::OnDblClkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnHostCacheConnect();
	*pResult = 0;
}

void CHostCacheWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;

	m_wndList.Sort( pNMListView->iSubItem );

	*pResult = 0;
}

void CHostCacheWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	// Do not update the list while user navigates through context menu
	m_bAllowUpdates = FALSE;
	Skin.TrackPopupMenu( L"CHostCacheWnd", point, ID_HOSTCACHE_CONNECT );
	m_bAllowUpdates = TRUE;
}

void CHostCacheWnd::OnNcMouseMove(UINT /*nHitTest*/, CPoint /*point*/)
{
	// Do not update for at least 5 sec while mouse is moving ouside host cache window
	m_bAllowUpdates = FALSE;
}

void CHostCacheWnd::OnUpdateHostCacheConnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CHostCacheWnd::OnHostCacheConnect()
{
	POSITION pos = m_wndList.GetFirstSelectedItemPosition();
	while ( pos )
	{
		const int nItem = m_wndList.GetNextSelectedItem( pos );
		if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
			pHost->ConnectTo();
	}
}

void CHostCacheWnd::OnUpdateHostCacheDisconnect(CCmdUI* pCmdUI)
{
	if ( m_nMode == PROTOCOL_G2 || m_nMode == PROTOCOL_G1 || m_nMode == PROTOCOL_ED2K || m_nMode == PROTOCOL_NULL )
	{
		// Lock Network objects until we are finished with them
		// Note this needs to be locked before the HostCache object to avoid deadlocks with the network thread
		CQuickLock oNetworkLock( Network.m_pSection );

		// Lock HostCache objects until we are finished with them
		CQuickLock oHostCacheLock( HostCache.ForProtocol(
			m_nMode ? m_nMode : PROTOCOL_G2 )->m_pSection );

		POSITION pos = m_wndList.GetFirstSelectedItemPosition();
		while ( pos )
		{
			const int nItem = m_wndList.GetNextSelectedItem( pos );
			if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
			{
				CNeighbour* pNeighbour = Neighbours.Get( pHost->m_pAddress );
				if ( pNeighbour )
				{
					pCmdUI->Enable( TRUE );
					return;
				}
			}
		}
	}
	pCmdUI->Enable( FALSE );
}

void CHostCacheWnd::OnHostCacheDisconnect()
{
	if ( m_nMode == PROTOCOL_G2 || m_nMode == PROTOCOL_G1 || m_nMode == PROTOCOL_ED2K || m_nMode == PROTOCOL_NULL )
	{
		// Lock Network objects until we are finished with them
		// Note this needs to be locked before the HostCache object to avoid deadlocks with the network thread
		CQuickLock oNetworkLock( Network.m_pSection );

		// Lock HostCache objects until we are finished with them
		CQuickLock oHostCacheLock( HostCache.ForProtocol(
			m_nMode ? m_nMode : PROTOCOL_G2 )->m_pSection );

		POSITION pos = m_wndList.GetFirstSelectedItemPosition();
		while ( pos )
		{
			const int nItem = m_wndList.GetNextSelectedItem( pos );
			if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
			{
				CNeighbour* pNeighbour = Neighbours.Get( pHost->m_pAddress );
				if ( pNeighbour )
					pNeighbour->Close();
			}
		}
	}
}

void CHostCacheWnd::OnUpdateHostcachePriority(CCmdUI* pCmdUI)
{
	if ( m_nMode != PROTOCOL_ED2K || m_wndList.GetSelectedCount() == 0 )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
		return;
	}

	CQuickLock oLock( HostCache.ForProtocol( PROTOCOL_ED2K )->m_pSection );

	pCmdUI->Enable( TRUE );

	POSITION pos = m_wndList.GetFirstSelectedItemPosition();
	while ( pos )
	{
		const int nItem = m_wndList.GetNextSelectedItem( pos );
		if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
		{
			if ( pHost->m_bPriority )
			{
				pCmdUI->SetCheck( TRUE );
				return;
			}
		}
	}

	pCmdUI->SetCheck( FALSE );
}

void CHostCacheWnd::OnHostcachePriority()
{
	if ( m_nMode != PROTOCOL_ED2K )
		return;

	CQuickLock oLock( HostCache.ForProtocol( PROTOCOL_ED2K )->m_pSection );

	POSITION pos = m_wndList.GetFirstSelectedItemPosition();
	while ( pos )
	{
		int nItem = m_wndList.GetNextSelectedItem( pos );
		if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
			pHost->m_bPriority = ! pHost->m_bPriority;
	}

	HostCache.eDonkey.m_nCookie ++;

	InvalidateRect( NULL );
	Update();
}

void CHostCacheWnd::OnUpdateNeighboursCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CHostCacheWnd::OnNeighboursCopy()
{
	CQuickLock oLock( HostCache.ForProtocol( m_nMode ? m_nMode : PROTOCOL_G2 )->m_pSection );

	CString strURL;

	CHostCacheHostPtr pHost = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) );
	if ( ! pHost ) return;

	if ( pHost->m_nProtocol == PROTOCOL_G2 || pHost->m_nProtocol == PROTOCOL_G1 )
		strURL.Format( L"gnutella:host:%s:%u", (LPCTSTR)pHost->Address(), pHost->m_nPort );
	else if ( pHost->m_nProtocol == PROTOCOL_ED2K )
		strURL.Format( L"ed2k://|server|%s|%u|/", (LPCTSTR)pHost->Address(), pHost->m_nPort );
	else if ( pHost->m_nProtocol == PROTOCOL_KAD )
		strURL.Format( L"ed2k://|kad|%s|%u|/", (LPCTSTR)pHost->Address(), pHost->m_nUDPPort );
	else if ( pHost->m_nProtocol == PROTOCOL_DC )
		strURL.Format( L"dchub://%s:%u/", (LPCTSTR)pHost->Address(), pHost->m_nUDPPort );
	else if ( pHost->m_nProtocol == PROTOCOL_BT )
		strURL.Format( L"envy:btnode:%s:%u", (LPCTSTR)pHost->Address(), pHost->m_nUDPPort );

	theApp.SetClipboard( strURL );
}

void CHostCacheWnd::OnUpdateHostCacheRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CHostCacheWnd::OnHostCacheRemove()
{
	CQuickLock oLock( HostCache.ForProtocol( m_nMode ? m_nMode : PROTOCOL_G2 )->m_pSection );

	POSITION pos = m_wndList.GetFirstSelectedItemPosition();
	while ( pos )
	{
		const int nItem = m_wndList.GetNextSelectedItem( pos );
		if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
			HostCache.Remove( pHost );
	}

	HostCache.CheckMinimumServers( m_nMode ? m_nMode : PROTOCOL_G2 );

	m_wndList.ClearSelection();

	Update();
}

void CHostCacheWnd::OnUpdateHostcacheG2Horizon(CCmdUI* pCmdUI)
{
	if ( Settings.General.GUIMode == GUI_BASIC )
	{
		if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
			pcCmdUI->Show( FALSE );
		return;
	}

	pCmdUI->SetCheck( m_nMode == PROTOCOL_NULL );
}

void CHostCacheWnd::OnHostcacheG2Horizon()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_NULL;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheG2Cache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_G2 );
}

void CHostCacheWnd::OnHostcacheG2Cache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_G2;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheG1Cache(CCmdUI* pCmdUI)
{
	if ( Settings.Experimental.LAN_Mode || ! Settings.Gnutella1.ShowInterface && ! Settings.Gnutella1.Enabled )
	{
		if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
			pcCmdUI->Show( FALSE );
		return;
	}

	pCmdUI->SetCheck( m_nMode == PROTOCOL_G1 );
}

void CHostCacheWnd::OnHostcacheG1Cache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_G1;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheEd2kCache(CCmdUI* pCmdUI)
{
	if ( Settings.Experimental.LAN_Mode || ! Settings.eDonkey.ShowInterface && ! Settings.eDonkey.Enabled )
	{
		if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
			pcCmdUI->Show( FALSE );
		return;
	}

	pCmdUI->SetCheck( m_nMode == PROTOCOL_ED2K );
}

void CHostCacheWnd::OnHostcacheEd2kCache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_ED2K;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheKADCache(CCmdUI* pCmdUI)
{
	if ( Settings.Experimental.LAN_Mode )	// || ! Settings.KAD.ShowInterface && ! Settings.KAD.Enabled
	{
		if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
			pcCmdUI->Show( FALSE );
		return;
	}

	pCmdUI->SetCheck( m_nMode == PROTOCOL_KAD );
}

void CHostCacheWnd::OnHostcacheKADCache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_KAD;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheDCCache(CCmdUI* pCmdUI)
{
	if ( Settings.Experimental.LAN_Mode || ! Settings.DC.ShowInterface && ! Settings.DC.Enabled )
	{
		if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
			pcCmdUI->Show( FALSE );
		return;
	}

	pCmdUI->SetCheck( m_nMode == PROTOCOL_DC );
}

void CHostCacheWnd::OnHostcacheDCCache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_DC;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheBTCache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_BT );
}

void CHostCacheWnd::OnHostcacheBTCache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_BT;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnHostcacheImport()
{
	// ToDo: Import/Export any host cache list from gui (incl. G1/G2), and Localize it
	CFileDialog dlg( TRUE, L"met", NULL, OFN_HIDEREADONLY,
		L"eDonkey2000 MET files|*.met|"
		L"Kademlia Nodes files|nodes.dat|"
		L"DC++ hub lists|*.xml.bz2|"
	//	L"G2 cache lists|*cache.xml|"
		+ LoadString( IDS_FILES_ALL ) + L"|*.*||", this );

	if ( dlg.DoModal() != IDOK ) return;

	CWaitCursor pCursor;
	HostCache.Import( dlg.GetPathName() );

	Update( TRUE );
}

void CHostCacheWnd::OnHostcacheFileDownload()
{
	CUpdateServersDlg dlg;
	if ( m_nMode == PROTOCOL_DC )
		dlg.m_sURL = Settings.DC.HubListURL;
	//else
	//	dlg.m_sURL = Settings.eDonkey.ServerListURL;
	if ( dlg.DoModal() == IDOK )
		Update( TRUE );
}

BOOL CHostCacheWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_TIMER )
	{
		// Switch updates when window is inactive
		m_bAllowUpdates = IsActive();
	}
	else if ( pMsg->message == WM_KEYDOWN )
	{
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			if ( pMsg->wParam == 'A' )
			{
				for ( int nItem = m_wndList.GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
				{
					m_wndList.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
				}
				return TRUE;
			}
		}
		else if ( pMsg->wParam == VK_DELETE )
		{
			OnHostCacheRemove();
		}
	}
	else if ( pMsg->message == WM_MOUSEWHEEL )
	{
		m_bAllowUpdates = FALSE;
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}

void CHostCacheWnd::RecalcLayout(BOOL bNotify)
{
	m_bAllowUpdates = FALSE;

	CPanelWnd::RecalcLayout(bNotify);
}
