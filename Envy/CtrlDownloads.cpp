//
// CtrlDownloads.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2015
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
#include "CtrlDownloads.h"
#include "WndDownloads.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "Transfers.h"
#include "FragmentBar.h"
#include "DisplayData.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Flags.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define HEADER_HEIGHT				20
// Skin ITEM_HEIGHT					17	// Settings.Skin.RowSize

// Set Column Order
enum {
	COL_TITLE,
	COL_SIZE,
	COL_TRANSFER,
	COL_SPEED,
	COL_PROGRESS,
	COL_PERCENT,
	COL_STATUS,
	COL_CLIENT,
	COL_COUNTRY,
	COL_LAST	// Count
};


IMPLEMENT_DYNAMIC(CDownloadsCtrl, CWnd)

BEGIN_MESSAGE_MAP(CDownloadsCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	ON_NOTIFY(HDN_ITEMCLICKA, AFX_IDW_PANE_FIRST, OnSortPanelItems)
	ON_NOTIFY(HDN_ITEMCLICKW, AFX_IDW_PANE_FIRST, OnSortPanelItems)
	ON_NOTIFY(HDN_ITEMCHANGEDW, AFX_IDW_PANE_FIRST, OnChangeHeader)
	ON_NOTIFY(HDN_ITEMCHANGEDA, AFX_IDW_PANE_FIRST, OnChangeHeader)
	ON_NOTIFY(HDN_ENDDRAG, AFX_IDW_PANE_FIRST, OnChangeHeader)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl construction

CDownloadsCtrl::CDownloadsCtrl()
	: m_nGroupCookie	( 0 )
	, m_nFocus			( 0 )
	, m_nHover			( -1 )
//	, m_pDragDrop		( NULL )	// Obsolete
	, m_bDragStart		( FALSE )
	, m_bDragActive		( FALSE )
	, m_bCreateDragImage( FALSE )
	, m_pDeselect1		( NULL )
	, m_pDeselect2		( NULL )
	, m_pbSortAscending	( NULL )
	, m_bShowSearching	( TRUE )
//	, m_tSwitchTimer	( 0 )		// Using static
{
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl operations

BOOL CDownloadsCtrl::Create(CWnd* pParentWnd, UINT nID)
{
	CRect rc( 0, 0, 0, 0 );
	return CWnd::CreateEx( 0, NULL, L"CDownloadsCtrl",
		WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP, rc, pParentWnd, nID );
}

BOOL CDownloadsCtrl::Update()
{
	OnSize( 1982, 0, 0 );
	return TRUE;
}

BOOL CDownloadsCtrl::Update(int nGroupCookie)
{
	m_nGroupCookie = nGroupCookie;
	OnSize( 1982, 0, 0 );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl system message handlers

int CDownloadsCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rect( 0, 0, 0, 0 );

	m_wndHeader.Create( WS_CHILD|HDS_DRAGDROP|HDS_HOTTRACK|HDS_FULLDRAG|(Settings.Downloads.SortColumns ? HDS_BUTTONS : 0 ), rect, this, AFX_IDW_PANE_FIRST );

	m_wndTip.Create( this, &Settings.Interface.TipDownloads );

	GetDesktopWindow()->GetWindowRect( &rect );

	InsertColumn( COL_TITLE, L"Downloaded File", LVCFMT_LEFT, rect.Width() > 1600 ? 400 : 300 );
	InsertColumn( COL_SIZE, L"Size", LVCFMT_CENTER, 64 );
	InsertColumn( COL_TRANSFER, L"Transfer", LVCFMT_CENTER, 64 );
	InsertColumn( COL_SPEED, L"Speed", LVCFMT_CENTER, 74 );
	InsertColumn( COL_PROGRESS, L"Progress", LVCFMT_CENTER, 100 );
	InsertColumn( COL_PERCENT, L"Percent", LVCFMT_CENTER, 58 );
	InsertColumn( COL_STATUS, L"Status", LVCFMT_CENTER, 76 );
	InsertColumn( COL_CLIENT, L"Client", LVCFMT_CENTER, 108 );
	InsertColumn( COL_COUNTRY, L"Country", LVCFMT_LEFT, 54 );

	LoadColumnState();

//	CoolInterface.LoadIconsTo( m_pProtocols, protocolIDs );

	m_nGroupCookie		= 0;
	m_nFocus			= 0;
	m_nHover			= -1;
//	m_pDragDrop			= NULL;
	m_bCreateDragImage	= FALSE;
	m_bDragStart		= FALSE;
	m_bDragActive		= FALSE;
	m_pDeselect1		= NULL;
	m_pDeselect2		= NULL;

	m_pbSortAscending	= new BOOL[ COL_LAST ];		// Was COLUMNS_TO_SORT
	for ( int i = 0; i < COL_LAST; i++ )
		m_pbSortAscending[ i ] = TRUE;

	return 0;
}

void CDownloadsCtrl::OnDestroy()
{
	SaveColumnState();

	delete[] m_pbSortAscending;
	CWnd::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl column helpers

void CDownloadsCtrl::InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth)
{
	HDITEM pColumn = {};

	pColumn.mask	= HDI_FORMAT | HDI_LPARAM | HDI_TEXT | HDI_WIDTH;
	pColumn.cxy		= nWidth;
	pColumn.pszText	= (LPTSTR)pszCaption;
	pColumn.fmt		= nFormat;
	pColumn.lParam	= nColumn;

	m_wndHeader.InsertItem( m_wndHeader.GetItemCount(), &pColumn );
}

void CDownloadsCtrl::SaveColumnState()
{
	HDITEM pItem = { HDI_WIDTH|HDI_ORDER };

	CString strOrdering, strWidths, strItem;

	for ( int nColumns = 0; m_wndHeader.GetItem( nColumns, &pItem ); nColumns++ )
	{
		m_wndHeader.GetItem( nColumns, &pItem );

		strItem.Format( L"%.2x", pItem.iOrder );
		strOrdering += strItem;

		strItem.Format( L"%.4x", pItem.cxy );
		strWidths += strItem;
	}

	theApp.WriteProfileString( L"ListStates", L"CDownloadCtrl.Ordering", strOrdering );
	theApp.WriteProfileString( L"ListStates", L"CDownloadCtrl.Widths", strWidths );
}

BOOL CDownloadsCtrl::LoadColumnState()
{
	CString strOrdering	= theApp.GetProfileString( L"ListStates", L"CDownloadCtrl.Ordering", L"" );
	CString strWidths	= theApp.GetProfileString( L"ListStates", L"CDownloadCtrl.Widths", L"" );

	HDITEM pItem = { HDI_WIDTH|HDI_ORDER };

	if ( _tcsncmp( strWidths, L"0000", 4 ) == 0 &&
		 _tcsncmp( strOrdering, L"00", 2 ) == 0 )
	{
		strWidths = strWidths.Mid( 4 );
		strOrdering = strOrdering.Mid( 2 );
	}

	for ( int nColumns = 0; m_wndHeader.GetItem( nColumns, &pItem ); nColumns++ )
	{
		if ( strWidths.GetLength() < 4 || strOrdering.GetLength() < 2 )
			return FALSE;

		if ( _stscanf( strWidths.Left( 4 ), L"%x", &pItem.cxy ) != 1 ||
			 _stscanf( strOrdering.Left( 2 ), L"%x", &pItem.iOrder ) != 1 )
			return FALSE;

		strWidths = strWidths.Mid( 4 );
		strOrdering = strOrdering.Mid( 2 );

		m_wndHeader.SetItem( nColumns, &pItem );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl item helpers

bool CDownloadsCtrl::IsFiltered(const CDownload* pDownload)
{
	ASSUME_LOCK( Transfers.m_pSection );

	DWORD nFilterMask = Settings.Downloads.FilterMask;

	if ( Settings.General.GUIMode == GUI_BASIC )
		return false;
	if ( pDownload->IsSeeding() )
		return ( ( nFilterMask & DLF_SEED ) == 0 );
	if ( pDownload->IsCompleted() )
		return false;
	if ( pDownload->IsPaused() )
		return ( ( nFilterMask & DLF_PAUSED ) == 0 );
	if ( pDownload->IsDownloading() || pDownload->IsMoving() )
		return ( ( nFilterMask & DLF_ACTIVE ) == 0 );
	if ( pDownload->GetEffectiveSourceCount() > 0 )
		return ( ( nFilterMask & DLF_QUEUED ) == 0 );
	//if ( pDownload->m_nSize == SIZE_UNKNOWN )
		return ( ( nFilterMask & DLF_SOURCES ) == 0 );
}

BOOL CDownloadsCtrl::IsExpandable(const CDownload* pDownload)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( pDownload->IsSeeding() && ! Settings.General.DebugBTSources )
		return FALSE;

	if ( Settings.Downloads.ShowSources )
		return ( pDownload->GetSourceCount() > 0 );

	for ( POSITION posSource = pDownload->GetIterator(); posSource; )
	{
		const CDownloadSource* pSource = pDownload->GetNext( posSource );
		if ( pSource->IsConnected() )
			return TRUE;
	}

	return FALSE;
}

void CDownloadsCtrl::SelectTo(int nIndex)
{
//	ASSUME_LOCK( Transfers.m_pSection );	// Lock as needed

	const BOOL bRight	= GetAsyncKeyState( VK_RBUTTON ) & 0x8000;
	const BOOL bControl	= GetAsyncKeyState( VK_CONTROL ) & 0x8000;
	const BOOL bShift	= GetAsyncKeyState( VK_SHIFT ) & 0x8000;

	int nMin, nMax;
	GetScrollRange( SB_VERT, &nMin, &nMax );
	nIndex = max( 0, min( nIndex, nMax - 1 ) );

	if ( ! bShift && ! bControl && ! bRight )
	{
		if ( m_pDeselect1 == NULL && m_pDeselect2 == NULL )
			DeselectAll();
		if ( nIndex < m_pDownloadsData.GetSize() )
			m_pDownloadsData[ nIndex ].m_bSelected = TRUE;
	}

	CDownload* pDownload;
	CDownloadSource* pSource;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	if ( bShift )
	{
		if ( m_nFocus < nIndex )
		{
			for ( m_nFocus ++; m_nFocus <= nIndex; m_nFocus ++ )
			{
				GetAt( m_nFocus, &pDownload, &pSource );
				if ( pDownload != NULL ) pDownload->m_bSelected = TRUE;
				if ( pSource != NULL ) pSource->m_bSelected = TRUE;
			}
		}
		else if ( m_nFocus > nIndex )
		{
			for ( m_nFocus --; m_nFocus >= nIndex; m_nFocus -- )
			{
				GetAt( m_nFocus, &pDownload, &pSource );
				if ( pDownload != NULL ) pDownload->m_bSelected = TRUE;
				if ( pSource != NULL ) pSource->m_bSelected = TRUE;
			}
		}

		m_nFocus = nIndex;
	}
	else
	{
		m_nFocus = nIndex;
		GetAt( m_nFocus, &pDownload, &pSource );

		if ( bControl &&
			! ( GetAsyncKeyState( VK_HOME ) & 0x8000 || GetAsyncKeyState( VK_END ) & 0x8000 ) )
		{
			if ( pDownload != NULL ) pDownload->m_bSelected = ! pDownload->m_bSelected;
			if ( pSource != NULL ) pSource->m_bSelected = ! pSource->m_bSelected;
		}
		else
		{
			if ( pDownload != NULL ) pDownload->m_bSelected = TRUE;
			if ( pSource != NULL ) pSource->m_bSelected = TRUE;
		}
	}

	CRect rcClient;
	GetClientRect( &rcClient );

	int nScroll = GetScrollPos( SB_VERT );
	int nHeight = ( rcClient.bottom - HEADER_HEIGHT ) / Settings.Skin.RowSize - 1;
	if ( nHeight < 0 ) nHeight = 0;

	BOOL bUpdate = TRUE;
	if ( m_nFocus < nScroll )
		SetScrollPos( SB_VERT, m_nFocus );
	else if ( m_nFocus > nScroll + nHeight )
		SetScrollPos( SB_VERT, max( 0, m_nFocus - nHeight ) );
	else
		bUpdate = FALSE;

	UpdateDownloadsData( TRUE );

	pLock.Unlock();

	bUpdate ? Update() : Invalidate();
}

void CDownloadsCtrl::SelectAll(CDownload* /*pDownload*/, CDownloadSource* /*pSource*/)
{
//	ASSUME_LOCK( Transfers.m_pSection );

	// Update display data immediately
	for ( int nIndex = m_pDownloadsData.GetUpperBound(); nIndex >= 0; nIndex-- )
	{
		m_pDownloadsData[ nIndex ].m_bSelected = TRUE;
		if ( m_pDownloadsData[ nIndex ].m_pSourcesData.GetUpperBound() <= 0 ) continue;
		for ( int nSource = m_pDownloadsData[ nIndex ].m_pSourcesData.GetUpperBound(); nSource >= 0; nSource-- )
			m_pDownloadsData[ nIndex ].m_pSourcesData[ nSource ].m_bSelected = TRUE;
	}

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	BOOL bSelected = FALSE;

	for ( POSITION pos = Downloads.GetIterator(); pos != NULL; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		// If a download is selected, select all downloads
		if ( pDownload->m_bSelected )
		{
			for ( POSITION pos2 = Downloads.GetIterator(); pos2 != NULL; )
			{
				if ( CDownload* pSelectedDownload = Downloads.GetNext( pos2 ) )
					pSelectedDownload->m_bSelected = TRUE;
			}

			bSelected = TRUE;
		}

		// If a source is selected, select all sources for that download
		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			if ( ! pDownload->GetNext( posSource )->m_bSelected )
				continue;

			for ( POSITION posSource2 = pDownload->GetIterator(); posSource2; )
			{
				pDownload->GetNext( posSource2 )->m_bSelected = TRUE;
			}

			bSelected = TRUE;
			break;
		}
	}

	// If nothing is selected, select all downloads
	if ( ! bSelected )
	{
		for ( POSITION pos = Downloads.GetIterator(); pos != NULL; )
		{
			CDownload* pDownload = Downloads.GetNext( pos );

			if ( pDownload != NULL )
				pDownload->m_bSelected = TRUE;
		}
	}

	UpdateDownloadsData( TRUE );

	pLock.Unlock();
	Invalidate();
}

void CDownloadsCtrl::DeselectAll(CDownload* pExcept1, CDownloadSource* pExcept2)
{
	// Update display data immediately
	for ( int nIndex = m_pDownloadsData.GetUpperBound(); nIndex >= 0; nIndex-- )
	{
		m_pDownloadsData[ nIndex ].m_bSelected = FALSE;
		if ( m_pDownloadsData[ nIndex ].m_pSourcesData.GetUpperBound() <= 0 ) continue;
		for ( int nSource = m_pDownloadsData[ nIndex ].m_pSourcesData.GetUpperBound(); nSource >= 0; nSource-- )
			m_pDownloadsData[ nIndex ].m_pSourcesData[ nSource ].m_bSelected = FALSE;
	}

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos != NULL; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload != pExcept1 )
			pDownload->m_bSelected = FALSE;

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource != pExcept2 )
				pSource->m_bSelected = FALSE;
		}
	}

	pLock.Unlock();
	Invalidate();
}

