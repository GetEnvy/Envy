//
// DlgConnectTo.cpp
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

// ConnectTo, BrowseTo, ChatTo, multifunction IP control

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "DlgConnectTo.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

typedef struct {
	CString		sHost;
	int			nPort;
	PROTOCOLID	nProtocol;
} CONNECT_HOST_DATA;

const LPCTSTR CONNECT_SECTION = L"ConnectTo";

IMPLEMENT_DYNAMIC(CConnectToDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CConnectToDlg, CSkinDialog)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_CONNECT_HOST, OnCbnSelchangeConnectHost)
	ON_CBN_SELCHANGE(IDC_CONNECT_PROTOCOL, OnCbnSelchangeConnectProtocol)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CConnectToDlg dialog

CConnectToDlg::CConnectToDlg(CWnd* pParent, Type nType)
	: CSkinDialog		( CConnectToDlg::IDD, pParent )
	, m_nType			( nType )
	, m_nPort			( protocolPorts[ PROTOCOL_G2 ] )
	, m_nProtocol		( PROTOCOL_G2 )
	, m_bNoUltraPeer	( FALSE )
{
}

void CConnectToDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_CONNECT_ADVANCED, m_wndAdvanced);
	DDX_Control(pDX, IDC_CONNECT_PROTOCOL, m_wndProtocol);
	DDX_Control(pDX, IDC_CONNECT_ULTRAPEER, m_wndUltrapeer);
	DDX_Control(pDX, IDC_CONNECT_PROMPT, m_wndPrompt);
	DDX_Control(pDX, IDC_CONNECT_PORT, m_wndPort);
	DDX_Control(pDX, IDC_CONNECT_HOST, m_wndHost);
	DDX_CBString(pDX, IDC_CONNECT_HOST, m_sHost);
	DDX_Check(pDX, IDC_CONNECT_ULTRAPEER, m_bNoUltraPeer);
	DDX_Text(pDX, IDC_CONNECT_PORT, m_nPort);
}

/////////////////////////////////////////////////////////////////////////////
// CConnectToDlg message handlers

BOOL CConnectToDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CConnectToDlg",
		( ( m_nType == Connect ) ? ID_NETWORK_CONNECT_TO :
		( ( m_nType == Browse ) ? ID_NETWORK_BROWSE_TO :
		/*( m_nType == Chat ) ?*/ ID_NETWORK_CHAT_TO ) ) );

	SelectCaption( this, (int)m_nType );
	SelectCaption( &m_wndPrompt, (int)m_nType );

	CoolInterface.LoadIconsTo( m_gdiProtocols, protocolIDs );

	m_wndProtocol.ResetContent();
	m_wndProtocol.SetItemData( m_wndProtocol.AddString( protocolNames[ PROTOCOL_G2 ] ), PROTOCOL_G2 );
	m_wndProtocol.SetItemData( m_wndProtocol.AddString( protocolNames[ PROTOCOL_G1 ] ), PROTOCOL_G1 );
	m_wndProtocol.SetItemData( m_wndProtocol.AddString( protocolNames[ PROTOCOL_ED2K ] ), PROTOCOL_ED2K );
	m_wndProtocol.SetItemData( m_wndProtocol.AddString( protocolNames[ PROTOCOL_DC ] ), PROTOCOL_DC );

	//m_wndAdvanced.ShowWindow( ( m_nType == Connect ) ? SW_SHOW : SW_HIDE );
	m_wndUltrapeer.ShowWindow( ( m_nType == Connect ) ? SW_SHOW : SW_HIDE );
	m_wndUltrapeer.EnableWindow( FALSE );

	int nItem, nCount = theApp.GetProfileInt( CONNECT_SECTION, L"Count", 0 );

	for ( nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CONNECT_HOST_DATA* pData = new CONNECT_HOST_DATA;
		if ( pData )
		{
			CString strItem;
			strItem.Format( L"%.3i.Host", nItem + 1 );
			pData->sHost = theApp.GetProfileString( CONNECT_SECTION, strItem, L"" );
			pData->sHost.Trim( L" \t\r\n:\"" );
			ToLower( pData->sHost );

			strItem.Format( L"%.3i.Protocol", nItem + 1 );
			pData->nProtocol = (PROTOCOLID)theApp.GetProfileInt( CONNECT_SECTION, strItem, PROTOCOL_G2 );

			strItem.Format( L"%.3i.Port", nItem + 1 );
			pData->nPort = theApp.GetProfileInt( CONNECT_SECTION, strItem, protocolPorts[ pData->nProtocol ] );

			// Validation
			if ( pData->sHost.GetLength() &&
				pData->nPort >= 0 &&
				pData->nPort <= 65535 &&
				( pData->nProtocol == PROTOCOL_G1 ||
				  pData->nProtocol == PROTOCOL_G2 ||
				  pData->nProtocol == PROTOCOL_ED2K ||
				  pData->nProtocol == PROTOCOL_DC ) &&
				m_wndHost.FindStringExact( -1, pData->sHost ) == CB_ERR )
			{
				int nIndex = m_wndHost.AddString( pData->sHost );
				ASSERT( nIndex != CB_ERR );
				VERIFY( m_wndHost.SetItemDataPtr( nIndex, pData ) != CB_ERR );
			}
			else
			{
				delete pData;
			}
		}
	}
	nCount = m_wndHost.GetCount();
	nItem = theApp.GetProfileInt( CONNECT_SECTION, L"Last.Index", 0 );
	if ( nItem >= nCount ) nItem = 0;
	LoadItem( nItem );

	m_wndPort.SendMessage(EM_SETLIMITTEXT, 5);

	UpdateData( FALSE );

	return TRUE;
}

