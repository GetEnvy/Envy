//
// Library.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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
#include "Library.h"
#include "LibraryMaps.h"
#include "LibraryFolders.h"
#include "LibraryDictionary.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"
#include "HashDatabase.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "AlbumFolder.h"
#include "DlgExistingFile.h"
#include "WndMain.h"

#include "QueryHit.h"
#include "QuerySearch.h"
#include "Application.h"

#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CLibrary, CComObject)

BEGIN_INTERFACE_MAP(CLibrary, CComObject)
	INTERFACE_PART(CLibrary, IID_ILibrary, Library)
END_INTERFACE_MAP()

CLibrary Library;


//////////////////////////////////////////////////////////////////////
// CLibrary construction

CLibrary::CLibrary()
	: m_nUpdateCookie	( 0 )
	, m_nForcedUpdate	( FALSE )
	, m_nScanCount		( 0 )
	, m_nScanCookie		( 1 )
	, m_nScanTime		( 0 )
	, m_nSaveCookie		( 0 )
	, m_nSaveTime		( 0 )
//	, m_nFileSwitch		( 0 )	// Using static
{
	EnableDispatch( IID_ILibrary );
}

CLibrary::~CLibrary()
{
}

//////////////////////////////////////////////////////////////////////
// CLibrary file and folder operations

CLibraryFile* CLibrary::LookupFile(DWORD nIndex, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	return LibraryMaps.LookupFile( nIndex, bSharedOnly, bAvailableOnly );
}

CAlbumFolder* CLibrary::GetAlbumRoot()
{
	return LibraryFolders.GetAlbumRoot();
}

void CLibrary::AddFile(CLibraryFile* pFile)
{
	LibraryMaps.OnFileAdd( pFile );

	if ( pFile->HasHash() )
		LibraryDictionary.AddFile( *pFile );

	if ( pFile->IsAvailable() )
	{
		if ( pFile->IsHashed() )
		{
			LibraryHistory.Submit( pFile );
			GetAlbumRoot()->OrganizeFile( pFile );

			if ( pFile->IsNewFile() )			// The new file was hashed
			{
				pFile->m_bNewFile = FALSE;

				CheckDuplicates( pFile );		// Check for duplicates (malware)
			}
		}
		else
		{
			LibraryBuilder.Add( pFile );		// Hash the file and add it again
		}
	}
	else
	{
		GetAlbumRoot()->OrganizeFile( pFile );
	}
}

void CLibrary::RemoveFile(CLibraryFile* pFile)
{
	LibraryMaps.OnFileRemove( pFile );

	LibraryBuilder.Remove( pFile );

	if ( pFile->m_nIndex )
		LibraryDictionary.RemoveFile( *pFile );
}

void CLibrary::CheckDuplicates(const CLibraryFile* pFile, bool bForce) const
{
	ASSUME_LOCK( m_pSection );

	// Malicious software are usually small, we won't search all duplicates
	if ( pFile->m_nSize < 48 || pFile->m_nSize > Settings.Library.MaliciousFileSize ) return;

	LPCTSTR pszExt = _tcsrchr( pFile->m_sName, L'.' );
	if ( ! pszExt ) return;
	if ( ! IsIn( Settings.Library.MaliciousFileTypes, ++pszExt ) ) return;	// exe/com/scr/vbs/wmv/cab/zip/rar/etc.

	DWORD nCount = 0;

	for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		const CLibraryFile* pExisting = LibraryMaps.GetNextFile( pos );

		if ( *pFile == *pExisting )
			nCount++;
	}

	if ( nCount < Settings.Library.MaliciousFileCount )		// Only more than ~4 duplicate files is suspicious
	{
		Settings.Live.LastDuplicateHash.Empty();
		return;
	}

	if ( Settings.Live.LastDuplicateHash == pFile->m_oMD5.toString() && ! bForce )
		return;		// Already warned about the same file

	Settings.Live.LastDuplicateHash = pFile->m_oMD5.toString();
	if ( ! theApp.m_bLive ) return;

	// Warn the user
	CExistingFileDlg dlg( pFile, NULL, true );
	Settings.Live.MaliciousWarning = true;

	//m_pSection.Unlock();

	dlg.DoModal();

	//m_pSection.Lock();

	switch ( dlg.GetResult() )
	{
	case CExistingFileDlg::ShowInLibrary:
		if ( CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd() )
		{
			CString strHash = L"urn:md5:" + Settings.Live.LastDuplicateHash;
			int nLen = strHash.GetLength() + 1;
			LPTSTR pszHash = new TCHAR[ nLen ];

			CopyMemory( pszHash, strHash.GetBuffer(), sizeof( TCHAR ) * nLen );
			pMainWnd->PostMessage( WM_LIBRARYSEARCH, (WPARAM)pszHash );
		}
		break;

	case CExistingFileDlg::Cancel:
		Settings.Live.LastDuplicateHash.Empty();
		break;

	//default:
	//	;
	}

	Settings.Live.MaliciousWarning = false;
}

