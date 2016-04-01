//
// WndNeighbours.cpp
//
// This file is part of Envy (getenvy.com) © 2016
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
#include "Network.h"
#include "Neighbours.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "DCNeighbour.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "HostCache.h"
#include "Security.h"
#include "LiveList.h"
#include "GProfile.h"
#include "ChatCore.h"
#include "ChatWindows.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Flags.h"
#include "Skin.h"

#include "WndMain.h"
#include "WndNeighbours.h"
#include "WndPacket.h"
#include "WndBrowseHost.h"
#include "WindowManager.h"
#include "WndSystem.h"
#include "DlgSettingsManager.h"

#ifdef _DEBUG
#include "DlgHex.h"
#endif


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Set Column Order
enum {
	COL_ADDRESS,
	COL_PORT,
	COL_TIME,
	COL_TRAFFIC,
	COL_TOTAL,
	COL_PACKETS,
	COL_FLOW,
	COL_LEAVES,
	COL_MODE,
	COL_CLIENT,
	COL_NAME,
	COL_COUNTRY,
	COL_LAST  // Column Count
};


IMPLEMENT_SERIAL(CNeighboursWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CNeighboursWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_WM_CONTEXTMENU()
	ON_WM_QUERYNEWPALETTE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_NEIGHBOURS, OnSortList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_NEIGHBOURS, OnCustomDrawList)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_DISCONNECT, OnUpdateNeighboursDisconnect)
	ON_COMMAND(ID_NEIGHBOURS_DISCONNECT, OnNeighboursDisconnect)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_VIEW_ALL, OnUpdateNeighboursViewAll)
	ON_COMMAND(ID_NEIGHBOURS_VIEW_ALL, OnNeighboursViewAll)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_VIEW_INCOMING, OnUpdateNeighboursViewIncoming)
	ON_COMMAND(ID_NEIGHBOURS_VIEW_INCOMING, OnNeighboursViewIncoming)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_VIEW_OUTGOING, OnUpdateNeighboursViewOutgoing)
	ON_COMMAND(ID_NEIGHBOURS_VIEW_OUTGOING, OnNeighboursViewOutgoing)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_CHAT, OnUpdateNeighboursChat)
	ON_COMMAND(ID_NEIGHBOURS_CHAT, OnNeighboursChat)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_BAN, OnUpdateSecurityBan)
	ON_COMMAND(ID_SECURITY_BAN, OnSecurityBan)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_URI, OnUpdateNeighboursCopy)
	ON_COMMAND(ID_NEIGHBOURS_URI, OnNeighboursCopy)
	ON_COMMAND(ID_NEIGHBOURS_SETTINGS, OnNeighboursSettings)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNeighboursWnd construction

CNeighboursWnd::CNeighboursWnd()
	: CPanelWnd( TRUE, TRUE )
//	, m_tLastUpdate( 0 )	// Using static
{
	Create( IDR_NEIGHBOURSFRAME );
}

UINT CNeighboursWnd::GetSelectedCount() const
{
	static UINT nCount = 0;
	static DWORD tLastUpdate = 0;
	const DWORD tNow = GetTickCount();
	if ( tNow > tLastUpdate + Settings.Interface.RefreshRateUI || tNow < tLastUpdate )
	{
		tLastUpdate = tNow;
		nCount = m_wndList.GetSelectedCount();
	}
	return nCount;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighboursWnd system message handlers

int CNeighboursWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_NEIGHBOURS );
	m_pSizer.Attach( &m_wndList );

	m_wndTip.Create( &m_wndList, &Settings.Interface.TipNeighbours );
	m_wndList.SetTip( &m_wndTip );

