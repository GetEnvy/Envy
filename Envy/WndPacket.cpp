//
// WndPacket.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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
#include "WndPacket.h"
#include "Network.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "LiveList.h"
#include "CoolMenu.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Set Column Order
enum {
	COL_TIME,
	COL_ADDRESS,
	COL_PROTOCOL,
	COL_TYPE,
	COL_HOPS,
	COL_GUID,
	COL_HEX,
	COL_ASCII,
	COL_LAST  // Count
};

// Context menu item detection
#define ID_PAUSE		1
#define ID_TCP			2
#define ID_UDP			3
#define ID_NONE			999
#define ID_BASE_IN		1000
#define ID_BASE_OUT		2000
#define ID_BASE_G1		3000
#define ID_BASE_G2		3100
#define ID_BASE_ED2K	3300
#define ID_BASE_DC		3400
#define ID_BASE_BT		3500
#define ID_BASE_LAST	3600	// Max Value (Other)


IMPLEMENT_SERIAL(CPacketWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CPacketWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PACKETS, OnCustomDrawList)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_CLEAR, OnUpdateSystemClear)
	ON_UPDATE_COMMAND_UI_RANGE(1, ID_BASE_LAST, OnUpdateBlocker)
END_MESSAGE_MAP()

G2_PACKET CPacketWnd::m_nG2[nTypeG2Size] = {
	G2_PACKET_CACHED_HUB,
	G2_PACKET_CRAWL_ANS,
	G2_PACKET_CRAWL_REQ,
	G2_PACKET_DISCOVERY,
	G2_PACKET_HAW,
	G2_PACKET_HIT,
	G2_PACKET_HIT_WRAP,
	G2_PACKET_KHL,
	G2_PACKET_KHL_ANS,
	G2_PACKET_KHL_REQ,
	G2_PACKET_LNI,
	G2_PACKET_PING,
	G2_PACKET_PONG,
	G2_PACKET_PROFILE_CHALLENGE,
	G2_PACKET_PROFILE_DELIVERY,
	G2_PACKET_PUSH,
	G2_PACKET_QHT,
	G2_PACKET_QUERY,
	G2_PACKET_QUERY_ACK,
	G2_PACKET_QUERY_KEY_ANS,
	G2_PACKET_QUERY_KEY_REQ,
	G2_PACKET_QUERY_WRAP
};

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd construction

CPacketWnd::CPacketWnd(CChildWnd* pOwner)
	: m_pOwner		( pOwner )
	, m_pCoolMenu	( NULL )
	, m_bPaused		( FALSE )
	, m_bTCP		( TRUE )
	, m_bUDP		( TRUE )
	, m_bTypeED		( TRUE )
	, m_bTypeDC		( TRUE )
	, m_bTypeBT		( TRUE )
	, m_bTypeOther	( TRUE )
	, m_nInputFilter ( 0 )
	, m_nOutputFilter ( 0 )
{
	for ( int nType = 0 ; nType < nTypeG1Size ; nType++ ) m_bTypeG1[ nType ] = TRUE;
	for ( int nType = 0 ; nType < nTypeG2Size ; nType++ ) m_bTypeG2[ nType ] = TRUE;

	Create( IDR_PACKETFRAME );
}

CPacketWnd::~CPacketWnd()
{
	CSingleLock pLock( &m_pSection, TRUE );

	m_bPaused = TRUE;
	theApp.m_pPacketWnd = NULL;

	for ( POSITION pos = m_pQueue.GetHeadPosition() ; pos ; )
	{
		delete m_pQueue.GetNext( pos );
	}
	m_pQueue.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd create

int CPacketWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_PACKETS );
	m_pSizer.Attach( &m_wndList );

	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );

	CRect rcList;
	m_wndList.GetClientRect( &rcList );

	m_wndList.InsertColumn( COL_TIME,	L"Time", 	LVCFMT_CENTER,	80, -1 );
	m_wndList.InsertColumn( COL_ADDRESS, L"Address", LVCFMT_LEFT,	110, -1 );
	m_wndList.InsertColumn( COL_PROTOCOL, L"Protocol", LVCFMT_CENTER, 80, 0 );
	m_wndList.InsertColumn( COL_TYPE,	L"Type", 	LVCFMT_CENTER,	80, 1 );
	m_wndList.InsertColumn( COL_HOPS,	L"TTL/Hops", LVCFMT_CENTER,	50, 2 );
	m_wndList.InsertColumn( COL_GUID,	L"GUID", 	LVCFMT_LEFT,	50, 5 );
	m_wndList.InsertColumn( COL_HEX,	L"Hex",		LVCFMT_LEFT,	50, 3 );
	m_wndList.InsertColumn( COL_ASCII,	L"ASCII",	LVCFMT_LEFT,	rcList.Width() - 540, 4 );

	LoadState();

	m_bPaused = FALSE;

	SetTimer( 2, 500, NULL );

	theApp.m_pPacketWnd = this;

	return 0;
}