void CConnectToDlg::LoadItem(int nItem)
{
	ASSERT( nItem != CB_ERR );

	if ( m_wndHost.GetCurSel() != nItem )
		m_wndHost.SetCurSel( nItem );

	CONNECT_HOST_DATA* pData = static_cast< CONNECT_HOST_DATA* >( m_wndHost.GetItemDataPtr( nItem ) );
	ASSERT( pData != NULL );
	if ( reinterpret_cast< INT_PTR >( pData ) != -1 )
	{
		m_sHost		= pData->sHost;
		m_nPort		= pData->nPort;
		m_nProtocol	= pData->nProtocol;
	}
	m_wndUltrapeer.EnableWindow( m_nProtocol == PROTOCOL_G1 );
	UpdateData( FALSE );
}

void CConnectToDlg::OnCbnSelchangeConnectHost()
{
	if ( ! UpdateData () ) return;

	const int nItem = m_wndHost.GetCurSel();
	ASSERT( nItem != CB_ERR );
	LoadItem( nItem );
}

void CConnectToDlg::OnCbnSelchangeConnectProtocol()
{
	if ( ! UpdateData () ) return;

	m_wndUltrapeer.EnableWindow( m_nProtocol == PROTOCOL_G1 );
}

void CConnectToDlg::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 256;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CConnectToDlg::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CPoint pt( rcItem.left + 1, rcItem.top + 1 );
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );
	if ( Settings.General.LanguageRTL )
		SetLayout( dc.m_hDC, LAYOUT_RTL );

	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );
	dc.SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	dc.FillSolidRect( &rcItem,
		GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHT : COLOR_WINDOW ) );
	dc.SetBkMode( TRANSPARENT );

	m_gdiProtocols.Draw( &dc, lpDrawItemStruct->itemData, pt,
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );

	CString str;
	m_wndProtocol.GetLBText( lpDrawItemStruct->itemID, str );

	rcItem.left += 22;
	rcItem.right -= 2;
	dc.DrawText( str, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX );

	dc.SelectObject( pOldFont );
	dc.Detach();
}

void CConnectToDlg::OnOK()
{
	if ( ! UpdateItems() )
		return;

	SaveItems();

	CSkinDialog::OnOK();
}

