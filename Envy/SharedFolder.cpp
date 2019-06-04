//
// SharedFolder.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2016
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "Library.h"
#include "LibraryDictionary.h"
#include "LibraryFolders.h"
#include "Application.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CLibraryFolder, CComObject)

BEGIN_INTERFACE_MAP(CLibraryFolder, CComObject)
	INTERFACE_PART(CLibraryFolder, IID_ILibraryFolder, LibraryFolder)
	INTERFACE_PART(CLibraryFolder, IID_ILibraryFolders, LibraryFolders)
	INTERFACE_PART(CLibraryFolder, IID_ILibraryFiles, LibraryFiles)
END_INTERFACE_MAP()

//////////////////////////////////////////////////////////////////////
// CLibraryFolder construction

CLibraryFolder::CLibraryFolder(CLibraryFolder* pParent, LPCTSTR pszPath)
	: m_nFiles		( 0 )
	, m_nVolume 	( 0 )
	, m_nScanCookie	( 0 )
	, m_nUpdateCookie ( 0 )
	, m_nSelectCookie ( 0 )
	, m_pParent 	( pParent )
	, m_sPath		( pszPath )
	, m_sName		( PathFindFileName( m_sPath ) )
	, m_bShared 	( pParent ? TRI_UNKNOWN : TRI_TRUE )
	, m_bExpanded	( pParent ? FALSE : TRUE )
	, m_hMonitor	( INVALID_HANDLE_VALUE )
	, m_bForceScan	( TRUE )
	, m_bOffline	( FALSE )
{
	EnableDispatch( IID_ILibraryFolder );
	EnableDispatch( IID_ILibraryFolders );
	EnableDispatch( IID_ILibraryFiles );

	RenewGUID();
}

CLibraryFolder::~CLibraryFolder()
{
	Clear();
}

void CLibraryFolder::CloseMonitor()
{
	if ( m_hMonitor != INVALID_HANDLE_VALUE )
	{
		FindCloseChangeNotification( m_hMonitor );
		m_hMonitor = INVALID_HANDLE_VALUE;
		m_bForceScan = FALSE;
	}
}

bool CLibraryFolder::operator==(const CLibraryFolder& val) const
{
	return ( m_oGUID == val.m_oGUID );
}

void CLibraryFolder::RenewGUID()
{
	CoCreateGuid( reinterpret_cast< GUID* > ( &*m_oGUID.begin() ) );
	m_oGUID.validate();
}

CXMLElement* CLibraryFolder::CreateXML(CXMLElement* pRoot, BOOL bSharedOnly, XmlType nType /*0*/) const
{
	if ( bSharedOnly && ! IsShared() )
		return NULL;

	// ToDo: Support private passkey

	CXMLElement* pFolder;

	if ( nType == xmlDC )
	{
		// Note case-sensitive http://adc.sourceforge.net/ADC.html#_file_list
		pFolder = pRoot->AddElement( L"Directory" );
		if ( pFolder )
			pFolder->AddAttribute( L"Name", m_sName );
	}
	else // Default
	{
		pFolder = pRoot->AddElement( L"folder" );
		if ( pFolder )
			pFolder->AddAttribute( L"name", m_sName );
	}

	if ( ! pFolder )
		return NULL;	// Out of memory

	for ( POSITION pos = GetFolderIterator(); pos; )
	{
		GetNextFolder( pos )->CreateXML( pFolder, bSharedOnly, nType );
	}

	for ( POSITION pos = GetFileIterator(); pos; )
	{
		GetNextFile( pos )->CreateXML( pFolder, bSharedOnly, nType );
	}

	return pFolder;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder folder list

POSITION CLibraryFolder::GetFolderIterator() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolders.GetStartPosition();
}

CLibraryFolder* CLibraryFolder::GetNextFolder(POSITION& pos) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolders.GetNextValue( pos );
}

CLibraryFolder* CLibraryFolder::GetFolderByName(LPCTSTR pszName) const
{
	ASSUME_LOCK( Library.m_pSection );

	CLibraryFolder* pOutput;
	LPCTSTR szNextName = _tcschr( pszName, L'\\' );
	if ( szNextName )
	{
		CString strName( pszName, (int)( szNextName - pszName ) );
		if ( m_pFolders.Lookup( strName, pOutput ) )
			return pOutput->GetFolderByName( szNextName + 1 );
	}
	else
	{
		if ( m_pFolders.Lookup( pszName, pOutput ) )
			return pOutput;
	}
	return NULL;
}

