//
// BTInfo.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2015
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

#include "Buffer.h"
#include "EnvyFile.h"

#define BAD_TRACKER_TOKEN L'•'	// *udp:// for display only

class CBuffer;
class CBENode;


class CBTInfo : public CEnvyFile
{
public:
	CBTInfo();
	CBTInfo(const CBTInfo& oSource);
	CBTInfo& operator=(const CBTInfo& oSource);
	virtual ~CBTInfo();

public:
	// Tracker status/types (SetTrackerMode)
	enum
	{
		tNull,				// No tracker
		tSingle,			// User locked tracker or single tracker
		tMultiFinding,		// Multi-tracker searching
		tMultiFound			// Multi-tracker that's found a tracker
	};

	// When to initiate new torrent transfers
	enum
	{
		dtAlways,			// Whenever wanted
		dtWhenRatio,		// Download ratio > 100%
		dtWhenRequested,	// Only when another client requests
		dtNever				// Never
	};

// Subclass
public:
	class CBTFile : public CEnvyFile
	{
	public:
		CString	FindFile() const;		// Find file on disk

	private:
		const CBTInfo*	m_pInfo;		// Parent torrent handler
		QWORD			m_nOffset;		// File offset inside torrent (cached)

		CBTFile(const CBTInfo* pInfo, const CBTFile* pFile = NULL);
		void Serialize(CArchive& ar, int nVersion);

		friend class CBTInfo;
	};

// Subclass
public:
	class CBTTracker
	{
	public:
		CBTTracker(LPCTSTR szAddress = NULL, INT nTier = 0);
		CBTTracker(const CBTTracker& oSource);
		CBTTracker& operator=(const CBTTracker& oSource);

		bool operator==(const CBTTracker& oSource);

	private:
		CString		m_sAddress;
		DWORD		m_tLastAccess;
		DWORD		m_tLastSuccess;
		DWORD		m_tNextTry;
		DWORD		m_nFailures;
		INT			m_nTier;
		INT			m_nType;

		void Serialize(CArchive& ar, int nVersion);

		friend class CBTInfo;
	};

public:
	CStringList	m_sURLs;				// Add sources from torrents - DWK
	CStringList	m_oNodes;				// DHT nodes list
	CList< CBTFile* > m_pFiles;			// List of files
	Hashes::BtPureHash* m_pBlockBTH;
	DWORD		m_nBlockSize;
	DWORD		m_nBlockCount;
	QWORD		m_nTotalUpload;			// Total amount uploaded
	QWORD		m_nTotalDownload;		// Total amount downloaded
	UINT		m_nEncoding;
	CString		m_sComment;
	CString		m_sCreatedBy;
	DWORD		m_tCreationDate;
	BOOL		m_bPrivate;
	int 		m_nStartDownloads;		// When do we start downloads for this torrent
	DWORD		m_nTrackerSeeds;		// Count from most recent scrape
	DWORD		m_nTrackerPeers;		//
	DWORD 		m_nTrackerWait; 		// min_request_interval (in ms)

private:
	CArray< CBTTracker > m_oTrackers;	// Tracker list
	int			m_nTrackerIndex;		// The tracker we are currently using
	int			m_nTrackerMode;			// The current tracker situation
	DWORD		m_tTrackerScrape;		// For freshness
	bool		m_bEncodingError;		// Torrent has encoding errors
	CSHA		m_pTestSHA1;
	DWORD		m_nTestByte;
	CBuffer		m_pSource;
	DWORD		m_nInfoStart;
	DWORD		m_nInfoSize;

	BOOL		CheckFiles();

public:
	void		Clear();
	void		Serialize(CArchive& ar);
//	void		ConvertOldTorrents();	// Legacy Shareaza 2.4 Reference

	int			NextInfoPiece() const;
	BOOL		LoadInfoPiece(BYTE *pPiece, DWORD nPieceSize, DWORD nInfoSize, DWORD nInfoPiece);
	DWORD		GetInfoPiece(DWORD nPiece, BYTE **pInfoPiece) const;
	DWORD		GetInfoSize() const;
	BOOL		CheckInfoData();
	BOOL		IsDeadTracker(const CString& sTracker);		// Known defunct URLs
	BOOL		LoadTorrentFile(LPCTSTR pszFile);
	BOOL		LoadTorrentBuffer(const CBuffer* pBuffer);
	BOOL		LoadTorrentTree(const CBENode* pRoot);
	BOOL		SaveTorrentFile(const CString& sFolder);

	void		BeginBlockTest();
	void		AddToTest(LPCVOID pInput, DWORD nLength);
	BOOL		FinishBlockTest(DWORD nBlock);

	int			AddTracker(const CBTTracker& oTracker);
	void		RemoveAllTrackers();
	void		SetTrackerAccess(DWORD tNow);
	void		SetTrackerSucceeded(DWORD tNow);
	void		SetTrackerRetry(DWORD tTime);
	void		SetTrackerNext(DWORD tTime = 0);
	CString		GetTrackerAddress(int nTrackerIndex = -1) const;
	TRISTATE	GetTrackerStatus(int nTrackerIndex = -1) const;
	int			GetTrackerTier(int nTrackerIndex = -1) const;
	DWORD		GetTrackerNextTry() const;
	DWORD		GetTrackerFailures() const;
	void		OnTrackerFailure();

	//BOOL		ScrapeTracker();

	// Count of files
	inline INT_PTR GetCount() const
	{
		return m_pFiles.GetCount();
	}

	inline bool IsAvailable() const
	{
		return m_oBTH;
	}

	inline bool IsAvailableInfo() const
	{
		return IsAvailable() && m_nBlockSize && m_nBlockCount;
	}

	inline bool HasEncodingError() const
	{
		return m_bEncodingError;
	}

	inline bool IsMultiTracker() const
	{
		return ( m_nTrackerMode > tSingle ) && m_oTrackers.GetCount() > 1;
	}

	inline bool HasTracker() const
	{
		return ( m_nTrackerMode != tNull ) && ! m_oTrackers.IsEmpty();
	}

	inline int GetTrackerIndex() const
	{
		return m_nTrackerIndex;
	}

	void SetTracker(const CString& sTracker);

	void SetNode(const CString& sNode);

	inline int GetTrackerMode() const
	{
		return m_nTrackerMode;
	}

	void SetTrackerMode(int nTrackerMode);

	inline int GetTrackerCount() const
	{
		return (int)m_oTrackers.GetCount();
	}

	// Return hex-encoded SHA1 string of all tracker URLs for "lt_tex" extension
	CString GetTrackerHash() const;
};
