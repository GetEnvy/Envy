//
// CtrlLibraryTreeView.cpp
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
#include "CtrlLibraryTreeView.h"
#include "CtrlLibraryFrame.h"
#include "CtrlCoolBar.h"
#include "CtrlCoolTip.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "FileExecutor.h"
#include "SchemaCache.h"
#include "ShellIcons.h"
#include "Skin.h"

#include "DlgFolderScan.h"
#include "DlgFolderProperties.h"
#include "DlgFilePropertiesSheet.h"
#include "DlgFileCopy.h"
#include "DlgCollectionExport.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CLibraryTreeView, CWnd)

BEGIN_MESSAGE_MAP(CLibraryTreeView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_SETFOCUS()
	ON_WM_GETDLGCODE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_XBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_PARENT, OnUpdateLibraryParent)
	ON_COMMAND(ID_LIBRARY_PARENT, OnLibraryParent)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_EXPLORE, OnUpdateLibraryExplore)
	ON_COMMAND(ID_LIBRARY_EXPLORE, OnLibraryExplore)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_SCAN, OnUpdateLibraryScan)
	ON_COMMAND(ID_LIBRARY_SCAN, OnLibraryScan)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_SHARED_FOLDER, OnUpdateLibraryShared)
	ON_COMMAND(ID_LIBRARY_SHARED_FOLDER, OnLibraryShared)
	ON_COMMAND(ID_LIBRARY_ADD, OnLibraryAdd)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REMOVE, OnUpdateLibraryRemove)
	ON_COMMAND(ID_LIBRARY_REMOVE, OnLibraryRemove)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_PROPERTIES, OnUpdateLibraryFolderProperties)
	ON_COMMAND(ID_LIBRARY_FOLDER_PROPERTIES, OnLibraryFolderProperties)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_NEW, OnUpdateLibraryFolderNew)
	ON_COMMAND(ID_LIBRARY_FOLDER_NEW, OnLibraryFolderNew)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_DELETE, OnUpdateLibraryFolderDelete)
	ON_COMMAND(ID_LIBRARY_FOLDER_DELETE, OnLibraryFolderDelete)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_METADATA, OnUpdateLibraryFolderMetadata)
	ON_COMMAND(ID_LIBRARY_FOLDER_METADATA, OnLibraryFolderMetadata)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_ENQUEUE, OnUpdateLibraryFolderEnqueue)
	ON_COMMAND(ID_LIBRARY_FOLDER_ENQUEUE, OnLibraryFolderEnqueue)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_FILE_PROPERTIES, OnUpdateLibraryFolderFileProperties)
	ON_COMMAND(ID_LIBRARY_FOLDER_FILE_PROPERTIES, OnLibraryFolderFileProperties)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REBUILD, OnUpdateLibraryRebuild)
	ON_COMMAND(ID_LIBRARY_REBUILD, OnLibraryRebuild)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_EXPORT_COLLECTION, OnUpdateLibraryExportCollection)
	ON_COMMAND(ID_LIBRARY_EXPORT_COLLECTION, OnLibraryExportCollection)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_CREATETORRENT, OnUpdateLibraryCreateTorrent)
	ON_COMMAND(ID_LIBRARY_CREATETORRENT, OnLibraryCreateTorrent)
END_MESSAGE_MAP()

//#define ITEM_HEIGHT 17	// Settings.Skin.RowSize

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView construction

CLibraryTreeView::CLibraryTreeView()
	: m_pRoot		( new CLibraryTreeItem() )
	, m_nTotal		( 0 )
	, m_nVisible	( 0 )
	, m_nScroll 	( 0 )
	, m_nSelected	( 0 )
	, m_nCleanCookie( 0 )
	, m_pSelFirst	( NULL )
	, m_pSelLast	( NULL )
	, m_pFocus		( NULL )
	, m_pFocusObject()
	, m_pDropItem	( NULL )
	, m_bVirtual	( -1 )
	, m_bDrag		( FALSE )
{
	m_pRoot->m_bExpanded = TRUE;
}

