//
// LibraryFolders.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2007
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
#include "LibraryFolders.h"

#include "AlbumFolder.h"
#include "Application.h"
#include "CollectionFile.h"
#include "GProfile.h"
#include "DlgHelp.h"
#include "Library.h"
#include "LibraryMaps.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "ShellIcons.h"
#include "Skin.h"	// Win7+
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CLibraryFolders, CComObject)

BEGIN_INTERFACE_MAP(CLibraryFolders, CComObject)
	INTERFACE_PART(CLibraryFolders, IID_ILibraryFolders, LibraryFolders)
END_INTERFACE_MAP()

CLibraryFolders LibraryFolders;


//////////////////////////////////////////////////////////////////////
// CLibraryFolders construction

CLibraryFolders::CLibraryFolders()
	: m_pAlbumRoot	( NULL )
{
	EnableDispatch( IID_ILibraryFolders );
}

CLibraryFolders::~CLibraryFolders()
{
	delete m_pAlbumRoot;
}

CXMLElement* CLibraryFolders::CreateXML(LPCTSTR szRoot, BOOL bSharedOnly, XmlType nType) const
{
	CXMLElement* pRoot;

	if ( nType == xmlDC )
	{
		// Note case-sensitive http://adc.sourceforge.net/ADC.html#_file_list
		pRoot = new CXMLElement( NULL, L"FileListing" );
		if ( pRoot )
		{
			pRoot->AddAttribute( L"Version", 1 );
			pRoot->AddAttribute( L"Base", szRoot );
			pRoot->AddAttribute( L"Generator", Settings.SmartAgent() );
			Hashes::Guid oGUID( MyProfile.oGUID );
			pRoot->AddAttribute( L"CID", oGUID.toString< Hashes::base32Encoding >() );
		}
	}
	else // Default
	{
		pRoot = new CXMLElement( NULL, L"folders" );
		if ( pRoot )
			pRoot->AddAttribute( L"xmlns", CSchema::uriFolder );
	}

	if ( ! pRoot )
		return NULL;	// Out of memory

	CSingleLock oLock( &Library.m_pSection, TRUE );

	if ( _tcsicmp( szRoot, L"/" ) == 0 )
	{
		// All folders
		for ( POSITION pos = LibraryFolders.GetFolderIterator() ; pos ; )
		{
			LibraryFolders.GetNextFolder( pos )->CreateXML( pRoot, bSharedOnly, nType );
		}
	}
	else if ( const CLibraryFolder* pFolder = LibraryFolders.GetFolderByName( szRoot ) )
	{
		// Specified folder
		pFolder->CreateXML( pRoot, bSharedOnly, nType );
	}

	return pRoot;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders physical folder enumeration

POSITION CLibraryFolders::GetFolderIterator() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolders.GetHeadPosition();
}

CLibraryFolder* CLibraryFolders::GetNextFolder(POSITION& pos) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolders.GetNext( pos );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders physical folder search

CLibraryFolder* CLibraryFolders::GetFolder(const CString& strPath) const
{
	ASSUME_LOCK( Library.m_pSection );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos )->GetFolderByPath( strPath );
		if ( pFolder != NULL )
			return pFolder;
	}

	return NULL;
}

BOOL CLibraryFolders::CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive) const
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pFolders.Find( pFolder ) != NULL ) return TRUE;
	if ( ! bRecursive ) return FALSE;

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		if ( GetNextFolder( pos )->CheckFolder( pFolder, TRUE ) ) return TRUE;
	}

	return FALSE;
}

