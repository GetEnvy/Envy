//
// UploadTransfer.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2007
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

#include "Transfer.h"
#include "EnvyFile.h"
#include "FragmentedFile.h"

class CUploadQueue;
class CUploadFile;
class CLibraryFile;
class CDownload;

#define ULA_SLOTS	16


class CUploadTransfer abstract : public CTransfer, public CEnvyFile
{
public:
	CUploadTransfer(PROTOCOLID nProtocol);
	virtual ~CUploadTransfer();

public:
	CUploadQueue*	m_pQueue;		// Queue reference
	CUploadFile*	m_pBaseFile;	// Reference file
	DWORD			m_nUserRating;	// Has the downloader uploaded anything?

	QWORD			m_nFileBase;	// Base offset in requested file
	BOOL			m_bFilePartial;	// Partial file flag
	CString			m_sFileTags;	// File sharing tags

	BOOL			m_bLive;		// Live connection tag
	DWORD			m_nRequests;	// Request count
	QWORD			m_nUploaded;	// Bytes uploaded
	DWORD			m_tContent;		// Send start timestamp

	BOOL			m_bPriority;	// User unlimited upload

protected:
	BOOL			m_bStopTransfer; // Should this transfer stop? (to allow queue rotation, etc)
	DWORD			m_tRotateTime;
	DWORD			m_tAverageTime;
	int				m_nAveragePos;
	DWORD			m_nAverageRate[ULA_SLOTS];
	DWORD			m_nMaxRate;		// Maximum average speed we got
	DWORD			m_tRatingTime;	// When rating was last calculated

private:
	auto_ptr< CFragmentedFile > m_pFile;	// Disk file

public:
	virtual void	Remove(BOOL bMessage = TRUE);
	virtual void	Close(UINT nError = 0);
	virtual BOOL	Promote(BOOL bPriority = FALSE);
	virtual BOOL	OnRename(LPCTSTR pszSource, LPCTSTR pszTarget); 	// pszTarget: 0 = delete file, 1 = release file

	virtual float	GetProgress() const;
	virtual DWORD	GetAverageSpeed();
	virtual DWORD	GetMeasuredSpeed();
	virtual DWORD	GetMaxSpeed(); //const;
	virtual void	SetSpeedLimit(DWORD nLimit);

protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual void	OnQueueKick() {};

	void			LongTermAverage(DWORD tNow);
	void			RotatingQueue(DWORD tNow);
	void			CalculateRating(DWORD tNow);
	void			ClearHashes();
	void			ClearRequest();
	BOOL			HashesFromURN(LPCTSTR pszURN);
	BOOL			RequestComplete(const CLibraryFile* pFile);
	BOOL			RequestPartial(CDownload* pDownload);
	void			StartSending(int nState);
	void			AllocateBaseFile();

	virtual BOOL	IsFileOpen() const;
	virtual BOOL	OpenFile();
	virtual void	CloseFile();
	virtual BOOL	WriteFile(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten = NULL);
	virtual BOOL	ReadFile(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead = NULL);
	void			AttachFile(auto_ptr< CFragmentedFile >& pFile);
};

enum UserRating
{
	urNull, urCredit, urSharing, urNew, urNotSharing
	// 1 = Uploaded more to us than we have to them (We 'owe' them upload)
	// 2 = Known Sharer (We've given them more than they have given us)
	// 3 = New user ()
	// 4 = Known user who has not uploaded (May simply not have anything we want)
};

enum UploadState
{
	upsNull, upsReady, upsConnecting,
	upsRequest, upsHeaders, upsQueued,
	upsUploading, upsResponse,
	upsBrowse, upsTigerTree, upsMetadata, upsPreview, upsPreQueue
};