CLibraryTreeView::~CLibraryTreeView()
{
	delete m_pRoot;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView root update

void CLibraryTreeView::SetVirtual(BOOL bVirtual)
{
	if ( bVirtual == m_bVirtual ) return;

	m_bVirtual = bVirtual;

	m_wndAlbumTip.Hide();
	m_wndFolderTip.Hide();
	m_wndAlbumTip.SetOwner( this );
	m_wndFolderTip.SetOwner( this );

	Clear();
}

BOOL CLibraryTreeView::Update(DWORD nSelectCookie)
{
	CQuickLock pLock( Library.m_pSection );

	BOOL bChanged = m_bVirtual ?
		UpdateVirtual( nSelectCookie ) : UpdatePhysical( nSelectCookie );

	if ( bChanged && m_pFocusObject[ m_bVirtual ] != NULL )
	{
		if ( ! SelectFolder( m_pFocusObject[ m_bVirtual ] ) )
			m_pFocusObject[ m_bVirtual ] = NULL;
	}

	return bChanged;
}

void CLibraryTreeView::PostUpdate()
{
	GetOwner()->PostMessage( WM_COMMAND, ID_LIBRARY_REFRESH );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView operations

BOOL CLibraryTreeView::Create(CWnd* pParentWnd)
{
	CRect rect;
	return CWnd::CreateEx( 0, NULL, L"CLibraryTreeView", WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL,
		rect, pParentWnd, IDC_LIBRARY_TREE, NULL );
}

void CLibraryTreeView::Clear()
{
	CQuickLock( Library.m_pSection );

	if ( m_pRoot->empty() ) return;

	m_pRoot->clear();

	m_nTotal		= 0;
	m_nSelected		= 0;
	m_pSelFirst		= NULL;
	m_pSelLast		= NULL;
	m_pFocus		= NULL;
	m_pDropItem		= NULL;

	m_wndAlbumTip.Hide();
	m_wndFolderTip.Hide();

	// NotifySelection(); NOT NOTIFIED
	UpdateScroll();
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView expand

BOOL CLibraryTreeView::Expand(CLibraryTreeItem* pItem, TRISTATE bExpand, BOOL bInvalidate)
{
	if ( pItem == NULL ) return FALSE;

	switch ( bExpand )
	{
	case TRI_UNKNOWN:
		pItem->m_bExpanded = ! pItem->m_bExpanded;
		break;
	case TRI_TRUE:
		if ( pItem->m_bExpanded ) return FALSE;
		pItem->m_bExpanded = TRUE;
		break;
	case TRI_FALSE:
		if ( ! pItem->m_bExpanded ) return FALSE;
		pItem->m_bExpanded = FALSE;
		break;
	}

	if ( pItem->m_pPhysical )
		pItem->m_pPhysical->m_bExpanded = pItem->m_bExpanded;
	else
		pItem->m_pVirtual->m_bExpanded = pItem->m_bExpanded;

	if ( ! pItem->IsVisible() ) return FALSE;

	if ( pItem->m_bExpanded )
	{
		m_nTotal += pItem->treeSize();
	}
	else
	{
		m_nTotal -= pItem->treeSize();
		DeselectAll( NULL, pItem, FALSE );
	}

	pItem->m_bContract1 = pItem->m_bExpanded == TRUE && bExpand == TRI_TRUE && bInvalidate == FALSE;

	if ( pItem->m_bContract1 == FALSE )
	{
		for ( CLibraryTreeItem* pParent = pItem; pParent != NULL; pParent = pParent->parent() )
			pParent->m_bContract1 = FALSE;
	}

	if ( bInvalidate )
	{
		UpdateScroll();
		Invalidate();
	}

	return TRUE;
}

BOOL CLibraryTreeView::CollapseRecursive(CLibraryTreeItem* pItem)
{
	BOOL bChanged = FALSE;

	if ( pItem != m_pRoot && pItem->m_bExpanded && pItem->m_bContract1 )
		bChanged |= Expand( pItem, TRI_FALSE, FALSE );

	for ( CLibraryTreeItem::iterator pChild = pItem->begin(); pChild != pItem->end(); ++pChild )
	{
		bChanged |= CollapseRecursive( &**pChild );
	}

	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView selection

BOOL CLibraryTreeView::Select(CLibraryTreeItem* pItem, TRISTATE bSelect, BOOL bInvalidate)
{
	if ( pItem == NULL ) return FALSE;

	switch ( bSelect )
	{
	case TRI_UNKNOWN:
		pItem->m_bSelected = ! pItem->m_bSelected;
		break;
	case TRI_TRUE:
		if ( pItem->m_bSelected ) return FALSE;
		pItem->m_bSelected = TRUE;
		break;
	case TRI_FALSE:
		if ( ! pItem->m_bSelected ) return FALSE;
		pItem->m_bSelected = FALSE;
		break;
	}

	if ( pItem->m_bSelected )
	{
		for ( CLibraryTreeItem* pRootItem = pItem->parent(); pRootItem; pRootItem = pRootItem->parent() )
		{
			Expand( pRootItem, TRI_TRUE, FALSE );
		}

		if ( m_bVirtual )
			m_pFocusObject[ 1 ] = pItem->m_pVirtual;
		else
			m_pFocusObject[ 0 ] = pItem->m_pPhysical;

		m_nSelected++;

		if ( m_pSelLast )
		{
			m_pSelLast->m_pSelNext = pItem;
			pItem->m_pSelPrev = m_pSelLast;
			pItem->m_pSelNext = NULL;
			m_pSelLast = pItem;
		}
		else
		{
			m_pSelFirst = m_pSelLast = pItem;
			pItem->m_pSelPrev = pItem->m_pSelNext = NULL;
		}
	}
	else
	{
		m_nSelected--;

		if ( pItem->m_pSelPrev )
			pItem->m_pSelPrev->m_pSelNext = pItem->m_pSelNext;
		else
			m_pSelFirst = pItem->m_pSelNext;

		if ( pItem->m_pSelNext )
			pItem->m_pSelNext = pItem->m_pSelNext->m_pSelPrev = pItem->m_pSelPrev;
		else
			m_pSelLast = pItem->m_pSelPrev;
	}

	if ( pItem->IsVisible() )
	{
		if ( bInvalidate )
			Invalidate();
		return TRUE;
	}

	return FALSE;
}

BOOL CLibraryTreeView::SelectAll(CLibraryTreeItem* pParent, BOOL bInvalidate)
{
	if ( pParent == NULL )
		pParent = m_pRoot;
	else if ( pParent->m_bExpanded == FALSE )
		return FALSE;

	BOOL bChanged = FALSE;

	for ( CLibraryTreeItem::iterator pChild = pParent->begin(); pChild != pParent->end(); ++pChild )
	{
		if ( (*pChild)->m_bSelected == FALSE )
		{
			Select( &**pChild, TRI_TRUE, FALSE );
			bChanged = TRUE;
		}

		if ( ! (*pChild)->empty() && (*pChild)->m_bExpanded )
			bChanged |= SelectAll( &**pChild, FALSE );
	}

	if ( bInvalidate && bChanged && pParent == m_pRoot )
		Invalidate();

	return bChanged;
}

BOOL CLibraryTreeView::DeselectAll(CLibraryTreeItem* pExcept, CLibraryTreeItem* pParent, BOOL bInvalidate)
{
	if ( pParent == NULL )
		pParent = m_pRoot;

	BOOL bChanged = FALSE;

	for ( CLibraryTreeItem::iterator pChild = pParent->begin(); pChild != pParent->end(); ++pChild )
	{
		if ( &**pChild != pExcept && (*pChild)->m_bSelected )
		{
			Select( &**pChild, TRI_FALSE, FALSE );
			bChanged = TRUE;
		}

		if ( ! (*pChild)->empty() )
			bChanged |= DeselectAll( pExcept, &**pChild, FALSE );
	}

	if ( bInvalidate && bChanged && pParent == m_pRoot )
		Invalidate();

	return bChanged;
}

int CLibraryTreeView::GetSelectedCount() const
{
	return m_nSelected;
}

CLibraryTreeItem* CLibraryTreeView::GetFirstSelected() const
{
	return m_pSelFirst;
}

CLibraryTreeItem* CLibraryTreeView::GetLastSelected() const
{
	return m_pSelLast;
}

BOOL CLibraryTreeView::Highlight(CLibraryTreeItem* pItem)
{
	m_pFocus = pItem;

	for ( CLibraryTreeItem* pParent = m_pFocus->parent(); pParent; pParent = pParent->parent() )
	{
		Expand( pParent, TRI_TRUE, FALSE );

		pParent->m_bContract2 = pParent->m_bContract1;
		pParent->m_bContract1 = FALSE;
	}

	CollapseRecursive( m_pRoot );

	for ( CLibraryTreeItem* pParent = m_pFocus->parent(); pParent; pParent = pParent->parent() )
	{
		pParent->m_bContract1 = pParent->m_bContract2;
	}

	CRect rcItem, rcClient;

	if ( GetRect( m_pFocus, &rcItem ) )
	{
		GetClientRect( &rcClient );

		if ( rcItem.top <= rcClient.top )
			ScrollBy( rcItem.top - rcClient.top );
		else if ( rcItem.bottom > rcClient.bottom )
			ScrollBy( rcItem.bottom - rcClient.bottom );
	}

	UpdateScroll();
	Invalidate();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView internal helpers

BOOL CLibraryTreeView::CleanItems(CLibraryTreeItem* pItem, DWORD nCookie, BOOL bVisible)
{
	BOOL bChanged = FALSE;

	for ( CLibraryTreeItem::iterator pChild = pItem->begin(); pChild != pItem->end(); )
	{
		if ( (*pChild)->m_nCleanCookie != nCookie )
		{
			if ( m_pFocus == &**pChild )
				m_pFocus = NULL;

			if ( (*pChild)->m_bSelected )
				Select( &**pChild, TRI_FALSE, FALSE );
			bChanged |= DeselectAll( NULL, &**pChild, FALSE );

			if ( bVisible )
			{
				m_nTotal -= (*pChild)->treeSize();
				bChanged = TRUE;
			}

			pChild = pItem->erase( pChild );
		}
		else
		{
			++pChild;
		}
	}

	return bChanged;
}

void CLibraryTreeView::NotifySelection()
{
	if ( ! m_hWnd ) return;
	NMHDR pNM = { GetSafeHwnd(), (UINT_PTR)GetDlgCtrlID(), LTN_SELCHANGED };
	GetOwner()->SendMessage( WM_NOTIFY, pNM.idFrom, (LPARAM)&pNM );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView search

CLibraryTreeItem* CLibraryTreeView::GetFolderItem(LPCVOID pSearch, CLibraryTreeItem* pParent)
{
	if ( pParent == NULL ) pParent = m_pRoot;

	for ( CLibraryTreeItem::iterator pChild = pParent->begin(); pChild != pParent->end(); ++pChild )
	{
		if ( pSearch == (*pChild)->m_pPhysical || pSearch == (*pChild)->m_pVirtual )
			return &**pChild;

		if ( ! (*pChild)->empty() )
		{
			CLibraryTreeItem* pFound = GetFolderItem( pSearch, &**pChild );
			if ( pFound ) return pFound;
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView message handlers

void CLibraryTreeView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	m_nVisible = cy;

	UpdateScroll();
}

void CLibraryTreeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rc;
	CLibraryTreeItem* pHit = HitTest( point, &rc );
	BOOL bChanged = FALSE;

	SetFocus();

	m_wndAlbumTip.Hide();
	m_wndFolderTip.Hide();

	if ( pHit && ! pHit->empty() && point.x >= rc.left && point.x < rc.left + 16 )
	{
		bChanged = Expand( pHit, TRI_UNKNOWN );
	}
	else if ( nFlags & MK_CONTROL )
	{
		if ( pHit ) bChanged = Select( pHit, TRI_UNKNOWN );
	}
	else if ( nFlags & MK_SHIFT )
	{
		if ( pHit ) bChanged = Select( pHit );
	}
	else
	{
		if ( ( nFlags & MK_RBUTTON ) == 0 || ( pHit && pHit->m_bSelected == FALSE ) )
			bChanged = DeselectAll( pHit );

		if ( bChanged )
			m_pFocusObject[ m_bVirtual ] = NULL;

		if ( pHit ) bChanged |= Select( pHit );
	}

	m_pFocus = pHit;

	if ( pHit && ( nFlags & MK_RBUTTON ) == 0 )
	{
		m_bDrag = TRUE;
		m_ptDrag = point;
	}

	if ( bChanged ) NotifySelection();

	CWnd::OnLButtonDown( nFlags, point );
}

void CLibraryTreeView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );

	if ( ! m_bVirtual )
		OnLibraryExplore();
	else if ( m_pFocus != NULL && ! m_pFocus->empty() && Expand( m_pFocus, TRI_UNKNOWN ) )
		NotifySelection();
}

void CLibraryTreeView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDrag = FALSE;

	CWnd::OnLButtonUp( nFlags, point );
}

void CLibraryTreeView::OnRButtonDown(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );

	CWnd::OnRButtonDown( nFlags, point );
}

void CLibraryTreeView::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		OnLibraryParent();
}

void CLibraryTreeView::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( m_bDrag && ( nFlags & MK_LBUTTON ) )
	{
		CSize szDiff = point - m_ptDrag;

		if ( abs( szDiff.cx ) > 5 || abs( szDiff.cy ) > 5 )
		{
			m_bDrag = FALSE;
			StartDragging( point );
		}
	}
	else
		m_bDrag = FALSE;

	if ( ! m_bDrag )
	{
		if ( CLibraryTreeItem* pItem = HitTest( point ) )
		{
			if ( pItem->m_pPhysical )
				m_wndFolderTip.Show( pItem->m_pPhysical );
			else
				m_wndAlbumTip.Show( pItem->m_pVirtual );
		}
		else
		{
			m_wndAlbumTip.Hide();
			m_wndFolderTip.Hide();
		}
	}

	CWnd::OnMouseMove( nFlags, point );

	// Tick Hoverstate
	if ( point.x < 16 )
	{
		CRect rcRefresh( 1, point.y - 48, 18, point.y + 48 );
		RedrawWindow(rcRefresh);
	}
	else if ( point.x < 32 )
	{
		CRect rcRefresh( 4, point.y - 42, 30, point.y + 42 );
		RedrawWindow(rcRefresh);
	}
}

void CLibraryTreeView::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	CLibraryTreeItem* pTo = NULL;
	BOOL bChanged = FALSE;
	CRect rc;

	m_wndAlbumTip.Hide();
	m_wndFolderTip.Hide();

	if ( nChar == VK_HOME || ( nChar == VK_UP && m_pFocus == NULL ) )
	{
		if ( ! m_pRoot->empty() )
			pTo = &**m_pRoot->begin();
	}
	else if ( nChar == VK_END || ( nChar == VK_DOWN && m_pFocus == NULL ) )
	{
		if ( ! m_pRoot->empty() )
			pTo = &**m_pRoot->rbegin();
	}
	else if ( nChar == VK_UP && m_pFocus != NULL )
	{
		if ( GetRect( m_pFocus, &rc ) )
		{
			CPoint pt( rc.left, ( rc.top + rc.bottom ) / 2 );
			pt.y -= Settings.Skin.RowSize;
			pTo = HitTest( pt );
		}
	}
	else if ( nChar == VK_DOWN && m_pFocus != NULL )
	{
		if ( GetRect( m_pFocus, &rc ) )
		{
			CPoint pt( rc.left, ( rc.top + rc.bottom ) / 2 );
			pt.y += Settings.Skin.RowSize;
			pTo = HitTest( pt );
		}
	}
	else if ( ( nChar == VK_LEFT || nChar == VK_SUBTRACT ) && m_pFocus != NULL )
	{
		for ( ;; )
		{
			if ( m_pFocus->m_bExpanded && ! m_pFocus->empty() )
			{
				Expand( m_pFocus, TRI_FALSE );
				break;
			}

			if ( m_pFocus->parent() == m_pRoot ) break;
			m_pFocus = m_pFocus->parent();

			bChanged |= DeselectAll( m_pFocus );
			bChanged |= Select( m_pFocus );
		}

		Highlight( m_pFocus );
	}
	else if ( ( nChar == VK_RIGHT || nChar == VK_ADD ) && m_pFocus != NULL )
	{
		if ( ! m_pFocus->m_bExpanded && ! m_pFocus->empty() )
			bChanged |= Expand( m_pFocus, TRI_TRUE );
	}
	else if ( _istalnum( TCHAR( nChar ) ) )
	{
		CLibraryTreeItem* pStart = m_pFocus;
		CLibraryTreeItem* pBase  = pStart ? pStart->parent() : m_pRoot;

		for ( int nLoop = 0; nLoop < 2; nLoop++ )
		{
			for ( CLibraryTreeItem::iterator pChild = pBase->begin(); pChild != pBase->end(); ++pChild )
			{
				if ( pStart != NULL )
				{
					if ( pStart == &**pChild ) pStart = NULL;
				}
				else if ( toupper( (*pChild)->m_sText.GetAt( 0 ) ) == (int)nChar )
				{
					DeselectAll( m_pFocus = &**pChild, NULL, FALSE );
					Select( m_pFocus, TRI_TRUE, FALSE );
					Highlight( m_pFocus );
					NotifySelection();
					return;
				}
			}
		}
	}

	if ( pTo != NULL )
	{
		if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 || m_pFocus == NULL )
		{
			bChanged |= DeselectAll( m_pFocus = pTo );
			bChanged |= Select( m_pFocus );
		}
		else
		{
			bChanged |= Select( m_pFocus = pTo );
		}

		Highlight( m_pFocus );
	}

	if ( bChanged ) NotifySelection();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView scrolling