CLibraryFolder* CLibraryFolders::GetFolderByName(LPCTSTR pszName) const
{
	ASSUME_LOCK( Library.m_pSection );

	CString strName( pszName );
	strName.MakeLower();		// Was ToLower()

	CString strNextName;
	const int nPos = strName.FindOneOf( L"\\/" );
	if ( nPos != -1 )
	{
		strNextName = strName.Mid( nPos + 1 );
		strName = strName.Left( nPos );
	}

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		if ( pFolder->m_sName.CompareNoCase( strName ) == 0 )
		{
			if ( ! strNextName.IsEmpty() )
				pFolder = pFolder->GetFolderByName( strNextName );

			return pFolder;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders add a root physical folder

CLibraryFolder* CLibraryFolders::AddFolder(LPCTSTR pszPath)
{
	CString strPath = pszPath;

	if ( strPath.GetLength() == 3 && strPath.GetAt( 2 ) == '\\' )
		strPath = strPath.Left( 2 );

	CQuickLock oLock( Library.m_pSection );

	if ( GetFolder( strPath ) ) return NULL;
//	if ( IsFolderShared( strPath ) ) return NULL;
//	if ( IsSubFolderShared( strPath ) ) return NULL;

	CLibraryFolder* pFolder = new CLibraryFolder( NULL, strPath );
	if ( ! pFolder ) return NULL;

	BOOL bAdded = FALSE;

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		POSITION posAdd = pos;
		if ( GetNextFolder( pos )->m_sName.CompareNoCase( pFolder->m_sName ) < 0 )
			continue;

		m_pFolders.InsertBefore( posAdd, pFolder );
		bAdded = TRUE;
		break;
	}

	if ( ! bAdded )
		m_pFolders.AddTail( pFolder );

	pFolder->Maintain( TRUE );

	Library.Update( true );

	Maintain();		// desktop.ini

	return pFolder;
}

CLibraryFolder* CLibraryFolders::AddFolder(LPCTSTR pszPath, BOOL bShared)
{
	CLibraryFolder* pFolder = AddFolder( pszPath );

	if ( pFolder )
		pFolder->SetShared( bShared ? TRI_TRUE : TRI_FALSE );

	return pFolder;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders add a shared folder to a List Control

bool CLibraryFolders::AddSharedFolder(CListCtrl& oList)
{
	// Store the last path selected
	static CString strLastPath;
	if ( strLastPath.IsEmpty() )
		strLastPath = oList.GetItemText( 0, 0 );

	// Let user select a path to share
	CString strPath( BrowseForFolder( L"Select folder to share:", strLastPath ) );
	if ( strPath.IsEmpty() )
		return false;

	strLastPath = strPath;

	// Check if path is valid
	if ( ! IsShareable( strPath ) )
	{
		CHelpDlg::Show( L"ShareHelp.BadShare" );
		return false;
	}

	// Convert path to lowercase
	CString strPathLow( strPath );
	ToLower( strPathLow );

	// Check if path is already shared
	bool bForceAdd( false );
	for ( int nItem( 0 ) ; nItem < oList.GetItemCount() ; ++nItem )
	{
		bool bSubFolder( false );
		CString strOldLow( oList.GetItemText( nItem, 0 ) );
		ToLower( strOldLow );

		if ( strPathLow.Compare( strOldLow ) == 0 )
		{
			// Matches exactly
		}
		else if ( strPathLow.GetLength() > strOldLow.GetLength() )
		{
			if ( strPathLow.Left( strOldLow.GetLength() + 1 ) != strOldLow + '\\' )
				continue;
		}
		else if ( strPathLow.GetLength() < strOldLow.GetLength() )
		{
			bSubFolder = true;
			if ( strOldLow.Left( strPathLow.GetLength() + 1 ) != strPathLow + L'\\' )
				continue;
		}
		else
		{
			continue;
		}

		if ( bSubFolder )
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_LIBRARY_SUBFOLDER_IN_LIBRARY ), strPath );

			if ( bForceAdd || MsgBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				// Don't bother asking again- remove all sub-folders
				bForceAdd = true;

				// Remove the sub-folder
				oList.DeleteItem( nItem );
				--nItem;
			}
			else
			{
				return false;
			}
		}
		else
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_WIZARD_SHARE_ALREADY ), strOldLow );
			MsgBox( strMessage, MB_ICONINFORMATION );
			return false;
		}
	}

	// Add path to shared list
	oList.InsertItem( LVIF_TEXT|LVIF_IMAGE, oList.GetItemCount(), strPath,
		0, 0, SHI_FOLDER_OPEN, 0 );

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders remove a root physical folder

