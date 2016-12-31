//
// Download.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2007
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

#pragma once

// Set at INTERNAL_VERSION on change:
#define DOWNLOAD_SER_VERSION 1

// nVersion History:
// 30 - Shareaza 2.0 (2004)
// 33 - added m_sSearchKeyword to CDownloadBase (CyberBob)
// 34 - added m_bSeeding and m_sServingFileName to CDownloadWithTorrent (Rolandas)
// 35 - added m_sCountry to CDownloadSource (dcat)
// 36 - nothing (Rolandas) (Shareaza 2.2.4.0?)
// 37 - added m_oBTH to CDownloadBase, m_bBTH and m_bMD5 to CDownloadSource (Ryo-oh-ki)
// 38 - added m_sCountryName to CDownloadSource (dcat)
// 39 - added m_bClientExtended to CDownloadSource (Ryo-oh-ki)
// 40 - added virtual fragmented file (Ryo-oh-ki)
// 41 - added m_sName to CFragmentedFile (Ryo-oh-ki)
// 42 - added m_bMetaIgnore to CDownloadSource (Ry-oh-ki) (Shareaza 2.5.2.0)
// 1000 - add m_tDate, remove sSearchKeyword in CDownloadBase
// 1  - (Envy 1.0)

#include "DownloadWithExtras.h"


class CDownload : public CDownloadWithExtras
{
public:
	CDownload();
	virtual ~CDownload();

public:
	DWORD		m_nSerID;
	BOOL		m_bExpanded;
	BOOL		m_bSelected;
	DWORD		m_tCompleted;
	int			m_nRunCookie;
	int			m_nGroupCookie;
	BOOL		m_bClearing;			// Briefly marked for removal or deletion (rarely visible, but may take longer than expected)
private:
	BOOL		m_bTempPaused;			// Disk full
	BOOL		m_bPaused;
	BOOL		m_bBoosted;
	BOOL		m_bShared;
	bool		m_bComplete;
	bool		m_bDownloading; 		// Store if a download is downloading, as performance tweak. Count transfers for 100% current answer.
	bool		m_bStableName;			// Download has a stable name  (Set in CDownloadTransferHTTP::OnHeaderLine "Content-Disposition")
	DWORD		m_tBegan;				// Time when this download began trying to download (Started searching, etc). 0 means not tried this session.
	DWORD		m_tSaved;

	CDownloadTask	m_pTask;

public:
	bool		HasStableName() const;	// Download has a stable name (return m_bStableName)
	void		SetStableName(bool bStable = true);

	void		Pause(BOOL bRealPause = TRUE);
	void		Resume();
	void		Remove();
	void		Boost(BOOL bBoost = TRUE);
	void		Share(BOOL bShared);
	bool		IsStarted() const;		// Has the download actually downloaded anything?
	bool		IsDownloading() const;	// Is the download receiving data?
	bool		IsBoosted() const;
	bool		IsShared() const;
	CString		GetDownloadSources() const;
	CString		GetDownloadStatus() const;
	int			GetClientStatus() const;
	BOOL		Launch(int nIndex, CSingleLock* pLock, BOOL bForceOriginal = FALSE);
	BOOL		Enqueue(int nIndex, CSingleLock* pLock);
	BOOL		Load(LPCTSTR pszPath);
	BOOL		Save(BOOL bFlush = FALSE);
	BOOL		OpenDownload();
	BOOL		SeedTorrent();
	BOOL		PrepareFile();
	void		ForceComplete();
	void		OnRun();

	void		Allocate();
	void		Copy();
	void		PreviewRequest(LPCTSTR szURL);
	void		MergeFile(CList< CString >* pFiles, BOOL bValidation = TRUE, const Fragments::List* pGaps = NULL);
	void		MergeFile(LPCTSTR szPath, BOOL bValidation = TRUE, const Fragments::List* pGaps = NULL);

private:
	void		StartTrying();
	void		StopTrying();
	void		AbortTask();
	DWORD		GetStartTimer() const;
	void		OnDownloaded();
//	void		SerializeOld(CArchive& ar, int nVersion);	// Legacy DOWNLOAD_SER_VERSION < 11 (2002), for reference only

public:
	virtual bool  Resize(QWORD nNewSize);	// Set new download size
	virtual float GetProgress() const;		// Statistics
	virtual dtask GetTaskType() const;		// Return currently running task
	virtual bool IsTasking() const;			// Check if a task is already running
	virtual bool IsTrying() const;			// Is the download currently trying to download?
	virtual bool IsPaused(bool bRealState = false) const;
	virtual bool IsCompleted() const;
	virtual bool IsMoving() const;
	virtual void OnMoved();					// File was moved to the Library
	virtual BOOL OnVerify(const CLibraryFile* pFile, TRISTATE bVerified);	// File was hashed and verified in the Library
	virtual void Serialize(CArchive& ar, int nVersion); 	// DOWNLOAD_SER_VERSION

	friend class CDownloadTransfer;			// GetVerifyLength
	friend class CDownloadWithTorrent;		// m_bComplete
	friend class CDownloadsWnd;				// m_pTask
	friend class CDownloads;				// m_bComplete for Load()
};