void CLibraryTreeView::UpdateScroll()
{
	SCROLLINFO pInfo = {};
	pInfo.cbSize	= sizeof( pInfo );
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= (int)m_nTotal * Settings.Skin.RowSize;
	pInfo.nPage		= m_nVisible;
	pInfo.nPos		= m_nScroll = max( 0, min( m_nScroll, pInfo.nMax - (int)pInfo.nPage + 1 ) );

	SetScrollInfo( SB_VERT, &pInfo, TRUE );
}

void CLibraryTreeView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	switch ( nSBCode )
	{
	case SB_BOTTOM:
		ScrollTo( 0xFFFFFFFF );
		break;
	case SB_LINEDOWN:
		ScrollBy( 16 );
		break;
	case SB_LINEUP:
		ScrollBy( -16 );
		break;
	case SB_PAGEDOWN:
		ScrollBy( m_nVisible );
		break;
	case SB_PAGEUP:
		ScrollBy( -m_nVisible );
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		ScrollTo( nPos );
		break;
	case SB_TOP:
		ScrollTo( 0 );
		break;
	}
}

BOOL CLibraryTreeView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// Scroll window under cursor
	if ( CWnd* pWnd = WindowFromPoint( pt ) )
	{
		if ( pWnd != this )
		{
			const LPCTSTR szScrollable[] = {
				L"CPanelCtrl",
				L"CLibraryTreeView",
				L"CLibraryAlbumView",
				L"CLibraryThumbView",
				L"CLibraryTileView",
				L"CLibraryDetailView",
				NULL
			};
			for ( int i = 0; szScrollable[ i ]; ++i )
			{
				if ( pWnd == FindWindowEx( GetParent()->GetSafeHwnd(), NULL, NULL, szScrollable[ i ] ) )
					return pWnd->PostMessage( WM_MOUSEWHEEL, MAKEWPARAM( nFlags, zDelta ), MAKELPARAM( pt.x, pt.y ) );
			}
		}
	}

	ScrollBy( zDelta * 3 * -(int)Settings.Skin.RowSize / WHEEL_DELTA );
	return TRUE;
}