void CPacketWnd::OnDestroy()
{
	m_bPaused = TRUE;
	theApp.m_pPacketWnd = NULL;

	KillTimer( 2 );

	Settings.SaveList( L"CPacketWnd", &m_wndList );
	SaveState( L"CPacketWnd" );

	CPanelWnd::OnDestroy();
}

void CPacketWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	// Columns
	Settings.LoadList( L"CPacketWnd", &m_wndList );

	// Fonts
	if ( m_pFont.m_hObject ) m_pFont.DeleteObject();
	m_pFont.CreateFont( -(int)(Settings.Fonts.DefaultSize /*- 1*/), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.PacketDumpFont );
	m_wndList.SetFont( &m_pFont );
	m_wndList.GetHeaderCtrl()->SetFont( &theApp.m_gdiFont );

	// Colors
	//m_wndList.SetTextColor( Colors.m_crText );
	m_wndList.SetBkColor( Colors.m_crWindow );

	// Icons
	CoolInterface.LoadIconsTo( m_gdiImageList, protocolIDs );
	VERIFY( m_gdiImageList.SetImageCount( m_gdiImageList.GetImageCount() + 2 ) );
	VERIFY( m_gdiImageList.Replace( PROTOCOL_LAST + 0, CoolInterface.ExtractIcon( IDI_INCOMING ) ) != -1 );
	VERIFY( m_gdiImageList.Replace( PROTOCOL_LAST + 1, CoolInterface.ExtractIcon( IDI_OUTGOING ) ) != -1 );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}


/////////////////////////////////////////////////////////////////////////////
// CPacketWnd operations