int CDownloadsCtrl::GetSelectedCount()
{
	ASSUME_LOCK( Transfers.m_pSection );

	int nCount = 0;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected )
			nCount++;

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );
			if ( pSource->m_bSelected )
				nCount++;
		}
	}

	return nCount;
}

BOOL CDownloadsCtrl::GetSelectedList(CList< CDownload* >& pList, BOOL bClearing /*FALSE*/)
{
	ASSUME_LOCK( Transfers.m_pSection );

	CDownload* pDownload;
	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		pDownload = Downloads.GetNext( pos );
		if ( ! pDownload->m_bSelected ) continue;

		if ( bClearing )
			pDownload->m_bClearing = TRUE;		// Mark for pending removal
		pList.AddTail( pDownload );
	}

	return ! pList.IsEmpty();
}

BOOL CDownloadsCtrl::HitTest(const CPoint& point, CDownload** ppDownload, CDownloadSource** ppSource, int* pnIndex, RECT* prcItem)
{
	ASSUME_LOCK( Transfers.m_pSection );

	CRect rcClient, rcItem;

	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;

	rcItem.CopyRect( &rcClient );
	rcItem.left -= GetScrollPos( SB_HORZ );
	rcItem.bottom = rcItem.top + Settings.Skin.RowSize;

	int nScroll = GetScrollPos( SB_VERT );
	int nIndex = 0;

	if ( ppDownload != NULL ) *ppDownload = NULL;
	if ( ppSource != NULL ) *ppSource = NULL;

	for ( POSITION posDownload = Downloads.GetIterator(); posDownload && rcItem.top < rcClient.bottom; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie )
			continue;
		if ( IsFiltered( pDownload ) )
			continue;

		if ( nScroll > 0 )
		{
			nScroll--;
		}
		else
		{
			if ( rcItem.PtInRect( point ) )
			{
				if ( ppDownload ) *ppDownload = pDownload;
				if ( pnIndex ) *pnIndex = nIndex;
				if ( prcItem ) *prcItem = rcItem;
				return TRUE;
			}
			rcItem.OffsetRect( 0, Settings.Skin.RowSize );
		}

		nIndex++;
		if ( ! pDownload->m_bExpanded || ( pDownload->IsSeeding() && ! Settings.General.DebugBTSources ) )
			continue;

		if ( Settings.Downloads.ShowSources )
		{
			int nSources = pDownload->GetSourceCount();

			if ( nScroll >= nSources )
			{
				nScroll -= nSources;
				nIndex += nSources;
				continue;
			}
		}

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
			{
				if ( nScroll > 0 )
				{
					nScroll--;
				}
				else
				{
					if ( rcItem.PtInRect( point ) )
					{
						if ( ppSource != NULL ) *ppSource = pSource;
						if ( pnIndex != NULL ) *pnIndex = nIndex;
						if ( prcItem != NULL ) *prcItem = rcItem;
						return TRUE;
					}
					rcItem.OffsetRect( 0, Settings.Skin.RowSize );
				}

				nIndex ++;
			}
		}
	}

	return FALSE;
}

BOOL CDownloadsCtrl::HitTest(int nIndex, CDownload** ppDownload, CDownloadSource** ppSource)
{
	ASSUME_LOCK( Transfers.m_pSection );

	int nScroll = GetScrollPos( SB_VERT );
	int nCount = nIndex - 1;

	if ( ppDownload != NULL ) *ppDownload = NULL;
	if ( ppSource != NULL ) *ppSource = NULL;

	for ( POSITION posDownload = Downloads.GetIterator(); posDownload && nIndex >= 0; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie )
			continue;
		if ( IsFiltered( pDownload ) )
			continue;

		if ( nScroll > 0 )
		{
			nScroll--;
		}
		else
		{
			if ( ! nCount )
			{
				if ( ppDownload ) *ppDownload = pDownload;
				return TRUE;
			}

			nCount--;
		}

		if ( ! pDownload->m_bExpanded || ( pDownload->IsSeeding() && ! Settings.General.DebugBTSources ) )
			continue;

		if ( Settings.Downloads.ShowSources )
		{
			int nSources = pDownload->GetSourceCount();

			if ( nScroll >= nSources )
			{
				nScroll -= nSources;
				continue;
			}
		}

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
			{
				if ( nScroll > 0 )
				{
					nScroll--;
				}
				else
				{
					if ( ! nCount )
					{
						if ( ppSource != NULL ) *ppSource = pSource;
						return TRUE;
					}
					nCount--;
				}
			}
		}
	}

	return FALSE;
}