BOOL CLibraryFolders::RemoveFolder(CLibraryFolder* pFolder)
{
	CWaitCursor pCursor;
	CQuickLock pLock( Library.m_pSection );

	POSITION pos = m_pFolders.Find( pFolder );
	if ( pos == NULL ) return FALSE;

	pFolder->Maintain( FALSE );

	pFolder->OnDelete( Settings.Library.CreateGhosts ? TRI_TRUE : TRI_FALSE );
	m_pFolders.RemoveAt( pos );

	Library.Update( true );

	Maintain();		// desktop.ini

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders check if a physical folder is part of the library

CLibraryFolder* CLibraryFolders::IsFolderShared(const CString& strPath) const
{
	// Convert path to lowercase
	CString strPathLow( strPath );
	ToLower( strPathLow );
	const int nPathLength = strPathLow.GetLength();

	CQuickLock oLock( Library.m_pSection );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		CString strOldLow( pFolder->m_sPath );
		ToLower( strOldLow );
		const int nLength = strOldLow.GetLength();

		if ( nPathLength > nLength )
		{
			if ( strPathLow.Left( nLength ) == strOldLow &&
				 strPathLow.GetAt( nLength ) == L'\\' )
				return pFolder;
		}
		else if ( strPathLow.Compare( strOldLow ) == 0 )
		{
			return pFolder;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders check if a subfolder of a physical folder is part of the library

CLibraryFolder* CLibraryFolders::IsSubFolderShared(const CString& strPath) const
{
	// Convert path to lowercase
	CString strPathLow( strPath );
	ToLower( strPathLow );
	const int nLength = strPathLow.GetLength();

	CQuickLock oLock( Library.m_pSection );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		CString strOldLow( pFolder->m_sPath );
		ToLower( strOldLow );

		if ( nLength < strOldLow.GetLength() )
		{
			if ( strOldLow.Left( nLength ) == strPathLow &&
				 strOldLow.GetAt( nLength ) == L'\\' )
				return pFolder;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders check if folder is not a system directory, incomplete folder etc...

bool CLibraryFolders::IsShareable(const CString& strPath)
{
	if ( strPath.IsEmpty() )
		return false;

	// Get system paths (to compare)

	const CString strWindows = theApp.GetWindowsFolder();
	if ( strPath.GetLength() < 4 && _tcsnicmp( strWindows.Left( 3 ), strPath, strPath.GetLength() ) == 0 )
		return false;
	if ( _tcsnicmp( strWindows, strPath, strPath.GetLength() ) == 0 )
		return false;

	const CString strProgramFiles = theApp.GetProgramFilesFolder();
	if ( _tcsnicmp( strProgramFiles, strPath, strPath.GetLength() ) == 0 )
		return false;

	const CString strProgramFiles64 = theApp.GetProgramFilesFolder64();
	if ( _tcsnicmp( strProgramFiles64, strPath, strPath.GetLength() ) == 0 )
		return false;

	// Get various Envy paths (to compare)
	if ( _tcsnicmp( strPath, Settings.General.Path, Settings.General.Path.GetLength() ) == 0 )
		return false;
	if ( _tcsnicmp( Settings.General.Path, strPath, strPath.GetLength() ) == 0 )
		return false;

	if ( _tcsicmp( strPath, Settings.General.UserPath ) == 0 )
		return false;
	if ( _tcsicmp( strPath, Settings.General.DataPath ) == 0 )
		return false;

	if ( _tcsnicmp( Settings.Downloads.IncompletePath, strPath, strPath.GetLength() ) == 0 )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album list access

CAlbumFolder* CLibraryFolders::GetAlbumRoot() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pAlbumRoot;
}

BOOL CLibraryFolders::CheckAlbum(CAlbumFolder* pFolder) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pAlbumRoot && m_pAlbumRoot->CheckFolder( pFolder, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album target search

CAlbumFolder* CLibraryFolders::GetAlbumTarget(LPCTSTR pszSchemaURI, LPCTSTR pszMember, LPCTSTR pszValue) const
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot == NULL ) return NULL;

	CSchemaPtr pSchema = SchemaCache.Get( pszSchemaURI );
	if ( pSchema == NULL ) return NULL;

	CSchemaMember* pMember = pSchema->GetMember( pszMember );

	if ( pMember == NULL )
	{
		if ( pSchema->GetMemberCount() == 0 ) return NULL;
		POSITION pos = pSchema->GetMemberIterator();
		pMember = pSchema->GetNextMember( pos );
	}

	if ( pszValue != NULL )
	{
		CString strValue( pszValue );
		CXMLNode::UniformString( strValue );
		return m_pAlbumRoot->GetTarget( pMember, strValue );
	}

	return m_pAlbumRoot->GetTarget( pMember, NULL );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album collection search

CAlbumFolder* CLibraryFolders::GetCollection(const Hashes::Sha1Hash& oSHA1)
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pAlbumRoot ? m_pAlbumRoot->FindCollection( oSHA1 ) : NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders mount a collection

BOOL CLibraryFolders::MountCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return FALSE;

	BOOL bSuccess = FALSE;

	if ( pCollection->GetThisURI().GetLength() )
		bSuccess |= m_pAlbumRoot->MountCollection( oSHA1, pCollection );

	//if ( pCollection->GetParentURI().GetLength() )
	//{
	//	if ( CAlbumFolder* pFolder = GetAlbumTarget( pCollection->GetParentURI(), NULL, NULL ) )
	//		bSuccess |= pFolder->MountCollection( oSHA1, pCollection, TRUE );
	//}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album default tree

CAlbumFolder* CLibraryFolders::CreateAlbumTree()
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot == NULL )
		m_pAlbumRoot = new CAlbumFolder( NULL, CSchema::uriLibrary );

	DWORD nCount = m_pAlbumRoot->GetFolderCount();

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriAllFiles ) == NULL )
	{
		/*CAlbumFolder* pAllFiles		=*/ m_pAlbumRoot->AddFolder( CSchema::uriAllFiles );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriApplicationRoot ) == NULL )
	{
		CAlbumFolder* pAppRoot			= m_pAlbumRoot->AddFolder( CSchema::uriApplicationRoot );
		/*CAlbumFolder* pAppAll			=*/ pAppRoot->AddFolder( CSchema::uriApplicationAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriArchiveRoot ) == NULL )
	{
		CAlbumFolder* pArchiveRoot		= m_pAlbumRoot->AddFolder( CSchema::uriArchiveRoot );
		/*CAlbumFolder* pArchiveAll		=*/ pArchiveRoot->AddFolder( CSchema::uriArchiveAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriBookRoot ) == NULL )
	{
		CAlbumFolder* pBookRoot			= m_pAlbumRoot->AddFolder( CSchema::uriBookRoot );
		/*CAlbumFolder* pBookAll		=*/ pBookRoot->AddFolder( CSchema::uriBookAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriImageRoot ) == NULL )
	{
		CAlbumFolder* pImageRoot		= m_pAlbumRoot->AddFolder( CSchema::uriImageRoot );
		/*CAlbumFolder* pImageAll		=*/ pImageRoot->AddFolder( CSchema::uriImageAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriMusicRoot ) == NULL )
	{
		CAlbumFolder* pMusicRoot		= m_pAlbumRoot->AddFolder( CSchema::uriMusicRoot );
		/*CAlbumFolder* pMusicAll		=*/ pMusicRoot->AddFolder( CSchema::uriMusicAll );
		/*CAlbumFolder* pMusicAlbum		=*/ pMusicRoot->AddFolder( CSchema::uriMusicAlbumCollection );
		/*CAlbumFolder* pMusicArtist	=*/ pMusicRoot->AddFolder( CSchema::uriMusicArtistCollection );
		/*CAlbumFolder* pMusicGenre		=*/ pMusicRoot->AddFolder( CSchema::uriMusicGenreCollection );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriVideoRoot ) == NULL )
	{
		CAlbumFolder* pVideoRoot		= m_pAlbumRoot->AddFolder( CSchema::uriVideoRoot );
		/*CAlbumFolder* pVideoAll		=*/ pVideoRoot->AddFolder( CSchema::uriVideoAll );
		/*CAlbumFolder* pVideoSeries	=*/ pVideoRoot->AddFolder( CSchema::uriVideoSeriesCollection );
		/*CAlbumFolder* pVideoFilm		=*/ pVideoRoot->AddFolder( CSchema::uriVideoFilmCollection );
		/*CAlbumFolder* pVideoMusic		=*/ pVideoRoot->AddFolder( CSchema::uriVideoMusicCollection );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriDocumentRoot ) == NULL )
	{
		CAlbumFolder* pDocumentRoot		= m_pAlbumRoot->AddFolder( CSchema::uriDocumentRoot );
		/*CAlbumFolder* pDocumentAll	=*/ pDocumentRoot->AddFolder( CSchema::uriDocumentAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriUnknownFolder ) == NULL )
	{
		/*CAlbumFolder* pUnknownFolder	=*/ m_pAlbumRoot->AddFolder( CSchema::uriUnknownFolder );
	}

	if ( Settings.BitTorrent.Enabled && m_pAlbumRoot->GetFolderByURI( CSchema::uriBitTorrentFolder ) == NULL )
	{
		/*CAlbumFolder* pTorrentFolder	=*/ m_pAlbumRoot->AddFolder( CSchema::uriBitTorrentFolder );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriFavoritesFolder ) == NULL )
	{
		/*CAlbumFolder* pFavorites		=*/ m_pAlbumRoot->AddFolder( CSchema::uriFavoritesFolder );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriCollectionsFolder ) == NULL )
	{
		/*CAlbumFolder* pCollections	=*/ m_pAlbumRoot->AddFolder( CSchema::uriCollectionsFolder );	// Include .torrent
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriGhostFolder ) == NULL )
	{
		/*CAlbumFolder* pGhostFolder	=*/ m_pAlbumRoot->AddFolder( CSchema::uriGhostFolder );
	}

	if ( m_pAlbumRoot->GetFolderCount() != nCount )
	{
		for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
		{
			CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
			if ( pFile->IsAvailable() )
				m_pAlbumRoot->OrganiseFile( pFile );
		}
	}

	return m_pAlbumRoot;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders file delete handler

