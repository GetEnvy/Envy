//
// LibraryBuilder.h
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
#include "LibraryBuilderInternals.h"
#include "LibraryBuilderPlugins.h"

class CLibraryFile;
class CXMLElement;


class CFileHash
{
public:
	CFileHash(QWORD nFileSize);

	void Add(const void* pBuffer, DWORD nBlock);
	void Finish();
	void CopyTo(CLibraryFile* pFile) const;

protected:
	CTigerTree	m_pTiger;
	CED2K		m_pED2K;
	CSHA		m_pSHA1;
	CMD5		m_pMD5;
};

class CLibraryBuilder :
	public CLibraryBuilderInternals
,	public CLibraryBuilderPlugins
,	public CThreadImpl
{
public:
	CLibraryBuilder();
//	virtual ~CLibraryBuilder();

public:
//	bool		m_bBusy;							// Something "IsMoving"

	bool		Add(const CLibraryFile* pFile);		// Add file to list
	void		Remove(const CLibraryFile* pFile);	// Remove file from list
	void		Remove(LPCTSTR szPath);				// Remove file from list
	void		Remove(DWORD nIndex);				// Remove file from list
	void		RequestPriority(LPCTSTR pszPath);	// Place file to the begin of list
	void		Skip(DWORD nIndex);					// Move file to the end of list
	void		StopThread();
	void		BoostPriority(bool bPriority);
	bool		GetBoostPriority() const;

	CString		GetCurrent() const;					// Hashing filename
	size_t		GetRemaining() const;				// Hashing queue size
	float		GetProgress() const;				// Hashing file progress (0..100%)

	int			SubmitMetadata(DWORD nIndex, LPCTSTR pszSchemaURI, CXMLElement* pXML);
	bool		SubmitCorrupted(DWORD nIndex);

	bool		RefreshMetadata(const CString& sPath);

private:
	class CFileInfo
	{
	public:
		CFileInfo(DWORD index = 0ul) :
			nIndex			( index )
		,	nNextAccessTime	( 0ull )
		{
		}
		CFileInfo(const CFileInfo& oFileInfo) :
			nIndex			( oFileInfo.nIndex )
		,	nNextAccessTime	( oFileInfo.nNextAccessTime )
		{
		}
		bool operator==(const CFileInfo& oFileInfo) const
		{
			return ( nIndex == oFileInfo.nIndex );
		}
		DWORD		nIndex;						// Library file index
		QWORD		nNextAccessTime;			// Next access time
	};
	typedef std::list< CFileInfo > CFileInfoList;

	mutable CMutex	m_pSection;					// Guarding
	CFileInfoList	m_pFiles;					// File list
	CString			m_sPath;					// Hashing filename
	bool			m_bPriority;				// Fast/Slow Hash Speed
	float			m_nProgress;				// Hashing file progress (0. - 100.0%)
	LARGE_INTEGER	m_nLastCall;				// (ticks)
	LARGE_INTEGER	m_nFreq;					// (Hz)
	QWORD			m_nReaded;					// (bytes)
	__int64			m_nElapsed;					// (mks)
	CEvent			m_oSkip;					// Request to skip hashing file

	// Get next file from list doing all possible tests
	// Returns 0 if no file available, sets m_sPath to current file and sets thread cancel event if no files left.
	DWORD		GetNextFileToHash();			// Sets m_sPath
	void		OnRun();
	bool		HashFile(LPCTSTR szPath, HANDLE hFile);
	bool		DetectVirtualFile(LPCTSTR szPath, HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	bool		DetectVirtualID3v1(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	bool		DetectVirtualID3v2(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	bool		DetectVirtualLAME(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	bool		DetectVirtualAPEHeader(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	bool		DetectVirtualAPEFooter(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	bool		DetectVirtualLyrics(HANDLE hFile, QWORD& nOffset, QWORD& nLength);

	inline bool	IsSkipped() { return WaitForSingleObject( m_oSkip, 0 ) != WAIT_TIMEOUT; }

	inline int	GetVbrHeaderOffset(int nId, int nMode)
	{
		int nOffset = 0;
		if ( nId )	// MPEG1
		{
			if ( nMode != 3 )
				nOffset = 32 + 4;
			else
				nOffset = 17 + 4;
		}
		else		// MPEG2
		{
			if ( nMode != 3 )
				nOffset = 17 + 4;
			else
				nOffset = 9 + 4;
		}
		return nOffset;
	}
};

extern CLibraryBuilder LibraryBuilder;
