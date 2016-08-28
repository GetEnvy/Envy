//
// VersionChecker.cpp
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "VersionChecker.h"
#include "DiscoveryServices.h"
#include "Library.h"
#include "SharedFile.h"
#include "Transfer.h"
#include "Network.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CVersionChecker VersionChecker;

#define VERSIONCHECKER_FREQUENCY	7


//////////////////////////////////////////////////////////////////////
// CVersionChecker construction

CVersionChecker::CVersionChecker()
	: m_bVerbose( false )
{
}

CVersionChecker::~CVersionChecker()
{
	Stop();
}

void CVersionChecker::ForceCheck()
{
	m_bVerbose = true;
	Settings.VersionCheck.UpdateCheck = true;
	Settings.VersionCheck.NextCheck = 0;
	Start();
}

void CVersionChecker::ClearVersionCheck()
{
	Settings.VersionCheck.UpgradePrompt.Empty();
	Settings.VersionCheck.UpgradeFile.Empty();
	Settings.VersionCheck.UpgradeSHA1.Empty();
	Settings.VersionCheck.UpgradeTiger.Empty();
	Settings.VersionCheck.UpgradeSize.Empty();
	Settings.VersionCheck.UpgradeSources.Empty();
	Settings.VersionCheck.UpgradeVersion.Empty();
}

