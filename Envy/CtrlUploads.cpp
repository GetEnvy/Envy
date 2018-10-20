//
// CtrlUploads.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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
#include "CtrlUploads.h"
#include "UploadQueues.h"
#include "UploadQueue.h"
#include "UploadFiles.h"
#include "UploadFile.h"
#include "UploadTransfer.h"
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

#define HEADER_HEIGHT			20
//#define ITEM_HEIGHT			17	// Settings.Skin.RowSize

// Set Column Order
enum {
	COL_TITLE,
	COL_SIZE,
	COL_TRANSFER,
	COL_SPEED,
	COL_PROGRESS,
	COL_RATING,
	COL_USER,
	COL_CLIENT,
	COL_COUNTRY,
	COL_LAST	// Count
};


IMPLEMENT_DYNAMIC(CUploadsCtrl, CWnd)

BEGIN_MESSAGE_MAP(CUploadsCtrl, CWnd)
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
	ON_NOTIFY(HDN_ITEMCHANGEDW, AFX_IDW_PANE_FIRST, OnChangeHeader)
	ON_NOTIFY(HDN_ITEMCHANGEDA, AFX_IDW_PANE_FIRST, OnChangeHeader)
	ON_NOTIFY(HDN_ENDDRAG, AFX_IDW_PANE_FIRST, OnChangeHeader)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CUploadsCtrl construction

CUploadsCtrl::CUploadsCtrl()
	: m_nFocus		( 0 )
	, m_nHover		( -1 )
	, m_pDeselect	( NULL )
{
}

//////////////////////////////////////////////////////////////////////////////
// CUploadsCtrl operations

BOOL CUploadsCtrl::Create(CWnd* pParentWnd, UINT nID)
{
	return CWnd::CreateEx( WS_EX_CONTROLPARENT, NULL, L"CUploadsCtrl",
		WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_GROUP, CRect( 0 ), pParentWnd, nID );
}

BOOL CUploadsCtrl::Update()
{
	OnSize( 1982, 0, 0 );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CUploadsCtrl system message handlers

int CUploadsCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	CRect rect( 0, 0, 0, 0 );
	m_wndHeader.Create( WS_CHILD|HDS_DRAGDROP|HDS_HOTTRACK|HDS_FULLDRAG, rect, this, AFX_IDW_PANE_FIRST );
	m_wndHeader.SetFont( &theApp.m_gdiFont );

	m_wndTip.Create( this, &Settings.Interface.TipUploads );

	GetDesktopWindow()->GetWindowRect( &rect );

	InsertColumn( COL_TITLE, L"Uploaded File", LVCFMT_LEFT, rect.Width() > 1600 ? 400 : 300 );
	InsertColumn( COL_SIZE, L"Size", LVCFMT_CENTER, 64 );
	InsertColumn( COL_TRANSFER, L"Transfer", LVCFMT_CENTER, 64 );
	InsertColumn( COL_SPEED, L"Speed", LVCFMT_CENTER, 74 );
	InsertColumn( COL_PROGRESS, L"Progress", LVCFMT_CENTER, 100 );
	InsertColumn( COL_RATING, L"Rating", LVCFMT_CENTER, 0 );
	InsertColumn( COL_USER, L"Remote User", LVCFMT_CENTER, 134 );
	InsertColumn( COL_CLIENT, L"Client", LVCFMT_CENTER, 108 );
	InsertColumn( COL_COUNTRY, L"Country", LVCFMT_LEFT, 54 );

	LoadColumnState();

//	CoolInterface.LoadIconsTo( m_gdiProtocols, protocolIDs );

	m_nFocus	= 0;
	m_nHover	= -1;
	m_pDeselect	= NULL;

	return 0;
}

void CUploadsCtrl::OnDestroy()
{
	SaveColumnState();
	CWnd::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////////
// CUploadsCtrl column helpers

void CUploadsCtrl::InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth)
{
	HDITEM pColumn = {};

	pColumn.mask	= HDI_FORMAT | HDI_LPARAM | HDI_TEXT | HDI_WIDTH;
	pColumn.cxy		= nWidth;
	pColumn.pszText	= (LPTSTR)pszCaption;
	pColumn.fmt		= nFormat;
	pColumn.lParam	= nColumn;

	m_wndHeader.InsertItem( m_wndHeader.GetItemCount(), &pColumn );
}

void CUploadsCtrl::SaveColumnState()
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

	theApp.WriteProfileString( L"ListStates", L"CUploadCtrl.Ordering", strOrdering );
	theApp.WriteProfileString( L"ListStates", L"CUploadCtrl.Widths", strWidths );
}