void CPacketWnd::SmartDump(const CPacket* pPacket, const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique)
{
	if ( m_bPaused || m_hWnd == NULL )
		return;

	if ( bUDP )
	{
		if ( ! m_bUDP )
			return;
	}
	else // TCP filter
	{
		if ( ! m_bTCP )
			return;
	}

	if ( nNeighbourUnique )
	{
		if ( bOutgoing )
		{
			if ( m_nOutputFilter && m_nOutputFilter != nNeighbourUnique )
				return;
		}
		else // Incoming filter
		{
			if ( m_nInputFilter && m_nInputFilter != nNeighbourUnique )
				return;
		}
	}
	else
	{
		if ( bOutgoing )
		{
			if ( m_nOutputFilter )
				return;
		}
		else // Incoming filter
		{
			if ( m_nInputFilter )
				return;
		}
	}

	switch ( pPacket->m_nProtocol )
	{
	case PROTOCOL_G1:
		if ( ! m_bTypeG1[ ((CG1Packet*)pPacket)->m_nTypeIndex ] )
			return;
		break;

	case PROTOCOL_G2:
		for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
		{
			if ( ((CG2Packet*)pPacket)->IsType( m_nG2[ nType ] ) )
			{
				if ( ! m_bTypeG2[ nType ] )
					return;
				break;
			}
		}
		break;

	case PROTOCOL_ED2K:
		// Filter ED2K packets?
		if ( ! m_bTypeED )
			return;
		break;

	case PROTOCOL_DC:
		// Filter DC++ packets?
		if ( ! m_bTypeDC )
			return;
		break;

	case PROTOCOL_BT:
		// Filter BitTorrent packets?
		if ( ! m_bTypeBT )
			return;
		break;

	default:
		// Filter HTTP/FTP packets?
		if ( ! m_bTypeOther )
			return;
	}

	CAutoPtr< CLiveItem > pItem( new CLiveItem( COL_LAST, bOutgoing ) );
	if ( ! pItem ) return;		// Out of memory

	CTime pNow( CTime::GetCurrentTime() );
	CString strNow;
	strNow.Format( L"%0.2i:%0.2i:%0.2i",
		pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond() );
	const CString strAddress( inet_ntoa( pAddress->sin_addr ) );
	const CString strProtocol( protocolAbbr[ pPacket->m_nProtocol ] );

	pItem->Set( COL_TIME,	strNow );
	pItem->Set( COL_ADDRESS, bUDP ? L"(" + strAddress + L")" : strAddress );
	pItem->Set( COL_PROTOCOL, strProtocol + ( bUDP ? L" UDP" : L" TCP" ) );
	pItem->Set( COL_TYPE,	pPacket->GetType() );
	pItem->Set( COL_HEX,	pPacket->ToHex() );
	pItem->Set( COL_ASCII,	pPacket->ToASCII() );

	pItem->SetImage( COL_TIME, PROTOCOL_LAST + ( bOutgoing ? 1 : 0 ) );
	pItem->SetImage( COL_PROTOCOL, pPacket->m_nProtocol );

	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		pItem->Format( COL_HOPS, L"%u/%u", ((CG1Packet*)pPacket)->m_nTTL, ((CG1Packet*)pPacket)->m_nHops );
		pItem->Set( COL_GUID, ((CG1Packet*)pPacket)->GetGUID() );
	}
	else if ( pPacket->m_nLength )
	{
		pItem->Format( COL_GUID, L"(%u)", pPacket->m_nLength );
	}

	CQuickLock pLock( m_pSection );

	if ( ! theApp.m_pPacketWnd )
		return;

	m_pQueue.AddTail( pItem.Detach() );
}

void CPacketWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent != 2 ) return;

	BOOL bScroll = m_wndList.GetTopIndex() + m_wndList.GetCountPerPage() >= m_wndList.GetItemCount();

	CSingleLock pLock( &m_pSection );
	BOOL bAny = FALSE;

	for ( ;; )
	{
		pLock.Lock();

		if ( m_pQueue.GetCount() == 0 ) break;
		CLiveItem* pItem = m_pQueue.RemoveHead();

		pLock.Unlock();

		if ( ! bAny )
			bAny = TRUE;

		if ( (DWORD)m_wndList.GetItemCount() >= Settings.Search.MonitorQueue && Settings.Search.MonitorQueue > 0 )
			m_wndList.DeleteItem( 0 );

		pItem->Add( &m_wndList, -1, 8 );

		delete pItem;
	}

	if ( bAny && bScroll )
		m_wndList.EnsureVisible( m_wndList.GetItemCount() - 1, FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd message handlers

void CPacketWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( ! m_wndList ) return;

	CPanelWnd::OnSize( nType, cx, cy );
	m_pSizer.Resize( cx );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CPacketWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( m_nInputFilter != 1 && m_nOutputFilter != 1 )
		{
			if ( pDraw->nmcd.lItemlParam )
				pDraw->clrText = Colors.m_crNetworkUp;
			else
				pDraw->clrText = Colors.m_crNetworkDown;
			pDraw->clrTextBk = Colors.m_crWindow;
		}
		*pResult = CDRF_DODEFAULT;
	}
}

void CPacketWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu pMenu, pHosts[2], pTypesG1, pTypesG2, pTypesED, pTypesDC, pTypesBT;

	CSingleLock pLock( &Network.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( int nGroup = 0 ; nGroup < 2 ; nGroup++ )
	{
		UINT nID = nGroup ? ID_BASE_OUT : ID_BASE_IN;

		pHosts[nGroup].CreatePopupMenu();

		AddNeighbour( pHosts, nGroup, nID++, 1, L"Disable" );
		AddNeighbour( pHosts, nGroup, nID++, 0, L"Any Neighbour" );
		pHosts[nGroup].AppendMenu( MF_SEPARATOR, ID_SEPARATOR );

		for ( POSITION pos = Neighbours.GetIterator() ; pos ; nID++ )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );
			if ( pNeighbour->m_nState < nrsConnected ) continue;
			AddNeighbour( pHosts, nGroup, nID, (DWORD_PTR)pNeighbour, pNeighbour->m_sAddress );
		}

		if ( ( nID % 1000 ) == 2 )
			pHosts[nGroup].AppendMenu( MF_STRING|MF_GRAYED, ID_NONE, L"No Neighbours" );
	}

	pTypesG1.CreatePopupMenu();
	for ( int nType = 0 ; nType < nTypeG1Size ; nType++ )
	{
		pTypesG1.AppendMenu( MF_STRING|( m_bTypeG1[ nType ] ? MF_CHECKED : 0 ), ID_BASE_G1 + nType, CG1Packet::m_pszPackets[ nType ] );
	}

	pTypesG2.CreatePopupMenu();
	for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
	{
		CStringA tmp;
		tmp.Append( (LPCSTR)&m_nG2[ nType ], G2_TYPE_LEN( m_nG2[ nType ] ) );
		pTypesG2.AppendMenu( MF_STRING|( m_bTypeG2[ nType ] ? MF_CHECKED : 0 ), ID_BASE_G2 + nType, CString( tmp ) );
	}

	pLock.Unlock();

// ToDo: Filter packet types for other networks
//	pTypesED.CreatePopupMenu();
//	pTypesED.AppendMenu( MF_STRING|( m_bTypeED ? MF_CHECKED : 0 ), ID_BASE_ED2K, LoadString( IDS_GENERAL_ALL ) );
//
//	pTypesDC.CreatePopupMenu();
//	pTypesDC.AppendMenu( MF_STRING|( m_bTypeDC ? MF_CHECKED : 0 ), ID_BASE_DC, LoadString( IDS_GENERAL_ALL ) );
//
//	pTypesBT.CreatePopupMenu();
//	pTypesBT.AppendMenu( MF_STRING|( m_bTypeBT ? MF_CHECKED : 0 ), ID_BASE_BT, LoadString( IDS_GENERAL_ALL ) );

	const CString strType = Settings.General.LanguageDefault ? L"Types" : LoadString( IDS_TIP_TYPE );

	pMenu.CreatePopupMenu();
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pHosts[0].GetSafeHmenu(), L"&Incoming" );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pHosts[1].GetSafeHmenu(), L"&Outgoing" );
	pMenu.AppendMenu( MF_SEPARATOR, ID_SEPARATOR );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesG1.GetSafeHmenu(), L"&G1 " + strType );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesG2.GetSafeHmenu(), L"G&2 " + strType );
