//
// Library.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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

#include "ComObject.h"
#include "ThreadImpl.h"
#include "SharedFile.h"

class CQueryHit;
class CQuerySearch;
class CLibraryFolder;
class CAlbumFolder;

#define LIBRARY_SER_VERSION		1000	// 29	(ToDo: Use INTERNAL_VERSION?)
// nVersion History:
// 27 - Changed CLibraryFile metadata saving order (ryo-oh-ki)
// 28 - Added CLibraryMaps m_pIndexMap, m_pNameMap and m_pPathMap counts (ryo-oh-ki)
// 29 - Added CLibraryDictionary serialize (ryo-oh-ki)
// 1000 - (Envy 1.0) (29)

class CLibrary :
	public CComObject,
	public CThreadImpl
{
	DECLARE_DYNAMIC(CLibrary)

public:
	CLibrary();
	virtual ~CLibrary();

public:
	mutable CMutexEx m_pSection;

protected:
//	int				m_nFileSwitch;			// Library next save .dat/.bak	(Using local static)
	volatile LONG	m_nUpdateCookie;		// Library cookie (ms)
	volatile LONG	m_nScanCookie;			// Used by CLibraryFolder::ThreadScan()
	volatile DWORD	m_nScanCount;			// Library scan counter
	volatile DWORD	m_nScanTime;			// Last library scan time (ms)
	volatile LONG	m_nForcedUpdate;		// Forced update request
	volatile LONG	m_nSaveCookie;			// Library last save cookie (ms)
	volatile DWORD	m_nSaveTime;			// Library last save time (ms)

// Sync Operations
public:
	inline DWORD GetCookie() const
	{
		return m_nUpdateCookie;
	}

	inline DWORD GetScanCookie()
	{
		return (DWORD)InterlockedIncrement( &m_nScanCookie );
	}

	inline DWORD GetScanCount() const
	{
		return m_nScanCount;
	}

	// Mark library as modified:
	// bForce = false	- Library has internal changes so it must be saved
	// bForce = true	- Library also has disk changes so it must be rescanned
	inline void Update(bool bForce = false)
	{
		InterlockedExchange( &m_nUpdateCookie, (LONG)GetTickCount() );

		if ( bForce )
			InterlockedExchange( &m_nForcedUpdate, TRUE );
	}

// File and Folder Operations
public:
	CAlbumFolder*	GetAlbumRoot();
	CLibraryFile*	LookupFile(DWORD nIndex, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	void			AddFile(CLibraryFile* pFile);
	void			RemoveFile(CLibraryFile* pFile);
	void			CheckDuplicates(LPCTSTR pszMD5Hash) const;

protected:
	void			CheckDuplicates(const CLibraryFile* pFile, bool bForce = false) const;

// General Operations
public:
	// Update library files alternate sources
	void			Clear();
	BOOL			Load();
	BOOL			Save();
	void			StopThread();
	bool			OnQueryHits(const CQueryHit* pHits);
	static BOOL		IsBadFile(LPCTSTR szFilenameOnly, LPCTSTR szPathOnly = NULL, DWORD dwFileAttributes = 0);
	CFileList*		Search(const CQuerySearch* pSearch, int nMaximum = 0, bool bLocal = false, bool bAvailableOnly = false);

protected:
	void			OnRun();
	void			Serialize(CArchive& ar);
	BOOL			SafeReadTime(CFile& pFile, FILETIME* pFileTime) throw();
	BOOL			SafeSerialize(CArchive& ar) throw();
	BOOL			ThreadScan();

// Automation
protected:
	BEGIN_INTERFACE_PART(Library, ILibrary)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get_Folders)(ILibraryFolders FAR* FAR* ppFolders);
		STDMETHOD(get_Albums)(IUnknown FAR* FAR* ppAlbums);
		STDMETHOD(get_Files)(ILibraryFiles FAR* FAR* ppFiles);
		STDMETHOD(FindByName)(BSTR sName, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(FindByPath)(BSTR sPath, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(FindByURN)(BSTR sURN, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(FindByIndex)(LONG nIndex, ILibraryFile FAR* FAR* ppFile);
	END_INTERFACE_PART(Library)

	DECLARE_INTERFACE_MAP()
};

extern CLibrary Library;

#include "LibraryList.h"
#include "LibraryMaps.h"
