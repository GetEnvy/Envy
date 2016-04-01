//
// PageSettingsNetworks.cpp
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
#include "WndSettingsSheet.h"
#include "PageSettingsNetworks.h"
#include "PageSettingsGnutella.h"
#include "PageSettingsDonkey.h"
#include "PageSettingsDC.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CNetworksSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CNetworksSettingsPage, CSettingsPage)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_G2_ENABLE, OnG2Enable)
	ON_BN_CLICKED(IDC_G1_ENABLE, OnG1Enable)
	ON_BN_CLICKED(IDC_ED2K_ENABLE, OnEd2kEnable)
	ON_BN_CLICKED(IDC_DC_ENABLE, OnDCEnable)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNetworksSettingsPage property page

CNetworksSettingsPage::CNetworksSettingsPage()
	: CSettingsPage( CNetworksSettingsPage::IDD )
	, m_bG2Enable ( TRUE )
	, m_bG1Enable ( FALSE )
	, m_bEDEnable ( FALSE )
	, m_bDCEnable ( FALSE )
{
}

CNetworksSettingsPage::~CNetworksSettingsPage()
{
}

void CNetworksSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_G2_SETUP, m_wndG2Setup);
	DDX_Control(pDX, IDC_G1_SETUP, m_wndG1Setup);
	DDX_Control(pDX, IDC_ED2K_SETUP, m_wndEDSetup);
	DDX_Control(pDX, IDC_DC_SETUP, m_wndDCSetup);
	DDX_Check(pDX, IDC_G2_ENABLE, m_bG2Enable);
	DDX_Check(pDX, IDC_G1_ENABLE, m_bG1Enable);
	DDX_Check(pDX, IDC_ED2K_ENABLE, m_bEDEnable);
	DDX_Check(pDX, IDC_DC_ENABLE, m_bDCEnable);
}

/////////////////////////////////////////////////////////////////////////////
// CNetworksSettingsPage message handlers

BOOL CNetworksSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bG2Enable = TRUE;	//Settings.Gnutella2.ShowInterface;
	m_bG1Enable = Settings.Gnutella1.ShowInterface;
	m_bEDEnable = Settings.eDonkey.ShowInterface;
	m_bDCEnable = Settings.DC.ShowInterface;

	//m_wndG2Setup.EnableWindow( m_bG2Enable );
	m_wndG1Setup.EnableWindow( m_bG1Enable );
	m_wndEDSetup.EnableWindow( m_bEDEnable );
	m_wndDCSetup.EnableWindow( m_bDCEnable );

	UpdateData( FALSE );

	if ( Settings.Experimental.LAN_Mode )	// #ifdef LAN_MODE
	{
		m_bG1Enable = FALSE;
		m_bEDEnable = FALSE;
		m_bDCEnable = FALSE;
		GetDlgItem( IDC_G2_ENABLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_G1_ENABLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_ED2K_ENABLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_DC_ENABLE )->EnableWindow( FALSE );
		m_wndG2Setup.EnableWindow( FALSE );
		m_wndG1Setup.EnableWindow( FALSE );
		m_wndEDSetup.EnableWindow( FALSE );
		m_wndDCSetup.EnableWindow( FALSE );
	}

	return TRUE;
}

HBRUSH CNetworksSettingsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSettingsPage::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( pWnd == &m_wndG2Setup || pWnd == &m_wndG1Setup || pWnd == &m_wndEDSetup || pWnd == &m_wndDCSetup )
	{
		if ( pWnd->IsWindowEnabled() )
		{
			pDC->SetTextColor( Colors.m_crTextLink );
			pDC->SelectObject( &theApp.m_gdiFontLine );
		}
	}

	return hbr;
}

