//
// UploadFile.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
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

#include "FileFragments.hpp"
#include "EnvyFile.h"

class CUploadTransfer;

class CUploadFile : public CEnvyFile
{
public:
	CUploadFile(CUploadTransfer* pUpload, const Hashes::Sha1Hash& oSHA1, LPCTSTR pszName, LPCTSTR pszPath, QWORD nSize);
	virtual ~CUploadFile();

public:
	Fragments::List		m_oFragments;
	IN_ADDR				m_pAddress;
	DWORD				m_nRequests;
	BOOL				m_bSelected;

	void				Add(CUploadTransfer* pUpload);
	BOOL				Remove(CUploadTransfer* pUpload);
	CUploadTransfer*	GetActive() const;
	void				AddFragment(QWORD nOffset, QWORD nLength);
	void				Remove();

	inline BOOL IsEmpty() const
	{
		return m_pTransfers.IsEmpty();
	}

protected:
	CList< CUploadTransfer* >	m_pTransfers;
};
