//
// DlgScheduleItem.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2010 and Shareaza 2010
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
#include "Envy.h"
#include "DlgScheduleTask.h"
#include "DlgSkinDialog.h"
#include "Scheduler.h"
#include "Settings.h"
#include "Network.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CScheduleTaskDlg, CSkinDialog)
	ON_BN_CLICKED(IDC_ONLYONCE, OnBnClickedOnlyonce)
	ON_BN_CLICKED(IDC_EVERYDAY, OnBnClickedEveryday)
	ON_BN_CLICKED(IDC_BUTTON_ALLDAYS, &CScheduleTaskDlg::OnBnClickedButtonAllDays)
	ON_CBN_SELCHANGE(IDC_EVENTTYPE, OnCbnSelchangeEventType)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE, OnDtnDateTimeChange)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME, OnDtnDateTimeChange)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CScheduleTaskDlg dialog

CScheduleTaskDlg::CScheduleTaskDlg(CWnd* pParent, CScheduleTask* pSchTask) : CSkinDialog(CScheduleTaskDlg::IDD, pParent)
{
	m_pScheduleTask = pSchTask;
	if ( m_pScheduleTask )
		m_bNew = false;
	else
		m_bNew = true;
}

CScheduleTaskDlg::~CScheduleTaskDlg()
{
	// If we are creating a new schedule item and it is created,
	// it should be already be added to scheduler list so we delete dialog's object
	if ( m_bNew && m_pScheduleTask )
		delete m_pScheduleTask;
}

void CScheduleTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TIME, m_wndTime);
	DDX_Control(pDX, IDC_DATE, m_wndDate);
	DDX_Control(pDX, IDC_ACTIVE, m_wndActiveCheck);
	DDX_Control(pDX, IDC_EVENTTYPE, m_wndTypeSel);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_NETWORKS, m_wndLimitedCheck);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN_DOWN, m_wndSpinDown);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN_UP, m_wndSpinUp);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_DOWN, m_wndLimitedEditDown);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_UP, m_wndLimitedEditUp);
	DDX_Control(pDX, IDC_STATIC_LIMITED_DOWN, m_wndLimitedStaticDown);
	DDX_Control(pDX, IDC_STATIC_LIMITED_UP, m_wndLimitedStaticUp);
	DDX_Control(pDX, IDC_EVERYDAY, m_wndRadioEveryDay);
	DDX_Control(pDX, IDC_ONLYONCE, m_wndRadioOnce);
	DDX_Control(pDX, IDC_CHECK_MON, m_wndChkDayMon);
	DDX_Control(pDX, IDC_CHECK_TUES, m_wndChkDayTues);
	DDX_Control(pDX, IDC_CHECK_WED, m_wndChkDayWed);
	DDX_Control(pDX, IDC_CHECK_THU, m_wndChkDayThu);
	DDX_Control(pDX, IDC_CHECK_FRI, m_wndChkDayFri);
	DDX_Control(pDX, IDC_CHECK_SAT, m_wndChkDaySat);
	DDX_Control(pDX, IDC_CHECK_SUN, m_wndChkDaySun);
	DDX_Control(pDX, IDC_BUTTON_ALLDAYS, m_wndBtnAllDays);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_NETWORKS, m_bLimitedNetworks);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED_DOWN, m_nLimitDown);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED_UP, m_nLimitUp);
	DDX_Text(pDX, IDC_DESCRIPTION, m_sDescription);
}

/////////////////////////////////////////////////////////////////////////////
// CScheduleTaskDlg message handlers