void CLibraryTreeView::ScrollBy(int nDelta)
{
	ScrollTo( max( 0, m_nScroll + nDelta ) );
}

void CLibraryTreeView::ScrollTo(int nPosition)
{
	if ( nPosition == m_nScroll ) return;
	m_nScroll = nPosition;

	UpdateScroll();

	CRect rc;
	GetClientRect( &rc );
	RedrawWindow( &rc, NULL, RDW_INVALIDATE );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView painting

BOOL CLibraryTreeView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CLibraryTreeView::OnPaint()
{
	CPaintDC dc( this );

	CRect rcClient;
	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );

	for ( CLibraryTreeItem::iterator pChild = m_pRoot->begin(); pChild != m_pRoot->end() && pt.y < rcClient.bottom; ++pChild )
	{
		Paint( dc, rcClient, pt, &**pChild );
	}

	dc.SelectObject( pOldFont );

	dc.FillSolidRect( &rcClient, Colors.m_crWindow );
}

void CLibraryTreeView::Paint(CDC& dc, CRect& rcClient, CPoint& pt, CLibraryTreeItem* pItem)
{
	CRect rc( pt.x, pt.y, pt.x, pt.y + Settings.Skin.RowSize );
	pt.y += Settings.Skin.RowSize;

	if ( rc.top >= rcClient.bottom )
		return;

	if ( rc.bottom >= rcClient.top )
	{
		if ( pItem->m_bBold )
			dc.SelectObject( &CoolInterface.m_fntBold );

		rc.right += 32 + dc.GetTextExtent( pItem->m_sText ).cx + 6;

		if ( dc.RectVisible( &rc ) )
		{
			pItem->Paint( dc, rc, m_pDropItem == pItem );
			dc.ExcludeClipRect( &rc );
		}

		if ( pItem->m_bBold )
			dc.SelectObject( &CoolInterface.m_fntNormal );
	}

	if ( pItem->m_bExpanded && ! pItem->empty() )
	{
		pt.x += 16;

		for ( CLibraryTreeItem::iterator pChild = pItem->begin(); pChild != pItem->end(); ++pChild )
		{
			Paint( dc, rcClient, pt, &**pChild );
			if ( pt.y >= rcClient.bottom ) break;
		}

		pt.x -= 16;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView hit testing

CLibraryTreeItem* CLibraryTreeView::HitTest(const POINT& point, RECT* pRect) const
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	for ( CLibraryTreeItem::iterator pChild = m_pRoot->begin(); pChild != m_pRoot->end() && pt.y < rcClient.bottom; ++pChild )
	{
		CLibraryTreeItem* pItem = HitTest( rcClient, pt, &**pChild, point, pRect );
		if ( pItem ) return pItem;
	}

	return NULL;
}

CLibraryTreeItem* CLibraryTreeView::HitTest(CRect& rcClient, CPoint& pt, CLibraryTreeItem* pItem, const POINT& point, RECT* pRect) const
{
	CRect rc( rcClient.left, pt.y, rcClient.right, pt.y + (long)Settings.Skin.RowSize );
	pt.y += (long)Settings.Skin.RowSize;

	if ( rc.top >= ( rcClient.bottom + (long)Settings.Skin.RowSize ) )
		return NULL;

	if ( rc.bottom >= ( rcClient.top - (long)Settings.Skin.RowSize ) )
	{
		if ( rc.PtInRect( point ) )
		{
			if ( pRect )
			{
				CopyMemory( pRect, &rc, sizeof( RECT ) );
				pRect->left = pt.x;
			}
			return pItem;
		}
	}

	if ( pItem->m_bExpanded && ! pItem->empty() )
	{
		pt.x += 16;

		for ( CLibraryTreeItem::iterator pChild = pItem->begin(); pChild != pItem->end(); ++pChild )
		{
			if ( CLibraryTreeItem* pHitItem = HitTest( rcClient, pt, &**pChild, point, pRect ) )
				return pHitItem;
			if ( pt.y >= ( rcClient.bottom + (long)Settings.Skin.RowSize ) )
				break;
		}

		pt.x -= 16;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView rect lookup

BOOL CLibraryTreeView::GetRect(CLibraryTreeItem* pItem, RECT* pRect)
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	for ( CLibraryTreeItem::iterator pChild = m_pRoot->begin(); pChild != m_pRoot->end(); ++pChild )
	{
		if ( GetRect( pt, &**pChild, pItem, pRect ) ) return TRUE;
	}

	return FALSE;
}

BOOL CLibraryTreeView::GetRect(CPoint& pt, CLibraryTreeItem* pItem, CLibraryTreeItem* pFind, RECT* pRect)
{
	if ( pItem == pFind )
	{
		pRect->left		= pt.x;
		pRect->top		= pt.y;
		pRect->right	= pt.x;
		pRect->bottom	= pt.y = ( pRect->top + (long)Settings.Skin.RowSize );

		CClientDC dc( this );
		CFont* pOld = (CFont*)dc.SelectObject( pItem->m_bBold ?
			&CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );
		pRect->right += 33 + dc.GetTextExtent( pItem->m_sText ).cx + 4;
		dc.SelectObject( pOld );

		return TRUE;
	}

	pt.y += (long)Settings.Skin.RowSize;

	if ( pItem->m_bExpanded && ! pItem->empty() )
	{
		pt.x += 16;

		for ( CLibraryTreeItem::iterator pChild = pItem->begin(); pChild != pItem->end(); ++pChild )
		{
			if ( GetRect( pt, &**pChild, pFind, pRect ) ) return TRUE;
		}

		pt.x -= 16;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView drag setup

void CLibraryTreeView::StartDragging(CPoint& ptMouse)
{
	if ( ! m_pSelFirst ) return;

	CQuickLock oLock( Library.m_pSection );

	CPoint ptMiddle( 0, 0 );
	HBITMAP pImage = CreateDragImage( ptMouse, ptMiddle );
	if ( ! pImage ) return;

	// Get GUID of parent folder
	Hashes::Guid oGUID;
	if ( m_pSelFirst->parent() && m_pSelFirst->parent()->m_pVirtual )
		oGUID = m_pSelFirst->parent()->m_pVirtual->m_oGUID;

	CEnvyDataSource::DoDragDrop ( m_pSelFirst, pImage, oGUID, ptMiddle );
}

HBITMAP CLibraryTreeView::CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle)
{
	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );

	GetClientRect( &rcClient );

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		GetRect( pItem, &rcOne );

		if ( rcOne.IntersectRect( &rcClient, &rcOne ) )
		{
			rcAll.left		= min( rcAll.left, rcOne.left );
			rcAll.top		= min( rcAll.top, rcOne.top );
			rcAll.right		= max( rcAll.right, rcOne.right );
			rcAll.bottom	= max( rcAll.bottom, rcOne.bottom );
		}
	}

	const BOOL bClipped = rcAll.Height() > MAX_DRAG_SIZE;

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

	ptMiddle.SetPoint( ptMouse.x - rcAll.left, ptMouse.y - rcAll.top );

	if ( bClipped )
	{
		CRgn pRgn;
		pRgn.CreateEllipticRgn(	ptMiddle.x - MAX_DRAG_SIZE_2, ptMiddle.y - MAX_DRAG_SIZE_2,
								ptMiddle.x + MAX_DRAG_SIZE_2, ptMiddle.y + MAX_DRAG_SIZE_2 );
		dcDrag.SelectClipRgn( &pRgn );
	}

	CFont* pOldFont = (CFont*)dcDrag.SelectObject( &CoolInterface.m_fntNormal );

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		GetRect( pItem, &rcOne );
		CRect rcDummy;
		if ( rcDummy.IntersectRect( &rcAll, &rcOne ) )
		{
			rcOne.OffsetRect( -rcAll.left, -rcAll.top );
			pItem->Paint( dcDrag, rcOne, FALSE, DRAG_COLOR_KEY );
		}
	}

	dcDrag.SelectObject( pOldFont );
	dcDrag.SelectObject( pOldDrag );
	dcDrag.DeleteDC();

	return (HBITMAP) bmDrag.Detach();
}


