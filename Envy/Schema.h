//
// Schema.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2015
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

class CSchema;
class CSchemaChild;
class CSchemaBitprints;
class CXMLElement;

typedef const CSchema* CSchemaPtr;
typedef const CSchemaChild* CSchemaChildPtr;

#ifdef WIN64

template<>
AFX_INLINE UINT AFXAPI HashKey(CSchemaPtr key)
{
	return HashKey< __int64 >( (__int64)key );
}

#endif // _WIN64

#include "SchemaMember.h"


class CSchema
{
public:
	CSchema();
	~CSchema();

public:
	enum SchemaType { typeAny = -1, typeFile, typeFolder };
	enum SchemaLevel { saDefault, saAdvanced, saSystem, saMax };

	SchemaType		m_nType;
	SchemaLevel		m_nAvailability;
	CString			m_sTitle;
	CString			m_sPlural;
	CString			m_sSingular;
	CString			m_sURIMapping;
	CString			m_sDonkeyType;
	BOOL			m_bPrivate;

	CList< CString >		m_pExtends;
	CList< CSchemaMember* >	m_pMembers;
	CList< CSchemaChildPtr >	m_pContains;
	CList< CSchemaBitprints* >	m_pBitprintsMap;
	CString			m_sDefaultColumns;
	CString			m_sBitprintsTest;
	CString			m_sLibraryView;
	CString			m_sHeaderTitle;
	CString			m_sHeaderSubtitle;
	CString			m_sTileLine1;
	CString			m_sTileLine2;

	CString			m_sIcon;
	int				m_nIcon16;
	int				m_nIcon32;
	int				m_nIcon48;

public:
	void			Clear();
	BOOL			Load(LPCTSTR pszName);
	CXMLElement*	Instantiate(BOOL bNamespace = FALSE) const;
	BOOL			Validate(CXMLElement* pXML, BOOL bFix) const;

	BOOL			FilterType(LPCTSTR pszFile) const;
	CString			GetFilterSet() const;
	POSITION		GetFilterIterator() const;
	void			GetNextFilter(POSITION& pos, CString& sType, BOOL& bResult) const;
	POSITION		GetMemberIterator() const;
	CSchemaMemberPtr GetNextMember(POSITION& pos) const;
	CSchemaMemberPtr GetMember(LPCTSTR pszName) const;
	INT_PTR			GetMemberCount() const;
	CString			GetFirstMemberName() const;
	const CXMLElement* GetType(const CXMLElement* pRoot, LPCTSTR pszName) const;
	CSchemaChildPtr	GetContained(LPCTSTR pszURI) const;
	CString			GetContainedURI(SchemaType nType) const;
	CString			GetIndexedWords(const CXMLElement* pXML) const;
	CString			GetVisibleWords(const CXMLElement* pXML) const;
	void			ResolveTokens(CString& str, CXMLElement* pXML) const;

protected:
	typedef CAtlMap < CString, BOOL, CStringElementTraitsI< CString > > CStringBoolMap;
	CStringBoolMap	m_pTypeFilters;
	CString			m_sURI;

	BOOL			LoadSchema(LPCTSTR pszFile);
	BOOL			LoadPrimary(const CXMLElement* pRoot, const CXMLElement* pType);
	BOOL			LoadDescriptor(LPCTSTR pszFile);
	void			LoadDescriptorTitles(const CXMLElement* pElement);
	void			LoadDescriptorIcons(const CXMLElement* pElement);
	void			LoadDescriptorMembers(const CXMLElement* pElement);
	void			LoadDescriptorTypeFilter(const CXMLElement* pElement);
	void			LoadDescriptorExtends(const CXMLElement* pElement);
	void			LoadDescriptorContains(const CXMLElement* pElement);
	void			LoadDescriptorHeader(const CXMLElement* pElement);
	void			LoadDescriptorView(const CXMLElement* pElement);
	void			LoadDescriptorBitprintsImport(const CXMLElement* pElement);
	BOOL			LoadIcon(CString sPath = L"");

	CSchemaMember*	GetWritableMember(LPCTSTR pszName) const;

// Inlines
public:
	inline CString GetURI() const
	{
		return m_sURI;
	}

	inline bool Equals(CSchemaPtr pSchema) const
	{
		return ( pSchema && ( ( this == pSchema ) || CheckURI( pSchema->m_sURI ) ) );
	}

	inline bool CheckURI(LPCTSTR pszURI) const
	{
		if ( ! pszURI ) return false;
		if ( m_sURI.CompareNoCase( pszURI ) == 0 ) return true;
		for ( POSITION pos = m_pExtends.GetHeadPosition(); pos; )
		{
			if ( m_pExtends.GetNext( pos ).CompareNoCase( pszURI ) == 0 ) return true;
		}
		return false;
	}

// Common Schemas
public:
	static LPCTSTR	uriApplication;
	static LPCTSTR	uriArchive;
	static LPCTSTR	uriAudio;
	static LPCTSTR	uriBook;
	static LPCTSTR	uriImage;
	static LPCTSTR	uriVideo;
	static LPCTSTR	uriROM;
	static LPCTSTR	uriDocument;
	static LPCTSTR	uriSourceCode;
	static LPCTSTR	uriSpreadsheet;
	static LPCTSTR	uriPresentation;
	static LPCTSTR	uriCollection;
	static LPCTSTR	uriLibrary;
	static LPCTSTR	uriFolder;
	static LPCTSTR	uriAllFiles;

	static LPCTSTR	uriCollectionsFolder;
	static LPCTSTR	uriFavoritesFolder;
	static LPCTSTR	uriSearchFolder;

	static LPCTSTR	uriUnsorted;
	static LPCTSTR	uriUnsortedFolder;
	static LPCTSTR	uriBitTorrent;
	static LPCTSTR	uriBitTorrentFolder;
	static LPCTSTR	uriGhostFolder;
	//static LPCTSTR uriComments;
	//static LPCTSTR uriSkin;

	static LPCTSTR	uriApplicationFolder;
	static LPCTSTR	uriArchiveFolder;
	static LPCTSTR	uriAudioFolder;
	static LPCTSTR	uriVideoFolder;
	static LPCTSTR	uriImageFolder;
	static LPCTSTR	uriDocumentFolder;

	// Legacy sorted subfolders removed. (Root/All/Collection)

private:
	CSchema(const CSchema&);
	CSchema& operator=(const CSchema&);
};


class CSchemaBitprints
{
public:
	CString		m_sFrom;
	CString		m_sTo;
	double		m_nFactor;

	BOOL		Load(const CXMLElement* pXML);
};


#define NO_VALUE		(L"(~ns~)")
#define MULTI_VALUE		(L"(~mt~)")