BOOL CConnectToDlg::UpdateItems()
{
	if ( ! UpdateData() )
		return FALSE;

	m_sHost.Trim( L" \t\r\n:\"\'\\" ).MakeLower();
	const int n = m_sHost.Find( L':' );
	if ( n != -1 )
	{
		m_nPort = _tstoi( m_sHost.Mid( n + 1 ) );
		m_sHost = m_sHost.Left( n );
	}
	if ( m_sHost.IsEmpty() || m_nPort <= 0 || m_nPort >= 65536 )
	{
		UpdateData( FALSE );
		return FALSE;
	}

	int nItem = m_wndHost.FindStringExact( -1, m_sHost );
	if ( nItem != CB_ERR )
	{
		// Edit existing item
		CONNECT_HOST_DATA* pData = static_cast< CONNECT_HOST_DATA* >( m_wndHost.GetItemDataPtr( nItem ) );
		ASSERT( pData != NULL && reinterpret_cast< INT_PTR >( pData ) != -1 );
		pData->nPort = m_nPort;
		pData->nProtocol = m_nProtocol;
		if ( m_wndHost.GetCurSel() != nItem )
			m_wndHost.SetCurSel( nItem );
	}
	else
	{
		// Create new item
		CONNECT_HOST_DATA* pData = new CONNECT_HOST_DATA;
		ASSERT( pData != NULL );
		if ( pData )
		{
			pData->sHost = m_sHost;
			pData->nPort = m_nPort;
			pData->nProtocol = m_nProtocol;
			nItem = m_wndHost.AddString( pData->sHost );
			ASSERT( nItem != CB_ERR );
			VERIFY( m_wndHost.SetItemDataPtr( nItem, pData ) != CB_ERR );
			VERIFY( m_wndHost.SetCurSel( nItem ) != CB_ERR );
		}
	}

	return TRUE;
}

BOOL CConnectToDlg::UpdateData(BOOL bSaveAndValidate)
{
	if ( bSaveAndValidate )
	{
		m_nProtocol = (PROTOCOLID)m_wndProtocol.GetItemData( m_wndProtocol.GetCurSel() );
	}
	else
	{
		int nIndex = 0;
		for ( int i = 0 ; i < m_wndProtocol.GetCount() ; i++ )
		{
			if ( (PROTOCOLID)m_wndProtocol.GetItemData( i ) == m_nProtocol )
			{
				nIndex = i;
				break;
			}
		}
		m_wndProtocol.SetCurSel( nIndex );
	}
	return CSkinDialog::UpdateData( bSaveAndValidate );
}

void CConnectToDlg::SaveItems()
{
	const int nCount = m_wndHost.GetCount();
	ASSERT( nCount != CB_ERR );
	theApp.WriteProfileInt( CONNECT_SECTION, L"Count", nCount );

	int nItem = m_wndHost.GetCurSel();
	ASSERT( nItem != CB_ERR );
	theApp.WriteProfileInt( CONNECT_SECTION, L"Last.Index", nItem );

	for ( nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CONNECT_HOST_DATA* pData = static_cast< CONNECT_HOST_DATA* >( m_wndHost.GetItemDataPtr( nItem ) );
		ASSERT( pData != NULL && reinterpret_cast< INT_PTR >( pData ) != -1 );

		CString strItem;
		strItem.Format( L"%.3i.Host", nItem + 1 );
		theApp.WriteProfileString( CONNECT_SECTION, strItem, pData->sHost );
		strItem.Format( L"%.3i.Port", nItem + 1 );
		theApp.WriteProfileInt( CONNECT_SECTION, strItem, pData->nPort );
		strItem.Format( L"%.3i.Protocol", nItem + 1 );
		theApp.WriteProfileInt( CONNECT_SECTION, strItem, pData->nProtocol );
	}
}

void CConnectToDlg::OnDestroy()
{
	while( m_wndHost.GetCount() )
	{
		CONNECT_HOST_DATA* pData = static_cast< CONNECT_HOST_DATA* >( m_wndHost.GetItemDataPtr( 0 ) );
		ASSERT( pData != NULL && reinterpret_cast< INT_PTR >( pData ) != -1 );

		m_wndHost.DeleteString( 0 );
		delete pData;
	}

	CSkinDialog::OnDestroy();
}
