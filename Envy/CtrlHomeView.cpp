//
// CtrlHomeView.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2016
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
#include "CtrlHomeView.h"
#include "RichElement.h"
#include "Images.h"
#include "Skin.h"

#include "Network.h"
#include "Datagrams.h"
#include "Neighbour.h"
#include "Neighbours.h"
#include "VersionChecker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CHomeViewCtrl, CRichViewCtrl)

BEGIN_MESSAGE_MAP(CHomeViewCtrl, CRichViewCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()

#define GROUP_DISCONNECTED		1
#define GROUP_CONNECTED			2
#define GROUP_UPGRADE			3
#define GROUP_REMOTE			4
#define GROUP_FIREWALLED		5
#define GROUP_FIREWALLED_TCP	6
#define GROUP_FIREWALLED_UDP	7


/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl construction

CHomeViewCtrl::CHomeViewCtrl()
	: m_peHeader	( NULL )
	, m_peSearch	( NULL )
	, m_peUpgrade	( NULL )
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl system message handlers

BOOL CHomeViewCtrl::Create(const RECT& rect, CWnd* pParentWnd)
{
	return CRichViewCtrl::Create( WS_CHILD|WS_CLIPCHILDREN, rect, pParentWnd, IDC_HOME_VIEW );
}

int CHomeViewCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CRichViewCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndSearch.Create( this, IDC_HOME_SEARCH );

	OnSkinChange();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl operations

void CHomeViewCtrl::OnSkinChange()
{
	m_pDocument.Clear();
	m_peHeader = m_peSearch = m_peUpgrade = m_peRemote1 = m_peRemote2 = m_peRemoteBrowse = NULL;

	if ( ! Skin.GetWatermark( &m_bmHeader1, L"CHomeViewCtrl.Header1" ) )	// Legacy .sks first
		Skin.GetWatermark( &m_bmHeader1, L"CHomeViewCtrl.Header" );
	if ( ! Skin.GetWatermark( &m_bmHeader2, L"CHomeViewCtrl.Header2" ) )	// Legacy .sks first
		Skin.GetWatermark( &m_bmHeader2, L"CHomeViewCtrl.HeaderRepeat" );

	CXMLElement* pXML = Skin.GetDocument( L"CHomeViewCtrl" );
	CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;

	if ( pXML == NULL || ! m_pDocument.LoadXML( pXML, &pMap ) )
	{
		SetDocument( &m_pDocument );
		m_wndSearch.OnSkinChange( m_pDocument.m_crBackground, m_pDocument.m_crText );
		return;
	}

	Images.BlendAlpha( &m_bmHeader1, m_pDocument.m_crBackground );
	Images.BlendAlpha( &m_bmHeader2, m_pDocument.m_crBackground );

	pMap.Lookup( L"Header", m_peHeader );
	pMap.Lookup( L"SearchBox", m_peSearch );
	pMap.Lookup( L"Upgrade", m_peUpgrade );
	pMap.Lookup( L"RemoteBrowseURL", m_peRemoteBrowse );
	pMap.Lookup( L"RemoteAccessURL", m_peRemote1 );
	pMap.Lookup( L"RemoteAccessLocal", m_peRemote2 );

	m_wndSearch.OnSkinChange( m_pDocument.m_crBackground, m_pDocument.m_crText );

	SetDocument( &m_pDocument );
	Update();
}

void CHomeViewCtrl::Activate()
{
	if ( m_wndSearch.IsWindowVisible() )
		m_wndSearch.Activate();
}

void CHomeViewCtrl::Update()
{
	BOOL bConnected = Network.IsConnected();
	m_pDocument.ShowGroup( GROUP_DISCONNECTED, ! bConnected );
	m_pDocument.ShowGroup( GROUP_CONNECTED, bConnected );

	BOOL bOnG2 = bConnected && Settings.Gnutella2.Enabled && ( Neighbours.GetCount( PROTOCOL_G2, nrsConnected, -1 ) >= Settings.Gnutella2.NumHubs );
	// BOOL bTCPFirewalled = Network.IsFirewalled(CHECK_TCP);
	BOOL bUDPFirewalled = Network.IsFirewalled(CHECK_UDP);

	m_pDocument.ShowGroup( GROUP_FIREWALLED, bOnG2 && bUDPFirewalled && ! Network.m_bUPnPPortsForwarded );
	m_pDocument.ShowGroup( GROUP_FIREWALLED_TCP, FALSE );
	m_pDocument.ShowGroup( GROUP_FIREWALLED_UDP, FALSE );

	// ToDo: Temp Disabled (?)
	//m_pDocument.ShowGroup( GROUP_FIREWALLED, bOnG2 && bTCPFirewalled && bUDPFirewalled );
	//m_pDocument.ShowGroup( GROUP_FIREWALLED_TCP, bOnG2 && bTCPFirewalled && ! bUDPFirewalled );
	//m_pDocument.ShowGroup( GROUP_FIREWALLED_UDP, bOnG2 && bUDPFirewalled && ! bTCPFirewalled );

	if ( VersionChecker.IsUpgradeAvailable() )
	{
		if ( m_peUpgrade )
			m_peUpgrade->SetText( Settings.VersionCheck.UpgradePrompt );
		m_pDocument.ShowGroup( GROUP_UPGRADE, TRUE );
	}
	else
	{
		m_pDocument.ShowGroup( GROUP_UPGRADE, FALSE );
	}

	if ( m_peRemoteBrowse )
	{
		CString strURL;
		strURL.Format( L"gnutella:browse:%s:%i",
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
			(int)ntohs( Network.m_pHost.sin_port ) );

		if ( Settings.Community.ServeFiles )
			m_peRemoteBrowse->m_sLink = L"command:copy:" + strURL;
		m_peRemoteBrowse->SetText( Settings.General.LanguageRTL ? L"\x202A" + strURL : strURL );
	}

	if ( Settings.Remote.Enable &&
		 ! Settings.Remote.Username.IsEmpty() &&
		 ! Settings.Remote.Password.IsEmpty() &&
		 Network.IsListening() )
	{
		if ( m_peRemote1 )
		{
			CString strURL;
			strURL.Format( L"http://%s:%i/remote",
				(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
				(int)ntohs( Network.m_pHost.sin_port ) );
			m_peRemote1->SetText( Settings.General.LanguageRTL ? L"\x202A" + strURL : strURL );
			m_peRemote1->m_sLink = strURL + L"/?username=" + Settings.Remote.Username;
		}
		if ( m_peRemote2 )
		{
			CString strURL;
			strURL.Format( L"http://localhost:%i/remote",
				(int)ntohs( Network.m_pHost.sin_port ) );
			m_peRemote2->SetText( Settings.General.LanguageRTL ? L"\x202A" + strURL : strURL );
			m_peRemote2->m_sLink = strURL + L"/?username=" + Settings.Remote.Username;
		}

		m_pDocument.ShowGroup( GROUP_REMOTE, TRUE );
	}
	else
	{
		m_pDocument.ShowGroup( GROUP_REMOTE, FALSE );
	}

	InvalidateIfModified();
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl layout event

void CHomeViewCtrl::OnLayoutComplete()
{
	if ( m_peSearch != NULL && ( m_peSearch->m_nFlags & retfHidden ) == 0 )
	{
		CRect rcClient, rcAnchor( 0, 0, 0, 0 );

		GetClientRect( &rcClient );
		GetElementRect( m_peSearch, &rcAnchor );
		rcAnchor.OffsetRect( 0, -GetScrollPos( SB_VERT ) );

		rcAnchor.left = rcClient.left;
		rcAnchor.right = rcClient.right;

		rcAnchor.DeflateRect( m_pDocument.m_szMargin.cx + 39, 0 );

		BOOL bShowed = m_wndSearch.IsWindowVisible() == FALSE;

		m_wndSearch.SetWindowPos( NULL, rcAnchor.left, rcAnchor.top, rcAnchor.Width(), rcAnchor.Height(), SWP_SHOWWINDOW|SWP_NOACTIVATE );

		if ( bShowed && GetFocus() == this )
			m_wndSearch.SetFocus();
	}
	else
	{
		m_wndSearch.ShowWindow( SW_HIDE );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl paint event

void CHomeViewCtrl::OnPaintBegin(CDC* pDC)
{
	if ( m_peHeader != NULL && ( m_peHeader->m_nFlags & retfHidden ) == 0 && m_bmHeader1.m_hObject != NULL )
	{
		CRect rcAnchor( 0, 0, 0, 0 );
		GetElementRect( m_peHeader, &rcAnchor );

		CRect rcClient;
		GetClientRect( &rcClient );
		rcAnchor.left = rcClient.left;
		rcAnchor.right = rcClient.right;

		CDC dcHeader;
		dcHeader.CreateCompatibleDC( pDC );
		CBitmap* pbmOld = (CBitmap*)dcHeader.SelectObject( &m_bmHeader1 );

		BITMAP pHeader1;
		m_bmHeader1.GetBitmap( &pHeader1 );

		pDC->BitBlt( rcAnchor.left, rcAnchor.top, min( rcAnchor.Width(), pHeader1.bmWidth ),
			min( rcAnchor.Height(), pHeader1.bmHeight ), &dcHeader, 0, 0, SRCCOPY );

		CRect rcMark( &rcAnchor );
		rcMark.left += pHeader1.bmWidth;

		if ( m_bmHeader2.m_hObject != NULL && pDC->RectVisible( &rcMark ) )
		{
			BITMAP pHeader2;
			m_bmHeader2.GetBitmap( &pHeader2 );
			dcHeader.SelectObject( &m_bmHeader2 );

			while ( rcMark.left < rcMark.right )
			{
				pDC->BitBlt( rcMark.left, rcMark.top, min( (LONG)rcMark.Width(), pHeader2.bmWidth ),
					min( (LONG)rcMark.Height(), pHeader2.bmHeight ), &dcHeader, 0, 0, SRCCOPY );
				rcMark.left += pHeader2.bmWidth;
			}
		}

		dcHeader.SelectObject( pbmOld );
		dcHeader.DeleteDC();

		if ( pHeader1.bmHeight < rcAnchor.Height() )
			rcAnchor.bottom = rcAnchor.top + pHeader1.bmHeight;
		if ( m_bmHeader2.m_hObject == NULL )
			rcAnchor.right = rcAnchor.left + pHeader1.bmWidth;

		pDC->ExcludeClipRect( &rcAnchor );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl other event handlers

void CHomeViewCtrl::OnVScrolled()
{
	OnLayoutComplete();
}

///////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl Scrolling (Non-Functional/Redundant)
//
//void CHomeViewCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
//{
//	SCROLLINFO pScroll = {};
//	pScroll.cbSize	= sizeof(pScroll);
//	pScroll.fMask	= SIF_ALL & ~SIF_TRACKPOS;
//
//	GetScrollInfo( SB_VERT, &pScroll );
//
//	switch ( nSBCode )
//	{
//	case SB_BOTTOM:
//		pScroll.nPos = pScroll.nMax;
//		break;
//	case SB_LINEDOWN:
//		pScroll.nPos ++;
//		break;
//	case SB_LINEUP:
//		pScroll.nPos --;
//		break;
//	case SB_PAGEDOWN:
//		pScroll.nPos += pScroll.nPage;
//		break;
//	case SB_PAGEUP:
//		pScroll.nPos -= pScroll.nPage;
//		break;
//	case SB_THUMBPOSITION:
//	case SB_THUMBTRACK:
//		pScroll.nPos = nPos;
//		break;
//	case SB_TOP:
//		pScroll.nPos = 0;
//		break;
//	}
//
//	pScroll.fMask = SIF_POS;
//	pScroll.nPos  = max( 0, min( pScroll.nPos, pScroll.nMax - (int)pScroll.nPage + 1 ) );
//	SetScrollPos( SB_VERT, pScroll.nPos, TRUE );
//
//	OnVScrolled();
//	Invalidate();
//}
