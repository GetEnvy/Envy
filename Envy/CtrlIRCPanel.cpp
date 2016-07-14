//
// CtrlIRCPanel.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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
#include "Settings.h"
#include "Envy.h"
#include "CtrlIRCPanel.h"

#include "DlgIrcInput.h"
#include "RichDocument.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CIRCPanel, CTaskPanel)
BEGIN_MESSAGE_MAP(CIRCPanel, CTaskPanel)
	ON_WM_CREATE()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CIRCChannelsBox, CTaskBox)
BEGIN_MESSAGE_MAP(CIRCChannelsBox, CTaskBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_NOTIFY(NM_DBLCLK, IDC_IRC_CHANNELS, OnChansDoubleClick)
	ON_COMMAND(IDC_IRC_ADDCHANNEL, OnAddChannel)
	ON_COMMAND(IDC_IRC_REMOVECHANNEL, OnRemoveChannel)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CIRCUsersBox, CTaskBox)
BEGIN_MESSAGE_MAP(CIRCUsersBox, CTaskBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_CONTEXTMENU()
	ON_WM_COMPAREITEM()
	ON_LBN_DBLCLK(IDC_IRC_USERS, OnUsersDoubleClick)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIRCPanel construction

CIRCPanel::CIRCPanel()
{
}

CIRCPanel::~CIRCPanel()
{
	m_pFont.Detach();
}

/////////////////////////////////////////////////////////////////////////////
// CIRCPanel message handlers

BOOL CIRCPanel::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CTaskPanel::Create( L"CIRCPanel", WS_VISIBLE, rect, pParentWnd, 0 );
}

int CIRCPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskPanel::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_boxChans.Create( this, 200, L"Channels", IDR_CHANSFRAME );
	m_boxUsers.Create( this, 200, L"Users", IDR_USERSFRAME );

	m_boxUsers.ModifyStyleEx( 0, WS_EX_COMPOSITED );	// Stop control flickering XP+

	m_pFont.Attach( theApp.m_gdiFontBold );
	m_boxChans.m_wndChanList.SetFont( &m_pFont );
	m_boxUsers.m_wndUserList.SetFont( &m_pFont );

	AddBox( &m_boxChans );
	AddBox( &m_boxUsers );

	SetStretchBox( &m_boxUsers );

	OnSkinChange();

	return 0;
}

void CIRCPanel::OnSkinChange()
{
	CString strCaption;

	LoadString( strCaption, IDS_IRC_CHANLIST );
	m_boxChans.SetCaption( strCaption );
	LoadString( strCaption, IDS_IRC_USERLIST );
	m_boxUsers.SetCaption( strCaption );

	SetWatermark( L"CIRCPanel" );
	SetFooter( L"CIRCPanel.Footer" );

	m_boxChans.SetWatermark( L"CIRCChannelsBox" );
	m_boxChans.SetCaptionmark( L"CIRCChannelsBox.Caption" );
	m_boxChans.OnSkinChange();

	m_boxUsers.SetWatermark( L"CIRCUsersBox" );
	m_boxUsers.SetCaptionmark( L"CIRCUsersBox.Caption" );
	m_boxUsers.OnSkinChange();

	Invalidate();
}


/////////////////////////////////////////////////////////////////////////////
// CIRCChannelsBox construction

CIRCChannelsBox::CIRCChannelsBox()
{
}

CIRCChannelsBox::~CIRCChannelsBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CIRCChannelsBox message handlers

int CIRCChannelsBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	SetOwner( GetParent() );

	CRect rc( 0, 0, 0, 0 );

	m_wndChanList.Create( WS_CHILD | WS_VSCROLL | WS_TABSTOP | WS_GROUP | WS_VISIBLE |
		LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SORTASCENDING | LVS_NOLABELWRAP,
		rc, this, IDC_IRC_CHANNELS );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	m_wndChanList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_TRANSPARENTBKGND|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );
	m_wndChanList.InsertColumn( 0, L"Channels", LVCFMT_LEFT, rc.right - 36 );
	m_wndChanList.InsertColumn( 1, L"UserCount", LVCFMT_RIGHT, 36 );

	m_wndAddChannel.Create( rc, this, IDC_IRC_ADDCHANNEL, WS_TABSTOP | BS_DEFPUSHBUTTON );
	m_wndAddChannel.SetHandCursor( TRUE );

	m_wndRemoveChannel.Create( rc, this, IDC_IRC_REMOVECHANNEL, WS_TABSTOP );
	m_wndRemoveChannel.SetHandCursor( TRUE );

	OnSkinChange();

	SetPrimary( TRUE );

	return 0;
}