// Note: Workaround fix in LiveList for VS2012
//#ifdef _USING_V110_SDK71_	// #if defined(_MSC_VER) && (_MSC_VER >= 1700)
//	m_wndList.ModifyStyleEx( 0, WS_EX_COMPOSITED );		// Stop flicker XP+, CPU intensive (Only needed here when targeting VS2012)
//#endif

	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );

	m_wndList.InsertColumn( COL_ADDRESS, L"Address",	LVCFMT_LEFT,	110 );
	m_wndList.InsertColumn( COL_PORT,	 L"Port", 	LVCFMT_CENTER,	 42 );
	m_wndList.InsertColumn( COL_TIME,	 L"Time",	LVCFMT_CENTER,	 56 );
	m_wndList.InsertColumn( COL_TRAFFIC, L"Traffic", LVCFMT_CENTER,	 84 );
	m_wndList.InsertColumn( COL_TOTAL,	 L"Total",	LVCFMT_CENTER,	 96 );
	m_wndList.InsertColumn( COL_PACKETS, L"Packets",	LVCFMT_CENTER,	 70 );
	m_wndList.InsertColumn( COL_FLOW,	 L"Flow", 	LVCFMT_CENTER,	  0 );
	m_wndList.InsertColumn( COL_LEAVES,	 L"Leaves",	LVCFMT_CENTER,	 52 );
	m_wndList.InsertColumn( COL_MODE,	 L"Mode", 	LVCFMT_CENTER,	 84 );
	m_wndList.InsertColumn( COL_CLIENT,	 L"Client",	LVCFMT_LEFT,	110 );
	m_wndList.InsertColumn( COL_NAME,	 L"Name", 	LVCFMT_LEFT,	100 );
	m_wndList.InsertColumn( COL_COUNTRY, L"Country",	LVCFMT_LEFT,	 54 );

	//CLiveList::Sort( &m_wndList, COL_MODE );	// Does not work

// Obsolete for reference:
//	CBitmap bmImages;
//	//bmProtocols.LoadBitmap( IDB_PROTOCOLS );
//
//	CImageFile pFile;
//	HBITMAP hBitmap;
//	pFile.LoadFromResource( AfxGetResourceHandle(), IDB_PROTOCOLS, RT_PNG );
//	hBitmap = pFile.CreateBitmap();
//	bmImages.Attach( hBitmap );
//	if ( Settings.General.LanguageRTL )
//		bmImages.m_hObject = CreateMirroredBitmap( (HBITMAP)bmImages.m_hObject );
//
//	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 7, 1 ) ||
//	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 7, 1 ) ||
//	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 7, 1 );
//	m_gdiImageList.Add( &bmImages, RGB( 0, 255, 0 ) );
//	bmImages.DeleteObject();
//
//	const int nImages = m_gdiImageList.GetImageCount();
//	const int nFlags = Flags.m_pImage.GetImageCount();
//	VERIFY( m_gdiImageList.SetImageCount( nImages + nFlags ) );
//
//	for ( int nFlag = 0 ; nFlag < nFlags ; nFlag++ )
//	{
//		if ( HICON hIcon = Flags.m_pImage.ExtractIcon( nFlag ) )
//		{
//			VERIFY( m_gdiImageList.Replace( nImages + nFlag, hIcon ) != -1 );
//			VERIFY( DestroyIcon( hIcon ) );
//		}
//	}
//
//	// Merge protocols and flags in one image list (OnSkin)
//	CoolInterface.LoadIconsTo( m_gdiImageList, protocolIDs );
//	CoolInterface.LoadFlagsTo( m_gdiImageList );
//	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	LoadState( L"CNeighboursWnd", FALSE );

	PostMessage( WM_TIMER, 1 );

	return 0;
}

