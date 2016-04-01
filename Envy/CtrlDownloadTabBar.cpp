//
// CtrlDownloadTabBar.cpp
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
#include "CtrlDownloadTabBar.h"
#include "Download.h"
#include "Downloads.h"
#include "DownloadGroup.h"
#include "DownloadGroups.h"
#include "DlgDownloadGroup.h"

#include "CoolMenu.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Transfers.h"
#include "ShellIcons.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CDownloadTabBar, CControlBar)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_DOWNLOAD_GROUP_NEW, OnDownloadGroupNew)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_REMOVE, OnUpdateDownloadGroupRemove)
	ON_COMMAND(ID_DOWNLOAD_GROUP_REMOVE, OnDownloadGroupRemove)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_MOVELEFT, OnUpdateDownloadGroupMoveLeft)
	ON_COMMAND(ID_DOWNLOAD_GROUP_MOVELEFT, OnDownloadGroupMoveLeft)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_MOVERIGHT, OnUpdateDownloadGroupMoveRight)
	ON_COMMAND(ID_DOWNLOAD_GROUP_MOVERIGHT, OnDownloadGroupMoveRight)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_PROPERTIES, OnUpdateDownloadGroupProperties)
	ON_COMMAND(ID_DOWNLOAD_GROUP_PROPERTIES, OnDownloadGroupProperties)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_RESUME, OnUpdateDownloadGroupResume)
	ON_COMMAND(ID_DOWNLOAD_GROUP_RESUME, OnDownloadGroupResume)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_PAUSE, OnUpdateDownloadGroupPause)
	ON_COMMAND(ID_DOWNLOAD_GROUP_PAUSE, OnDownloadGroupPause)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_CLEAR, OnUpdateDownloadGroupClear)
	ON_COMMAND(ID_DOWNLOAD_GROUP_CLEAR, OnDownloadGroupClear)
	ON_COMMAND(ID_DOWNLOAD_GROUP_OPEN, OnDownloadGroupOpen)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar construction

CDownloadTabBar::CDownloadTabBar()
	: m_pHot		( NULL )
	, m_bTimer		( FALSE )
	, m_bMenuGray	( FALSE )
	, m_nCookie 	( 0 )
	, m_nMessage	( 0 )
	, m_nMaximumWidth ( 140 )
{
}

CDownloadTabBar::~CDownloadTabBar()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete m_pItems.GetNext( pos );
	}

	m_pItems.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar operations

BOOL CDownloadTabBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc( 0, 0, 0, 0 );
	return CWnd::CreateEx( 0, NULL, L"CDownloadTabBar",
		dwStyle | WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP, rc, pParentWnd, nID, NULL );
}

void CDownloadTabBar::OnSkinChange()
{
	// SetWatermark:
	if ( m_bmImage.m_hObject ) m_bmImage.DeleteObject();
	if ( HBITMAP hBitmap = Skin.GetWatermark( L"CDownloadTabBar" ) )
		m_bmImage.Attach( hBitmap );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar message handlers

int CDownloadTabBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 ) return -1;
	//if ( Settings.Skin.MenuBorders )
	m_dwStyle |= CBRS_BORDER_3D;
	return 0;
}

CSize CDownloadTabBar::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	CSize size( 32767, Settings.Skin.GroupsbarHeight );

	if ( CWnd* pParent = AfxGetMainWnd() )
	{
		CRect rc;
		pParent->GetWindowRect( &rc );
		if ( rc.Width() > 32 )
			size.cx = rc.Width() + 2;
	}

	return size;
}

void CDownloadTabBar::Update(int nCookie)
{
	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );

	if ( m_nCookie != DownloadGroups.GetGroupCookie() )
		UpdateGroups( nCookie );
	else
		UpdateStates( nCookie );
}

void CDownloadTabBar::UpdateGroups(int nCookie)
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete m_pItems.GetNext( pos );
	}

	m_pItems.RemoveAll();

	BOOL bFoundHot = ( m_pHot == NULL );

	for ( POSITION pos = DownloadGroups.GetIterator() ; pos ; )
	{
		CDownloadGroup* pGroup = DownloadGroups.GetNext( pos );
		m_pItems.AddTail( new TabItem( pGroup, nCookie ) );

		if ( ! bFoundHot && pGroup == m_pHot->m_pGroup )
			bFoundHot = TRUE;
	}

	if ( ! bFoundHot ) m_pHot = NULL;

	m_nCookie = DownloadGroups.GetGroupCookie();
	Invalidate();
}

