//
// AlbumFolder.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "AlbumFolder.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "CollectionFile.h"
#include "SharedFile.h"
#include "Schema.h"
#include "SchemaChild.h"
#include "SchemaCache.h"
#include "RegExp.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CAlbumFolder construction

CAlbumFolder::CAlbumFolder(CAlbumFolder* pParent, LPCTSTR pszSchemaURI, LPCTSTR pszName, BOOL bAutoDelete)
	: m_sSchemaURI		( pszSchemaURI )
	, m_pSchema			( SchemaCache.Get( pszSchemaURI ) )
	, m_pXML			( NULL )
	, m_pParent			( pParent )
	, m_pCollection		( NULL )
	, m_bExpanded		( pParent == NULL || pParent->m_pParent == NULL )
	, m_bAutoDelete		( bAutoDelete )
	, m_nUpdateCookie	( 0 )
	, m_nSelectCookie	( 0 )
	, m_nListCookie		( 0 )
{
	if ( pszName > (LPCTSTR)1 )
	{
		m_sName = pszName;
	}
	else if ( pszName != (LPCTSTR)1 )
	{
		if ( m_pSchema != NULL )
		{
			m_sName = m_pSchema->m_sTitle;
			const int nColon = m_sName.Find( L':' );
			if ( nColon >= 0 )
				m_sName = m_sName.Mid( nColon + 1 ).Trim();
		}

		if ( m_sName.IsEmpty() )
			m_sName = LoadString( IDS_NEW_FOLDER );
	}

	RenewGUID();
}

CAlbumFolder::~CAlbumFolder()
{
	Clear();
}

void CAlbumFolder::RenewGUID()
{
	CoCreateGuid( reinterpret_cast< GUID* > ( &*m_oGUID.begin() ) );
	m_oGUID.validate();
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder folder list

void CAlbumFolder::AddFolder(CAlbumFolder* pFolder)
{
	ASSUME_LOCK( Library.m_pSection );

	ASSERT( pFolder->m_pParent == this );

	if ( m_pFolders.Find( pFolder ) )
		return; 	// Already added

	// Change album GUID to avoid duplicates
	pFolder->RenewGUID();

	m_pFolders.AddTail( pFolder );

	m_nUpdateCookie++;
}

CAlbumFolder* CAlbumFolder::AddFolder(LPCTSTR pszSchemaURI, LPCTSTR pszName, BOOL bAutoDelete)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( pszSchemaURI == NULL && m_pSchema != NULL )
		pszSchemaURI = m_pSchema->GetContainedURI( CSchema::stFolder );

	if ( pszSchemaURI == NULL )
		pszSchemaURI = CSchema::uriFolder;

	CAlbumFolder* pFolder = new CAlbumFolder( this, pszSchemaURI, pszName, bAutoDelete );

	m_pFolders.AddTail( pFolder );

	m_nUpdateCookie++;

	return pFolder;
}

DWORD CAlbumFolder::GetFolderCount() const
{
	ASSUME_LOCK( Library.m_pSection );

	return (DWORD)m_pFolders.GetCount();
}

POSITION CAlbumFolder::GetFolderIterator() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolders.GetHeadPosition();
}

CAlbumFolder* CAlbumFolder::GetNextFolder(POSITION& pos) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolders.GetNext( pos );
}

CAlbumFolder* CAlbumFolder::GetParent() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pParent;
}

CAlbumFolder* CAlbumFolder::GetFolder(LPCTSTR pszName) const
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CAlbumFolder* pCheck = GetNextFolder( pos );
		if ( pCheck->m_sName.CompareNoCase( pszName ) == 0 ) return pCheck;
	}

	return NULL;
}

CAlbumFolder* CAlbumFolder::GetFolderByURI(LPCTSTR pszURI) const
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CAlbumFolder* pCheck = GetNextFolder( pos );
		if ( pCheck->m_pSchema != NULL &&
			 pCheck->m_pSchema->CheckURI( pszURI ) ) return pCheck;
	}

	return NULL;
}

BOOL CAlbumFolder::CheckFolder(CAlbumFolder* pFolder, BOOL bRecursive) const
{
	if ( this == pFolder ) return TRUE;
	if ( *this == *pFolder ) return TRUE;
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CAlbumFolder* pCheck = GetNextFolder( pos );
		if ( pCheck == pFolder ) return TRUE;
		if ( *pCheck == *pFolder ) return TRUE;
		if ( bRecursive && pCheck->CheckFolder( pFolder, TRUE ) ) return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder search for objects

CAlbumFolder* CAlbumFolder::GetTarget(CSchemaMember* pMember, LPCTSTR pszValue) const
{
	if ( m_pSchema == pMember->m_pSchema )
	{
		if ( pszValue == NULL )
			return (CAlbumFolder*)this;

		if ( m_pXML != NULL )
		{
			CString strValue = pMember->GetValueFrom( m_pXML, NULL, TRUE );
			CXMLNode::UniformString( strValue );
			if ( strValue.CompareNoCase( pszValue ) == 0 )
				return (CAlbumFolder*)this;
		}
	}

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CAlbumFolder* pCheck  = GetNextFolder( pos );
		CAlbumFolder* pResult = pCheck->GetTarget( pMember, pszValue );
		if ( pResult ) return pResult;
	}

	return NULL;
}

CAlbumFolder* CAlbumFolder::FindCollection(const Hashes::Sha1Hash& oSHA1)
{
	if ( validAndEqual( m_oCollSHA1, oSHA1 ) ) return this;

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CAlbumFolder* pFolder = GetNextFolder( pos );
		if ( CAlbumFolder* pFind = pFolder->FindCollection( oSHA1 ) ) return pFind;
	}

	return NULL;
}

