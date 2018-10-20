//
// LibraryDictionary.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2012
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

#include "SharedFile.h"

class CQueryHashTable;
class CQuerySearch;


class CLibraryDictionary
{
public:
	CLibraryDictionary();
	virtual ~CLibraryDictionary();

public:
	void		AddFile(CLibraryFile& oFile);
	void		RemoveFile(CLibraryFile& oFile);
	void		BuildHashTable();		// Build hash table if needed
	const CQueryHashTable*	GetHashTable();
	void		Invalidate();			// Force dictionary and hash table to re-build
	void		Clear();
	void		Serialize(CArchive& ar, int nVersion);
	CFileList*	Search(const CQuerySearch* pSearch, int nMaximum = 0, bool bLocal = false, bool bAvailableOnly = true);

	INT_PTR 	GetWordCount() const { return m_oWordMap.GetCount(); }	// For Debug Benchmark

private:
	typedef CMap< CString, const CString&, CFileList*, CFileList*& > CWordMap;

	CWordMap	m_oWordMap;
	CQueryHashTable* m_pTable;
	bool		m_bValid;				// Table is up to date
	DWORD		m_nSearchCookie;

	void		ProcessFile(CLibraryFile& oFile, bool bAdd, bool bCanUpload);
	void		ProcessPhrase(CLibraryFile& oFile, const CString& strPhrase, bool bAdd, bool bCanUpload);
	void		ProcessWord(CLibraryFile& oFile, const CString& strWord, bool bAdd, bool bCanUpload);
};

extern CLibraryDictionary LibraryDictionary;
