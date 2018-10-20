//
// VersionChecker.h
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

#pragma once

#include "HttpRequest.h"

class CLibraryFile;


class CVersionChecker : public CThreadImpl
{
public:
	CVersionChecker();
	virtual ~CVersionChecker();

public:
	CString		m_sMessage;
	CString		m_sUpgradePath;

protected:
	CMap< CString, const CString&, CString, CString& > m_pResponse;
	CHttpRequest m_pRequest;
	bool		m_bVerbose;

public:
	BOOL		Start();
	void		Stop();
	void		ForceCheck();
	static void ClearVersionCheck();
	void		SetNextCheck(int nDays);
	BOOL		CheckUpgradeHash(const CLibraryFile* pFile = NULL);

	static BOOL	IsVersionNewer();		// Test if available version is newer than current

	inline bool	IsUpgradeAvailable() const throw()
	{
		return ! Settings.VersionCheck.UpgradePrompt.IsEmpty();
	}

	inline bool IsVerbose() const throw()
	{
		return m_bVerbose;
	}

protected:
	void		OnRun();
	BOOL		NeedToCheck();
	BOOL		ExecuteRequest();
	void		ProcessResponse();
};

extern CVersionChecker VersionChecker;

enum VERSION_CHECK	// WM_VERSIONCHECK message wParam argument
{
	VC_MESSAGE_AND_CONFIRM = 0, // Show message and then ask to download new version
	VC_CONFIRM = 1,				// Ask to download new version
	VC_UPGRADE = 2				// Ask then start upgrading of already downloaded installer
};
