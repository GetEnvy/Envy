//
// SchemaCache.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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


class CSchemaCache
{
public:
	CSchemaCache();
	~CSchemaCache();

public:
	int		Load();
	void	Clear();

// Inlines
public:
	POSITION GetIterator() const
	{
		return m_pURIs.GetStartPosition();
	}

	CSchemaPtr GetNext(POSITION& pos) const
	{
		return m_pURIs.GetNextValue( pos );
	}

	CSchemaPtr Get(LPCTSTR pszURI) const
	{
		if ( ! pszURI || ! *pszURI ) return NULL;

		CSchemaPtr pSchema;
		return ( m_pURIs.Lookup( pszURI, pSchema ) ) ? pSchema :
			( m_pAltURIs.Lookup( pszURI, pSchema ) ) ? pSchema :
			NULL;
	}

	CSchemaPtr GuessByFilename(LPCTSTR pszFile) const
	{
		if ( ! pszFile || ! *pszFile )
			return NULL;

		LPCTSTR pszExt = PathFindExtension( pszFile );
		if ( ! *pszExt )
			return NULL;

		CSchemaPtr pSchema;
		return m_pTypeFilters.Lookup( pszExt + 1, pSchema ) ? pSchema : NULL;
	}

	CSchemaPtr Guess(LPCTSTR pszName) const
	{
		if ( ! pszName || ! *pszName ) return NULL;

		// A quick hack for Limewire documents schema
		// ToDo: Remove it when the full schema mapping is verified
		//if ( strName == L"document" )
		//	return m_pNames.Lookup( L"wordprocessing", pSchema ) ? pSchema : NULL;

		CSchemaPtr pSchema;
		return m_pNames.Lookup( pszName, pSchema ) ? pSchema : NULL;
	}

	CString GetFilter(LPCTSTR pszURI) const;

	inline bool IsFilter(const CString& sType) const
	{
		CSchemaPtr pSchema;
		return m_pTypeFilters.Lookup( sType, pSchema );
	}

	// Detect schema and normilize resulting XML
	bool Normalize(CSchemaPtr& pSchema, CXMLElement*& pXML) const;

private:
	typedef CAtlMap< CString, CSchemaPtr, CStringElementTraitsI< CString > > CSchemaMap;
	CSchemaMap m_pURIs;
	CSchemaMap m_pAltURIs;
	CSchemaMap m_pNames;
	CSchemaMap m_pTypeFilters;	// Combined "file type":"schema"

	CSchemaCache(const CSchemaCache&);
	CSchemaCache& operator=(const CSchemaCache&);
};

extern CSchemaCache	SchemaCache;

// Compare two schema URIs with schema mapping
inline bool CheckURI(const CString& strURI1, LPCTSTR szURI2)
{
	if ( strURI1.CompareNoCase( szURI2 ) == 0 )
		return true;
	const CSchemaPtr pSchema1 = SchemaCache.Get( strURI1 );
	if ( pSchema1 && pSchema1->CheckURI( szURI2 ) )
		return true;
	const CSchemaPtr pSchema2 = SchemaCache.Get( szURI2 );
	if ( pSchema2 && pSchema2->CheckURI( strURI1 ) )
		return true;
	return false;
}
