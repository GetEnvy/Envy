//
// PageSettingsWeb.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2007
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
#include "EnvyURL.h"
#include "PageSettingsWeb.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CWebSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CWebSettingsPage, CSettingsPage)
	ON_CBN_EDITCHANGE(IDC_EXT_LIST, OnEditChangeExtList)
	ON_CBN_SELCHANGE(IDC_EXT_LIST, OnSelChangeExtList)
	ON_BN_CLICKED(IDC_EXT_ADD, OnExtAdd)
	ON_BN_CLICKED(IDC_EXT_REMOVE, OnExtRemove)
	ON_BN_CLICKED(IDC_WEB_HOOK, OnWebHook)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWebSettingsPage property page

CWebSettingsPage::CWebSettingsPage()
	: CSettingsPage( CWebSettingsPage::IDD )
	, m_bWebHook	( FALSE )
	, m_bUriMagnet	( FALSE )
	, m_bUriGnutella ( FALSE )
	, m_bUriED2K	( FALSE )
	, m_bUriDC		( FALSE )
	, m_bUriPiolet	( FALSE )
	, m_bUriTorrent	( FALSE )
{
}

CWebSettingsPage::~CWebSettingsPage()
{
}

void CWebSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXT_REMOVE, m_wndExtRemove);
	DDX_Control(pDX, IDC_EXT_ADD, m_wndExtAdd);
	DDX_Control(pDX, IDC_EXT_LIST, m_wndExtensions);
	DDX_Check(pDX, IDC_WEB_HOOK, m_bWebHook);
	DDX_Check(pDX, IDC_URI_MAGNET, m_bUriMagnet);
	DDX_Check(pDX, IDC_URI_GNUTELLA, m_bUriGnutella);
	DDX_Check(pDX, IDC_URI_ED2K, m_bUriED2K);
	DDX_Check(pDX, IDC_URI_DC, m_bUriDC);
	DDX_Check(pDX, IDC_URI_PIOLET, m_bUriPiolet);
	DDX_Check(pDX, IDC_URI_TORRENT, m_bUriTorrent);
}

/////////////////////////////////////////////////////////////////////////////
// CWebSettingsPage message handlers

BOOL CWebSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bUriMagnet	= Settings.Web.Magnet;
	m_bUriGnutella	= Settings.Web.Gnutella;
	m_bUriED2K		= Settings.Web.ED2K;
	m_bUriDC		= Settings.Web.DC;
	m_bUriPiolet	= Settings.Web.Piolet;
	m_bUriTorrent	= Settings.Web.Torrent;

	m_bWebHook		= Settings.Downloads.WebHookEnable;

	for ( string_set::const_iterator i = Settings.Downloads.WebHookExtensions.begin() ;
		i != Settings.Downloads.WebHookExtensions.end() ; i++ )
	{
		m_wndExtensions.AddString( (*i) );
	}

	UpdateData( FALSE );

	OnWebHook();

	return TRUE;
}

void CWebSettingsPage::OnWebHook()
{
	UpdateData( TRUE );
	m_wndExtensions.EnableWindow( m_bWebHook );
	OnEditChangeExtList();
	OnSelChangeExtList();
}

void CWebSettingsPage::OnEditChangeExtList()
{
	m_wndExtAdd.EnableWindow( m_bWebHook && m_wndExtensions.GetWindowTextLength() > 0 );
}

void CWebSettingsPage::OnSelChangeExtList()
{
	m_wndExtRemove.EnableWindow( m_bWebHook && m_wndExtensions.GetCurSel() >= 0 );
}

void CWebSettingsPage::OnExtAdd()
{
	CString strType;
	m_wndExtensions.GetWindowText( strType );

	ToLower( strType );

	strType.Trim();
	if ( strType.IsEmpty() ) return;

	if ( m_wndExtensions.FindString( -1, strType ) >= 0 ) return;

	m_wndExtensions.AddString( strType );
	m_wndExtensions.SetWindowText( L"" );
}

void CWebSettingsPage::OnExtRemove()
{
	int nItem = m_wndExtensions.GetCurSel();
	if ( nItem >= 0 ) m_wndExtensions.DeleteString( nItem );
	m_wndExtRemove.EnableWindow( FALSE );
}

void CWebSettingsPage::OnOK()
{
	UpdateData();

	Settings.Web.Magnet		= m_bUriMagnet != FALSE;
	Settings.Web.Gnutella	= m_bUriGnutella != FALSE;
	Settings.Web.ED2K		= m_bUriED2K != FALSE;
	Settings.Web.DC 		= m_bUriDC != FALSE;
	Settings.Web.Piolet		= m_bUriPiolet != FALSE;
	Settings.Web.Torrent	= m_bUriTorrent != FALSE;

	Settings.Downloads.WebHookEnable = m_bWebHook != FALSE;

	Settings.Downloads.WebHookExtensions.clear();
	for ( int nItem = 0 ; nItem < m_wndExtensions.GetCount() ; nItem++ )
	{
		CString str;
		m_wndExtensions.GetLBText( nItem, str );
		if ( ! str.IsEmpty() )
			Settings.Downloads.WebHookExtensions.insert( str );
	}

	CEnvyURL::Register();

	CSettingsPage::OnOK();
}