BOOL CVersionChecker::IsVersionNewer()
{
	WORD nVersion[ 4 ];
	return ( _stscanf_s( Settings.VersionCheck.UpgradeVersion, L"%hu.%hu.%hu.%hu",
		&nVersion[ 0 ], &nVersion[ 1 ], &nVersion[ 2 ], &nVersion[ 3 ] ) == 4 ) &&
	//	( theApp.m_nVersion[ 0 ] < nVersion[ 0 ] ||
		( theApp.m_nVersion[ 0 ] == nVersion[ 0 ] &&
		( theApp.m_nVersion[ 1 ] < nVersion[ 1 ] ||
		( theApp.m_nVersion[ 1 ] == nVersion[ 1 ] &&
		( theApp.m_nVersion[ 2 ] < nVersion[ 2 ] ||
		( theApp.m_nVersion[ 2 ] == nVersion[ 2 ] &&
		( theApp.m_nVersion[ 3 ] < nVersion[ 3 ] ) ) ) ) ) );
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker time check

BOOL CVersionChecker::NeedToCheck()
{
	if ( ! IsVersionNewer() )	// User manually upgraded
		ClearVersionCheck();

	if ( ! Settings.VersionCheck.UpdateCheck ) return FALSE;
	if ( ! Settings.VersionCheck.NextCheck ) return TRUE;
	if ( Settings.VersionCheck.NextCheck == 0xFFFFFFFF ) return FALSE;

	return (DWORD)CTime::GetCurrentTime().GetTime() >= Settings.VersionCheck.NextCheck;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker start

BOOL CVersionChecker::Start()
{
	if ( IsThreadAlive() )
		return TRUE;

	m_pRequest.Clear();

	return BeginThread( "VersionChecker" );
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker stop

void CVersionChecker::Stop()
{
	if ( IsThreadAlive() )
	{
		m_pRequest.Cancel();
		CloseThread();
	}
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker thread run

void CVersionChecker::OnRun()
{
	if ( NeedToCheck() )
	{
		if ( ExecuteRequest() )
			ProcessResponse();
		else
			SetNextCheck( VERSIONCHECKER_FREQUENCY );

		if ( IsThreadEnabled() )
			PostMainWndMessage( WM_VERSIONCHECK, VC_MESSAGE_AND_CONFIRM );
	}
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker undertake request

BOOL CVersionChecker::ExecuteRequest()
{
	// ToDo: https://sf.net/p/getenvy/code/HEAD/tree/trunk/release?format=raw
	// Or:   https://raw.githubusercontent.com/GetEnvy/Envy/master/release
	// Or use L"http://getenvy.sourceforge.net/update"

	const CString strURL = UPDATE_URL		// Settings.VersionCheck.UpdateCheckURL
		  L"?Version=" + theApp.m_sVersion
#ifdef WIN64
		+ L"&Bits=64"
#else
		+ L"&Bits=32"
#endif	// WIN64
		+ L"&Language=" + Settings.General.Language.Left(2);

	if ( ! m_pRequest.SetURL( strURL ) )
		return FALSE;

	theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, L"[VersionChecker] Request: %s", (LPCTSTR)strURL );

	if ( ! m_pRequest.Execute( false ) )
		return FALSE;

	int nStatusCode = m_pRequest.GetStatusCode();
	if ( nStatusCode < 200 || nStatusCode > 299 ) return FALSE;

	CString strOutput = m_pRequest.GetResponseString();

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"[VersionChecker] Response: %s", (LPCTSTR)strOutput );

	for ( strOutput += L'&' ; ! strOutput.IsEmpty() ; )
	{
		CString strItem	= strOutput.SpanExcluding( L"&" );
		strOutput		= strOutput.Mid( strItem.GetLength() + 1 );

		CString strKey = strItem.SpanExcluding( L"=" );
		if ( strKey.GetLength() == strItem.GetLength() ) continue;

		strItem = URLDecode( strItem.Mid( strKey.GetLength() + 1 ) );
		strItem.Trim();

		m_pResponse.SetAt( strKey, strItem );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker process response

void CVersionChecker::ProcessResponse()
{
	int nDays = VERSIONCHECKER_FREQUENCY;
	CString strValue;

	if ( m_pResponse.Lookup( L"Message", strValue ) ||
		 m_pResponse.Lookup( L"MessageBox", strValue ) )
	{
		m_sMessage = strValue;
	}

	if ( m_pResponse.Lookup( L"Quote", strValue ) )
	{
		Settings.VersionCheck.Quote = strValue;
	}

	if ( m_pResponse.Lookup( L"SystemMsg", strValue ) )
	{
		for ( strValue += '\n' ; strValue.GetLength() ; )
		{
			CString strLine	= strValue.SpanExcluding( L"\r\n" );
			strValue		= strValue.Mid( strLine.GetLength() + 1 );
			if ( ! strLine.IsEmpty() ) theApp.Message( MSG_NOTICE, strLine );
		}
	}

	if ( m_pResponse.Lookup( L"UpgradePrompt", strValue ) )
	{
		Settings.VersionCheck.UpgradePrompt = strValue;

		m_pResponse.Lookup( L"UpgradeFile", Settings.VersionCheck.UpgradeFile );
		m_pResponse.Lookup( L"UpgradeSHA1", Settings.VersionCheck.UpgradeSHA1 );
		m_pResponse.Lookup( L"UpgradeTiger", Settings.VersionCheck.UpgradeTiger );
		m_pResponse.Lookup( L"UpgradeSize", Settings.VersionCheck.UpgradeSize );
		m_pResponse.Lookup( L"UpgradeSources", Settings.VersionCheck.UpgradeSources );
		m_pResponse.Lookup( L"UpgradeVersion", Settings.VersionCheck.UpgradeVersion );

		// Old name
		if ( Settings.VersionCheck.UpgradeSHA1.IsEmpty() )
			m_pResponse.Lookup( L"UpgradeHash", Settings.VersionCheck.UpgradeSHA1 );
	}
	else
	{
		ClearVersionCheck();
	}

	if ( ! IsVersionNewer() )
		ClearVersionCheck();

	if ( m_pResponse.Lookup( L"AddDiscovery", strValue ) )
	{
		strValue.Trim();
		theApp.Message( MSG_DEBUG, L"[VersionChecker] %s = %s", L"AddDiscovery", (LPCTSTR)strValue );
		DiscoveryServices.Add( strValue, CDiscoveryService::dsWebCache );
	}

	if ( m_pResponse.Lookup( L"AddDiscoveryUHC", strValue ) )
	{
		strValue.Trim();
		theApp.Message( MSG_DEBUG, L"[VersionChecker] %s = %s", L"AddDiscoveryUHC", (LPCTSTR)strValue );
		DiscoveryServices.Add( strValue, CDiscoveryService::dsGnutella, PROTOCOL_G1 );
	}

	if ( m_pResponse.Lookup( L"AddDiscoveryKHL", strValue ) )
	{
		strValue.Trim();
		theApp.Message( MSG_DEBUG, L"[VersionChecker] %s = %s", L"AddDiscoveryKHL", (LPCTSTR)strValue );
		DiscoveryServices.Add( strValue, CDiscoveryService::dsGnutella, PROTOCOL_G2 );
	}

	if ( m_pResponse.Lookup( L"NextCheck", strValue ) )
	{
		strValue.Trim();
		theApp.Message( MSG_DEBUG, L"[VersionChecker] %s = %s", L"NextCheck", (LPCTSTR)strValue );
		_stscanf( strValue, L"%i", &nDays );
	}

	SetNextCheck( nDays );

	m_pResponse.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker NextCheck update

void CVersionChecker::SetNextCheck(int nDays)
{
	CTimeSpan tPeriod( nDays, 0, 0, 0 );
	CTime tNextCheck = CTime::GetCurrentTime() + tPeriod;
	Settings.VersionCheck.NextCheck = (DWORD)tNextCheck.GetTime();
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker check if a download is an upgrade

BOOL CVersionChecker::CheckUpgradeHash(const CLibraryFile* pFile)
{
	if ( ! IsUpgradeAvailable() )
		return FALSE;

	CEnvyFile oFilter;
	if ( oFilter.m_oSHA1.fromString( Settings.VersionCheck.UpgradeSHA1 ) )
	{
		oFilter.m_nSize = _tstoi64( Settings.VersionCheck.UpgradeSize );

		CQuickLock oLock( Library.m_pSection );
		if ( ! pFile )
			pFile = LibraryMaps.LookupFileByHash( &oFilter, FALSE, TRUE );

		if ( pFile &&
			 pFile->m_nSize == oFilter.m_nSize &&
			 validAndEqual( pFile->m_oSHA1, oFilter.m_oSHA1 ) &&
			 EndsWith( pFile->GetPath(), _P( L".exe" ) ) )	//_tcsicmp( PathFindExtension( pFile->GetPath() ), L".exe" ) == 0 )
		{
			m_sUpgradePath = pFile->GetPath();
			PostMainWndMessage( WM_VERSIONCHECK, VC_UPGRADE );
			return TRUE;
		}
	}

	return FALSE;
}