void CIRCChannelsBox::OnPaint()
{
	CTaskBox::OnPaint();
}

void CIRCChannelsBox::OnSkinChange()
{
	CString strCaption;

	LoadString( strCaption, IDS_IRC_ADDCHANNEL );
	m_wndAddChannel.SetWindowText( strCaption );
	m_wndAddChannel.SetCoolIcon( ID_IRC_ADD, Settings.General.LanguageRTL );

	LoadString( strCaption, IDS_IRC_REMOVECHANNEL );
	m_wndRemoveChannel.SetWindowText( strCaption );
	m_wndRemoveChannel.SetCoolIcon( ID_IRC_REMOVE, Settings.General.LanguageRTL );

	if ( m_wndChanList.SetBkImage( Skin.GetWatermark( L"CIRCChannelsBox" ) ) )
		m_wndChanList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP ); 	// No LVS_EX_DOUBLEBUFFER
	else
		m_wndChanList.SetBkColor( Colors.m_crTaskBoxClient );

	m_wndChanList.SetTextColor( Colors.m_crTaskBoxText );
}

void CIRCChannelsBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );

	HDWP hDWP = BeginDeferWindowPos( 3 );

	int nButton = ( cx - 24 ) / 2;

	DeferWindowPos( hDWP, m_wndChanList, NULL, 2, 2, cx - 2, cy - 24 - 18,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndAddChannel, NULL, 9, cy - 24 - 8, nButton, 24,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndRemoveChannel, NULL, cx - nButton - 9, cy - 24 - 8, nButton, 24,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );

	EndDeferWindowPos( hDWP );

	m_wndChanList.SetColumnWidth( 0, cx - 40 - GetSystemMetrics( SM_CXVSCROLL ) );
	m_wndChanList.SetColumnWidth( 1, 36 );
}

void CIRCChannelsBox::OnChansDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	IRC_PANELEVENT pNotify;
	pNotify.hdr.hwndFrom	= GetSafeHwnd();
	pNotify.hdr.idFrom		= IDC_IRC_DBLCLKCHANNELS;
	pNotify.hdr.code		= NM_DBLCLK;
	CWnd* m_wndFrame = GetOwner()->GetOwner();
	m_wndFrame->PostMessage( WM_NOTIFY, pNotify.hdr.idFrom, (LPARAM)&pNotify );
	*pResult = 0;
}

void CIRCChannelsBox::OnAddChannel()
{
	CIrcInputDlg dlg( this, 0, FALSE );	// 0 = select the first caption

	if ( dlg.DoModal() != IDOK ) return;

	CString strChannel = dlg.m_sAnswer;
	if ( ! strChannel.IsEmpty() )
	{
		if ( strChannel.GetAt( 0 ) != '#' ) strChannel = '#' + strChannel;
		for ( int nChannel=0 ; nChannel < m_wndChanList.GetItemCount() ; nChannel++ )
		{
			if ( strChannel.CompareNoCase( m_wndChanList.GetItemText( nChannel, 0 ) ) == 0 )
			{
				MsgBox( L"Channel already in list.", MB_OK );
				return;
			}
		}
		if ( strChannel.GetAt( 0 ) == '#' ) strChannel = strChannel.Mid( 1 );
		strChannel = strChannel.Left( 1 ).MakeUpper() + strChannel.Mid( 1 );
		m_wndChanList.InsertItem( -1, strChannel );
		m_sPassedChannel = strChannel;
		GetOwner()->GetOwner()->PostMessage( WM_ADDCHANNEL, IDC_IRC_CHANNELS );
	}
}

void CIRCChannelsBox::OnRemoveChannel()
{
	GetOwner()->GetOwner()->PostMessage( WM_REMOVECHANNEL, IDC_IRC_CHANNELS );
}


/////////////////////////////////////////////////////////////////////////////
// CIRCUsersBox construction

CIRCUsersBox::CIRCUsersBox()
{
}

CIRCUsersBox::~CIRCUsersBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CIRCUsersBox message handlers

int CIRCUsersBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc( 0, 0, 0, 0 );
	m_wndUserList.Create( WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP |
		LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_SORT | LBS_HASSTRINGS,
		rc, this, IDC_IRC_USERS );
	//m_wndUserList.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );	// WS_EX_TRANSPARENT?
	if ( Settings.General.LanguageRTL )
		m_wndUserList.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );

	OnSkinChange();

	return 0;
}