BOOL CLibraryFolders::OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot )
		m_pAlbumRoot->OnFileDelete( pFile, bDeleteGhost );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		if ( GetNextFolder( pos )->OnFileDelete( pFile ) )
			return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders clear

void CLibraryFolders::Clear()
{
	ASSUME_LOCK( Library.m_pSection );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
		delete GetNextFolder( pos );
	m_pFolders.RemoveAll();

	delete m_pAlbumRoot;
	m_pAlbumRoot = NULL;
}

void CLibraryFolders::ClearGhosts()
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot )
	{
		if ( CAlbumFolder* pGhosts = m_pAlbumRoot->GetFolderByURI( CSchema::uriGhostFolder ) )
		{
			for ( POSITION pos = pGhosts->GetFileIterator() ; pos ; )
			{
				CLibraryFile* pFile = pGhosts->GetNextFile( pos );
				ASSERT( ! pFile->IsAvailable() );
				pFile->Delete( TRUE );
			}
		}
	}
}

DWORD CLibraryFolders::GetGhostCount() const
{
	CQuickLock oLock( Library.m_pSection );

	if ( m_pAlbumRoot )
	{
		if ( CAlbumFolder* pGhosts = m_pAlbumRoot->GetFolderByURI( CSchema::uriGhostFolder ) )
			return pGhosts->GetFileCount();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders thread scan

BOOL CLibraryFolders::ThreadScan(const BOOL bForce)
{
	ASSUME_LOCK( Library.m_pSection );

	BOOL bChanged = FALSE;

	for ( POSITION pos = GetFolderIterator() ; pos && Library.IsThreadEnabled() ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		if ( GetFileAttributes( SafePath( pFolder->m_sPath ) ) != INVALID_FILE_ATTRIBUTES )
		{
			if ( pFolder->SetOnline() ) bChanged = TRUE;

			if ( pFolder->IsChanged() || bForce )
			{
				if ( pFolder->ThreadScan() ) bChanged = TRUE;
			}
		}
		else
		{
			if ( pFolder->SetOffline() ) bChanged = TRUE;
		}
	}

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders serialize

void CLibraryFolders::Serialize(CArchive& ar, int nVersion)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot == NULL )
		m_pAlbumRoot = new CAlbumFolder( NULL, CSchema::uriLibrary );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( GetFolderCount() );

		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			GetNextFolder( pos )->Serialize( ar, nVersion );
		}
	}
	else // Loading
	{
		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CLibraryFolder* pFolder = new CLibraryFolder( NULL );
			pFolder->Serialize( ar, nVersion );
			m_pFolders.AddTail( pFolder );
		}
	}

	//if ( nVersion > 6 )
		m_pAlbumRoot->Serialize( ar, nVersion );
}