void CNeighboursWnd::OnDestroy()
{
	Settings.SaveList( L"CNeighboursWnd", &m_wndList );
	SaveState( L"CNeighboursWnd" );
	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CNeighboursWnd operations

void CNeighboursWnd::Update()
{
	static DWORD tLastUpdate = 0;
	const DWORD tNow = GetTickCount();

	if ( tNow < tLastUpdate + 30 || ( ! IsPartiallyVisible() && tNow < tLastUpdate + 30000 ) )
		return;

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	CLiveList pLiveList( COL_LAST );

	tLastUpdate = tNow;		// Was m_tLastUpdate

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CString str;
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		CLiveItem* pItem = pLiveList.Add( pNeighbour );

		pItem->Set( COL_ADDRESS, L" " + pNeighbour->m_sAddress );
		pItem->Format( COL_PORT, L"%hu", htons( pNeighbour->m_pHost.sin_port ) );

		const DWORD nTime = ( tLastUpdate - pNeighbour->m_tConnected ) / 1000;

		switch ( pNeighbour->m_nState )
		{
		case nrsConnecting:
			LoadString( str, IDS_NEIGHBOUR_CONNECTING );
			break;
		case nrsHandshake1:
		case nrsHandshake2:
		case nrsHandshake3:
			LoadString( str, IDS_NEIGHBOUR_HANDSHAKING );
			break;
		case nrsRejected:
			LoadString( str, IDS_NEIGHBOUR_REJECTED );
			break;
		case nrsClosing:
			LoadString( str, IDS_NEIGHBOUR_CLOSING );
			break;
		case nrsConnected:
			if ( nTime > 86400 )
				str.Format( L"%u:%.2u:%.2u:%.2u", nTime / 86400, ( nTime / 3600 ) % 24, ( nTime / 60 ) % 60, nTime % 60 );
			else
				str.Format( L"%u:%.2u:%.2u", nTime / 3600, ( nTime / 60 ) % 60, nTime % 60 );
			break;
		case nrsNull:
		default:
			LoadString( str, IDS_NEIGHBOUR_UNKNOWN );
			break;
		}

		pItem->Set( COL_TIME, str );

		pNeighbour->Measure();

		pItem->Format( COL_TRAFFIC, L"%s - %s",
			Settings.SmartSpeed( pNeighbour->m_mInput.nMeasure ),
			Settings.SmartSpeed( pNeighbour->m_mOutput.nMeasure ) );
		pItem->Format( COL_TOTAL, L"%s - %s",
			Settings.SmartVolume( pNeighbour->m_mInput.nTotal ),
			Settings.SmartVolume( pNeighbour->m_mOutput.nTotal ) );
		pItem->Format( COL_PACKETS, L"%u - %u", pNeighbour->m_nInputCount, pNeighbour->m_nOutputCount );
		pItem->Format( COL_FLOW, L"%u (%u)", pNeighbour->m_nOutbound, pNeighbour->m_nLostCount );

		pItem->Set( COL_CLIENT, pNeighbour->m_sUserAgent );

		if ( pNeighbour->m_nState >= nrsConnected )
		{
			pItem->SetImage( pNeighbour->m_nProtocol );

			if ( pNeighbour->GetUserCount() )
			{
				if ( pNeighbour->GetUserLimit() )
					pItem->Format( COL_LEAVES, L"%u/%u", pNeighbour->GetUserCount(), pNeighbour->GetUserLimit() );
				else
					pItem->Format( COL_LEAVES, L"%u", pNeighbour->GetUserCount() );
			}

			if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
			{
			//	CG1Neighbour* pG1 = reinterpret_cast<CG1Neighbour*>(pNeighbour);

				switch ( pNeighbour->m_nNodeType )
				{
				case ntNode:
					LoadString( str, IDS_NEIGHBOUR_G1PEER );
					break;
				case ntHub:
					LoadString( str, IDS_NEIGHBOUR_G1ULTRA );
					break;
				case ntLeaf:
					LoadString( str, IDS_NEIGHBOUR_G1LEAF );
					break;
				}
				pItem->Set( COL_MODE, str );

				pItem->Set( COL_LEAVES, L"-" );		// Note leaves/peers count is only returned to crawler header
			}
			else if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
			{
				CG2Neighbour* pG2 = static_cast<CG2Neighbour*>(pNeighbour);

				switch ( pG2->m_nNodeType )
				{
				case ntNode:
					LoadString( str, IDS_NEIGHBOUR_G2PEER );
					break;
				case ntHub:
					LoadString( str, IDS_NEIGHBOUR_G2HUB );
					break;
				case ntLeaf:
					LoadString( str, IDS_NEIGHBOUR_G2LEAF );
					break;
				}
				pItem->Set( COL_MODE, str );
			}
			else if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
			{
				CEDNeighbour* pED2K = static_cast<CEDNeighbour*>(pNeighbour);

				LoadString( str, pED2K->m_nClientID > 0 ? CEDPacket::IsLowID( pED2K->m_nClientID ) ?
					IDS_NEIGHBOUR_ED2K_LOWID : IDS_NEIGHBOUR_ED2K_HIGHID : IDS_NEIGHBOUR_ED2K_SERVER );
				pItem->Set( COL_CLIENT, str );

				pItem->Set( COL_MODE, L"eDonkey2000" );
			}
			else if ( pNeighbour->m_nProtocol == PROTOCOL_DC )
			{
				//CDCNeighbour* pDC = static_cast< CDCNeighbour* >( pNeighbour );

				pItem->Set( COL_MODE, L"NMDC Hub" );		// ToDo: Support ADC mode hubs (adc://)
			}
		}

		pItem->Set( COL_NAME, pNeighbour->m_pProfile ? pNeighbour->m_pProfile->GetNick() : pNeighbour->m_sServerName );

		pItem->Set( COL_COUNTRY, pNeighbour->m_sCountry );
		const int nFlagIndex = Flags.GetFlagIndex( pNeighbour->m_sCountry );
		if ( nFlagIndex >= 0 )
			pItem->SetImage( COL_COUNTRY, PROTOCOL_LAST + nFlagIndex );
	}

	pLiveList.Apply( &m_wndList, TRUE );

	tLastUpdate = GetTickCount();
}