void CDownloadTabBar::UpdateStates(int nCookie)
{
	BOOL bChanged = FALSE;

	for ( POSITION posNext = m_pItems.GetHeadPosition() ; posNext ; )
	{
		POSITION posThis = posNext;
		TabItem* pItem = m_pItems.GetNext( posNext );

		if ( DownloadGroups.Check( pItem->m_pGroup ) )
		{
			bChanged |= pItem->Update( nCookie );
		}
		else
		{
			m_pItems.RemoveAt( posThis );
			delete pItem;
		}
	}

	if ( bChanged ) Invalidate();
}

CDownloadTabBar::TabItem* CDownloadTabBar::HitTest(const CPoint& point, CRect* pItemRect) const
{
	if ( m_pItems.IsEmpty() ) return NULL;

	CRect rc;
	GetClientRect( &rc );
	CalcInsideRect( rc, FALSE );

	rc.left -= m_cyTopBorder;
	rc.top  -= m_cxLeftBorder;
	rc.right  += m_cyBottomBorder;
	rc.bottom += m_cxRightBorder;

	CRect rcItem( rc.left + 3, rc.top + 1, 0, rc.bottom - 1 );
	rcItem.right = static_cast< LONG >( ( rc.Width() - 3 * m_pItems.GetCount() ) / m_pItems.GetCount() + 3 );
	rcItem.right = min( rcItem.right, m_nMaximumWidth );

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );

		if ( rcItem.PtInRect( point ) )
		{
			if ( pItemRect ) *pItemRect = rcItem;
			return pItem;
		}

		rcItem.OffsetRect( rcItem.Width() + 3, 0 );
	}

	return NULL;
}

INT_PTR CDownloadTabBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rcItem;
	TabItem* pItem = HitTest( point, &rcItem );

	if ( pItem == NULL ) return -1;
	if ( pTI == NULL ) return 1;

	CString strTip;
	if ( pItem->m_sName == LoadString( IDS_GENERAL_ALL ) )
	{
		LoadString( strTip, IDS_DOWNLOAD_GROUP_DEFAULT );
	}
	else // All other groups
	{
		strTip.Format( LoadString( IDS_DOWNLOAD_GROUP ), (LPCTSTR)pItem->m_sName );

		// Special case tutorial (No translation needed?)
		if ( pItem->m_sName == L"Custom" )	// Assume Settings.General.LanguageDefault
			strTip += L", or right-click to customize this tab now.";
	}

	pTI->uFlags		= TTF_NOTBUTTON;
	pTI->hwnd		= GetSafeHwnd();
	pTI->uId		= NULL;
	pTI->rect		= rcItem;
	pTI->lpszText	= _tcsdup( strTip );

	return pTI->uId;
}

void CDownloadTabBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CDC* pOutDC = pDC;

	CRect rc;
	GetClientRect( &rc );

	if ( m_bmImage.m_hObject != NULL )
	{
		CSize size = rc.Size();
		pDC = CoolInterface.GetBuffer( *pDC, size );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmImage );
	}

	DrawBorders( pDC, rc );

	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( ! m_pItems.IsEmpty() )
	{
		CRect rcItem( rc.left + 3, rc.top + 1, 0, rc.bottom - 1 );
		rcItem.right = static_cast< LONG >( rc.Width() / m_pItems.GetCount() );
		rcItem.right = min( rcItem.right, m_nMaximumWidth );

		for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			TabItem* pItem = m_pItems.GetNext( pos );

			pItem->Paint( this, pDC, &rcItem, m_pHot == pItem, pDC != pOutDC );
			pDC->ExcludeClipRect( &rcItem );

			rcItem.OffsetRect( rcItem.Width() + 3, 0 );
		}

		if ( pDC == pOutDC )
			pDC->FillSolidRect( &rc, Colors.m_crMidtone );
	}
	else
	{
		CSize  sz = pDC->GetTextExtent( m_sMessage );
		CPoint pt = rc.CenterPoint();
		pt.x -= sz.cx / 2;
		pt.y -= sz.cy / 2 + 1;

		pDC->SetBkColor( Colors.m_crMidtone );
		pDC->SetTextColor( Colors.m_crDisabled );

		if ( pDC == pOutDC )
		{
			pDC->SetBkMode( OPAQUE );
			pDC->ExtTextOut( pt.x, pt.y, ETO_CLIPPED|ETO_OPAQUE, &rc, m_sMessage, NULL );
		}
		else
		{
			pDC->SetBkMode( TRANSPARENT );
			pDC->ExtTextOut( pt.x, pt.y, ETO_CLIPPED, &rc, m_sMessage, NULL );
		}
	}

	pDC->SelectObject( pOldFont );

	if ( pDC != pOutDC )
	{
		GetClientRect( &rc );
		pOutDC->BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );
		pDC->SelectClipRgn( NULL );
	}
}