CAlbumFolder* CAlbumFolder::FindFolder(const Hashes::Guid& oGUID)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_oGUID == oGUID )
		return this;		// It's me.

	// Find between children
	POSITION pos = m_pFolders.GetHeadPosition();
	while ( pos )
	{
		CAlbumFolder* pTemp = m_pFolders.GetNext( pos )->FindFolder( oGUID );
		if ( pTemp )
			return pTemp;	// Found
	}
	return NULL;
}

bool CAlbumFolder::OnFolderDelete(CAlbumFolder* pFolder)
{
	ASSUME_LOCK( Library.m_pSection );

	ASSERT( pFolder->m_pParent == this );
	pFolder->m_pParent = NULL;

	// Find by pointer (direct)
	if ( POSITION pos = m_pFolders.Find( pFolder ) )
		m_pFolders.RemoveAt( pos );
	else
		return false;

	ASSERT( ! LibraryFolders.CheckAlbum( pFolder ) );

	Library.Update();
	m_nUpdateCookie++;
	Delete( TRUE );
	return true;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder file list

void CAlbumFolder::AddFile(CLibraryFile* pFile)
{
	ASSUME_LOCK( Library.m_pSection );
	ASSERT_VALID( pFile );

	if ( pFile == NULL ) return;

	POSITION pos = m_pFiles.Find( pFile );
	if ( pos != NULL ) return;

	m_pFiles.AddTail( pFile );

	if ( m_oCollSHA1 )
	{
		if ( CLibraryFile* pCollection = LibraryMaps.LookupFileBySHA1( m_oCollSHA1, FALSE, TRUE ) )
			pFile->m_nCollIndex = pCollection->m_nIndex;
		else
			m_oCollSHA1.clear();
	}

	m_nUpdateCookie++;
	Library.Update();
}

POSITION CAlbumFolder::GetFileIterator() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFiles.GetHeadPosition();
}

CLibraryFile* CAlbumFolder::GetNextFile(POSITION& pos) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFiles.GetNext( pos );
}

DWORD CAlbumFolder::GetFileCount(BOOL bRecursive) const
{
	ASSUME_LOCK( Library.m_pSection );

	DWORD nCount = (DWORD)m_pFiles.GetCount();

	if ( bRecursive )
	{
		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			const CAlbumFolder* pFolder = GetNextFolder( pos );
			nCount += pFolder->GetFileCount( bRecursive );
		}
	}

	return nCount;
}

QWORD CAlbumFolder::GetFileVolume(BOOL bRecursive) const
{
	if ( CheckURI( m_sSchemaURI, CSchema::uriGhostFolder ) )
		return 0;	// Skip ghost folder files

	QWORD nVolume = 0;

	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		const CLibraryFile* pFile = GetNextFile( pos );
		nVolume += pFile->m_nSize;
	}

	if ( bRecursive )
	{
		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			const CAlbumFolder* pFolder = GetNextFolder( pos );
			nVolume += pFolder->GetFileVolume( bRecursive );
		}
	}

	return nVolume;
}

DWORD CAlbumFolder::GetSharedCount(BOOL bRecursive) const
{
	ASSUME_LOCK( Library.m_pSection );

	if ( CheckURI( m_sSchemaURI, CSchema::uriGhostFolder ) )
		return 0;	// Skip ghost folder files

	DWORD nCount = 0;

	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		const CLibraryFile* pFile = GetNextFile( pos );
		if ( pFile->IsShared() ) nCount++;
	}

	if ( bRecursive )
	{
		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			const CAlbumFolder* pFolder = GetNextFolder( pos );
			nCount += pFolder->GetSharedCount( bRecursive );
		}
	}

	return nCount;
}

void CAlbumFolder::RemoveFile(CLibraryFile* pFile)
{
	ASSUME_LOCK( Library.m_pSection );
	ASSERT_VALID( pFile );

	if ( POSITION pos = m_pFiles.Find( pFile ) )
	{
		m_pFiles.RemoveAt( pos );
		m_nUpdateCookie++;
		Library.Update();
		Delete( TRUE );
	}
}

void CAlbumFolder::OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost)
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		GetNextFolder( pos )->OnFileDelete( pFile, bDeleteGhost );
	}

	if ( ! bDeleteGhost && CheckURI( m_sSchemaURI, CSchema::uriGhostFolder ) )
	{
		m_nUpdateCookie++;
		Library.Update();
		return;
	}

	RemoveFile( pFile );
}

const CAlbumFolder* CAlbumFolder::FindFile(const CLibraryFile* pFile) const
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pFiles.Find( const_cast< CLibraryFile* >( pFile ) ) != NULL )
		return this;

	POSITION pos = GetFolderIterator();
	const CAlbumFolder* pFirst = pos ? GetNextFolder( pos ) : NULL;

	if ( GetFolderCount() > 1 )
	{
		while ( pos )
		{
			const CAlbumFolder* pFolder = GetNextFolder( pos )->FindFile( pFile );
			if ( pFolder != NULL ) return pFolder;
		}
	}

	if ( pFirst != NULL )
	{
		const CAlbumFolder* pFolder = pFirst->FindFile( pFile );
		if ( pFolder != NULL ) return pFolder;
	}

	return NULL;
}