BOOL CDownloadsCtrl::GetAt(int nSelect, CDownload** ppDownload, CDownloadSource** ppSource)
{
	ASSUME_LOCK( Transfers.m_pSection );

	/*int nScroll =*/ GetScrollPos( SB_VERT );
	int nIndex = 0;

	if ( ppDownload != NULL ) *ppDownload = NULL;
	if ( ppSource != NULL ) *ppSource = NULL;

	for ( POSITION posDownload = Downloads.GetIterator(); posDownload; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie )
			continue;
		if ( IsFiltered( pDownload ) )
			continue;

		if ( nIndex++ == nSelect )
		{
			if ( ppDownload != NULL )
				*ppDownload = pDownload;
			return TRUE;
		}

		if ( ! pDownload->m_bExpanded || ( pDownload->IsSeeding() && ! Settings.General.DebugBTSources ) )
			continue;

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
			{
				if ( nIndex++ == nSelect )
				{
					if ( ppSource != NULL ) *ppSource = pSource;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL CDownloadsCtrl::GetRect(CDownload* pSelect, RECT* prcItem)
{
	ASSUME_LOCK( Transfers.m_pSection );

	CRect rcClient, rcItem;

	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;

	rcItem.CopyRect( &rcClient );
	rcItem.left -= GetScrollPos( SB_HORZ );
	rcItem.bottom = rcItem.top + Settings.Skin.RowSize;

	int nScroll = GetScrollPos( SB_VERT );
	rcItem.OffsetRect( 0, Settings.Skin.RowSize * -nScroll );

	for ( POSITION posDownload = Downloads.GetIterator(); posDownload; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie )
			continue;
		if ( IsFiltered( pDownload ) )
			continue;

		if ( pDownload == pSelect )
		{
			*prcItem = rcItem;
			return TRUE;
		}

		rcItem.OffsetRect( 0, Settings.Skin.RowSize );

		if ( ! pDownload->m_bExpanded )
			continue;

		if ( Settings.Downloads.ShowSources )
		{
			int nSources = pDownload->GetSourceCount();
			rcItem.OffsetRect( 0, Settings.Skin.RowSize * nSources );
			continue;
		}

		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->IsConnected() )
				rcItem.OffsetRect( 0, Settings.Skin.RowSize );
		}
	}

	return FALSE;
}

void CDownloadsCtrl::MoveSelected(int nDelta)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CDownload*> pList;
	if ( ! GetSelectedList( pList ) )
		return;

	if ( ! SafeLock( pLock ) ) return;	// Release/Relock

	POSITION pos = nDelta > 0 ? pList.GetTailPosition() : pList.GetHeadPosition();

	while ( pos )
	{
		CDownload* pDownload = nDelta > 0 ? pList.GetPrev( pos ) : pList.GetNext( pos );
		Downloads.Move( pDownload, nDelta );
	}

	m_nFocus += nDelta;
	if ( m_nFocus < 0 ) m_nFocus = 0;

	pLock.Unlock();
	Update();
}

void CDownloadsCtrl::MoveToTop()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected )
			Downloads.Move( pDownload, -2 );
		if ( ! pDownload->IsTrying() )
			pDownload->Resume();
	}
	// Workaround to Correct Order
	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( ! pDownload->m_bSelected ) break;
		Downloads.Move( pDownload, -2 );
	}

	pLock.Unlock();		// Lock as needed
	SelectTo( 0 );
}

void CDownloadsCtrl::MoveToEnd()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Downloads.GetReverseIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetPrevious( pos );
		if ( pDownload->m_bSelected )
			Downloads.Move( pDownload, 2 );
	}
	// Workaround to Correct Order
	for ( POSITION pos = Downloads.GetReverseIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetPrevious( pos );
		if ( ! pDownload->m_bSelected ) break;
		Downloads.Move( pDownload, 2 );
	}
}

BOOL CDownloadsCtrl::DropObjects(CList< CDownload* >* pSel, const CPoint& ptScreen)
{
	CPoint ptLocal( ptScreen );
	CRect rcClient;

	ScreenToClient( &ptLocal );
	GetClientRect( &rcClient );

//	m_pDragDrop = NULL;
	m_bDragActive = FALSE;

	if ( pSel == NULL || ! rcClient.PtInRect( ptLocal ) ) return FALSE;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CDownload* pHit = NULL;
	HitTest( ptLocal, &pHit, NULL, NULL, NULL );

	for ( POSITION pos = pSel->GetHeadPosition(); pos; )
	{
		CDownload* pDownload = (CDownload*)pSel->GetNext( pos );

		if ( Downloads.Check( pDownload ) && pDownload != pHit )
			Downloads.Reorder( pDownload, pHit );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl presentation message handlers

void CDownloadsCtrl::OnSize(UINT nType, int cx, int cy)
{
	int nWidth = 0, nHeight = 0;
	CRect rcClient;

	if ( nType != 1982 ) CWnd::OnSize( nType, cx, cy );

	GetClientRect( &rcClient );

	HDITEM pColumn = {};
	pColumn.mask = HDI_WIDTH;

	for ( int nColumn = 0; m_wndHeader.GetItem( nColumn, &pColumn ); nColumn ++ )
		nWidth += pColumn.cxy;

	SCROLLINFO pScroll = {};
	pScroll.cbSize	= sizeof( pScroll );
	pScroll.fMask	= SIF_RANGE|SIF_PAGE;
	pScroll.nMin	= 0;
	pScroll.nMax	= nWidth;
	pScroll.nPage	= rcClient.right;
	SetScrollInfo( SB_HORZ, &pScroll, TRUE );

	int nScroll = GetScrollPos( SB_HORZ );
	m_wndHeader.SetWindowPos( NULL, -nScroll, 0, rcClient.right + nScroll, HEADER_HEIGHT, SWP_SHOWWINDOW );

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	for ( POSITION posDownload = Downloads.GetIterator(); posDownload; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie || IsFiltered( pDownload ) )
		{
			pDownload->m_bSelected = FALSE;

			for ( POSITION posSource = pDownload->GetIterator(); posSource; )
			{
				CDownloadSource* pSource = pDownload->GetNext( posSource );

				pSource->m_bSelected = FALSE;
			}
			continue;
		}

		nHeight ++;

		if ( ! pDownload->m_bExpanded || ( pDownload->IsSeeding() && ! Settings.General.DebugBTSources ) )
			continue;

		if ( Settings.Downloads.ShowSources )
		{
			nHeight += pDownload->GetSourceCount();
		}
		else
		{
			for ( POSITION posSource = pDownload->GetIterator(); posSource; )
			{
				CDownloadSource* pSource = pDownload->GetNext( posSource );

				if ( /*Settings.Downloads.ShowSources ||*/ pSource->IsConnected() )
					nHeight ++;
			}
		}
	}

	pLock.Unlock();

	ZeroMemory( &pScroll, sizeof( pScroll ) );
	pScroll.cbSize	= sizeof( pScroll );
	pScroll.fMask	= SIF_RANGE|SIF_PAGE;
	pScroll.nMin	= 0;
	pScroll.nMax	= nHeight;
	pScroll.nPage	= ( rcClient.bottom - HEADER_HEIGHT ) / Settings.Skin.RowSize + 1;
	SetScrollInfo( SB_VERT, &pScroll, TRUE );

	m_nFocus = min( m_nFocus, max( 0, nHeight - 1 ) );

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl populate duplicate Downloads/Sources data (isolated to minimize locks)

void CDownloadsCtrl::UpdateDownloadsData(BOOL bForce /*FALSE*/)
{
	static DWORD tUpdate = 0;
	if ( ! bForce && tUpdate + 500 > GetTickCount() )
		return;

	CRect rcClient;
	GetClientRect( &rcClient );
	const int nMin = GetScrollPos( SB_VERT );
	const int nMax = nMin + ( rcClient.Height() / (int)Settings.Skin.RowSize ) + 1;
	int nCount = 0;

	CSingleLock pTransfersLock( &Transfers.m_pSection );
	if ( ! pTransfersLock.Lock( bForce ? 250 : 50 ) )
		return;

	INT_PTR nIndex = 0;
	for ( POSITION posDownload = Downloads.GetIterator(); posDownload; )
	{
		const CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie ) || IsFiltered( pDownload ) )
			continue;

		if ( nCount++ < nMin - 1 && ! pDownload->m_bExpanded )
		{
		//	m_pDownloadsData.SetAtGrow( nIndex, CDownloadDisplayData() );
			nIndex++;
			continue;
		}

		CDownloadDisplayData pDownloadData( pDownload );

		UINT nRange = 0;
		BOOL bvSuccess;		// Pass/Fail
		for ( QWORD nvOffset = 0, nvLength = 0; pDownload->GetNextVerifyRange( nvOffset, nvLength, bvSuccess ); nRange++ )
		{
#if defined(_MSC_VER) && (_MSC_VER >= 1800)		// VS2013+
			pDownloadData.m_pVerifyRanges.SetAtGrow( nRange, { nvOffset, nvLength, bvSuccess } );
#else	// VS2008 (VS2012?)
			CDownloadDisplayData::VERIFYRANGE pVerifyRange = { nvOffset, nvLength, bvSuccess };
			pDownloadData.m_pVerifyRanges.SetAtGrow( nRange, pVerifyRange );
#endif
			nvOffset += nvLength;
		}

		if ( ! pDownloadData.m_bCompleted && ! pDownload->IsPaused() && ( ! pDownloadData.m_bSeeding || Settings.General.DebugBTSources ) )		// Not just for pDownloadData.m_bExpanded
		{
			UINT nSource = 0;
			for ( POSITION posSource = pDownload->GetIterator(); posSource; )
			{
				CDownloadSource* pSource = pDownload->GetNext( posSource );

				if ( pDownloadData.m_bSeeding && ! pDownloadData.m_bExpanded )
				{
					pDownloadData.m_bExpandable = TRUE;
					break;
				}

				if ( ! Settings.Downloads.ShowSources && ! pSource->IsConnected() )
					continue;

				if ( ! pDownloadData.m_bExpanded && ! pSource->GetTransfer()->m_nLength && pSource->m_oAvailable.empty() && pSource->m_oPastFragments.empty() )
					continue;

				if ( pSource->m_nColor < 0 )
					pSource->GetColor();

				pDownloadData.m_pSourcesData.SetAtGrow( nSource, CSourceDisplayData( pSource ) );
				nSource++;

			//	if ( pDownloadData.m_bExpanded && nCount++ > nMax )
			//		break;
			}

			if ( nSource )
				pDownloadData.m_bExpandable = TRUE;

			pDownloadData.m_nSourceCount = nSource;
		}

		m_pDownloadsData.SetAtGrow( nIndex, pDownloadData );

		nIndex++;
		if ( nCount > nMax )
			break;
	}

	pTransfersLock.Unlock();

	while ( m_pDownloadsData.GetCount() > nIndex )
		m_pDownloadsData.RemoveAt( nIndex );

	tUpdate = GetTickCount();
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl painting

void CDownloadsCtrl::OnPaint()
{
	CPaintDC dc( this );
	CRect rcClient, rcItem;
	const DWORD tNow = GetTickCount();
	const BOOL bFocus = ( GetFocus() == this );

	static DWORD tSwitchTimer = 0;
	if ( tNow > tSwitchTimer + 8000 )
	{
		tSwitchTimer = tNow;
		m_bShowSearching = ! m_bShowSearching;
	}

	UpdateDownloadsData();

	CFont* pfOld = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );
	if ( Settings.General.LanguageRTL )
		dc.SetTextAlign( TA_RTLREADING );

	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;

	rcItem.CopyRect( &rcClient );
	rcItem.left  -= GetScrollPos( SB_HORZ );
	rcItem.bottom = rcItem.top + (LONG)Settings.Skin.RowSize;

	int nScroll = GetScrollPos( SB_VERT );
	int nIndex = 0;

	for ( INT_PTR nDownload = 0; nDownload < m_pDownloadsData.GetCount(); nDownload++ )
	{
		if ( rcItem.top > rcClient.bottom )
			break;

		const CDownloadDisplayData* pDownloadData = &m_pDownloadsData[ nDownload ];

		if ( nScroll > 0 )
		{
			nScroll--;
		}
		else
		{
			PaintDownload( dc, rcItem, pDownloadData, nIndex == m_nFocus && bFocus );
			rcItem.OffsetRect( 0, (int)Settings.Skin.RowSize );
			if ( rcItem.top > rcClient.bottom )
				break;
		}

		nIndex++;

		if ( ! pDownloadData->m_bExpanded || ! pDownloadData->m_bExpandable || ( pDownloadData->m_bSeeding && ! Settings.General.DebugBTSources ) )
			continue;

		const int nSources = (int)pDownloadData->m_nSourceCount;
		if ( ! nSources )
			continue;

		if ( Settings.Downloads.ShowSources && nScroll >= nSources )
		{
			nScroll -= nSources;
			nIndex  += nSources;
			continue;
		}

		for ( int nSource = 0; nSource < nSources; nSource++ )
		{
			if ( nScroll > 0 )
			{
				--nScroll;
			}
			else
			{
				PaintSource( dc, rcItem, &pDownloadData->m_pSourcesData[ nSource ], nIndex == m_nFocus && bFocus );
				rcItem.OffsetRect( 0, (int)Settings.Skin.RowSize );
				if ( rcItem.top > rcClient.bottom )
					break;
			}

			nIndex++;
		}
	}

	if ( rcItem.top < rcClient.bottom )
	{
		rcClient.top = rcItem.top;
		dc.FillSolidRect( &rcClient, Colors.m_crWindow );
	}

	dc.SelectObject( pfOld );
}