BOOL CNetworksSettingsPage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CWnd* pLinks[] = { &m_wndG2Setup, &m_wndG1Setup, &m_wndEDSetup, &m_wndDCSetup, NULL };

	CRect rc;
	CPoint point;
	GetCursorPos( &point );

	for ( int nLink = 0 ; pLinks[ nLink ] ; nLink++ )
	{
		pLinks[ nLink ]->GetWindowRect( &rc );

		if ( pLinks[ nLink ]->IsWindowEnabled() && rc.PtInRect( point ) )
		{
			SetCursor( theApp.LoadCursor( IDC_HAND ) );
			return TRUE;
		}
	}

	return CSettingsPage::OnSetCursor( pWnd, nHitTest, message );
}

void CNetworksSettingsPage::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rc;

	ClientToScreen( &point );

	m_wndG2Setup.GetWindowRect( &rc );
	if ( rc.PtInRect( point ) )
	{
		if ( m_wndG2Setup.IsWindowEnabled() )
			GetSheet()->SetActivePage( GetSheet()->GetPage( RUNTIME_CLASS(CGnutellaSettingsPage) ) );
	}
	else
	{
		m_wndG1Setup.GetWindowRect( &rc );
		if ( rc.PtInRect( point ) )
		{
			if ( m_wndG1Setup.IsWindowEnabled() )
				GetSheet()->SetActivePage( GetSheet()->GetPage( RUNTIME_CLASS(CGnutellaSettingsPage) ) );
		}
		else
		{
			m_wndEDSetup.GetWindowRect( &rc );
			if ( rc.PtInRect( point ) )
			{
				if ( m_wndEDSetup.IsWindowEnabled() )
					GetSheet()->SetActivePage( GetSheet()->GetPage( RUNTIME_CLASS(CDonkeySettingsPage) ) );
			}
			else
			{
				m_wndDCSetup.GetWindowRect( &rc );
				if ( rc.PtInRect( point ) )
				{
					if ( m_wndDCSetup.IsWindowEnabled() )
						GetSheet()->SetActivePage( GetSheet()->GetPage( RUNTIME_CLASS(CDCSettingsPage) ) );
				}
			}
		}
	}

	CSettingsPage::OnLButtonUp( nFlags, point );
}

BOOL CNetworksSettingsPage::OnSetActive()
{
//	CGnutellaSettingsPage* ppGnutella =
//		(CGnutellaSettingsPage*)GetPage( RUNTIME_CLASS(CGnutellaSettingsPage) );
//
//	if ( ppGnutella->GetSafeHwnd() != NULL )
//	{
//		ppGnutella->UpdateData();
//		m_bG2Enable = ppGnutella->m_bG2Today;
//		m_bG1Enable = ppGnutella->m_bG1Today;
//	}

//	CDonkeySettingsPage* ppDonkey =
//		(CDonkeySettingsPage*)GetPage( RUNTIME_CLASS(CDonkeySettingsPage) );
//
//	if ( ppDonkey->GetSafeHwnd() != NULL )
//	{
//		ppDonkey->UpdateData();
//		m_bEDEnable = ppDonkey->m_bEnabled;
//	}

//	CDCSettingsPage* ppDC =
//		(CDCSettingsPage*)GetPage( RUNTIME_CLASS(CDCSettingsPage) );
//
//	if ( ppDC->GetSafeHwnd() != NULL )
//	{
//		ppDC->UpdateData();
//		m_bDCEnable = ppDC->m_bEnabled;
//	}

	UpdateData( FALSE );
	//m_wndG2Setup.EnableWindow( m_bG2Enable );
	//m_wndG1Setup.EnableWindow( m_bG1Enable );
	//m_wndEDSetup.EnableWindow( m_bEDEnable );
	//m_wndDCSetup.EnableWindow( m_bDCEnable );

	return CSettingsPage::OnSetActive();
}

