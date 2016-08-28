//
// DlgQueueProperties.cpp
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "DlgQueueProperties.h"
#include "LiveList.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CQueuePropertiesDlg, CSkinDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_QUEUE_PARTIALONLY, OnPartialClicked)
	ON_BN_CLICKED(IDC_QUEUE_LIBRARYONLY, OnLibraryClicked)
	ON_BN_CLICKED(IDC_QUEUE_BOTH, OnBothClicked)
	ON_BN_CLICKED(IDC_MINIMUM_CHECK, OnMinimumCheck)
	ON_BN_CLICKED(IDC_MAXIMUM_CHECK, OnMaximumCheck)
	ON_BN_CLICKED(IDC_PROTOCOLS_CHECK, OnProtocolsCheck)
	ON_BN_CLICKED(IDC_MARKED_CHECK, OnMarkedCheck)
	ON_BN_CLICKED(IDC_ROTATE_ENABLE, OnRotateEnable)
	ON_BN_CLICKED(IDC_MATCH_CHECK, OnMatchCheck)
	ON_EN_CHANGE(IDC_TRANSFERS_MAX, OnChangeTransfersMax)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CQueuePropertiesDlg dialog

CQueuePropertiesDlg::CQueuePropertiesDlg(CUploadQueue* pQueue, BOOL bEnable, CWnd* pParent)
	: CSkinDialog(CQueuePropertiesDlg::IDD, pParent)
	, m_nCapacity	( 0 )
	, m_bMaxSize	( FALSE )
	, m_bMinSize	( FALSE )
	, m_bMatch		( FALSE )
	, m_bMarked 	( FALSE )
	, m_bEnable 	( FALSE )
	, m_bProtocols	( FALSE )
	, m_bReward 	( FALSE )
	, m_bRotate 	( FALSE )
	, m_nRotateTime	( 0 )
	, m_nTransfersMin ( 0 )
	, m_nTransfersMax ( 0 )
{
	ASSERT( pQueue != NULL );
	m_pQueue = pQueue;
	m_bEnableOverride = bEnable;
	m_nFileStatusFlag = CUploadQueue::ulqBoth;
}

void CQueuePropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_QUEUE_PARTIALONLY, m_wndPartialOnly);
	DDX_Control(pDX, IDC_QUEUE_LIBRARYONLY, m_wndLibraryOnly);
	DDX_Control(pDX, IDC_QUEUE_BOTH, m_wndBoth);
	DDX_Control(pDX, IDC_MATCH_TEXT, m_wndMatch);
	DDX_Control(pDX, IDC_BANDWIDTH_POINTS, m_wndBandwidthPoints);
	DDX_Control(pDX, IDC_BANDWIDTH_VALUE, m_wndBandwidthValue);
	DDX_Control(pDX, IDC_TRANSFERS_MIN_SPIN, m_wndTransfersMin);
	DDX_Control(pDX, IDC_TRANSFERS_MAX_SPIN, m_wndTransfersMax);
	DDX_Control(pDX, IDC_ROTATE_TIME_SPIN, m_wndRotateTimeSpin);
	DDX_Control(pDX, IDC_ROTATE_TIME, m_wndRotateTime);
	DDX_Control(pDX, IDC_PROTOCOLS_LIST, m_wndProtocols);
	DDX_Control(pDX, IDC_MINIMUM_VALUE, m_wndMinSize);
	DDX_Control(pDX, IDC_MAXIMUM_VALUE, m_wndMaxSize);
	DDX_Control(pDX, IDC_MARKED_LIST, m_wndMarked);
	DDX_Control(pDX, IDC_CAPACITY_SPIN, m_wndCapacity);
	DDX_Control(pDX, IDC_BANDWIDTH_SLIDER, m_wndBandwidthSlider);
	DDX_Text(pDX, IDC_CAPACITY, m_nCapacity);
	DDX_Check(pDX, IDC_MAXIMUM_CHECK, m_bMaxSize);
	DDX_Text(pDX, IDC_MAXIMUM_VALUE, m_sMaxSize);
	DDX_Check(pDX, IDC_MINIMUM_CHECK, m_bMinSize);
	DDX_Text(pDX, IDC_MINIMUM_VALUE, m_sMinSize);
	DDX_Check(pDX, IDC_MARKED_CHECK, m_bMarked);
	DDX_Text(pDX, IDC_NAME, m_sName);
	DDX_Check(pDX, IDC_PROTOCOLS_CHECK, m_bProtocols);
	DDX_Check(pDX, IDC_ROTATE_ENABLE, m_bRotate);
	DDX_Text(pDX, IDC_ROTATE_TIME, m_nRotateTime);
	DDX_Text(pDX, IDC_TRANSFERS_MAX, m_nTransfersMax);
	DDX_Text(pDX, IDC_TRANSFERS_MIN, m_nTransfersMin);
	DDX_Check(pDX, IDC_REWARD_ENABLE, m_bReward);
	DDX_Check(pDX, IDC_MATCH_CHECK, m_bMatch);
	DDX_Text(pDX, IDC_MATCH_TEXT, m_sMatch);
	DDX_Check(pDX, IDC_ENABLE, m_bEnable);
	DDX_CBString(pDX, IDC_MARKED_LIST, m_sMarked);
}

