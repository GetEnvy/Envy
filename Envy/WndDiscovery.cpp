//
// WndDiscovery.cpp
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
#include "WndDiscovery.h"
#include "DiscoveryServices.h"
#include "DlgDiscoveryService.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Network.h"
#include "LiveList.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Set Column Order
enum {
	COL_ADDRESS,
	COL_TYPE,
	COL_TIME,
	COL_HOSTS,
	COL_HOSTS_TOTAL,
	COL_URLS,
	COL_URLS_TOTAL,
	COL_ACCESSES,
	COL_UPDATES,
	COL_FAILURES,
	COL_PONG,
	COL_LAST	// Column Count
};

const static UINT nImageID[] =
{
	IDR_HOSTCACHEFRAME,
	IDR_DISCOVERYFRAME,
	IDI_DISCOVERY_BLUE,
	IDI_DISCOVERY_GRAY,
	IDI_WEB_URL,
	IDI_FIREWALLED,
	NULL
};

IMPLEMENT_SERIAL(CDiscoveryWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CDiscoveryWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_DBLCLK, IDC_SERVICES, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVICES, OnSortList)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_QUERY, OnUpdateDiscoveryQuery)
	ON_COMMAND(ID_DISCOVERY_QUERY, OnDiscoveryQuery)
	ON_COMMAND(ID_DISCOVERY_ADD, OnDiscoveryAdd)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_EDIT, OnUpdateDiscoveryEdit)
	ON_COMMAND(ID_DISCOVERY_EDIT, OnDiscoveryEdit)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_REMOVE, OnUpdateDiscoveryRemove)
	ON_COMMAND(ID_DISCOVERY_REMOVE, OnDiscoveryRemove)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_GNUTELLA, OnUpdateDiscoveryGnutella)
	ON_COMMAND(ID_DISCOVERY_GNUTELLA, OnDiscoveryGnutella)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_WEBCACHE, OnUpdateDiscoveryWebcache)
	ON_COMMAND(ID_DISCOVERY_WEBCACHE, OnDiscoveryWebcache)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_SERVERLIST, OnUpdateDiscoveryServerList)
	ON_COMMAND(ID_DISCOVERY_SERVERLIST, OnDiscoveryServerList)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_BLOCKED, OnUpdateDiscoveryBlocked)
	ON_COMMAND(ID_DISCOVERY_BLOCKED, OnDiscoveryBlocked)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_ADVERTISE, OnUpdateDiscoveryAdvertise)
	ON_COMMAND(ID_DISCOVERY_ADVERTISE, OnDiscoveryAdvertise)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_BROWSE, OnUpdateDiscoveryBrowse)
	ON_COMMAND(ID_DISCOVERY_BROWSE, OnDiscoveryBrowse)
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SERVICES, OnCustomDrawList)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd construction

CDiscoveryWnd::CDiscoveryWnd()
	: m_bShowGnutella	( TRUE )
	, m_bShowWebCache	( TRUE )
	, m_bShowServerList	( TRUE )
	, m_bShowBlocked	( TRUE )
{
	Create( IDR_DISCOVERYFRAME );
}

//CDiscoveryWnd::~CDiscoveryWnd()
//{
//}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd message handlers

int CDiscoveryWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) )
		m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_CHILD|WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT|LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_SERVICES, COL_LAST );
	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );
	m_pSizer.Attach( &m_wndList );

