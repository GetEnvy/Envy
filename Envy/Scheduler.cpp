//
// Scheduler.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2010
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
// Explanation:
// Scheduler module consists of four objects that work together to
// setup and execute user defined events. These objects are:
// ScheduleTask: Keeps type and time of execution for a particular task.
// Scheduler: Keeps list of ScheduleTasks. Iterates through tasks to see if a time has come, and sets global Settings.Scheduler.Enable if not.
// WndScheduler: Window that shows list of ScheduleTasks and enables user to add, edit, remove, import or export tasks.
// DlgScheduleTask: The dialog to create or edit a ScheduleTask.

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "Scheduler.h"
#include "Buffer.h"
#include "DlgHelp.h"
#include "Network.h"
#include "XML.h"

//#include <atltime.h>
#include <Reason.h>	// Shutdown code define
#include <Ras.h>	// Dialup disconnect

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CScheduler Scheduler;


//////////////////////////////////////////////////////////////////////
// CScheduler class construction

CScheduler::CScheduler()
{
}

CScheduler::~CScheduler()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CScheduler load and save

BOOL CScheduler::Load()
{
	const CString strFile = Settings.General.DataPath + L"Scheduler.dat";

	CQuickLock oLock( Scheduler.m_pSection );

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeRead ) )
	{
		theApp.Message( MSG_ERROR, L"Failed to open Scheduler.dat" );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::load );	// 4 KB buffer
		Serialize( ar );
		ar.Close();
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	pFile.Close();

	return TRUE;
}

BOOL CScheduler::Save()
{
	const CString strFile = Settings.General.DataPath + L"Scheduler.dat";

	CQuickLock oLock( Scheduler.m_pSection );

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
		return FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::store );	// 4 KB buffer
		try
		{
			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CScheduler serialize

// Set at INTERNAL_VERSION on change:
#define SCHEDULER_SER_VERSION 1

void CScheduler::Serialize(CArchive& ar)
{
	int nVersion = SCHEDULER_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;

		ar.WriteCount( GetCount() );		// Write the number of scheduled tasks

		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CScheduleTask *pSchTask = GetNext( pos );		// Get a pointer to each task
			pSchTask->Serialize( ar, nVersion );			// Store each task's data
		}
	}
	else // Loading
	{
		Clear();	// First clear any existing tasks

		ar >> nVersion;

		for ( int nNumTasks = ar.ReadCount() ; nNumTasks > 0 ; nNumTasks-- )	// Read the number of tasks to load
		{
			CScheduleTask *pSchTask = new CScheduleTask();	// Create a new instance of each task
			pSchTask->Serialize( ar, nVersion );			// Read each task's data
			m_pScheduleTasks.AddTail( pSchTask );			// Add the task to the task list
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CScheduleTask class construction

CScheduleTask::CScheduleTask(BOOL bCreate)
	: m_nAction			( 0 )		// Invalid value
	, m_nDays			( 0x7F ) 	// All days of week
	, m_bActive			( false )
	, m_bExecuted		( false )
	, m_bSpecificDays	( false )
	, m_tScheduleDateTime ( 0 )
{
	if ( bCreate )
		CoCreateGuid( &m_pGUID );
}

CScheduleTask::CScheduleTask(const CScheduleTask& pSchTask)
{
	*this = pSchTask;
}

CScheduleTask::~CScheduleTask()
{
}

//////////////////////////////////////////////////////////////////////
// CScheduleTask serialize

void CScheduleTask::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		// Store all task variables
		ar << m_bSpecificDays;
		ar << m_nAction;
		ar << m_sDescription;
		ar << m_tScheduleDateTime;
		ar << m_bActive;
		ar << m_bExecuted;
		ar << m_nLimitDown;
		ar << m_nLimitUp;
		ar << m_bLimitedNetworks;
		ar << m_nDays;
		ar.Write( &m_pGUID, sizeof( GUID ) );
	}
	else // Loading
	{
		// Load all task variables
		ar >> m_bSpecificDays;
		ar >> m_nAction;
		ar >> m_sDescription;
		ar >> m_tScheduleDateTime;
		ar >> m_bActive;
		ar >> m_bExecuted;
		ar >> m_nLimitDown;
		ar >> m_nLimitUp;
		ar >> m_bLimitedNetworks;
		ar >> m_nDays;
		ReadArchive( ar, &m_pGUID, sizeof( GUID ) );

		// ToDo: Support Shareaza import ?
	}
}
/////////////////////////////////////////////////////////////////////
// CSchedulerTask XML