void CLibrary::CheckDuplicates(LPCTSTR pszMD5Hash) const
{
	Hashes::Md5Hash oMD5;
	oMD5.fromString( pszMD5Hash );

	if ( oMD5 )
	{
		CSingleLock oLock( &m_pSection );
		if ( ! oLock.Lock( 200 ) ) return;

		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByMD5( oMD5, FALSE, TRUE ) )
		{
			CheckDuplicates( pFile, true );
		}
		else
		{
			Settings.Live.LastDuplicateHash.Empty();
			Settings.Live.MaliciousWarning = false;
		}
	}
}

bool CLibrary::OnQueryHits(const CQueryHit* pHits)
{
	CSingleLock oLock( &m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return false;

	for ( const CQueryHit* pHit = pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( ! pHit->m_sURL.IsEmpty() )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByHash( pHit ) )
				pFile->AddAlternateSources( pHit->m_sURL );
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibrary search

CFileList* CLibrary::Search(const CQuerySearch* pSearch, int nMaximum, bool bLocal, bool bAvailableOnly)
{
	ASSUME_LOCK( m_pSection );

	if ( pSearch == NULL )
	{
		// Host browsing
		ASSERT( ! bLocal );
		return LibraryMaps.Browse( nMaximum );
	}

	if ( pSearch->m_bWhatsNew )
	{
		// "Whats New" search
		ASSERT( ! bLocal );
		return LibraryMaps.WhatsNew( pSearch, nMaximum );
	}

	// Hash or exactly filename+size search
	if ( CFileList* pHits = LibraryMaps.LookupFilesByHash( pSearch, ! bLocal, bAvailableOnly, nMaximum ) )
	{
		return pHits;
	}

	// Regular keywords search
	return LibraryDictionary.Search( pSearch, nMaximum, bLocal, bAvailableOnly );
}

//////////////////////////////////////////////////////////////////////
// CLibrary clear

void CLibrary::Clear()
{
	CSingleLock pLock( &m_pSection, TRUE );

	LibraryHistory.Clear();
	LibraryDictionary.Clear();
	LibraryFolders.Clear();
	LibraryMaps.Clear();
}

void CLibrary::StopThread()
{
	Exit();
	Wakeup();
}

//////////////////////////////////////////////////////////////////////
// CLibrary serialize

void CLibrary::Serialize(CArchive& ar)
{
	int nVersion = LIBRARY_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;
	}
	else // Loading
	{
		ar >> nVersion;
	//	if ( nVersion > INTERNAL_VERSION && nVersion != 1000 )	// ToDo: Allow Shareaza imports
	//		AfxThrowUserException();
	}

	LibraryDictionary.Serialize( ar, nVersion );
	LibraryMaps.Serialize1( ar, nVersion );
	LibraryFolders.Serialize( ar, nVersion );
	LibraryHistory.Serialize( ar, nVersion );
	LibraryMaps.Serialize2( ar, nVersion );
}

//////////////////////////////////////////////////////////////////////
// CLibrary load from disk

BOOL CLibrary::SafeSerialize(CArchive& ar) throw()
{
	CFile* fp = ar.GetFile();

	__try
	{
		Serialize( ar );

		ar.Close();
		fp->Close();

		return TRUE;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	__try
	{
		ar.Close();
		fp->Close();

		Clear();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}

	LibraryFolders.CreateAlbumTree();

	return FALSE;
}

BOOL CLibrary::SafeReadTime(CFile& pFile, FILETIME* pFileTime) throw()
{
	__try
	{
		return pFile.Read( pFileTime, sizeof( FILETIME ) ) == sizeof( FILETIME );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	return FALSE;
}

BOOL CLibrary::Load()
{
#ifdef _DEBUG
	const __int64 nBenchmarkStart = GetMicroCount();
#endif

	CFile pFileDat, pFileBak;
	FILETIME pFileDatTime = { 0, 0 }, pFileBakTime = { 0, 0 };

	BOOL bFileDat = pFileDat.Open( Settings.General.DataPath + L"Library.dat", CFile::modeRead ) && SafeReadTime( pFileDat, &pFileDatTime );
	BOOL bFileBak = pFileBak.Open( Settings.General.DataPath + L"Library.bak", CFile::modeRead ) && SafeReadTime( pFileBak, &pFileBakTime );

	// Try legacy format fallback
	if ( ! bFileDat && ! bFileBak )
		bFileDat = pFileDat.Open( Settings.General.DataPath + L"Library1.dat", CFile::modeRead ) && SafeReadTime( pFileDat, &pFileDatTime );

	CSingleLock pLock( &m_pSection, TRUE );

	if ( bFileDat && bFileBak )
	{
		// .dat/.bak files are saved alternately for safe redundancy, so prefer latest one
		CFile* pPreferred = ( ( CompareFileTime( &pFileDatTime, &pFileBakTime ) >= 0 ) ? &pFileDat : &pFileBak );

		CArchive ar( pPreferred, CArchive::load, 262144 );		// 256 KB buffer
		if ( ! SafeSerialize( ar ) )
		{
			pPreferred = pPreferred == &pFileDat ? &pFileBak : &pFileDat;

			CArchive ar2( pPreferred, CArchive::load, 262144 );
			SafeSerialize( ar2 );
		}
	}
	else if ( bFileDat || bFileBak )
	{
		CArchive ar( bFileDat ? &pFileDat : &pFileBak, CArchive::load, 262144 );
		SafeSerialize( ar );
	}
	else
	{
		CreateDirectory( Settings.Downloads.CompletePath );
		LibraryFolders.AddFolder( Settings.Downloads.CompletePath );

		CreateDirectory( Settings.Downloads.CollectionPath );
		LibraryFolders.AddFolder( Settings.Downloads.CollectionPath );

		CreateDirectory( Settings.Downloads.TorrentPath );
		LibraryFolders.AddFolder( Settings.Downloads.TorrentPath );
	}

	LibraryFolders.CreateAlbumTree();
//	LibraryFolders.Maintain();			// Update desktop.ini's	(May take several seconds, call separately)

	LibraryHashDB.Create();
	LibraryBuilder.BoostPriority( Settings.Library.HighPriorityHash );

#ifdef _DEBUG
	const __int64 nBenchmarkEnd = GetMicroCount();
	theApp.Message( MSG_DEBUG, L"Library load time : %I64i ms. Files: %d, Keywords: %d, Names: %d, Paths: %d\n",
		( nBenchmarkEnd - nBenchmarkStart ) / 1000,
		LibraryMaps.GetFileCount(),
		LibraryDictionary.GetWordCount(),
		LibraryMaps.GetNameCount(),
		LibraryMaps.GetPathCount() );
#endif

	Update();

	m_nSaveCookie = m_nUpdateCookie;
	m_nSaveTime = GetTickCount();

	BeginThread( "Library" );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibrary save to disk

BOOL CLibrary::Save()
{
	static BOOL bFileSwitch = FALSE;

	const CString strFile = Settings.General.DataPath +
		( bFileSwitch ? L"Library.bak" : L"Library.dat" );

	bFileSwitch = ! bFileSwitch;
	m_nSaveTime = GetTickCount();

	CSingleLock pLock( &m_pSection, TRUE );

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
	{
		theApp.Message( MSG_ERROR, L"Library save error to: %s", (LPCTSTR)strFile );
		return FALSE;
	}

	try
	{
		FILETIME pFileTime = {};
		pFile.Write( &pFileTime, sizeof( FILETIME ) );
		pFile.Flush();

		CArchive ar( &pFile, CArchive::store, 262144 );		// 256 KB buffer
		try
		{
			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, L"Library save error to: %s", (LPCTSTR)strFile );
			return FALSE;
		}

		pFile.Flush();
		GetSystemTimeAsFileTime( &pFileTime );
		pFile.Seek( 0, 0 );
		pFile.Write( &pFileTime, sizeof( FILETIME ) );
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		theApp.Message( MSG_ERROR, L"Library save error to: %s", (LPCTSTR)strFile );
		return FALSE;
	}

	m_nSaveCookie = m_nUpdateCookie;
	theApp.Message( MSG_DEBUG, L"Library successfully saved to: %s", (LPCTSTR)strFile );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibrary thread run

void CLibrary::OnRun()
{
	while ( IsThreadEnabled() )
	{
		Doze( 1000 );

		// Delay thread load at startup
		if ( ! theApp.m_bLive )
		{
			Sleep( 0 );
			continue;
		}

		ThreadScan();
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary threaded scan

BOOL CLibrary::ThreadScan()
{
	// Do not start scanning until app is loaded
	if ( ! theApp.m_bLive || theApp.m_bClosing ) return FALSE;

	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) )
	{
		Wakeup();	// Skip default delay
		return FALSE;
	}

	// Scan was requested by Library.Update( true ) call
	BOOL bForcedScan = InterlockedCompareExchange( &m_nForcedUpdate, FALSE, TRUE );

	// If folders not watched then scan them at periodic basis (default 5 seconds)
	BOOL bPeriodicScan = ! Settings.Library.WatchFolders &&
		( m_nScanTime < GetTickCount() - Settings.Library.WatchFoldersTimeout * 1000 );

	BOOL bChanged = LibraryFolders.ThreadScan( bPeriodicScan || bForcedScan );

	if ( bPeriodicScan || bForcedScan || bChanged )
	{
		m_nScanTime = GetTickCount();

		if ( bChanged )
			Update();	// Mark library as changed
	}

	m_nScanCount++;

	// Save library changes but not frequently (30 seconds)
	if ( m_nUpdateCookie != m_nSaveCookie && GetTickCount() - m_nSaveTime > 30000 )
		Save();

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibrary sanity check

BOOL CLibrary::IsBadFile(LPCTSTR pszFilenameOnly, LPCTSTR pszPathOnly, DWORD dwFileAttributes)
{
	// Ignore common unshareworthy file/folder types

	if ( dwFileAttributes == INVALID_FILE_ATTRIBUTES )								// Ignore error files (Access denied)
		return TRUE;

	if ( dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM) )			// Ignore hidden or system files or folders
		return TRUE;

	if ( pszFilenameOnly && pszFilenameOnly[0] == L'.' )							// Ignore files or folders begins from dot
		return TRUE;

	if ( pszFilenameOnly && _tcsicmp( pszFilenameOnly, L"Metadata" ) == 0 )		// Ignore metadata file or folder
		return TRUE;

	if ( ! ( dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
	{
		if ( ( dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED ) )						// Ignore encrypted files
			return TRUE;

		if ( pszFilenameOnly )
		{
			if ( _tcsicmp( pszFilenameOnly, L"Thumbs.db" ) == 0 ||				// Ignore windows thumbnail database
				 _tcsicmp( pszFilenameOnly, L"dxva_sig.txt" ) == 0 ||			// Ignore video tag-file
				 _tcsnicmp( pszFilenameOnly, L"~uTorrentPartFile_", 18 ) == 0 ||	// uTorrent part files
				 _tcsnicmp( pszFilenameOnly, L"___ARESTRA___", 13 ) == 0 ||		// Ares Galaxy partials
				 _tcsnicmp( pszFilenameOnly, L"signons", 7 ) == 0 )				// FireFox Password files "signons3.txt"
			{
				return TRUE;
			}

			if ( LPCTSTR pszExt = _tcsrchr( pszFilenameOnly, L'.' ) )
			{
				pszExt++;

				if ( IsIn( Settings.Library.PrivateTypes, pszExt ) )				// Ignore private type files
					return TRUE;

				if ( pszPathOnly && _tcsistr( pszPathOnly, L"kazaa" ) && _tcsicmp( pszExt, L"dat" ) == 0 )
					return TRUE;													// Ignore .dat files in Kazaa folder
			}
		}
	}

	// Ignore Typical Private Directories
	if ( pszPathOnly && _tcsistr( pszPathOnly, L"\\Temporary Internet Files" ) )		// MS Internet Explorer folder
		return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibrary library download queue

IMPLEMENT_DISPATCH_DISPATCH(CLibrary, Library)

STDMETHODIMP_(ULONG) CLibrary::XLibrary::AddRef()
{
	METHOD_PROLOGUE( CLibrary, Library )
	pThis->m_pSection.Lock();
	return pThis->ComAddRef( this );
}

STDMETHODIMP_(ULONG) CLibrary::XLibrary::Release()
{
	METHOD_PROLOGUE( CLibrary, Library )
	pThis->m_pSection.Unlock();
	return pThis->ComRelease( this );
}

STDMETHODIMP CLibrary::XLibrary::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE( CLibrary, Library )
	HRESULT hr = pThis->ComQueryInterface( this, iid, ppvObj );
	if ( SUCCEEDED(hr) ) pThis->m_pSection.Lock();
	return hr;
}

STDMETHODIMP CLibrary::XLibrary::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppApplication == NULL ) return E_INVALIDARG;
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibrary::XLibrary::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppLibrary == NULL ) return E_INVALIDARG;
	*ppLibrary = (ILibrary*)pThis->GetInterface( IID_ILibrary, TRUE );
	return *ppLibrary ? S_OK : E_NOINTERFACE;
}

STDMETHODIMP CLibrary::XLibrary::get_Folders(ILibraryFolders FAR* FAR* ppFolders)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppFolders == NULL ) return E_INVALIDARG;
	*ppFolders = (ILibraryFolders*)pThis->GetInterface( IID_ILibraryFolders, TRUE );
	return *ppFolders ? S_OK : E_NOINTERFACE;
}

STDMETHODIMP CLibrary::XLibrary::get_Albums(IUnknown FAR* FAR* ppAlbums)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppAlbums == NULL ) return E_INVALIDARG;
	return E_NOTIMPL;
}

