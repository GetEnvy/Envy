//
// FragmentedFile.h
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

#include "EnvyFile.h"
#include "FileFragments.hpp"
#include "TransferFile.h"

class CEnvyFile;
class CBTInfo;
class CDownload;
class CDownloadTask;


class CFragmentedFile : public CObject
{
	DECLARE_DYNCREATE( CFragmentedFile )

public:
	CFragmentedFile();
protected:
	virtual ~CFragmentedFile();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Multifile Download priority settings
	// Sets unused PageTorrentFiles combobox order
	// Note: Changing this breaks torrents at startup
	enum { prUnwanted, prLow, prNormal, prHigh };

protected:
	class CVirtualFilePart : public CEnvyFile
	{
	public:
		CVirtualFilePart();
		CVirtualFilePart(const CVirtualFilePart& p);
		CVirtualFilePart& operator=(const CVirtualFilePart& p);

	public:
		void Release();

		inline bool operator ==(LPCTSTR pszFile) const
		{
			return ! m_sPath.CompareNoCase( pszFile );
		}

		inline bool operator <(const CVirtualFilePart& p) const
		{
			return ( m_nOffset < p.m_nOffset );
		}

		CTransferFile*	m_pFile;		// Opened file handler
		QWORD			m_nOffset;		// File offset (0 - for first/single file)
		BOOL			m_bWrite;		// File opened for write
		int				m_nPriority;	// Download priority (NotWanted, Low, Normal or High)
	};

	typedef std::vector< CVirtualFilePart > CVirtualFile;

	struct Less : public std::binary_function< CVirtualFilePart, CVirtualFilePart, bool >
	{
		inline bool operator()(const CVirtualFilePart& _Left, const CVirtualFilePart& _Right) const
		{
			return _Left < _Right;
		}
	};

	struct Greater : public std::binary_function< CVirtualFilePart, QWORD, bool >
	{
		inline bool operator()(const CVirtualFilePart& _Left, QWORD _Right) const
		{
			return _Left.m_nOffset > _Right;
		}
	};

	struct Flusher : public std::unary_function< CVirtualFilePart, void >
	{
		inline void operator()(const CVirtualFilePart& p) const
		{
			if ( p.m_pFile )
			{
				FlushFileBuffers( p.m_pFile->GetHandle() );
			}
		}
	};

	struct Releaser : public std::unary_function< CVirtualFilePart, void >
	{
		inline void operator()(CVirtualFilePart& p) const
		{
			if ( p.m_pFile )
			{
				p.m_pFile->Release();
				p.m_pFile = NULL;
			}
		}
	};

	struct Completer : public std::unary_function< CVirtualFilePart, void >
	{
		inline void operator()(const CVirtualFilePart& p) const
		{
			if ( p.m_pFile )
			{
				HANDLE hFile = p.m_pFile->GetHandle( TRUE );
				if ( hFile != INVALID_HANDLE_VALUE )
				{
					LARGE_INTEGER nLength;
					nLength.QuadPart = p.m_nSize;
					SetFilePointerEx( hFile, nLength, NULL, FILE_BEGIN );
					SetEndOfFile( hFile );
				}
			}
		}
	};

	struct EnsureWriter : public std::unary_function< CVirtualFilePart, bool >
	{
		inline bool operator()(const CVirtualFilePart& p) const
		{
			return ! p.m_pFile || p.m_pFile->EnsureWrite();
		}
	};

	mutable CCriticalSection	m_pSection;
	CVirtualFile				m_oFile;
	Fragments::List				m_oFList;
	QWORD						m_nUnflushed;
	DWORD						m_nFileError;
	CString						m_sFileError;
	volatile LONG				m_dwRef;
	const CDownload*			m_pDownload;	// Reference download object (optional)

	BOOL	VirtualRead(QWORD nOffset, char* pBuffer, QWORD nBuffer, QWORD* pnRead);
	BOOL	VirtualWrite(QWORD nOffset, const char* pBuffer, QWORD nBuffer, QWORD* pnWritten);

	// Get completed size of defined range (in bytes)
	QWORD	GetCompleted(QWORD nOffset, QWORD nLength) const;

public:
	// By hash from library: Open file from disk or create file inside incomplete folder
	BOOL	Open(const CEnvyFile* pEnvyFile, BOOL bWrite);
	// By .torrent: Open file from disk or create file inside incomplete folder file(s)
	BOOL	Open(const CBTInfo& oInfo, BOOL bWrite);
protected:
	// By path: Open file from disk
	BOOL	Open(LPCTSTR pszFile, QWORD nOffset = 0, QWORD nLength = SIZE_UNKNOWN, BOOL bWrite = FALSE, LPCTSTR pszName = NULL, int nPriority = prNormal );

public:
	void	SetDownload(const CDownload* pDownload);
	ULONG	AddRef();
	ULONG	Release();
	BOOL	Flush();				// Flush unsaved data to disk
	void	Close();				// Close all subfiles
	BOOL	SetSize(QWORD nSize);	// Set new file size
	BOOL	MakeComplete();
	void	Serialize(CArchive& ar, int nVersion);
	BOOL	EnsureWrite();
	void	Delete();				// Delete file(s)
	// Move file to destination. Returns 0 on success or file error number.
	DWORD	Move(DWORD nIndex, LPCTSTR pszDestination, LPPROGRESS_ROUTINE lpProgressRoutine = NULL, CDownloadTask* pTask = NULL);
	BOOL	Write(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten = NULL);
	BOOL	Read(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead = NULL);
	QWORD	InvalidateRange(QWORD nOffset, QWORD nLength);
	// Check if specified file handled
	BOOL	FindByPath(const CString& sPath) const;

