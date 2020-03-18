//
// DownloadWithTiger.h
//
// This file is part of Envy (getenvy.com) © 2016-2020
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

#include "DownloadWithTorrent.h"

class CDownloadTransfer;


class CDownloadWithTiger : public CDownloadWithTorrent
{
protected:
	CDownloadWithTiger();
	virtual ~CDownloadWithTiger();

private:
	CTigerTree	m_pTigerTree;
	BYTE*		m_pTigerBlock;
	DWORD		m_nTigerBlock;
	DWORD		m_nTigerSize;
	DWORD		m_nTigerSuccess;

	CED2K		m_pHashset;
	BYTE*		m_pHashsetBlock;
	DWORD		m_nHashsetBlock;
	DWORD		m_nHashsetSuccess;

	DWORD		m_nVerifyCookie;
	int			m_nVerifyHash;
	DWORD		m_nVerifyBlock;
	QWORD		m_nVerifyOffset;
	QWORD		m_nVerifyLength;
	DWORD		m_tVerifyLast;

	mutable CMutexEx		m_pTigerSection;

	mutable Fragments::List	m_oWantedListCache;			// Wanted fragment list cache, was m_oWFLCache
	mutable QWORD			m_nWantedListCookie;		// Wanted fragment list last modified size cookie, was m_nWFLCookie
	mutable DWORD			m_nWantedListTime;			// Wanted fragment list last modified time

public:
	const CTigerTree* GetTigerTree() const;
	const CED2K* GetHashset() const;
	BOOL		SetTigerTree(BYTE* pTiger, DWORD nTiger, BOOL bLevel1 = FALSE);
	BOOL		SetHashset(BYTE* pSource, DWORD nSource);
	BOOL		NeedTigerTree() const;
	BOOL		NeedHashset() const;

	void		ResetVerification();
	void		ClearVerification();
	void		RunValidation();
	bool		RunMergeFile(LPCTSTR szFilename, BOOL bMergeValidation, const Fragments::List& oMissedGaps, CDownloadTask* pTask, float fProgress);
	BOOL		GetNextVerifyRange(QWORD& nOffset, QWORD& nLength, BOOL& bSuccess, int nHash = HASH_NULL) const;
	DWORD		GetVerifyLength(PROTOCOLID nProtocol = PROTOCOL_ANY, int nHash = HASH_NULL) const;

	// Get list of empty fragments we really want
	Fragments::List GetWantedFragmentList() const;

	BOOL		GetFragment(CDownloadTransfer* pTransfer);	// Select a fragment for a transfer
	BOOL		AreRangesUseful(const Fragments::List& oAvailable) const;
	BOOL		IsRangeUseful(QWORD nOffset, QWORD nLength) const;
	BOOL		IsRangeUsefulEnough(CDownloadTransfer* pTransfer, QWORD nOffset, QWORD nLength) const;
	// Check range against fragments list got from GetWantedFragmentList() call
	// UsefulEnough() is like IsRangeUseful() but takes the amount of useful ranges,
	// relative to the amount of garbage and source speed into account

protected:
	bool		IsFullyVerified() const;

	virtual void	Serialize(CArchive& ar, int nVersion);

private:
	DWORD		GetValidationCookie() const;
	BOOL		FindNewValidationBlock(int nHash);
	void		ContinueValidation();
	void		FinishValidation();
	void		SubtractHelper(Fragments::List& ppCorrupted, BYTE* pBlock, QWORD nBlock, QWORD nSize);

	// Get list of all fragments which must be downloaded
	// but rounded to nearest smallest hash block (torrent, tiger or ed2k)
	Fragments::List GetHashableFragmentList() const;

	// Get list of possible download fragments
	Fragments::List GetPossibleFragments(const Fragments::List& oAvailable, Fragments::Fragment& oLargest);

	friend class CEDClient; 	// AddSourceED2K && m_nHashsetBlock && m_pHashsetBlock
	friend class CDownloadTipCtrl;
};
