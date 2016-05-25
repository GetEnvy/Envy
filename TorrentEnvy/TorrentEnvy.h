//
// TorrentEnvy.h
//
// This file is part of Torrent Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008,2012 and Shareaza 2007
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "Resource.h"
#include "WizardSheet.h"


class CTorrentEnvyApp : public CWinApp
{
// Construction
public:
	CTorrentEnvyApp();

// Attributes
public:
	CString		m_sPath;
	CString		m_sVersion;
	WORD		m_nVersion[4];
	CFont		m_fntNormal;
	CFont		m_fntBold;
	CFont		m_fntLine;
	CFont		m_fntTiny;

	CStringList	m_sCommandLinePaths;
	CString		m_sCommandLineSourceFile;
	CString		m_sCommandLineDestination;
	CString		m_sCommandLineTracker;
	CString		m_sCommandLineComment;

// Operations
protected:
	CWizardSheet*	m_pSheet;

	void		InitEnvironment();
	void		InitResources();

// Overrides
protected:
	//{{AFX_VIRTUAL(CTorrentEnvyApp)
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CTorrentEnvyApp)
	afx_msg void OnHelp();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

extern CTorrentEnvyApp theApp;

CString SmartSize(QWORD nVolume);
