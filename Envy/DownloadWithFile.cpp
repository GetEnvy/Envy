//
// DownloadWithFile.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2016
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
#include "DownloadWithFile.h"
#include "DownloadWithTorrent.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadGroups.h"
#include "Downloads.h"
#include "Uploads.h"
#include "Transfers.h"
#include "FragmentedFile.h"

#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"
#include "SharedFile.h"
#include "SchemaCache.h"
#include "XML.h"
#include "ID3.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CDownloadWithFile construction

CDownloadWithFile::CDownloadWithFile()
	: m_bVerify		( TRI_UNKNOWN )
	, m_tReceived	( GetTickCount() - Settings.Downloads.StarveTimeout + Settings.Connection.TimeoutTraffic )
	, m_pFile		( new CFragmentedFile )
	, m_nFileError	( ERROR_SUCCESS )
{
	if ( m_pFile.get() ) m_pFile->SetDownload( static_cast< CDownload*>( this ) );
}

CDownloadWithFile::~CDownloadWithFile()
{
}

BOOL CDownloadWithFile::IsValid() const
{
	return m_pFile.get() && m_pFile->IsValid();
}

BOOL CDownloadWithFile::IsFileOpen() const
{
	return m_pFile.get() && m_pFile->IsOpen();
}

Fragments::List CDownloadWithFile::GetFullFragmentList() const
{
	return m_pFile.get() ? m_pFile->GetFullFragmentList() : Fragments::List( 0 );
}

Fragments::List CDownloadWithFile::GetEmptyFragmentList() const
{
	return m_pFile.get() ? m_pFile->GetEmptyFragmentList() : Fragments::List( 0 );
}

// Get list of empty fragments we really want to download
//Fragments::List CDownloadWithFile::GetWantedFragmentList() const
//{
//	return m_pFile.get() ? m_pFile->GetWantedFragmentList() : Fragments::List( 0 );
//}

CFragmentedFile* CDownloadWithFile::GetFile()
{
	if ( m_pFile.get() ) m_pFile->AddRef();
	return m_pFile.get();
}

BOOL CDownloadWithFile::FindByPath(const CString& sPath) const
{
	return m_pFile.get() && m_pFile->FindByPath( sPath );
}

// Get amount of subfiles
DWORD CDownloadWithFile::GetFileCount() const
{
	return m_pFile.get() ? m_pFile->GetCount() : 0;
}

// Get subfile offset
QWORD CDownloadWithFile::GetOffset(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetOffset( nIndex ) : 0;
}

// Get subfile length
QWORD CDownloadWithFile::GetLength(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetLength( nIndex ) : SIZE_UNKNOWN;
}

// Get path of subfile
CString CDownloadWithFile::GetPath(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetPath( nIndex ) : CString();
}

// Get original name of subfile
CString CDownloadWithFile::GetName(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetName( nIndex ) : CString();
}

// Get completed size of subfile (in bytes)
QWORD CDownloadWithFile::GetCompleted(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetCompleted( nIndex ) : 0;
}

// Select subfile (with user interaction)
int CDownloadWithFile::SelectFile(CSingleLock* pLock /*NULL*/) const
{
	return m_pFile.get() ? m_pFile->SelectFile( pLock ) : -1;
}

// Get last file/disk operation error
DWORD CDownloadWithFile::GetFileError() const
{
	return m_nFileError;
}

// Get more information about last file/disk operation error
const CString& CDownloadWithFile::GetFileErrorString() const
{
	return m_sFileError;
}

// Set file/disk error status
void CDownloadWithFile::SetFileError(DWORD nFileError, LPCTSTR szFileError)
{
	m_nFileError = nFileError;
	m_sFileError = szFileError;
}