STDMETHODIMP CLibrary::XLibrary::get_Files(ILibraryFiles FAR* FAR* ppFiles)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppFiles == NULL ) return E_INVALIDARG;
	*ppFiles = (ILibraryFiles*)LibraryMaps.GetInterface( IID_ILibraryFiles, TRUE );
	return *ppFiles ? S_OK : E_NOINTERFACE;
}

STDMETHODIMP CLibrary::XLibrary::FindByName(BSTR sName, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = LibraryMaps.LookupFileByName( CString( sName ), SIZE_UNKNOWN, FALSE, FALSE );
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	return pFile ? S_OK : S_FALSE;
}

STDMETHODIMP CLibrary::XLibrary::FindByPath(BSTR sPath, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = LibraryMaps.LookupFileByPath( CString( sPath ) );
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	return pFile ? S_OK : S_FALSE;
}

STDMETHODIMP CLibrary::XLibrary::FindByURN(BSTR sURN, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = LibraryMaps.LookupFileByURN( CString( sURN ) );
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	return pFile ? S_OK : S_FALSE;
}

STDMETHODIMP CLibrary::XLibrary::FindByIndex(LONG nIndex, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = pThis->LookupFile( (DWORD)nIndex );
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	return pFile ? S_OK : S_FALSE;
}