// Task type XML keywords:
#define BANDWIDTH_FULL_TEXT			L"Bandwidth:Full"
#define BANDWIDTH_LIMITED_TEXT		L"Bandwidth:Limited"
#define BANDWIDTH_STOP_TEXT 		L"Bandwidth:Stop"
#define SYSTEM_EXIT_TEXT			L"System:Exit"
#define SYSTEM_SHUTDOWN_TEXT		L"System:Shutdown"
#define SYSTEM_DISCONNECT_TEXT		L"System:Disconnect"
#define SYSTEM_NOTICE_TEXT			L"System:Notice"
// Legacy Shareaza imports:
#define BANDWIDTH_FULL_TEXT_ALT 	L"Bandwidth - Full Speed"
#define BANDWIDTH_LIMITED_TEXT_ALT	L"Bandwidth - Reduced Speed"
#define BANDWIDTH_STOP_TEXT_ALT 	L"Bandwidth - Stop"
#define SYSTEM_EXIT_TEXT_ALT		L"System - Exit Shareaza"
#define SYSTEM_SHUTDOWN_TEXT_ALT	L"System - Shutdown"
#define SYSTEM_DISCONNECT_TEXT_ALT	L"System - Dial-Up Disconnect"

CXMLElement* CScheduleTask::ToXML()
{
	CXMLElement* pXML = new CXMLElement( NULL, L"task" );
	CString strValue;

	wchar_t szGUID[39];
	szGUID[ StringFromGUID2( *(GUID*)&m_pGUID, szGUID, 39 ) - 2 ] = 0;
	pXML->AddAttribute( L"guid", (CString)&szGUID[1] );

	strValue.Format( L"%I64i", m_tScheduleDateTime.GetTime() );
	pXML->AddAttribute( L"time", strValue );

	strValue.Format( L"%i|%i|%i|%i|%i|%i|%i",
		(m_nDays & MONDAY) != 0,
		(m_nDays & TUESDAY) != 0,
		(m_nDays & WEDNESDAY) != 0,
		(m_nDays & THURSDAY) != 0,
		(m_nDays & FRIDAY) != 0,
		(m_nDays & SATURDAY) != 0,
		(m_nDays & SUNDAY) != 0 );

	pXML->AddAttribute( L"days", strValue );

	switch ( m_nAction )
	{
	case BANDWIDTH_FULL:
		pXML->AddAttribute( L"action", BANDWIDTH_FULL_TEXT );
		break;
	case BANDWIDTH_LIMITED:
		pXML->AddAttribute( L"action", BANDWIDTH_LIMITED_TEXT );
		break;
	case BANDWIDTH_STOP:
		pXML->AddAttribute( L"action", BANDWIDTH_STOP_TEXT );
		break;
	case SYSTEM_EXIT:
		pXML->AddAttribute( L"action", SYSTEM_EXIT_TEXT );
		break;
	case SYSTEM_SHUTDOWN:
		pXML->AddAttribute( L"action", SYSTEM_SHUTDOWN_TEXT );
		break;
	case SYSTEM_DISCONNECT:
		pXML->AddAttribute( L"action", SYSTEM_DISCONNECT_TEXT );
		break;
	case SYSTEM_NOTICE:
		pXML->AddAttribute( L"action", SYSTEM_NOTICE_TEXT );
		break;
	}

	pXML->AddAttribute( L"active", m_bActive ? L"Yes" : L"No" );
	pXML->AddAttribute( L"executed", m_bExecuted ? L"Yes" : L"No" );
	pXML->AddAttribute( L"specificdays", m_bSpecificDays ? L"Yes" : L"No" );
	pXML->AddAttribute( L"limitednet", m_bLimitedNetworks ? L"Yes" : L"No" );

	strValue.Format( L"%i", m_nLimitDown );
	pXML->AddAttribute( L"limitdown", strValue );

	strValue.Format( L"%i", m_nLimitUp );
	pXML->AddAttribute( L"limitup", strValue );

	if ( ! m_sDescription.IsEmpty() )
		pXML->AddAttribute( L"comment", m_sDescription );

	return pXML;
}