/////////////////////////////////////////////////////////////////////////////
// CQueuePropertiesDlg message handlers

BOOL CQueuePropertiesDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CQueuePropertiesDlg", ID_VIEW_UPLOADS );

	m_wndTransfersMin.SetRange( 1, 128 );
	m_wndTransfersMax.SetRange( 1, 512 );
	m_wndRotateTimeSpin.SetRange( 30, 15 * 60 );

	CoolInterface.LoadIconsTo( m_gdiProtocols, protocolIDs );
	m_wndProtocols.SetImageList( &m_gdiProtocols, LVSIL_SMALL );

	m_wndProtocols.SetExtendedStyle( LVS_EX_CHECKBOXES );
	m_wndProtocols.InsertItem( LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM, 0, L"HTTP", 0, 0, PROTOCOL_HTTP, PROTOCOL_HTTP );
	m_wndProtocols.InsertItem( LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM, 1, L"ED2K", 0, 0, PROTOCOL_ED2K, PROTOCOL_ED2K );
	m_wndProtocols.InsertItem( LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM, 2, L"DC++", 0, 0, PROTOCOL_DC, PROTOCOL_DC );
	m_wndProtocols.InsertItem( LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM, 3, L"BitTorrent", 0, 0, PROTOCOL_BT, PROTOCOL_BT );

	CSingleLock pLock( &UploadQueues.m_pSection, TRUE );

	if ( ! UploadQueues.Check( m_pQueue ) )
	{
		PostMessage( WM_CLOSE );
		return TRUE;
	}

	m_sName = m_pQueue->m_sName;

	m_nFileStatusFlag = m_pQueue->m_nFileStateFlag;

	if ( m_nFileStatusFlag == CUploadQueue::ulqPartial )
	{
		m_wndPartialOnly.SetCheck(BST_CHECKED);
		m_wndLibraryOnly.SetCheck(BST_UNCHECKED);
		m_wndBoth.SetCheck(BST_UNCHECKED);
	}
	else if ( m_nFileStatusFlag == CUploadQueue::ulqLibrary )
	{
		m_wndPartialOnly.SetCheck(BST_UNCHECKED);
		m_wndLibraryOnly.SetCheck(BST_CHECKED);
		m_wndBoth.SetCheck(BST_UNCHECKED);
	}
	else
	{
		m_wndPartialOnly.SetCheck(BST_UNCHECKED);
		m_wndLibraryOnly.SetCheck(BST_UNCHECKED);
		m_wndBoth.SetCheck(BST_CHECKED);
	}

	m_bMinSize = m_pQueue->m_nMinSize > 0;
	m_sMinSize = Settings.SmartVolume( m_bMinSize ? m_pQueue->m_nMinSize : 0 );

	m_bMaxSize = m_pQueue->m_nMaxSize < SIZE_UNKNOWN;
	m_sMaxSize = Settings.SmartVolume( m_bMaxSize ? m_pQueue->m_nMaxSize : 0 );

	m_bMarked = ( ! m_pQueue->m_sShareTag.IsEmpty() );
	m_sMarked = m_pQueue->m_sShareTag;

	m_bMatch = ( ! m_pQueue->m_sNameMatch.IsEmpty() );
	m_sMatch = m_pQueue->m_sNameMatch;

	m_bProtocols = ( m_pQueue->m_nProtocols != 0 );

	if ( ! m_bProtocols || ( m_pQueue->m_nProtocols & (1<<PROTOCOL_HTTP) ) )
		m_wndProtocols.SetItemState( 0, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK );
	if ( ! m_bProtocols || ( m_pQueue->m_nProtocols & (1<<PROTOCOL_ED2K) ) )
		m_wndProtocols.SetItemState( 1, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK );
	if ( ! m_bProtocols || ( m_pQueue->m_nProtocols & (1<<PROTOCOL_DC) ) )
		m_wndProtocols.SetItemState( 2, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK );
	if ( ! m_bProtocols || ( m_pQueue->m_nProtocols & (1<<PROTOCOL_BT) ) )
		m_wndProtocols.SetItemState( 3, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK );

	m_bEnable		= m_pQueue->m_bEnable || m_bEnableOverride;

	m_nCapacity		= max( m_pQueue->m_nCapacity, m_pQueue->m_nMaxTransfers );
	m_nTransfersMin	= m_pQueue->m_nMinTransfers;
	m_nTransfersMax	= m_pQueue->m_nMaxTransfers;

	m_bRotate		= m_pQueue->m_bRotate;
	m_nRotateTime	= m_pQueue->m_nRotateTime;

	m_bReward		= m_pQueue->m_bRewardUploaders;

	//DWORD nTotal = Settings.Connection.OutSpeed * 1024 / 8;
	//DWORD nLimit = Settings.Bandwidth.Uploads;
	//if ( nLimit == 0 || nLimit > nTotal ) nLimit = nTotal;

	int nOtherPoints = (int)UploadQueues.GetTotalBandwidthPoints( !( m_pQueue->m_nProtocols & (1<<PROTOCOL_ED2K) ) )
						- (int)m_pQueue->m_nBandwidthPoints;
	if ( nOtherPoints < 0 ) nOtherPoints = 0;

	m_wndBandwidthSlider.SetRange( 1, max( 100, nOtherPoints * 3 ) );
	m_wndBandwidthSlider.SetPos( m_pQueue->m_nBandwidthPoints );

	UpdateData( FALSE );

	m_wndPartialOnly.EnableWindow(TRUE);
	m_wndLibraryOnly.EnableWindow(TRUE);
	m_wndBoth.EnableWindow(TRUE);

	m_wndMinSize.EnableWindow( m_bMinSize );
	m_wndMaxSize.EnableWindow( m_bMaxSize );
	m_wndMarked.EnableWindow( m_bMarked );
	m_wndMatch.EnableWindow( m_bMatch );
	m_wndProtocols.EnableWindow( m_bProtocols );
	m_wndRotateTime.EnableWindow( m_bRotate );
	m_wndRotateTimeSpin.EnableWindow( m_bRotate );
	m_wndCapacity.SetRange32( static_cast< int >( m_nTransfersMax ), 4096 );
	OnHScroll( 0, 0, NULL );

	if ( Settings.General.GUIMode == GUI_BASIC &&
		 !( Settings.eDonkey.EnableAlways | Settings.eDonkey.Enabled ) )
	{
		m_bProtocols = FALSE;
		m_wndProtocols.EnableWindow( FALSE );
		m_wndProtocols.ShowWindow( FALSE );

		(GetProtocolCheckbox())->EnableWindow( FALSE );
		(GetProtocolCheckbox())->ShowWindow( FALSE );
	}

	return TRUE;
}