/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem construction

CLibraryTreeItem::CLibraryTreeItem(CLibraryTreeItem* pParent, const CString& name)
	: m_pParent	( pParent )
	, m_oList	( )
	, m_sText	( name )
{
	m_pSelPrev		= NULL;
	m_pSelNext		= NULL;
	m_nCleanCookie	= 0;

	m_bExpanded		= FALSE;
	m_bSelected		= FALSE;
	m_bContract1	= FALSE;
	m_bContract2	= FALSE;

	m_pPhysical		= NULL;
	m_pVirtual		= NULL;
	m_nCookie		= 0;
	m_bBold			= FALSE;
	m_bShared		= TRUE;
	m_bCollection	= FALSE;
	m_nIcon16		= -1;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem add

struct CLibraryTreeItemCompare
{
	// ToDo: _tcsicoll isn't really suitable here since it can fail
	bool operator()(const CLibraryTreeItem& lhs, const CLibraryTreeItem& rhs) const
	{
		return _tcsicoll( lhs.m_sText, rhs.m_sText ) < 0;
	}
	bool operator()(const CString& lhs, const CLibraryTreeItem& rhs) const
	{
		return _tcsicoll( lhs, rhs.m_sText ) < 0;
	}
	bool operator()(const CLibraryTreeItem& lhs, const CString& rhs) const
	{
		return _tcsicoll( lhs.m_sText, rhs ) < 0;
	}
};

CLibraryTreeItem* CLibraryTreeItem::addItem(const CString& name)
{
	return &**m_oList.insert( std::upper_bound( m_oList.begin(), m_oList.end(), name, CLibraryTreeItemCompare() ), new CLibraryTreeItem( this, name ) );		// No Boost::ptr_list
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem delete

//void CLibraryTreeItem::Delete()
//{
//	m_pParent->Delete( this );
//}

//void CLibraryTreeItem::Delete(CLibraryTreeItem* pItem)
//{
//	ASSERT( pItem->m_bSelected == FALSE );
//	CLibraryTreeItem** pChild = m_pList;
//	for ( int nChild = m_nCount; nChild; nChild--, pChild++ )
//	{
//		if ( **pChild == pItem )
//		{
//			MoveMemory( pChild, pChild + 1, ( nChild - 1 ) * sizeof *pChild );
//			m_nCount--;
//			break;
//		}
//	}
//	delete pItem;
//}

//void CLibraryTreeItem::Delete(int nItem)
//{
//	if ( nItem < 0 || nItem >= m_nCount ) return;
//
//	ASSERT( m_pList[ nItem ]->m_bSelected == FALSE );
//	delete m_pList[ nItem ];
//	MoveMemory( m_pList + nItem, m_pList + nItem + 1, ( m_nCount - nItem - 1 ) * sizeof *m_pList );
//	m_nCount--;
//}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem visibility

BOOL CLibraryTreeItem::IsVisible() const
{
	for ( const CLibraryTreeItem* pRoot = parent(); pRoot; pRoot = pRoot->parent() )
	{
		if ( ! pRoot->m_bExpanded ) return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem paint

void CLibraryTreeItem::Paint(CDC& dc, CRect& rc, BOOL bTarget, COLORREF crBack) const
{
	POINT ptHover;
	RECT  rcTick = { rc.left+2, rc.top+2, rc.left+14, rc.bottom-2 };
	GetCursorPos( &ptHover );
	if ( dc.GetWindow() != NULL )
		dc.GetWindow()->ScreenToClient( &ptHover );

	if ( crBack == CLR_NONE ) crBack = Colors.m_crWindow;
	dc.FillSolidRect( rc.left, rc.top, 33, Settings.Skin.RowSize, crBack );

	if ( ! empty() )
	{
		if ( m_bExpanded )
		{
			CoolInterface.Draw( &dc, PtInRect( &rcTick, ptHover ) ?
				IDI_CLOSETICK_HOVER : IDI_CLOSETICK, 16, rc.left, rc.top, crBack );
		}
		else
		{
			CoolInterface.Draw( &dc, PtInRect( &rcTick, ptHover ) ?
				IDI_OPENTICK_HOVER : IDI_OPENTICK, 16, rc.left, rc.top, crBack );
		}
	}

	if ( m_nIcon16 >= 0 )
		ShellIcons.Draw( &dc, m_nIcon16, 16, rc.left + 16, rc.top,
			crBack, ( m_bSelected || bTarget ) );
	else
		CoolInterface.Draw( &dc,
			( ( m_bExpanded && ! empty() ) ? IDI_FOLDER_OPEN : IDI_FOLDER_CLOSED ), 16,
			rc.left + 16, rc.top, crBack, ( m_bSelected || bTarget ) );
	if ( ! m_bShared )
		CoolInterface.Draw( &dc, IDI_LOCKED, 16,
			rc.left + 16, rc.top, CLR_NONE, ( m_bSelected || bTarget ), FALSE );
	if ( m_bCollection )
		CoolInterface.Draw( &dc, IDI_COLLECTION_MASK, 16,
			rc.left + 16, rc.top, CLR_NONE, ( m_bSelected || bTarget ), FALSE );

	crBack = ( m_bSelected || bTarget ) ? Colors.m_crHighlight : crBack;
	COLORREF crText = ( m_bSelected || bTarget ) ? Colors.m_crHiText : Colors.m_crText;

	const BOOL bSelectmark = ( m_bSelected || bTarget ) && ( Images.m_bmSelected.m_hObject != NULL );

	rc.left += 33;
	dc.SetTextColor( crText );
	dc.SetBkColor( bSelectmark ? CLR_NONE : crBack );
	dc.SetBkMode( bSelectmark ? TRANSPARENT : OPAQUE );

	if ( bSelectmark )
		CoolInterface.DrawWatermark( &dc, &rc, &Images.m_bmSelected );

	CString strName = m_sText;
	if ( Settings.General.LanguageRTL ) strName = L"\x202A" + strName;
	dc.ExtTextOut( rc.left + 2, rc.top + 1,
		ETO_CLIPPED|( bSelectmark ? 0 : ETO_OPAQUE ),
		&rc, strName, NULL );
	rc.left -= 33;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem get child files

int CLibraryTreeItem::GetFileList(CLibraryList* pList, BOOL bRecursive) const
{
	if ( LibraryFolders.CheckFolder( m_pPhysical, TRUE ) )
		return m_pPhysical->GetFileList( pList, bRecursive );
	if ( LibraryFolders.CheckAlbum( m_pVirtual ) )
		return m_pVirtual->GetFileList( pList, bRecursive );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView update physical

BOOL CLibraryTreeView::UpdatePhysical(DWORD nSelectCookie)
{
	DWORD nCleanCookie = m_nCleanCookie++;
	BOOL bChanged = FALSE;

	for ( POSITION pos = LibraryFolders.GetFolderIterator(); pos; )
	{
		CLibraryFolder* pFolder = LibraryFolders.GetNextFolder( pos );

		CLibraryTreeItem::iterator pChild = m_pRoot->begin();

		for ( ; pChild != m_pRoot->end(); ++pChild )
		{
			CLibraryFolder* pOld = (*pChild)->m_pPhysical;

			if ( pOld == pFolder )
			{
				bChanged |= Update( pFolder, &**pChild, m_pRoot, TRUE, TRUE,
					nCleanCookie, nSelectCookie, FALSE );
				break;
			}
		}

		if ( pChild == m_pRoot->end() )
		{
			bChanged |= Update( pFolder, NULL, m_pRoot, TRUE, TRUE,
				nCleanCookie, nSelectCookie, FALSE );
		}
	}

	bChanged |= CleanItems( m_pRoot, nCleanCookie, TRUE );

	if ( bChanged )
	{
		UpdateScroll();
		Invalidate();
		NotifySelection();
	}

	return bChanged;
}

BOOL CLibraryTreeView::Update(CLibraryFolder* pFolder, CLibraryTreeItem* pItem, CLibraryTreeItem* pParent, BOOL bVisible, BOOL bShared, DWORD nCleanCookie, DWORD nSelectCookie, BOOL bRecurse)
{
	BOOL bChanged = FALSE;

	pFolder->GetShared( bShared );

	if ( pItem == NULL )
	{
		CString strName = pFolder->m_sName;
		if ( pFolder->m_pParent == NULL && Settings.General.GUIMode != GUI_BASIC )
		{
			if ( pFolder->m_sPath[1] == L':' )
				strName += L"  (" + pFolder->m_sPath.Left( 1 ) + L":)";
			else if ( pFolder->m_sPath[1] == L'\\' )
				strName += L"  (" + pFolder->m_sPath.Mid( 2, pFolder->m_sPath.Find( L'\\', 3 ) - 2 ) + L")";
		}
		pItem = pParent->addItem( strName );
		if ( bVisible ) m_nTotal++;

		pItem->m_bExpanded	= pFolder->m_bExpanded;
		pItem->m_pPhysical	= pFolder;
		pItem->m_bShared	= bShared;
		pItem->m_bBold		= ( pFolder->m_sPath.CompareNoCase( Settings.Downloads.CompletePath ) == 0 );

		bChanged = bVisible;
	}
	else
	{
		if ( pFolder->m_pParent == NULL )
		{
			BOOL bBold = ( pFolder->m_sPath.CompareNoCase( Settings.Downloads.CompletePath ) == 0 );

			if ( bBold != pItem->m_bBold )
			{
				pItem->m_bBold = bBold;
				bChanged |= bVisible;
			}
		}

		if ( pItem->m_bShared != bShared )
		{
			pItem->m_bShared = bShared;
			bChanged |= bVisible;
		}
	}

	pItem->m_nCleanCookie = nCleanCookie;

	bVisible = bVisible && pItem->m_bExpanded;

	if ( nSelectCookie )
	{
		if ( bRecurse || pItem->m_bSelected )
		{
			for ( POSITION pos = pFolder->GetFileIterator(); pos; )
			{
				CLibraryFile* pFile = pFolder->GetNextFile( pos );
				pFile->m_nSelectCookie = nSelectCookie;
			}

			pFolder->m_nSelectCookie = nSelectCookie;
			bRecurse |= ( ! pItem->m_bExpanded );
		}
	}

	nCleanCookie = m_nCleanCookie++;

	for ( POSITION pos = pFolder->GetFolderIterator(); pos; )
	{
		CLibraryFolder* pSub = pFolder->GetNextFolder( pos );

		CLibraryTreeItem::iterator pChild = pItem->begin();

		for ( ; pChild != pItem->end(); ++pChild )
		{
			CLibraryFolder* pOld = (*pChild)->m_pPhysical;

			if ( pOld == pSub )
			{
				bChanged |= Update( pSub, &**pChild, pItem, bVisible, bShared,
					nCleanCookie, nSelectCookie, bRecurse );
				break;
			}
		}

		if ( pChild == pItem->end() )
		{
			bChanged |= Update( pSub, NULL, pItem, bVisible, bShared,
				nCleanCookie, nSelectCookie, bRecurse );
		}
	}

	bChanged |= CleanItems( pItem, nCleanCookie, bVisible );

	pItem->m_nCookie = pFolder->m_nUpdateCookie;

	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView update virtual

BOOL CLibraryTreeView::UpdateVirtual(DWORD nSelectCookie)
{
	if ( ! Update( Library.GetAlbumRoot(), m_pRoot, NULL, TRUE, 0, nSelectCookie ) )
		return FALSE;

	UpdateScroll();
	if ( m_hWnd )
		Invalidate();
	NotifySelection();

	return TRUE;
}

BOOL CLibraryTreeView::Update(CAlbumFolder* pFolder, CLibraryTreeItem* pItem, CLibraryTreeItem* pParent, BOOL bVisible, DWORD nCleanCookie, DWORD nSelectCookie)
{
	BOOL bChanged = FALSE;

	if ( ! pFolder )
		return bChanged;

	if ( pItem != NULL && pParent != NULL && pItem->m_sText != pFolder->m_sName )
		pItem = NULL;	// CleanCookie is not updated so it will be dropped later

	if ( pItem == NULL )
	{
		pItem = pParent->addItem( pFolder->m_sName );
		if ( bVisible ) m_nTotal++;

		pItem->m_bExpanded	= pFolder->m_bExpanded;
		pItem->m_pVirtual	= pFolder;
		pItem->m_nIcon16	= pFolder->m_pSchema ? pFolder->m_pSchema->m_nIcon16 : -1;
		pItem->m_bBold		= pItem->m_bCollection = bool( pFolder->m_oCollSHA1 );

		bChanged = bVisible;
	}
	else
	{
		if ( pFolder->m_pSchema != NULL && pItem->m_nIcon16 != pFolder->m_pSchema->m_nIcon16 )
		{
			pItem->m_nIcon16 = pFolder->m_pSchema->m_nIcon16;
			bChanged = bVisible;
		}

		if ( pItem->m_bCollection != static_cast< BOOL >( bool( pFolder->m_oCollSHA1 ) ) )
		{
			pItem->m_bBold = pItem->m_bCollection = bool( pFolder->m_oCollSHA1 );
			bChanged = bVisible;
		}
	}

	pItem->m_nCleanCookie = nCleanCookie;

	bVisible = bVisible && pItem->m_bExpanded;

	if ( nSelectCookie && pItem->m_bSelected )
	{
		for ( POSITION pos = pFolder->GetFileIterator(); pos; )
		{
			CLibraryFile* pFile = pFolder->GetNextFile( pos );
			pFile->m_nSelectCookie = nSelectCookie;
		}

		pFolder->m_nSelectCookie = nSelectCookie;
	}

	nCleanCookie = m_nCleanCookie++;

	for ( POSITION pos = pFolder->GetFolderIterator(); pos; )
	{
		CAlbumFolder* pSub = pFolder->GetNextFolder( pos );

		CLibraryTreeItem::iterator pChild = pItem->begin();

		for ( ; pChild != pItem->end(); ++pChild )
		{
			CAlbumFolder* pOld = (*pChild)->m_pVirtual;

			if ( pOld == pSub )
			{
				bChanged |= Update( pSub, &**pChild, pItem, bVisible,
					nCleanCookie, nSelectCookie );
				break;
			}
		}

		if ( pChild == pItem->end() )
		{
			bChanged |= Update( pSub, NULL, pItem, bVisible,
				nCleanCookie, nSelectCookie );
		}
	}

	bChanged |= CleanItems( pItem, nCleanCookie, bVisible );

	pItem->m_nCookie = pFolder->m_nUpdateCookie;

	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView folder selection

BOOL CLibraryTreeView::SelectFolder(LPCVOID pSearch)
{
	CLibraryTreeItem* pItem = GetFolderItem( pSearch );
	if ( pItem == NULL ) return FALSE;

	if ( m_nSelected == 1 && pItem->m_bSelected )
	{
		Highlight( pItem );
		return TRUE;
	}

	DeselectAll( pItem, NULL, FALSE );
	if ( Select( pItem ) )
	{
		Highlight( pItem );
		NotifySelection();
		RedrawWindow();
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView message handlers

int CLibraryTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndFolderTip.Create( this );
	m_wndAlbumTip.Create( this );

	ENABLE_DROP()

	return 0;
}

void CLibraryTreeView::OnDestroy()
{
	DISABLE_DROP()

	CWnd::OnDestroy();
}

BOOL CLibraryTreeView::PreTranslateMessage(MSG* pMsg)
{
	if ( m_bVirtual )
	{
		if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE )
		{
			OnLibraryFolderDelete();
			return TRUE;
		}
		if ( pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_RETURN )
		{
			OnLibraryFolderProperties();
			return TRUE;
		}
	}
	else
	{
		if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
		{
			OnLibraryExplore();
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage( pMsg );
}

void CLibraryTreeView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	if ( m_bVirtual )
	{
		Skin.TrackPopupMenu( L"CLibraryTree.Virtual", point, ID_LIBRARY_FOLDER_PROPERTIES );
	}
	else
	{
		CStringList oFiles;

		CSingleLock pLock( &Library.m_pSection );
		if ( ! SafeLock( pLock ) ) return;

		for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
		{
			if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
				oFiles.AddTail( pItem->m_pPhysical->m_sPath );
		}

		pLock.Unlock();

		Skin.TrackPopupMenu( L"CLibraryTree.Physical", point, ID_LIBRARY_EXPLORE, oFiles );
	}
}

void CLibraryTreeView::OnUpdateLibraryParent(CCmdUI* pCmdUI)
{
	CLibraryFrame* pFrame = (CLibraryFrame*)GetParent();
	ASSERT_KINDOF(CLibraryFrame, pFrame);

	CCoolBarCtrl* pBar = pFrame->GetViewTop();
	CCoolBarItem* pItem = pBar->GetID( ID_LIBRARY_PARENT );

	const BOOL bAvailable = ( m_nSelected == 1 );

	if ( pItem == pCmdUI ) pItem->Show( bAvailable );
	pCmdUI->Enable( bAvailable );
}

void CLibraryTreeView::OnLibraryParent()
{
	CLibraryTreeItem* pNew = NULL;

	if ( m_nSelected == 1 && m_pSelFirst->parent() != m_pRoot )
		pNew = m_pSelFirst->parent();

	DeselectAll( pNew );

	if ( pNew != NULL )
	{
		Select( pNew );
		Highlight( pNew );
	}
	else
	{
		m_pFocusObject[ m_bVirtual ] = NULL;
	}

	Invalidate();
	NotifySelection();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView physical command handlers

void CLibraryTreeView::OnUpdateLibraryExplore(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bVirtual && m_nSelected == 1 );
}

void CLibraryTreeView::OnLibraryExplore()
{
	if ( m_bVirtual || m_nSelected != 1 || m_pSelFirst == NULL ) return;

	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	if ( ! LibraryFolders.CheckFolder( m_pSelFirst->m_pPhysical, TRUE ) ) return;
	CString strPath = m_pSelFirst->m_pPhysical->m_sPath;
	pLock.Unlock();

//	CFileExecutor::Execute( strPath );
	ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL, strPath, NULL, NULL, SW_SHOWNORMAL );
}

void CLibraryTreeView::OnUpdateLibraryScan(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bVirtual && m_nSelected > 0 );
}

void CLibraryTreeView::OnLibraryScan()
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
			pItem->m_pPhysical->Scan();
	}
}

void CLibraryTreeView::OnUpdateLibraryShared(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	TRISTATE bShared = TRI_UNKNOWN;

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
		{
			if ( bShared == TRI_UNKNOWN )
			{
				bShared = pItem->m_pPhysical->IsShared() ? TRI_TRUE : TRI_FALSE;
			}
			else if ( ( bShared == TRI_TRUE ) != pItem->m_pPhysical->IsShared() )
			{
				pCmdUI->Enable( FALSE );
				return;
			}
		}
	}

	pCmdUI->Enable( m_nSelected > 0 );
	pCmdUI->SetCheck( bShared == TRI_TRUE );
}

void CLibraryTreeView::OnLibraryShared()
{
	{
		CQuickLock oLock( Library.m_pSection );

		for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
		{
			if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
			{
				BOOL bShared = pItem->m_pPhysical->IsShared();
				pItem->m_pPhysical->SetShared( TRI_UNKNOWN );

				if ( bShared )
					pItem->m_pPhysical->SetShared( pItem->m_pPhysical->IsShared() ? TRI_FALSE : TRI_UNKNOWN );
				else
					pItem->m_pPhysical->SetShared( pItem->m_pPhysical->IsShared() ? TRI_UNKNOWN : TRI_TRUE );
				pItem->m_pPhysical->m_nUpdateCookie++;
			}
		}

		Library.Update();
	}
	PostUpdate();
}

void CLibraryTreeView::OnUpdateLibraryRemove(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
		{
			if ( pItem->m_pPhysical->m_pParent == NULL )
			{
				pCmdUI->Enable( TRUE );
				return;
			}
		}
	}

	pCmdUI->Enable( FALSE );
}

void CLibraryTreeView::OnLibraryRemove()
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
		{
			if ( pItem->m_pPhysical->m_pParent == NULL )
				LibraryFolders.RemoveFolder( pItem->m_pPhysical );
		}
	}

	Library.Save();
	PostUpdate();
}

void CLibraryTreeView::OnLibraryAdd()
{
	CString strPath( BrowseForFolder( L"Select folder to share:" ) );
	if ( strPath.IsEmpty() )
		return;

	CFolderScanDlg dlgScan;

	LibraryFolders.AddFolder( strPath );

	dlgScan.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView virtual command handlers

void CLibraryTreeView::OnUpdateLibraryFolderEnqueue(CCmdUI* pCmdUI)
{
	CSingleLock oLock( &Library.m_pSection );
	if ( ! oLock.Lock( 200 ) ) return;

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckAlbum( pItem->m_pVirtual ) &&
			 pItem->m_pVirtual->GetFileCount() > 0 &&
			 ! CheckURI( pItem->m_pVirtual->m_sSchemaURI, CSchema::uriGhostFolder ) )
		{
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	pCmdUI->Enable( FALSE );
}

void CLibraryTreeView::OnLibraryFolderEnqueue()
{
	CStringList pList;		// CList< CString >

	{
		CSingleLock oLock( &Library.m_pSection );
		if ( ! oLock.Lock( 200 ) ) return;

		for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
		{
			if ( LibraryFolders.CheckAlbum( pItem->m_pVirtual ) &&
				 pItem->m_pVirtual->GetFileCount() > 0 &&
				 ! CheckURI( pItem->m_pVirtual->m_sSchemaURI, CSchema::uriGhostFolder ) )
			{
				for ( POSITION pos = pItem->m_pVirtual->GetFileIterator(); pos; )
				{
					CLibraryFile* pFile = pItem->m_pVirtual->GetNextFile( pos );
					pList.AddTail( pFile->GetPath() );
				}
			}
		}
	}

	CFileExecutor::Enqueue( pList );
}

void CLibraryTreeView::OnUpdateLibraryFolderMetadata(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_nSelected > 0 && m_pSelFirst->m_pVirtual != NULL );
}

void CLibraryTreeView::OnLibraryFolderMetadata()
{
	CQuickLock oLock( Library.m_pSection );

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		CAlbumFolder* pFolder = pItem->m_pVirtual;
		if ( LibraryFolders.CheckAlbum( pFolder ) ) pFolder->MetaToFiles( TRUE );
	}

	Library.Update();
}