BOOL CScheduleTask::FromXML(CXMLElement* pXML)
{
	BOOL bLegacy = FALSE;
	CString strValue = pXML->GetAttributeValue( L"action" );

	if ( strValue == BANDWIDTH_FULL_TEXT )
		m_nAction = BANDWIDTH_FULL;
	else if ( strValue == BANDWIDTH_LIMITED_TEXT )
		m_nAction = BANDWIDTH_LIMITED;
	else if ( strValue == BANDWIDTH_STOP_TEXT )
		m_nAction = BANDWIDTH_STOP;
	else if ( strValue == SYSTEM_EXIT_TEXT )
		m_nAction = SYSTEM_EXIT;
	else if ( strValue == SYSTEM_SHUTDOWN_TEXT )
		m_nAction = SYSTEM_SHUTDOWN;
	else if ( strValue == SYSTEM_DISCONNECT_TEXT )
		m_nAction = SYSTEM_DISCONNECT;
	else if ( strValue == SYSTEM_NOTICE_TEXT )
		m_nAction = SYSTEM_NOTICE;
	else // Shareaza Import
	{
		if ( strValue == BANDWIDTH_FULL_TEXT_ALT )
			m_nAction = BANDWIDTH_FULL;
		else if ( strValue == BANDWIDTH_LIMITED_TEXT_ALT )
			m_nAction = BANDWIDTH_LIMITED;
		else if ( strValue == BANDWIDTH_STOP_TEXT_ALT )
			m_nAction = BANDWIDTH_STOP;
		else if ( strValue == SYSTEM_EXIT_TEXT_ALT )
			m_nAction = SYSTEM_EXIT;
		else if ( strValue == SYSTEM_SHUTDOWN_TEXT_ALT )
			m_nAction = SYSTEM_SHUTDOWN;
		else if ( strValue == SYSTEM_DISCONNECT_TEXT_ALT )
			m_nAction = SYSTEM_DISCONNECT;
		else
			return FALSE;

		bLegacy = TRUE;
	}

	m_sDescription = pXML->GetAttributeValue( bLegacy ? L"description" : L"comment" );

	strValue = pXML->GetAttributeValue( L"time" );
	__time64_t tTemp;
	if ( _stscanf( strValue, L"%I64i", &tTemp ) == EOF )
		return FALSE;
	if ( tTemp > 0 )
		m_tScheduleDateTime = tTemp;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( L"active" );
	if ( strValue == L"Yes" )
		m_bActive = TRUE;
	else if ( strValue == L"No" )
		m_bActive = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( L"specificdays" );
	if ( strValue == L"Yes" )
		m_bSpecificDays = TRUE;
	else if ( strValue == L"No" )
		m_bSpecificDays = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( L"executed" );
	if ( strValue == L"Yes" )
		m_bExecuted = TRUE;
	else if ( strValue == L"No" )
		m_bExecuted = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( L"limitednet" );
	if ( strValue == L"Yes" )
		m_bLimitedNetworks = TRUE;
	else if ( strValue == L"No" )
		m_bLimitedNetworks = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( L"limitdown" );
	if ( _stscanf( strValue, L"%i", &m_nLimitDown) == EOF ) return FALSE;

	strValue = pXML->GetAttributeValue( L"limitup" );
	if ( _stscanf( strValue, L"%i", &m_nLimitUp) == EOF ) return FALSE;

	strValue = pXML->GetAttributeValue( L"days" );

	// _wtoi returns 0 on failure so bad data won't crash the application
	m_nDays = 0;
	wchar_t wcTmp;
	wcTmp = strValue[0];
	if ( _wtoi( &wcTmp ) )	m_nDays |= ( bLegacy ? SUNDAY : MONDAY );
	wcTmp = strValue[2];
	if ( _wtoi( &wcTmp ) )	m_nDays |= ( bLegacy ? MONDAY : TUESDAY );
	wcTmp = strValue[4];
	if ( _wtoi( &wcTmp ) )	m_nDays |= ( bLegacy ? TUESDAY : WEDNESDAY );
	wcTmp = strValue[6];
	if ( _wtoi( &wcTmp ) )	m_nDays |= ( bLegacy ? WEDNESDAY : THURSDAY );
	wcTmp = strValue[8];
	if ( _wtoi( &wcTmp ) )	m_nDays |= ( bLegacy ? THURSDAY : FRIDAY );
	wcTmp = strValue[10];
	if ( _wtoi( &wcTmp ) )	m_nDays |= ( bLegacy ? FRIDAY : SATURDAY );
	wcTmp = strValue[12];
	if ( _wtoi( &wcTmp ) )	m_nDays |= ( bLegacy ? SATURDAY : SUNDAY );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// GUID

CScheduleTask* CScheduler::GetGUID(const GUID& pGUID) const
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = m_pScheduleTasks.GetHeadPosition() ; pos ; )
	{
		CScheduleTask* pSchTask = m_pScheduleTasks.GetNext( pos );
		if ( pSchTask->m_pGUID == pGUID )
			return pSchTask;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////
// CScheduler item modification

void CScheduler::Add(CScheduleTask* pSchTask)
{
	CQuickLock oLock( m_pSection );

	CScheduleTask* pExistingTask = GetGUID( pSchTask->m_pGUID );
	if ( pExistingTask == NULL )
	{
		m_pScheduleTasks.AddHead( pSchTask );
	}
	else if ( pExistingTask != pSchTask )
	{
		*pExistingTask = *pSchTask;
		delete pSchTask;
	}
}

void CScheduler::Remove(CScheduleTask* pSchTask)
{
	CQuickLock oLock( m_pSection );

	POSITION pos = m_pScheduleTasks.Find( pSchTask );
	if ( pos )
		m_pScheduleTasks.RemoveAt( pos );
	delete pSchTask;
}

void CScheduler::Clear()
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pScheduleTasks.RemoveAll();
}