BOOL CScheduleTaskDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CScheduleTaskDlg", IDR_SCHEDULERFRAME );

	m_wndSpinDown.SetRange( 1, 99 );
	m_wndSpinUp.SetRange( 1, 101 );

	// New tasks must be added to the list in the same order as Scheduler.h enum
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_FULL ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_LIMITED ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_STOP ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_SYSTEM_EXIT ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_SYSTEM_SHUTDOWN ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_SYSTEM_DIALUP_OFF ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_SYSTEM_NOTICE ) );

	if ( m_bNew )	// We are creating new schedule task, setting default values
	{
		m_pScheduleTask		= new CScheduleTask ();
		m_nAction			= BANDWIDTH_FULL;
		m_bActive			= true;
		m_bLimitedNetworks	= FALSE;
		m_bSpecificDays		= true;
		m_nDays				= 0x7F;
		m_nLimitDown		= 50;
		m_nLimitUp			= 70;

		m_wndDate.GetTime( m_tDateAndTime );	// Simply set m_tDateTime to now
		m_tDateAndTime -= m_tDateAndTime.GetSecond();
		m_wndTime.SetTime( &m_tDateAndTime );	// Discard seconds for convenience

		m_wndActiveCheck.SetCheck( 1 );
		m_wndRadioEveryDay.SetCheck( 1 );
		m_wndRadioOnce.SetCheck( 0 );

		m_wndLimitedCheck.EnableWindow( false );
		m_wndLimitedStaticDown.EnableWindow( false );
		m_wndLimitedStaticUp.EnableWindow( false );
		m_wndLimitedEditDown.EnableWindow( false );
		m_wndLimitedEditUp.EnableWindow( false );
		m_wndSpinDown.EnableWindow( false );
		m_wndSpinUp.EnableWindow( false );
		m_wndDate.EnableWindow( false );
	}
	else	// We are editing an existing schedule task, getting values from it
	{
		m_bSpecificDays	= m_pScheduleTask->m_bSpecificDays;
		m_nAction		= m_pScheduleTask->m_nAction;
		m_sDescription	= m_pScheduleTask->m_sDescription;
		m_tDateAndTime	= m_pScheduleTask->m_tScheduleDateTime;
		m_bActive		= m_pScheduleTask->m_bActive;
		m_nDays			= m_pScheduleTask->m_nDays;
		m_nLimitDown	= m_pScheduleTask->m_nLimitDown;
		m_nLimitUp		= m_pScheduleTask->m_nLimitUp;
		m_bLimitedNetworks	= m_pScheduleTask->m_bLimitedNetworks;

		if ( m_pScheduleTask->m_bExecuted && ! m_pScheduleTask->m_bSpecificDays )
			m_bActive = true;

		m_wndActiveCheck.SetCheck( m_bActive );

		switch ( m_nAction )
		{
		case 0:	// Should never happen
			m_wndTypeSel.SetCurSel(-1);
			break;
		case BANDWIDTH_FULL:
			m_wndTypeSel.SetCurSel(0);
			break;
		case BANDWIDTH_LIMITED:
			m_wndTypeSel.SetCurSel(1);
			break;
		case BANDWIDTH_STOP:
			m_wndTypeSel.SetCurSel(2);
			break;
		case SYSTEM_EXIT:
			m_wndTypeSel.SetCurSel(3);
			break;
		case SYSTEM_SHUTDOWN:
			m_wndTypeSel.SetCurSel(4);
			break;
		case SYSTEM_DISCONNECT:
			m_wndTypeSel.SetCurSel(5);
			break;
		case SYSTEM_NOTICE:
			m_wndTypeSel.SetCurSel(6);
			break;
		}

		m_wndRadioOnce.SetCheck( ! m_bSpecificDays );
		m_wndRadioEveryDay.SetCheck( m_bSpecificDays );
		m_wndDate.SetTime( &m_tDateAndTime );
		m_wndTime.SetTime( &m_tDateAndTime );

		// If task is set for specifs days disable date window
		if ( m_wndRadioEveryDay.GetCheck() )
			m_wndDate.EnableWindow( false );

		if ( m_wndTypeSel.GetCurSel() + 1 == BANDWIDTH_LIMITED )
		{
			m_wndLimitedEditDown.EnableWindow( true );
			m_wndLimitedEditUp.EnableWindow( true );
			m_wndLimitedStaticDown.EnableWindow( true );
			m_wndLimitedStaticUp.EnableWindow( true );
			m_wndSpinDown.EnableWindow( true );
			m_wndSpinUp.EnableWindow( true );
		}
		else
		{
			m_wndLimitedCheck.EnableWindow( false );
			m_wndLimitedEditDown.EnableWindow( false );
			m_wndLimitedEditUp.EnableWindow( false );
			m_wndSpinDown.EnableWindow( false );
			m_wndSpinUp.EnableWindow( false );
		}
	}

	m_wndChkDayMon.SetCheck( m_nDays & MONDAY );
	m_wndChkDayTues.SetCheck( m_nDays & TUESDAY );
	m_wndChkDayWed.SetCheck( m_nDays & WEDNESDAY );
	m_wndChkDayThu.SetCheck( m_nDays & THURSDAY );
	m_wndChkDayFri.SetCheck( m_nDays & FRIDAY );
	m_wndChkDaySat.SetCheck( m_nDays & SATURDAY );
	m_wndChkDaySun.SetCheck( m_nDays & SUNDAY );

	m_wndSpinDown.SetPos( m_nLimitDown );
	m_wndSpinUp.SetPos( m_nLimitUp );

	EnableDaysOfWeek( m_bSpecificDays );

	UpdateData( FALSE );

	return FALSE;
}

