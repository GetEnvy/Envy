//
// WndSearchMonitor.cpp
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
#include "WndSearchMonitor.h"
#include "WndSearch.h"
#include "WndBrowseHost.h"
#include "QuerySearch.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "LiveList.h"
#include "Security.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Set Column Order
enum {
	COL_SEARCH,
//	COL_URN,
//	COL_SIZE,
	COL_SCHEMA,
//	COL_NETWORK,
	COL_ENDPOINT,
	COL_LAST	// Count
};

const static UINT nImageID[] =
{
	IDR_SEARCHMONITORFRAME,
	NULL
};

IMPLEMENT_SERIAL(CSearchMonitorWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSearchMonitorWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_SEARCHMONITOR_PAUSE, OnUpdateSearchMonitorPause)
	ON_COMMAND(ID_SEARCHMONITOR_PAUSE, OnSearchMonitorPause)
	ON_COMMAND(ID_SEARCHMONITOR_CLEAR, OnSearchMonitorClear)
	ON_UPDATE_COMMAND_UI(ID_HITMONITOR_SEARCH, OnUpdateSearchMonitorSearch)
	ON_COMMAND(ID_HITMONITOR_SEARCH, OnSearchMonitorSearch)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_BAN, OnUpdateSecurityBan)
	ON_COMMAND(ID_SECURITY_BAN, OnSecurityBan)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SEARCHES, OnDblClkList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SEARCHES, OnCustomDrawList)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSearchMonitorWnd construction

CSearchMonitorWnd::CSearchMonitorWnd()
	: m_bPaused	( FALSE )
{
	Create( IDR_SEARCHMONITORFRAME );
}

//CSearchMonitorWnd::~CSearchMonitorWnd()
//{
//}

/////////////////////////////////////////////////////////////////////////////
// CSearchMonitorWnd message handlers

int CSearchMonitorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndList.Create( WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_CHILD|WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT|LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_SEARCHES );
	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );
	m_pSizer.Attach( &m_wndList );

//	VERIFY( m_gdiImageList.Create( 16, 16, ILC_MASK|ILC_COLOR32, 1, 1 ) );
//	AddIcon( IDR_SEARCHMONITORFRAME, m_gdiImageList );
//	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
//	m_wndList.SetFont( &theApp.m_gdiFont );

	m_wndList.InsertColumn( COL_SEARCH, L"Search", LVCFMT_LEFT, 400, -1 );
//	m_wndList.InsertColumn( COL_URN, L"URN", LVCFMT_LEFT, 340, 0 );
//	m_wndList.InsertColumn( COL_SIZE, L"Size", LVCFMT_LEFT, 100, 1 );
	m_wndList.InsertColumn( COL_SCHEMA, L"Schema", LVCFMT_LEFT, 150, 1 );
//	m_wndList.InsertColumn( COL_NETWORK, L"Network", LVCFMT_LEFT, 60, 3 );
	m_wndList.InsertColumn( COL_ENDPOINT, L"Endpoint", LVCFMT_LEFT, 150, 2 );

	LoadState( L"CSearchMonitorWnd", TRUE );

//	m_bPaused = FALSE;
	SetTimer( 2, 250, NULL );

	return 0;
}

void CSearchMonitorWnd::OnDestroy()
{
	KillTimer( 2 );

	{
		CSingleLock pLock( &m_pSection, TRUE );

		m_bPaused = TRUE;

		for ( POSITION pos = m_pQueue.GetHeadPosition() ; pos ; )
		{
			delete m_pQueue.GetNext( pos );
		}
		m_pQueue.RemoveAll();
	}

	Settings.SaveList( L"CSearchMonitorWnd", &m_wndList );
	SaveState( L"CSearchMonitorWnd" );

	CPanelWnd::OnDestroy();
}

void CSearchMonitorWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	// Columns
	Settings.LoadList( L"CSearchMonitorWnd", &m_wndList );

	// Fonts & Colors
	m_wndList.SetFont( &theApp.m_gdiFont );
	m_wndList.SetTextColor( Colors.m_crText );
	m_wndList.SetTextBkColor( Colors.m_crWindow );
	m_wndList.SetBkColor( Colors.m_crWindow );

	// Icons
	CoolInterface.LoadIconsTo( m_gdiImageList, nImageID );	// IDR_SEARCHMONITORFRAME
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}

void CSearchMonitorWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( ! m_wndList ) return;

	CPanelWnd::OnSize( nType, cx, cy );
	m_pSizer.Resize( cx );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CSearchMonitorWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	Skin.TrackPopupMenu( L"CSearchMonitorWnd", point, ID_HITMONITOR_SEARCH );
}

void CSearchMonitorWnd::OnUpdateSearchMonitorSearch(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CSearchMonitorWnd::OnSearchMonitorSearch()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );

	if ( nItem >= 0 )
	{
		CQuerySearchPtr pSearch = new CQuerySearch();
		pSearch->m_sSearch = m_wndList.GetItemText( nItem, 0 );

		if ( pSearch->m_sSearch.IsEmpty() ||
			 _tcscmp( pSearch->m_sSearch, L"\\" ) == 0 )
		{
			pSearch->m_sSearch = m_wndList.GetItemText( nItem, 1 );

			if ( _tcsicmp( pSearch->m_sSearch, L"None" ) != 0 &&
				 _tcsncmp( pSearch->m_sSearch, L"btih:", 5 ) != 0 )
				pSearch->m_sSearch = L"urn:" + m_wndList.GetItemText( nItem, 1 );
			else
				pSearch->m_sSearch.Empty();
		}

		if ( ! pSearch->m_sSearch.IsEmpty() )
			CQuerySearch::OpenWindow( pSearch );
	}
}

void CSearchMonitorWnd::OnUpdateSearchMonitorPause(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bPaused );
}

void CSearchMonitorWnd::OnSearchMonitorPause()
{
	m_bPaused = ! m_bPaused;
}

void CSearchMonitorWnd::OnSearchMonitorClear()
{
	m_wndList.DeleteAllItems();
}

