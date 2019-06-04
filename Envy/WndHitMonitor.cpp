//
// WndHitMonitor.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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
#include "WndHitMonitor.h"
#include "WndSearch.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "MatchObjects.h"
#include "Network.h"
#include "Packet.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "Skin.h"

#include "DlgHitColumns.h"
#include "DlgNewSearch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_SERIAL(CHitMonitorWnd, CBaseMatchWnd, 0)

BEGIN_MESSAGE_MAP(CHitMonitorWnd, CBaseMatchWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_HITMONITOR_CLEAR, OnHitMonitorClear)
	ON_UPDATE_COMMAND_UI(ID_HITMONITOR_PAUSE, OnUpdateHitMonitorPause)
	ON_COMMAND(ID_HITMONITOR_PAUSE, OnHitMonitorPause)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHitMonitorWnd construction

CHitMonitorWnd::CHitMonitorWnd()
{
	Create( IDR_HITMONITORFRAME );
	m_bPaused = FALSE;
}

CHitMonitorWnd::~CHitMonitorWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHitMonitorWnd message handlers

int CHitMonitorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CBaseMatchWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_pMatches->m_sFilter = Settings.Search.MonitorFilter;
	m_pMatches->Filter();

	if ( CSchemaPtr pSchema = SchemaCache.Get( Settings.Search.MonitorSchemaURI ) )
	{
		CSchemaMemberList pColumns;
		CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
		m_wndList.SelectSchema( pSchema, &pColumns );
	}

	LoadState( L"CHitMonitorWnd", TRUE );

	return 0;
}

void CHitMonitorWnd::OnDestroy()
{
	Settings.Search.MonitorFilter = m_pMatches->m_sFilter;

	if ( m_wndList.m_pSchema )
		Settings.Search.MonitorSchemaURI = m_wndList.m_pSchema->GetURI();
	else
		Settings.Search.MonitorSchemaURI.Empty();

	SaveState( L"CHitMonitorWnd" );

	CBaseMatchWnd::OnDestroy();
}

void CHitMonitorWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	if ( m_bContextMenu )
		Skin.TrackPopupMenu( L"CHitMonitorWnd", point, ID_SEARCH_DOWNLOAD );
	else
		CBaseMatchWnd::OnContextMenu( pWnd, point );
}

void CHitMonitorWnd::OnUpdateHitMonitorPause(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bPaused );
}

void CHitMonitorWnd::OnHitMonitorPause()
{
	m_bPaused = ! m_bPaused;
}

void CHitMonitorWnd::OnHitMonitorClear()
{
	m_wndList.DestructiveUpdate();
	m_pMatches->Clear();
	m_bUpdate = TRUE;
	PostMessage( WM_TIMER, 2 );
}

/////////////////////////////////////////////////////////////////////////////
// CHitMonitorWnd event handlers

void CHitMonitorWnd::OnSkinChange()
{
	OnSize( 0, 0, 0 );
	CBaseMatchWnd::OnSkinChange();
	Skin.Translate( L"CMatchCtrl", &m_wndList.m_wndHeader );
	Skin.CreateToolBar( L"CHitMonitorWnd", &m_wndToolBar );
}

BOOL CHitMonitorWnd::OnQueryHits(const CQueryHit* pHits)
{
	if ( m_bPaused || m_hWnd == NULL ) return FALSE;

	CSingleLock pLock( &m_pMatches->m_pSection );

	if ( pLock.Lock( 100 ) && ! m_bPaused )
	{
		m_pMatches->AddHits( pHits );
		m_bUpdate = TRUE;
		return TRUE;
	}

	return FALSE;
}