DWORD CAlbumFolder::GetFileList(CLibraryList* pList, BOOL bRecursive) const
{
	ASSUME_LOCK( Library.m_pSection );

	int nCount = 0;

	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		pList->CheckAndAdd( GetNextFile( pos ) );
		nCount++;
	}

	if ( bRecursive )
	{
		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			GetNextFolder( pos )->GetFileList( pList, bRecursive );
		}
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder clear

void CAlbumFolder::Delete(BOOL bIfEmpty)
{
	if ( m_pParent == NULL ) return;

	if ( bIfEmpty )
	{
		if ( ! m_bAutoDelete ) return;
		if ( m_oCollSHA1 ) return;
		if ( GetFolderCount() ) return;
		if ( GetFileCount() ) return;
	}

	m_pParent->OnFolderDelete( this );

	delete this;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder metadata

BOOL CAlbumFolder::SetMetadata(CXMLElement* pXML)
{
	m_nUpdateCookie++;
	Library.Update();

	if ( m_pXML != NULL )
	{
		delete m_pXML;
		m_pXML		= NULL;
		m_pSchema	= NULL;
		m_sSchemaURI.Empty();
	}

	if ( pXML == NULL )
		return TRUE;

	m_sSchemaURI	= pXML->GetAttributeValue( CXMLAttribute::schemaName );
	m_pSchema		= SchemaCache.Get( m_sSchemaURI );
	m_pXML			= pXML->GetFirstElement();

	if ( m_pSchema == NULL || m_pXML == NULL )
	{
		m_pXML		= NULL;
		m_pSchema	= NULL;
		m_sSchemaURI.Empty();
		return FALSE;
	}

	m_pXML->Detach();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder metadata synchronization

//BOOL CAlbumFolder::MetaFromFile(CLibraryFile* pFile)
//{
//	if ( m_pSchema == NULL || pFile->m_pMetadata == NULL ) return FALSE;
//
//	CSchemaChild* pChild = m_pSchema->GetContained( pFile->m_pSchema->GetURI() );
//	if ( pChild == NULL ) return FALSE;
//
//	if ( m_pXML == NULL )
//		m_pXML = new CXMLElement( NULL, m_pSchema->m_sSingular );
//
//	pChild->MemberCopy( m_pXML, pFile->m_pMetadata );
//
//	m_nUpdateCookie++;
//	Library.Update();
//
//	return TRUE;
//}

BOOL CAlbumFolder::MetaToFiles(BOOL bAggressive)
{
	if ( m_pSchema == NULL || m_pXML == NULL ) return FALSE;

	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = GetNextFile( pos );

		if ( pFile->m_pMetadata && pFile->m_pSchema )
		{
			if ( CSchemaChild* pChild = m_pSchema->GetContained( pFile->m_pSchema->GetURI() ) )
			{
				CXMLElement* pXML = pFile->m_pMetadata->Clone();

				if ( pChild->MemberCopy( m_pXML, pXML, TRUE, bAggressive ) )
				{
					CXMLElement* pRoot = pFile->m_pSchema->Instantiate( TRUE );
					pRoot->AddElement( pXML );
					pFile->SetMetadata( pRoot );
					delete pRoot;
				}
				else
				{
					delete pXML;
				}
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder select the best view

CString CAlbumFolder::GetBestView() const
{
	if ( ! m_sBestView.IsEmpty() )
		return m_sBestView;

	if ( m_oCollSHA1 )
		return L"CLibraryCollectionView";

	if ( m_pSchema && ! m_pSchema->m_sLibraryView.IsEmpty() )
		return m_pSchema->m_sLibraryView;

	return CString();
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder mount a collection

BOOL CAlbumFolder::MountCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection, BOOL bForce)
{
	BOOL bResult = FALSE;
	bool bGoingDeeper = false;

	// If folder doesn't contain any schema defined then don't go deeper
	if ( m_pSchema == NULL ) return FALSE;

	// Must we mount the collection here?
	// The parent may be absent thus we will try to mount it somewhere.
	// Otherwise, if the validation succeeds we will mount it at the exact parent.
	// If parent can not hold object like this we will mount at the collection root only.
	CString strParentURI = pCollection->GetParentURI();
	TRISTATE bMountHere = TRI_UNKNOWN;

	if ( ! strParentURI.IsEmpty() )
		bMountHere = m_sSchemaURI == strParentURI ? TRI_TRUE : TRI_FALSE;

	// If this folder is a collection or simple folder don't mount it
	// (some collections are folder types which in turn can hold folders)
	if ( m_oCollSHA1 || CheckURI( m_sSchemaURI, CSchema::uriFolder ) )
		bMountHere = TRI_FALSE;

	// If the folder schema allows to hold objects having URIs of the collection
	// or when the folder URI is the root collection folder
	if ( bMountHere != TRI_FALSE &&
		 m_pSchema->GetContained( pCollection->GetThisURI() ) != NULL ||
		 CheckURI( m_sSchemaURI, CSchema::uriCollectionsFolder ) )
	{
		CAlbumFolder* pFolder = NULL;

		if ( ! bForce )
		{
			bGoingDeeper = true;

			for ( POSITION pos = GetFolderIterator() ; pos ; )
			{
				CAlbumFolder* pSubFolder = GetNextFolder( pos );
				// Mount it deeper if we can
				bResult |= pSubFolder->MountCollection( oSHA1, pCollection, bForce );

				// Check if the same collection exists
				if ( validAndEqual( pSubFolder->m_oCollSHA1, oSHA1 ) )
					pFolder = pSubFolder;
			}
		}

		// If the collection doesn't exist or we are forcing, mount it and update Library
		if ( pFolder == NULL )
		{
			pFolder = AddFolder( pCollection->GetThisURI(), pCollection->GetTitle() );
			if ( pFolder )
			{
				pFolder->SetCollection( oSHA1, pCollection );

				m_nUpdateCookie++;
				Library.Update();
				bResult = TRUE;
			}
		}
	}

	// If the criteria for the mounting didn't match and we haven't iterated subfolders
	if ( ! bGoingDeeper )
	{
		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			bResult |= GetNextFolder( pos )->MountCollection( oSHA1, pCollection, bForce );
		}
	}
	return bResult;
}

void CAlbumFolder::SetCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection)
{
	m_oCollSHA1 = oSHA1;
	m_sBestView.Empty();

	if ( m_pCollection != NULL )
	{
		delete m_pCollection;
		m_pCollection = NULL;
	}

	if ( CXMLElement* pMetadata = pCollection->GetMetadata() )
	{
		pMetadata = pMetadata->Clone();
		SetMetadata( pMetadata );
		delete pMetadata;
	}

	for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );

		if ( pFile->IsAvailable() )		// Not ghost
		{
			if ( validAndEqual( m_oCollSHA1, pFile->m_oSHA1 ) ||
				 pCollection->FindFile( pFile, TRUE ) )
				AddFile( pFile );
		}
	}

	m_nUpdateCookie++;
	Library.Update();
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder fetch a colleciton

CCollectionFile* CAlbumFolder::GetCollection()
{
	if ( ! m_oCollSHA1 ) return NULL;
	if ( m_pCollection != NULL ) return m_pCollection;

	if ( CLibraryFile* pFile = LibraryMaps.LookupFileBySHA1( m_oCollSHA1, FALSE, TRUE ) )
	{
		m_pCollection = new CCollectionFile();

		if ( m_pCollection->Open( pFile->GetPath() ) )
			return m_pCollection;

		delete m_pCollection;
		m_pCollection = NULL;
	}

	m_oCollSHA1.clear();
	m_nUpdateCookie++;
	Library.Update();

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder organizing

BOOL CAlbumFolder::OrganizeFile(CLibraryFile* pFile)
{
	BOOL bResult = FALSE;

	// Ghost Files Folder
	if ( ! pFile->IsAvailable() )	// Not necessarily ghost
	{
		if ( CheckURI( m_sSchemaURI, CSchema::uriGhostFolder ) )
		{
			AddFile( pFile );
			return TRUE;
		}
		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			bResult |= GetNextFolder( pos )->OrganizeFile( pFile );
		}
		return bResult;
	}

	// All Files Folder
	if ( CheckURI( m_sSchemaURI, CSchema::uriAllFiles ) )
	{
		AddFile( pFile );
		return TRUE;
	}

	// Collection?
	if ( m_oCollSHA1 && ( m_pCollection || GetCollection() ) )
	{
		if ( validAndEqual( m_oCollSHA1, pFile->m_oSHA1 ) ||
			 m_pCollection->FindFile( pFile, TRUE ) )
		{
			AddFile( pFile );
			return TRUE;
		}

		return FALSE;
	}

	// Unsorted Folder
	if ( CheckURI( m_sSchemaURI, CSchema::uriUnsortedFolder ) )
	{
		if ( pFile->IsSchemaURI( CSchema::uriAudio ) ||
			 pFile->IsSchemaURI( CSchema::uriVideo ) ||
			 pFile->IsSchemaURI( CSchema::uriImage ) ||
			 pFile->IsSchemaURI( CSchema::uriArchive ) ||
			 pFile->IsSchemaURI( CSchema::uriApplication ) ||
			 pFile->IsSchemaURI( CSchema::uriDocument ) ||
			 pFile->IsSchemaURI( CSchema::uriBook ) ||
			 pFile->IsSchemaURI( CSchema::uriROM ) ||
			 pFile->IsSchemaURI( CSchema::uriSpreadsheet ) ||
			 pFile->IsSchemaURI( CSchema::uriPresentation ) ||
			 pFile->IsSchemaURI( CSchema::uriCollection ) ||
			 pFile->IsSchemaURI( CSchema::uriBitTorrent ) )
			return FALSE;

		AddFile( pFile );
		return TRUE;
	}

	// .Torrents Folder  (.collections, ToDo: Metalink/Magma/Etc.?)
	if ( CheckURI( m_sSchemaURI, CSchema::uriBitTorrentFolder ) )
	{
		if ( ! pFile->IsSchemaURI( CSchema::uriBitTorrent ) &&
			 ! pFile->IsSchemaURI( CSchema::uriCollection ) )
			return FALSE;

		AddFile( pFile );
		return TRUE;
	}

	if ( pFile->m_pMetadata == NULL && m_pParent != NULL )
		return FALSE;

	// Archive Folder
	if ( CheckURI( m_sSchemaURI, CSchema::uriArchiveFolder ) )
	{
		if ( ! pFile->IsSchemaURI( CSchema::uriArchive ) )
			return FALSE;

		AddFile( pFile );
		return TRUE;
	}

	// Application Folder
	if ( CheckURI( m_sSchemaURI, CSchema::uriFolder ) )
	{
		if ( ! pFile->IsSchemaURI( CSchema::uriApplication ) &&
			 ! pFile->IsSchemaURI( CSchema::uriROM ) )
			return FALSE;

		AddFile( pFile );
		return TRUE;
	}

	// Audio Folder
	if ( CheckURI( m_sSchemaURI, CSchema::uriAudioFolder ) )
	{
		if ( ! pFile->IsSchemaURI( CSchema::uriAudio ) )
			return FALSE;

		AddFile( pFile );
		//MetaFromFile( pFile );

		return TRUE;
	}

	// Video Folder
	if ( CheckURI( m_sSchemaURI, CSchema::uriVideoFolder ) )
	{
		if ( ! pFile->IsSchemaURI( CSchema::uriVideo ) )
			return FALSE;

		AddFile( pFile );
		//MetaFromFile( pFile );

		return TRUE;
	}

	// Image Folder
	if ( CheckURI( m_sSchemaURI, CSchema::uriImageFolder ) )
	{
		if ( ! pFile->IsSchemaURI( CSchema::uriImage ) )
			return FALSE;

		AddFile( pFile );
		return TRUE;
	}

	// Documents Folder
	if ( CheckURI( m_sSchemaURI, CSchema::uriDocumentFolder ) )
	{
		if ( ! pFile->IsSchemaURI( CSchema::uriBook ) &&
			 ! pFile->IsSchemaURI( CSchema::uriDocument ) &&
			 ! pFile->IsSchemaURI( CSchema::uriSourceCode ) &&
			 ! pFile->IsSchemaURI( CSchema::uriSpreadsheet ) &&
			 ! pFile->IsSchemaURI( CSchema::uriPresentation ) )
			return FALSE;

		AddFile( pFile );
		return TRUE;
	}

// Legacy Unused Subfolders:
//
//	if ( CheckURI( m_sSchemaURI, CSchema::uriApplicationRoot ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriApplication ) &&
//			 ! pFile->IsSchemaURI( CSchema::uriROM ) ) return FALSE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriApplicationAll ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriApplication ) &&
//			 ! pFile->IsSchemaURI( CSchema::uriROM ) ) return FALSE;
//		AddFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriArchiveRoot ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriArchive ) ) return FALSE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriArchiveAll ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriArchive ) ) return FALSE;
//		AddFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriDocumentRoot ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriDocument ) &&
//			 ! pFile->IsSchemaURI( CSchema::uriSpreadsheet ) &&
//			 ! pFile->IsSchemaURI( CSchema::uriPresentation ) ) return FALSE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriDocumentAll ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriDocument ) &&
//			 ! pFile->IsSchemaURI( CSchema::uriSpreadsheet ) &&
//			 ! pFile->IsSchemaURI( CSchema::uriPresentation ) ) return FALSE;
//		AddFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriBookRoot ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriBook ) ) return FALSE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriBookAll ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriBook ) ) return FALSE;
//		AddFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriImageRoot ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriImage ) ) return FALSE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriImageAll ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriImage ) ) return FALSE;
//		AddFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriMusicRoot ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriAudio ) ) return FALSE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriMusicAlbumCollection ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriAudio ) ) return FALSE;
//
//		CString strAlbum = pFile->m_pMetadata->GetAttributeValue( L"album" );
//		CXMLNode::UniformString( strAlbum );
//
//		if ( strAlbum.IsEmpty() ) return FALSE;
//		if ( _tcsicmp( strAlbum, L"tba" ) == 0 ) return FALSE;
//		if ( _tcsicmp( strAlbum, L"na" ) == 0 ) return FALSE;
//		if ( _tcsicmp( strAlbum, L"n/a" ) == 0 ) return FALSE;
//		if ( _tcsicmp( strAlbum, L"none" ) == 0 ) return FALSE;
//		if ( _tcsicmp( strAlbum, L"empty" ) == 0 ) return FALSE;
//		if ( _tcsicmp( strAlbum, L"unknown" ) == 0 ) return FALSE;
//		if ( _tcsistr( strAlbum, L"uploaded by" ) ) return FALSE;
//		if ( _tcsistr( strAlbum, L"ripped by" ) ) return FALSE;
//		if ( _tcsistr( strAlbum, L"downloaded" ) ) return FALSE;
//		if ( _tcsistr( strAlbum, L"http" ) ) return FALSE;
//		if ( _tcsistr( strAlbum, L"mp3" ) ) return FALSE;
//		if ( _tcsistr( strAlbum, L"www.mp3sfinder.com" ) ) return FALSE;
//		if ( _tcsistr( strAlbum, L"single" ) ) strAlbum = L"Singles";
//
//		for ( POSITION pos = GetFolderIterator() ; pos ; )
//		{
//			CAlbumFolder* pAlbum = GetNextFolder( pos );
//
//			if ( pAlbum->m_sName.CompareNoCase( strAlbum ) == 0 )
//				bResult = pAlbum->OrganizeFile( pFile );
//			else if ( pAlbum->m_bAutoDelete )
//				pAlbum->RemoveFile( pFile );
//		}
//
//		if ( bResult ) return TRUE;
//
//		CAlbumFolder* pAlbum = AddFolder( CSchema::uriMusicAlbum, strAlbum, TRUE );
//
//		return pAlbum->OrganizeFile( pFile );
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriMusicAlbum ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriAudio ) ) return FALSE;
//
//		CString strAlbum = pFile->m_pMetadata->GetAttributeValue( L"album" );
//		CXMLNode::UniformString( strAlbum );
//		if ( _tcsistr( strAlbum, L"single" ) ) strAlbum = L"Singles";
//		if ( strAlbum.CompareNoCase( m_sName ) ) return FALSE;
//
//		AddFile( pFile );
//
//		// ToDo: Scrap artist specific info ?
//		//if ( _tcsistr( m_sName, L"soundtrack" ) != NULL || _tcsistr( m_sName, L"ost" ) != NULL )
//		//	MetaFromFile( pFile );
//		//else
//			MetaFromFile( pFile );
//
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriMusicArtistCollection ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriAudio ) ) return FALSE;
//
//		CString strArtist = pFile->m_pMetadata->GetAttributeValue( L"artist" );
//		CXMLNode::UniformString( strArtist );
//
//		strArtist.Replace( L" (www.mp3sfinder.com)", L"" );
//		if ( strArtist.IsEmpty() ) return FALSE;
//
//		for ( POSITION pos = GetFolderIterator() ; pos ; )
//		{
//			CAlbumFolder* pAlbum = GetNextFolder( pos );
//
//			if ( pAlbum->m_sName.CompareNoCase( strArtist ) == 0 )
//				bResult = pAlbum->OrganizeFile( pFile );
//			else if ( pAlbum->m_bAutoDelete )
//				pAlbum->RemoveFile( pFile );
//		}
//
//		if ( bResult ) return TRUE;
//
//		CAlbumFolder* pAlbum = AddFolder( CSchema::uriMusicArtist, strArtist, TRUE );
//		return pAlbum->OrganizeFile( pFile );
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriMusicArtist ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriAudio ) ) return FALSE;
//
//		CString strArtist = pFile->m_pMetadata->GetAttributeValue( L"artist" );
//		CXMLNode::UniformString( strArtist );
//		if ( strArtist.CompareNoCase( m_sName ) ) return FALSE;
//
//		AddFile( pFile );
//		MetaFromFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriMusicGenreCollection ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriAudio ) ) return FALSE;
//
//		CString strGenre = pFile->m_pMetadata->GetAttributeValue( L"genre" );
//		if ( strGenre.IsEmpty() ) return FALSE;
//
//		for ( POSITION pos = GetFolderIterator() ; pos ; )
//		{
//			CAlbumFolder* pAlbum = GetNextFolder( pos );
//
//			if ( pAlbum->m_sName.CompareNoCase( strGenre ) == 0 )
//				bResult = pAlbum->OrganizeFile( pFile );
//			else if ( pAlbum->m_bAutoDelete )
//				pAlbum->RemoveFile( pFile );
//		}
//
//		if ( bResult ) return TRUE;
//
//		CAlbumFolder* pAlbum = AddFolder( CSchema::uriMusicGenre, strGenre, TRUE );
//		return pAlbum->OrganizeFile( pFile );
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriMusicGenre ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriAudio ) ) return FALSE;
//
//		CString strGenre = pFile->m_pMetadata->GetAttributeValue( L"genre" );
//		if ( strGenre.CompareNoCase( m_sName ) ) return FALSE;
//
//		AddFile( pFile );
//		MetaFromFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriMusicAll ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriAudio ) ) return FALSE;
//		AddFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriVideoRoot ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriVideo ) ) return FALSE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriVideoSeriesCollection ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriVideo ) ) return FALSE;
//
//		CString strSeries = pFile->m_pMetadata->GetAttributeValue( L"series" );
//		CXMLNode::UniformString( strSeries );
//		if ( strSeries.IsEmpty() )
//		{
//			if ( ! Settings.Library.SmartSeriesDetection )
//				return FALSE;
//
//			CString strFileName = (LPCTSTR)pFile->m_sName;
//			CXMLNode::UniformString( strFileName );
//
//			std::vector<std::wstring> results;
//
//			LPTSTR szResults = NULL;
//			size_t nCount = RegExp::Split( L"(.*)(\\bse?a?s?o?n?)\\s*([0-9]+)\\s*(ep?i?s?o?d?e?)\\s*([0-9]+)[^0-9]+.*", strFileName, &szResults );
//			LPCTSTR p = szResults;
//			for ( size_t i = 0 ; i < nCount ; ++i )
//			{
//				results.push_back( p );
//				p += lstrlen( p ) + 1;
//			}
//			GlobalFree( szResults );
//
//			if ( nCount >= 6 )
//			{
//				LPCTSTR szSeason  = _tcsistr( L"season", results[2].c_str() );
//				LPCTSTR szEpisode = _tcsistr( L"episode", results[4].c_str() );
//				if ( szSeason && szEpisode &&
//					_tcsicmp( szSeason, L"season" ) == 0 &&
//					_tcsicmp( szEpisode, L"episode" ) == 0 )
//				{
//					std::vector<std::wstring>::iterator it =
//						 std::find( results.begin(), results.end(), results[2].c_str() );
//					results.erase( it );
//					it = std::find( results.begin(), results.end(), results[3].c_str() );
//					results.erase( it );
//					nCount -= 2;
//				}
//				else
//					nCount = 0;
//			}
//			else
//				nCount = 0;
//
//			if ( nCount < 4 )
//			{
//				nCount = RegExp::Split( L"(.*[^0-9]+\\b)([0-9]+)\\s*[xX]\\s*([0-9]+)[^0-9]+.*", strFileName, &szResults );
//				p = szResults;
//				for ( size_t i = 0 ; i < nCount ; ++i )
//				{
//					results.push_back( p );
//					p += lstrlen( p ) + 1;
//				}
//				GlobalFree( szResults );
//
//				return FALSE;
//			}
//
//			// nCount >= 4
//
//			strSeries = results[1].c_str();
//			strSeries.TrimRight( L"- " );
//
//			if ( strSeries.IsEmpty() )
//				return FALSE;	// Bad detection
//
//			TCHAR cLast = strSeries.GetAt( strSeries.GetLength() - 1 );
//			if ( cLast == L'[' || cLast == L'(' || cLast == L'{' )
//				return FALSE;	// Bad detection
//
//			int nSeriesNumber  = _tstoi( results[2].c_str() );
//			int nEpisodeNumber = _tstoi( results[3].c_str() );
//			if ( nSeriesNumber < 0 || nSeriesNumber > 200 || nEpisodeNumber < 0 || nEpisodeNumber > 400 )
//				return FALSE;	// Bad detection
//
//			// Capitalize first letter
//			strSeries = strSeries.Left( 1 ).MakeUpper() + strSeries.Mid( 1 );
//
//			pFile->m_pMetadata->AddAttribute( L"series", strSeries );
//
//			CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"seriesnumber" );
//			if ( ! pAttribute )
//				pFile->m_pMetadata->AddAttribute( L"seriesnumber", results[2].c_str() );
//
//			pAttribute = pFile->m_pMetadata->GetAttribute( L"episodenumber" );
//			if ( ! pAttribute )
//				pFile->m_pMetadata->AddAttribute( L"episodenumber", results[3].c_str() );
//		}
//
//		for ( POSITION pos = GetFolderIterator() ; pos ; )
//		{
//			CAlbumFolder* pAlbum = GetNextFolder( pos );
//
//			if ( pAlbum->m_sName.CompareNoCase( strSeries ) == 0 )
//				bResult = pAlbum->OrganizeFile( pFile );
//			else if ( pAlbum->m_bAutoDelete )
//				pAlbum->RemoveFile( pFile );
//		}
//
//		if ( bResult ) return TRUE;
//
//		CAlbumFolder* pAlbum = AddFolder( CSchema::uriVideoSeries, strSeries, TRUE );
//		return pAlbum->OrganizeFile( pFile );
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriVideoSeries ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriVideo ) ) return FALSE;
//
//		CString strSeries = pFile->m_pMetadata->GetAttributeValue( L"series" );
//		CXMLNode::UniformString( strSeries );
//		if ( strSeries.CompareNoCase( m_sName ) ) return FALSE;
//
//		AddFile( pFile );
//		MetaFromFile( pFile );
//
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriVideoFilmCollection ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriVideo ) ) return FALSE;
//
//		CString strType = pFile->m_pMetadata->GetAttributeValue( L"type" );
//		if ( strType.CompareNoCase( L"film" ) ) return FALSE;
//
//		CString strTitle = pFile->m_pMetadata->GetAttributeValue( L"title" );
//		CXMLNode::UniformString( strTitle );
//		if ( strTitle.IsEmpty() ) return FALSE;
//
//		for ( POSITION pos = GetFolderIterator() ; pos ; )
//		{
//			CAlbumFolder* pAlbum = GetNextFolder( pos );
//			if ( pAlbum->m_sName.CompareNoCase( strTitle ) == 0 )
//				bResult = pAlbum->OrganizeFile( pFile );
//			else if ( pAlbum->m_bAutoDelete )
//				pAlbum->RemoveFile( pFile );
//		}
//
//		if ( bResult ) return TRUE;
//
//		CAlbumFolder* pAlbum = AddFolder( CSchema::uriVideoFilm, strTitle, TRUE );
//
//		return pAlbum->OrganizeFile( pFile );
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriVideoFilm ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriVideo ) ) return FALSE;
//
//		CString strType = pFile->m_pMetadata->GetAttributeValue( L"type" );
//		if ( strType.CompareNoCase( L"film" ) ) return FALSE;
//
//		CString strTitle = pFile->m_pMetadata->GetAttributeValue( L"title" );
//		CXMLNode::UniformString( strTitle );
//		if ( strTitle.CompareNoCase( m_sName ) ) return FALSE;
//
//		AddFile( pFile );
//		MetaFromFile( pFile );
//
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriVideoMusicCollection ) )
//	{
//		if ( ! pFile->m_pMetadata || ! pFile->IsSchemaURI( CSchema::uriVideo ) ) return FALSE;
//
//		CString strType = pFile->m_pMetadata->GetAttributeValue( L"type" );
//		if ( strType.CompareNoCase( L"music video" ) ) return FALSE;
//		AddFile( pFile );
//		return TRUE;
//	}
//	else if ( CheckURI( m_sSchemaURI, CSchema::uriVideoAll ) )
//	{
//		if ( ! pFile->IsSchemaURI( CSchema::uriVideo ) ) return FALSE;
//		AddFile( pFile );
//		return TRUE;
//	}
// End Legacy Unused Subfolders

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		bResult |= GetNextFolder( pos )->OrganizeFile( pFile );
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder serialize