void CSearchMonitorWnd::OnDblClkList(NMHDR* /*pNotifyStruct*/, LRESULT *pResult)
{
	OnSearchMonitorSearch();
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPanelWnd event handlers

void CSearchMonitorWnd::OnQuerySearch(const CQuerySearch* pSearch)
{
	if ( m_bPaused || m_hWnd == NULL )
		return;

	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	if ( m_bPaused )
		return;

	CLiveItem* pItem = new CLiveItem( COL_LAST, NULL );

	CString strSearch = pSearch->m_sSearch;

//	LoadString( strSchema, IDS_NEIGHBOUR_COMPRESSION_NONE );	// ToDo: Generic "None" translation ?
//	LoadString( strURN, IDS_NEIGHBOUR_COMPRESSION_NONE );

	CString strSize;
	if ( pSearch->m_nMinSize > 100 )
	{
		strSize = Settings.SmartVolume( pSearch->m_nMinSize );
		if ( pSearch->m_nMaxSize != SIZE_UNKNOWN && ( pSearch->m_nMaxSize - pSearch->m_nMinSize ) < 1024 * 1025 )
			strSize = L"~ " + Settings.SmartVolume( pSearch->m_nMaxSize );	// Specific size
		else if ( pSearch->m_nMaxSize != SIZE_UNKNOWN && pSearch->m_nMaxSize > 512 )
			strSize = strSize + L" - " + Settings.SmartVolume( pSearch->m_nMaxSize );
		else
			strSize = L"> " + strSize;
	}
	else if ( pSearch->m_nMaxSize != SIZE_UNKNOWN && pSearch->m_nMaxSize > 100 )
	{
		strSize = L"< " + Settings.SmartVolume( pSearch->m_nMaxSize );
	}

	CString strNode;
	if ( pSearch->m_pEndpoint.sin_addr.s_addr )
		strNode.Format( L"%hs:%u",
			inet_ntoa( pSearch->m_pEndpoint.sin_addr ),
			ntohs( pSearch->m_pEndpoint.sin_port ) );

	if ( pSearch->m_nProtocol > PROTOCOL_NULL )
	{
		if ( pSearch->m_nProtocol == PROTOCOL_G2 )
			strNode += L"  G2";
		else if ( pSearch->m_nProtocol == PROTOCOL_G1 )
			strNode += L"  G1";
		else if ( pSearch->m_nProtocol == PROTOCOL_ED2K )
			strNode += L"  ED2K";
		else if ( pSearch->m_nProtocol == PROTOCOL_DC )
			strNode += L"  DC++";
		else
			strNode += L"  ??";		// Others?
	}

	CString strURN;
	if ( pSearch->HasHash() )
		strURN = pSearch->GetShortURN();
	else
		strURN = L"None";

	if ( pSearch->m_bWhatsNew )
		strSearch = L"What's New?";

	if ( pSearch->m_pXML )
	{
		strSearch += L'«';
		strSearch += pSearch->m_pXML->GetRecursiveWords();
		strSearch += L'»';
	}

	CString strSchema;
	if ( pSearch->m_pSchema )
		strSchema = pSearch->m_pSchema->m_sTitle;
	else
		strSchema = L"-";

	// ToDo: Add proper Size and Network columns to HitMonitor instead?
	if ( strSize.GetLength() > 1 )
	{
		if ( strSchema.GetLength() > 3 )
			strSchema += L"  " + strSize;
		else
			strSchema = strSize;
	}

	if ( ! strURN.IsEmpty() )
	{
		if ( strSearch.GetLength() > 1 )
			strSearch += L"  " + strURN;
		else
			strSearch = strURN;
	}

	pItem->Set( COL_SEARCH, strSearch );
//	pItem->Set( COL_URN, strURN );
//	pItem->Set( COL_SIZE, strSize );
	pItem->Set( COL_SCHEMA, strSchema );
//	pItem->Set( COL_NETWORK, strNetwork );
	pItem->Set( COL_ENDPOINT, strNode );

	m_pQueue.AddTail( pItem );
}

void CSearchMonitorWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent != 2 ) return;

	BOOL bScroll = m_wndList.GetTopIndex() + m_wndList.GetCountPerPage() >= m_wndList.GetItemCount();

	for ( ;; )
	{
		CLiveItem* pItem;

		{
			CSingleLock pLock( &m_pSection );
			if ( ! pLock.Lock( 250 ) )
				break;

			if ( m_pQueue.GetCount() == 0 )
				break;

			pItem = m_pQueue.RemoveHead();
		}

		if ( (DWORD)m_wndList.GetItemCount() >= Settings.Search.MonitorQueue && Settings.Search.MonitorQueue > 0 )
			m_wndList.DeleteItem( 0 );

		/*int nItem =*/ pItem->Add( &m_wndList, -1, COL_LAST );

		delete pItem;
	}

	if ( bScroll )
		m_wndList.EnsureVisible( m_wndList.GetItemCount() - 1, FALSE );
}

void CSearchMonitorWnd::OnUpdateSecurityBan(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetNextItem( -1, LVNI_SELECTED ) >= 0 );
}

void CSearchMonitorWnd::OnSecurityBan()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem < 0 ) return;

	SOCKADDR_IN pHost = { 0 };
	pHost.sin_family = AF_INET;
	CString strNode = m_wndList.GetItemText( nItem, 3 );
	int nPos = strNode.Find( L':' );
	pHost.sin_addr.s_addr = inet_addr( CT2CA( (LPCTSTR)strNode.Left( nPos ) ) );
	pHost.sin_port = htons( (WORD)_tstoi( strNode.Mid( nPos + 1 ) ) );
	Security.Ban( &pHost.sin_addr, banSession );
}

void CSearchMonitorWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetNextItem( -1, LVNI_SELECTED ) >= 0 );
}

void CSearchMonitorWnd::OnBrowseLaunch()
{
	const int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		SOCKADDR_IN pHost = { 0 };
		pHost.sin_family = AF_INET;
		CString strNode = m_wndList.GetItemText( nItem, 3 );
		int nPos = strNode.Find( L':' );
		pHost.sin_addr.s_addr = inet_addr( CT2CA( (LPCTSTR)strNode.Left( nPos ) ) );
		pHost.sin_port = htons( (WORD)_tstoi( strNode.Mid( nPos + 1 ) ) );
		new CBrowseHostWnd( PROTOCOL_ANY, &pHost );
	}
}

void CSearchMonitorWnd::OnCustomDrawList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	*pResult = CDRF_DODEFAULT;
}
