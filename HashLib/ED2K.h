//
// ED2K.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2006 and PeerProject 2008-2012
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "MD4.h"

const DWORD ED2K_PART_SIZE = 9500 * 1024u;


class HASHLIB_API CED2K
{
public:
	CED2K();
	~CED2K();

	void	Clear();
	void	Save(uchar* pBuf) const;
	void	Load(const uchar* pBuf);
	uint32	GetSerialSize() const;
	LPCVOID	GetRawPtr() const;

	void	GetRoot(__in_bcount(16) uchar* pHash) const;
	void	FromRoot(__in_bcount(16) const uchar* pHash);

	void	BeginFile(uint64 nLength);
	void	AddToFile(LPCVOID pInput, uint32 nLength);
	BOOL	FinishFile();

	void	BeginBlockTest();
	void	AddToTest(LPCVOID pInput, uint32 nLength);
	BOOL	FinishBlockTest(uint32 nBlock);

	BOOL	ToBytes(BYTE** ppOutput, uint32* pnOutput);	// To free ppOutput, use GlobalFree function
	BOOL	FromBytes(BYTE* pOutput, uint32 nOutput, uint64 nSize = 0);
	BOOL	CheckIntegrity();

	BOOL	IsAvailable() const;
	void	SetSize(uint32 nSize);
	uint32	GetSize() const;
	uint32	GetBlockCount() const;

private:
	CMD4::Digest m_pRoot;
	CMD4::Digest* m_pList;
	uint32	m_nList;
	uint32	m_nCurHash;
	uint32	m_nCurByte;
	CMD4	m_pSegment;
	bool	m_bNullBlock;
};