//	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesED.GetSafeHmenu(), L"&ED2K " + strType );
//	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesDC.GetSafeHmenu(), L"&DC " + strType );
//	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesBT.GetSafeHmenu(), L"&BT " + strType );
	pMenu.AppendMenu( MF_STRING|( m_bTypeED ? MF_CHECKED : 0 ), ID_BASE_ED2K, L"&ED2K " + strType );
	pMenu.AppendMenu( MF_STRING|( m_bTypeDC ? MF_CHECKED : 0 ), ID_BASE_DC, L"&DC " + strType );
	pMenu.AppendMenu( MF_STRING|( m_bTypeBT ? MF_CHECKED : 0 ), ID_BASE_BT, L"&BT " + strType );
	pMenu.AppendMenu( MF_STRING|( m_bTypeOther ? MF_CHECKED : 0 ), ID_BASE_LAST, L"&Other " + strType );
	pMenu.AppendMenu( MF_SEPARATOR, ID_SEPARATOR );
	pMenu.AppendMenu( MF_STRING|( m_bPaused ? MF_CHECKED : 0 ), 1, L"&Pause Display" );
	pMenu.AppendMenu( MF_STRING, ID_SYSTEM_CLEAR, L"&Clear Buffer" );

	m_pCoolMenu = new CCoolMenu();
	m_pCoolMenu->AddMenu( &pMenu, TRUE );

	m_pCoolMenu->SetWatermark( Skin.GetWatermark( L"CCoolMenu" ) );

	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	UINT nCmd = pMenu.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD,
		point.x, point.y, this );

	delete m_pCoolMenu;
	m_pCoolMenu = NULL;

	DWORD_PTR* pModify = NULL;

	switch ( nCmd )
	{
	case ID_PAUSE:
		m_bPaused = ! m_bPaused;
		return;
	case ID_TCP:
		m_bTCP = ! m_bTCP;
		return;
	case ID_UDP:
		m_bUDP = ! m_bUDP;
		return;
	case ID_SYSTEM_CLEAR:
		m_wndList.DeleteAllItems();
		return;
	case ID_BASE_ED2K:	// 3300 range unused
		m_bTypeED = ! m_bTypeED;
		return;
	case ID_BASE_DC:	// 3400 range unused
		m_bTypeDC = ! m_bTypeDC;
		return;
	case ID_BASE_BT:	// 3500 range unused
		m_bTypeBT = ! m_bTypeBT;
		return;
	case ID_BASE_LAST:	// 3600 max unused
		m_bTypeOther = ! m_bTypeOther;
		return;
	//default:	// Detect specific types below?
	}

	if ( nCmd >= ID_BASE_G1 && nCmd < ID_BASE_G1 + nTypeG1Size )	// 3000 range
	{
		nCmd -= ID_BASE_G1;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			for ( int nType = 0 ; nType < nTypeG1Size ; nType++ )
			{
				m_bTypeG1[ nType ] = ( nCmd == (UINT)nType ) ? TRUE : FALSE;
			}
		}
		else
		{
			m_bTypeG1[ nCmd ] = ! m_bTypeG1[ nCmd ];
		}

		return;
	}

	if ( nCmd >= ID_BASE_G2 && nCmd < ID_BASE_G2 + nTypeG2Size )	// 3100 range
	{
		nCmd -= ID_BASE_G2;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
			{
				m_bTypeG2[ nType ] = ( nCmd == (UINT)nType ) ? TRUE : FALSE;
			}
		}
		else
		{
			m_bTypeG2[ nCmd ] = ! m_bTypeG2[ nCmd ];
		}

		return;
	}

	if ( nCmd >= ID_BASE_IN && nCmd < ID_BASE_OUT )			// 1000 range
	{
		pModify = &m_nInputFilter;
		nCmd -= ID_BASE_IN;
	}
	else if ( nCmd >= ID_BASE_OUT && nCmd < ID_BASE_G1 )	// 2000 range
	{
		pModify = &m_nOutputFilter;
		nCmd -= ID_BASE_OUT;
	}
	else
	{
		return;
	}

	if ( nCmd == 0 )
	{
		*pModify = 1;
	}
	else if ( nCmd == ID_PAUSE )
	{
		*pModify = 0;
	}
	else if ( SafeLock( pLock ) )
	{
		nCmd -= 2;
		for ( POSITION pos = Neighbours.GetIterator() ; pos ; nCmd-- )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );
			if ( ! nCmd )
			{
				*pModify = (DWORD_PTR)pNeighbour;
				break;
			}
		}
		pLock.Unlock();
	}

	Invalidate();
}

void CPacketWnd::AddNeighbour(CMenu* pMenus, int nGroup, UINT nID, DWORD_PTR nTarget, LPCTSTR pszText)
{
	UINT nChecked = ( ( nGroup == 1 && m_nOutputFilter == nTarget ) ||
		 ( nGroup == 0 && m_nInputFilter == nTarget ) ) ? MF_CHECKED : 0;

	pMenus[nGroup].AppendMenu( MF_STRING|nChecked, nID, pszText );
}

void CPacketWnd::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( m_pCoolMenu )
		m_pCoolMenu->OnMeasureItem( lpMeasureItemStruct );
}

void CPacketWnd::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( m_pCoolMenu )
		m_pCoolMenu->OnDrawItem( lpDrawItemStruct );
}

void CPacketWnd::OnUpdateBlocker(CCmdUI* pCmdUI)
{
	if ( m_pCoolMenu )
		pCmdUI->Enable( pCmdUI->m_nID != ID_NONE );
	else
		pCmdUI->ContinueRouting();
}

void CPacketWnd::OnUpdateSystemClear(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
}