void CAlbumFolder::Serialize(CArchive& ar, int nVersion)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( ar.IsStoring() )
	{
		ar << m_sSchemaURI;

		ar.WriteCount( m_pXML != NULL ? 1 : 0 );
		if ( m_pXML ) m_pXML->Serialize( ar );
		SerializeOut( ar, m_oCollSHA1 );

		SerializeOut( ar, m_oGUID );

		ar << m_sName;
		ar << m_bExpanded;
		ar << m_bAutoDelete;
		ar << m_sBestView;

		ar.WriteCount( GetFolderCount() );

		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			CAlbumFolder* pFolder = GetNextFolder( pos );
			pFolder->Serialize( ar, nVersion );
		}

		ar.WriteCount( GetFileCount() );

		for ( POSITION pos = GetFileIterator() ; pos ; )
		{
			CLibraryFile* pFile = GetNextFile( pos );
			ar << pFile->m_nIndex;
		}
	}
	else // Loading
	{
		CLibraryFile* pCollection = NULL;

		if ( m_pParent != NULL )
		{
			ar >> m_sSchemaURI;
			m_pSchema = SchemaCache.Get( m_sSchemaURI );
		}
		else
		{
			CString str;
			ar >> str;
		}

		if ( ar.ReadCount() )
		{
			ASSERT( m_pXML == NULL );
			m_pXML = new CXMLElement();
			m_pXML->Serialize( ar );
		}

		SerializeIn( ar, m_oCollSHA1, nVersion );
		pCollection = LibraryMaps.LookupFileBySHA1( m_oCollSHA1, FALSE, TRUE );
		// ToDo: Needs better validation.
		// Some collections are bound to URIs that assign the whole library as one big collection.
		if ( pCollection == NULL || m_pSchema &&
				( m_pSchema->CheckURI( CSchema::uriAllFiles ) ||
				  m_pSchema->CheckURI( CSchema::uriGhostFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriUnsortedFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriBitTorrentFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriApplicationFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriArchiveFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriAudioFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriVideoFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriImageFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriDocumentFolder ) ||
				  m_pSchema->CheckURI( CSchema::uriLibrary )
				) )
			m_oCollSHA1.clear();

		SerializeIn( ar, m_oGUID, nVersion );

		ar >> m_sName;
		ar >> m_bExpanded;
		ar >> m_bAutoDelete;
		ar >> m_sBestView;

		DWORD_PTR nCount = ar.ReadCount();

		while ( nCount-- > 0 )
		{
			unique_ptr< CAlbumFolder > pFolder( new CAlbumFolder( this, NULL, (LPCTSTR)1 ) );
			pFolder->Serialize( ar, nVersion );
			m_pFolders.AddTail( pFolder.release() );
		}

		nCount = ar.ReadCount();

		while ( nCount-- > 0 )
		{
			DWORD nIndex;
			ar >> nIndex;

			if ( CLibraryFile* pFile = Library.LookupFile( nIndex ) )
			{
				POSITION pos = m_pFiles.Find( pFile );
				if ( pos == NULL )
					m_pFiles.AddTail( pFile );
				if ( pCollection != NULL )
					pFile->m_nCollIndex = pCollection->m_nIndex;

				theApp.KeepAlive();
			}
		}
	}
}