void CScheduleTaskDlg::OnOK()
{
	UpdateData( TRUE );

	// New tasks added here in same list order as Scheduler.h enum
	switch ( m_wndTypeSel.GetCurSel() )
	{
	case -1:
		MsgBox( IDS_SCHEDULER_SELECTTASK );
		return;
	case 0:
		m_nAction = BANDWIDTH_FULL;
		break;
	case 1:
		m_nAction = BANDWIDTH_LIMITED;
		break;
	case 2:
		m_nAction = BANDWIDTH_STOP;
		break;
	case 3:
		m_nAction = SYSTEM_EXIT;
		break;
	case 4:
		m_nAction = SYSTEM_SHUTDOWN;
		break;
	case 5:
		m_nAction = SYSTEM_DISCONNECT;
		break;
	case 6:
		m_nAction = SYSTEM_NOTICE;
		break;
	}

	if ( m_wndRadioOnce.GetCheck() )
	{
		m_bSpecificDays = false;

		if ( CTime::GetCurrentTime() >= m_tDateAndTime )
		{
			MsgBox( IDS_SCHEDULER_TIME_PASSED );
			return;
		}
	}
	else // Specified days selected
	{
		m_nDays = 0;
		if ( m_wndChkDaySun.GetCheck() ) m_nDays |= SUNDAY;
		if ( m_wndChkDayMon.GetCheck() ) m_nDays |= MONDAY;
		if ( m_wndChkDayTues.GetCheck() ) m_nDays |= TUESDAY;
		if ( m_wndChkDayWed.GetCheck() ) m_nDays |= WEDNESDAY;
		if ( m_wndChkDayThu.GetCheck() ) m_nDays |= THURSDAY;
		if ( m_wndChkDayFri.GetCheck() ) m_nDays |= FRIDAY;
		if ( m_wndChkDaySat.GetCheck() ) m_nDays |= SATURDAY;
		if ( ! m_nDays )
		{
			MsgBox( IDS_SCHEDULER_SELECTADAY );
			return;
		}

		m_bSpecificDays = true;
	}

	m_bActive = m_wndActiveCheck.GetCheck() ? true : false;

	m_pScheduleTask->m_nLimitDown		= m_nLimitDown;
	m_pScheduleTask->m_nLimitUp			= m_nLimitUp;
	m_pScheduleTask->m_bLimitedNetworks	= m_bLimitedNetworks != 0;
	m_pScheduleTask->m_tScheduleDateTime = m_tDateAndTime;
	m_pScheduleTask->m_bSpecificDays	= m_bSpecificDays;
	m_pScheduleTask->m_nAction			= m_nAction;
	m_pScheduleTask->m_sDescription		= m_sDescription;
	m_pScheduleTask->m_bActive			= m_bActive;
	m_pScheduleTask->m_bExecuted		= false;
	m_pScheduleTask->m_nDays			= m_nDays;

	Scheduler.Add(m_pScheduleTask);
	m_pScheduleTask = NULL;

	CSkinDialog::OnOK();
}

