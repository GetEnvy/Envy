//
// SchemaChild.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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

#include "Schema.h"

class CSchema;
class CSchemaChild;
class CSchemaChildMap;
class CXMLElement;

typedef const CSchemaChildMap* CSchemaChildMapPtr;

class CSchemaChild
{
public:
	CSchemaChild(CSchemaPtr pSchema);
	~CSchemaChild();

public:
	CString		m_sURI;
	CSchema::SchemaType m_nType;

	BOOL		Load(const CXMLElement* pXML);
	BOOL		MemberCopy(CXMLElement* pLocal, CXMLElement* pRemote, BOOL bToRemote = FALSE, BOOL bAggressive = FALSE) const;

	inline INT_PTR GetCount() const { return m_pMap.GetCount(); }

protected:
	CSchemaPtr	m_pSchema;
	CList< CSchemaChildMapPtr > m_pMap;

	void		Clear();

private:
	CSchemaChild(const CSchemaChild&);
	CSchemaChild& operator=(const CSchemaChild&);
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

private:
	CSchemaChildMap(const CSchemaChildMap&);
	CSchemaChildMap& operator=(const CSchemaChildMap&);
};