void CQueuePropertiesDlg::OnMinimumCheck()
{
	UpdateData();
	m_wndMinSize.EnableWindow( m_bMinSize );
}

void CQueuePropertiesDlg::OnMaximumCheck()
{
	UpdateData();
	m_wndMaxSize.EnableWindow( m_bMaxSize );
}

void CQueuePropertiesDlg::OnMarkedCheck()
{
	UpdateData();
	m_wndMarked.EnableWindow( m_bMarked );
}

void CQueuePropertiesDlg::OnMatchCheck()
{
	UpdateData();
	m_wndMatch.EnableWindow( m_bMatch );
}

void CQueuePropertiesDlg::OnProtocolsCheck()
{
	if ( Settings.General.GUIMode == GUI_BASIC &&
		 !( Settings.eDonkey.EnableAlways | Settings.eDonkey.Enabled ) )
		return;

	UpdateData();
	m_wndProtocols.EnableWindow( m_bProtocols );
}

void CQueuePropertiesDlg::OnChangeTransfersMax()
{
	if ( m_wndBandwidthValue.m_hWnd != NULL )
	{
		UpdateData();
		if ( m_nTransfersMax > m_nCapacity)
			m_nCapacity = m_nTransfersMax;
		m_wndCapacity.SetRange( short( m_nTransfersMax ), 1024 );
		UpdateData( FALSE );
	}
}

