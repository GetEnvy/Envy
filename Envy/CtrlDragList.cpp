//
// CtrlDragList.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
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
#include "Envy.h"
#include "CtrlDragList.h"
#include "LiveList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CDragListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CDragListCtrl, CListCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDragListCtrl construction

CDragListCtrl::CDragListCtrl()
	: m_pDragImage		 ( NULL )
	, m_bCreateDragImage ( FALSE )
{
}

CDragListCtrl::~CDragListCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDragListCtrl message handlers

void CDragListCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	CPoint ptAction( pNMListView->ptAction );

	m_bCreateDragImage = TRUE;
	m_pDragImage = CLiveList::CreateDragImage( this, ptAction );
	m_bCreateDragImage = FALSE;

	if ( m_pDragImage == NULL ) return;
	m_nDragDrop = -1;

	UpdateWindow();

	CRect rcClient;
	GetClientRect( &rcClient );
	ClientToScreen( &rcClient );
	ClipCursor( &rcClient );
	SetCapture();

	SetFocus();
	UpdateWindow();

	m_pDragImage->DragEnter( this, ptAction );
}

void CDragListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( m_pDragImage != NULL )
	{
		int nHit = HitTest( point );

		m_pDragImage->DragMove( point );

		if ( nHit != m_nDragDrop )
		{
			CImageList::DragShowNolock( FALSE );
			if ( m_nDragDrop >= 0 ) SetItemState( m_nDragDrop, 0, LVIS_DROPHILITED );
			m_nDragDrop = nHit;
			if ( m_nDragDrop >= 0 ) SetItemState( m_nDragDrop, LVIS_DROPHILITED, LVIS_DROPHILITED );
			UpdateWindow();
			CImageList::DragShowNolock( TRUE );
		}
	}

	CListCtrl::OnMouseMove( nFlags, point );
}

void CDragListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( m_pDragImage == NULL )
	{
		CListCtrl::OnLButtonUp( nFlags, point );
		return;
	}

	ClipCursor( NULL );
	ReleaseCapture();

	m_pDragImage->DragLeave( this );
	m_pDragImage->EndDrag();
	delete m_pDragImage;
	m_pDragImage = NULL;

	if ( m_nDragDrop >= 0 )
		SetItemState( m_nDragDrop, 0, LVIS_DROPHILITED );
	//else
	//	m_nDragDrop = GetItemCount();

	OnDragDrop( m_nDragDrop );
}

void CDragListCtrl::OnDragDrop(int nDrop)
{
	NM_LISTVIEW pNotify;
	pNotify.hdr.hwndFrom	= GetSafeHwnd();
	pNotify.hdr.idFrom		= GetDlgCtrlID();
	pNotify.hdr.code		= LVN_DRAGDROP;
	pNotify.iItem			= nDrop;
	GetOwner()->SendMessage( WM_NOTIFY, pNotify.hdr.idFrom, (LPARAM)&pNotify );
}
