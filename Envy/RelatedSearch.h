//
// RelatedSearch.h
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

#include "EnvyFile.h"
#include "Schema.h"

class CXMLElement;
class CMatchFile;
class CLibraryFile;


class CRelatedSearch : public CEnvyFile
{
public:
	CRelatedSearch(CMatchFile* pFile);
	CRelatedSearch(CLibraryFile* pFile);
	virtual ~CRelatedSearch();

public:
	CSchemaPtr		m_pSchema;
	CXMLElement*	m_pXML;
	BOOL			m_bXML;

public:
	BOOL		CanSearchForThis();
	BOOL		RunSearchForThis();
	BOOL		CanSearchForSimilar();
	BOOL		RunSearchForSimilar();
	BOOL		CanSearchForArtist();
	BOOL		RunSearchForArtist();
	BOOL		CanSearchForAlbum();
	BOOL		RunSearchForAlbum();
	BOOL		CanSearchForSeries();
	BOOL		RunSearchForSeries();
protected:
	static CString Tokenise(LPCTSTR psz);
};
