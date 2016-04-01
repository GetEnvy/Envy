//
// HashDatabase.h
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

typedef struct
{
	DWORD nIndex;
	DWORD nOffset;
	DWORD nLength;
} HASHDB_INDEX_1000;

typedef struct
{
	DWORD nIndex;
	DWORD nType;
	DWORD nOffset;
	DWORD nLength;
} HASHDB_INDEX_1001, HASHDB_INDEX;

class CTigerTree;
class CED2K;


class CHashDatabase
{
public:
	CHashDatabase();
	~CHashDatabase();

public:
	BOOL	Create();
	void	Close();
	BOOL	DeleteAll(DWORD nIndex);

	BOOL	GetTiger(DWORD nIndex, CTigerTree* pTree);
	BOOL	StoreTiger(DWORD nIndex, CTigerTree* pTree);
	BOOL	DeleteTiger(DWORD nIndex);
	BOOL	GetED2K(DWORD nIndex, CED2K* pSet);
	BOOL	StoreED2K(DWORD nIndex, CED2K* pSet);
	BOOL	DeleteED2K(DWORD nIndex);

	static void Serialize(CArchive& ar, CTigerTree* pTree);
	static void Serialize(CArchive& ar, CED2K* pSet);

protected:
	CCriticalSection m_pSection;

	CString			m_sPath;
	CFile			m_pFile;
	BOOL			m_bOpen;

	HASHDB_INDEX*	m_pIndex;
	DWORD			m_nIndex;
	DWORD			m_nOffset;
	DWORD			m_nBuffer;

	HASHDB_INDEX*	Lookup(DWORD nIndex, DWORD nType);
	HASHDB_INDEX*	PrepareToStore(DWORD nIndex, DWORD nType, DWORD nLength);
	BOOL			Erase(DWORD nIndex, DWORD nType);
	BOOL			Commit();
};

extern CHashDatabase LibraryHashDB;