CNeighbour* CNeighboursWnd::GetItem(int nItem)
{
	if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
		return Neighbours.Get( m_wndList.GetItemData( nItem ) );

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighboursWnd message handlers

BOOL CNeighboursWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndToolBar.m_hWnd )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}

	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CNeighboursWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );

	BOOL bSized = m_pSizer.Resize( cx );

	SizeListAndBar( &m_wndList, &m_wndToolBar );

	if ( bSized && m_wndList.GetItemCount() == 0 )
		m_wndList.Invalidate();
}

void CNeighboursWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
		Update();
}

void CNeighboursWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CNeighboursWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	Skin.TrackPopupMenu( L"CNeighboursWnd", point );
}

void CNeighboursWnd::OnUpdateNeighboursDisconnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CNeighboursWnd::OnNeighboursDisconnect()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
	{
		if ( CNeighbour* pNeighbour = GetItem( nItem ) )
			pNeighbour->Close( IDS_CONNECTION_CLOSED );
	}
}

void CNeighboursWnd::OnUpdateNeighboursCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CNeighboursWnd::OnNeighboursCopy()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) );
	if ( ! pNeighbour ) return;

	CString strURL;

	if ( pNeighbour->m_nProtocol == PROTOCOL_G2 || pNeighbour->m_nProtocol == PROTOCOL_G1 )
	{
		strURL.Format( L"gnutella:host:%s:%u",
			(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );
	}
	else if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
	{
		strURL.Format( L"ed2k://|server|%s|%u|/",
			(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );
	}
	else if ( pNeighbour->m_nProtocol == PROTOCOL_DC )
	{
		strURL.Format( L"dchub://%s:%u/",
			(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );
	}

	theApp.SetClipboard( strURL );
}

void CNeighboursWnd::OnUpdateNeighboursChat(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	if ( Settings.Community.ChatEnable && GetSelectedCount() == 1 )
	{
		CSingleLock pNetworkLock( &Network.m_pSection );
		if ( pNetworkLock.Lock( 250 ) )
		{
			if ( CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) )
			{
				bEnable = (
					pNeighbour->m_nProtocol == PROTOCOL_G2 ||
					pNeighbour->m_nProtocol == PROTOCOL_G1 ||
					pNeighbour->m_nProtocol == PROTOCOL_DC );
			}
			pNetworkLock.Unlock();
		}
	}
	pCmdUI->Enable( bEnable );
}

void CNeighboursWnd::OnNeighboursChat()
{
	if ( ! Settings.Community.ChatEnable || ! GetSelectedCount() )
		return;

	CSingleLock pLock( &Network.m_pSection, TRUE );

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
	{
		if ( CNeighbour* pNeighbour = GetItem( nItem ) )
		{
			if ( pNeighbour->m_nProtocol == PROTOCOL_G2 ||
				 pNeighbour->m_nProtocol == PROTOCOL_G1 )
				ChatWindows.OpenPrivate( pNeighbour->m_oGUID, &pNeighbour->m_pHost, FALSE, pNeighbour->m_nProtocol );
			else if ( pNeighbour->m_nProtocol == PROTOCOL_DC )
				ChatCore.OnMessage( static_cast< CDCNeighbour*>( pNeighbour ) );
		}
	}
}

void CNeighboursWnd::OnUpdateSecurityBan(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() );
}

