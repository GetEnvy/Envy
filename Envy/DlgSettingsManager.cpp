//
// DlgSettingsManager.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2007
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
#include "DlgSettingsManager.h"
#include "Skin.h"

#include "PageSettingsRich.h"
#include "PageSettingsGeneral.h"
#include "PageSettingsLibrary.h"
#include "PageSettingsMedia.h"
#include "PageSettingsCommunity.h"
#include "PageSettingsIRC.h"
#include "PageSettingsWeb.h"
#include "PageSettingsConnection.h"
#include "PageSettingsDownloads.h"
#include "PageSettingsUploads.h"
#include "PageSettingsRemote.h"
#include "PageSettingsNetworks.h"
#include "PageSettingsGnutella.h"
#include "PageSettingsDonkey.h"
#include "PageSettingsDC.h"
#include "PageSettingsBitTorrent.h"
#include "PageSettingsProtocols.h"
#include "PageSettingsPlugins.h"
#include "PageSettingsSkins.h"
#include "PageSettingsAdvanced.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CSettingsManagerDlg, CSettingsSheet)

BEGIN_MESSAGE_MAP(CSettingsManagerDlg, CSettingsSheet)
	ON_COMMAND(IDRETRY, OnApply)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSettingsManagerDlg dialog

CSettingsManagerDlg::CSettingsManagerDlg(CWnd* pParent)
	: CSettingsSheet( pParent, IDS_SETTINGS )
{
}

void CSettingsManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsManagerDlg operations

CSettingsManagerDlg* CSettingsManagerDlg::m_pThis = NULL;

BOOL CSettingsManagerDlg::Run(LPCTSTR pszWindow)
{
	if ( m_pThis ) return FALSE;
	CSettingsManagerDlg pSheet;
	m_pThis = &pSheet;
	BOOL bResult = ( pSheet.DoModal( pszWindow ) == IDOK );
	m_pThis = NULL;
	return bResult;
}

void CSettingsManagerDlg::OnSkinChange(BOOL bSet)
{
	if ( m_pThis == NULL ) return;

	if ( ! bSet )
	{
		m_pThis->RemoveSkin();
		return;
	}

	m_pThis->SkinMe( L"CSettingSheet", IDR_MAINFRAME );

	for ( INT_PTR i = 0 ; i < m_pThis->GetPageCount() ; ++i )
	{
		CSettingsPage* pPage = m_pThis->GetPage( i );
		pPage->OnSkinChange();
	}

	m_pThis->Invalidate();
}

INT_PTR CSettingsManagerDlg::DoModal(LPCTSTR pszWindow)
{
	const BOOL bAdvanced = Settings.General.GUIMode != GUI_BASIC;

	CAutoPtr< CRichSettingsPage >		gGeneral( new CRichSettingsPage( L"CGeneralSettingsGroup" ) );
	CAutoPtr< CGeneralSettingsPage >	pGeneral( new CGeneralSettingsPage );
	CAutoPtr< CLibrarySettingsPage >	pLibrary( new CLibrarySettingsPage );
	CAutoPtr< CWebSettingsPage >		pWeb( new CWebSettingsPage );
	CAutoPtr< CMediaSettingsPage >		pMedia( new CMediaSettingsPage );
	CAutoPtr< CIRCSettingsPage >		pIRC( new CIRCSettingsPage );
	CAutoPtr< CCommunitySettingsPage >	pCommunity( new CCommunitySettingsPage );
	CAutoPtr< CRemoteSettingsPage >		pRemote( new CRemoteSettingsPage );
	CAutoPtr< CRichSettingsPage >		gInternet( new CRichSettingsPage( L"CInternetSettingsGroup" ) );
	CAutoPtr< CConnectionSettingsPage >	pConnection( new CConnectionSettingsPage );
	CAutoPtr< CDownloadsSettingsPage >	pDownloads( new CDownloadsSettingsPage );
	CAutoPtr< CUploadsSettingsPage >	pUploads( new CUploadsSettingsPage );
	CAutoPtr< CNetworksSettingsPage >	gNetworks( new CNetworksSettingsPage );
	CAutoPtr< CGnutellaSettingsPage >	pGnutella( new CGnutellaSettingsPage );
	CAutoPtr< CDonkeySettingsPage >		pDonkey( new CDonkeySettingsPage );
	CAutoPtr< CDCSettingsPage > 		pDC( new CDCSettingsPage );
	CAutoPtr< CBitTorrentSettingsPage >	pTorrent( new CBitTorrentSettingsPage );
	CAutoPtr< CSkinsSettingsPage >		pSkins( new CSkinsSettingsPage );
	CAutoPtr< CPluginsSettingsPage >	pPlugins( new CPluginsSettingsPage );
	CAutoPtr< CAdvancedSettingsPage >	pAdvanced( new CAdvancedSettingsPage );
	CAutoPtr< CProtocolsSettingsPage >	pProtocols( new CProtocolsSettingsPage );

	AddGroup( gGeneral );			// Richdoc CGeneralSettingsGroup
	AddPage( pGeneral );			// IDD_SETTINGS_GENERAL
	AddPage( pLibrary );			// IDD_SETTINGS_LIBRARY
	if ( bAdvanced )
	{
		AddPage( pWeb );			// IDD_SETTINGS_WEB
		AddPage( pMedia ); 			// IDD_SETTINGS_MEDIA
		AddPage( pIRC );			// IDD_SETTINGS_IRC
		AddPage( pCommunity );		// IDD_SETTINGS_COMMUNITY
		AddPage( pRemote );			// IDD_SETTINGS_REMOTE
	}

	AddGroup( gInternet ); 			// Richdoc CInternetSettingsGroup
	AddPage( pConnection );			// IDD_SETTINGS_CONNECTION
	AddPage( pDownloads ); 			// IDD_SETTINGS_DOWNLOADS
	AddPage( pUploads );			// IDD_SETTINGS_UPLOADS

	if ( bAdvanced )
	{
		AddGroup( gNetworks ); 		// IDD_SETTINGS_NETWORKS
		AddPage( pGnutella );		// IDD_SETTINGS_GNUTELLA
		if ( ! Settings.Experimental.LAN_Mode )
		{
			//if ( Settings.eDonkey.ShowInterface )
			AddPage( pDonkey );		// IDD_SETTINGS_DONKEY
			//if ( Settings.DC.ShowInterface )
			AddPage( pDC );			// IDD_SETTINGS_DC
			AddPage( pTorrent );	// IDD_SETTINGS_BITTORRENT
		}
	}

	AddGroup( pSkins );				// IDD_SETTINGS_SKINS
	if ( bAdvanced )
	{
		AddGroup( pPlugins );		// IDD_SETTINGS_PLUGINS
		AddGroup( pAdvanced ); 		// IDD_SETTINGS_ADVANCED
		AddPage( pProtocols ); 		// IDD_SETTINGS_PROTOCOLS	ToDo: Remove or Improve?
	}

	SetActivePage( GetPage( pszWindow ? pszWindow : Settings.General.LastSettingsPage ) );

	INT_PTR nReturn = CSettingsSheet::DoModal();

	if ( m_pFirst )
		Settings.General.LastSettingsPage = m_pFirst->GetRuntimeClass()->m_lpszClassName;

	return nReturn;
}