BOOL CUploadsCtrl::LoadColumnState()
{
	CString strOrdering	= theApp.GetProfileString( L"ListStates", L"CUploadCtrl.Ordering", L"" );
	CString strWidths	= theApp.GetProfileString( L"ListStates", L"CUploadCtrl.Widths", L"" );

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
// CUploadsCtrl item helpers

void CUploadsCtrl::SelectTo(int nIndex)
{
	BOOL bShift		= GetAsyncKeyState( VK_SHIFT ) & 0x8000;
	BOOL bControl	= GetAsyncKeyState( VK_CONTROL ) & 0x8000;
	BOOL bRight		= GetAsyncKeyState( VK_RBUTTON ) & 0x8000;

	if ( ! bShift && ! bControl && ! bRight && m_pDeselect == NULL )
	{
		DeselectAll();
		Update();
	}

	int nMin, nMax;
	GetScrollRange( SB_VERT, &nMin, &nMax );
	nIndex = max( 0, min( nIndex - nMin, nMax - 1 ) );	// Visible index, to match DisplayData

	const int nScroll = GetScrollPos( SB_VERT );

	CUploadFile* pFile;
	CUploadQueue* pQueue;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	if ( bShift )
	{
		if ( m_nFocus < nIndex )
		{
			for ( m_nFocus++; m_nFocus <= nIndex; m_nFocus++ )
			{
				GetAt( m_nFocus, &pQueue, &pFile );
				if ( pQueue ) pQueue->m_bSelected = TRUE;
				if ( pFile )  pFile->m_bSelected = TRUE;
			}
		}
		else if ( m_nFocus > nIndex )
		{
			for ( m_nFocus--; m_nFocus >= nIndex; m_nFocus-- )
			{
				GetAt( m_nFocus, &pQueue, &pFile );
				if ( pQueue != NULL ) pQueue->m_bSelected = TRUE;
				if ( pFile != NULL ) pFile->m_bSelected = TRUE;
			}
		}

		m_nFocus = nIndex;
	}
	else
	{
		m_nFocus = nIndex;
		GetAt( m_nFocus, &pQueue, &pFile );

		if ( bControl )
		{
			if ( pQueue != NULL ) pQueue->m_bSelected = ! pQueue->m_bSelected;
			if ( pFile != NULL ) pFile->m_bSelected = ! pFile->m_bSelected;
		}
		else
		{
			if ( pQueue != NULL ) pQueue->m_bSelected = TRUE;
			if ( pFile != NULL ) pFile->m_bSelected = TRUE;
		}
	}

	BOOL bUpdate = TRUE;
	if ( m_nFocus < nScroll )
	{
		SetScrollPos( SB_VERT, m_nFocus );
	}
	else
	{
		CRect rcClient;
		GetClientRect( &rcClient );
		int nHeight = ( rcClient.bottom - HEADER_HEIGHT ) / Settings.Skin.RowSize - 1;
		if ( nHeight < 0 ) nHeight = 0;

		if ( m_nFocus > nScroll + nHeight )
			SetScrollPos( SB_VERT, max( 0, m_nFocus - nHeight ) );
		else
			bUpdate = FALSE;
	}

	UpdateUploadsData( TRUE );

	pLock.Unlock();

	bUpdate ? Update() : Invalidate();
}

void CUploadsCtrl::DeselectAll(CUploadFile* /*pExcept*/)
{
	CSingleLock pLock( &UploadQueues.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	UploadQueues.m_pTorrentQueue->m_bSelected = FALSE;
	UploadQueues.m_pHistoryQueue->m_bSelected = FALSE;

	for ( POSITION pos = UploadQueues.GetIterator(); pos; )
	{
		CUploadQueue* pQueue = UploadQueues.GetNext( pos );
		pQueue->m_bSelected = FALSE;
	}

	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
	//	if ( pFile == pExcept ) continue;
		pFile->m_bSelected = FALSE;
	}

	pLock.Unlock();

	Invalidate();
}

BOOL CUploadsCtrl::HitTest(const CPoint& point, CUploadQueue** ppQueue, CUploadFile** ppFile, int* pnIndex, RECT* prcItem)
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

	if ( ppQueue != NULL ) *ppQueue = NULL;
	if ( ppFile != NULL ) *ppFile = NULL;

	CSingleLock pLock( &UploadQueues.m_pSection );
	if ( ! pLock.Lock( 500 ) )
		return FALSE;

	for ( POSITION posQueue = GetQueueIterator(); posQueue && rcItem.top < rcClient.bottom; )
	{
		CUploadQueue* pQueue = GetNextQueue( posQueue );

		POSITION posFile = GetFileIterator( pQueue );
		if ( posFile == NULL ) continue;

		if ( nScroll > 0 )
		{
			nScroll --;
		}
		else
		{
			if ( rcItem.PtInRect( point ) )
			{
				if ( ppQueue != NULL ) *ppQueue = pQueue;
				if ( pnIndex != NULL ) *pnIndex = nIndex;
				if ( prcItem != NULL ) *prcItem = rcItem;
				return TRUE;
			}
			rcItem.OffsetRect( 0, Settings.Skin.RowSize );
		}

		nIndex ++;
		if ( ! pQueue->m_bExpanded ) continue;

		while ( posFile && rcItem.top < rcClient.bottom )
		{
			CUploadFile* pFile = GetNextFile( pQueue, posFile );
			if ( pFile == NULL ) continue;

			if ( nScroll > 0 )
			{
				nScroll --;
			}
			else
			{
				if ( rcItem.PtInRect( point ) )
				{
					if ( ppFile != NULL ) *ppFile = pFile;
					if ( pnIndex != NULL ) *pnIndex = nIndex;
					if ( prcItem != NULL ) *prcItem = rcItem;
					return TRUE;
				}
				rcItem.OffsetRect( 0, Settings.Skin.RowSize );
			}

			nIndex ++;
		}
	}

	return FALSE;
}

BOOL CUploadsCtrl::HitTest(int nIndex, CUploadQueue** ppQueue, CUploadFile** ppFile)
{
	ASSUME_LOCK( Transfers.m_pSection );

	int nScroll = GetScrollPos( SB_VERT );
	int nCount = nIndex - 1;

	if ( ppQueue != NULL ) *ppQueue = NULL;
	if ( ppFile != NULL ) *ppFile = NULL;

	CSingleLock pLock( &UploadQueues.m_pSection );
	if ( ! pLock.Lock( 500 ) )
		return FALSE;

	for ( POSITION posQueue = GetQueueIterator(); posQueue && nCount >= 0; )
	{
		CUploadQueue* pQueue = GetNextQueue( posQueue );

		POSITION posFile = GetFileIterator( pQueue );
		if ( posFile == NULL ) continue;

		if ( nScroll > 0 )
		{
			nScroll --;
		}
		else
		{
			if ( ! nCount )
			{
				if ( ppQueue != NULL ) *ppQueue = pQueue;
				return TRUE;
			}
			nCount--;
		}

		if ( ! pQueue->m_bExpanded ) continue;

		while ( posFile && nCount )
		{
			CUploadFile* pFile = GetNextFile( pQueue, posFile );
			if ( pFile == NULL ) continue;

			if ( nScroll > 0 )
			{
				nScroll--;
			}
			else
			{
				if ( ! nCount )
				{
					if ( ppFile != NULL ) *ppFile = pFile;
					return TRUE;
				}
				nCount--;
			}
		}
	}

	return FALSE;
}