void CLibraryTreeView::OnUpdateLibraryFolderDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_nSelected > 0 && m_pSelFirst->m_pVirtual != NULL );
}

void CLibraryTreeView::OnLibraryFolderDelete()
{
	if ( m_pSelFirst == NULL || m_pSelFirst->m_pVirtual == NULL ) return;

	CString strFormat, strMessage;
	Skin.LoadString( strFormat, IDS_LIBRARY_DELETE_FOLDER );
	strMessage.Format( strFormat, m_nSelected );

	if ( MsgBox( strMessage, MB_ICONQUESTION|MB_OKCANCEL ) != IDOK ) return;

	{
		CQuickLock oLock( Library.m_pSection );

		for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
		{
			CAlbumFolder* pFolder = pItem->m_pVirtual;
			if ( LibraryFolders.CheckAlbum( pFolder ) )
				pFolder->Delete();
		}

		DeselectAll();
	}

	NotifySelection();
}

void CLibraryTreeView::OnUpdateLibraryFolderNew(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_nSelected == 0 || ( m_nSelected == 1 && m_pSelFirst->m_pVirtual != NULL ) );
}

void CLibraryTreeView::OnLibraryFolderNew()
{
	if ( m_pSelFirst != NULL && m_pSelFirst->m_pVirtual == NULL ) return;

	CAlbumFolder* pFolder;
	{
		CQuickLock oLock( Library.m_pSection );

		pFolder = Library.GetAlbumRoot();
		if ( ! pFolder )
			pFolder = LibraryFolders.CreateAlbumTree();

		if ( m_pSelFirst )
			pFolder = m_pSelFirst->m_pVirtual;

		pFolder = pFolder->AddFolder( NULL, LoadString( IDS_NEW_FOLDER ) );

		if ( m_pSelFirst )
			Expand( m_pSelFirst, TRI_TRUE, FALSE );

		NotifySelection();

		if ( CLibraryTreeItem* pItem = GetFolderItem( pFolder ) )
		{
			Select( pItem, TRI_TRUE, FALSE );
			DeselectAll( pItem, NULL, FALSE );
		}
	}

	Invalidate();

	if ( pFolder )
		PostMessage( WM_COMMAND, ID_LIBRARY_FOLDER_PROPERTIES );
}

