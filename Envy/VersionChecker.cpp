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
	Settings.VersionCheck.UpgradeVersion.Empty();
	Settings.VersionCheck.UpgradeFile.Empty();
	Settings.VersionCheck.UpgradeDate.Empty();
	Settings.VersionCheck.UpgradeSize.Empty();
	Settings.VersionCheck.UpgradeSHA1.Empty();
	Settings.VersionCheck.UpgradeTiger.Empty();
	Settings.VersionCheck.UpgradePrompt.Empty();
	Settings.VersionCheck.UpgradeSources.Empty();
}

BOOL CVersionChecker::IsVersionNewer()
{
	WORD nVersion[ 2 ];
	return ( _stscanf_s( Settings.VersionCheck.UpgradeVersion, L"%hu.%hu",
		&nVersion[ 0 ], &nVersion[ 1 ] ) == 2 ) &&
		( theApp.m_nVersion[ 0 ] < nVersion[ 0 ] ||
		( theApp.m_nVersion[ 0 ] == nVersion[ 0 ] &&
		( theApp.m_nVersion[ 1 ] < nVersion[ 1 ] ) ) );
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
	// URL: http://sf.net/p/getenvy/code/HEAD/tree/trunk/release?format=raw
	// ALT: http://raw.githubusercontent.com/GetEnvy/Envy/master/release
	// Response: Version=1.0&Name=Envy.1.0.exe&Size=10009496&Date=20161230&SHA1=ZAEGMBJ6CS2NZM2I7ZZ3HLNLORBCXBUB&Tiger=IUOKZDQZPLTFU4R7YJJNMVVROW7XOMUR4OJH2AA&Info=First+public+release+1.0

//	const CString strURL = Settings.VersionCheck.UpdateCheckURL
//		  L"?Version=" + theApp.m_sVersion
//#ifdef WIN64
//		+ L"&Bits=64"
//#else
//		+ L"&Bits=32"
//#endif
//		+ L"&Language=" + Settings.General.Language.Left(2);

	BOOL bSuccess = FALSE;
	theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, L"[VersionChecker] Request: %s", UPDATE_URL );
	if ( m_pRequest.SetURL( UPDATE_URL ) && m_pRequest.Execute( false ) )
	{
		int nStatusCode = m_pRequest.GetStatusCode();
		if ( nStatusCode >= 200 && nStatusCode < 300 )
			bSuccess = TRUE;
	}

	if ( ! bSuccess )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, L"[VersionChecker] Request: %s", UPDATE_URL_ALT );
		if ( m_pRequest.SetURL( UPDATE_URL_ALT ) && m_pRequest.Execute( false ) )
		{
			int nStatusCode = m_pRequest.GetStatusCode();
			if ( nStatusCode >= 200 && nStatusCode < 300 )
				bSuccess = TRUE;
		}
	}

	if ( ! bSuccess )
		return FALSE;

	CString strOutput = m_pRequest.GetResponseString();
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"[VersionChecker] Response: %s", (LPCTSTR)strOutput );

	if ( strOutput.GetLength() < 3 )
		return FALSE;

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

	m_pResponse.Lookup( L"Version", Settings.VersionCheck.UpgradeVersion );		// "UpgradeVersion"

	if ( IsVersionNewer() )
	{
		m_pResponse.Lookup( L"File", Settings.VersionCheck.UpgradeFile );		// "UpgradeFile"
		m_pResponse.Lookup( L"Date", Settings.VersionCheck.UpgradeDate );
		m_pResponse.Lookup( L"Size", Settings.VersionCheck.UpgradeSize );		// "UpgradeSize"
		m_pResponse.Lookup( L"SHA1", Settings.VersionCheck.UpgradeSHA1 );		// "UpgradeSHA1"
		m_pResponse.Lookup( L"Tiger", Settings.VersionCheck.UpgradeTiger );		// "UpgradeTiger"
		m_pResponse.Lookup( L"Sources", Settings.VersionCheck.UpgradeSources );	// "UpgradeSources"
		m_pResponse.Lookup( L"Info", Settings.VersionCheck.UpgradePrompt );		// "UpgradePrompt"
	}
	else
	{
		ClearVersionCheck();
	}

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
