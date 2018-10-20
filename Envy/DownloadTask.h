//
// DownloadTask.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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

#include "ThreadImpl.h"
#include "FileFragments.hpp"

class CDownload;
class CHttpRequest;


enum dtask
{
	dtaskNone = 0,		// No task
	dtaskAllocate,		// Allocating...
	dtaskCopy,			// Moving...
	dtaskMergeFile,		// Merging...
	dtaskPreviewRequest	// Previewing...
};


class CDownloadTask : public CThreadImpl
{
public:
	CDownloadTask(CDownload* pDownload);
	virtual ~CDownloadTask();

public:
	float				m_fProgress;		// Progress of current operation (0-100%)

	void				Allocate();
	void				Copy();
	void				PreviewRequest(LPCTSTR szURL);
	void				MergeFile(CList< CString >* pFiles, BOOL bValidation = TRUE, const Fragments::List* pGaps = NULL);
	void				MergeFile(LPCTSTR szPath, BOOL bValidation = TRUE, const Fragments::List* pGaps = NULL);

	void				Abort();
	bool				HasSucceeded() const;
	DWORD				GetFileError() const;
	dtask				GetTaskType() const;
	CString				GetRequest() const;
	float				GetProgress() const;	// Get progress of current operation (0-100%)
	CBuffer*			IsPreviewAnswerValid(const Hashes::Sha1Hash& oRequestedSHA1) const;

private:
	CAutoPtr< CHttpRequest > m_pRequest;
	CDownload*			m_pDownload;
	dtask				m_nTask;
	bool				m_bSuccess;
	DWORD				m_nFileError;
	CString				m_sDestination;
//	CString				m_sURL;				// Request URL
	CList< CString >	m_oMergeFiles;		// Source filename(s)
	Fragments::List		m_oMergeGaps;		// Missed ranges in source file
	BOOL				m_bMergeValidation;	// Run validation after merging

	static DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
		LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize,
		LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber,
		DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile,
		LPVOID lpData);

	void				Construct(dtask nTask);

	void				OnRun();
	void				RunPreviewRequest();
	void				RunAllocate();
	void				RunCopy();
	void				RunMerge();
	void				RunMergeFile(CDownload* pDownload, LPCTSTR szFilename, BOOL bMergeValidation, const Fragments::List& oMissedGaps, float fProgress = 100.0f);
//	void				CreatePathForFile(const CString& strBase, const CString& strPath);
//	BOOL				CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength);
	BOOL				CopyFileToBatch(HANDLE hSource, QWORD nOffset, QWORD nLength, LPCTSTR pszPath);
	BOOL				MakeBatchTorrent();
};