//////////////////////////////////////////////////////////////////////
// CScheduler schedule checking

void CScheduler::CheckSchedule()
{
	//m_tLastCheckTicks = 0;
	bool bSchedulerIsEnabled = false;
	CTime tNow = CTime::GetCurrentTime();

	// Enable this to test GetHoursTo()
	//int nHoursToDisconnect = Scheduler.GetHoursTo(BANDWIDTH_STOP|SYSTEM_DISCONNECT|SYSTEM_EXIT|SYSTEM_SHUTDOWN );
	//theApp.Message( MSG_DEBUG, L"Calculated time to disconnect is %i hours.", nHoursToDisconnect );

	CQuickLock oLock(m_pSection);
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CScheduleTask *pSchTask = GetNext( pos );

		// Check if a task should be executed
		if ( pSchTask->m_bActive )
		{
			bSchedulerIsEnabled = true;		// Always ignore deactivated tasks

			// Shorter if structure
			if ( pSchTask->m_bExecuted )
			{
				// Task is executed and active. The task is either "Only Once" or "Specific Days of Week"
				// In the first case if the date is for the days passed, its a task not executed and expired
				// In the second case it should mark as not executed so in the next CheckSchedule() call it will enter else block.
				if ( ! pSchTask->m_bSpecificDays || ( ScheduleFromToday( pSchTask ) < 0 ) )
					pSchTask->m_bExecuted = false;
			}
			else if ( IsScheduledTimePassed( pSchTask ) )
			{
				// Time is passed so task should be executed if one of two conditions is met:
				// It is scheduled for a specific date and time ("Only Once"). Checking for date.
				// Or, it is scheduled for specific days of week. Checking for day.

				if ( ( ! pSchTask->m_bSpecificDays && ScheduleFromToday( pSchTask ) == 0 ) ||
					   ( pSchTask->m_bSpecificDays && ( ( 1 << ( tNow.GetDayOfWeek() - 1 ) ) & pSchTask->m_nDays ) ) )
				{
					//static_cast<int>(pow(2.0f, tNow.GetDayOfWeek() - 1)
					// It will also mark it as executed
					ExecuteScheduledTask( pSchTask );

					// If active but not executed, scheduler will remain enabled
					bSchedulerIsEnabled = false;

					// Smart way for deactivating task if it is "Only Once"
					pSchTask->m_bActive = pSchTask->m_bSpecificDays;

					// Setting the date of task to last execution for further checks
					pSchTask->m_tScheduleDateTime = CTime( tNow.GetYear(), tNow.GetMonth(), tNow.GetDay(),
						pSchTask->m_tScheduleDateTime.GetHour(), pSchTask->m_tScheduleDateTime.GetMinute(), pSchTask->m_tScheduleDateTime.GetSecond() );
				}
			}

			Scheduler.Save();
		}
	}

	// Scheduler is enabled when an active task is scheduled
	Settings.Scheduler.Enable = bSchedulerIsEnabled;
}

