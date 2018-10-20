//
// PageSettingsProtocols.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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
#include "PageSettingsProtocols.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CProtocolsSettingsPage, CSettingsPage)

//BEGIN_MESSAGE_MAP(CProtocolsSettingsPage, CSettingsPage)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProtocolsSettingsPage property page

CProtocolsSettingsPage::CProtocolsSettingsPage() : CSettingsPage(CProtocolsSettingsPage::IDD)
{
}

CProtocolsSettingsPage::~CProtocolsSettingsPage()
{
}

void CProtocolsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROTOCOLS, m_wndTree);
}

/////////////////////////////////////////////////////////////////////////////
// CProtocolsSettingsPage message handlers

BOOL CProtocolsSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	HTREEITEM hNetworks = AddItem( NULL, L"Peer-to-Peer Network Protocols" );

	HTREEITEM hG2 = AddItem( hNetworks, L"Gnutella2 Network" );
	AddItem( hG2, L"Name", L"Gnutella2  (\"G2\")" );
	AddItem( hG2, L"Type", L"Decentralized" );
	AddItem( hG2, L"NodeClass", L"Hub, Leaf" );
	AddItem( hG2, L"PrimaryURN", L"sha1" );
	AddItem( hG2, L"ProtocolVersion", L"1.0" );
	AddItem( hG2, L"ComponentVersion", L"1.0.0.0" );

	HTREEITEM hG1 = AddItem( hNetworks, L"Gnutella Network" );
	AddItem( hG1, L"Name", L"Gnutella" );
	AddItem( hG1, L"Type", L"Decentralized" );
	AddItem( hG1, L"NodeClass", L"Leaf, Ultrapeer*" );
	AddItem( hG1, L"PrimaryURN", L"sha1" );
	AddItem( hG1, L"ProtocolVersion", L"0.6" );
	AddItem( hG1, L"ComponentVersion", L"1.0.0.0" );

	HTREEITEM hED = AddItem( hNetworks, L"eDonkey2000 Network" );
	AddItem( hED, L"Name", L"eDonkey2000" );
	AddItem( hED, L"Type", L"Server-Based" );
	AddItem( hED, L"NodeClass", L"Client" );
	AddItem( hED, L"PrimaryURN", L"ed2k (compound md4)" );
	AddItem( hED, L"ProtocolVersion", L"1.1" );
	AddItem( hED, L"ComponentVersion", L"1.0.0.0" );

	HTREEITEM hDC = AddItem( hNetworks, L"ADC/NMDC Network" );
	AddItem( hDC, L"Name", L"Direct Connect  (\"DC++\")" );
	AddItem( hDC, L"Type", L"Hub-Based" );
	AddItem( hDC, L"NodeClass", L"Client" );
	AddItem( hDC, L"PrimaryURN", L"ttr" );
	AddItem( hDC, L"ProtocolVersion", L"1.0" );
	AddItem( hDC, L"ComponentVersion", L"1.0.0.0" );

	//HTREEITEM hED = AddItem( hNetworks, L"KAD Network" );
	//AddItem( hED, L"Name", L"KAD (Inactive)" );
	//AddItem( hED, L"ComponentVersion", L"0.0.0.0" );

	HTREEITEM hTransfers = AddItem( NULL, L"File Transfer Protocols" );

	HTREEITEM hHTTP = AddItem( hTransfers, L"Hypertext Transfer Protocol (HTTP)" );
	AddItem( hHTTP, L"Name", L"Hypertext Transfer Protocol (HTTP)" );
	AddItem( hHTTP, L"Prefix", L"http://" );
	AddItem( hHTTP, L"TransferMode", L"Stream" );
	AddItem( hHTTP, L"Directions", L"Download, Upload" );
	AddItem( hHTTP, L"Encodings", L"Deflate, Backwards" );
	AddItem( hHTTP, L"Capabilities", L"THEX, PFS, Metadata, HUGE, Browse" );
	AddItem( hHTTP, L"ProtocolVersion", L"1.1" );
	AddItem( hHTTP, L"ComponentVersion", L"1.0.0.0" );

	HTREEITEM hFTP = AddItem( hTransfers, L"File Transfer Protocol (FTP)" );
	AddItem( hFTP, L"Name", L"File Transfer Protocol (FTP)" );
	AddItem( hFTP, L"Prefix", L"ftp://" );
	AddItem( hFTP, L"TransferMode", L"Stream" );
	AddItem( hFTP, L"Directions", L"Download" );
	AddItem( hFTP, L"Capabilities", L"Passive" );
	AddItem( hFTP, L"ProtocolVersion", L"1.0" );
	AddItem( hFTP, L"ComponentVersion", L"1.0.0.0" );

	HTREEITEM hEFTP = AddItem( hTransfers, L"eDonkey2000 Client Link (EFTP)" );
	AddItem( hEFTP, L"Name", L"eDonkey2000 Client Link FTP (EFTP)" );
	AddItem( hEFTP, L"Prefix", L"ed2kftp://" );
	AddItem( hEFTP, L"TransferMode", L"Block" );
	AddItem( hEFTP, L"Directions", L"Download, Upload" );
	AddItem( hEFTP, L"Capabilities", L"Hashset, SourceExchange, Deflate" );
	AddItem( hEFTP, L"ProtocolVersion", L"1.1" );
	AddItem( hEFTP, L"ComponentVersion", L"1.0.0.0" );

	HTREEITEM hBT = AddItem( hTransfers, L"BitTorrent Coupling (BT)" );
	AddItem( hBT, L"Name", L"BitTorrent Coupling (BT)" );
	AddItem( hBT, L"Prefix", L"btc://" );
	AddItem( hBT, L"TransferMode", L"Block" );
	AddItem( hBT, L"Directions", L"Download, Upload" );
	AddItem( hBT, L"Capabilities", L"PeerExchange, (etc.)" );
	AddItem( hBT, L"ProtocolVersion", L"1.0" );
	AddItem( hBT, L"ComponentVersion", L"1.0.0.0" );

	return TRUE;
}

HTREEITEM CProtocolsSettingsPage::AddItem(HTREEITEM hParent, LPCTSTR pszText, LPCTSTR pszValue)
{
	if ( Settings.General.LanguageRTL ) m_wndTree.ModifyStyleEx( 0, TVS_RTLREADING, 0 );
	if ( pszValue != NULL )
	{
		CString str;
		str.Format( L"%s = %s", pszText, pszValue );
		if ( Settings.General.LanguageRTL ) str = L"\x202A" + str;
		return m_wndTree.InsertItem( TVIF_TEXT|TVIF_STATE, str,
			0, 0, 0, 0, 0, hParent, TVI_LAST );
	}
	else
	{
		CString str( pszText );
		if ( Settings.General.LanguageRTL ) str = L"\x202A" + str;
		return m_wndTree.InsertItem( TVIF_TEXT|TVIF_STATE, str,
			0, 0, TVIS_EXPANDED|TVIS_BOLD, TVIS_EXPANDED|TVIS_BOLD, 0, hParent, TVI_LAST );
	}
}
