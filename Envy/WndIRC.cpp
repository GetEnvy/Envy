//
// WndIRC.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2005-2007
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
// Original Author: peer_l_@hotmail.com
//

#include "StdAfx.h"
#include "Envy.h"
#include "WndIRC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_SERIAL(CIRCWnd, CPanelWnd, 1)

BEGIN_MESSAGE_MAP(CIRCWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_NCLBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIRCWnd construction

CIRCWnd::CIRCWnd() : CPanelWnd( TRUE )
{
	Create( IDR_IRCFRAME );
}

/////////////////////////////////////////////////////////////////////////////
// CIRCWnd operations

/////////////////////////////////////////////////////////////////////////////
// CIRCWnd message handlers

int CIRCWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndFrame.Create( this ) ) return -1;

	LoadState();

	OnSkinChange();

	return 0;
}

void CIRCWnd::OnDestroy()
{
	SaveState();
	CPanelWnd::OnDestroy();
}

void CIRCWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	m_wndFrame.OnSkinChange();
}

BOOL CIRCWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndFrame.m_hWnd != NULL )
	{
		if ( m_wndFrame.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}

	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

LRESULT CIRCWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if ( m_wndFrame.m_hWnd != NULL && m_wndFrame.GetParent() != this )
		m_wndFrame.OnUpdateCmdUI();

	CPanelWnd::OnIdleUpdateCmdUI();
	wParam = 0;	lParam = 0;
	return 0;
}

BOOL CIRCWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN )
	{
		if ( m_wndFrame.m_hWnd && m_wndFrame.PreTranslateMessage( pMsg ) ) return TRUE;
	}

	return CPanelWnd::PreTranslateMessage(pMsg);
}

void CIRCWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ( ( nID & 0xFFF0 ) == SC_CLOSE )
	{
		PostMessage( WM_CLOSE );
		return;
	}

	CPanelWnd::OnSysCommand( nID, lParam );
}

void CIRCWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );

	if ( m_wndFrame.m_hWnd != NULL && m_wndFrame.GetParent() == this )
		m_wndFrame.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CIRCWnd::OnPaint()
{
	CPaintDC dc( this );

	if ( m_wndFrame.m_hWnd == NULL || m_wndFrame.GetParent() != this )
	{
		CRect rc;
		GetClientRect( &rc );
		dc.FillSolidRect( &rc, RGB(40,90,130) );
	}
}

void CIRCWnd::OnClose()
{
	CPanelWnd::OnClose();
}