//////////////////////////////////////////////////////////////////////
// CScheduler execute task (Add new tasks here)

void CScheduler::ExecuteScheduledTask(CScheduleTask *pSchTask)
{
	// Execute the selected scheduled task
	pSchTask->m_bExecuted = true;

	switch ( pSchTask->m_nAction )
	{
	case BANDWIDTH_FULL:				// Set the bandwidth to full speed
		theApp.Message( MSG_NOTICE, L"Scheduler| Bandwidth: Full" );
		Settings.Live.BandwidthScaleIn	= 101;
		Settings.Live.BandwidthScaleOut	= 101;
		Settings.Bandwidth.Downloads	= 0;
		Settings.Bandwidth.Uploads		= ( ( ( Settings.Connection.OutSpeed * ( 100 - Settings.Uploads.FreeBandwidthFactor ) ) / 100 ) / 8 ) * 1024;
		Settings.Gnutella2.Enabled		= true;
		Settings.Gnutella1.Enabled		= Settings.Gnutella1.EnableAlways;
		Settings.eDonkey.Enabled		= Settings.eDonkey.EnableAlways;
		Settings.DC.Enabled				= Settings.DC.EnableAlways;
		Settings.BitTorrent.Enabled		= true;
		if ( ! Network.IsConnected() )
			Network.Connect( TRUE );
		break;

	case BANDWIDTH_LIMITED:				// Set the bandwidth to limited speeds
		theApp.Message( MSG_NOTICE, L"Scheduler| Bandwidth: Limited" );
		Settings.Live.BandwidthScaleIn	= pSchTask->m_nLimitDown;
		Settings.Live.BandwidthScaleOut	= pSchTask->m_nLimitUp;
		Settings.Bandwidth.Downloads	= ( Settings.Connection.InSpeed * 1024 ) / 8;
		Settings.Bandwidth.Uploads		= ( ( ( Settings.Connection.OutSpeed * ( 100 - Settings.Uploads.FreeBandwidthFactor ) ) / 100 ) / 8 ) * 1024;
		Settings.Gnutella2.Enabled		= true;
		Settings.Gnutella1.Enabled		= pSchTask->m_bLimitedNetworks ? false : Settings.Gnutella1.EnableAlways;
		Settings.eDonkey.Enabled		= pSchTask->m_bLimitedNetworks ? false : Settings.eDonkey.EnableAlways;
		Settings.DC.Enabled 			= pSchTask->m_bLimitedNetworks ? false : Settings.DC.EnableAlways;
	//	Settings.BitTorrent.Enabled		= true;
		if ( ! Network.IsConnected() )
			Network.Connect( TRUE );
		break;

	case BANDWIDTH_STOP:				// Set the bandwidth to 0 and disconnect all networks
		theApp.Message( MSG_NOTICE, L"Scheduler| Bandwidth: Stop" );
		Settings.Live.BandwidthScaleIn	= 0;
		Settings.Live.BandwidthScaleOut	= 0;
		Settings.Gnutella2.Enabled		= false;
		Settings.Gnutella1.Enabled		= false;
		Settings.eDonkey.Enabled		= false;
		Settings.DC.Enabled				= false;
	//	Settings.BitTorrent.Enabled		= false;
		if ( Network.IsConnected() )
			Network.Disconnect();
		break;

	case SYSTEM_EXIT:					// Exit Envy
		theApp.Message( MSG_DEBUG, L"Scheduler| System: Exit Envy" );
		if ( ! PostMainWndMessage( WM_CLOSE ) )
			theApp.Message( MSG_ERROR, L"Scheduler failed to send CLOSE message" );
		break;

	case SYSTEM_SHUTDOWN:				// Shut down the computer
		theApp.Message( MSG_NOTICE, L"Scheduler| System: Shut Down Computer" );

		if ( ! SetShutdownRights() )	// If we dont have shutdown rights
		{
			theApp.Message( MSG_DEBUG, L"Insufficient rights to shut down the system" );
			return;
		}
		if ( ShutDownComputer() )
		{
			// Close Envy if shutdown successfully started
			if ( ! PostMainWndMessage( WM_CLOSE ) )
				theApp.Message( MSG_ERROR, L"Scheduler failed to send CLOSE message" );
			else
				theApp.Message( MSG_DEBUG, L"System shutdown failed." );
		}
		break;

	case SYSTEM_DISCONNECT:				// Dial-Up Connection
		theApp.Message( MSG_NOTICE, L"Scheduler| System: Disconnect Dial-Up" );
		Settings.Live.BandwidthScaleIn	= 0;
		Settings.Live.BandwidthScaleOut	= 0;
		Settings.Gnutella2.Enabled		= false;
		Settings.Gnutella1.Enabled		= false;
		Settings.eDonkey.Enabled		= false;
		Settings.DC.Enabled 			= false;
	//	Settings.BitTorrent.Enabled		= false;
		if ( Network.IsConnected() )
			Network.Disconnect();
		HangUpConnection();				// Close
		break;

	case SYSTEM_NOTICE:					// Reminder Notes
		LoadString( IDS_SCHEDULER_REMINDER_NOTICE );
		theApp.Message( MSG_NOTICE, L"Scheduler| System: Reminder Notice | " + pSchTask->m_sDescription );
		theApp.Message( MSG_TRAY|MSG_NOTICE, LoadString( IDS_SCHEDULER_REMINDER_NOTICE ) );

		PostMainWndMessage( WM_COMMAND, ID_TRAY_OPEN );

		pSchTask->m_bActive = false;	// Repeat MsgBox workaround
		MsgBox( LoadString( IDS_SCHEDULER_REMINDER_NOTICE ) + L"\n\n" + pSchTask->m_sDescription, MB_OK );
		pSchTask->m_bActive = true;
		break;

	default: // Error
		pSchTask->m_bExecuted = false;
		theApp.Message( MSG_ERROR, L"Scheduler| Invalid task scheduled" );
	}
}

