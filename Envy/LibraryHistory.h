//
// LibraryHistory.h
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

#include "EnvyFile.h"

class CLibraryRecent;
class CLibraryFile;
class CDownload;


class CLibraryHistory
{
public:
	CLibraryHistory();
	~CLibraryHistory();

public:
	struct sTorrentDetails
	{
		CString			m_sName;
		CString			m_sPath;
		Hashes::BtHash  m_oBTH;
		DWORD			m_tLastSeeded;
		QWORD			m_nUploaded;
		QWORD			m_nDownloaded;
	};

	sTorrentDetails	LastSeededTorrent;		// Most recently seeded torrent (for home page button)
	sTorrentDetails	LastCompletedTorrent;	// Most recently completed torrent that didn't reach 100% ratio

	POSITION		GetIterator() const;
	CLibraryRecent*	GetNext(POSITION& pos) const;
	INT_PTR			GetCount() const { return m_pList.GetCount(); }
	void			Clear();

	BOOL			Check(CLibraryRecent* pRecent, int nScope = 0) const;
	void			Add(LPCTSTR pszPath, const CDownload* pDownload = NULL);
	void			Submit(CLibraryFile* pFile);
	void			OnFileDelete(CLibraryFile* pFile);
	void			Serialize(CArchive& ar, int nVersion);

protected:
	CList< CLibraryRecent* > m_pList;

	CLibraryRecent*	GetByPath(LPCTSTR pszPath) const;
	void			Prune();
};


class CLibraryRecent
{
protected:
	CLibraryRecent();
	CLibraryRecent(LPCTSTR pszPath, const CDownload* pDownload = NULL);

public:
	FILETIME					m_tAdded;
	CLibraryFile*				m_pFile;
	CString						m_sSources;
	CString						m_sPath;
	Hashes::Sha1ManagedHash		m_oSHA1;
	Hashes::TigerManagedHash	m_oTiger;
	Hashes::Md5ManagedHash		m_oMD5;
	Hashes::Ed2kManagedHash		m_oED2K;
	Hashes::BtManagedHash		m_oBTH;

protected:
	void	Serialize(CArchive& ar, int nVersion);

	friend class CLibraryHistory;
};

extern CLibraryHistory LibraryHistory;