CLibraryFolder* CLibraryFolder::GetFolderByPath(const CString& strPath) const
{
	ASSUME_LOCK( Library.m_pSection );

	// Test for exact match
	if ( m_sPath.CompareNoCase( strPath ) == 0 )
		return const_cast< CLibraryFolder* >( this );

	// Test for partial match
	if ( m_sPath.GetLength() > strPath.GetLength() )
	{
		const CString strPart = m_sPath.Right( strPath.GetLength() );
		const TCHAR cDel = m_sPath.GetAt( m_sPath.GetLength() - strPath.GetLength() - 1 );
		if ( cDel == L'\\' && strPart.CompareNoCase( strPath ) == 0 )
			return const_cast< CLibraryFolder* >( this );
	}

	// Test for nested folders
	for ( POSITION pos = GetFolderIterator(); pos; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos )->GetFolderByPath( strPath );
		if ( pFolder ) return pFolder;
	}

	return NULL;
}

BOOL CLibraryFolder::CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive) const
{
	ASSUME_LOCK( Library.m_pSection );

	for ( POSITION pos = GetFolderIterator(); pos; )
	{
		CLibraryFolder* pCheck = GetNextFolder( pos );
		if ( pCheck == pFolder ) return TRUE;
		if ( bRecursive && pCheck->CheckFolder( pFolder, TRUE ) ) return TRUE;
	}

	return FALSE;
}

DWORD CLibraryFolder::GetFolderCount() const
{
	ASSUME_LOCK( Library.m_pSection );

	return (DWORD)m_pFolders.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder file list

POSITION CLibraryFolder::GetFileIterator() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFiles.GetStartPosition();
}

CLibraryFile* CLibraryFolder::GetNextFile(POSITION& pos) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFiles.GetNextValue( pos );
}

CLibraryFile* CLibraryFolder::GetFile(LPCTSTR pszName) const
{
	ASSUME_LOCK( Library.m_pSection );

	CLibraryFile* pOutput;
	return ( m_pFiles.Lookup( pszName, pOutput ) ) ? pOutput : NULL;
}

DWORD CLibraryFolder::GetFileCount() const
{
	ASSUME_LOCK( Library.m_pSection );

	return (DWORD)m_pFiles.GetCount();
}

DWORD CLibraryFolder::GetFileList(CLibraryList* pList, BOOL bRecursive) const
{
	DWORD nCount = 0;

	for ( POSITION pos = GetFileIterator(); pos; )
	{
		const CLibraryFile* pFile = GetNextFile( pos );
		pList->CheckAndAdd( pFile );
		nCount++;
	}

	if ( bRecursive )
	{
		for ( POSITION pos = GetFolderIterator(); pos; )
		{
			const CLibraryFolder* pFolder = GetNextFolder( pos );
			nCount += pFolder->GetFileList( pList, bRecursive );
		}
	}

	return nCount;
}