void CQueuePropertiesDlg::OnRotateEnable()
{
	UpdateData();
	m_wndRotateTime.EnableWindow( m_bRotate );
	m_wndRotateTimeSpin.EnableWindow( m_bRotate );
}

void CQueuePropertiesDlg::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* /*pScrollBar*/)
{
	DWORD nTotal = Settings.Connection.OutSpeed * 1024 / 8;
	DWORD nLimit = Settings.Bandwidth.Uploads;

	if ( nLimit == 0 || nLimit > nTotal )
		nLimit = nTotal;

	int nOtherPoints = (int)UploadQueues.GetTotalBandwidthPoints( !( m_pQueue->m_nProtocols & (1<<PROTOCOL_ED2K) ) )
					 - (int)m_pQueue->m_nBandwidthPoints;
	if ( nOtherPoints < 0 )
		nOtherPoints = 0;

	int nLocalPoints = m_wndBandwidthSlider.GetPos();
	int nTotalPoints = nLocalPoints + nOtherPoints;

	DWORD nBandwidth = nLimit * nLocalPoints / max( 1, nTotalPoints );

	CString str;
	str.Format( L"%u%% (%i/%i)", ( 100 * nBandwidth ) / nLimit, nLocalPoints, nTotalPoints );

	m_wndBandwidthPoints.SetWindowText( str );
	m_wndBandwidthValue.SetWindowText( Settings.SmartSpeed( nBandwidth ) + L'+' );
}