void CLibraryFolders::Maintain()
{
	CQuickLock oLock( Library.m_pSection );

#ifndef __IShellLibrary_INTERFACE_DEFINED__	// ~VS2008
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		GetNextFolder( pos )->Maintain( TRUE );
	}
	return;
#endif	// No IShellLibrary (VS2008)

	// Update desktop.ini's only
	if ( ! Settings.Library.UseWindowsLibrary || theApp.m_nWinVer < WIN_7 )
	{
		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			GetNextFolder( pos )->Maintain( TRUE );
		}
		return;
	}

	// Update Windows 7/8 Libraries too
	CComPtr< IShellLibrary > pIShellLib;
	if ( theApp.m_nWinVer >= WIN_7 && Settings.Library.UseWindowsLibrary )
		pIShellLib.CoCreateInstance( CLSID_ShellLibrary );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		pFolder->Maintain( TRUE );

		if ( pIShellLib && theApp.m_pfnSHCreateItemFromParsingName )
		{
			CComPtr< IShellItem > psiFolder;
			theApp.m_pfnSHCreateItemFromParsingName( (LPCWSTR)CT2W( pFolder->m_sPath ), NULL, IID_PPV_ARGS( &psiFolder ) );
			if ( psiFolder )
				pIShellLib->AddFolder( psiFolder );
		}
	}

	if ( pIShellLib )
	{
		CString strPath = Settings.General.Path + L"\\Schemas\\WindowsLibrary.ico";
		LPCTSTR pszPath = strPath;
		if ( ! PathFileExists( pszPath ) )
			pszPath = (LPCWSTR)CT2W( Skin.GetImagePath( IDI_COLLECTION ) );
		pIShellLib->SetIcon( pszPath );

		CComPtr< IShellItem > psiLibrary;
		pIShellLib->SaveInKnownFolder( FOLDERID_UsersLibraries, CLIENT_NAME, LSF_OVERRIDEEXISTING, &psiLibrary );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders ILibraryFolders

IMPLEMENT_DISPATCH(CLibraryFolders, LibraryFolders)

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get__NewEnum(IUnknown FAR* FAR* /*ppEnum*/)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Item(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )

	CQuickLock oLock( Library.m_pSection );

	CLibraryFolder* pFolder = NULL;
	*ppFolder = NULL;

	if ( vIndex.vt == VT_BSTR )
	{
		CString strName( vIndex.bstrVal );
		pFolder = pThis->GetFolder( strName );
	}
	else
	{
		VARIANT va;
		VariantInit( &va );

		if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
			return E_INVALIDARG;
		if ( va.lVal < 0 || va.lVal >= pThis->GetFolderCount() )
			return E_INVALIDARG;

		for ( POSITION pos = pThis->GetFolderIterator() ; pos ; )
		{
			pFolder = pThis->GetNextFolder( pos );
			if ( va.lVal-- == 0 ) break;
			pFolder = NULL;
		}
	}

	*ppFolder = pFolder ? (ILibraryFolder*)pFolder->GetInterface( IID_ILibraryFolder, TRUE ) : NULL;

	return S_OK;
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )

	CQuickLock oLock( Library.m_pSection );

	*pnCount = static_cast< LONG >( pThis->GetFolderCount() );

	return S_OK;
}