void CScheduleTaskDlg::OnDtnDateTimeChange(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	SYSTEMTIME tDate;
	SYSTEMTIME tTime;
	m_wndDate.GetTime( &tDate );
	m_wndTime.GetTime( &tTime );
	CTime tTemp( tDate.wYear, tDate.wMonth, tDate.wDay, tTime.wHour, tTime.wMinute, tTime.wSecond );
	m_tDateAndTime = tTemp;
	*pResult = 0;
}

void CScheduleTaskDlg::OnBnClickedOnlyonce()
{
	m_wndRadioEveryDay.SetCheck( 0 );
	m_wndDate.EnableWindow( true );
	EnableDaysOfWeek( false );

	m_bSpecificDays = false;
}

void CScheduleTaskDlg::OnBnClickedEveryday()
{
	m_wndRadioOnce.SetCheck( 0 );
	m_wndDate.EnableWindow( false );
	EnableDaysOfWeek( true );
}

void CScheduleTaskDlg::EnableDaysOfWeek(bool bEnable)
{
	m_wndChkDaySun.EnableWindow( bEnable );
	m_wndChkDayMon.EnableWindow( bEnable );
	m_wndChkDayTues.EnableWindow( bEnable );
	m_wndChkDayWed.EnableWindow( bEnable );
	m_wndChkDayThu.EnableWindow( bEnable );
	m_wndChkDayFri.EnableWindow( bEnable );
	m_wndChkDaySat.EnableWindow( bEnable );
	m_wndBtnAllDays.EnableWindow( bEnable );
}

void CScheduleTaskDlg::OnCbnSelchangeEventType()
{
	if ( m_wndTypeSel.GetCurSel() + 1 == BANDWIDTH_LIMITED )
	{
		m_wndLimitedEditDown.EnableWindow( true );
		m_wndLimitedEditUp.EnableWindow( true );
		m_wndSpinDown.EnableWindow( true );
		m_wndSpinUp.EnableWindow( true );

		m_wndLimitedCheck.EnableWindow( true );
		m_wndLimitedStaticDown.EnableWindow( true );
		m_wndLimitedStaticUp.EnableWindow( true );
	}
	else
	{
		m_wndLimitedCheck.EnableWindow( false );
		m_wndLimitedStaticDown.EnableWindow( false );
		m_wndLimitedStaticUp.EnableWindow( false );
		m_wndLimitedEditDown.EnableWindow( false );
		m_wndLimitedEditUp.EnableWindow( false );
		m_wndSpinDown.EnableWindow( false );
		m_wndSpinUp.EnableWindow( false );
	}
}

void CScheduleTaskDlg::OnBnClickedButtonAllDays()
{
	if ( m_wndChkDayMon.GetCheck() &&
		m_wndChkDayTues.GetCheck() &&
		m_wndChkDayWed.GetCheck() &&
		m_wndChkDayThu.GetCheck() &&
		m_wndChkDayFri.GetCheck() &&
		m_wndChkDaySat.GetCheck() &&
		m_wndChkDaySun.GetCheck() )
	{
		m_wndChkDayMon.SetCheck( false );
		m_wndChkDayTues.SetCheck( false );
		m_wndChkDayWed.SetCheck( false );
		m_wndChkDayThu.SetCheck( false );
		m_wndChkDayFri.SetCheck( false );
		m_wndChkDaySat.SetCheck( false );
		m_wndChkDaySun.SetCheck( false );
		return;
	}

	m_wndChkDayMon.SetCheck( true );
	m_wndChkDayTues.SetCheck( true );
	m_wndChkDayWed.SetCheck( true );
	m_wndChkDayThu.SetCheck( true );
	m_wndChkDayFri.SetCheck( true );
	m_wndChkDaySat.SetCheck( true );
	m_wndChkDaySun.SetCheck( true );
}
