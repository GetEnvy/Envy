//
// AlbumFolder.h
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

#include "Schema.h"

class CCollectionFile;
class CLibraryFile;
class CLibraryList;
class CSchemaMember;
class CXMLElement;


class CAlbumFolder
{
public:
	CAlbumFolder(CAlbumFolder* pParent = NULL, LPCTSTR pszSchemaURI = NULL, LPCTSTR pszName = NULL, BOOL bAutoDelete = FALSE);
	virtual ~CAlbumFolder();

public:
	CString				m_sSchemaURI;
	CSchemaPtr			m_pSchema;
	CXMLElement*		m_pXML;
	Hashes::Sha1Hash	m_oCollSHA1;
	CString				m_sName;
	CString				m_sBestView;
	BOOL				m_bExpanded;
	BOOL				m_bAutoDelete;
	DWORD				m_nUpdateCookie;
	DWORD				m_nSelectCookie;
	DWORD				m_nListCookie;
	Hashes::Guid		m_oGUID;

protected:
	CAlbumFolder*		m_pParent;
	CList< CAlbumFolder* >	m_pFolders;
	CList< CLibraryFile* >	m_pFiles;
	CCollectionFile*	m_pCollection;

public:
	void				AddFolder(CAlbumFolder* pFolder);
	CAlbumFolder*		AddFolder(LPCTSTR pszSchemaURI = NULL, LPCTSTR pszName = NULL, BOOL bAutoDelete = FALSE);
	POSITION			GetFolderIterator() const;
	CAlbumFolder*		GetParent() const;
	CAlbumFolder*		GetNextFolder(POSITION& pos) const;
	CAlbumFolder*		GetFolder(LPCTSTR pszName) const;
	CAlbumFolder*		GetFolderByURI(LPCTSTR pszURI) const;
	DWORD				GetFolderCount() const;
	BOOL				CheckFolder(CAlbumFolder* pFolder, BOOL bRecursive = FALSE) const;
	CAlbumFolder*		GetTarget(CSchemaMember* pMember, LPCTSTR pszValue) const;
	CAlbumFolder*		FindCollection(const Hashes::Sha1Hash& oSHA1);
	CAlbumFolder*		FindFolder(const Hashes::Guid& oGUID);

	void				AddFile(CLibraryFile* pFile);
	void				RemoveFile(CLibraryFile* pFile);
	const CAlbumFolder*	FindFile(const CLibraryFile* pFile) const;
	POSITION			GetFileIterator() const;
	CLibraryFile*		GetNextFile(POSITION& pos) const;
	DWORD				GetSharedCount(BOOL bRecursive = FALSE) const;
	DWORD				GetFileCount(BOOL bRecursive = FALSE) const;
	QWORD				GetFileVolume(BOOL bRecursive = FALSE) const;
	DWORD				GetFileList(CLibraryList* pList, BOOL bRecursive) const;

	void				Clear();
	void				Delete(BOOL bIfEmpty = FALSE);
	BOOL				SetMetadata(CXMLElement* pXML);
//	BOOL				MetaFromFile(CLibraryFile* pFile);
	BOOL				MetaToFiles(BOOL bAggressive = FALSE);
	BOOL				OrganizeFile(CLibraryFile* pFile);
	BOOL				MountCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection, BOOL bForce = FALSE);
	CCollectionFile*	GetCollection();
	CString				GetBestView() const;
	void				Serialize(CArchive& ar, int nVersion);
	void				SetCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection);
	bool				OnFolderDelete(CAlbumFolder* pFolder);
	void				OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost = FALSE);
	void				RenewGUID();
	CXMLElement*		CreateXML() const;

//protected:
//	CXMLElement*		CopyMetadata(CXMLElement* pOriginMetadata) const;

private:
	CAlbumFolder(const CAlbumFolder&);
	CAlbumFolder& operator=(const CAlbumFolder&);

public:
	bool operator==(const CAlbumFolder& val) const;
};