bool CAlbumFolder::operator==(const CAlbumFolder& val) const
{
	return ( m_oGUID == val.m_oGUID );
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder clear

void CAlbumFolder::Clear()
{
	ASSUME_LOCK( Library.m_pSection );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		delete GetNextFolder( pos );
	}

	m_pFolders.RemoveAll();
	m_pFiles.RemoveAll();

	delete m_pXML;
	m_pXML = NULL;

	delete m_pCollection;
	m_pCollection = NULL;

	m_nUpdateCookie++;
}

//////////////////////////////////////////////////////////////////////
// CAlbumFolder Create XML collection

CXMLElement* CAlbumFolder::CreateXML() const
{
	// Collect wizard entries
	CXMLElement* pRoot = new CXMLElement( NULL, L"collection" );
	if ( pRoot )
	{
		pRoot->AddAttribute( L"xmlns", CSchema::uriCollection );

		if ( CXMLElement* pProperties = pRoot->AddElement( L"properties" ) )
		{
			pProperties->AddElement( L"title" )->SetValue(
				m_sName.IsEmpty() ? L"Envy Collection" : (LPCTSTR)m_sName );

			if ( m_pXML && ! m_sSchemaURI.IsEmpty() )
			{
				if ( CXMLElement* pMeta = pProperties->AddElement( L"metadata" ) )
				{
					pMeta->AddAttribute( L"xmlns:s", m_sSchemaURI );
					pMeta->AddElement( m_pXML->Prefix( L"s:" ) );
				}
			}
			else
			{
				if ( CXMLElement* pMounting = pProperties->AddElement( L"mounting" ) )
				{
					if ( CXMLElement* pParent = pMounting->AddElement( L"parent" ) )
					{
						pParent->AddAttribute( L"uri", CSchema::uriCollectionsFolder );

						if ( CXMLElement* pElement = pMounting->AddElement( L"this" ) )
						{
							pElement->AddAttribute( L"uri", CSchema::uriFolder );
						}
					}
				}
			}
		}

		if ( CXMLElement* pContents = pRoot->AddElement( L"contents" ) )
		{
			for ( POSITION pos = GetFileIterator() ; pos ; )
			{
				if ( const CLibraryFile* pFile = GetNextFile( pos ) )
				{
					pFile->CreateXML( pContents, FALSE, xmlDefault );
				}
			}
		}

		// Moved to SharedFile CreateXML:
		//CXMLElement* pFileRoot = pContents->AddElement( L"file" );
	}

	return pRoot;
}

//CXMLElement* CAlbumFolder::CopyMetadata(CXMLElement* pOriginMetadata) const
//{
//	CXMLElement* pMetadata = pOriginMetadata->Clone();
//
//	pMetadata->SetName( L"s:" + pMetadata->GetName() );
//
//	for ( POSITION pos = pMetadata->GetElementIterator() ; pos ; )
//	{
//		CXMLElement* pNode = pMetadata->GetNextElement( pos );
//		pNode->SetName( L"s:" + pNode->GetName() );
//	}
//
//	for ( POSITION pos = pMetadata->GetAttributeIterator() ; pos ; )
//	{
//		CXMLAttribute* pNode = pMetadata->GetNextAttribute( pos );
//		pNode->SetName( L"s:" + pNode->GetName() );
//	}
//
//	return pMetadata;
//}