void CLibraryTreeView::OnUpdateLibraryRebuild(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_nSelected > 0 );
}

void CLibraryTreeView::OnLibraryRebuild()
{
	CSingleLock oLock( &Library.m_pSection );
	if ( ! oLock.Lock( 80 ) ) return;

	CLibraryListPtr pList( new CLibraryList() );

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		pItem->GetFileList( pList, TRUE );
	}

	for ( POSITION pos = pList->GetHeadPosition(); pos; )
	{
		if ( CLibraryFile* pFile = pList->GetNextFile( pos ) )
			pFile->Rebuild();
	}

	Library.Update( true );
}

void CLibraryTreeView::OnUpdateLibraryFolderProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_nSelected == 1 && m_pSelFirst->m_pVirtual != NULL );
}

void CLibraryTreeView::OnLibraryFolderProperties()
{
	if ( m_pSelFirst == NULL || m_pSelFirst->m_pVirtual == NULL ) return;

	CAlbumFolder* pFolder = m_pSelFirst->m_pVirtual;

	CFolderPropertiesDlg dlg( NULL, pFolder );

	if ( dlg.DoModal() == IDOK )
	{
		NotifySelection();

		if ( CLibraryTreeItem* pItem = GetFolderItem( pFolder ) )
		{
			Select( pItem, TRI_TRUE, FALSE );
			DeselectAll( pItem, NULL, FALSE );
			Invalidate();
			NotifySelection();
		}
	}
}