DWORD CLibraryFolder::GetSharedCount(BOOL bRecursive) const
{
	ASSUME_LOCK( Library.m_pSection );

	DWORD nCount = 0;

	for ( POSITION pos = GetFileIterator(); pos; )
	{
		const CLibraryFile* pFile = GetNextFile( pos );
		if ( pFile->IsShared() ) nCount++;
	}

	if ( bRecursive )
	{
		for ( POSITION pos = GetFolderIterator(); pos; )
		{
			const CLibraryFolder* pFolder = GetNextFolder( pos );
			nCount += pFolder->GetSharedCount( bRecursive );
		}
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder clear

void CLibraryFolder::Clear()
{
	CloseMonitor();

	while ( POSITION pos = GetFileIterator() )
	{
		CLibraryFile* pFile = GetNextFile( pos );
		VERIFY( m_pFiles.RemoveKey( pFile->m_sName ) );
		delete pFile;
	}

	while ( POSITION pos = GetFolderIterator() )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );
		VERIFY( m_pFolders.RemoveKey( pFolder->m_sName ) );
		delete pFolder;
	}

	m_nFiles	= 0;
	m_nVolume	= 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder serialize

void CLibraryFolder::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		SaveGUID( m_sPath, m_oGUID );

		ar << m_sPath;
		ar << m_bShared;
		ar << m_bExpanded;

		ar.WriteCount( GetFolderCount() );

		for ( POSITION pos = GetFolderIterator(); pos; )
		{
			GetNextFolder( pos )->Serialize( ar, nVersion );
		}

		ar.WriteCount( GetFileCount() );

		for ( POSITION pos = GetFileIterator(); pos; )
		{
			GetNextFile( pos )->Serialize( ar, nVersion );
		}
	}
	else // Loading
	{
		Clear();

		ar >> m_sPath;
		m_sName = PathFindFileName( m_sPath );

		LoadGUID( m_sPath, m_oGUID );

		ar >> m_bShared;
		ar >> m_bExpanded;

		UINT nCount = (UINT)ar.ReadCount();	// DWORD_PTR

		m_pFolders.InitHashTable( GetBestHashTableSize( nCount ), FALSE );

		for ( ; nCount > 0; nCount-- )
		{
			CLibraryFolder* pFolder = new CLibraryFolder( this );
			if ( ! pFolder )
				break;		// theApp.Message( MSG_DEBUG, L"Memory allocation error in CLibraryFolder::Serialize" );

			pFolder->Serialize( ar, nVersion );

			m_pFolders.SetAt( pFolder->m_sName, pFolder );

			m_nFiles  += pFolder->m_nFiles;
			m_nVolume += pFolder->m_nVolume;
		}

		nCount = (UINT)ar.ReadCount();	// DWORD_PTR
		m_pFiles.InitHashTable( GetBestHashTableSize( nCount ), FALSE );

		for ( ; nCount > 0; nCount-- )
		{
			//CLibraryFile* pFile = new CLibraryFile( this );
			CAutoPtr< CLibraryFile > pFile( new CLibraryFile( this ) );
			if ( ! pFile )
				AfxThrowMemoryException();
				// theApp.Message( MSG_DEBUG, L"Memory allocation error in CLibraryFolder::Serialize" );

			pFile->Serialize( ar, nVersion );

			m_pFiles.SetAt( pFile->m_sName, pFile );

			m_nFiles++;
			m_nVolume += pFile->m_nSize;

			Library.AddFile( pFile.Detach() );

			theApp.KeepAlive();
		}
	}
}