BOOL CUploadsCtrl::GetAt(int nSelect, CUploadQueue** ppQueue, CUploadFile** ppFile)
{
	ASSUME_LOCK( Transfers.m_pSection );

	/*int nScroll =*/ GetScrollPos( SB_VERT );
	int nIndex = 0;

	if ( ppQueue != NULL ) *ppQueue = NULL;
	if ( ppFile != NULL ) *ppFile = NULL;

	CSingleLock pLock( &UploadQueues.m_pSection );
	if ( ! SafeLock( pLock ) )
		return FALSE;

	for ( POSITION posQueue = GetQueueIterator(); posQueue; )
	{
		CUploadQueue* pQueue = GetNextQueue( posQueue );

		POSITION posFile = GetFileIterator( pQueue );
		if ( posFile == NULL ) continue;

		if ( nIndex++ == nSelect )
		{
			if ( ppQueue != NULL ) *ppQueue = pQueue;
			return TRUE;
		}

		if ( ! pQueue->m_bExpanded ) continue;

		while ( posFile )
		{
			CUploadFile* pFile = GetNextFile( pQueue, posFile );
			if ( pFile == NULL ) continue;

			if ( nIndex++ == nSelect )
			{
				if ( ppFile != NULL )
					*ppFile = pFile;
				return TRUE;
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// CUploadsCtrl queue / file abstractation layer

POSITION CUploadsCtrl::GetQueueIterator()
{
	ASSUME_LOCK( UploadQueues.m_pSection );

	if ( Settings.Uploads.FilterMask & ULF_TORRENT )
		return (POSITION)UploadQueues.m_pTorrentQueue;
	if ( Settings.Uploads.FilterMask & ( ULF_ACTIVE | ULF_QUEUED ) )
		return UploadQueues.GetIterator();
	if ( Settings.Uploads.FilterMask & ULF_HISTORY )
		return (POSITION)UploadQueues.m_pHistoryQueue;

	return NULL;
}

CUploadQueue* CUploadsCtrl::GetNextQueue(POSITION& pos)
{
	ASSUME_LOCK( UploadQueues.m_pSection );
	ASSERT( pos != NULL );

	if ( pos == (POSITION)UploadQueues.m_pTorrentQueue )
	{
		if ( Settings.Uploads.FilterMask & ( ULF_ACTIVE | ULF_QUEUED ) )
		{
			pos = UploadQueues.GetIterator();
			if ( pos == NULL )
				pos = (POSITION)UploadQueues.m_pHistoryQueue;
		}
		else if ( Settings.Uploads.FilterMask & ULF_HISTORY )
		{
			pos = (POSITION)UploadQueues.m_pHistoryQueue;
		}
		else
		{
			pos = NULL;
		}

		return UploadQueues.m_pTorrentQueue;
	}
	else if ( pos == (POSITION)UploadQueues.m_pHistoryQueue )
	{
		pos = NULL;
		return UploadQueues.m_pHistoryQueue;
	}
	else // Normal queue
	{
		CUploadQueue* pQueue = UploadQueues.GetNext( pos );

		if ( pos == NULL && ( Settings.Uploads.FilterMask & ULF_HISTORY ) )
			pos = (POSITION)UploadQueues.m_pHistoryQueue;

		return pQueue;
	}
}

POSITION CUploadsCtrl::GetFileIterator(CUploadQueue* pQueue)
{
	ASSUME_LOCK( UploadQueues.m_pSection );

	if ( pQueue == UploadQueues.m_pTorrentQueue )
	{
		for ( POSITION posNext = UploadFiles.GetIterator(); posNext; )
		{
			POSITION posThis = posNext;
			CUploadFile* pFile = UploadFiles.GetNext( posNext );
			CUploadTransfer* pTransfer = pFile->GetActive();
			if ( pTransfer == NULL || pTransfer->m_nState == upsNull ) continue;
			if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
			return posThis;
		}
	}
	else if ( pQueue == UploadQueues.m_pHistoryQueue )
	{
		for ( POSITION posNext = UploadFiles.GetIterator(); posNext; )
		{
			POSITION posThis = posNext;
			CUploadFile* pFile = UploadFiles.GetNext( posNext );
			CUploadTransfer* pTransfer = pFile->GetActive();
			if ( pTransfer != NULL )
			{
				if ( pTransfer->m_nProtocol == PROTOCOL_BT && pTransfer->m_nState != upsNull ) continue;
				if ( pTransfer->m_pQueue != NULL ) continue;
			}
			return posThis;
		}
	}
	else // Normal queue
	{
		if ( Settings.Uploads.FilterMask & ULF_ACTIVE )
		{
			if ( pQueue->GetActiveCount() > 0 )
				return pQueue->GetActiveIterator();
		}

		if ( Settings.Uploads.FilterMask & ULF_QUEUED )
		{
			if ( pQueue->GetQueuedCount() > 0 )
				return (POSITION)1;
		}
	}

	return NULL;
}

CUploadFile* CUploadsCtrl::GetNextFile(CUploadQueue* pQueue, POSITION& pos, int* pnPosition)
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( pos != NULL );

	if ( pnPosition != NULL )
		*pnPosition = -1;

	if ( pQueue == UploadQueues.m_pTorrentQueue )
	{
		CUploadFile* pReturn = UploadFiles.GetNext( pos );

		for ( ; pos; )
		{
			POSITION posThis = pos;
			CUploadFile* pFile = UploadFiles.GetNext( pos );
			CUploadTransfer* pTransfer = pFile->GetActive();
			if ( pTransfer == NULL || pTransfer->m_nState == upsNull ) continue;
			if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
			pos = posThis;
			break;
		}

		return pReturn;
	}

	if ( pQueue == UploadQueues.m_pHistoryQueue )
	{
		CUploadFile* pReturn = UploadFiles.GetNext( pos );

		for ( ; pos; )
		{
			POSITION posThis = pos;
			CUploadFile* pFile = UploadFiles.GetNext( pos );
			CUploadTransfer* pTransfer = pFile->GetActive();
			if ( pTransfer != NULL )
			{
				//if ( pTransfer->m_nProtocol == PROTOCOL_BT && pTransfer->m_nState != upsNull ) continue;
				if ( pTransfer->m_nState != upsNull ) continue;
				if ( pTransfer->m_pQueue != NULL ) continue;
			}
			pos = posThis;
			break;
		}

		return pReturn;
	}

	if ( (DWORD_PTR)pos > pQueue->GetQueuedCount() )
	{
		CUploadTransfer* pTransfer = pQueue->GetNextActive( pos );

		if ( pos == NULL && ( Settings.Uploads.FilterMask & ULF_QUEUED ) )
		{
			if ( pQueue->GetQueuedCount() > 0 )
				pos = (POSITION)1;
		}

		if ( pnPosition != NULL )
			*pnPosition = 0;
		return pTransfer->m_pBaseFile;
	}

	// Default
	{
		DWORD_PTR nPos = (DWORD_PTR)pos;
		CUploadTransfer* pTransfer = pQueue->GetQueuedAt( nPos - 1 );
		if ( pnPosition != NULL )
			*pnPosition = static_cast< int >( nPos );
		nPos++;
		if ( nPos > pQueue->GetQueuedCount() )
			nPos = 0;
		pos = (POSITION)nPos;
		return pTransfer->m_pBaseFile;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CUploadsCtrl presentation message handlers

void CUploadsCtrl::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 ) CWnd::OnSize( nType, cx, cy );

	int nWidth = 0, nHeight = 0;

	CRect rcClient;
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

	CSingleLock pTransfersLock( &Transfers.m_pSection );		// First, for GetNextFile()
	if ( ! pTransfersLock.Lock( 500 ) )
		return;

	CSingleLock pUploadQueuesLock( &UploadQueues.m_pSection );
	if ( ! pUploadQueuesLock.Lock( 250 ) )
		return;

	for ( POSITION posQueue = GetQueueIterator(); posQueue; )
	{
		CUploadQueue* pQueue = GetNextQueue( posQueue );

		POSITION posFile = GetFileIterator( pQueue );

		if ( posFile == NULL )
		{
			pQueue->m_bSelected = FALSE;
			continue;
		}

		nHeight++;

		if ( ! pQueue->m_bExpanded )
			continue;

		while ( posFile )
		{
			if ( GetNextFile( pQueue, posFile ) )
				nHeight++;
		}
	}

	pUploadQueuesLock.Unlock();
	pTransfersLock.Unlock();

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
// CDownloadsCtrl populate duplicate Uploads/Queues data (isolated to minimize locks)

void CUploadsCtrl::UpdateUploadsData(BOOL bForce /*FALSE*/)
{
	static DWORD tUpdate = 0;
	if ( ! bForce && tUpdate + 500 > GetTickCount() )
		return;

	CRect rcClient;
	GetClientRect( &rcClient );
	const int nMin = GetScrollPos( SB_VERT );
	const int nMax = nMin + ( rcClient.Height() / Settings.Skin.RowSize ) + 1;
	int nCount = 0;

	CSingleLock pUploadQueuesLock( &UploadQueues.m_pSection );
	if ( ! pUploadQueuesLock.Lock( 250 ) )
		return;

	CSingleLock pTransfersLock( &Transfers.m_pSection );
	if ( ! pTransfersLock.Lock( 50 ) )
		return;

	INT_PTR nQueue = 0;
	for ( POSITION posQueue = GetQueueIterator(); posQueue; )
	{
		CUploadQueue* pQueue = GetNextQueue( posQueue );

		POSITION posFile = GetFileIterator( pQueue );
		if ( posFile == NULL )
			continue;

		if ( nCount++ < nMin - 1 && ! pQueue->m_bExpanded )
			continue;

		CQueueDisplayData pQueueData( pQueue );

		if ( pQueue->m_bExpanded )
		{
			if ( nCount < nMin )
				pQueueData.m_sName.Empty();		// Do not display

			if ( pQueueData.m_nCount )
				pQueueData.m_pUploadsData.SetSize( min( pQueueData.m_nCount, (DWORD)nMax - nCount ) );

			UINT nUpload = 0;
			while ( posFile )
			{
				int nPosition;
				CUploadFile* pFile = GetNextFile( pQueue, posFile, &nPosition );
				if ( pFile == NULL )
					continue;

				if ( nCount++ < nMin )
					continue;

				pQueueData.m_pUploadsData.SetAtGrow( nUpload, CUploadDisplayData( pFile, pFile->GetActive() ) );
				nUpload++;

				if ( nCount > nMax )
					break;
			}
			pQueueData.m_nCount = nUpload;
		}

		m_pDisplayData.SetAtGrow( nQueue, pQueueData );
		nQueue++;

		if ( nCount > nMax )
			break;
	}

	pTransfersLock.Unlock();
	pUploadQueuesLock.Unlock();

	while ( m_pDisplayData.GetCount() > nQueue )
		m_pDisplayData.RemoveAt( nQueue );

	tUpdate = GetTickCount();
}

//////////////////////////////////////////////////////////////////////////////
// CUploadsCtrl painting

void CUploadsCtrl::OnPaint()
{
	CPaintDC dc( this );
	CRect rcClient, rcItem;

	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;

	rcItem.CopyRect( &rcClient );
	rcItem.left -= GetScrollPos( SB_HORZ );
	rcItem.bottom = rcItem.top + Settings.Skin.RowSize;

	// ToDo: Better Focus scroll tracking for limited DisplayData
	const BOOL bFocus = ( GetFocus() == this );
	const int nFocus  = bFocus ? m_nFocus - GetScrollPos( SB_VERT ) : -1;

	if ( Settings.General.LanguageRTL )
		dc.SetTextAlign( TA_RTLREADING );

	UpdateUploadsData();

	CFont* pfOld = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );

	int nIndex = 0;
	int nDisplayCount = (int)m_pDisplayData.GetCount();
	for ( int nQueue = 0; nQueue < nDisplayCount && rcItem.top < rcClient.bottom; nQueue++ )
	{
		const CQueueDisplayData* pQueueData = &m_pDisplayData[ nQueue ];

		if ( rcItem.bottom > rcClient.top && ! pQueueData->m_sName.IsEmpty() )
		{
			PaintQueue( dc, rcItem, pQueueData, bFocus && ( nFocus == nIndex ) );
			rcItem.OffsetRect( 0, Settings.Skin.RowSize );
			nIndex++;
		}

		if ( ! pQueueData->m_bExpanded )
			continue;

		for ( int nUpload = 0; (DWORD)nUpload < pQueueData->m_nCount && rcItem.top < rcClient.bottom; nUpload++ )
		{
			PaintFile( dc, rcItem, &pQueueData->m_pUploadsData[ nUpload ], bFocus && ( nFocus == nIndex ) );
			rcItem.OffsetRect( 0, Settings.Skin.RowSize );
			nIndex++;
		}
	}

	dc.SelectObject( pfOld );

	rcClient.top = rcItem.top;
	if ( rcClient.top < rcClient.bottom )
		dc.FillSolidRect( &rcClient, Colors.m_crWindow );
}

void CUploadsCtrl::PaintQueue(CDC& dc, const CRect& rcRow, const CQueueDisplayData* pQueueData, BOOL bFocus)
{
//	ASSUME_NO_LOCK( UploadQueues.m_pSection );		// Note no locks

	const BOOL bSelected = pQueueData->m_bSelected;
	//const BOOL bActive = bSelected && ( GetFocus() == this );
	BOOL bLeftMargin = TRUE;

	COLORREF crNatural	= Colors.m_crWindow;
	COLORREF crBack		= bSelected ? Colors.m_crHighlight : crNatural;
	COLORREF crLeftMargin = crBack;

	// Skinnable Selection Highlight
	BOOL bSelectmark = FALSE;
	if ( bSelected && Images.DrawButtonState( &dc, &rcRow, GetFocus() == this ? IMAGE_SELECTED : IMAGE_SELECTEDGREY ) )
	{
		bSelectmark = TRUE;
	}
	else
	{
		// Update Full Row Highlight
		dc.FillSolidRect( rcRow, crBack );
		dc.SetBkColor( crBack );
	}

	dc.SetBkMode( bSelectmark ? TRANSPARENT : OPAQUE );
	dc.SetTextColor( bSelected ? Colors.m_crHiText : Colors.m_crText );

	int nTextLeft = rcRow.right, nTextRight = rcRow.left;

	HDITEM pColumn = {};
	pColumn.mask = HDI_FORMAT | HDI_LPARAM;

	dc.SelectObject( &CoolInterface.m_fntBold );

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
			crLeftMargin = ( bLeftMargin ? crNatural : bSelected ? -1 : crBack );
			if ( bLeftMargin || ! bSelected && rcCell.Height() > 16 )
				dc.FillSolidRect( rcCell.left, rcCell.top + 16, 32, rcCell.Height() - 16, crLeftMargin );

			{
				POINT ptHover;
				GetCursorPos( &ptHover );
				ScreenToClient( &ptHover );
				RECT rcTick = { rcCell.left + 1, rcCell.top + 1, rcCell.left + 15, rcCell.bottom - 1 };

				if ( PtInRect( &rcTick, ptHover ) )
					CoolInterface.Draw( &dc, pQueueData->m_bExpanded ? IDI_CLOSETICK_HOVER : IDI_OPENTICK_HOVER, 16, rcCell.left, rcCell.top, crLeftMargin );
				else
					CoolInterface.Draw( &dc, pQueueData->m_bExpanded ? IDI_CLOSETICK : IDI_OPENTICK, 16, rcCell.left, rcCell.top, crLeftMargin );
			}

			rcCell.left += 16;
			if ( pQueueData->m_bTorrentQueue )
			{
				ImageList_DrawEx( m_gdiProtocols, PROTOCOL_BT, dc.GetSafeHdc(),
						rcCell.left, rcCell.top, 16, 16, crLeftMargin, CLR_DEFAULT, bSelected ? ILD_SELECTED : ILD_NORMAL );
			}
			else if ( pQueueData->m_bHTTPQueue )
			{
				ImageList_DrawEx( m_gdiProtocols, PROTOCOL_HTTP, dc.GetSafeHdc(),
						rcCell.left, rcCell.top, 16, 16, crLeftMargin, CLR_DEFAULT, bSelected ? ILD_SELECTED : ILD_NORMAL );
			}
			else if ( pQueueData->m_bED2KQueue )
			{
				ImageList_DrawEx( m_gdiProtocols, PROTOCOL_ED2K, dc.GetSafeHdc(),
						rcCell.left, rcCell.top, 16, 16, crLeftMargin, CLR_DEFAULT, bSelected ? ILD_SELECTED : ILD_NORMAL );
			}
			else
			{
				CoolInterface.Draw( &dc, pQueueData->m_bExpanded ? IDI_FOLDER_OPEN : IDI_FOLDER_CLOSED,
					16, rcCell.left, rcCell.top, crLeftMargin, bSelected );
			}
			rcCell.left += 16;
			if ( bLeftMargin || ! bSelected )
				dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crLeftMargin );
			rcCell.left++;

			strText = pQueueData->m_sName;
			break;

		case COL_SIZE:
			if ( ! pQueueData->m_bHistoryQueue )
				strText.Format( L"%u/%u", pQueueData->m_nMinTransfers, pQueueData->m_nMaxTransfers );	// GetTransferCount(), GetQueuedCount()
			break;

		case COL_SPEED:
			if ( ! pQueueData->m_bHistoryQueue )
				strText = Settings.SmartSpeed( pQueueData->m_nSpeed );
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
	}

	// Non-column whitespace area (redundant)
	//if ( nTextRight < rcRow.right && ! bSelectmark )
	//	dc.FillSolidRect( nTextRight, rcRow.top, rcRow.right, rcRow.bottom, crBack );

	dc.SelectObject( &CoolInterface.m_fntNormal );

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

void CUploadsCtrl::PaintFile(CDC& dc, const CRect& rcRow, const CUploadDisplayData* pUploadData, BOOL bFocus)
{
//	ASSUME_NO_LOCK( Transfers.m_pSection );	// Note no lock

	const BOOL bSelected = pUploadData->m_bSelected;
	//const BOOL bFocus = bSelected && ( GetFocus() == this );
	BOOL bLeftMargin = TRUE;

	COLORREF crNatural		= Colors.m_crWindow;
	COLORREF crBorder		= bSelected ? Colors.m_crFragmentBorderSelected : Colors.m_crFragmentBorder;
	COLORREF crBack			= bSelected ? Colors.m_crHighlight : crNatural;
	COLORREF crLeftMargin	= crBack;

	// Skinnable Selection Highlight
	BOOL bSelectmark = FALSE;
	if ( bSelected && Images.m_bmSelected.m_hObject )
	{
		CRect rcDraw = rcRow;
		if ( Images.m_bmSelectedGrey.m_hObject && GetFocus() != this )
			CoolInterface.DrawWatermark( &dc, &rcDraw, &Images.m_bmSelectedGrey );
		else
			CoolInterface.DrawWatermark( &dc, &rcDraw, &Images.m_bmSelected );
		bSelectmark = TRUE;
	}
	else
	{
		// Update Full Row Highlight
		dc.FillSolidRect( rcRow, crBack );
		dc.SetBkColor( crBack );
	}

	dc.SetBkMode( bSelectmark ? TRANSPARENT : OPAQUE );

	if ( bSelected )
		dc.SetTextColor( Colors.m_crHiText );
	else if ( pUploadData->m_bTransferNull )
		dc.SetTextColor( Colors.m_crDisabled );
	else
		dc.SetTextColor( Colors.m_crText );

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
			if ( bLeftMargin || ! bSelectmark && Settings.Skin.RowSize > 16 )
				dc.FillSolidRect( rcCell.left, rcCell.top + 16, 16, rcCell.Height() - 16, crLeftMargin );

			{
				CString strExt = PathFindExtension( pUploadData->m_sPath );
				if ( pUploadData->m_bTorrent || strExt.Compare( L".partial" ) == 0 )
					strExt = PathFindExtension( pUploadData->m_sName );

				if ( pUploadData->m_bTorrent && ( strExt.IsEmpty() || ! IsValidExtension( strExt ) ) )
					CoolInterface.Draw( &dc, IDI_MULTIFILE, 16, rcCell.left, rcCell.top, crLeftMargin, bSelected );
				else
					ShellIcons.Draw( &dc, ShellIcons.Get( strExt, 16 ), 16, rcCell.left, rcCell.top, crLeftMargin, bSelected );
			}

			rcCell.left += 16;
			if ( bLeftMargin || ! bSelectmark )
				dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crLeftMargin );
			rcCell.left++;
			strText = pUploadData->m_sName;
			break;

		case COL_SIZE:
			strText = Settings.SmartVolume( pUploadData->m_nSize );
			break;

		case COL_PROGRESS:
			rcCell.DeflateRect( 1, 2 );
			dc.Draw3dRect( &rcCell, crBorder, crBorder );
			rcCell.DeflateRect( 1, 1 );
			CFragmentBar::DrawUpload( &dc, &rcCell, pUploadData, crNatural );
			break;

		case COL_TRANSFER:
			if ( pUploadData->m_nUploaded != NULL )
				strText = Settings.SmartVolume( pUploadData->m_nUploaded );
			break;

		case COL_SPEED:
			if ( pUploadData->m_bTransferNull )
			{
				LoadString( strText, IDS_STATUS_COMPLETED );
			}
			else if ( pUploadData->m_bTorrent )
			{
				if ( ! pUploadData->m_sState.IsEmpty() )
					strText = pUploadData->m_sState;	// IDS_STATUS_UNINTERESTED IDS_STATUS_CHOKED
				else if ( pUploadData->m_nSpeed )
					strText = Settings.SmartSpeed( pUploadData->m_nSpeed );
			}
			else if ( pUploadData->m_nPosition > 0 )
			{
				strText.Format( L"%s %i", (LPCTSTR)LoadString( IDS_STATUS_Q ), pUploadData->m_nPosition );
			}
			else
			{
				if ( pUploadData->m_nSpeed )
					strText = Settings.SmartSpeed( pUploadData->m_nSpeed );
				else
					LoadString( strText, IDS_STATUS_NEXT );
			}
			break;

		case COL_RATING:
			strText.Format( L"%u", pUploadData->m_nUserRating );
			break;

		case COL_USER:
		//	if ( pUploadData->m_bTransferNull )
		//		strText.Empty();
			if ( pUploadData->m_sRemoteNick.IsEmpty() )
				strText = pUploadData->m_sAddress;
			else
				strText = pUploadData->m_sRemoteNick + L" (" + pUploadData->m_sAddress + L")";
			break;

		case COL_CLIENT:
		//	if ( ! pUploadData->m_bTransferNull )
				strText = pUploadData->m_sUserAgent;
			break;

		case COL_COUNTRY:
			int nFlagImage = Flags.GetFlagIndex( pUploadData->m_sCountry );

			if ( ! bSelectmark )
				dc.FillSolidRect( rcCell.left, rcCell.top, Flags.Width + 4, rcCell.Height(), crBack );
			rcCell.left += 2;
			if ( nFlagImage >= 0 )
				Flags.Draw( nFlagImage, dc.GetSafeHdc(), rcCell.left, rcCell.top,
					bSelectmark ? CLR_NONE : crBack, CLR_DEFAULT, bSelected ? ILD_SELECTED : ILD_NORMAL );
			rcCell.left += Flags.Width;

			strText = pUploadData->m_sCountry;
			break;
		}

		if ( strText.IsEmpty() )	// pColumn.lParam == COL_PROGRESS
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

		int nPos = 0;

		switch ( pColumn.fmt & LVCFMT_JUSTIFYMASK )
		{
		case LVCFMT_CENTER:
			nPos = ( ( rcCell.left + rcCell.right ) / 2 ) - ( dc.GetTextExtent( strText ).cx / 2 );
			break;
		case LVCFMT_RIGHT:
			nPos = ( rcCell.right - 4 - dc.GetTextExtent( strText ).cx );
			break;
		default:
			nPos = ( rcCell.left + 4 );
			break;
		}

		dc.SetBkColor( bSelectmark ? CLR_NONE : crBack );
		dc.ExtTextOut( nPos, rcCell.top + 2,
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

void CUploadsCtrl::OnSkinChange()
{
	m_wndHeader.SetFont( &CoolInterface.m_fntNormal );

	CoolInterface.LoadIconsTo( m_gdiProtocols, protocolIDs );

	// Update Dropshadow
	m_wndTip.DestroyWindow();
	m_wndTip.Create( this, &Settings.Interface.TipUploads );
}

//////////////////////////////////////////////////////////////////////////////
// CUploadsCtrl interaction message handlers

void CUploadsCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	SCROLLINFO pInfo = {};
	pInfo.cbSize	= sizeof( pInfo );
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;

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
	Invalidate();
}

void CUploadsCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	SCROLLINFO pInfo = {};
	pInfo.cbSize	= sizeof( pInfo );
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;

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

BOOL CUploadsCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	OnVScroll( SB_THUMBPOSITION, GetScrollPos( SB_VERT ) - zDelta / WHEEL_DELTA * theApp.m_nMouseWheel );
	return TRUE;
}

void CUploadsCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_wndTip.Hide();

	CSingleLock pLock( &Transfers.m_pSection, FALSE );	// Only where needed

	switch ( nChar )
	{
	case VK_HOME:
		SelectTo( 0 );
		return;
	case VK_END:
		{
			int nMin, nMax;
			GetScrollRange( SB_VERT, &nMin, &nMax );
			if ( nMax > 0 )
				SelectTo( nMax - 1 );
		}
		return;
	case VK_UP:
		SelectTo( m_nFocus - 1 );
		return;
	case VK_DOWN:
		SelectTo( m_nFocus + 1 );
		return;
	case VK_PRIOR:
		{
			CRect rcClient;
			GetClientRect( &rcClient );
			int nMax = ( rcClient.Height() / Settings.Skin.RowSize ) - 1;
			if ( nMax < 1 ) nMax = 1;
			SelectTo( m_nFocus - nMax );
		}
		return;
	case VK_NEXT:
		{
			CRect rcClient;
			GetClientRect( &rcClient );
			int nMax = ( rcClient.Height() / Settings.Skin.RowSize ) - 1;
			if ( nMax < 1 ) nMax = 1;
			SelectTo( m_nFocus + nMax );
		}
		return;
	case VK_LEFT:
	case '-':
		if ( SafeLock( pLock ) )
		{
			CUploadFile* pFile;
			CUploadQueue* pQueue;
			if ( GetAt( m_nFocus, &pQueue, &pFile ) )
			{
				if ( pFile != NULL && pFile->GetActive() != NULL )
					pQueue = pFile->GetActive()->m_pQueue;
				if ( pQueue != NULL && pQueue->m_bExpanded == TRUE )
				{
					pQueue->m_bExpanded = FALSE;
					pLock.Unlock();
					Update();
				}
			}
		}
		return;
	case VK_RIGHT:
	case '+':
		if ( SafeLock( pLock ) )
		{
			CUploadQueue* pQueue;
			if ( GetAt( m_nFocus, &pQueue, NULL ) )
			{
				if ( pQueue != NULL && pQueue->m_bExpanded == FALSE )
				{
					pQueue->m_bExpanded = TRUE;
					pLock.Unlock();
					Update();
				}
			}
		}
		return;
	}

	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CUploadsCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CUploadFile* pFile;
	CUploadQueue* pQueue;
	CRect rcItem;
	int nIndex;

	SetFocus();
	m_wndTip.Hide();

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	if ( HitTest( point, &pQueue, &pFile, &nIndex, &rcItem ) )
	{
		int nTitleStarts = GetExpandableColumnX();
		if ( point.x > nTitleStarts && point.x <= nTitleStarts + rcItem.left + 16 )
		{
			if ( pQueue != NULL )
			{
				pQueue->m_bExpanded = ! pQueue->m_bExpanded;

				if ( ! pQueue->m_bExpanded )
				{
					for ( POSITION posActive = pQueue->GetActiveIterator(); posActive; )
					{
						CUploadTransfer* pTransfer = pQueue->GetNextActive( posActive );
						if ( pTransfer->m_pBaseFile != NULL )
							pTransfer->m_pBaseFile->m_bSelected = FALSE;
					}

					for ( DWORD nPos = 0; nPos < pQueue->GetQueuedCount(); nPos++ )
					{
						CUploadTransfer* pTransfer = (CUploadTransfer*)pQueue->GetQueuedAt( nPos );
						if ( pTransfer->m_pBaseFile != NULL )
							pTransfer->m_pBaseFile->m_bSelected = FALSE;
					}
				}

				UpdateUploadsData( TRUE );

				pLock.Unlock();
				Update();
			}
		}
		else
		{
			if ( pFile != NULL && pFile->m_bSelected )
			{
				if ( ( nFlags & ( MK_SHIFT | MK_CONTROL | MK_RBUTTON ) ) == 0 )
					m_pDeselect = pFile;
			}
			else if ( nFlags & MK_RBUTTON )
			{
				DeselectAll();
			}

			SelectTo( nIndex );
		}
	}
	else if ( ( nFlags & ( MK_SHIFT | MK_CONTROL ) ) == 0 )
	{
		pLock.Unlock();
		DeselectAll();
		Update();
	}
}

void CUploadsCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	//m_wndTip.Hide();
	OnLButtonDown( nFlags, point );
	CWnd::OnRButtonDown( nFlags, point );
}

void CUploadsCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nTitleStarts = GetExpandableColumnX();
	if ( point.x <= nTitleStarts + 16 && point.x > nTitleStarts )
		return OnLButtonDown( nFlags, point );	// Toggle

	CUploadFile* pFile;
	CUploadQueue* pQueue;
	CRect rcItem;

	SetFocus();

	CSingleLock pLock( &Transfers.m_pSection );
	if ( SafeLock( pLock ) )
	{
		if ( HitTest( point, &pQueue, &pFile, NULL, &rcItem ) )
		{
			if ( pQueue != NULL )
			{
				GetOwner()->PostMessage( WM_TIMER, 5 );
				GetOwner()->PostMessage( WM_COMMAND, ID_UPLOADS_SETTINGS );
			}
			else if ( pFile != NULL )
			{
				GetOwner()->PostMessage( WM_TIMER, 5 );
				GetOwner()->PostMessage( WM_COMMAND, ID_UPLOADS_LAUNCH );
			}
		}

		pLock.Unlock();
	}

	CWnd::OnLButtonDblClk( nFlags, point );
}

void CUploadsCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( m_pDeselect )
	{
		DeselectAll( m_pDeselect );
		m_pDeselect = NULL;
	}

	CWnd::OnLButtonUp( nFlags, point );
}

void CUploadsCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ( m_pDeselect )
	{
		DeselectAll( m_pDeselect );
		m_pDeselect = NULL;
	}

	CWnd::OnRButtonUp( nFlags, point );
}

void CUploadsCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove( nFlags, point );

	const int nIndex = int( point.y / Settings.Skin.RowSize );

	if ( ( nFlags & ( MK_LBUTTON|MK_RBUTTON ) ) != 0 )
	{
		m_wndTip.Hide();
		m_nHover = nIndex;
		return;
	}

	if ( nIndex == m_nHover )
	{
		if ( point.x < 22 && point.x > 10 )
			RedrawWindow( CRect( 1, ( nIndex * Settings.Skin.RowSize ) + 1, 16, ( nIndex * Settings.Skin.RowSize ) + ( Settings.Skin.RowSize - 1 ) ) );
		return;
	}

	// Expandable Tick Hoverstates
	if ( point.x < 18 )
	{
		CRect rcUpdate( 1, ( nIndex * Settings.Skin.RowSize ) + 1, 15, ( nIndex * Settings.Skin.RowSize ) + ( Settings.Skin.RowSize - 1 ) );
		if ( m_nHover > nIndex )
			rcUpdate.bottom = ( m_nHover * Settings.Skin.RowSize ) + ( Settings.Skin.RowSize - 1 );
		else if ( m_nHover >= 0 )
			rcUpdate.top = ( m_nHover * Settings.Skin.RowSize ) + 1;

		m_nHover = nIndex;
		RedrawWindow( rcUpdate );

		m_wndTip.Hide();
		return;
	}

	CSingleLock pLock( &Transfers.m_pSection );
	if ( pLock.Lock( 100 ) )
	{
		m_nHover = nIndex;
		CUploadFile* pFile;
		if ( HitTest( nIndex, NULL, &pFile ) )
		{
			if ( pFile != NULL )
			{
				m_wndTip.Show( pFile );
				return;
			}
		}
	//	m_wndTip.Hide();
	}
}

void CUploadsCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus( pOldWnd );
	Invalidate();
}

void CUploadsCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus( pNewWnd );
	Invalidate();
}

void CUploadsCtrl::OnChangeHeader(NMHDR* /*pNotifyStruct*/, LRESULT* /*pResult*/)
{
	Update();
}

int CUploadsCtrl::GetExpandableColumnX() const
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

UINT CUploadsCtrl::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}