	// Get amount of files
	inline DWORD GetCount() const
	{
		return (DWORD)m_oFile.size();
	}

	// Get subfile
	inline const CEnvyFile* GetAt(DWORD nIndex) const
	{
		CQuickLock oLock( m_pSection );

		return nIndex < m_oFile.size() ?
			static_cast< const CEnvyFile* >( &m_oFile[ nIndex ] ) : NULL;
	}

	// Are all of subfiles open?
	BOOL IsOpen() const;

	// Get subfile offset
	QWORD GetOffset(DWORD nIndex) const;

	// Get subfile length
	QWORD GetLength(DWORD nIndex) const;

	// Get subfile path
	CString GetPath(DWORD nIndex) const;

	// Set subfile path
	void SetPath(DWORD nIndex, LPCTSTR szPath);

	// Select subfile (with user interaction)
	int SelectFile(CSingleLock* pLock = NULL) const;

	// Get subfile original name
	CString GetName(DWORD nIndex) const;

	// Set subfile original name
	void SetName(DWORD nIndex, LPCTSTR szName);

	// Get subfile priority
	int GetPriority(DWORD nIndex) const;

	// Set subfile priority
	void SetPriority(DWORD nIndex, int nPriority);

	// Get last file/disk error
	inline DWORD GetFileError() const
	{
		return m_nFileError;
	}

	// Get last file/disk error message
	inline const CString& GetFileErrorString() const
	{
		return m_sFileError;
	}

	// Is file has size?
	inline BOOL IsValid() const
	{
		return m_oFList.limit() > 0;
	}

	// Returns file download progress ( < 0 - unknown or 0..100% )
	float GetProgress(DWORD nIndex) const;

	// Get total size of whole file (in bytes)
	inline QWORD GetTotal() const
	{
	//	CQuickLock oLock( m_pSection );

		return m_oFList.limit();
	}

	inline QWORD GetRemaining() const
	{
		//	CQuickLock oLock( m_pSection );

		if ( m_oFList.limit() == 0 || m_oFList.limit() == SIZE_UNKNOWN && m_oFList.length_sum() )
			return SIZE_UNKNOWN;

		return m_oFList.length_sum();
	}

	// Get list of all fragments which must be downloaded
	Fragments::List GetFullFragmentList() const;

	// Get list of empty fragments
	inline Fragments::List GetEmptyFragmentList() const
	{
	//	CQuickLock oLock( m_pSection );		// High Contention?

		return m_oFList;
	}

	// Get list of empty fragments we really want to download
	Fragments::List GetWantedFragmentList() const;

	inline BOOL IsPositionRemaining(QWORD nOffset) const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.has_position( nOffset );
	}

	inline BOOL DoesRangeOverlap(QWORD nOffset, QWORD nLength) const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.overlaps( Fragments::Fragment( nOffset, nOffset + nLength ) );
	}

	inline QWORD GetRangeOverlap(QWORD nOffset, QWORD nLength) const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.overlapping_sum( Fragments::Fragment( nOffset, nOffset + nLength ) );
	}

	// Get completed size of whole file (in bytes)
	inline QWORD GetCompleted() const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.missing();
	}

	// Get completed size of subfile (in bytes)
	QWORD GetCompleted(DWORD nIndex) const;

	// Is download complete?
	inline bool IsComplete() const
	{
		CQuickLock oLock( m_pSection );

		return IsValid() && GetWantedFragmentList().empty();
	}

//	inline QWORD GetEmptyFragmentCount() const
//	{
//		CQuickLock oLock( m_pSection );
//
//		return m_oFList.size();
//	}

//	inline BOOL IsFlushNeeded() const
//	{
//		CQuickLock oLock( m_pSection );
//
//		return ( m_pFile != NULL ) && ( m_nUnflushed > 0 );
//	}
};

// For augment::auto_ptr< CFragmentedFile >
// Defined in Augment/auto_ptr.hpp, was boost::checked_delete
template<>
inline void augment::checked_delete< CFragmentedFile >(CFragmentedFile* x)
{
	if ( x ) x->Release();
}