//	m_gdiImageList.Create( 16, 16, ILC_MASK|ILC_COLOR32, 4, 1 );
//	AddIcon( IDR_HOSTCACHEFRAME, m_gdiImageList );	// 0
//	AddIcon( IDR_DISCOVERYFRAME, m_gdiImageList );	// 1
//	AddIcon( IDI_DISCOVERY_BLUE, m_gdiImageList );	// 2
//	AddIcon( IDI_DISCOVERY_GRAY, m_gdiImageList );	// 3
//	AddIcon( IDI_WEB_URL, m_gdiImageList );			// 4
//	AddIcon( IDI_FIREWALLED, m_gdiImageList );		// 5
//	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.InsertColumn( COL_ADDRESS,	L"Address",	LVCFMT_LEFT,	260, -1 );
	m_wndList.InsertColumn( COL_TYPE,		L"Type", 	LVCFMT_CENTER,	86, 0 );
	m_wndList.InsertColumn( COL_TIME,		L"Last Access", LVCFMT_CENTER, 130, 1 );
	m_wndList.InsertColumn( COL_HOSTS,		L"Hosts",	LVCFMT_CENTER,	50, 2 );
	m_wndList.InsertColumn( COL_HOSTS_TOTAL, L"Total Hosts", LVCFMT_CENTER, 70, 3 );
	m_wndList.InsertColumn( COL_URLS,		L"URLs", 	LVCFMT_CENTER,	50, 4 );
	m_wndList.InsertColumn( COL_URLS_TOTAL, L"Total URLs", LVCFMT_CENTER, 70, 5 );
	m_wndList.InsertColumn( COL_ACCESSES,	L"Accesses",	LVCFMT_CENTER,	70, 6 );
	m_wndList.InsertColumn( COL_UPDATES,	L"Updates",	LVCFMT_CENTER,	55, 7 );
	m_wndList.InsertColumn( COL_FAILURES,	L"Failures",	LVCFMT_CENTER,	55, 8 );
	m_wndList.InsertColumn( COL_PONG,		L"Pong", 	LVCFMT_CENTER,	150, 9 );

	LoadState( L"CDiscoveryWnd", TRUE );

//	CWaitCursor pCursor;
//	Update();

	return 0;
}

void CDiscoveryWnd::OnDestroy()
{
	DiscoveryServices.Save();

	Settings.SaveList( L"CDiscoveryWnd", &m_wndList );
	SaveState( L"CDiscoveryWnd" );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd operations

void CDiscoveryWnd::Update()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	for ( POSITION pos = DiscoveryServices.GetIterator(); pos; )
	{
		CDiscoveryService* pService = DiscoveryServices.GetNext( pos );

		CLiveItem* pItem = NULL;

		switch ( pService->m_nType )
		{
		case CDiscoveryService::dsGnutella:
			if ( ! m_bShowGnutella ) continue;
			pItem = m_wndList.Add( pService );
			pItem->Set( COL_TYPE, L"Bootstrap" );
			pItem->SetImage( 0 );			// IDR_HOSTCACHEFRAME
			break;
		case CDiscoveryService::dsWebCache:
			if ( ! m_bShowWebCache ) continue;
			pItem = m_wndList.Add( pService );
			if ( pService->m_bGnutella2 && ! pService->m_sPong.IsEmpty() )
				pItem->Set( COL_PONG, pService->m_sPong );
			if ( pService->m_bGnutella2 && ! pService->m_bGnutella1 )
				pItem->Set( COL_TYPE, L"GWebCache2" );
			else if ( pService->m_bGnutella1 && ! pService->m_bGnutella2 )
				pItem->Set( COL_TYPE, L"GWebCache1" );
			else
				pItem->Set( COL_TYPE, L"GWebCache " );
			if ( pService->m_bGnutella2 && pService->m_bGnutella1 )
				pItem->SetImage( 1 );		// IDR_DISCOVERYFRAME Full-colored
			else if ( pService->m_bGnutella2 )
				pItem->SetImage( 2 );		// IDI_DISCOVERY_BLUE
			else if ( pService->m_bGnutella1 )
				pItem->SetImage( 3 );		// IDI_DISCOVERY_GRAY
			else
				pItem->SetImage( 4 );		// Blank?
			break;
		case CDiscoveryService::dsServerList:
			if ( ! m_bShowServerList ) continue;
			pItem = m_wndList.Add( pService );
			pItem->Set( COL_TYPE, pService->m_nProtocolID == PROTOCOL_DC ? L"Hublist" : L"Server.met" );
			pItem->SetImage( 4 );			// IDI_WEB_URL
			break;
		case CDiscoveryService::dsBlocked:
			if ( ! m_bShowBlocked ) continue;
			pItem = m_wndList.Add( pService );
			pItem->Set( COL_TYPE, L"Blocked" );	// ToDo: Translate?
			pItem->SetImage( 5 );			// IDI_FIREWALLED
			break;
		default:
			continue;
		}

		pItem->Set( COL_ADDRESS, pService->m_sAddress );

		if ( pService->m_tAccessed )
		{
			CTime pTime( (time_t)pService->m_tAccessed );
			pItem->Set( COL_TIME, pTime.Format( L"%Y-%m-%d %H:%M:%S" ) );
		}
		else if ( pService->m_nType != CDiscoveryService::dsBlocked )
		{
			pItem->Set( COL_TIME, L" - " );
		}

		if ( pService->m_nType == CDiscoveryService::dsBlocked )
			continue;

		pItem->Format( COL_ACCESSES, L"%u", pService->m_nAccesses );
		pItem->Format( COL_FAILURES, L"%u", pService->m_nFailures );

		if ( pService->m_tAccessed )
		{
			pItem->Format( COL_HOSTS,		L"%u", pService->m_nHosts );
			pItem->Format( COL_HOSTS_TOTAL,	L"%u", pService->m_nTotalHosts );
			pItem->Format( COL_URLS,		L"%u", pService->m_nURLs );
			pItem->Format( COL_URLS_TOTAL,	L"%u", pService->m_nTotalURLs );
			pItem->Format( COL_UPDATES, 	L"%u", pService->m_nUpdates );
		}
	}

	pLock.Unlock();

	m_wndList.Apply();
}