void CDownloadTabBar::OnMouseMove(UINT nFlags, CPoint point)
{
	TabItem* pItem = HitTest( point );

	if ( pItem != m_pHot )
	{
		m_pHot = pItem;
		Invalidate();
	}

	if ( ! m_bTimer && m_pHot )
	{
		SetTimer( 1, 100, NULL );
		m_bTimer = TRUE;
	}
	else if ( m_bTimer && ! m_pHot )
	{
		KillTimer( 1 );
		m_bTimer = FALSE;
	}

	CControlBar::OnMouseMove( nFlags, point );
}

// Was BOOL CDownloadTabBar::DropShowTarget(CList< CDownload* >* /*pList*/, const CPoint& ptScreen)
void CDownloadTabBar::OnMouseMoveDrag(const CPoint& ptScreen)
{
	CPoint ptLocal( ptScreen );
	ScreenToClient( &ptLocal );

	TabItem* pItem = HitTest( ptLocal );

	if ( pItem != m_pHot )
	{
		m_pHot = pItem;
		CImageList::DragShowNolock( FALSE );
		RedrawWindow();
		CImageList::DragShowNolock( TRUE );
	}
}

void CDownloadTabBar::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		CRect rcWindow;
		CPoint point;

		GetClientRect( &rcWindow );
		ClientToScreen( &rcWindow );
		GetCursorPos( &point );

		if ( ! rcWindow.PtInRect( point ) )
		{
			KillTimer( nIDEvent );
			m_pHot   = NULL;
			m_bTimer = FALSE;
			Invalidate();
		}
	}

	CControlBar::OnTimer( nIDEvent );
}

void CDownloadTabBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ( TabItem* pHit = HitTest( point ) )
	{
		BOOL bShift		= ( nFlags & MK_SHIFT ) != 0;
		BOOL bControl	= ( nFlags & MK_CONTROL ) != 0;

		if ( ( bControl && pHit->Select( ! pHit->m_bSelected ) ) ||
			 ( bShift && pHit->Select( TRUE ) ) ||
			 Select( pHit ) )
			NotifySelection();

		return;
	}

	CControlBar::OnLButtonDown( nFlags, point );
}

void CDownloadTabBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ( TabItem* pHit = HitTest( point ) )
	{
		PostMessage( WM_COMMAND, ID_DOWNLOAD_GROUP_PROPERTIES );
		return;
	}

	OnLButtonDown( nFlags, point );
}

void CDownloadTabBar::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	CRect rcItem;

	if ( TabItem* pItem = HitTest( point, &rcItem ) )
	{
		m_bMenuGray = TRUE;
		if ( Select( pItem ) )
			NotifySelection();
		Invalidate();
		ClientToScreen( &rcItem );
		CoolMenu.RegisterEdge( Settings.General.LanguageRTL ? rcItem.right : rcItem.left, rcItem.bottom - 1, rcItem.Width() );
		Skin.TrackPopupMenu( L"CDownloadTabBar", CPoint( Settings.General.LanguageRTL ? rcItem.right : rcItem.left, rcItem.bottom - 1 ),
			ID_DOWNLOAD_GROUP_PROPERTIES );
		m_bMenuGray = FALSE;
		Invalidate();

		return;
	}

	ClientToScreen( &point );
	Skin.TrackPopupMenu( L"CDownloadTabBar", point );

//	CControlBar::OnRButtonUp( nFlags, point );
}

void CDownloadTabBar::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CoolMenu.OnMeasureItem( lpMeasureItemStruct );
}

void CDownloadTabBar::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CoolMenu.OnDrawItem( lpDrawItemStruct );
}

BOOL CDownloadTabBar::Select(TabItem* pHit)
{
	BOOL bChanged = FALSE;

	if ( ! pHit )
		pHit = m_pItems.GetHead();	// Default Supergroup

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );
		bChanged |= pItem->Select( pItem == pHit );
	}

	return bChanged;
}

int CDownloadTabBar::GetSelectedCount(BOOL bDownloads)
{
	int nCount = 0;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );

		if ( pItem->m_bSelected )
			nCount += bDownloads ? pItem->m_nCount : 1;
	}

	return nCount;
}

