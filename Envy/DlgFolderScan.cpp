//
// DlgFolderScan.cpp
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
#include "DlgFolderScan.h"
#include "Library.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CFolderScanDlg, CSkinDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

CFolderScanDlg* CFolderScanDlg::m_pDialog = NULL;
CCriticalSection CFolderScanDlg::m_pProgressSection;


/////////////////////////////////////////////////////////////////////////////
// CFolderScanDlg dialog

CFolderScanDlg::CFolderScanDlg(CWnd* pParent)
    : CSkinDialog(CFolderScanDlg::IDD, pParent)
    , m_nGeneration(0)
    , m_nFiles(0)
    , m_nVolume(0)
    , m_nError(ERROR_SUCCESS)
{
	m_tLastUpdate = 0;
	m_bActive = FALSE;

	CSingleLock oLock(&m_pProgressSection, TRUE);
	m_pDialog = this;
}

CFolderScanDlg::~CFolderScanDlg()
{
	CSingleLock pLock(&m_pProgressSection, TRUE);
	if (m_pDialog == this)
		m_pDialog = NULL;
}

void CFolderScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCAN_VOLUME, m_wndVolume);
	DDX_Control(pDX, IDC_SCAN_FILES, m_wndFiles);
	DDX_Control(pDX, IDC_SCAN_FILE, m_wndFile);
}

/////////////////////////////////////////////////////////////////////////////
// CFolderScanDlg operations

BOOL CFolderScanDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CFolderScanDlg", IDR_LIBRARYFRAME );

	// Callers change the shared folders before showing the dialog, so a scan
	// requested now covers those changes; earlier or concurrent passes do not
	// complete it.
	{
		// Under the progress lock so ReportError() sees the generation as soon
		// as the worker can start it.
		CSingleLock oLock(&m_pProgressSection, TRUE);
		m_nGeneration = Library.RequestScan();
	}

	SetTimer( 1, 500, NULL );
	m_tLastUpdate	= 0;
	m_bActive		= TRUE;

	return TRUE;
}

void CFolderScanDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	DWORD nFiles, nError;
	QWORD nVolume;
	CString strFile, strErrorPath;
	{
		CSingleLock pLock(&m_pProgressSection, TRUE);
		nFiles = m_nFiles;
		nVolume = m_nVolume;
		nError = m_nError;
		strFile = m_sCurrentFile;
		strErrorPath = m_sErrorPath;
	}

	const EnvyLibraryScan::Phase nPhase = Library.GetScanPhase(m_nGeneration);
	if (strFile.IsEmpty() && nPhase != EnvyLibraryScan::Phase::Completed)
		strFile = LoadString(nPhase == EnvyLibraryScan::Phase::Pending ? IDS_FOLDER_SCAN_WAITING
		                                                               : IDS_FOLDER_SCAN_RUNNING);

	CString strItem;
	strItem.Format(L"%lu", nFiles);
	m_wndFiles.SetWindowText(strItem);
	m_wndVolume.SetWindowText(Settings.SmartVolume(nVolume, KiloBytes));
	m_wndFile.SetWindowText(strFile);

	if (nPhase != EnvyLibraryScan::Phase::Completed && !Library.IsThreadAlive())
	{
		// Without the worker the request can never complete; say so instead of
		// leaving the counter at zero.
		KillTimer(1);
		MsgBox(LoadString(IDS_FOLDER_SCAN_NO_WORKER), MB_OK | MB_ICONWARNING);
		OnCancel();
		return;
	}

	if (nPhase == EnvyLibraryScan::Phase::Completed)
	{
		KillTimer(1);
		{
			// The scan has finished: read the error it may have reported meanwhile.
			CSingleLock pProgressLock(&m_pProgressSection, TRUE);
			nError = m_nError;
			strErrorPath = m_sErrorPath;
		}
		if (nError != ERROR_SUCCESS)
		{
			strItem.Format(LoadString(IDS_FOLDER_SCAN_FAILED), (LPCTSTR)strErrorPath, (LPCTSTR)GetErrorString(nError),
			               nError);
			MsgBox(strItem, MB_OK | MB_ICONWARNING);
		}
		OnCancel();
	}
}

void CFolderScanDlg::OnCancel()
{
	// Closing the dialog only stops watching: the scan goes on in the background
	// and is cancelled by application shutdown, which never prunes entries the
	// interrupted listing did not reach.
	m_bActive = FALSE;
	CSingleLock pLock(&m_pProgressSection, TRUE);
	if (m_pDialog == this)
		m_pDialog = NULL;
	pLock.Unlock();

	CSkinDialog::OnCancel();
}

void CFolderScanDlg::Update(LPCTSTR pszName, DWORD nVolume)
{
	CSingleLock oLock(&m_pProgressSection, TRUE);
	if (m_pDialog != NULL)
		m_pDialog->InstanceUpdate(pszName, nVolume);
}

void CFolderScanDlg::ReportError(DWORD nError, LPCTSTR pszPath, bool bLog)
{
	if (bLog)
		theApp.Message(MSG_ERROR,
		               L"Shared-folder scan failed on %s (%s, Windows error %lu). Existing entries were kept.", pszPath,
		               (LPCTSTR)GetErrorString(nError), nError);

	CSingleLock oLock(&m_pProgressSection, TRUE);
	// The dialog reports the first failure of the scan it requested; failures
	// from passes that started earlier only reach the log.
	if (m_pDialog != NULL && m_pDialog->m_nError == ERROR_SUCCESS && m_pDialog->m_nGeneration != 0 &&
	    Library.GetScanPhase(m_pDialog->m_nGeneration) != EnvyLibraryScan::Phase::Pending)
	{
		m_pDialog->m_nError = nError;
		m_pDialog->m_sErrorPath = pszPath;
	}
}

void CFolderScanDlg::InstanceUpdate(LPCTSTR pszName, DWORD nVolume)
{
	m_nFiles++;
	m_nVolume += nVolume;
	m_sCurrentFile = pszName;
}