CDiscoveryService* CDiscoveryWnd::GetItem(int nItem)
{
	if ( nItem >= 0 && m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
	{
		CDiscoveryService* pService = (CDiscoveryService*)m_wndList.GetItemData( nItem );
		if ( DiscoveryServices.Check( pService ) )
			return pService;
	}

	return NULL;
}

void CDiscoveryWnd::OnSkinChange()
{
	OnSize( 0, 0, 0 );
	CPanelWnd::OnSkinChange();

	Skin.CreateToolBar( L"CDiscoveryWnd", &m_wndToolBar );

	// Columns
	Settings.LoadList( L"CDiscoveryWnd", &m_wndList, 3 );

	// Fonts & Colors
	m_wndList.SetFont( &theApp.m_gdiFont );
	m_wndList.SetTextColor( Colors.m_crText );
	m_wndList.SetTextBkColor( Colors.m_crWindow );
	m_wndList.SetBkColor( Colors.m_crWindow );

	// Icons
	CoolInterface.LoadIconsTo( m_gdiImageList, nImageID );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}

void CDiscoveryWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( ! m_wndList ) return;

	CPanelWnd::OnSize( nType, cx, cy );

	SizeListAndBar( &m_wndList, &m_wndToolBar );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy - Settings.Skin.ToolbarHeight, SWP_NOZORDER );
}

void CDiscoveryWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )	// && IsPartiallyVisible()
		Update();
}

void CDiscoveryWnd::OnDblClkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnDiscoveryQuery();
	*pResult = 0;
}

void CDiscoveryWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CDiscoveryWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	Skin.TrackPopupMenu( L"CDiscoveryWnd", point, ID_DISCOVERY_EDIT );
}

void CDiscoveryWnd::OnUpdateDiscoveryQuery(CCmdUI* pCmdUI)
{
	if ( m_wndList.GetSelectedCount() == 1 )
	{
		CSingleLock pLock( &Network.m_pSection );
		if ( pLock.Lock( 200 ) )
		{
			CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
			if ( pService && pService->m_nType != CDiscoveryService::dsBlocked )
			{
				pCmdUI->Enable( TRUE );
				return;
			}
		}
	}

	pCmdUI->Enable( FALSE );
}

void CDiscoveryWnd::OnDiscoveryQuery()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! SafeLock( pLock ) )
		return;

	for ( int nItem = -1; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0; )
	{
		CDiscoveryService* pService = GetItem( nItem );

		if ( pService != NULL )
		{
			DiscoveryServices.Execute( pService,
				( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ?
				CDiscoveryServices::wcmCaches : CDiscoveryServices::wcmHosts );
			break;
		}
	}
}

