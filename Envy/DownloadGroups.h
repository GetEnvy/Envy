//
// DownloadGroups.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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

class CDownload;
class CDownloadGroup;


class CDownloadGroups
{
public:
	CDownloadGroups();
	virtual ~CDownloadGroups();

public:
	CCriticalSection	m_pSection;
protected:
	CList< CDownloadGroup* > m_pList;
	CDownloadGroup*		m_pSuper;
	int					m_nBaseCookie;
	int					m_nSaveCookie;
	int					m_nGroupCookie;

public:
	CDownloadGroup*		GetSuperGroup();
	CDownloadGroup*		Add(LPCTSTR pszName = NULL, BOOL bTemporary = FALSE, BOOL bUseExisting = FALSE);
	void				Remove(CDownloadGroup* pGroup);
	void				MoveRight(CDownloadGroup* pGroup);
	void				MoveLeft(CDownloadGroup* pGroup);
	void				Link(CDownload* pDownload);
	void				Unlink(CDownload* pDownload, BOOL bAndSuper = TRUE);
	void				CreateDefault();
	CString				GetCompletedPath(CDownload* pDownload);
public:
	void				Clear();
	BOOL				Load();
	BOOL				Save(BOOL bForce = TRUE);
protected:
	void				Serialize(CArchive& ar);
	void				CleanTemporary();

// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CDownloadGroup* GetNext(POSITION& pos) const
	{
		return m_pList.GetNext( pos );
	}

	inline INT_PTR GetCount() const
	{
		return m_pList.GetCount();
	}

	inline BOOL Check(CDownloadGroup* pGroup) const
	{
		return m_pList.Find( pGroup ) != NULL;
	}

	inline int GetGroupCookie() const
	{
		return m_nGroupCookie;
	}

	inline void IncBaseCookie()
	{
		m_nBaseCookie ++;
	}

private:
	CDownloadGroups(const CDownloadGroups&);
	CDownloadGroups& operator=(const CDownloadGroups&);
};

extern CDownloadGroups DownloadGroups;