void CNetworksSettingsPage::OnG2Enable()
{
	UpdateData();

	if ( ! m_bG2Enable &&
		 MsgBox( IDS_NETWORK_DISABLE_G2, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
	{
		m_bG2Enable = TRUE;
		UpdateData( FALSE );
	}

//	CGnutellaSettingsPage* ppGnutella =
//		(CGnutellaSettingsPage*)GetPage( RUNTIME_CLASS(CGnutellaSettingsPage) );
//
//	if ( ppGnutella->GetSafeHwnd() != NULL )
//	{
//		ppGnutella->UpdateData( TRUE );
//		ppGnutella->m_bG2Today = m_bG2Enable;
//		ppGnutella->UpdateData( FALSE );
//	}

	m_wndG2Setup.EnableWindow( m_bG2Enable );
}

void CNetworksSettingsPage::OnG1Enable()
{
	UpdateData();

	if ( m_bG1Enable && Settings.GetOutgoingBandwidth() < 2 )
	{
		MsgBox( IDS_NETWORK_BANDWIDTH_LOW, MB_OK );
		m_bG1Enable = FALSE;
		UpdateData( FALSE );
	}

//	CGnutellaSettingsPage* ppGnutella =
//		(CGnutellaSettingsPage*)GetPage( RUNTIME_CLASS(CGnutellaSettingsPage) );
//
//	if ( ppGnutella->GetSafeHwnd() != NULL )
//	{
//		ppGnutella->UpdateData( TRUE );
//		ppGnutella->m_bG1Today = m_bG1Enable;
//		ppGnutella->UpdateData( FALSE );
//	}

	m_wndG1Setup.EnableWindow( m_bG1Enable );
}

void CNetworksSettingsPage::OnEd2kEnable()
{
	UpdateData();

	if ( m_bEDEnable && Settings.GetOutgoingBandwidth() < 2 )
	{
		MsgBox( IDS_NETWORK_BANDWIDTH_LOW, MB_OK );
		m_bEDEnable = FALSE;
		UpdateData( FALSE );
	}

//	CDonkeySettingsPage* ppDonkey =
//		(CDonkeySettingsPage*)GetPage( RUNTIME_CLASS(CDonkeySettingsPage) );
//
//	if ( ppDonkey->GetSafeHwnd() != NULL )
//	{
//		ppDonkey->UpdateData( TRUE );
//		ppDonkey->m_bEnabled = m_bEDEnable;
//		ppDonkey->UpdateData( FALSE );
//	}

	m_wndEDSetup.EnableWindow( m_bEDEnable );
}

void CNetworksSettingsPage::OnDCEnable()
{
	UpdateData();

	if ( m_bDCEnable && Settings.GetOutgoingBandwidth() < 2 )
	{
		MsgBox( IDS_NETWORK_BANDWIDTH_LOW, MB_OK );
		m_bDCEnable = FALSE;
		UpdateData( FALSE );
	}

//	CDCSettingsPage* ppDC =
//		(CDCSettingsPage*)GetPage( RUNTIME_CLASS(CDCSettingsPage) );
//
//	if ( ppDC->GetSafeHwnd() != NULL )
//	{
//		ppDC->UpdateData( TRUE );
//		ppDC->m_bEnabled = m_bDCEnable;
//		ppDC->UpdateData( FALSE );
//	}

	m_wndDCSetup.EnableWindow( m_bDCEnable );
}

BOOL CNetworksSettingsPage::OnApply()
{
	UpdateData( TRUE );

	Update();

	return CSettingsPage::OnApply();
}

void CNetworksSettingsPage::OnOK()
{
	UpdateData( TRUE );

	Update();

	CSettingsPage::OnOK();
}

void CNetworksSettingsPage::Update()
{
	if ( (BOOL)Settings.Gnutella1.ShowInterface == m_bG1Enable &&
		 (BOOL)Settings.eDonkey.ShowInterface	== m_bEDEnable &&
		 (BOOL)Settings.DC.ShowInterface		== m_bDCEnable )
		return;

	//Settings.Gnutella2.ShowInterface	= m_bG2Enable != FALSE;
	Settings.Gnutella1.ShowInterface	= m_bG1Enable != FALSE;
	Settings.eDonkey.ShowInterface		= m_bEDEnable != FALSE;
	Settings.DC.ShowInterface			= m_bDCEnable != FALSE;

	// Update Interface
	PostMainWndMessage( WM_SKINCHANGED );
}