void CDiscoveryWnd::OnUpdateDiscoveryAdvertise(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( pLock.Lock( 100 ) )
	{
		CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
		pCmdUI->Enable( pService && pService->m_nType == CDiscoveryService::dsWebCache );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CDiscoveryWnd::OnDiscoveryAdvertise()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );

	if ( pService )
		DiscoveryServices.Execute( pService, CDiscoveryServices::wcmSubmit );
}

void CDiscoveryWnd::OnUpdateDiscoveryBrowse(CCmdUI* pCmdUI)
{
	OnUpdateDiscoveryAdvertise( pCmdUI );
}

void CDiscoveryWnd::OnDiscoveryBrowse()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	CString strURL;

	if ( pService && pService->m_nType == CDiscoveryService::dsWebCache )
		strURL = pService->m_sAddress;

	pLock.Unlock();

	if ( ! strURL.IsEmpty() )
		ShellExecute( GetSafeHwnd(), L"open", strURL, NULL, NULL, SW_SHOWNORMAL );
}

void CDiscoveryWnd::OnUpdateDiscoveryRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CDiscoveryWnd::OnDiscoveryRemove()
{
	if ( m_wndList.GetSelectedCount() <= 0 ) return;

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 500 ) ) return;

	for ( int nItem = -1; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0; )
	{
		CDiscoveryService* pService = GetItem( nItem );
		if ( pService ) pService->Remove( FALSE );
	}

	DiscoveryServices.CheckMinimumServices();

	m_wndList.ClearSelection();

	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryEdit(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CDiscoveryWnd::OnDiscoveryEdit()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 500 ) ) return;

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( ! pService ) return;

	pLock.Unlock();

	CDiscoveryServiceDlg dlg( NULL, pService );

	if ( dlg.DoModal() == IDOK )
		Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryGnutella(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bShowGnutella );
}

void CDiscoveryWnd::OnDiscoveryGnutella()
{
	m_bShowGnutella = ! m_bShowGnutella;
	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryWebcache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bShowWebCache );
}

void CDiscoveryWnd::OnDiscoveryWebcache()
{
	m_bShowWebCache = ! m_bShowWebCache;
	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryServerList(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bShowServerList );
}

void CDiscoveryWnd::OnDiscoveryServerList()
{
	m_bShowServerList = ! m_bShowServerList;
	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryBlocked(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bShowBlocked );
}

void CDiscoveryWnd::OnDiscoveryBlocked()
{
	m_bShowBlocked = ! m_bShowBlocked;
	Update();
}

void CDiscoveryWnd::OnDiscoveryAdd()
{
	CDiscoveryServiceDlg dlg;

	if ( dlg.DoModal() == IDOK )
		Update();
}

void CDiscoveryWnd::OnCustomDrawList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	*pResult = CDRF_DODEFAULT;
}

BOOL CDiscoveryWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == 'A' && GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			for ( int nItem = m_wndList.GetItemCount() - 1; nItem >= 0; nItem-- )
			{
				m_wndList.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
			}
			return TRUE;
		}
		if ( pMsg->wParam == VK_ESCAPE )
		{
			for ( int nItem = m_wndList.GetItemCount() - 1; nItem >= 0; nItem-- )
			{
				m_wndList.SetItemState( nItem, 0, LVIS_SELECTED );
			}
			return TRUE;
		}
		if ( pMsg->wParam == VK_DELETE )
		{
			PostMessage( WM_COMMAND, ID_DISCOVERY_REMOVE );
			return TRUE;
		}
		if ( pMsg->wParam == VK_INSERT )
		{
			PostMessage( WM_COMMAND, ID_DISCOVERY_ADD );
			return TRUE;
		}
		if ( pMsg->wParam == VK_RETURN )
		{
			PostMessage( WM_COMMAND, ID_DISCOVERY_QUERY );
			return TRUE;
		}
		if ( pMsg->wParam == VK_F2 )
		{
			PostMessage( WM_COMMAND, ID_DISCOVERY_EDIT );
			return TRUE;
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}