CDownloadTabBar::TabItem* CDownloadTabBar::GetSelectedItem()
{
	TabItem* pSelected = NULL;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );

		if ( pItem->m_bSelected )
		{
			if ( pSelected == NULL )
				pSelected = pItem;
			else
				return NULL;
		}
	}

	return pSelected;
}

CDownloadGroup* CDownloadTabBar::GetSelectedGroup()
{
	TabItem* pItem = GetSelectedItem();
	return pItem ? pItem->m_pGroup : NULL;
}

void CDownloadTabBar::GetSelectedDownloads(CList< CDownload* >* pDownloads)
{
	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = m_pItems.GetNext( pos );

		if ( pItem->m_bSelected && DownloadGroups.Check( pItem->m_pGroup ) )
			pItem->m_pGroup->CopyList( *pDownloads );
	}
}

void CDownloadTabBar::NotifySelection()
{
//	GetOwner()->PostMessage( WM_KEYDOWN, VK_ESCAPE );	// Deselect all
	Invalidate();
	GetOwner()->PostMessage( WM_TIMER, 2 );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar command handlers

void CDownloadTabBar::OnDownloadGroupNew()
{
	CDownloadGroup* pGroup = DownloadGroups.Add( (LPCTSTR)LoadString( IDS_DOWNLOAD_NEW_GROUP ) );
	NotifySelection();

	CDownloadGroupDlg dlg( pGroup );

	if ( dlg.DoModal() == IDOK )
	{
		CQuickLock oLock( Transfers.m_pSection );

		if ( DownloadGroups.Check( pGroup ) )
			pGroup->LinkAll();
	}
	else
	{
		DownloadGroups.Remove( pGroup );
	}

	NotifySelection();
}

void CDownloadTabBar::OnUpdateDownloadGroupRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 && GetSelectedGroup() != DownloadGroups.GetSuperGroup() );
}

void CDownloadTabBar::OnDownloadGroupRemove()
{
	DownloadGroups.Remove( GetSelectedGroup() );
	NotifySelection();
}

void CDownloadTabBar::OnUpdateDownloadGroupMoveLeft(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 && GetSelectedGroup() != DownloadGroups.GetSuperGroup() );
}

void CDownloadTabBar::OnDownloadGroupMoveLeft()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	DownloadGroups.MoveLeft( GetSelectedGroup() );
	NotifySelection();
}

void CDownloadTabBar::OnUpdateDownloadGroupMoveRight(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 && GetSelectedGroup() != DownloadGroups.GetSuperGroup() );
}

void CDownloadTabBar::OnDownloadGroupMoveRight()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	DownloadGroups.MoveRight( GetSelectedGroup() );
	NotifySelection();
}

void CDownloadTabBar::OnUpdateDownloadGroupProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CDownloadTabBar::OnDownloadGroupProperties()
{
	CDownloadGroup* pGroup = GetSelectedGroup();
	CDownloadGroupDlg dlg( pGroup );
	if ( dlg.DoModal() == IDOK )
	{
		CQuickLock oLock( Transfers.m_pSection );

		if ( pGroup != DownloadGroups.GetSuperGroup() )
		{
			pGroup->Clear();
			pGroup->LinkAll();
		}
	}

	NotifySelection();
}

void CDownloadTabBar::OnUpdateDownloadGroupResume(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount( TRUE ) > 0 );
}

void CDownloadTabBar::OnDownloadGroupResume()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList< CDownload* > pDownloads;

	GetSelectedDownloads( &pDownloads );

	for ( POSITION pos = pDownloads.GetHeadPosition() ; pos ; )
	{
		CDownload* pDownload = (CDownload*)pDownloads.GetNext( pos );
		if ( Downloads.Check( pDownload ) )
			pDownload->Resume();
	}
}

void CDownloadTabBar::OnUpdateDownloadGroupPause(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount( TRUE ) > 0 );
}

void CDownloadTabBar::OnDownloadGroupPause()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList< CDownload* > pDownloads;

	GetSelectedDownloads( &pDownloads );

	for ( POSITION pos = pDownloads.GetHeadPosition() ; pos ; )
	{
		CDownload* pDownload = (CDownload*)pDownloads.GetNext( pos );
		if ( Downloads.Check( pDownload ) )
			pDownload->Pause();
	}
}

void CDownloadTabBar::OnUpdateDownloadGroupClear(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount( TRUE ) > 0 );
}