CString CLibraryFolder::GetRelativeName() const
{
	if ( m_pParent )
		return m_pParent->GetRelativeName() + L"\\" + m_sName;

	return m_sName;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder threaded scan

BOOL CLibraryFolder::ThreadScan(DWORD nScanCookie)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_sPath.CompareNoCase( Settings.Downloads.IncompletePath ) == 0 )
		return FALSE;

	WIN32_FIND_DATA pFind;
	HANDLE hSearch = FindFirstFile( SafePath( m_sPath + L"\\*.*" ), &pFind );

	m_nScanCookie	= nScanCookie;
	nScanCookie		= Library.GetScanCookie();

	BOOL bChanged = FALSE;

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( CLibrary::IsBadFile( pFind.cFileName, m_sPath, pFind.dwFileAttributes ) )
				continue;

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				CLibraryFolder* pFolder = GetFolderByName( pFind.cFileName );
				if ( pFolder != NULL )
				{
					m_nFiles  -= pFolder->m_nFiles;
					m_nVolume -= pFolder->m_nVolume;

					if ( _tcsicmp( pFolder->m_sName, pFind.cFileName ) != 0 )
					{
						VERIFY( m_pFolders.RemoveKey( pFolder->m_sName ) );
						pFolder->OnDelete();
						pFolder = new CLibraryFolder( this, m_sPath + L"\\" + pFind.cFileName );
						m_pFolders.SetAt( pFolder->m_sName, pFolder );
						m_nUpdateCookie++;
						bChanged = TRUE;
					}
				}
				else
				{
					pFolder = new CLibraryFolder( this, m_sPath + L"\\" + pFind.cFileName );
					m_pFolders.SetAt( pFolder->m_sName, pFolder );
					m_nUpdateCookie++;
					bChanged = TRUE;
				}

				if ( pFolder->ThreadScan( nScanCookie ) )
					bChanged = TRUE;

				m_nFiles	+= pFolder->m_nFiles;
				m_nVolume	+= pFolder->m_nVolume;
			}
			else
			{
				BOOL bNew;
				CLibraryFile* pFile = AddFile( pFind.cFileName, bNew );
				if ( ! bNew )
				{
					m_nVolume -= pFile->m_nSize;
					if ( pFile->m_sName.CompareNoCase( pFind.cFileName ) )
					{
						Library.RemoveFile( pFile );
						pFile->m_sName = pFind.cFileName;
						Library.AddFile( pFile );
						bChanged = TRUE;
					}
				}
				else
					bChanged = TRUE;

				QWORD nLongSize = (QWORD)pFind.nFileSizeLow |
					( (QWORD)pFind.nFileSizeHigh << 32 );

				if ( pFile->ThreadScan( nScanCookie, nLongSize, &pFind.ftLastWriteTime ) )
					bChanged = TRUE;

				m_nVolume += pFile->m_nSize;
			}
		}
		while ( Library.IsThreadEnabled() && FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}

	if ( ! Library.IsThreadEnabled() ) return FALSE;

	CList< CLibraryFolder* > pExtraFolders;
	for ( POSITION pos = GetFolderIterator(); pos; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		if ( pFolder->m_nScanCookie != nScanCookie )
		{
			pExtraFolders.AddTail( pFolder );
		}
	}

	CList< CLibraryFile* > pExtraFiles;
	for ( POSITION pos = GetFileIterator(); pos; )
	{
		CLibraryFile* pFile = GetNextFile( pos );

		if ( pFile->m_nScanCookie != nScanCookie )
		{
			pExtraFiles.AddTail( pFile );
		}
	}

	for ( POSITION pos = pExtraFolders.GetHeadPosition() ; pos ; )
	{
		CLibraryFolder* pFolder = pExtraFolders.GetNext( pos );

		m_nFiles	-= pFolder->m_nFiles;
		m_nVolume	-= pFolder->m_nVolume;

		VERIFY( m_pFolders.RemoveKey( pFolder->m_sName ) );
		pFolder->OnDelete( Settings.Library.CreateGhosts ? TRI_TRUE : TRI_FALSE );

		bChanged = TRUE;
	}

	for ( POSITION pos = pExtraFiles.GetHeadPosition(); pos; )
	{
		CLibraryFile* pFile = pExtraFiles.GetNext( pos );

		pFile->OnDelete( TRUE, Settings.Library.CreateGhosts ? TRI_TRUE : TRI_FALSE );

		bChanged = TRUE;
	}

	if ( bChanged )
		m_nUpdateCookie++;

	return bChanged;
}