void CLibraryTreeView::OnUpdateLibraryFolderFileProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_nSelected > 0 );
}

void CLibraryTreeView::OnLibraryFolderFileProperties()
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CLibraryListPtr pList( new CLibraryList() );

	for ( CLibraryTreeItem* pItem = m_pSelFirst; pItem; pItem = pItem->m_pSelNext )
	{
		pItem->GetFileList( pList, TRUE );
	}

	pLock.Unlock();

	CFilePropertiesSheet dlg;
	dlg.Add( pList );
	dlg.DoModal();
}

void CLibraryTreeView::OnUpdateLibraryExportCollection(CCmdUI *pCmdUI)
{
	if ( m_nSelected != 1 )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	CSingleLock oLock( &Library.m_pSection );
	if ( ! oLock.Lock( 200 ) ) return;

	BOOL bAllowExport = TRUE;

	// Allow max 1000 files to be parsed and do not export from Ghost or Collection folder
	if ( ! m_pSelFirst || ! m_pSelFirst->m_pVirtual ||
		 m_pSelFirst->m_pVirtual->GetFileCount() == 0 ||
		 m_pSelFirst->m_pVirtual->GetFileCount() > 1000 ||
		 CheckURI( m_pSelFirst->m_pVirtual->m_sSchemaURI, CSchema::uriGhostFolder ) ||
		 m_pSelFirst->m_pVirtual->m_oCollSHA1 )
		bAllowExport = FALSE;

	pCmdUI->Enable( bAllowExport );
}

void CLibraryTreeView::OnLibraryExportCollection()
{
	if ( m_pSelFirst == NULL || m_pSelFirst->m_pSelNext != NULL ) return;
	if ( m_pSelFirst->m_pVirtual == NULL ) return;

	CCollectionExportDlg dlg( m_pSelFirst->m_pVirtual );
	dlg.DoModal();
}

void CLibraryTreeView::OnUpdateLibraryCreateTorrent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bVirtual && ! Settings.BitTorrent.TorrentCreatorPath.IsEmpty() && GetSelectedCount() < 2 );		// ToDo: Allow m_bVirtual
}

void CLibraryTreeView::OnLibraryCreateTorrent()
{
	CString strCommandLine;

	CSingleLock pLock( &Library.m_pSection );
	if ( SafeLock( pLock ) )
	{
		if ( CLibraryTreeItem* pItem = GetFirstSelected() )
		{
			CString strPath;
			if ( m_bVirtual )
			{
				// ToDo:
			}
			else
			{
				strPath = pItem->m_pPhysical->m_sPath;
			}
			pLock.Unlock();

			if ( ! strPath.IsEmpty() )
			{
				CString strCommandLine = L" -sourcefile \"" + strPath + L"\"";
				if ( Settings.BitTorrent.DefaultTracker.GetLength() > 10 && Settings.Downloads.TorrentPath.GetLength() > 4 )
					strCommandLine += L" -destination \"" + Settings.Downloads.TorrentPath + L"\" -tracker \"" + Settings.BitTorrent.DefaultTracker + L"\"";
			}
		}
		else
			pLock.Unlock();
	}

	ShellExecute( GetSafeHwnd(), L"open",
		Settings.BitTorrent.TorrentCreatorPath, strCommandLine,
		Settings.Downloads.TorrentPath,
		SW_SHOWNORMAL );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView drag drop

IMPLEMENT_DROP(CLibraryTreeView, CWnd)

BOOL CLibraryTreeView::OnDrop(IDataObject* pDataObj, DWORD grfKeyState, POINT ptScreen, DWORD* pdwEffect, BOOL bDrop)
{
	if ( ! pDataObj )
	{
		RedrawWindow();
		return TRUE;
	}

	CPoint pt( ptScreen );
	ScreenToClient( &pt );

	CRect rcClient, rcItem;
	GetClientRect( &rcClient );

	CLibraryTreeItem* pHit = HitTest( pt, &rcItem );
	if ( pHit && ! rcItem.PtInRect( pt ) )
		pHit = NULL;

	if ( pHit )
	{
		DeselectAll( pHit );
		if ( Select( pHit ) )
		{
			NotifySelection();
			RedrawWindow();
		}
	}

	CQuickLock oLock( Library.m_pSection );

	if ( pHit )
	{
		if ( pHit->m_pPhysical )
			return CEnvyDataSource::DropToFolder( pDataObj, grfKeyState,
				pdwEffect, bDrop, pHit->m_pPhysical->m_sPath );

		if ( pHit->m_pVirtual )
			return CEnvyDataSource::DropToAlbum( pDataObj, grfKeyState,
				pdwEffect, bDrop, pHit->m_pVirtual );
	}
	else
	{
		if ( m_bVirtual )
			return CEnvyDataSource::DropToAlbum( pDataObj, grfKeyState,
				pdwEffect, bDrop, Library.GetAlbumRoot() );
	}

	return FALSE;
}

void CLibraryTreeView::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus( pOldWnd );

	if ( ! m_pFocus && ! m_pRoot->empty() )
	{
		m_pFocus = &**m_pRoot->begin();
		BOOL bChanged = Select( m_pFocus );
		Highlight( m_pFocus );
		if ( bChanged )
			NotifySelection();
	}
}

UINT CLibraryTreeView::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}
