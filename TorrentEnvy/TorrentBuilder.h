//
// TorrentBuilder.h
//
// This file is part of Torrent Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2008,2014 and Shareaza 2007
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#ifdef _PORTABLE
#include "Portable\SHA1.h"
#include "Portable\ED2K.h"
#include "Portable\MD4.h"
#endif

class CTorrentBuilder : public CWinThread
{
// Construction
public:
	CTorrentBuilder();
	virtual ~CTorrentBuilder();

	DECLARE_DYNCREATE(CTorrentBuilder)

// Operations
public:
	BOOL	SetName(LPCTSTR pszName);
	BOOL	SetOutputFile(LPCTSTR pszPath);
	void	SetPieceSize(int nPieceIndex);
	void	Enable(BOOL bSHA1, BOOL bED2K, BOOL bMD5);
	BOOL	AddFile(LPCTSTR pszPath);
	BOOL	AddTracker(LPCTSTR pszURL);
	BOOL	AddTrackerURL(LPCTSTR pszURL);
	BOOL	AddTrackerURL2(LPCTSTR pszURL);
	BOOL	SetComment(LPCTSTR pszComment);
//	BOOL	SetSource(LPCTSTR pszSource);
	BOOL	SetPrivate(BOOL bPrivate);
public:
	BOOL	Start();
	void	Stop();
	BOOL	SetPriority(int nPriority);
	BOOL	IsRunning();
	BOOL	IsFinished();
	BOOL	GetTotalProgress(DWORD& nPosition, DWORD& nScale);
	BOOL	GetCurrentFile(CString& strFile);
	BOOL	GetMessageString(CString& strMessage);
protected:
	BOOL	ScanFiles();
	BOOL	ProcessFiles();
	BOOL	ProcessFile(DWORD nFile, LPCTSTR pszFile);
	BOOL	WriteOutput();

// Attributes
protected:
	CCriticalSection	m_pSection;
	BOOL				m_bActive;
	BOOL				m_bFinished;
	BOOL				m_bAbort;
	BOOL				m_bPrivate;
	CString				m_sMessage;
	CString				m_sName;
	CString				m_sOutput;
	CString				m_sTracker;
	CString				m_sTracker2;
	CStringList			m_pTrackers;
	CString				m_sComment;
//	CString				m_sSource;
	CStringList			m_pFiles;
	CString				m_sThisFile;
	QWORD				m_nTotalSize;
	QWORD				m_nTotalPos;
	QWORD*				m_pFileSize;
	DWORD				m_nPieceSize;
	DWORD				m_nPieceCount;
	DWORD				m_nPiecePos;
	DWORD				m_nPieceUsed;
	BOOL				m_bAutoPieces;

	BOOL				m_bSHA1;		// Enable SHA1 creation
	BOOL				m_bED2K;		// Enable MD4 creation
	BOOL				m_bMD5;			// Enable MD5 creation
#ifdef _PORTABLE
	CHashSHA1			m_pDataSHA1;
	CHashMD4			m_pDataED2K;
	CHashSHA1*			m_pFileSHA1;
	CHashMD4*			m_pFileED2K;
	CHashSHA1*			m_pPieceSHA1;
	CSHA1*				m_phPieceSHA1;
	CSHA1*				m_phFullSHA1;
	CSHA1*				m_phFileSHA1;
	CED2K*				m_phFullED2K;
	CED2K*				m_phFileED2K;
#else // Use HashLib
	CSHA				m_oDataSHA1;	// Total SHA1
	CED2K				m_oDataED2K;	// Total MD4
	CMD5				m_oDataMD5;		// Total MD5
	CSHA*				m_pFileSHA1;	// SHA1 per file
	CED2K*				m_pFileED2K;	// MD4 per file
	CMD5*				m_pFileMD5;		// MD5 per file
	CSHA*				m_pPieceSHA1;	// BitTorrent SHA1 per piece
	CSHA				m_oPieceSHA1;	// BitTorrent piece SHA1 (temporary)
#endif

	BYTE*				m_pBuffer;
	DWORD				m_nBuffer;

// Overrides
public:
	virtual BOOL InitInstance() { return TRUE; }
	virtual int Run();

	DECLARE_MESSAGE_MAP()
};