CLibraryFile* CLibraryFolder::AddFile(LPCTSTR szName, BOOL& bNew)
{
	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = GetFile( szName );
	if ( pFile == NULL )
	{
		pFile = new CLibraryFile( this, szName );
		m_pFiles.SetAt( pFile->m_sName, pFile );
		m_nFiles++;
		m_nUpdateCookie++;
		bNew = TRUE;
	}
	else
	{
		bNew = FALSE;
	}
	return pFile;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder monitor

BOOL CLibraryFolder::IsChanged()
{
	BOOL bChanged = FALSE;

	// Monitor changes
	if ( m_hMonitor != INVALID_HANDLE_VALUE )
	{
		switch ( WaitForSingleObject( m_hMonitor, 0 ) )
		{
		case WAIT_TIMEOUT:		// No changes
			break;

		case WAIT_OBJECT_0:		// Changes detected
			bChanged = TRUE;

			// Reset monitor
			if ( FindNextChangeNotification( m_hMonitor ) )
				break;

		default:				// Errors
			CloseMonitor();
		}
	}

	if ( m_hMonitor == INVALID_HANDLE_VALUE && Settings.Library.WatchFolders )
	{
		// Enable monitor
		m_hMonitor = FindFirstChangeNotification( m_sPath, TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_LAST_WRITE );
	}
	else if ( m_hMonitor != INVALID_HANDLE_VALUE && ! Settings.Library.WatchFolders )
	{
		// Disable monitor
		CloseMonitor();
	}

	if ( m_bForceScan )
	{
		// Change status forced
		bChanged = TRUE;
		m_bForceScan = FALSE;
	}

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder scan

void CLibraryFolder::Scan()
{
	CLibraryFolder* pFolder = this;
	for ( ; pFolder->m_pParent; pFolder = pFolder->m_pParent );
	if ( pFolder ) pFolder->m_bForceScan = TRUE;
	Library.Wakeup();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder shared status managing

BOOL CLibraryFolder::IsShared() const
{
	if ( m_bOffline )
		return FALSE;

	for ( const CLibraryFolder* pFolder = this; pFolder; pFolder = pFolder->m_pParent )
	{
		if ( pFolder->m_bShared )
		{
			if ( pFolder->m_bShared == TRI_TRUE )
				return TRUE;
			if ( pFolder->m_bShared == TRI_FALSE )
				return FALSE;
			//if ( pFolder->m_bShared == TRI_UNKNOWN )
			//	return FALSE;
		}
	}

	return TRUE;
}

void CLibraryFolder::GetShared(BOOL& bShared) const
{
	if ( m_bOffline )
		bShared = FALSE;
	else if ( m_bShared == TRI_TRUE )
		bShared = TRUE;
	else if ( m_bShared == TRI_FALSE )
		bShared = FALSE;
}

void CLibraryFolder::SetShared(TRISTATE bShared)
{
	if ( m_bShared != bShared )
	{
		m_bShared = bShared;
		m_nUpdateCookie++;

		LibraryDictionary.Invalidate();
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder offline status managing

BOOL CLibraryFolder::IsOffline() const
{
	return m_bOffline;
}

BOOL CLibraryFolder::SetOffline()
{
	if ( ! m_bOffline )
	{
		m_bOffline = TRUE;
		m_nUpdateCookie++;

		for ( POSITION pos = GetFolderIterator(); pos; )
		{
			GetNextFolder( pos )->SetOffline();
		}

		LibraryDictionary.Invalidate();

		return TRUE;
	}
	return FALSE;
}

BOOL CLibraryFolder::SetOnline()
{
	if ( m_bOffline )
	{
		m_bOffline = FALSE;
		m_nUpdateCookie++;

		for ( POSITION pos = GetFolderIterator(); pos; )
		{
			GetNextFolder( pos )->SetOnline();
		}

		LibraryDictionary.Invalidate();

		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder callbacks

void CLibraryFolder::OnDelete(TRISTATE bCreateGhost)
{
	while ( POSITION pos = GetFileIterator() )
	{
		CLibraryFile* pFile = GetNextFile( pos );
		VERIFY( m_pFiles.RemoveKey( pFile->m_sName ) );
		pFile->OnDelete( FALSE, bCreateGhost );
	}

	while ( POSITION pos = GetFolderIterator() )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );
		VERIFY( m_pFolders.RemoveKey( pFolder->m_sName ) );
		pFolder->OnDelete( bCreateGhost );
	}

	delete this;
}

BOOL CLibraryFolder::OnFileDelete(CLibraryFile* pRemovingFile)
{
	CLibraryFile* pFile;
	if ( m_pFiles.Lookup( pRemovingFile->m_sName, pFile ) )
	{
		if ( pFile == pRemovingFile )
		{
			m_nFiles --;
			m_nVolume -= pFile->m_nSize;
			VERIFY( m_pFiles.RemoveKey( pFile->m_sName ) );
			m_nUpdateCookie++;
			return TRUE;
		}
	}

	for ( POSITION pos = GetFolderIterator(); pos; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );
		if ( pFolder->OnFileDelete( pRemovingFile ) )
			return TRUE;
	}

	return FALSE;
}

void CLibraryFolder::OnFileRename(CLibraryFile* pFile)
{
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		CLibraryFile* pOld;
		CString strName;

		m_pFiles.GetNextAssoc( pos, strName, pOld );

		if ( pFile == pOld )
		{
			VERIFY( m_pFiles.RemoveKey( strName ) );
			m_pFiles.SetAt( pFile->m_sName, pFile );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders maintain physical folder icon (desktop.ini)

void CLibraryFolder::Maintain(BOOL bAdd)
{
	if ( ! Settings.Library.UseCustomFolders )
		bAdd = FALSE;

	CString strDesktopINI( m_sPath + L"\\desktop.ini" );
	DWORD dwDesktopINIAttr = GetFileAttributes( strDesktopINI );

	CString strIconResource;
	CString strInfoTip = LoadString( IDS_FOLDER_TIP );
	CString strIconIndex = L"0";
	CString strIconFile = Settings.General.Path + (
#ifdef XPSUPPORT
		theApp.m_nWinVer < WIN_VISTA ? L"\\Schemas\\WindowsFolder.Safe.ico" :
#endif
		theApp.m_nWinVer < WIN_10 ? L"\\Schemas\\WindowsFolder.Alt.ico" : L"\\Schemas\\WindowsFolder.ico" );
	if ( ! PathFileExists( strIconFile ) )
	{
		strIconResource = Skin.GetImagePath( IDI_COLLECTION );
		int nPos = strIconResource.ReverseFind( L',' );
		strIconIndex = strIconResource.Mid( nPos + 1 );
		strIconFile  = strIconResource.Left( nPos );
	}

	// Check if file exists, is our desktop.ini, and requires changes
	if ( dwDesktopINIAttr == INVALID_FILE_ATTRIBUTES )
	{
		if ( ! bAdd )
			return;

		// New Unicode File
		CFile file;
		WORD wMark = 0xfeff;
		if ( file.Open( strDesktopINI, CFile::modeCreate | CFile::modeWrite ) )
		{
			file.Write( &wMark, 2 );
			file.Close();
		}
	}
	else
	{
		CString strIconFileGet;
		GetPrivateProfileString( L".ShellClassInfo", L"IconFile", L"",
			strIconFileGet.GetBuffer( MAX_PATH ), MAX_PATH, strDesktopINI );
		strIconFileGet.ReleaseBuffer();
		strIconFileGet.Trim();

		if ( ! strIconFileGet.IsEmpty() &&
			 ! StartsWith( strIconFileGet, Settings.General.Path ) &&
			 strIconFileGet.Find( L"Envy" ) == -1 )
			return;		// Not ours, no action required (ToDo: Handle Shareaza?)

		if ( ! bAdd )
		{
			PathUnmakeSystemFolder( m_sPath );
			SetFileAttributes( strDesktopINI, dwDesktopINIAttr & ~( FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY ) );
			DeleteFileEx( strDesktopINI, FALSE, FALSE, FALSE );
			return;
		}

		CString strInfoTipGet, strIconIndexGet, strIconResourceGet;

		GetPrivateProfileString( L".ShellClassInfo", L"IconIndex", L"",
			strIconIndexGet.GetBuffer( MAX_PATH ), MAX_PATH, strDesktopINI );
		strIconIndexGet.ReleaseBuffer();
		strIconIndexGet.Trim();

		GetPrivateProfileString( L".ShellClassInfo", L"InfoTip", L"",
			strInfoTipGet.GetBuffer( MAX_PATH ), MAX_PATH, strDesktopINI );
		strInfoTipGet.ReleaseBuffer();
		strInfoTipGet.Trim();

		GetPrivateProfileString( L".ShellClassInfo", L"IconResource", L"",
			strIconResourceGet.GetBuffer( MAX_PATH ), MAX_PATH, strDesktopINI );
		strIconResourceGet.ReleaseBuffer();
		strIconResourceGet.Trim();

		if ( ! strIconResourceGet.IsEmpty() && ! StartsWith( strIconResourceGet, Settings.General.Path ) )
			return;		// Not ours?

		if ( strInfoTip.Compare( strInfoTipGet ) == 0 &&
			 strIconFile.Compare( strIconFileGet ) == 0 &&
			 strIconIndex.Compare( strIconIndexGet ) == 0 &&
			 strIconResource.Compare( strIconResourceGet ) == 0 )
			return;		// No action required

		// Remove Hidden and System attributes temporarily
		if ( ( dwDesktopINIAttr & ( FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) )
			SetFileAttributes( strDesktopINI, dwDesktopINIAttr & ~( FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) );
	}

	WritePrivateProfileString( L".ShellClassInfo", L"ConfirmFileOp", L"0", strDesktopINI );
	WritePrivateProfileString( L".ShellClassInfo", L"InfoTip", strInfoTip, strDesktopINI );
	WritePrivateProfileString( L".ShellClassInfo", L"IconFile", strIconFile, strDesktopINI );
	WritePrivateProfileString( L".ShellClassInfo", L"IconIndex", strIconIndex, strDesktopINI );
	if ( ! strIconResource.IsEmpty() )
		WritePrivateProfileString( L".ShellClassInfo", L"IconResource", strIconResource, strDesktopINI );

	SetFileAttributes( strDesktopINI, dwDesktopINIAttr | ( FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) );

	PathMakeSystemFolder( m_sPath );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder ILibraryFolder

IMPLEMENT_DISPATCH(CLibraryFolder, LibraryFolder)

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Parent(ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	if ( pThis->m_pParent )
		*ppFolder = (ILibraryFolder*)pThis->m_pParent->GetInterface( IID_ILibraryFolder, TRUE );
	else
		*ppFolder = NULL;
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Path(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*psPath = CComBSTR( pThis->m_sPath ).Detach();
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Name(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*psPath = CComBSTR( pThis->m_sName ).Detach();
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Shared(TRISTATE FAR* pnValue)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*pnValue = pThis->m_bShared;
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::put_Shared(TRISTATE nValue)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	pThis->m_bShared = nValue;
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_EffectiveShared(VARIANT_BOOL FAR* pbValue)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*pbValue = pThis->IsShared() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Folders(ILibraryFolders FAR* FAR* ppFolders)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*ppFolders = (ILibraryFolders*)pThis->GetInterface( IID_ILibraryFolders, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Files(ILibraryFiles FAR* FAR* ppFiles)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*ppFiles = (ILibraryFiles*)pThis->GetInterface( IID_ILibraryFiles, TRUE );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder ILibraryFolders

IMPLEMENT_DISPATCH(CLibraryFolder, LibraryFolders)

STDMETHODIMP CLibraryFolder::XLibraryFolders::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibraryFolder::XLibraryFolders::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolders::get__NewEnum(IUnknown FAR* FAR* /*ppEnum*/)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFolder::XLibraryFolders::get_Item(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )

	CQuickLock oLock( Library.m_pSection );

	CLibraryFolder* pFolder = NULL;
	*ppFolder = NULL;

	if ( vIndex.vt == VT_BSTR )
	{
		CString strName( vIndex.bstrVal );
		if ( strName.Find( L'\\' ) >= 0 )
			pFolder = pThis->GetFolderByPath( strName );
		else
			pFolder = pThis->GetFolderByName( strName );
	}
	else
	{
		VARIANT va;
		VariantInit( &va );

		if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
			return E_INVALIDARG;
		if ( va.lVal < 0 || va.lVal >= (LONG)pThis->GetFolderCount() )
			return E_INVALIDARG;

		for ( POSITION pos = pThis->GetFolderIterator(); pos; )
		{
			pFolder = pThis->GetNextFolder( pos );
			if ( va.lVal-- == 0 ) break;
			pFolder = NULL;
		}
	}

	*ppFolder = pFolder ? (ILibraryFolder*)pFolder->GetInterface( IID_ILibraryFolder, TRUE ) : NULL;

	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolders::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )

	CQuickLock oLock( Library.m_pSection );

	*pnCount = static_cast< LONG >( pThis->GetFolderCount() );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder ILibraryFiles

IMPLEMENT_DISPATCH(CLibraryFolder, LibraryFiles)

STDMETHODIMP CLibraryFolder::XLibraryFiles::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibraryFolder::XLibraryFiles::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFiles::get__NewEnum(IUnknown FAR* FAR* /*ppEnum*/)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFolder::XLibraryFiles::get_Item(VARIANT vIndex, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = NULL;
	*ppFile = NULL;

	if ( vIndex.vt == VT_BSTR )
	{
		CString strName( vIndex.bstrVal );
		pFile = pThis->GetFile( strName );
	}
	else
	{
		VARIANT va;
		VariantInit( &va );

		if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
			return E_INVALIDARG;
		if ( va.lVal < 0 || va.lVal >= (LONG)pThis->GetFileCount() )
			return E_INVALIDARG;

		for ( POSITION pos = pThis->GetFileIterator(); pos; )
		{
			pFile = pThis->GetNextFile( pos );
			if ( va.lVal-- == 0 ) break;
			pFile = NULL;
		}
	}

	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;

	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFiles::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )

	CQuickLock oLock( Library.m_pSection );

	*pnCount = static_cast< LONG >( pThis->GetFileCount() );

	return S_OK;
}
