//
// DownloadGroup.h
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

class CDownload;


class CDownloadGroup
{
public:
	CDownloadGroup(const LPCTSTR szName = NULL, const BOOL bTemporary = FALSE);
	virtual ~CDownloadGroup();

protected:
	CList< CDownload* >	m_pDownloads;		// List of linked downloads
	TRISTATE			m_bTemporary;		// Temporary group:
	//  TRI_UNKNOWN	- Persistent group;
	//	TRI_FALSE	- Temporary group, not completed yet;
	//	TRI_TRUE	- Temporary group, feel free to delete.

public:
	CString				m_sName;			// Group name
	CString				m_sSchemaURI;		// Default schema (used to fill filters list)
	CString				m_sFolder;			// Folder for completed downloads
	CList< CString >	m_pFilters;			// Filters list
	int					m_nImage;			// 16x16 group icon
	BOOL				m_bRemoteSelected;	// Active(selected) group for Remote Interface
	//BOOL				m_bTorrent;			// Filter BitTorrent downloads (Obsolete, Schema is detected)

public:
	void		Add(CDownload* pDownload);
	void		Remove(CDownload* pDownload);
	void		Clear();
	void		SetCookie(int nCookie);
	void		CopyList(CList< CDownload* >& pList);
	BOOL		Link(CDownload* pDownload);
	int			LinkAll();
	void		AddFilter(const CString& strFilter);
	void		RemoveFilter(const CString& strFilter);
	void		SetSchema(LPCTSTR pszURI, BOOL bRemoveOldFilters = FALSE);
	void		SetFolder(LPCTSTR pszFolder);
	void		Serialize(CArchive& ar, const int nVersion);
	BOOL		IsTemporary();
	void		SetDefaultFilters();		// Load file extensions from schema

// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pDownloads.GetHeadPosition();
	}

	inline CDownload* GetNext(POSITION& pos) const
	{
		return m_pDownloads.GetNext( pos );
	}

	inline BOOL Contains(CDownload* pDownload) const
	{
		return m_pDownloads.Find( pDownload ) != NULL;
	}

	inline INT_PTR GetCount() const
	{
		return m_pDownloads.GetCount();
	}
};
