//
// QueryHashTable.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2006 and PeerProject 2008-2014
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

class CBuffer;
class CPacket;
class CNeighbour;
class CEnvyFile;
class CQueryHashGroup;
class CQuerySearch;
class CXMLElement;


class CQueryHashTable
{
public:
	CQueryHashTable();
	virtual ~CQueryHashTable();

public:
	bool				m_bLive;
	DWORD				m_nCookie;
	BYTE*				m_pHash;
	DWORD				m_nHash;
	DWORD				m_nBits;
	DWORD				m_nInfinity;
	DWORD				m_nCount;
	CBuffer*			m_pBuffer;
	CQueryHashGroup*	m_pGroup;

public:
	// Split phrase to keywords
	static void		MakeKeywords(const CString& strPhrase, CStringList& oKeywords);
	static DWORD	HashWord(LPCTSTR pszString, size_t nStart, size_t nLength, DWORD nBits);

protected:
	// Split word to keywords (Katakana/Hiragana/Kanji helper)
	static void		MakeKeywords(const CString& strWord, WORD nWordType, CStringList& oKeywords);

public:
	void	Create();
	void	Clear();
	bool	Merge(const CQueryHashTable* pSource);
	bool	Merge(const CQueryHashGroup* pSource);
	bool	PatchTo(const CQueryHashTable* pTarget, CNeighbour* pNeighbour);
	bool	OnPacket(CPacket* pPacket);
	void	AddFile(const CEnvyFile& oFile);		// Add filename and hashes split on keywords
	void	AddHashes(const CEnvyFile& oFile);	// Add file hashes
	void	AddExactString(const CString& strString);	// Add string exactly
	DWORD	HashWord(LPCTSTR pszString, size_t nStart, size_t nLength) const;	// Hash string (MUST BE LOWERCASED)
	bool	CheckHash(const DWORD nHash) const;
	bool	Check(const CQuerySearch* pSearch) const;
	int		GetPercent() const;
	void	Draw(HDC hDC, const RECT* pRC);
protected:
	bool	OnReset(CPacket* pPacket);
	bool	OnPatch(CPacket* pPacket);
//	bool	PatchToOldShareaza(const CQueryHashTable* pTarget, CNeighbour* pNeighbour);
};
