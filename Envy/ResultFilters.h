//
// ResultFilters.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2008
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
// Original Author: roo_koo_too@yahoo.com
//

#pragma once

// Create a class with some of the CMatchObjects members

class CFilterOptions
{
public:
	CFilterOptions();

	CString m_sName;	// The options set name
	CString	m_sFilter;
	BOOL	m_bFilterPush;
	BOOL	m_bFilterBusy;
	BOOL	m_bFilterUnstable;
	BOOL	m_bFilterLocal;
	BOOL	m_bFilterReject;
	BOOL	m_bFilterBogus;
	BOOL	m_bFilterDRM;
	BOOL	m_bFilterRestricted;
	BOOL	m_bFilterSuspicious;
	BOOL	m_bFilterAdult;
	BOOL	m_bRegExp;
	DWORD	m_nFilterSources;
	QWORD	m_nFilterMinSize;
	QWORD	m_nFilterMaxSize;

	void	Serialize(CArchive& ar, int nVersion);
};

class CResultFilters
{
public:
	CResultFilters();
	~CResultFilters();

private:
	mutable CCriticalSection m_pSection;

public:
	CFilterOptions **	m_pFilters;		// Array of filter options
	DWORD				m_nFilters;		// Count of filter options
	DWORD				m_nDefault;		// Index of the default filter options

	int  Search(const CString& strName) const;
	void Serialize(CArchive& ar);
	void Add(CFilterOptions *pOptions);
	void Remove(DWORD index);
	BOOL Load();
	BOOL Save();
	void Clear();
};

const DWORD NONE = ~0u;