// Disconnect a dial-up connection
void CScheduler::HangUpConnection()
{
	DWORD dwCb = sizeof( RASCONN );
	DWORD dwConnections = 0;
	RASCONN* lpRasConn = NULL;
	LPRASCONNSTATUS RasConStatus = 0;

	for ( ;; )
	{
		// Free memory if necessary
		if ( lpRasConn != NULL )
		{
			HeapFree( GetProcessHeap(), 0, lpRasConn );
			lpRasConn = NULL;
		}

		// Allocate the size needed for the RAS structure
		lpRasConn = (RASCONN*)HeapAlloc( GetProcessHeap(), 0, dwCb );
		if ( ! lpRasConn )
			return;		// Out of memory

		// Set the first structure size for version checking purposes
		lpRasConn->dwSize = sizeof( RASCONN );

		// Call the RAS API
		const DWORD ret = RasEnumConnections( lpRasConn, &dwCb, &dwConnections );
		if ( ret == 0 )
			break;		// Okay
		if ( ret == ERROR_NOT_ENOUGH_MEMORY )
			continue;	// Re-allocate more memory

		return; 		// Error unknown
	}

	for ( DWORD i = 0, loop = 0 ; i < dwConnections ; i++ )			// Loop through all current connections
	{
		RasHangUp( lpRasConn[i].hrasconn );							// Hang up the connection
		while ( RasGetConnectStatus( lpRasConn[i].hrasconn, RasConStatus ) || loop > 10 )
		{
			// Loop until the connection handle is invalid, or 3 seconds have passed total
			Sleep( 300 );
			loop++;
		}
	}

	// Free the memory if necessary.
	if ( lpRasConn != NULL )
	{
		HeapFree( GetProcessHeap(), 0, lpRasConn );
		lpRasConn = NULL;
	}
}