void CDownloadsCtrl::PaintDownload(CDC& dc, const CRect& rcRow, const CDownloadDisplayData* pDownloadData, BOOL bFocus /*FALSE*/)
{
	const BOOL bSelected = pDownloadData->m_bSelected;
	//const BOOL bActive = bSelected && ( GetFocus() == this );
	BOOL bLeftMargin = TRUE;

	const COLORREF crNatural	= m_bCreateDragImage ? DRAG_COLOR_KEY : Colors.m_crWindow;
	const COLORREF crBack		= bSelected ? Colors.m_crHighlight : crNatural;
	const COLORREF crBorder		= bSelected ? Colors.m_crFragmentBorderSelected : Colors.m_crFragmentBorder;
	COLORREF crLeftMargin		= crNatural;
	COLORREF crText				= Colors.m_crText;

	// Skinnable Selection Highlight
	BOOL bSelectmark = FALSE;
	if ( bSelected && Images.DrawButtonState( &dc, &rcRow, ( GetFocus() != this ? IMAGE_SELECTED : IMAGE_SELECTEDGREY ) ) )
	{
		// Was CoolInterface.DrawWatermark( &dc, &rcDraw, &Images.m_bmSelected );
		bSelectmark = TRUE;
	}
	else
	{
		// Update Full Row Highlight
		dc.FillSolidRect( rcRow, crBack );
		dc.SetBkColor( crBack );
	}

	dc.SetBkMode( bSelectmark ? TRANSPARENT : OPAQUE );

	if ( pDownloadData->m_bExpandable )
		dc.SelectObject( &CoolInterface.m_fntBold );

	if ( m_bDragActive && m_nHover == int( rcRow.top / Settings.Skin.RowSize ) )
	{
		CRect rcDrop( rcRow.left, rcRow.top, rcRow.right, rcRow.top + 2 );
		dc.Draw3dRect( &rcDrop, 0, 0 );		// ToDo: Skinable Line?
		dc.ExcludeClipRect( &rcDrop );
	}

	// Modify Text color if needed
	if ( pDownloadData->m_bClearing )
	{
		// Briefly marked for removal/deletion.  ToDo: m_crTransferClearing?
		crText = Colors.m_crNetworkNull;
	}
	else if ( pDownloadData->m_bCompleted )
	{
		if ( pDownloadData->m_bFailedVerify )
			crText = bSelected ? Colors.m_crTransferVerifyFailSelected : Colors.m_crTransferVerifyFail;
		else if ( pDownloadData->m_bSeeding && pDownloadData->m_nVolumeComplete < pDownloadData->m_nSize )
			crText = bSelected ? Colors.m_crTransferVerifyPassSelected : Colors.m_crTransferVerifyPass;
		else
			crText = bSelected ? Colors.m_crTransferCompletedSelected : Colors.m_crTransferCompleted;
	}
	else if ( bSelected )
	{
		crText = Colors.m_crHiText;
	}

	dc.SetTextColor( crText );

	int nTextLeft = rcRow.right, nTextRight = rcRow.left;

	HDITEM pColumn = {};
	pColumn.mask = HDI_FORMAT | HDI_LPARAM;

	for ( int nColumn = 0; m_wndHeader.GetItem( nColumn, &pColumn ); nColumn++ )
	{
		CString strText;
		CRect rcCell;

		m_wndHeader.GetItemRect( nColumn, &rcCell );
		rcCell.left		+= rcRow.left;
		rcCell.right	+= rcRow.left;
		rcCell.top		= rcRow.top;
		rcCell.bottom	= rcRow.bottom;

		if ( rcCell.Width() < 6 )
		{
			if ( pColumn.lParam == COL_TITLE && rcRow.left == rcCell.left )
				dc.FillSolidRect( rcCell.left, rcCell.top, rcCell.Width(), Settings.Skin.RowSize, crNatural );
			continue;
		}

		switch ( pColumn.lParam )
		{
		case COL_TITLE:
			bLeftMargin = rcRow.left == rcCell.left;
			crLeftMargin = ( bLeftMargin ? crNatural : bSelectmark ? -1 : crBack );
			if ( bLeftMargin || ! bSelectmark && Settings.Skin.RowSize > 16 )
				dc.FillSolidRect( rcCell.left, rcCell.top + 16, 32, Settings.Skin.RowSize - 16, crLeftMargin );
			if ( pDownloadData->m_bExpandable )
			{
				BOOL bHover = m_nHover == int( rcCell.top / Settings.Skin.RowSize );
				if ( bHover )
				{
					POINT ptHover;
					GetCursorPos( &ptHover );
					ScreenToClient( &ptHover );
					bHover = ptHover.x < rcCell.left + 16 && ptHover.x > rcCell.left;
				}
				if ( pDownloadData->m_bExpanded )
					CoolInterface.Draw( &dc, bHover ? IDI_CLOSETICK_HOVER : IDI_CLOSETICK, 16, rcCell.left, rcCell.top, crLeftMargin );
				else
					CoolInterface.Draw( &dc, bHover ? IDI_OPENTICK_HOVER : IDI_OPENTICK, 16, rcCell.left, rcCell.top, crLeftMargin );
			}
			else if ( bLeftMargin || ! bSelectmark )
			{
				dc.FillSolidRect( rcCell.left, rcCell.top, 16, Settings.Skin.RowSize, crLeftMargin );
			}
			rcCell.left += 16;

			// Draw file icon
			if ( pDownloadData->m_bMultiFileTorrent )	// Special case
				CoolInterface.Draw( &dc, IDI_MULTIFILE, 16, rcCell.left, rcCell.top, crLeftMargin, bSelected );
			else
				ShellIcons.Draw( &dc, ShellIcons.Get( pDownloadData->m_sName, 16 ), 16, rcCell.left, rcCell.top, crLeftMargin, bSelected );

			// Add rating overlay
			switch ( pDownloadData->m_nRating )
			{
			case 0:		// No reviews
				break;
			case 1:		// Ratings suggest fake file
				CoolInterface.Draw( &dc, IDI_RATING_FAKE, 16, rcCell.left, rcCell.top, CLR_NONE, bSelected );
				break;
			case 2:
			case 3:
			case 4:		// Ratings suggest average file
				CoolInterface.Draw( &dc, IDI_RATING_AVERAGE, 16, rcCell.left, rcCell.top, CLR_NONE, bSelected );
				break;
			default:	// Ratings suggest good file
				CoolInterface.Draw( &dc, IDI_RATING_GOOD, 16, rcCell.left, rcCell.top, CLR_NONE, bSelected );
				break;
			}

			rcCell.left += 16;
			if ( bLeftMargin || ! bSelectmark )
				dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crLeftMargin );
			rcCell.left++;

			strText = pDownloadData->m_sDisplayName;
			break;

		case COL_SIZE:
			if ( pDownloadData->m_nSize < SIZE_UNKNOWN )
				strText = Settings.SmartVolume( pDownloadData->m_nSize );
			else
				LoadString( strText, IDS_STATUS_UNKNOWN );
			break;

		case COL_PROGRESS:
			if ( rcCell.Width() >= 70 )
			{
				strText.Empty();
				rcCell.DeflateRect( 1, 2 );

				dc.Draw3dRect( &rcCell, crBorder, crBorder );
				rcCell.DeflateRect( 1, 1 );
				CFragmentBar::DrawDownload( &dc, &rcCell, pDownloadData, crNatural );		// Note new abstracted method
			}
			else if ( pDownloadData->m_nSize < SIZE_UNKNOWN && pDownloadData->m_nSize > 0 )
			{
				if ( rcCell.Width() > 50 )
					strText.Format( L"%.2f%%", pDownloadData->m_fProgress );
				else
					strText.Format( L"%i%%", int( pDownloadData->m_fProgress ) );
			}
			break;

		case COL_SPEED:
			if ( pDownloadData->m_bTrying && pDownloadData->m_nAverageSpeed )
				strText = Settings.SmartSpeed( pDownloadData->m_nAverageSpeed );
			break;

		case COL_STATUS:
			if ( m_bShowSearching && pDownloadData->m_bSearching )
				LoadString( strText, IDS_STATUS_SEARCHING );
			else
				strText = pDownloadData->m_sDownloadStatus;
			break;

		case COL_CLIENT:
			strText = pDownloadData->m_sDownloadSources;
			break;

		case COL_TRANSFER:
			strText = Settings.SmartVolume( pDownloadData->m_nVolumeComplete );	// bSeeding ? m_nTorrentUploaded : GetVolumeComplete()
			break;

		case COL_PERCENT:
			if ( pDownloadData->m_nSize < SIZE_UNKNOWN && pDownloadData->m_nSize > 0 )
			{
				if ( rcCell.Width() > 50 )
					strText.Format( L"%.2f%%",  ( pDownloadData->m_bSeeding ? pDownloadData->m_fRatio : pDownloadData->m_fProgress ) );
				else
					strText.Format( L"%i%%", int( pDownloadData->m_bSeeding ? pDownloadData->m_fRatio : pDownloadData->m_fProgress ) );
			}
			else
				LoadString( strText, IDS_STATUS_UNKNOWN );
			break;
		}

		if ( strText.IsEmpty() ) continue;

		if ( dc.GetTextExtent( strText ).cx > rcCell.Width() - 8 )
		{
			while ( dc.GetTextExtent( strText + L'\x2026' ).cx > ( rcCell.Width() - 8 ) && ! strText.IsEmpty() )
			{
				strText.Truncate( strText.GetLength() - 1 );
			}

			if ( ! strText.IsEmpty() ) strText += L'\x2026';
		}

		nTextLeft	= min( nTextLeft, (int)rcCell.left );
		nTextRight	= max( nTextRight, (int)rcCell.right );

		int nPosition = 0;

		switch ( pColumn.fmt & LVCFMT_JUSTIFYMASK )
		{
		default:
			nPosition = rcCell.left + 4;
			break;
		case LVCFMT_CENTER:
			nPosition = ( ( rcCell.left + rcCell.right ) / 2 ) - ( dc.GetTextExtent( strText ).cx / 2 );
			break;
		case LVCFMT_RIGHT:
			nPosition = rcCell.right - 4 - dc.GetTextExtent( strText ).cx;
			break;
		}

		dc.SetBkColor( bSelectmark ? CLR_NONE : crBack );
		dc.ExtTextOut( nPosition, rcCell.top + 2,
			ETO_CLIPPED|( bSelectmark ? 0 : ETO_OPAQUE ),
			&rcCell, strText, NULL );
		// ToDo: Fix text for m_bCreateDragImage
	}

	// Non-column whitespace area (redundant)
	//if ( nTextRight < rcRow.right && ! bSelectmark )
	//	dc.FillSolidRect( nTextRight, rcRow.top, rcRow.right, rcRow.bottom, crBack );

	if ( bFocus )
	{
		CRect rcFocus( nTextLeft, rcRow.top, max( (int)rcRow.right, nTextRight ), rcRow.bottom );
		dc.Draw3dRect( &rcFocus, Colors.m_crHiBorder, Colors.m_crHiBorder );

		if ( Settings.Skin.RoundedSelect )
		{
			dc.SetPixel( rcFocus.left, rcFocus.top, crNatural );
			dc.SetPixel( rcFocus.left, rcFocus.bottom - 1, crNatural );
			dc.SetPixel( rcRow.right - 1, rcRow.top, crNatural );
			dc.SetPixel( rcRow.right - 1, rcRow.bottom - 1, crNatural );
		}

		if ( Colors.m_crHiBorderIn )
		{
			rcFocus.DeflateRect( 1, 1 );
			dc.Draw3dRect( &rcFocus, Colors.m_crHiBorderIn, Colors.m_crHiBorderIn );
		}
	}

	dc.SelectObject( &CoolInterface.m_fntNormal );
}

