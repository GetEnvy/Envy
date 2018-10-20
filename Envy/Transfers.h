//
// Transfers.h
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

#include "ThreadImpl.h"

class CTransfer;


class CTransfers : public CThreadImpl
{
public:
	CTransfers();
	virtual ~CTransfers();

public:
	mutable CMutexEx m_pSection;

	BOOL	IsConnectedTo(const IN_ADDR* pAddress) const;
	BOOL	StartThread();
	void	StopThread();
	void	Add(CTransfer* pTransfer);
	void	Remove(CTransfer* pTransfer);

	INT_PTR	GetActiveCount() const;

private:
	CList< CTransfer* >	m_pList;
	DWORD	m_nRunCookie;

	void	OnRun();
	void	OnRunTransfers();
	void	OnCheckExit();
};

extern CTransfers Transfers;
