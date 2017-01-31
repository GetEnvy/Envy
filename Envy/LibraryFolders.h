//
// LibraryFolders.h
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

class CLibraryFolder;
class CAlbumFolder;
class CLibraryFile;
class CCollectionFile;
class CXMLElement;

enum XmlType
{
	xmlDefault,		// Default
//	xmlPassKey,		// Private Share
	xmlDC			// DC++ file listing
};

// ToDo: Support Private Share Key
//enum ShowType
//{
//	ShowAll,		// bSharedOnly FALSE
//	ShowPublic,		// bSharedOnly TRUE
//	ShowPrivate		// Passkey
//};


class CLibraryFolders : public CComObject
{
	DECLARE_DYNAMIC(CLibraryFolders)

public:
	CLibraryFolders();
	virtual ~CLibraryFolders();

protected:
	CList< CLibraryFolder* > m_pFolders;
	CAlbumFolder*	m_pAlbumRoot;

// Physical Folder Operations
public:
	CXMLElement*	CreateXML(LPCTSTR szRoot, BOOL bSharedOnly, XmlType nType = xmlDefault) const;
	POSITION		GetFolderIterator() const;
	CLibraryFolder*	GetNextFolder(POSITION& pos) const;
	INT_PTR			GetFolderCount() const { return m_pFolders.GetCount(); }
	CLibraryFolder*	GetFolder(const CString& strPath) const;
	BOOL			CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive = FALSE) const;
	CLibraryFolder*	GetFolderByName(LPCTSTR pszName) const;
	CLibraryFolder*	AddFolder(LPCTSTR pszPath);
	CLibraryFolder*	AddFolder(LPCTSTR pszPath, BOOL bShared);
	bool			AddSharedFolder(CListCtrl& oList);
	BOOL			RemoveFolder(CLibraryFolder* pFolder);
	CLibraryFolder*	IsFolderShared(const CString& strPath) const;
	CLibraryFolder*	IsSubFolderShared(const CString& strPath) const;
	static bool		IsShareable(const CString& strPath);
	void			Maintain();

// Virtual Album Operations
	BOOL			CheckAlbum(CAlbumFolder* pFolder) const;
	CAlbumFolder* 	CreateAlbumTree();
	CAlbumFolder*	GetAlbumRoot() const;
	CAlbumFolder*	GetAlbumTarget(LPCTSTR pszSchemaURI, LPCTSTR pszMember, LPCTSTR pszValue) const;
	CAlbumFolder*	GetCollection(const Hashes::Sha1Hash& oSHA1);
	BOOL			MountCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection);
	BOOL			OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost = FALSE);	// Remove file from all albums and folders
	void			ClearGhosts();				// Remove all ghost files
	DWORD			GetGhostCount() const;		// Get total amount of ghost files

// Core
protected:
	void			Clear();
	void			Serialize(CArchive& ar, int nVersion);
	BOOL			ThreadScan(const BOOL bForce = FALSE);

// COM
protected:
	BEGIN_INTERFACE_PART(LibraryFolders, ILibraryFolders)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
	END_INTERFACE_PART(LibraryFolders)

	DECLARE_INTERFACE_MAP()

	friend class CLibrary;
};

extern CLibraryFolders LibraryFolders;