bool CScheduler::ShutDownComputer()
{
	int ShutdownSuccess = 0;

	// Try 2000/XP way first
	ShutdownSuccess = InitiateSystemShutdownEx( NULL,
		L"Envy Scheduled Shutdown\n\nA system shutdown was scheduled using Envy. The system will now shut down.",
		30, Settings.Scheduler.ForceShutdown, FALSE, SHTDN_REASON_FLAG_USER_DEFINED );

	// Fall back to 9x way if this does not work
	if ( ! ShutdownSuccess && GetLastError() != ERROR_SHUTDOWN_IN_PROGRESS )
	{
		UINT ShutdownFlags = Settings.Scheduler.ForceShutdown ? ( EWX_POWEROFF | EWX_FORCE ) : ( EWX_POWEROFF | EWX_FORCEIFHUNG );
		DWORD dReason = ( SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED );
		ShutdownSuccess = ExitWindowsEx( ShutdownFlags, dReason );
	}

	return ( ShutdownSuccess != 0 );
}

// Give the process shutdown rights
bool CScheduler::SetShutdownRights()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	// Get a token for this process.
	if ( ! OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
		return( FALSE );

	// Get the LUID for the shutdown privilege.
	LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid );

	tkp.PrivilegeCount = 1;		// One privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get the shutdown privilege for this process.
	AdjustTokenPrivileges( hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0 );

	if ( GetLastError() != ERROR_SUCCESS )
		return FALSE;

	return TRUE;
}

bool CScheduler::IsScheduledTimePassed(CScheduleTask* pSchTask) const
{
	CTime tNow = CTime::GetCurrentTime();

	if ( tNow.GetHour() < pSchTask->m_tScheduleDateTime.GetHour() )
		return false;

	if ( tNow.GetHour() == pSchTask->m_tScheduleDateTime.GetHour() )
	{
		if ( tNow.GetMinute() < pSchTask->m_tScheduleDateTime.GetMinute() )
			return false;

		if ( tNow.GetMinute() == pSchTask->m_tScheduleDateTime.GetMinute() )
		{
			if ( tNow.GetSecond() < pSchTask->m_tScheduleDateTime.GetSecond() )
				return false;

			//if ( tNow.GetSecond() == pSchTask->m_tScheduleDateTime.GetSecond() )
			//	return true;
		}
	}

	return true;
}

int CScheduler::ScheduleFromToday(CScheduleTask* pSchTask) const
{
	CTime tNow = CTime::GetCurrentTime();

	// Task should be executed: today 0, in the past -1, or later 1.

	if ( tNow.GetYear() > pSchTask->m_tScheduleDateTime.GetYear() )
		return -1;

	if ( tNow.GetYear() < pSchTask->m_tScheduleDateTime.GetYear() )
		return 1;

	if ( tNow.GetMonth() > pSchTask->m_tScheduleDateTime.GetMonth() )
		return -1;

	if ( tNow.GetMonth() < pSchTask->m_tScheduleDateTime.GetMonth() )
		return 1;

	if ( tNow.GetDay() > pSchTask->m_tScheduleDateTime.GetDay() )
		return -1;

	if ( tNow.GetDay() < pSchTask->m_tScheduleDateTime.GetDay() )
		return 1;

	//if ( tNow.GetDay() == pSchTask->m_tScheduleDateTime.GetDay() )
		return 0;
}