// Clear file/disk error status
void CDownloadWithFile::ClearFileError()
{
	m_nFileError = ERROR_SUCCESS;
	m_sFileError.Empty();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile open the file

// Legacy workaround for magnet torrent crash (r9213)
BOOL CDownloadWithFile::OpenFile()
{
	if ( m_sName.IsEmpty() )
		return TRUE;	// Start download without known name (Magnet)

	if ( IsFileOpen() )
		return TRUE;

	SetModified();

	CDownload* pThis = static_cast< CDownload* >( this );	// ToDo: Fix bad inheritance
	if ( m_pFile.get() )
	{
		ClearFileError();

		if ( pThis->IsTorrent() )
		{
			if ( m_pFile->Open( pThis->m_pTorrent, ! IsCompleted() ) )
				return TRUE;
		}
		else
		{
			// ToDo: Refactor m_sTorrentTrackerError
			pThis->m_sTorrentTrackerError.Empty();

			if ( m_pFile->Open( this, ! IsCompleted() ) )
				return TRUE;
		}

		SetFileError( m_pFile->GetFileError(), m_pFile->GetFileErrorString() );
	}
	else if ( m_nSize != SIZE_UNKNOWN &&
		! Downloads.IsSpaceAvailable( m_nSize, Downloads.dlPathIncomplete ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DISK_SPACE, m_sName, Settings.SmartVolume( m_nSize ) );

		m_nFileError = ERROR_DISK_FULL;
	}

	// ToDo: Refactor m_sTorrentTrackerError
	if ( m_nFileError != ERROR_SUCCESS )
		pThis->m_sTorrentTrackerError = GetErrorString( m_nFileError );

	return FALSE;
}

BOOL CDownloadWithFile::Open(const CEnvyFile* pFile)
{
	if ( m_pFile.get() )
	{
		ClearFileError();

		if ( m_pFile->Open( pFile, ! IsCompleted() ) )
			return TRUE;

		SetFileError( m_pFile->GetFileError(), m_pFile->GetFileErrorString() );
	}

	return FALSE;
}

BOOL CDownloadWithFile::Open(const CBTInfo& pBTInfo)
{
	if ( m_pFile.get() )
	{
		ClearFileError();

		if ( m_pFile->Open( pBTInfo, ! IsCompleted() ) )
			return TRUE;

		SetFileError( m_pFile->GetFileError(), m_pFile->GetFileErrorString() );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile close the file

void CDownloadWithFile::CloseFile()
{
	if ( m_pFile.get() )
		m_pFile->Close();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile clear the file

//void CDownloadWithFile::ClearFile()
//{
//	if ( m_pFile.get() )
//		m_pFile->Clear();
//}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile attach the file

void CDownloadWithFile::AttachFile(CFragmentedFile* pFile)
{
	m_pFile.reset( pFile );
	if ( m_pFile.get() ) m_pFile->SetDownload( static_cast< CDownload*>( this ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile delete the file

void CDownloadWithFile::DeleteFile()
{
	ASSERT( ! IsTasking() );

	if ( m_pFile.get() )
	{
		m_pFile->Delete();
		m_pFile.reset();
	}

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile rename the file

bool CDownloadWithFile::Rename(const CString& strName)
{
	CString strNewName = SafeFilename( strName );

	// Don't bother if renaming to same name.
	if ( m_sName == strNewName )
		return false;

	// Rename fragmented files
	if ( m_pFile.get() )
	{
		const DWORD nCount = m_pFile->GetCount();
		for( DWORD nIndex = 0; nIndex < nCount; ++nIndex )
		{
			CString strFragmentName = m_pFile->GetName( nIndex ), strLeftover;
			if ( ! strFragmentName.IsEmpty() )
			{
				int nPos = strFragmentName.Find( L'\\' );
				if ( nPos != -1 )
				{
					strLeftover = strFragmentName.Mid( nPos );
					strFragmentName = strFragmentName.Left( nPos );
				}

				if ( strFragmentName.CompareNoCase( m_sName ) == 0 )
					strFragmentName = strNewName + strLeftover;
				else
					strFragmentName = strNewName + L"\\" + strFragmentName + strLeftover;

				m_pFile->SetName( nIndex, strFragmentName );
			}
		}
	}

	// Set new name
	m_sName = strNewName;
	SetModified();

	return true;
}

// Move file(s) to destination. Returns 0 on success or file error number.
DWORD CDownloadWithFile::MoveFile(LPCTSTR pszDestination, LPPROGRESS_ROUTINE lpProgressRoutine, CDownloadTask* pTask)
{
	ASSERT( IsMoving() );

	DWORD ret = ERROR_SUCCESS;

	if ( ! m_pFile.get() )
	{
		ClearSources();
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_CANT_MOVE ), (LPCTSTR)GetDisplayName(), pszDestination );
		theApp.Message( MSG_ERROR | MSG_TRAY, L"%s %s", (LPCTSTR)strMessage, (LPCTSTR)GetErrorString( ERROR_FILE_NOT_FOUND ) );
		return ERROR_FILE_NOT_FOUND;
	}

	CSingleLock oLibraryLock( &Library.m_pSection, FALSE );

	const CString strRoot = (CString)pszDestination + L"\\";

	const DWORD nCount = m_pFile->GetCount();

	CString strNewFolder;
	if ( nCount > 1 )
	{
		strNewFolder = m_sName;
		strNewFolder.Trim( L" .*<>:?|/\\\"" );
		if ( strNewFolder.FindOneOf( L"*<>:?|/\\\"" ) > 0 )
			strNewFolder.Empty();
	}

	for ( DWORD nIndex = 0; nIndex < nCount; ++nIndex )
	{
		CString strName( m_pFile->GetName( nIndex ) );

		// Change embedded folder when torrent display name changed
		if ( nCount > 1 && ! strNewFolder.IsEmpty() && strNewFolder != strName.Left( strName.Find( L"\\" ) ) )
		{
			strName = strNewFolder + strName.Mid( strName.Find( L"\\" ) );
			m_pFile->SetName( nIndex, strName );
		}

		if ( nCount > 1 && StartsWith( strName, _P( L"_____padding_file_" ) ) )
			continue;	// Skip any torrent padding files

		// Handle conflicts
		if ( Settings.Downloads.RenameExisting && PathFileExists( strRoot + strName ) )
		{
			if ( m_pFile->GetLength( nIndex ) < 2 )
				continue;	// Just skip empty files

			BOOL bDuplicate = FALSE;

			// Do quick size comparison
			if ( oLibraryLock.Lock( 100 ) )
			{
				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strRoot + strName ) )
				{
					if ( pFile->m_nSize == m_pFile->GetLength( nIndex ) )
					{
						bDuplicate = TRUE;
						if ( nCount == 1 && HasHash() &&
							( validAndUnequal( m_oSHA1, pFile->m_oSHA1 ) ||
							  validAndUnequal( m_oTiger, pFile->m_oTiger ) ||
							  validAndUnequal( m_oED2K, pFile->m_oED2K ) ||
							  validAndUnequal( m_oBTH, pFile->m_oBTH ) ) )
							bDuplicate = FALSE;
						//else if ( m_pFile->GetAt( nIndex )->HasHash() )
						// ToDo: Otherwise get temp hash or byte confirmation ?
					}
				}

				oLibraryLock.Unlock();
			}

			if ( bDuplicate )	// Process presumed identical file ?
			{
				theApp.Message( MSG_INFO, L"Overwriting Duplicate File:  %s", (LPCTSTR)strName );
			}
			else // Smart Rename
			{
				CString strFormat = strName;
				int nPos = strFormat.ReverseFind( L'.' );
				if ( nPos > strFormat.ReverseFind( L'\\' ) )
					strFormat.Insert( nPos, L".%i" );		// Filename.1.ext
				else
					strFormat.Append( L".%i" );

				for ( int i = 1; i < 100; i++ )
				{
					strName.Format( strFormat, i );
					if ( ! PathFileExists( strRoot + strName ) )
					{
						m_pFile->SetName( nIndex, strName );
						theApp.Message( MSG_INFO, L"New File Renamed:  %s", (LPCTSTR)strName );
						break;
					}
				}
			}
		}

		// Create new file
		DWORD dwError = m_pFile->Move( nIndex, pszDestination, lpProgressRoutine, pTask );

		if ( dwError != ERROR_SUCCESS )
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_DOWNLOAD_CANT_MOVE ), (LPCTSTR)GetDisplayName(), pszDestination );
			theApp.Message( MSG_ERROR | MSG_TRAY, L"%s %s", (LPCTSTR)strMessage, (LPCTSTR)GetErrorString( dwError ) );
			return dwError;
		}

		const CString strPath = m_pFile->GetPath( nIndex );

		MarkFileAsDownload( strPath );

		// Save download every move (or every few if many)
		if ( nCount < 600 || nIndex % 4 == 0 )
			static_cast< CDownload* >( this )->Save();

		LibraryBuilder.RequestPriority( strPath );

		// Update with download hashes, single-file download only
		if ( nCount == 1 )
			LibraryHistory.Add( strPath, static_cast< CDownload* >( this ) );
		//else // Multifile torrent
			// ToDo: Get hashes for all files of download?

		// Early metadata update
		if ( oLibraryLock.Lock( 100 ) )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath ) )
				pFile->UpdateMetadata( static_cast< CDownload* >( this ) );
			oLibraryLock.Unlock();
		}
	}

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_MOVED, (LPCTSTR)GetDisplayName(), (LPCTSTR)pszDestination );

	return ret;
}

