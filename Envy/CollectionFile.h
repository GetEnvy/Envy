//
// CollectionFile.h
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

#include "EnvyFile.h"

class CZIPFile;
class CXMLElement;
class CLibraryFile;


class CCollectionFile : public CComObject
{
	DECLARE_DYNAMIC(CCollectionFile)

public:
	CCollectionFile();
	virtual ~CCollectionFile();

public:
	enum CollectionType { EnvyCollection, SimpleCollection };

// Member File Class
public:
	class File : public CEnvyFile
	{
		public:
		File(CCollectionFile* pParent);
		virtual ~File();

		public:
		CCollectionFile*	m_pParent;
		CXMLElement*		m_pMetadata;
	//	CString				m_sSource;		// ToDo: Use sources ?

		public:
		BOOL	Parse(CXMLElement* pXML);	// Load from XML
		BOOL	Parse(CFile& pFile);		// Load from .emulecollection-file
		BOOL	Parse(LPCTSTR szText);		// Load from text line
		BOOL	IsComplete() const;
		BOOL	IsDownloading() const;
		BOOL	Download();
		BOOL	ApplyMetadata(CLibraryFile* pShared);
	};

protected:
	CList< File* >	m_pFiles;
	CString			m_sTitle;
	CString			m_sThisURI;
	CString			m_sParentURI;
	CXMLElement*	m_pMetadata;
	CollectionType	m_nType;

public:
	BOOL		Open(LPCTSTR lpszFileName);
	void		Close();
	void		Render(CString& strBuffer) const;	// Render file list as HTML

	File*		FindByURN(LPCTSTR pszURN);
	File*		FindFile(CLibraryFile* pShared, BOOL bApply = FALSE);
	int			GetMissingCount() const;

protected:
	BOOL		LoadCollection(LPCTSTR pszFile);	// Load zipped Envy collection
	BOOL		LoadEMule(LPCTSTR pszFile);			// Load binary eMule collection
	BOOL		LoadDC(LPCTSTR pszFile);			// Load DC++ file listing
	void		LoadDC(CXMLElement* pRoot);			// Load DC++ file listing
	BOOL		LoadText(LPCTSTR pszFile);			// Load simple text file with links
	static CXMLElement* CloneMetadata(CXMLElement* pMetadata);

// Inlines
public:
	inline BOOL IsOpen() const
	{
		return ( m_pFiles.GetCount() > 0 );
	}

	inline POSITION GetFileIterator() const
	{
		return m_pFiles.GetHeadPosition();
	}

	inline File* GetNextFile(POSITION& pos) const
	{
		return m_pFiles.GetNext( pos );
	}

	inline INT_PTR GetFileCount() const
	{
		return m_pFiles.GetCount();
	}

	inline CString GetTitle() const
	{
		return m_sTitle;
	}

	inline CString GetThisURI() const
	{
		return m_sThisURI;
	}

	inline CString GetParentURI() const
	{
		return m_sParentURI;
	}

	inline CXMLElement* GetMetadata() const
	{
		return m_pMetadata;
	}

	inline bool IsType(CollectionType nType) const
	{
		return m_nType == nType;
	}
};