// Calculates the different between current hour and shutdown hour
// Caller must first check to see if scheduler is enabled or not
LONGLONG CScheduler::GetHoursTo(unsigned int nTaskCombination)
{
	int nHoursToTasks = 0xFFFF;
	POSITION pos = GetIterator();
	CTime tNow = CTime::GetCurrentTime();

	CQuickLock oLock( m_pSection );

	while ( pos )
	{
		CScheduleTask *pSchTask = GetNext( pos );
		if ( pSchTask->m_bActive && ( pSchTask->m_nAction & nTaskCombination ) )
		{
			CTimeSpan tToTasks( 1, 0, 0, 0 );
			if ( pSchTask->m_bSpecificDays )
			{
				for ( int i = -1 ; i < 6 ; ++i )
				{
					if ( ( ( 1 << ( ( tNow.GetDayOfWeek() + i ) % 7 ) ) & pSchTask->m_nDays ) && ( i != -1 || ! pSchTask->m_bExecuted ) )
					{
						tToTasks = CTime( tNow.GetYear(), tNow.GetMonth(), tNow.GetDay(), pSchTask->m_tScheduleDateTime.GetHour(), pSchTask->m_tScheduleDateTime.GetMinute(), pSchTask->m_tScheduleDateTime.GetSecond() ) + CTimeSpan( i + 1, 0, 0, 0 ) - tNow;
						break;
					}
				}
			}
			else
			{
				tToTasks = pSchTask->m_tScheduleDateTime - tNow;
			}

			if ( tToTasks.GetTotalHours() < nHoursToTasks )
				nHoursToTasks = tToTasks.GetTotalHours();
		}
	}

	return nHoursToTasks;
}

//////////////////////////////////////////////////////////////////////
// CScheduler XML

LPCTSTR CScheduler::xmlns = L"http://schemas.getenvy.com/Scheduler.xsd";

CXMLElement* CScheduler::ToXML(BOOL bTasks)
{
	CXMLElement* pXML = new CXMLElement( NULL, L"scheduler" );
	pXML->AddAttribute( L"xmlns", CScheduler::xmlns );

	if ( bTasks )
	{
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			pXML->AddElement( GetNext( pos )->ToXML() );
		}
	}

	return pXML;
}

BOOL CScheduler::FromXML(CXMLElement* pXML)
{
	if ( ! pXML->IsNamed( L"scheduler" ) )
		return FALSE;

	int nCount = 0;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pXML->GetNextElement( pos );

		if ( pElement->IsNamed( L"task" ) )
		{
			CQuickLock oLock( m_pSection );
			CScheduleTask* pSchTask	= NULL;
			CString strGUID = pElement->GetAttributeValue( L"guid" );
			BOOL bExisting = FALSE;
			GUID pGUID;

			if ( Hashes::fromGuid( strGUID, &pGUID ) )
			{
				if ( ( pSchTask = GetGUID( pGUID ) ) != NULL )
					bExisting = TRUE;

				if ( pSchTask == NULL )
				{
					pSchTask = new CScheduleTask( FALSE );
					pSchTask->m_pGUID = pGUID;
				}
			}
			else
			{
				pSchTask = new CScheduleTask();
			}

			if ( pSchTask->FromXML( pElement ) )
			{
				if ( ! bExisting )
					m_pScheduleTasks.AddTail( pSchTask );

				nCount++;
			}
			else	// Unsuccessful read from XML
			{
				if ( ! bExisting )
					delete pSchTask;
			}
		}
	}

	return nCount > 0;
}

//////////////////////////////////////////////////////////////////////
// CScheduler import

BOOL CScheduler::Import(LPCTSTR pszFile)
{
	CFile pFile;
	if ( ! pFile.Open( pszFile, CFile::modeRead ) ) return FALSE;

	CBuffer pBuffer;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();

	BOOL bResult = FALSE;

	if ( CXMLElement* pXML = CXMLElement::FromBytes( pBuffer.m_pBuffer, pBuffer.m_nLength, TRUE ) )
	{
		bResult = FromXML( pXML );
		delete pXML;
	}

	return bResult;
}