void CDownloadsCtrl::PaintSource(CDC& dc, const CRect& rcRow, const CSourceDisplayData* pSourceData, BOOL bFocus /*FALSE*/)
{
	const BOOL bSelected = pSourceData->m_bSelected;
	//const BOOL bActive = bSelected && ( GetFocus() == this );
	BOOL bLeftMargin = TRUE;

	COLORREF crNatural		= m_bCreateDragImage ? DRAG_COLOR_KEY : Colors.m_crWindow;
	COLORREF crBack			= bSelected ? Colors.m_crHighlight : crNatural;
	COLORREF crLeftMargin	= crBack;
	COLORREF crBorder		= bSelected ? Colors.m_crFragmentBorderSelected : Colors.m_crFragmentBorder;

	// Skinnable Selection Highlight
	BOOL bSelectmark = FALSE;
	if ( bSelected && Images.DrawButtonState( &dc, &rcRow, ( GetFocus() != this ? IMAGE_SELECTED : IMAGE_SELECTEDGREY ) ) )
	{
		// Was CoolInterface.DrawWatermark( &dc, &CRect( rcRow ), &Images.m_bmSelected );	// Non-const
		bSelectmark = TRUE;
	}
	else
	{
		// Update Full Row Highlight
		dc.FillSolidRect( rcRow, crBack );
		dc.SetBkColor( crBack );
	}

	dc.SetBkMode( bSelectmark ? TRANSPARENT : OPAQUE );
	dc.SetTextColor( bSelected ? Colors.m_crHiText : Colors.m_crTransferSource );

	int nTextLeft = rcRow.right, nTextRight = rcRow.left;

	HDITEM pColumn = {};
	pColumn.mask = HDI_FORMAT | HDI_LPARAM;

	for ( int nColumn = 0; m_wndHeader.GetItem( nColumn, &pColumn ); nColumn++ )
	{
		CString strText;
		CRect rcCell;

		m_wndHeader.GetItemRect( nColumn, &rcCell );
		rcCell.left		+= rcRow.left;
		rcCell.right	+= rcRow.left;
		rcCell.top		= rcRow.top;
		rcCell.bottom	= rcRow.bottom;

		if ( rcCell.Width() < 6 )
			continue;

		switch ( pColumn.lParam )
		{
		case COL_TITLE:
			bLeftMargin = rcRow.left == rcCell.left;
			crLeftMargin = ( bLeftMargin ? crNatural : bSelectmark ? -1 : crBack );

			if ( bLeftMargin || ! bSelectmark )
				dc.FillSolidRect( rcCell.left, rcCell.top, 24, rcCell.Height(), crLeftMargin );
			rcCell.left += 24;
			if ( ( bLeftMargin || ! bSelectmark ) && Settings.Skin.RowSize > 16 )
				dc.FillSolidRect( rcCell.left, rcCell.top + 16, 16, rcCell.Height() - 16, crLeftMargin );
			ImageList_DrawEx( m_pProtocols, pSourceData->m_nProtocol, dc.GetSafeHdc(),
					rcCell.left, rcCell.top, 16, 16, crLeftMargin, CLR_DEFAULT, bSelected ? ILD_SELECTED : ILD_NORMAL );
			rcCell.left += 16;
			if ( bLeftMargin || ! bSelectmark )
				dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crLeftMargin );
			rcCell.left++;

			// Is this a firewalled eDonkey client
			if ( pSourceData->m_nProtocol == PROTOCOL_ED2K && pSourceData->m_bPushOnly )
			{
				strText.Format( L"%lu@%s:%u",
					pSourceData->m_nAddress,				// pSource->m_pAddress.S_un.S_addr
					(LPCTSTR)pSourceData->m_sAddress,		// inet_ntoa( pSource->m_pServerAddress )
					pSourceData->m_nServerPort );
			}
			else if ( pSourceData->m_nProtocol == PROTOCOL_DC )	// Or DC++
			{
				strText.Format( L"%s:%u",
					(LPCTSTR)pSourceData->m_sAddress,		// inet_ntoa( pSource->m_pServerAddress )
					pSourceData->m_nServerPort );
			}
			else if ( pSourceData->m_bIdle )	// Or an active transfer
			{
				strText.Format( L"%s:%u",
					(LPCTSTR)pSourceData->m_sAddressGet,
					ntohs( pSourceData->m_nPortGet ) );
			}
			else	// Or just queued
			{
				strText.Format( L"%s:%u",
					(LPCTSTR)pSourceData->m_sAddress,		// inet_ntoa( pSource->m_pAddress )
					pSourceData->m_nPort );
			}

			// Add the Nickname if there is one and they are being shown
			if ( Settings.Search.ShowNames && ! pSourceData->m_sNick.IsEmpty() )
				strText = pSourceData->m_sNick + L" (" + strText + L")";

			// Indicate if this is a firewalled client
			if ( pSourceData->m_bPushOnly )
				strText += L" (push)";

			break;

		case COL_SIZE:
			if ( ! pSourceData->m_bIdle )
				if ( pSourceData->m_nState > dtsHeaders && pSourceData->m_oAvailable.empty() )
					strText = Settings.SmartVolume( pSourceData->m_nSize );
				else
					strText = Settings.SmartVolume( pSourceData->m_oAvailable.length_sum() );
			break;

		case COL_PROGRESS:
			if ( rcCell.Width() > 75 )
			{
				rcCell.DeflateRect( 1, 2 );
				dc.Draw3dRect( &rcCell, crBorder, crBorder );
				rcCell.DeflateRect( 1, 1 );
				CFragmentBar::DrawSource( &dc, &rcCell, pSourceData, Colors.m_crTransferRanges );			// Was pSource->Draw( &dc, &rcCell, Colors.m_crTransferRanges )
			}
			else if ( ! pSourceData->m_bIdle )
			{
				if ( pSourceData->m_nState > dtsHeaders && pSourceData->m_oAvailable.empty() )
					rcCell.Width() > 50 ? strText = L"100.00%" : strText = L"100%";
				else if ( rcCell.Width() > 50 )
					strText.Format( L"%.2f%%", float( pSourceData->m_oAvailable.length_sum() * 10000 / pSourceData->m_nSize ) / 100 );
				else
					strText.Format( L"%i%%", int( pSourceData->m_oAvailable.length_sum() * 100 / pSourceData->m_nSize ) );
			}
			break;

		case COL_SPEED:
			if ( ! pSourceData->m_bIdle )
			{
				DWORD nSpeed = pSourceData->m_nSpeed;
				if ( nSpeed )
					strText = Settings.SmartSpeed( nSpeed );
			}
			break;

		case COL_STATUS:
			if ( ! pSourceData->m_bIdle )
			{
				strText = pSourceData->m_sState;
			}
			else if ( pSourceData->m_tAttempt && pSourceData->m_bTrying )
			{
				DWORD nTime = GetTickCount();

				if ( pSourceData->m_tAttempt >= nTime )
				{
					nTime = ( pSourceData->m_tAttempt - nTime ) / 1000;
					strText.Format( L"%.2u:%.2u", nTime / 60, nTime % 60 );
				}
			}
			break;

		case COL_CLIENT:
			strText = pSourceData->m_sServer;
			break;

		case COL_TRANSFER:
			if ( ! pSourceData->m_bIdle )
				strText = Settings.SmartVolume( pSourceData->m_nDownloaded );
			break;

		case COL_PERCENT:
			if ( ! pSourceData->m_bIdle && pSourceData->m_nDownloaded > 0 &&
				pSourceData->m_nSize < SIZE_UNKNOWN && pSourceData->m_nSize > 0 )
			{
				if ( rcCell.Width() > 50 )
					strText.Format( L"%.2f%%", float( pSourceData->m_nDownloaded * 10000 / pSourceData->m_nSize ) / 100.0f );
				else
					strText.Format( L"%i%%", int( pSourceData->m_nDownloaded * 100 / pSourceData->m_nSize ) );
			}
			break;

		case COL_COUNTRY:
			int nFlagImage = Flags.GetFlagIndex( pSourceData->m_sCountry );

			if ( ! bSelectmark )
				dc.FillSolidRect( rcCell.left, rcCell.top, Flags.Width + 4, rcCell.Height(), crBack );
			rcCell.left += 3;
			if ( nFlagImage >= 0 )
				Flags.Draw( nFlagImage, dc.GetSafeHdc(), rcCell.left, rcCell.top,
					bSelectmark ? CLR_NONE : crBack, CLR_DEFAULT, pSourceData->m_bSelected ? ILD_SELECTED : ILD_NORMAL );
			rcCell.left += Flags.Width;	// 18

			strText = pSourceData->m_sCountry;
			break;
		}

		if ( strText.IsEmpty() )
			continue;

		if ( dc.GetTextExtent( strText ).cx > rcCell.Width() - 8 )
		{
			while ( dc.GetTextExtent( strText + L'\x2026' ).cx > ( rcCell.Width() - 8 ) && ! strText.IsEmpty() )
			{
				strText.Truncate( strText.GetLength() - 1 );
			}

			if ( ! strText.IsEmpty() )
				strText += L'\x2026';
		}

		nTextLeft  = min( nTextLeft,  (int)rcCell.left );
		nTextRight = max( nTextRight, (int)rcCell.right );

		int nPosition = 0;

		switch ( pColumn.fmt & LVCFMT_JUSTIFYMASK )
		{
		case LVCFMT_CENTER:
			nPosition = ( ( rcCell.left + rcCell.right ) / 2 ) - ( dc.GetTextExtent( strText ).cx / 2 );
			break;
		case LVCFMT_RIGHT:
			nPosition = ( rcCell.right - 4 - dc.GetTextExtent( strText ).cx );
			break;
		default:
			nPosition = ( rcCell.left + 4 );
			break;
		}

		dc.SetBkColor( bSelectmark ? CLR_NONE : crBack );
		dc.ExtTextOut( nPosition, rcCell.top + 2,
			ETO_CLIPPED|( bSelectmark ? 0 : ETO_OPAQUE ),
			&rcCell, strText, NULL );
	}

	// Non-column whitespace area (redundant)
	//if ( nTextRight < rcRow.right && ! bSelectmark )
	//	dc.FillSolidRect( nTextRight, rcRow.top, rcRow.right, rcRow.bottom, crBack );

	if ( bFocus )
	{
		CRect rcFocus( nTextLeft, rcRow.top, max( (int)rcRow.right, nTextRight ), rcRow.bottom );
		dc.Draw3dRect( &rcFocus, Colors.m_crHiBorder, Colors.m_crHiBorder );

		if ( Settings.Skin.RoundedSelect )
		{
			dc.SetPixel( rcFocus.left, rcFocus.top, crNatural );
			dc.SetPixel( rcFocus.left, rcFocus.bottom - 1, crNatural );
			dc.SetPixel( rcRow.right - 1, rcRow.top, crNatural );
			dc.SetPixel( rcRow.right - 1, rcRow.bottom - 1, crNatural );
		}

		if ( Colors.m_crHiBorderIn )
		{
			rcFocus.DeflateRect( 1, 1 );
			dc.Draw3dRect( &rcFocus, Colors.m_crHiBorderIn, Colors.m_crHiBorderIn );
		}
	}
}