BOOL CDownloadWithFile::FlushFile()
{
	return m_pFile.get() && m_pFile->Flush();
}

BOOL CDownloadWithFile::IsComplete() const
{
	return m_pFile.get() && m_pFile->IsComplete();
}

BOOL CDownloadWithFile::IsRemaining() const
{
	return m_pFile.get() && m_pFile->IsOpen() && ! m_pFile->IsComplete();
}

BOOL CDownloadWithFile::ReadFile(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead)
{
	return m_pFile.get() && m_pFile->Read( nOffset, pData, nLength, pnRead );
}

BOOL CDownloadWithFile::WriteFile(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten)
{
	return m_pFile.get() && m_pFile->Write( nOffset, pData, nLength, pnWritten );
}

QWORD CDownloadWithFile::InvalidateFileRange(QWORD nOffset, QWORD nLength)
{
	return m_pFile.get() && m_pFile->InvalidateRange( nOffset, nLength );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile statistics

float CDownloadWithFile::GetProgress() const
{
	if ( m_nSize == 0 || m_nSize == SIZE_UNKNOWN ) return 0;
	const QWORD nComplete = GetVolumeComplete();
	if ( m_nSize == nComplete ) return 100.0f;
	return float( nComplete * 10000 / m_nSize ) / 100.0f;
}

QWORD CDownloadWithFile::GetVolumeComplete() const
{
	if ( m_pFile.get() )
	{
		if ( m_pFile->IsValid() )
		{
			const QWORD nCompleted = m_pFile->GetCompleted();
			if ( nCompleted != SIZE_UNKNOWN )
				return nCompleted;
		}

		return 0;
	}
	return m_nSize;
}

QWORD CDownloadWithFile::GetVolumeRemaining() const
{
	if ( m_pFile.get() )
	{
		if ( m_pFile->IsValid() )
			return m_pFile->GetRemaining();

		return m_nSize;
	}
	return 0;
}

DWORD CDownloadWithFile::GetTimeRemaining() const
{
	DWORD nSpeed = GetAverageSpeed();
	if ( nSpeed == 0 ) return 0xFFFFFFFF;

	QWORD nRemaining = GetVolumeRemaining();
	if ( nRemaining == SIZE_UNKNOWN ) return 0xFFFFFFFF;

	QWORD nTimeRemaining = nRemaining / nSpeed;
	if ( nTimeRemaining > 0xFFFFFFFF ) return 0xFFFFFFFF;

	return (DWORD)nTimeRemaining;
}

CString CDownloadWithFile::GetDisplayName() const
{
	if ( ! m_sName.IsEmpty() )
		return m_sName;

	if ( m_oSHA1 )
		return m_oSHA1.toShortUrn();
	if ( m_oTiger )
		return m_oTiger.toShortUrn();
	if ( m_oED2K )
		return m_oED2K.toShortUrn();
	if ( m_oBTH )
		return m_oBTH.toShortUrn();
	if ( m_oMD5 )
		return m_oMD5.toShortUrn();

	return Settings.General.LanguageDefault ?
		CString( L"Unknown File" ) :
		LoadString( IDS_STATUS_UNKNOWN );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile get a list of possible download fragments

//Fragments::List CDownloadWithFile::GetPossibleFragments(
//	const Fragments::List& oAvailable, Fragments::Fragment& oLargest)
//{
//	if ( ! m_pFile.get() ) return Fragments::List( oAvailable.limit() );
//	Fragments::List oPossible( oAvailable );
//
//	if ( oAvailable.empty() )
//	{
//		oPossible = GetWantedFragmentList();
//	}
//	else
//	{
//		Fragments::List tmp( inverse( GetWantedFragmentList() ) );
//		oPossible.erase( tmp.begin(), tmp.end() );
//	}
//
//	if ( oPossible.empty() ) return oPossible;
//
//	oLargest = *oPossible.largest_range();
//
//	for ( CDownloadTransfer* pTransfer = GetFirstTransfer();
//		! oPossible.empty() && pTransfer;
//		pTransfer = pTransfer->m_pDlNext )
//	{
//		pTransfer->SubtractRequested( oPossible );
//	}
//
//	return oPossible;
//}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile select a fragment for a transfer

//inline DWORD CalcChunkSize(QWORD nSize)
//{
//	if ( nSize <= 268435456 ) return 1024 * 1024;		// Try to keep chunk size reasonably large
//	DWORD nChunk = DWORD( ( nSize - 1 ) / 256 ), nTemp;	// Default treeheight of 9
//	while ( nTemp = nChunk & ( nChunk - 1 ) ) nChunk = nTemp;
//	return nChunk * 2;
//}

//BOOL CDownloadWithFile::GetFragment(CDownloadTransfer* pTransfer)
//{
//	ASSUME_LOCK( Transfers.m_pSection );
//
//	if ( ! m_pFile.get() ) return NULL;
//
//	Fragments::Fragment oLargest( SIZE_UNKNOWN, SIZE_UNKNOWN );
//	Fragments::List oPossible = GetPossibleFragments( pTransfer->GetSource()->m_oAvailable, oLargest );
//
//	if ( oLargest.begin() == SIZE_UNKNOWN )
//	{
//		ASSERT( oPossible.empty() );
//		return FALSE;
//	}
//
//	if ( ! oPossible.empty() )
//	{
//		Fragments::List::const_iterator pRandom = oPossible.begin()->begin() == 0
//			? oPossible.begin() : oPossible.random_range();
//		// Streaming Download and Rarest Piece Selection?
//		//	: ( Settings.Downloads.NoRandomFragments ? oPossible.begin() : oPossible.random_range() );
//
//		pTransfer->m_nOffset = pRandom->begin();
//		pTransfer->m_nLength = pRandom->size();
//
//		return TRUE;
//	}
//	else
//	{
//		CDownloadTransfer* pExisting = NULL;
//
//		for ( CDownloadTransfer* pOther = GetFirstTransfer(); pOther; pOther = pOther->m_pDlNext )
//		{
//			if ( pOther->m_bRecvBackwards )
//			{
//				if ( pOther->m_nOffset + pOther->m_nLength - pOther->m_nPosition != oLargest.end() )
//					 continue;
//			}
//			else
//			{
//				if ( pOther->m_nOffset + pOther->m_nPosition != oLargest.begin() )
//					continue;
//			}
//
//			pExisting = pOther;
//			break;
//		}
//
//		if ( pExisting == NULL )
//		{
//			pTransfer->m_nOffset = oLargest.begin();
//			pTransfer->m_nLength = oLargest.size();
//			return TRUE;
//		}
//
//		if ( oLargest.size() < 32 ) return FALSE;
//
//		DWORD nOldSpeed	= pExisting->GetAverageSpeed();
//		DWORD nNewSpeed	= pTransfer->GetAverageSpeed();
//		QWORD nLength	= oLargest.size() / 2;
//
//		if ( nOldSpeed > 5 && nNewSpeed > 5 )
//		{
//			nLength = oLargest.size() * nNewSpeed / ( nNewSpeed + nOldSpeed );
//
//			if ( oLargest.size() > 102400 )
//			{
//				nLength = max( nLength, 51200ull );
//				nLength = min( nLength, oLargest.size() - 51200ull );
//			}
//		}
//
//		if ( pExisting->m_bRecvBackwards )
//		{
//			pTransfer->m_nOffset		= oLargest.begin();
//			pTransfer->m_nLength		= nLength;
//			pTransfer->m_bWantBackwards	= FALSE;
//		}
//		else
//		{
//			pTransfer->m_nOffset		= oLargest.end() - nLength;
//			pTransfer->m_nLength		= nLength;
//			pTransfer->m_bWantBackwards	= TRUE;
//		}
//
//		return TRUE;
//	}
//}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile check if a byte position is empty

BOOL CDownloadWithFile::IsPositionEmpty(QWORD nOffset)
{
	return m_pFile.get() &&
		m_pFile->IsValid() &&
		m_pFile->IsPositionRemaining( nOffset );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile check if a range would "help"

//BOOL CDownloadWithFile::AreRangesUseful(const Fragments::List& oAvailable)
//{
//	return m_pFile.get() && m_pFile->IsValid() &&
//		GetWantedFragmentList().overlaps( oAvailable );
//}

//BOOL CDownloadWithFile::IsRangeUseful(QWORD nOffset, QWORD nLength)
//{
//	return m_pFile.get() && m_pFile->IsValid() &&
//		GetWantedFragmentList().overlaps( Fragments::Fragment( nOffset, nOffset + nLength ) );
//}

// Like IsRangeUseful( ) but takes the amount of useful ranges
// relative to the amount of garbage and source speed into account
//BOOL CDownloadWithFile::IsRangeUsefulEnough(CDownloadTransfer* pTransfer, QWORD nOffset, QWORD nLength)
//{
//	if ( ! m_pFile.get() || ! m_pFile->IsValid() )
//		return FALSE;
//
//	// Range is useful if at least byte within the next amount of data transferable within the next 5 seconds is useful
//	DWORD nLength2 = 5 * pTransfer->GetAverageSpeed();
//	if ( nLength2 < nLength )
//	{
//		if ( ! pTransfer->m_bRecvBackwards )
//			nOffset += nLength - nLength2;
//		nLength = nLength2;
//	}
//	return GetWantedFragmentList().overlaps( Fragments::Fragment( nOffset, nOffset + nLength ) );
//}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile get a string of available ranges

bool CDownloadWithFile::GetAvailableRanges(CString& strRanges) const
{
	strRanges.Empty();

	if ( ! m_pFile.get() || ! m_pFile->IsValid() )
		return false;

	Fragments::List oAvailable( inverse( GetEmptyFragmentList() ) );
	if ( oAvailable.empty() )
		return false;

	strRanges = L"bytes ";

	CString strRange;
	Fragments::List::const_iterator pItr = oAvailable.begin();
	const Fragments::List::const_iterator pEnd = oAvailable.end();
	for ( ; pItr != pEnd && strRanges.GetLength() < HTTP_HEADER_MAX_LINE - 256; ++pItr )
	{
		strRange.Format( L"%I64i-%I64i,", pItr->begin(), pItr->end() - 1 );
		strRanges += strRange;
	}

	strRanges.TrimRight( L',' );

	return true;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile clip a range to valid portions

BOOL CDownloadWithFile::ClipUploadRange(QWORD nOffset, QWORD& nLength) const
{
	if ( ! m_pFile.get() || ! m_pFile->IsValid() )
		return FALSE;

	if ( nOffset >= m_nSize || m_pFile->IsPositionRemaining( nOffset ) )
		return FALSE;

	if ( nOffset + nLength > m_nSize )
		nLength = m_nSize - nOffset;

	Fragments::Fragment oMatch( nOffset, nOffset + nLength );
	Fragments::List oList( GetEmptyFragmentList() );
	Fragments::List::const_iterator_pair pMatches = oList.equal_range( oMatch );

	if ( pMatches.first != pMatches.second )
	{
		if ( pMatches.first->begin() <= nOffset )
			nLength = 0;
		else
			nLength = pMatches.first->end() - nOffset;
	}

	return nLength > 0;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile select a random range of available data

BOOL CDownloadWithFile::GetRandomRange(QWORD& nOffset, QWORD& nLength) const
{
	if ( ! m_pFile.get() || ! m_pFile->IsValid() )
		return FALSE;

	if ( m_pFile->GetCompleted() == 0 )
		return FALSE;

	Fragments::List oFilled = inverse( GetEmptyFragmentList() );
	Fragments::List::const_iterator pRandom = oFilled.random_range();

	nOffset = pRandom->begin();
	nLength = pRandom->size();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile erase a range

QWORD CDownloadWithFile::EraseRange(QWORD nOffset, QWORD nLength)
{
	if ( ! m_pFile.get() )
		return 0;

	const QWORD nCount = m_pFile->InvalidateRange( nOffset, nLength );
	if ( nCount > 0 )
		SetModified();
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile submit data

BOOL CDownloadWithFile::SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength)
{
	SetModified();
	m_tReceived = GetTickCount();

	// Note BitTorrent-specific virtual function

	return WriteFile( nOffset, pData, nLength, NULL );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile set size

BOOL CDownloadWithFile::SetSize(QWORD nSize)
{
	m_nSize = nSize;
	return m_pFile.get() && m_pFile->SetSize( nSize );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile make the file appear complete

BOOL CDownloadWithFile::MakeComplete()
{
	return m_pFile.get() && m_pFile->MakeComplete();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile verification handler

BOOL CDownloadWithFile::OnVerify(const CLibraryFile* pFile, TRISTATE bVerified)
{
	if ( ! pFile || ! m_pFile.get() || ! m_pFile->FindByPath( pFile->GetPath() ) )
		return FALSE;

	if ( bVerified != TRI_UNKNOWN )
		m_bVerify = bVerified;

	SetModified();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile append intrinsic metadata

//BOOL CDownloadWithFile::AppendMetadata()
//{
//	if ( ! Settings.Library.VirtualFiles ) return FALSE;
//
//	if ( GetMetadata() == NULL ) return FALSE;
//	CXMLElement* pXML = GetMetadata()->GetFirstElement();
//	if ( pXML == NULL ) return FALSE;
//
//	HANDLE hFile = CreateFile( m_sPath, GENERIC_READ | GENERIC_WRITE,
//		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
//		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
//	VERIFY_FILE_ACCESS( hFile, m_sPath )
//	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;
//
//	CString strURI = GetMetadata()->GetAttributeValue( CXMLAttribute::schemaName );
//	BOOL bSuccess = FALSE;
//
//	if ( CheckURI( strURI, CSchema::uriAudio ) )
//	{
//		if ( _tcsistr( m_sPath, L".mp3" ) != NULL )
//			bSuccess |= AppendMetadataID3v1( hFile, pXML );
//	}
//
//	CloseHandle( hFile );
//	return bSuccess;
//}

//BOOL CDownloadWithFile::AppendMetadataID3v1(HANDLE hFile, CXMLElement* pXML)
//{
//	DWORD nBytes;
//	CString str;
//
//	ID3V1 pID3 = {};
//	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
//
//	if ( !::ReadFile( hFile, &pID3, 3, &nBytes, NULL ) ) return FALSE;
//	if ( memcmp( pID3.szTag, ID3V2_TAG, 3 ) == 0 ) return FALSE;
//
//	ZeroMemory( &pID3, sizeof( pID3 ) );
//	SetFilePointer( hFile, -(int)sizeof( pID3 ), NULL, FILE_END );
//
//	if ( !::ReadFile( hFile, &pID3, sizeof( pID3 ), &nBytes, NULL ) ) return FALSE;
//	if ( memcmp( pID3.szTag, ID3V1_TAG, 3 ) == 0 ) return FALSE;
//
//	ZeroMemory( &pID3, sizeof( pID3 ) );
//	std::memcpy( pID3.szTag, ID3V1_TAG, 3 );
//
//	str = pXML->GetAttributeValue( L"title" );
//	if ( ! str.IsEmpty() )
//		strncpy( pID3.szSongname, CT2CA( str ), 30 );
//	str = pXML->GetAttributeValue( L"artist" );
//	if ( ! str.IsEmpty() )
//		strncpy( pID3.szArtist, CT2CA( str ), 30 );
//	str = pXML->GetAttributeValue( L"album" );
//	if ( ! str.IsEmpty() )
//		strncpy( pID3.szAlbum, CT2CA( str ), 30 );
//	str = pXML->GetAttributeValue( L"year" );
//	if ( ! str.IsEmpty() )
//		strncpy( pID3.szYear, CT2CA( str ), 4 );
//	str = pXML->GetAttributeValue( L"genre" );
//	if ( ! str.IsEmpty() )
//	{
//		int nGenre = LibraryBuilder.LookupID3v1Genre( str );
//		if ( nGenre != -1 )
//			pID3.nGenre = static_cast< BYTE >( nGenre );
//	}
//
//	SetFilePointer( hFile, 0, NULL, FILE_END );
//	::WriteFile( hFile, &pID3, sizeof( pID3 ), &nBytes, NULL );
//
//	return TRUE;
//}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile serialize

// DOWNLOAD_SER_VERSION in Download.h

void CDownloadWithFile::Serialize(CArchive& ar, int nVersion)
{
	CDownloadWithTransfers::Serialize( ar, nVersion );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( m_pFile.get() != NULL );

		// Restore original filename
		DWORD nIndex = 0;
		if ( static_cast< CDownload* >( this )->IsTorrent() )
		{
			CBTInfo& oInfo = static_cast< CDownload* >( this )->m_pTorrent;
			for ( POSITION pos = oInfo.m_pFiles.GetHeadPosition(); pos; ++nIndex )
			{
				CBTInfo::CBTFile* pBTFile = oInfo.m_pFiles.GetNext( pos );
				if ( m_pFile.get() && m_pFile->GetName( nIndex ).IsEmpty() )
					m_pFile->SetName( nIndex, pBTFile->m_sPath );
			}
		}
		else
		{
			if ( m_pFile.get() && m_pFile->GetName( nIndex ).IsEmpty() )
				m_pFile->SetName( nIndex, m_sName );
		}

		SerializeFile( ar, nVersion );
	}
	else
	{
	//	if ( nVersion < 28 )
	//	{
	//		CString strLocalName;
	//		ar >> strLocalName;
	//		if ( ! strLocalName.IsEmpty() )
	//		{
	//			if ( ! m_sPath.IsEmpty() )
	//				::MoveFile( m_sPath, strLocalName + L".pd" );	// Imported .sd?
	//			m_sPath = strLocalName + L".pd";
	//		}
	//	}

		if ( ar.ReadCount() )
			SerializeFile( ar, nVersion );
		else
			CloseFile();
	}
}

void CDownloadWithFile::SerializeFile(CArchive& ar, int nVersion)
{
	if ( m_pFile.get() )
		m_pFile->Serialize( ar, nVersion );
}
