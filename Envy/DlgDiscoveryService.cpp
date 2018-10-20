//
// DlgDiscoveryService.cpp
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
#include "DlgDiscoveryService.h"
#include "DiscoveryServices.h"
#include "Network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CDiscoveryServiceDlg, CSkinDialog)
	ON_EN_CHANGE(IDC_ADDRESS, OnChangeAddress)
	ON_CBN_SELCHANGE(IDC_SERVICE_TYPE, OnSelChangeServiceType)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDiscoveryServiceDlg dialog

CDiscoveryServiceDlg::CDiscoveryServiceDlg(CWnd* pParent, CDiscoveryService* pService)
	: CSkinDialog(CDiscoveryServiceDlg::IDD, pParent)
	, m_pService	( pService )
	, m_nType 		( -1 )
	, m_bNew		( FALSE )
{
}

CDiscoveryServiceDlg::~CDiscoveryServiceDlg()
{
	if ( m_pService && m_bNew ) delete m_pService;
}

void CDiscoveryServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Text(pDX, IDC_ADDRESS, m_sAddress);
	DDX_CBIndex(pDX, IDC_SERVICE_TYPE, m_nType);
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryServiceDlg message handlers

BOOL CDiscoveryServiceDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CDiscoveryServiceDlg", IDR_DISCOVERYFRAME );

	CSingleLock pLock( &Network.m_pSection, TRUE );

	m_bNew = ! DiscoveryServices.Check( m_pService );
	if ( m_bNew ) m_pService = new CDiscoveryService();

	m_sAddress	= m_pService->m_sAddress;
	m_nType		= m_pService->m_nType - 1;

	// Reassigning the combo-box placeholder:
	if ( m_nType == 1 )
	{
		if ( m_pService->m_bGnutella1 && m_pService->m_bGnutella2 )
			m_nType = 3;
		else if ( m_pService->m_bGnutella2 )
			m_nType = 2;
	}
	else if ( m_nType > 1 )
	{
		m_nType += 2;
	}

	if ( m_bNew )
		m_nType = 1;

	pLock.Unlock();

	UpdateData( FALSE );

	OnChangeAddress();

	return TRUE;
}

void CDiscoveryServiceDlg::OnChangeAddress()
{
	UpdateData();

	m_wndOK.EnableWindow( m_nType ?
		_tcsncmp( m_sAddress, L"http://", 7 ) == 0 :
		_tcschr( m_sAddress, '.' ) != NULL );
}

void CDiscoveryServiceDlg::OnSelChangeServiceType()
{
	OnChangeAddress();
}

void CDiscoveryServiceDlg::OnOK()
{
	UpdateData();

	CSingleLock pLock( &Network.m_pSection, TRUE );

	if ( ! m_bNew && ! DiscoveryServices.Check( m_pService ) )
		m_pService = new CDiscoveryService();

	m_pService->m_sAddress = m_sAddress;

	switch ( m_nType )
	{
	case 0:
		m_pService->m_nType = CDiscoveryService::dsGnutella;
		break;
	case 1:
		m_pService->m_nType = CDiscoveryService::dsWebCache;
		m_pService->m_bGnutella1 = TRUE;
		m_pService->m_bGnutella2 = FALSE;
		m_pService->m_nProtocolID = PROTOCOL_G1;
		break;
	case 2:
		m_pService->m_nType = CDiscoveryService::dsWebCache;
		m_pService->m_bGnutella1 = FALSE;
		m_pService->m_bGnutella2 = TRUE;
		m_pService->m_nProtocolID = PROTOCOL_G2;
		break;
	case 3:
		m_pService->m_nType = CDiscoveryService::dsWebCache;
		m_pService->m_bGnutella1 = TRUE;
		m_pService->m_bGnutella2 = TRUE;
		m_pService->m_nProtocolID = PROTOCOL_ANY;
		break;
	case 4:
		m_pService->m_nType = CDiscoveryService::dsServerList;
		m_pService->m_nProtocolID =
			( m_sAddress.Find( L"hublist", 6 ) > 0 || m_sAddress.Find( L".xml.bz2", 8 ) > 0 ) ?
			PROTOCOL_DC : PROTOCOL_ED2K;
		break;
	default:
		m_pService->m_nType = CDiscoveryService::dsBlocked;
		m_pService->m_nProtocolID = PROTOCOL_NULL;
	}

	if ( m_pService->m_nType == CDiscoveryService::dsGnutella )
	{
		if ( StartsWith( m_sAddress, _P( L"gnutella2:host:" ) ) ||
			 StartsWith( m_sAddress, _P( L"g2:host:" ) ) )
		{
			m_pService->m_bGnutella2	= TRUE;
			m_pService->m_bGnutella1	= FALSE;
			m_pService->m_nProtocolID	= PROTOCOL_G2;
			m_pService->m_nSubType		= CDiscoveryService::dsGnutella2TCP;
		}
		else if ( StartsWith( m_sAddress, _P( L"gnutella1:host:" ) ) ||
			 StartsWith( m_sAddress, _P( L"gnutella:host:" ) ) )
		{
			m_pService->m_bGnutella2	= FALSE;
			m_pService->m_bGnutella1	= TRUE;
			m_pService->m_nProtocolID	= PROTOCOL_G1;
			m_pService->m_nSubType		= CDiscoveryService::dsGnutellaTCP;
		}
		else if ( StartsWith( m_sAddress, _P( L"uhc:" ) ) )
		{
			m_pService->m_bGnutella2	= FALSE;
			m_pService->m_bGnutella1	= TRUE;
			m_pService->m_nProtocolID	= PROTOCOL_G1;
			m_pService->m_nSubType		= CDiscoveryService::dsGnutellaUDPHC;
		}
		else if ( StartsWith( m_sAddress, _P( L"ukhl:" ) ) )
		{
			m_pService->m_bGnutella2	= TRUE;
			m_pService->m_bGnutella1	= FALSE;
			m_pService->m_nProtocolID	= PROTOCOL_G2;
			m_pService->m_nSubType		= CDiscoveryService::dsGnutella2UDPKHL;
		}
	}

	DiscoveryServices.Add( m_pService );
	m_pService = NULL;

	pLock.Unlock();

	CSkinDialog::OnOK();
}