void CDownloadsCtrl::OnSkinChange()
{
	m_wndHeader.SetFont( &CoolInterface.m_fntNormal );

	CoolInterface.LoadIconsTo( m_pProtocols, protocolIDs );

	// Obsolete for reference:
	//for ( int nImage = 1; nImage < PROTOCOL_LAST; nImage++ )
	//{
	//	if ( HICON hIcon = CoolInterface.ExtractIcon( (UINT)protocolCmdMap[ nImage ].commandID, FALSE ) )
	//	{
	//		m_pProtocols.Replace( nImage, hIcon );
	//		DestroyIcon( hIcon );
	//	}
	//}

	// Update Dropshadow
	m_wndTip.DestroyWindow();
	m_wndTip.Create( this, &Settings.Interface.TipDownloads );
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl interaction message handlers

void CDownloadsCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	SCROLLINFO pInfo = {};
	pInfo.cbSize = sizeof( pInfo );
	pInfo.fMask  = SIF_ALL & ~SIF_TRACKPOS;

	GetScrollInfo( SB_VERT, &pInfo );
	int nDelta = pInfo.nPos;

	switch ( nSBCode )
	{
	case SB_BOTTOM:
		pInfo.nPos = pInfo.nMax - pInfo.nPage;
		break;
	case SB_LINEDOWN:
		pInfo.nPos ++;
		break;
	case SB_LINEUP:
		pInfo.nPos --;
		break;
	case SB_PAGEDOWN:
		pInfo.nPos += pInfo.nPage;
		break;
	case SB_PAGEUP:
		pInfo.nPos -= pInfo.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pInfo.nPos = nPos;
		break;
	case SB_TOP:
		pInfo.nPos = 0;
		break;
	}

	pInfo.nPos = max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	if ( pInfo.nPos == nDelta ) return;

	SetScrollInfo( SB_VERT, &pInfo, TRUE );

	m_nHover = -1;
	UpdateDownloadsData( TRUE );
	Invalidate();
}

void CDownloadsCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	SCROLLINFO pInfo = {};
	pInfo.cbSize = sizeof( pInfo );
	pInfo.fMask  = SIF_ALL & ~SIF_TRACKPOS;

	GetScrollInfo( SB_HORZ, &pInfo );
	int nDelta = pInfo.nPos;

	switch ( nSBCode )
	{
	case SB_BOTTOM:
		pInfo.nPos = pInfo.nMax - pInfo.nPage;
		break;
	case SB_LINEDOWN:
		pInfo.nPos ++;
		break;
	case SB_LINEUP:
		pInfo.nPos --;
		break;
	case SB_PAGEDOWN:
		pInfo.nPos += pInfo.nPage;
		break;
	case SB_PAGEUP:
		pInfo.nPos -= pInfo.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pInfo.nPos = nPos;
		break;
	case SB_TOP:
		pInfo.nPos = 0;
		break;
	}

	pInfo.nPos = max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	if ( pInfo.nPos == nDelta ) return;

	SetScrollInfo( SB_HORZ, &pInfo, TRUE );

	CRect rcClient;
	GetClientRect( &rcClient );

	m_wndHeader.SetWindowPos( NULL, -pInfo.nPos, 0,
		rcClient.right + pInfo.nPos, HEADER_HEIGHT, SWP_NOZORDER );

	RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
}

BOOL CDownloadsCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	OnVScroll( SB_THUMBPOSITION, GetScrollPos( SB_VERT ) - zDelta / WHEEL_DELTA * theApp.m_nMouseWheel );
	return TRUE;
}

void CDownloadsCtrl::OnChangeHeader(NMHDR* /*pNotifyStruct*/, LRESULT* /*pResult*/)
{
	Update();
}

void CDownloadsCtrl::BubbleSortDownloads(int nColumn)	// BinaryInsertionSortDownloads(int nColumn)
{
	m_pbSortAscending[ nColumn ] = ! m_pbSortAscending[ nColumn ];

	if ( Downloads.GetCount() < 2 ) return;

	CSingleLock pLock( &Transfers.m_pSection );	// GetIterator() GetNext()
	if ( ! SafeLock( pLock ) ) return;

	POSITION pos = Downloads.GetIterator(), pos_y = pos;
	Downloads.GetNext( pos );

	while ( pos != NULL )
	{
		POSITION pos_x = pos;
		CDownload *x = Downloads.GetNext( pos );

		BOOL bOK = FALSE, bRlBk = TRUE;
		CDownload *y = NULL;
		while ( bRlBk && ( pos_y != NULL ) )
		{
			y = Downloads.GetPrevious( pos_y );
			if ( m_pbSortAscending[ nColumn ] == FALSE )
			{
				switch ( nColumn )
				{
				case COL_TITLE:
					if ( x->GetDisplayName().CompareNoCase( y->GetDisplayName() ) < 0 )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_SIZE:
					if ( x->m_nSize < y->m_nSize )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_PROGRESS:
					if ( x->GetProgress() < y->GetProgress() )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_SPEED:
					if ( x->GetMeasuredSpeed() < y->GetMeasuredSpeed() )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_STATUS:
					if ( x->GetDownloadStatus().CompareNoCase( y->GetDownloadStatus() ) < 0 )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_CLIENT:
					if ( x->GetClientStatus() < y->GetClientStatus() )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_TRANSFER:
					if ( x->GetVolumeComplete() < y->GetVolumeComplete() )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_PERCENT:
					if ( ((double)(x->GetVolumeComplete() ) / (double)(x->m_nSize)) < ((double)(y->GetVolumeComplete() ) / (double)(y->m_nSize)) )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				} // end switch
			}
			else // Sort Ascending
			{
				switch ( nColumn )
				{
				case COL_TITLE:
					if ( x->GetDisplayName().CompareNoCase( y->GetDisplayName() ) > 0 )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_SIZE:
					if ( x->m_nSize > y->m_nSize )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_PROGRESS:
					if ( x->GetProgress() > y->GetProgress() )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_SPEED:
					if ( x->GetMeasuredSpeed() > y->GetMeasuredSpeed() )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_STATUS:
					if ( x->GetDownloadStatus().CompareNoCase( y->GetDownloadStatus() ) > 0 )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_CLIENT:
					if ( x->GetClientStatus() > y->GetClientStatus() )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_TRANSFER:
					if ( x->GetVolumeComplete() > y->GetVolumeComplete() )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				case COL_PERCENT:
					if ( ( (double)(x->GetVolumeComplete() ) / (double)(x->m_nSize) ) > ( (double)(y->GetVolumeComplete() ) / (double)(y->m_nSize) ) )
						bOK = TRUE;
					else
						bRlBk = FALSE;
					break;
				} // end switch
			} // end if else
		} // end while bRlBk

		if ( bOK )
		{
			Downloads.Reorder( x, y );
			if ( ! bRlBk )
				Downloads.Move( x, 1 );
			if ( pos == NULL )
				break;
			pos_y = pos;
			Downloads.GetPrevious( pos_y );
		}
		else
		{
			pos_y = pos_x;
		}
	} // end while pos
}

