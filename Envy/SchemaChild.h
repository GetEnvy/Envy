//
// SchemaChild.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2007
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

#include "Schema.h"

class CXMLElement;
class CSchemaChildMap;


class CSchemaChild
{
public:
	CSchemaChild(CSchemaPtr pSchema);
	virtual ~CSchemaChild();

public:
	CSchemaPtr	m_pSchema;
	CString		m_sURI;
	int			m_nType;

	CList< const CSchemaChildMap* >	m_pMap;

	BOOL		Load(const CXMLElement* pXML);
	BOOL		MemberCopy(CXMLElement* pLocal, CXMLElement* pRemote, BOOL bToRemote = FALSE, BOOL bAggressive = FALSE) const;
	void		Clear();
};


class CSchemaChildMap
{
public:
	CSchemaChildMap();
//	virtual ~CSchemaChildMap();

public:
	BOOL		m_bIdentity;
	CString		m_sLocal;
	CString		m_sRemote;

	BOOL		Load(const CXMLElement* pXML);
};