void CSettingsManagerDlg::AddPage(CSettingsPage* pPage)
{
	CString strCaption = Skin.GetDialogCaption( CString( pPage->GetRuntimeClass()->m_lpszClassName ) );
	CSettingsSheet::AddPage( pPage, strCaption.GetLength() ? (LPCTSTR)strCaption : NULL );
}

void CSettingsManagerDlg::AddGroup(CSettingsPage* pPage)
{
	if ( pPage->IsKindOf( RUNTIME_CLASS(CRichSettingsPage) ) )
	{
		CString strCaption = ((CRichSettingsPage*)pPage)->m_sCaption;
		CSettingsSheet::AddGroup( pPage, strCaption );
	}
	else
	{
		CString strName( pPage->GetRuntimeClass()->m_lpszClassName );
		CString strCaption = Skin.GetDialogCaption( strName );
		CSettingsSheet::AddGroup( pPage, strCaption.GetLength() ? (LPCTSTR)strCaption : NULL );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsManagerDlg message handlers

BOOL CSettingsManagerDlg::OnInitDialog()
{
	CSettingsSheet::OnInitDialog();

//	m_bmHeader.LoadBitmap( IDB_BANNER );	// Obsolete

	SkinMe( L"CSettingSheet", IDR_MAINFRAME, TRUE );

	return TRUE;
}

void CSettingsManagerDlg::OnOK()
{
	CSettingsSheet::OnOK();
	Settings.Save();
	AfxGetMainWnd()->Invalidate();
}

void CSettingsManagerDlg::OnApply()
{
	CSettingsSheet::OnApply();
	AfxGetMainWnd()->Invalidate();
}


// Obsolete Banner Method:
//void CSettingsManagerDlg::DoPaint(CDC& dc)
//{
//	CRect rc;
//	GetClientRect( &rc );
//	BITMAP pInfo;
//	m_bmHeader.GetBitmap( &pInfo );
//
//	CDC mdc;
//	mdc.CreateCompatibleDC( &dc );
//	CBitmap* pOldBitmap = (CBitmap*)mdc.SelectObject( &m_bmHeader );
//	dc.BitBlt( 0, 0, pInfo.bmWidth, pInfo.bmHeight, &mdc, 0, 0, SRCCOPY );
//	mdc.SelectObject( pOldBitmap );
//	mdc.DeleteDC();
//
//	//dc.FillSolidRect( 438, 0, rc.right - 438, 48, RGB( 0xBE, 0, 0 ) );
//	//dc.Draw3dRect( 438, 48, rc.right - 437, 2, RGB( 169, 0, 0 ), RGB( 110, 59, 59 ) );
//	//dc.Draw3dRect( 0, 50, rc.Width() + 1, 1, RGB( 128, 128, 128 ), RGB( 128, 128, 128 ) );
//
//	CSettingsSheet::DoPaint( dc );
//}
