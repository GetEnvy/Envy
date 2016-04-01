//
// CtrlPanel.cpp (Library)
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
#include "CtrlPanel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CPanelCtrl, CWnd)

BEGIN_MESSAGE_MAP(CPanelCtrl, CWnd)
	ON_WM_MOUSEWHEEL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPanelCtrl construction

CPanelCtrl::CPanelCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPanelCtrl operations

BOOL CPanelCtrl::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::CreateEx( WS_EX_CONTROLPARENT, NULL, L"CPanelCtrl", WS_CHILD | WS_TABSTOP |
		WS_VSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, pParentWnd, 0, NULL );
}

BOOL CPanelCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// Scroll window under cursor
	if ( CWnd* pWnd = WindowFromPoint( pt ) )
	{
		if ( pWnd != this )
			return pWnd->SendMessage( WM_MOUSEWHEEL, MAKEWPARAM( nFlags, zDelta ), MAKELPARAM( pt.x, pt.y ) );
	}

	PostMessage( WM_VSCROLL, MAKEWPARAM( SB_THUMBPOSITION,
		GetScrollPos( SB_VERT ) - zDelta / WHEEL_DELTA * 3 * 8 ), NULL );	// theApp.m_nMouseWheel lines = 3, line height = 8

	return TRUE;
}

void CPanelCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	SCROLLINFO pScroll = {};
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_ALL;
	GetScrollInfo( SB_VERT, &pScroll );

	switch ( nSBCode )
	{
	case SB_TOP:
		pScroll.nPos = 0;
		break;
	case SB_BOTTOM:
		pScroll.nPos = pScroll.nMax - 1;
		break;
	case SB_LINEUP:
		pScroll.nPos -= 8;
		break;
	case SB_LINEDOWN:
		pScroll.nPos += 8;
		break;
	case SB_PAGEUP:
		pScroll.nPos -= pScroll.nPage;
		break;
	case SB_PAGEDOWN:
		pScroll.nPos += pScroll.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pScroll.nPos = nPos;
		break;
	}

	pScroll.fMask = SIF_POS;
	pScroll.nPos  = max( 0, min( pScroll.nPos, pScroll.nMax ) );

	SetScrollInfo( SB_VERT, &pScroll );

	Invalidate();
}

void CPanelCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	Update();
	UpdateWindow();
}