void CDownloadsCtrl::OnSortPanelItems(NMHDR* pNotifyStruct, LRESULT* /*pResult*/)
{
	//CSingleLock pLock( &Transfers.m_pSection, TRUE );		// In BubbleSort
	NMLISTVIEW *pLV = (NMLISTVIEW*)pNotifyStruct;
	BubbleSortDownloads( pLV->iItem );
	Invalidate();
}

void CDownloadsCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_wndTip.Hide();

	const BOOL bControl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0;
//	const BOOL bShift   = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;

	CSingleLock pLock( &Transfers.m_pSection, FALSE );	// Only where needed

	switch ( nChar )
	{
	case VK_HOME:
		if ( bControl )
			MoveToTop();
		else
			SelectTo( 0 );
		return;
	case VK_END:
		{
			int nMin, nMax;
			GetScrollRange( SB_VERT, &nMin, &nMax );
			if ( nMax < 2 ) return;
			if ( bControl )
				MoveToEnd();
			SelectTo( nMax - 1 );
		}
		return;
	case VK_UP:
		if ( ! bControl )
			SelectTo( m_nFocus - 1 );
		//else
		//	MoveSelected( -1 );
		return;
	case VK_DOWN:
		if ( ! bControl )
			SelectTo( m_nFocus + 1 );
		//else
		//	MoveSelected( 1 );
		return;
	case VK_PRIOR:
		{
			CRect rcView;
			GetWindowRect( &rcView );
			int nRows = ( rcView.Height() - Settings.Skin.ToolbarHeight ) / Settings.Skin.RowSize;

			if ( bControl )
			{
				for ( ; nRows; --nRows )
					MoveSelected( -1 );
			}
			else
				SelectTo( m_nFocus - nRows );
		}
		return;
	case VK_NEXT:
		{
			CRect rcView;
			GetWindowRect( &rcView );
			int nRows = ( rcView.Height() - Settings.Skin.ToolbarHeight ) / Settings.Skin.RowSize;

			if ( bControl )
			{
				for ( ; nRows; --nRows )
					MoveSelected( 1 );
			}
			else
				SelectTo( m_nFocus + nRows );
		}
		return;
	case VK_LEFT:
	case VK_SUBTRACT:
		if ( SafeLock( pLock ) )
		{
			CDownload* pDownload;
			CDownloadSource* pSource;
			if ( GetAt( m_nFocus, &pDownload, &pSource ) )
			{
				if ( pSource != NULL )
				{
					pDownload = pSource->m_pDownload;

					CDownloadsWnd* pWindow = (CDownloadsWnd*)GetOwner();
					ASSERT_KINDOF( CDownloadsWnd, pWindow );
					pWindow->Select( pDownload );
				}

				if ( pDownload && pDownload->m_bExpanded )
				{
					pDownload->m_bExpanded = FALSE;
					UpdateDownloadsData( TRUE );
					Update();
					return;
				}
			}
		}
		if ( nChar == VK_LEFT && ! bControl )
			SelectTo( m_nFocus - 1 );
		return;
	case VK_RIGHT:
	case VK_ADD:
		if ( SafeLock( pLock ) )
		{
			CDownload* pDownload;
			if ( GetAt( m_nFocus, &pDownload, NULL ) && pDownload != NULL && pDownload->m_bExpanded == FALSE && ! pDownload->IsCompleted() )
			{
				pDownload->m_bExpanded = TRUE;
				UpdateDownloadsData( TRUE );
				Update();
				return;
			}
		}
		if ( nChar == VK_RIGHT && ! bControl )
			SelectTo( m_nFocus + 1 );
		return;
	case 'A':
		if ( bControl )
			SelectAll();
		return;
	case 'C':
		if ( bControl )
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_URI );
		return;
	case 'V':
		if ( bControl )
			GetOwner()->PostMessage( WM_COMMAND, ID_TOOLS_DOWNLOAD );
		return;
	case VK_INSERT:
		if ( bControl )
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_URI );
		else //if ( bShift )
			GetOwner()->PostMessage( WM_COMMAND, ID_TOOLS_DOWNLOAD );
		return;
	case 'E':
		if ( bControl )
		{
			GetOwner()->PostMessage( WM_TIMER, 5 );
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_ENQUEUE );	// Add the current file to playlist
		}
		return;
	case 'R':
		if ( bControl )
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_VIEW_REVIEWS );
		return;
	case VK_DELETE:
		if ( bControl || ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0 )
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_FILE_DELETE );
		else
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_CLEAR );
		return;
	case VK_RETURN:
		if ( SafeLock( pLock ) )
			OnEnterKey();
		return;
	}

	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CDownloadsCtrl::OnEnterKey()
{
	ASSUME_LOCK( Transfers.m_pSection );	// For GetAt

	CDownload* pDownload;
	CDownloadSource* pSource;

	GetAt( m_nFocus, &pDownload, &pSource );								// Get data for the current focus
	if ( pDownload != NULL )												// Selected object is a download...
	{
		GetOwner()->PostMessage( WM_TIMER, 5 );
		GetOwner()->PostMessage( WM_COMMAND, pDownload->IsCompleted() ?
			ID_DOWNLOADS_LAUNCH_COMPLETE : ID_DOWNLOADS_LAUNCH_COPY );		// Launch current file/partial
	}
	else if ( pSource != NULL )												// Selected object is a download source...
	{
		GetOwner()->PostMessage( WM_TIMER, 5 );
		GetOwner()->PostMessage( WM_COMMAND, ID_TRANSFERS_CONNECT );		// Connect to the source
	}
}

void CDownloadsCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	m_wndTip.Hide();

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 500 ) ) return;

	int nIndex;
	CRect rcItem;
	CDownload* pDownload;
	CDownloadSource* pSource;

	if ( HitTest( point, &pDownload, &pSource, &nIndex, &rcItem ) )
	{
		const int nTitleStarts = GetExpandableColumnX();
		if ( point.x > nTitleStarts && point.x <= nTitleStarts + rcItem.left + 16 )		// Tick
		{
			if ( pDownload != NULL && IsExpandable( pDownload ) )
			{
				pDownload->m_bExpanded = ! pDownload->m_bExpanded;

				if ( ! pDownload->m_bExpanded )
				{
					for ( POSITION posSource = pDownload->GetIterator(); posSource; )
					{
						// CDownloadSource* pDownloadSource
						pDownload->GetNext( posSource )->m_bSelected = FALSE;
					}
				}

				pLock.Unlock();
				Update();
				return;
			}
		}
		else
		{
			if ( ( nFlags & ( MK_SHIFT | MK_CONTROL | MK_RBUTTON ) ) == 0 )
			{
				if ( pDownload != NULL && pDownload->m_bSelected )
					m_pDeselect1 = pDownload;
				else if ( pSource != NULL && pSource->m_bSelected )
					m_pDeselect2 = pSource;
			}
		//	else if ( nFlags & MK_RBUTTON )
		//	{
		//		DeselectAll();
		//	}
		//	else if ( pDownload != NULL && ! pDownload->m_bSelected )
		//	{
		//		m_pDownloadsData[ nIndex ].bSelected = TRUE;
		//		Update();
		//	}

			SelectTo( nIndex );
		}

		if ( ( nFlags & MK_LBUTTON ) && GetSelectedCount() > 0 )
		{
			m_bDragStart = TRUE;
			m_ptDrag = point;
		}
	}
	else if ( ( nFlags & ( MK_SHIFT | MK_CONTROL ) ) == 0 )
	{
		DeselectAll();
	}
}

void CDownloadsCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_wndTip.Hide();
	OnLButtonDown( nFlags, point );
	CWnd::OnRButtonDown( nFlags, point );
}

void CDownloadsCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	SetFocus();

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 500 ) ) return;

	CRect rcItem;
	CDownload* pDownload;
	CDownloadSource* pSource;

	if ( HitTest( point, &pDownload, &pSource, NULL, &rcItem ) )
	{
		if ( pDownload != NULL )
		{
			int nTitleStarts = GetExpandableColumnX();
			if ( point.x <= nTitleStarts + rcItem.left + 16 && point.x > nTitleStarts )
			{
				if ( IsExpandable( pDownload ) )
				{
					pDownload->m_bExpanded = ! pDownload->m_bExpanded;

					if ( ! pDownload->m_bExpanded )
					{
						for ( POSITION posSource = pDownload->GetIterator(); posSource; )
						{
							CDownloadSource* pDownloadSource = pDownload->GetNext( posSource );
							pDownloadSource->m_bSelected = FALSE;
						}
					}

					pLock.Unlock();
					Update();
					return;
				}
			}

			GetOwner()->PostMessage( WM_TIMER, 5 );
			GetOwner()->PostMessage( WM_COMMAND, pDownload->IsCompleted() ?
				ID_DOWNLOADS_LAUNCH_COMPLETE : ID_DOWNLOADS_LAUNCH_COPY );
		}
		else if ( pSource != NULL )
		{
			GetOwner()->PostMessage( WM_TIMER, 5 );
			GetOwner()->PostMessage( WM_COMMAND, ID_TRANSFERS_CONNECT );
		}
	}

	pLock.Unlock();

	CWnd::OnLButtonDblClk( nFlags, point );
}

void CDownloadsCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDragStart = m_bDragActive = FALSE;

	if ( m_pDeselect1 != NULL )
	{
		DeselectAll( m_pDeselect1 );
		m_pDeselect1 = NULL;
	}
	else if ( m_pDeselect2 != NULL )
	{
		DeselectAll( NULL, m_pDeselect2 );
		m_pDeselect2 = NULL;
	}

	CWnd::OnLButtonUp( nFlags, point );
}

void CDownloadsCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bDragStart = m_bDragActive = FALSE;

//	if ( m_pDeselect1 != NULL )
//	{
//		DeselectAll( m_pDeselect1 );
//		m_pDeselect1 = NULL;
//	}
//	else if ( m_pDeselect2 != NULL )
//	{
//		DeselectAll( NULL, m_pDeselect2 );
//		m_pDeselect2 = NULL;
//	}

	CWnd::OnRButtonUp( nFlags, point );
}

void CDownloadsCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove( nFlags, point );

	const int nIndex = int( point.y / Settings.Skin.RowSize );

	if ( ( nFlags & ( MK_LBUTTON|MK_RBUTTON ) ) != 0 )
	{
		if ( m_bDragStart )
		{
			if ( abs( point.x - m_ptDrag.x ) >= GetSystemMetrics( SM_CXDRAG ) ||
				 abs( point.y - m_ptDrag.y ) >= GetSystemMetrics( SM_CYDRAG ) )
			{
				OnBeginDrag( point );
				m_bDragStart = FALSE;
				m_bDragActive = TRUE;
			}
		}

		m_nHover = nIndex;
		m_wndTip.Hide();
		return;
	}

	m_bDragActive = FALSE;

	if ( nIndex == m_nHover )
	{
		if ( point.x < 22 && point.x > 10 )
			RedrawWindow( CRect( 1, ( nIndex * Settings.Skin.RowSize ) + 1, 16, ( nIndex * Settings.Skin.RowSize ) + ( Settings.Skin.RowSize - 1 ) ) );
		return;
	}

	// Expandable Tick Hoverstates
	if ( point.x < 18 )
	{
		CRect rcUpdate( 1, ( nIndex * Settings.Skin.RowSize ) + 1, 16, ( nIndex * Settings.Skin.RowSize ) + ( Settings.Skin.RowSize - 1 ) );
		if ( m_nHover > nIndex )
			rcUpdate.bottom = ( m_nHover * Settings.Skin.RowSize ) + ( Settings.Skin.RowSize - 1 );
		else if ( m_nHover >= 0 )
			rcUpdate.top = ( m_nHover * Settings.Skin.RowSize ) + 1;

		m_nHover = nIndex;
		RedrawWindow( rcUpdate );

		m_wndTip.Hide();
		return;
	}

	// ToDo: Delay lock for Settings.Interface.TipDelay
	//if ( ! m_wndTip.IsVisible() )
	//{
	//	static DWORD tUpdate = 0;
	//	DWORD tNow = GetTickCount();
	//	tUpdate = tNow;
	//	Sleep( Settings.Interface.TipDelay - 1 );
	//	if ( tUpdate != tNow )
	//		return;
	//}

	m_nHover = nIndex;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( pLock.Lock( 100 ) )
	{
		CDownload* pDownload;
		CDownloadSource* pSource;

		if ( HitTest( nIndex, &pDownload, &pSource ) )
		{
			if ( pDownload != NULL )
			{
				m_wndTip.Show( pDownload );
				return;
			}
			if ( pSource != NULL )
			{
				m_wndTip.Show( pSource );
				return;
			}
		}
	}

//	m_wndTip.Hide();
}

// Was BOOL CDownloadsCtrl::DropShowTarget(CList< CDownload* >* /*pSel*/, const CPoint& ptScreen)
void CDownloadsCtrl::OnMouseMoveDrag(const CPoint& ptScreen)
{
	CPoint ptLocal( ptScreen );
	ScreenToClient( &ptLocal );

	const int nIndex = int( ptLocal.y / Settings.Skin.RowSize );

	if ( nIndex == m_nHover )
		return;

	m_nHover = nIndex;
	CImageList::DragShowNolock( FALSE );
	RedrawWindow();
	CImageList::DragShowNolock( TRUE );

// Obsolete:
//	CSingleLock pLock( &Transfers.m_pSection, TRUE );
//	CDownload* pHit = NULL;
//	if ( bLocal )
//		HitTest( ptLocal, &pHit, NULL, NULL, NULL );
//	if ( pHit != m_pDragDrop )
//	{
//		CImageList::DragShowNolock( FALSE );
//		m_pDragDrop = pHit;
//		UpdateDownloadsData( TRUE );
//		pLock.Unlock();
//		RedrawWindow();
//		CImageList::DragShowNolock( TRUE );
//	}
}

void CDownloadsCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus( pOldWnd );
	Invalidate();
}

void CDownloadsCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus( pNewWnd );
	Invalidate();
}

void CDownloadsCtrl::OnBeginDrag(CPoint ptAction)
{
	m_wndTip.Hide();

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 300 ) ) return;

	m_pDeselect1 = NULL;
	m_pDeselect2 = NULL;

	CList< CDownload* >* pSel = new CList< CDownload* >;

	for ( POSITION pos = Downloads.GetIterator(); pos; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected )
			pSel->AddTail( pDownload );
	}

	if ( pSel->IsEmpty() )
	{
		delete pSel;
		return;
	}

	m_bCreateDragImage = TRUE;
	CImageList* pDragImage = CreateDragImage( pSel, ptAction );
	m_bCreateDragImage = FALSE;

	if ( pDragImage == NULL )
	{
		delete pSel;
		return;
	}

//	m_pDragDrop = NULL;

	ClientToScreen( &ptAction );

	CDownloadsWnd* pWindow = (CDownloadsWnd*)GetOwner();
	ASSERT_KINDOF( CDownloadsWnd, pWindow );
	pWindow->DragDownloads( pSel, pDragImage, ptAction );

	pLock.Unlock();
	UpdateWindow();
}

CImageList* CDownloadsCtrl::CreateDragImage(CList< CDownload* >* pSel, const CPoint& ptMouse)
{
	ASSUME_LOCK( Transfers.m_pSection );

	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );

	GetClientRect( &rcClient );

	for ( POSITION pos = pSel->GetHeadPosition(); pos; )
	{
		CDownload* pDownload = (CDownload*)pSel->GetNext( pos );
		GetRect( pDownload, &rcOne );

		if ( rcOne.IntersectRect( &rcClient, &rcOne ) )
		{
			rcAll.left		= min( rcAll.left, rcOne.left );
			rcAll.top		= min( rcAll.top, rcOne.top );
			rcAll.right		= max( rcAll.right, rcOne.right );
			rcAll.bottom	= max( rcAll.bottom, rcOne.bottom );
		}
	}

	BOOL bClipped = rcAll.Height() > MAX_DRAG_SIZE;

	if ( bClipped )
	{
		rcAll.left		= max( rcAll.left, ptMouse.x - MAX_DRAG_SIZE_2 );
		rcAll.right		= max( rcAll.right, ptMouse.x + MAX_DRAG_SIZE_2 );
		rcAll.top		= max( rcAll.top, ptMouse.y - MAX_DRAG_SIZE_2 );
		rcAll.bottom	= max( rcAll.bottom, ptMouse.y + MAX_DRAG_SIZE_2 );
	}

	CClientDC dcClient( this );
	CDC dcMem, dcDrag;
	CBitmap bmDrag;

	if ( ! dcMem.CreateCompatibleDC( &dcClient ) )
		return NULL;
	if ( ! dcDrag.CreateCompatibleDC( &dcClient ) )
		return NULL;
	if ( ! bmDrag.CreateCompatibleBitmap( &dcClient, rcAll.Width(), rcAll.Height() ) )
		return NULL;

	CBitmap *pOldDrag = dcDrag.SelectObject( &bmDrag );

	dcDrag.FillSolidRect( 0, 0, rcAll.Width(), rcAll.Height(), DRAG_COLOR_KEY );

	CRgn pRgn;
	if ( bClipped )
	{
		CPoint ptMiddle( ptMouse.x - rcAll.left, ptMouse.y - rcAll.top );
		pRgn.CreateEllipticRgn(	ptMiddle.x - MAX_DRAG_SIZE_2, ptMiddle.y - MAX_DRAG_SIZE_2,
								ptMiddle.x + MAX_DRAG_SIZE_2, ptMiddle.y + MAX_DRAG_SIZE_2 );
		dcDrag.SelectClipRgn( &pRgn );
	}

	CFont* pOldFont = (CFont*)dcDrag.SelectObject( &CoolInterface.m_fntNormal );

	int nIndex = 0;		// Workaround
	for ( POSITION pos = pSel->GetHeadPosition(); pos; nIndex++ )
	{
		CDownload* pDownload = (CDownload*)pSel->GetNext( pos );
		GetRect( pDownload, &rcOne );
		CRect rcDummy, rcOut( &rcOne );
		rcOut.OffsetRect( -rcAll.left, -rcAll.top );
		if ( ! rcDummy.IntersectRect( &rcAll, &rcOne ) )
			continue;

		for ( ; ! m_pDownloadsData[ nIndex ].m_bSelected && nIndex <= m_pDownloadsData.GetUpperBound(); nIndex++ );	// Workaround loop	(ToDo: Fix properly)

		dcDrag.FillSolidRect( &rcOut, DRAG_COLOR_KEY );
		PaintDownload( dcDrag, rcOut, &m_pDownloadsData[ nIndex ] );
	}

	dcDrag.SelectObject( pOldFont );
	dcDrag.SelectObject( pOldDrag );
	dcDrag.DeleteDC();

	CImageList* pAll = new CImageList();
	pAll->Create( rcAll.Width(), rcAll.Height(), ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	pAll->Create( rcAll.Width(), rcAll.Height(), ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	pAll->Create( rcAll.Width(), rcAll.Height(), ILC_COLOR16|ILC_MASK, 1, 1 );
	pAll->Add( &bmDrag, DRAG_COLOR_KEY );

	bmDrag.DeleteObject();

	pAll->BeginDrag( 0, ptMouse - rcAll.TopLeft() );

	return pAll;
}

int CDownloadsCtrl::GetExpandableColumnX() const
{
	int nTitleStarts = 0;

	HDITEM pColumn = {};
	pColumn.mask = HDI_LPARAM | HDI_WIDTH;

	for ( int nColumn = 0; m_wndHeader.GetItem( m_wndHeader.OrderToIndex( nColumn ), &pColumn ); nColumn++ )
	{
		if ( pColumn.lParam == COL_TITLE )
			break;

		nTitleStarts += pColumn.cxy;
	}

	return nTitleStarts;
}

UINT CDownloadsCtrl::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}