void CDownloadTabBar::OnDownloadGroupClear()
{
	if ( MsgBox( IDS_DOWNLOAD_CLEAR_GROUP, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList< CDownload* > pDownloads;

	GetSelectedDownloads( &pDownloads );

	for ( POSITION pos = pDownloads.GetHeadPosition() ; pos ; )
	{
		CDownload* pDownload = (CDownload*)pDownloads.GetNext( pos );
		if ( Downloads.Check( pDownload ) )
			pDownload->Remove();
	}
}

void CDownloadTabBar::OnDownloadGroupOpen()
{
	CDownloadGroup* pGroup = GetSelectedGroup();
	CString strPath = pGroup->m_sFolder;

	if ( strPath.IsEmpty() )
	{
		strPath = Settings.Downloads.CompletePath;
	}
	else if ( ! PathIsDirectory( strPath ) )
	{
		strPath = Settings.Downloads.CompletePath + L"\\" + strPath;
		if ( ! PathIsDirectory( strPath ) )
			strPath = Settings.Downloads.CompletePath;
	}

	ShellExecute( GetSafeHwnd(), L"open", strPath, NULL, NULL, SW_SHOWNORMAL );
}


/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar drag and drop

// BOOL CDownloadTabBar::DropShowTarget(CList< CDownload* >* /*pList*/, const CPoint& ptScreen) moved to OnMouseMoveDrag(pt)

BOOL CDownloadTabBar::DropObjects(CList< CDownload* >* pList, const CPoint& ptScreen)
{
	CPoint ptLocal( ptScreen );
	ScreenToClient( &ptLocal );

	if ( m_pHot != NULL )
	{
		m_pHot = NULL;
		Invalidate();
	}

	TabItem* pItem = HitTest( ptLocal );
	if ( pItem == NULL ) return FALSE;

	BOOL bMove = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0;

	CSingleLock pLock1( &Transfers.m_pSection, TRUE );
	CSingleLock pLock2( &DownloadGroups.m_pSection, TRUE );

	if ( DownloadGroups.Check( pItem->m_pGroup ) )
	{
		for ( POSITION posD = pList->GetHeadPosition() ; posD ; )
		{
			CDownload* pDownload = (CDownload*)pList->GetNext( posD );

			if ( Downloads.Check( pDownload ) )
			{
				if ( bMove )
					DownloadGroups.Unlink( pDownload, FALSE );

				pItem->m_pGroup->Add( pDownload );
			}
		}
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar::TabItem construction

CDownloadTabBar::TabItem::TabItem(CDownloadGroup* pGroup, int nCookie)
	: m_pGroup	( pGroup )
	, m_nImage	( 0 )
	, m_nCount	( 0 )
	, m_bSelected ( pGroup == DownloadGroups.GetSuperGroup() )
{
	Update( nCookie );
}

CDownloadTabBar::TabItem::~TabItem()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar::TabItem update

BOOL CDownloadTabBar::TabItem::Update(int nCookie)
{
	if ( m_bSelected && nCookie )
		m_pGroup->SetCookie( nCookie );

	BOOL bChanged = FALSE;

	const int nCount = static_cast< int >( m_pGroup->GetCount() );

	if ( m_sName != m_pGroup->m_sName )
	{
		m_sName = m_pGroup->m_sName;
		bChanged = TRUE;
	}

	if ( m_nCount != nCount )
	{
		m_nCount = nCount;
		bChanged = TRUE;
	}

	if ( m_nImage != m_pGroup->m_nImage )
	{
		m_nImage = m_pGroup->m_nImage;
		bChanged = TRUE;
	}

	if ( bChanged )
		m_sCaption.Format( L"%s (%i)", (LPCTSTR)m_sName, m_nCount );

	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar::TabItem select

BOOL CDownloadTabBar::TabItem::Select(BOOL bSelect)
{
	if ( bSelect == m_bSelected ) return FALSE;
	m_bSelected = bSelect;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTabBar::TabItem paint

void CDownloadTabBar::TabItem::Paint(CDownloadTabBar* pBar, CDC* pDC, CRect* pRect, BOOL bHot, BOOL bTransparent)
{
	CRect rc( pRect );
	COLORREF crBack = bTransparent ? CLR_NONE : Colors.m_crMidtone;

	BOOL bPopulated = m_nCount > 0;
	BOOL bSkinned = FALSE;

	// Background:

	rc.InflateRect( 1, 1 );
	rc.bottom++;

	if ( bHot && m_bSelected )
		bSkinned = Images.DrawButtonState( pDC, &rc, DOWNLOADGROUP_PRESS );		// Active Hover
	else if ( m_bSelected )
		bSkinned = Images.DrawButtonState( pDC, &rc, DOWNLOADGROUP_ACTIVE );		// Active Group
	else if ( bHot )
		bSkinned = Images.DrawButtonState( pDC, &rc, DOWNLOADGROUP_HOVER );		// Hover
	else if ( m_bSelected && pBar->m_bMenuGray )
		bSkinned = Images.DrawButtonState( pDC, &rc, DOWNLOADGROUP_DISABLED );	// Greyed (Empty)
	else
		bSkinned = Images.DrawButtonState( pDC, &rc, DOWNLOADGROUP_DEFAULT ); 	// Available (Default)

	rc.bottom--;
	rc.DeflateRect( 1, 1 );

	if ( bSkinned )
	{
		crBack = CLR_NONE;
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		if ( m_bSelected && pBar->m_bMenuGray )
		{
			crBack = Colors.m_crBackNormal;
			pDC->Draw3dRect( &rc, Colors.m_crDisabled, Colors.m_crDisabled );
		}
		else if ( bHot || m_bSelected )
		{
			crBack = ( bHot && m_bSelected ) ? Colors.m_crBackCheckSel : Colors.m_crBackSel;
			pDC->Draw3dRect( &rc, Colors.m_crBorder, Colors.m_crBorder );
		}
		else if ( crBack != CLR_NONE )
		{
			pDC->Draw3dRect( &rc, crBack, crBack );
		}

		if ( crBack != CLR_NONE )
			pDC->SetBkColor( crBack );
	}

	rc.DeflateRect( 1, 1 );

	// Icon:

	CPoint ptImage( rc.left + 2, rc.top + 1 );

	if ( bHot && ! m_bSelected )
	{
		ptImage.Offset( -1, -1 );

		if ( crBack != CLR_NONE )
		{
			pDC->FillSolidRect( ptImage.x, ptImage.y, 18, 2, crBack );
			pDC->FillSolidRect( ptImage.x, ptImage.y + 2, 2, 16, crBack );
		}

		ptImage.Offset( 2, 2 );
		pDC->SetTextColor( Colors.m_crShadow );
		ShellIcons.Draw( pDC, m_nImage, 16, ptImage.x, ptImage.y, crBack, CLR_NONE, ILD_MASK );

		ptImage.Offset( -2, -2 );
		ShellIcons.Draw( pDC, m_nImage, 16, ptImage.x, ptImage.y, CLR_NONE, CLR_NONE, ILD_NORMAL );

		pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 18, ptImage.y + 18 );

		ptImage.Offset( 1, 1 );
	}
	else
	{
		if ( m_bSelected )	// Default ImageList_DrawEx()
			ShellIcons.Draw( pDC, m_nImage, 16, ptImage.x, ptImage.y, crBack );
		else
			ShellIcons.Draw( pDC, m_nImage, 16, ptImage.x, ptImage.y, crBack, Colors.m_crShadow, ILD_BLEND50 );
		pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
	}

	// Text:

	rc.left += 20;

	CString strText = m_sCaption;
	if ( Settings.General.LanguageRTL ) strText = L"\x202A" + strText;

	if ( pDC->GetTextExtent( strText ).cx > rc.Width() )
	{
		while ( pDC->GetTextExtent( strText + L'\x2026' ).cx > rc.Width() && ! strText.IsEmpty() )
		{
			strText = strText.Left( strText.GetLength() - 1 );
		}

		if ( ! strText.IsEmpty() ) strText += L'\x2026';
	}

	rc.left -= 20;

	if ( crBack != CLR_NONE )
		pDC->SetBkColor( crBack );
	if ( m_bSelected || bHot )
		pDC->SetTextColor( Colors.m_crCmdTextSel );
	else if ( ! bPopulated )
		pDC->SetTextColor( Colors.m_crDisabled );
	else
		pDC->SetTextColor( Colors.m_crCmdText );

	if ( crBack != CLR_NONE )
	{
		pDC->SetBkMode( OPAQUE );
		pDC->ExtTextOut( rc.left + 20, rc.top + 2, ETO_CLIPPED|ETO_OPAQUE, &rc, strText, NULL );
	}
	else
	{
		pDC->SetBkMode( TRANSPARENT );
		pDC->ExtTextOut( rc.left + 20, rc.top + 2, ETO_CLIPPED, &rc, strText, NULL );
	}
}