void CNeighboursWnd::OnSecurityBan()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
	{
		if ( CNeighbour* pNeighbour = GetItem( nItem ) )
		{
			IN_ADDR pAddress = pNeighbour->m_pHost.sin_addr;
			pNeighbour->Close();
			pLock.Unlock();
			Security.Ban( &pAddress, banSession );
			pLock.Lock();
		}
	}
}

void CNeighboursWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	if ( GetSelectedCount() == 1 )
	{
		CSingleLock pNetworkLock( &Network.m_pSection );
		if ( pNetworkLock.Lock( 250 ) )
		{
			if ( CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) )
			{
				bEnable = (
					pNeighbour->m_nProtocol == PROTOCOL_G2 ||
					pNeighbour->m_nProtocol == PROTOCOL_G1 );
			}
		}
		pNetworkLock.Unlock();
	}
	pCmdUI->Enable( bEnable );
}

void CNeighboursWnd::OnBrowseLaunch()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	if ( CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) )
	{
		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 ||
			 pNeighbour->m_nProtocol == PROTOCOL_G2 )
		{
			PROTOCOLID nProtocol = pNeighbour->m_nProtocol;
			SOCKADDR_IN pAddress = pNeighbour->m_pHost;
			Hashes::Guid oGUID = pNeighbour->m_oGUID;

			pLock.Unlock();

			new CBrowseHostWnd( nProtocol, &pAddress, FALSE, oGUID );
		}
	}
}

void CNeighboursWnd::OnUpdateNeighboursViewAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CNeighboursWnd::OnNeighboursViewAll()
{
	OpenPacketWnd( TRUE, TRUE );
}

void CNeighboursWnd::OnUpdateNeighboursViewIncoming(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CNeighboursWnd::OnNeighboursViewIncoming()
{
	OpenPacketWnd( TRUE, FALSE );
}

void CNeighboursWnd::OnUpdateNeighboursViewOutgoing(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CNeighboursWnd::OnNeighboursViewOutgoing()
{
	OpenPacketWnd( FALSE, TRUE );
}

void CNeighboursWnd::OnNeighboursSettings()
{
	CSettingsManagerDlg::Run( L"CNetworksSettingsPage" );
}

void CNeighboursWnd::OpenPacketWnd(BOOL bIncoming, BOOL bOutgoing)
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CWindowManager* pManager = GetManager();
	CPacketWnd* pWnd = NULL;

	while ( ( pWnd = (CPacketWnd*)pManager->Find( RUNTIME_CLASS(CPacketWnd), pWnd ) ) != NULL )
	{
		if ( pWnd->m_pOwner == this ) break;
	}

	if ( ! pWnd ) pWnd = new CPacketWnd( this );

	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		if ( CNeighbour* pNeighbour = GetItem( nItem ) )
		{
			pWnd->m_nInputFilter  = bIncoming ? (DWORD_PTR)pNeighbour : 1;
			pWnd->m_nOutputFilter = bOutgoing ? (DWORD_PTR)pNeighbour : 1;
		}
	}

	pWnd->m_bPaused = FALSE;
	pWnd->BringWindowToTop();
}

void CNeighboursWnd::OnSkinChange()
{
	OnSize( 0, 0, 0 );
	CPanelWnd::OnSkinChange();

	// Columns, Toolbar, Font
	Settings.LoadList( L"CNeighboursWnd", &m_wndList );
	Skin.CreateToolBar( L"CNeighboursWnd", &m_wndToolBar );
	m_wndList.SetFont( &theApp.m_gdiFont );

	CoolInterface.LoadIconsTo( m_gdiImageList, protocolIDs );
	CoolInterface.LoadFlagsTo( m_gdiImageList );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	if ( m_wndList.SetBkImage( Skin.GetWatermark( L"CNeighboursWnd" ) ) || m_wndList.SetBkImage( Skin.GetWatermark( L"System.Windows" ) ) )	// Images.m_bmSystemWindow.m_hObject
		m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );	// No LVS_EX_DOUBLEBUFFER	-LVS_EX_TRANSPARENTBKGND ?
	else
		m_wndList.SetBkColor( Colors.m_crWindow );

	// Update Dropshadow
	m_wndTip.DestroyWindow();
	m_wndTip.Create( this, &Settings.Interface.TipNeighbours );
}

void CNeighboursWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	if ( ! ::IsWindow( m_wndList.GetSafeHwnd() ) ) return;

	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		if ( m_wndList.GetItemCount() == 0 && ! Network.IsConnected() )
			DrawEmptyMessage( CDC::FromHandle( pDraw->nmcd.hdc ) );

		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		LV_ITEM pItem = { LVIF_IMAGE, static_cast< int >( pDraw->nmcd.dwItemSpec ) };
		m_wndList.GetItem( &pItem );

		if ( m_wndList.GetBkColor() == Colors.m_crWindow )
			pDraw->clrTextBk = Colors.m_crWindow;

		int nImage = pItem.iImage;
		switch ( nImage )
		{
		case PROTOCOL_NULL:
			pDraw->clrText = Colors.m_crNetworkNull;
			break;
		case PROTOCOL_G1:
			pDraw->clrText = Colors.m_crNetworkG1;
			break;
		case PROTOCOL_G2:
			pDraw->clrText = Colors.m_crNetworkG2;
			break;
		case PROTOCOL_ED2K:
			pDraw->clrText = Colors.m_crNetworkED2K;
			break;
		case PROTOCOL_DC:
			pDraw->clrText = Colors.m_crNetworkDC;
			break;
		}

		*pResult = CDRF_DODEFAULT;
	}
}

void CNeighboursWnd::DrawEmptyMessage(CDC* pDC)
{
	CRect rcClient, rcText;
	CString strText;

	m_wndList.GetClientRect( &rcClient );

	if ( CWnd* pHeader = m_wndList.GetWindow( GW_CHILD ) )
	{
		pHeader->GetWindowRect( &rcText );
		rcClient.top += rcText.Height();
	}

	rcText.SetRect( rcClient.left, 16, rcClient.right, 0 );
	rcText.bottom = ( rcClient.top + rcClient.bottom ) / 2;
	rcText.top = rcText.bottom - rcText.top;

	pDC->SetBkMode( TRANSPARENT );
	CFont* pOldFont = (CFont*)pDC->SelectObject( &theApp.m_gdiFont );
	pDC->SetTextColor( Colors.m_crText );
	LoadString( strText, IDS_NEIGHBOURS_NOT_CONNECTED );
	pDC->DrawText( strText, &rcText, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX );

	rcText.OffsetRect( 0, rcText.Height() );

	LoadString( strText, IDS_NEIGHBOURS_CONNECT );
	pDC->DrawText( strText, &rcText, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX );

	pDC->SelectObject( pOldFont );
}

BOOL CNeighboursWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		switch ( pMsg->wParam )
		{
		case VK_TAB:
			GetManager()->Open( RUNTIME_CLASS(CSystemWnd) );
			return TRUE;
		case VK_DELETE:
			if ( GetSelectedCount() )
				OnNeighboursDisconnect();
			return TRUE;
		case 'C':
			if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0 && GetSelectedCount() == 1 )
				OnNeighboursCopy();
			return TRUE;
#ifdef _DEBUG
		case 'H':
			if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0 )
			{
				CSingleLock pLock( &Network.m_pSection, TRUE );
				if ( CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) )
				{
					pLock.Unlock();

					CHexDlg dlg;
					if ( dlg.DoModal() == IDOK )
					{
						pLock.Lock();

						BOOL bResult = pNeighbour->ProcessPackets( dlg.GetData() );

						pLock.Unlock();

						if ( bResult )
							MsgBox( L"Packet was successfully processed." );
						else
							MsgBox( L"Packet was rejected." );
					}
				}
			}
			return TRUE;
#endif
		}
	}

	return CPanelWnd::PreTranslateMessage(pMsg);
}

void CNeighboursWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CPanelWnd::OnActivate(nState, pWndOther, bMinimized);
	Update();
}
