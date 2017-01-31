//
// ThumbCache.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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
#include "SQLite.h"
#include "ThumbCache.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "Library.h"
#include "SharedFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CThumbCache init

void CThumbCache::InitDatabase()
{
	unique_ptr< CDatabase > db( theApp.GetDatabase( DB_THUMBS ) );
	if ( ! *db )
	{
		TRACE( "CThumbCache::InitDatabase : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return;
	}

	// Recreate table
	if ( ! db->Exec( L"CREATE TABLE Files ("
			 L"Filename TEXT UNIQUE NOT NULL PRIMARY KEY, "
			 L"FileSize INTEGER NOT NULL, "
			 L"LastWriteTime INTEGER NOT NULL, "
			 L"Image BLOB NOT NULL, "	// as JPEG
			 L"Flags INTEGER DEFAULT 0 NULL, "
			 L"SHA1 TEXT NULL, TTH TEXT NULL, ED2K TEXT NULL, MD5 TEXT NULL); "
			 L"CREATE INDEX IDX_SHA1 ON Files(SHA1 ASC); "
			 L"CREATE INDEX IDX_TTH ON Files(TTH ASC); "
			 L"CREATE INDEX IDX_ED2K ON Files(ED2K ASC); "
			 L"CREATE INDEX IDX_MD5 ON Files(MD5 ASC);" ) )
	{
		// Cleanup existing
		//TIMER_START
		theApp.KeepAlive();
		db->Exec( L"PRAGMA synchronous=OFF" );		// Async return (15% faster, ~1sec)
		db->Exec( L"PRAGMA journal_mode=OFF" );		// No temp "-journal" rollback file created (2X time)
		db->Exec( L"VACUUM;" );						// Several seconds if large
		theApp.KeepAlive();
		//TIMER_STOP
	}
}

//////////////////////////////////////////////////////////////////////
// CThumbCache load

BOOL CThumbCache::Load(LPCTSTR pszPath, CImageFile* pImage)
{
	ASSERT( pszPath );
	ASSERT( pImage );

	// Load file info from disk
	WIN32_FIND_DATA fd = { 0 };
	if ( ! GetFileAttributesEx( pszPath, GetFileExInfoStandard, &fd ) )
	{
		// Deleted (or Ghost) file
		TRACE( "CThumbCache::Load : Can't load info for %s\n", (LPCSTR)CT2A( pszPath ) );
		return FALSE;
	}

	// Load file info from database
	unique_ptr< CDatabase > db( theApp.GetDatabase( DB_THUMBS ) );
	if ( ! *db )
	{
		TRACE( "CThumbCache::InitDatabase : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return FALSE;
	}

	CString strPath( pszPath );
	strPath.MakeLower();

	if ( ! db->Prepare( L"SELECT FileSize, LastWriteTime, Image FROM Files WHERE Filename == ?;" ) ||
		 ! db->Bind( 1, strPath ) ||
		 ! db->Step() ||
		 ! ( db->GetCount() == 0 || db->GetCount() == 3 ) )
	{
		TRACE( "CThumbCache::Load : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return FALSE;
	}

	if ( db->GetCount() == 0 )
	{
		TRACE( "CThumbCache::Load : No thumbnail for %s\n", (LPCSTR)CT2A( pszPath ) );
		return FALSE;
	}

	QWORD nFileSize = (QWORD)db->GetInt64( L"FileSize" );
	QWORD nLastWriteTime = (QWORD)db->GetInt64( L"LastWriteTime" );
	int data_len;
	LPCVOID data = db->GetBlob( L"Image", &data_len );
	if ( ! data )
	{
		TRACE( "CThumbCache::Load : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return FALSE;
	}

	// Compare it
	BOOL loaded = FALSE;
	if ( nFileSize == MAKEQWORD( fd.nFileSizeLow, fd.nFileSizeHigh ) &&
		nLastWriteTime == MAKEQWORD( fd.ftLastWriteTime.dwLowDateTime, fd.ftLastWriteTime.dwHighDateTime ) )
	{
		// Load image
		loaded = pImage->LoadFromMemory( L".jpg", data, data_len );
	}

	if ( ! loaded )
		Delete( pszPath );		// Remove outdated or bad thumbnail

	return loaded;
}

void CThumbCache::Delete(LPCTSTR pszPath)
{
	unique_ptr< CDatabase > db( theApp.GetDatabase( DB_THUMBS ) );
	if ( ! *db )
	{
		TRACE( "CThumbCache::InitDatabase : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return;
	}

	CString strPath( pszPath );
	strPath.MakeLower();

	if ( ! db->Prepare( L"DELETE FROM Files WHERE Filename == ?;" ) ||
		 ! db->Bind( 1, strPath ) )
	{
		TRACE( "CThumbCache::Load : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return;
	}

	db->Step();
}

//////////////////////////////////////////////////////////////////////
// CThumbCache store

BOOL CThumbCache::Store(LPCTSTR pszPath, CImageFile* pImage)
{
	ASSERT( pszPath );
	ASSERT( pImage );
	ASSERT( pImage->m_nWidth >= 0 && pImage->m_nHeight >= 0 );

	// Load file info from disk
	WIN32_FIND_DATA fd = { 0 };
	if ( ! GetFileAttributesEx( pszPath, GetFileExInfoStandard, &fd ) )
	{
		TRACE( "CThumbCache::Store : Can't load info for %s\n", (LPCSTR)CT2A( pszPath ) );
		return FALSE;
	}

	unique_ptr< CDatabase > db( theApp.GetDatabase( DB_THUMBS ) );
	if ( ! *db )
	{
		TRACE( "CThumbCache::InitDatabase : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return FALSE;
	}

	CString strPath( pszPath );
	strPath.MakeLower();

	// Save to memory as JPEG image
	BYTE* buf = NULL;
	DWORD data_len = 0;
	if ( ! pImage->SaveToMemory( L".jpg", Settings.Library.ThumbQuality, &buf, &data_len ) )		// ~75% JPEG
	{
		TRACE( "CThumbCache::Store : Can't save thumbnail to JPEG for %s\n", (LPCSTR)CT2A( pszPath ) );
		return FALSE;
	}
	auto_array< BYTE > data( buf );

	// Remove old image
	if ( ! db->Prepare( L"DELETE FROM Files WHERE Filename == ?;" ) ||
		 ! db->Bind( 1, strPath ) )
	{
		TRACE( "CThumbCache::Store : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return FALSE;
	}
	db->Step();

	// Store new one
	if ( ! db->Prepare( L"INSERT INTO Files ( Filename, FileSize, LastWriteTime, Image ) VALUES ( ?, ?, ?, ? );" ) ||
		 ! db->Bind( 1, strPath ) ||
		 ! db->Bind( 2, (__int64)MAKEQWORD( fd.nFileSizeLow, fd.nFileSizeHigh ) ) ||
		 ! db->Bind( 3, (__int64)MAKEQWORD( fd.ftLastWriteTime.dwLowDateTime, fd.ftLastWriteTime.dwHighDateTime ) ) ||
		 ! db->Bind( 4, data.get(), data_len ) ||
		 ! db->Step() )
	{
		TRACE( "CThumbCache::Store : Database error: %s\n", (LPCSTR)CT2A( db->GetLastErrorMessage() ) );
		return FALSE;
	}

	TRACE( "CThumbCache::Store : Thumbnail saved for %s\n", (LPCSTR)CT2A( pszPath ) );

	CSingleLock oLock( &Library.m_pSection, FALSE );
	if ( ! oLock.Lock( 300 ) ) return TRUE;

	if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( pszPath ) )
	{
		ASSERT( pFile->GetPath().MakeLower() == strPath );
		pFile->m_bCachedPreview = TRUE;
		Library.Update();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CThumbCache cache

BOOL CThumbCache::Cache(LPCTSTR pszPath, CImageFile* pImage, BOOL bLoadFromFile)
{
	CImageFile pFile;
	if ( ! pImage )
		pImage = &pFile;

	// Load from cache
	if ( CThumbCache::Load( pszPath, pImage ) )
		return TRUE;

	if ( ! bLoadFromFile )
		return FALSE;

	// Load from file
	if ( ! pImage->LoadFromFile( pszPath, FALSE, TRUE ) || ! pImage->EnsureRGB() )
		return FALSE;	// Failed

	// Resample to desired size
	if ( ! pImage->FitTo( Settings.Library.ThumbSize, Settings.Library.ThumbSize ) )	// Was THUMB_STORE_SIZE
		return FALSE;	// Failed

	// Save to cache
	CThumbCache::Store( pszPath, pImage );

	return TRUE;
}