void CIRCUsersBox::OnSkinChange()
{
	// ToDo: Skin m_wndUserList ListBox HBRUSH:  Colors.m_crTaskBoxClient + Skin.GetWatermark( &bmTaskBox, L"CIRCUsersBox" )
}

HBRUSH CIRCUsersBox::OnCtlColor(CDC* pDC, CWnd* /*pWnd*/, UINT /*nCtlColor*/)
{
	// pWnd == &m_wndUserList
	pDC->SetTextColor( Colors.m_crTaskBoxText );	// Was Colors.m_crRichdocText
	pDC->SetBkColor( Colors.m_crTaskBoxClient );
	return CreateSolidBrush( Colors.m_crTaskBoxClient );
}

void CIRCUsersBox::OnPaint()
{
	CTaskBox::OnPaint();
}

void CIRCUsersBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );

	m_wndUserList.SetWindowPos( NULL, 6, 1, cx - 6, cy - 2,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
}

void CIRCUsersBox::OnUsersDoubleClick()
{
	CPoint pt;
	GetCursorPos( &pt );
	int nItem = HitTest( pt );
	if ( nItem >= 0 )
	{
		IRC_PANELEVENT pNotify;
		pNotify.hdr.hwndFrom	= GetSafeHwnd();
		pNotify.hdr.idFrom		= IDC_IRC_DBLCLKUSERS;
		pNotify.hdr.code		= NM_DBLCLK;
		CWnd* m_wndFrame = GetOwner()->GetOwner();
		m_wndFrame->PostMessage( WM_NOTIFY, pNotify.hdr.idFrom, (LPARAM)&pNotify );
	}
}

void CIRCUsersBox::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	int nItem = HitTest( point );
	if ( nItem >= 0 )
	{
		m_wndUserList.SetCurSel( (int)nItem );

		IRC_PANELEVENT pNotify;
		pNotify.hdr.hwndFrom	= GetSafeHwnd();
		pNotify.hdr.idFrom		= IDC_IRC_MENUUSERS;
		pNotify.hdr.code		= NM_RCLICK;
		CWnd* m_wndFrame = GetOwner()->GetOwner();
		m_wndFrame->PostMessage( WM_NOTIFY, pNotify.hdr.idFrom, (LPARAM)&pNotify );
	}
}

int CIRCUsersBox::HitTest(const CPoint& pt) const
{
	if ( m_wndUserList.GetCount() != 0 )
	{
		BOOL bOutside = FALSE;
		CPoint point( pt );
		ScreenToClient( &point );
		UINT nItem = m_wndUserList.ItemFromPoint( point, bOutside );
		if ( ! bOutside ) return nItem;
	}
	return -1;
}

void CIRCUsersBox::UpdateCaptionCount()
{
	CString strCount;
	strCount.Format( L" (%d)", m_wndUserList.GetCount() );
	SetCaption( LoadString( IDS_IRC_USERLIST ) + strCount );
}

int CIRCUsersBox::OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	if ( nIDCtl != IDC_IRC_USERS )
		return CTaskBox::OnCompareItem( nIDCtl, lpCompareItemStruct );

	ASSERT( lpCompareItemStruct->CtlType == ODT_LISTBOX );
	LPCTSTR lpszUser1 = (LPCTSTR) lpCompareItemStruct->itemData1;
	ASSERT( lpszUser1 != NULL );
	LPCTSTR lpszUser2 = (LPCTSTR) lpCompareItemStruct->itemData2;
	ASSERT( lpszUser2 != NULL );

	// Sort by user status
	int nModeColumn1 = 0, nModeColumn2 = 0;
	switch ( *lpszUser1 )
	{
		case L'+': nModeColumn1 = 1; break;
		case L'%': nModeColumn1 = 2; break;
		case L'@': nModeColumn1 = 3; break;
		case L'&': nModeColumn1 = 4;
	}
	switch ( *lpszUser2 )
	{
		case L'+': nModeColumn2 = 1; break;
		case L'%': nModeColumn2 = 2; break;
		case L'@': nModeColumn2 = 3; break;
		case L'&': nModeColumn2 = 4;
	}
	if ( nModeColumn1 == nModeColumn2 )
	{
		// Sort by user name
		if ( nModeColumn1 ) lpszUser1++;
		if ( nModeColumn2 ) lpszUser2++;
		return _tcsicmp( lpszUser1, lpszUser2 );
	}
	return ( nModeColumn2 - nModeColumn1 );
}
