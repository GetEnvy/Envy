//
// SharedFolder.h
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

#include "LibraryFolders.h"

class CLibraryList;
class CXMLElement;


class CLibraryFolder : public CComObject
{
	DECLARE_DYNAMIC(CLibraryFolder)

public:
	CLibraryFolder(CLibraryFolder* pParent, LPCTSTR pszPath = NULL);
	virtual ~CLibraryFolder();

public:
	DWORD			m_nUpdateCookie;
	DWORD			m_nSelectCookie;
	CLibraryFolder*	m_pParent;
	CString			m_sPath;
	CString			m_sName;
	BOOL			m_bExpanded;
	DWORD			m_nFiles;
	QWORD			m_nVolume;

protected:
	CMap< CString, const CString&, CLibraryFile*, CLibraryFile* >		m_pFiles;
	CMap< CString, const CString&, CLibraryFolder*, CLibraryFolder* >	m_pFolders;

	DWORD			m_nScanCookie;
	CString			m_sNameLow;
	TRISTATE		m_bShared;
	HANDLE			m_hMonitor;
	BOOL			m_bForceScan;		// TRUE - next scan forced (root folder only)
	BOOL			m_bOffline;			// TRUE - folder absent (root folder only)
	Hashes::Guid	m_oGUID;

public:
	CXMLElement*	CreateXML(CXMLElement* pRoot, BOOL bSharedOnly, XmlType nType) const;
	POSITION		GetFolderIterator() const;
	CLibraryFolder*	GetNextFolder(POSITION& pos) const;
	CLibraryFolder*	GetFolderByName(LPCTSTR pszName) const;
	CLibraryFolder*	GetFolderByPath(const CString& strPath) const;
	BOOL			CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive = FALSE) const;
	DWORD			GetFolderCount() const;
	CLibraryFile*	AddFile(LPCTSTR szName, BOOL& bNew);		// Add new or get existing file
	BOOL			OnFileDelete(CLibraryFile* pRemovingFile);	// Remove existing file recursively.  TRUE if file found and removed, FALSE when not found
	POSITION		GetFileIterator() const;
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	CLibraryFile*	GetFile(LPCTSTR pszName) const;
	DWORD			GetFileCount() const;
	DWORD			GetFileList(CLibraryList* pList, BOOL bRecursive) const;
	DWORD			GetSharedCount(BOOL bRecursive = TRUE) const;
	CString			GetRelativeName() const;

	void			Scan();
	BOOL			IsShared() const;
	void			GetShared(BOOL& bShared) const;		// Set to TRUE if shared, to FALSE if not, and leave unchanged if unknown
	void			SetShared(TRISTATE bShared);
	BOOL			IsOffline() const;
	BOOL			SetOffline();
	BOOL			SetOnline();
	void			Serialize(CArchive& ar, int nVersion);
	BOOL			ThreadScan(DWORD nScanCookie = 0);
	BOOL			IsChanged();						// Manage filesystem change notification. Returns TRUE if changes detected.
	void			OnDelete(TRISTATE bCreateGhost = TRI_UNKNOWN);
	void			OnFileRename(CLibraryFile* pFile);
	void			Maintain(BOOL bAdd);

protected:
	// Disable change notification monitor
	void			CloseMonitor();
	void			Clear();
	void			PathToName();
	void			RenewGUID();
	bool			operator==(const CLibraryFolder& val) const;

// Automation
protected:
	BEGIN_INTERFACE_PART(LibraryFolder, ILibraryFolder)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get_Parent)(ILibraryFolder FAR* FAR* ppFolder);
		STDMETHOD(get_Path)(BSTR FAR* psPath);
		STDMETHOD(get_Name)(BSTR FAR* psPath);
		STDMETHOD(get_Shared)(TRISTATE FAR* pnValue);
		STDMETHOD(put_Shared)(TRISTATE nValue);
		STDMETHOD(get_EffectiveShared)(VARIANT_BOOL FAR* pbValue);
		STDMETHOD(get_Folders)(ILibraryFolders FAR* FAR* ppFolders);
		STDMETHOD(get_Files)(ILibraryFiles FAR* FAR* ppFiles);
	END_INTERFACE_PART(LibraryFolder)

	BEGIN_INTERFACE_PART(LibraryFolders, ILibraryFolders)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
	END_INTERFACE_PART(LibraryFolders)

	BEGIN_INTERFACE_PART(LibraryFiles, ILibraryFiles)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
	END_INTERFACE_PART(LibraryFiles)

	DECLARE_INTERFACE_MAP()
};