void CQueuePropertiesDlg::OnOK()
{
	UpdateData();

	CSingleLock pLock( &UploadQueues.m_pSection, TRUE );

	if ( ! UploadQueues.Check( m_pQueue ) )
	{
		CSkinDialog::OnCancel();
		return;
	}

	m_pQueue->m_sName = m_sName;

	if ( m_wndBoth.GetCheck() )			m_nFileStatusFlag = (CUploadQueue::ulqBoth);
	if ( m_wndLibraryOnly.GetCheck() )	m_nFileStatusFlag = (CUploadQueue::ulqLibrary);
	if ( m_wndPartialOnly.GetCheck() )	m_nFileStatusFlag = (CUploadQueue::ulqPartial);
	m_pQueue->m_nFileStateFlag = ( m_nFileStatusFlag != CUploadQueue::ulqNull ) ? m_nFileStatusFlag : (CUploadQueue::ulqBoth);

	if ( m_bMaxSize )
	{
		m_pQueue->m_nMaxSize = Settings.ParseVolume( m_sMaxSize );
		if ( m_pQueue->m_nMaxSize == 0 )
			m_pQueue->m_nMaxSize = SIZE_UNKNOWN;
	}
	else
	{
		m_pQueue->m_nMaxSize = SIZE_UNKNOWN;
	}

	if ( m_bMinSize )
		m_pQueue->m_nMinSize = Settings.ParseVolume( m_sMinSize );
	else
		m_pQueue->m_nMinSize = 0;

	if ( m_bMarked )
		m_pQueue->m_sShareTag = m_sMarked;
	else
		m_pQueue->m_sShareTag.Empty();

	if ( m_bMatch )
		m_pQueue->m_sNameMatch = m_sMatch;
	else
		m_pQueue->m_sNameMatch.Empty();

	m_pQueue->m_nProtocols = 0;

	if ( m_bProtocols )
	{
		if ( m_wndProtocols.GetItemState( 0, LVIS_STATEIMAGEMASK ) == INDEXTOSTATEIMAGEMASK(2) )
			m_pQueue->m_nProtocols |= (1<<PROTOCOL_HTTP);
		if ( m_wndProtocols.GetItemState( 1, LVIS_STATEIMAGEMASK ) == INDEXTOSTATEIMAGEMASK(2) )
			m_pQueue->m_nProtocols |= (1<<PROTOCOL_ED2K);
		if ( m_wndProtocols.GetItemState( 2, LVIS_STATEIMAGEMASK ) == INDEXTOSTATEIMAGEMASK(2) )
			m_pQueue->m_nProtocols |= (1<<PROTOCOL_DC);
		if ( m_wndProtocols.GetItemState( 3, LVIS_STATEIMAGEMASK ) == INDEXTOSTATEIMAGEMASK(2) )
			m_pQueue->m_nProtocols |= (1<<PROTOCOL_BT);

		if ( m_pQueue->m_nProtocols == ( (1<<PROTOCOL_HTTP)|(1<<PROTOCOL_ED2K)|(1<<PROTOCOL_DC)|(1<<PROTOCOL_BT) ) )
			m_pQueue->m_nProtocols = 0;
	}

	m_pQueue->m_nCapacity		= min( (int)m_nCapacity, ( m_pQueue->m_nProtocols & (1<<PROTOCOL_ED2K) ) ? 4096 : 64 );

	m_pQueue->m_bEnable			= m_bEnable;
	//m_pQueue->m_nMinTransfers	= max( (DWORD)m_nTransfersMin, 1 );
	//m_pQueue->m_nMaxTransfers	= (int)max( m_nTransfersMin, m_nTransfersMax );		// INT_PTR use below

	if ( m_nTransfersMin > 1 )
		IntPtrToDWord( m_nTransfersMin, &m_pQueue->m_nMinTransfers );
	else
		m_pQueue->m_nMinTransfers = 1;

	if ( m_nTransfersMax > m_nTransfersMin )
		IntPtrToDWord( m_nTransfersMax, &m_pQueue->m_nMaxTransfers );
	else
		m_pQueue->m_nMaxTransfers =  m_pQueue->m_nMinTransfers;

	m_pQueue->m_bRotate			= m_bRotate;
	m_pQueue->m_nRotateTime		= max( m_nRotateTime, 30 );

	m_pQueue->m_nBandwidthPoints = m_wndBandwidthSlider.GetPos();

	m_pQueue->m_bRewardUploaders = m_bReward;

	CSkinDialog::OnOK();
}

void CQueuePropertiesDlg::OnPartialClicked()
{
	m_wndPartialOnly.SetCheck(BST_CHECKED);
	m_wndLibraryOnly.SetCheck(BST_UNCHECKED);
	m_wndBoth.SetCheck(BST_UNCHECKED);
}

void CQueuePropertiesDlg::OnLibraryClicked()
{
	m_wndPartialOnly.SetCheck(BST_UNCHECKED);
	m_wndLibraryOnly.SetCheck(BST_CHECKED);
	m_wndBoth.SetCheck(BST_UNCHECKED);
}

void CQueuePropertiesDlg::OnBothClicked()
{
	m_wndPartialOnly.SetCheck(BST_UNCHECKED);
	m_wndLibraryOnly.SetCheck(BST_UNCHECKED);
	m_wndBoth.SetCheck(BST_CHECKED);
}
