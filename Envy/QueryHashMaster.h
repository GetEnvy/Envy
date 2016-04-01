//
// QueryHashMaster.h
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

#include "QueryHashTable.h"

class CQueryHashGroup;


class CQueryHashMaster : public CQueryHashTable
{
public:
	CQueryHashMaster();
	virtual ~CQueryHashMaster();

protected:
	CList< CQueryHashGroup* > m_pGroups;
	int			m_nPerGroup;
	BOOL		m_bValid;

public:
	void		Create();
	void		Add(CQueryHashTable* pTable);
	void		Remove(CQueryHashTable* pTable);
	void		Build();

// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pGroups.GetHeadPosition();
	}

	inline CQueryHashGroup* GetNext(POSITION& pos) const
	{
		return m_pGroups.GetNext( pos );
	}

	inline INT_PTR GetCount() const
	{
		return m_pGroups.GetCount();
	}

	inline void Invalidate()
	{
		m_bValid = FALSE;
	}
};

extern CQueryHashMaster QueryHashMaster;
