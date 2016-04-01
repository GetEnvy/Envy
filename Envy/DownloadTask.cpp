//
// DownloadTask.cpp
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "DownloadTask.h"
#include "DownloadGroups.h"
#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"
#include "Uploads.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "LibraryMaps.h"
#include "SharedFile.h"
#include "FragmentedFile.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "HttpRequest.h"
#include "BTInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


/////////////////////////////////////////////////////////////////////////////
// CDownloadTask construction

CDownloadTask::CDownloadTask(CDownload* pDownload)
	: m_pDownload		( pDownload )
	, m_nTask			( dtaskNone )
	, m_bSuccess		( false )
	, m_nFileError		( NO_ERROR )
	, m_bMergeValidation( FALSE )
	, m_oMergeGaps		( 0 )
	, m_fProgress		( 0 )
{
}

CDownloadTask::~CDownloadTask()
{
	Abort();
}

void CDownloadTask::Construct(dtask nTask)
{
	ASSERT_VALID( m_pDownload );
	ASSERT( m_nTask == dtaskNone );
	ASSERT( ! IsThreadAlive() );

	m_nTask = nTask;
	m_pRequest.Free();
	m_bSuccess = false;
	m_sDestination.Empty();
	m_nFileError = NO_ERROR;
	m_oMergeFiles.RemoveAll();
	m_bMergeValidation = FALSE;
	m_fProgress = 0;
}

bool CDownloadTask::HasSucceeded() const
{
	return m_bSuccess;
}

DWORD CDownloadTask::GetFileError() const
{
	return m_nFileError;
}

dtask CDownloadTask::GetTaskType() const
{
	return m_nTask;
}

CString CDownloadTask::GetRequest() const
{
	return m_pRequest ? m_pRequest->GetURL() : CString();
}

float CDownloadTask::GetProgress() const
{
	return m_fProgress;
}


/////////////////////////////////////////////////////////////////////////////
// CDownloadTask operations construction

void CDownloadTask::Allocate()
{
	Construct( dtaskAllocate );

	VERIFY( BeginThread( "Download Task : Allocate" ) );
}

void CDownloadTask::Copy()
{
	Construct( dtaskCopy );

	m_sDestination = DownloadGroups.GetCompletedPath( m_pDownload ).TrimRight( L"\\" );

	VERIFY( BeginThread( "Download Task : Copy" ) );		// RunCopy()
}

void CDownloadTask::PreviewRequest(LPCTSTR szURL)
{
	Construct( dtaskPreviewRequest );

	m_pRequest.Attach( new CHttpRequest() );
	if ( ! m_pRequest )
		return;		// Out of memory

	m_pRequest->SetURL( szURL );
	m_pRequest->AddHeader( L"Accept", L"image/jpeg" );
	m_pRequest->LimitContentLength( Settings.Search.MaxPreviewLength );

	VERIFY( BeginThread( "Download Task : Preview" ) );		// RunPreviewRequest()
}

void CDownloadTask::MergeFile(CList< CString >* pFiles, BOOL bValidation, const Fragments::List* pGaps)
{
	Construct( dtaskMergeFile );

	m_oMergeFiles.AddTail( pFiles );

	if ( pGaps )
		m_oMergeGaps = *pGaps;
	m_bMergeValidation = bValidation;

	VERIFY( BeginThread( "Download Task : Merge" ) );		// RunMerge()
}

void CDownloadTask::MergeFile(LPCTSTR szPath, BOOL bValidation, const Fragments::List* pGaps)
{
	Construct( dtaskMergeFile );

	m_oMergeFiles.AddTail( (CString)szPath );

	if ( pGaps )
		m_oMergeGaps = *pGaps;
	m_bMergeValidation = bValidation;

	VERIFY( BeginThread( "Download Task : Merge" ) );		// RunMerge()
}


/////////////////////////////////////////////////////////////////////////////
// CDownloadTask aborting

void CDownloadTask::Abort()
{
	if ( m_nTask == dtaskNone )
		return;

	Exit();

	if ( m_pRequest )
		m_pRequest->Cancel();

	CloseThread();

	m_nTask = dtaskNone;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask run

void CDownloadTask::OnRun()
{
//	if ( theApp.m_bIsVistaOrNewer )
//		::SetThreadPriority( GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN );	// Too aggressive

	switch ( m_nTask )
	{
	case dtaskAllocate:
		RunAllocate();
		break;
	case dtaskCopy:
		RunCopy();
		break;
	case dtaskMergeFile:
		RunMerge();
		break;
	case dtaskPreviewRequest:
		RunPreviewRequest();
		break;
	default:
		ASSERT( m_nTask != dtaskNone );
	}

	m_nTask = dtaskNone;
	m_fProgress = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask allocate

void CDownloadTask::RunAllocate()
{
	HANDLE hFile = CreateFile( m_pDownload->m_sPath, GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	VERIFY_FILE_ACCESS( hFile, m_pDownload->m_sPath )
	if ( hFile == INVALID_HANDLE_VALUE ) return;

	if ( GetFileSize( hFile, NULL ) != 0 )
	{
		CloseHandle( hFile );
		return;
	}

	const DWORD nSize = m_pDownload->m_nSize;
	if ( nSize && nSize != SIZE_UNKNOWN )
	{
		if ( Settings.Downloads.SparseThreshold &&
			 nSize >= Settings.Downloads.SparseThreshold * 1024 )
		{
			DWORD dwOut = 0;
			if ( ! DeviceIoControl( hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwOut, NULL ) )
			{
				DWORD nError = GetLastError();
				theApp.Message( MSG_ERROR, L"Unable to set sparse file: \"%s\", Win32 error %x.", m_pDownload->m_sPath, nError );
			}
		}

		DWORD nOffsetLow  = (DWORD)(   ( nSize - 1 ) & 0x00000000FFFFFFFF );
		DWORD nOffsetHigh = (DWORD)( ( ( nSize - 1 ) & 0xFFFFFFFF00000000 ) >> 32 );
		SetFilePointer( hFile, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );

		BYTE bZero = 0;
		DWORD nSizeTemp = 0;
		WriteFile( hFile, &bZero, 1, &nSizeTemp, NULL );
	}

	CloseHandle( hFile );
	m_bSuccess = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask simple copy

void CDownloadTask::RunCopy()
{
	m_nFileError = m_pDownload->MoveFile( m_sDestination, CopyProgressRoutine, this );

	m_bSuccess = ( m_nFileError == ERROR_SUCCESS );

	if ( ! IsThreadEnabled() ) return;	// Aborted

	if ( m_bSuccess )
		m_pDownload->OnMoved();
	else
		m_pDownload->SetFileError( GetFileError(), L"" );
}

DWORD CALLBACK CDownloadTask::CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER /*StreamSize*/,
	LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/,
	DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/,
	LPVOID lpData)
{
	CDownloadTask* pThis = (CDownloadTask*)lpData;

	if ( TotalFileSize.QuadPart )
		pThis->m_fProgress = (float)( TotalBytesTransferred.QuadPart * 100 ) / (float)TotalFileSize.QuadPart;
	//else
	//	pThis->m_fProgress = 100.0f;

	return pThis->IsThreadEnabled() ? PROGRESS_CONTINUE : PROGRESS_CANCEL;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask preview request

void CDownloadTask::RunPreviewRequest()
{
	m_bSuccess = m_pRequest->Execute( false );	// Without threading

	if ( ! IsThreadEnabled() || m_pDownload->m_sPath.IsEmpty() ) return;

	// Save downloaded preview as png-file
	const CString strPath = m_pDownload->m_sPath + L".png";
	CImageFile pImage;
	CBuffer* pBuffer = IsPreviewAnswerValid( m_pDownload->m_oSHA1 );
	if ( pBuffer && pBuffer->m_pBuffer && pBuffer->m_nLength &&
		 pImage.LoadFromMemory( L".jpg", pBuffer->m_pBuffer, pBuffer->m_nLength, FALSE, TRUE ) &&
		 pImage.SaveToFile( (LPCTSTR)strPath, 100 ) )
	{
		// Make it hidden, so the files won't be shared
		SetFileAttributes( (LPCTSTR)strPath, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM );
	}
	else
	{
		// Failed
		m_pDownload->m_bWaitingPreview = FALSE;
		theApp.Message( MSG_ERROR, IDS_SEARCH_DETAILS_PREVIEW_FAILED, (LPCTSTR)GetRequest() );
	}
}

CBuffer* CDownloadTask::IsPreviewAnswerValid(const Hashes::Sha1Hash& oRequestedSHA1) const
{
	if ( m_nTask != dtaskPreviewRequest || ! m_pRequest->IsFinished() )
		return NULL;

	m_pRequest->GetStatusCode();

	if ( m_pRequest->GetStatusSuccess() == FALSE )
	{
		theApp.Message( MSG_DEBUG, L"Preview failed: HTTP status code %i", m_pRequest->GetStatusCode() );
		return NULL;
	}

	const CString strURN = m_pRequest->GetHeader( L"X-Previewed-URN" );
	if ( ! strURN.IsEmpty() )
	{
		Hashes::Sha1Hash oSHA1;
		if ( oSHA1.fromUrn( strURN ) )
		{
			if ( oRequestedSHA1 && validAndUnequal( oSHA1, oRequestedSHA1 ) )
			{
				theApp.Message( MSG_DEBUG, L"Preview failed: Wrong URN." );
				return NULL;
			}
			else if ( ! LibraryMaps.LookupFileBySHA1( oSHA1 ) )
			{
				theApp.Message( MSG_DEBUG, L"Preview failed: Not Found." );
				return NULL;
			}
		}
	}

	const CString strMIME = m_pRequest->GetHeader( L"Content-Type" );
	if ( strMIME.CompareNoCase( L"image/jpeg" ) != 0 )
	{
		theApp.Message( MSG_DEBUG, L"Preview failed: Unacceptable content type." );
		return NULL;
	}

	return m_pRequest->GetResponseBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask merge local file

void CDownloadTask::RunMerge()
{
	m_fProgress = 0;
	const float fIncrement = 100.0f / m_oMergeFiles.GetCount();

	for ( POSITION pos = m_oMergeFiles.GetHeadPosition() ; pos && IsThreadEnabled() ; )
	{
		if ( m_pDownload->IsCompleted() || m_pDownload->IsMoving() ) break;

		m_bSuccess |= m_pDownload->RunMergeFile( m_oMergeFiles.GetNext( pos ), m_bMergeValidation, m_oMergeGaps, this, fIncrement );
	}

	m_fProgress = 100.0f;
}

/////////////////////////////////////////////////////////////////////////////
// Obsolete:

// Note: CDownloadTask::RunMergeFile() moved to DownloadWithTiger

//CString CDownloadTask::SafeFilename(LPCTSTR pszName)
//{
//	static LPCTSTR pszValid = L" `~!@#$%^&()-_=+[]{}';.,";
//	CString strName = pszName;
//
//	for ( int nChar = 0 ; nChar < strName.GetLength() ; nChar++ )
//	{
//		TCHAR cChar = strName.GetAt( nChar );
//		if ( (DWORD)cChar > 128 || IsCharacter( cChar ) || _tcschr( pszValid, cChar ) != NULL )
//			continue;
//
//		strName.SetAt( nChar, '_' );
//	}
//
//	LPCTSTR pszExt = _tcsrchr( strName, '.' );
//	if ( pszExt )
//	{
//		if ( _tcsicmp( pszExt, L".pd" ) == 0 || _tcsicmp( pszExt, L".sd" ) == 0 )
//			strName += L"x";
//	}
//
//	// Maximum filepath length is:
//	// <Windows limit = 256 - 1> - <length of path to download directory> - <length of hash = 39(tiger)> - <space = 1> - <length of ".pd.sav" = 7>
//	const int nMaxFilenameLength = 208 - Settings.Downloads.IncompletePath.GetLength();
//	if ( strName.GetLength() > nMaxFilenameLength )
//	{
//		int nExtLen = pszExt ? static_cast< int >( _tcslen( pszExt ) ) : 0;
//		strName = strName.Left( nMaxFilenameLength - nExtLen ) + strName.Right( nExtLen );
//	}
//
//	return strName;
//}

//BOOL CDownloadTask::CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength)
//{
//	HANDLE hTarget = CreateFile( SafePath( pszTarget ), GENERIC_WRITE,
//		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
//	m_nFileError = GetLastError();
//	VERIFY_FILE_ACCESS( hTarget, pszTarget )
//	if ( hTarget == INVALID_HANDLE_VALUE ) return FALSE;
//
//	BYTE* pBuffer = new BYTE[ BUFFER_SIZE ];
//
//	while ( nLength )
//	{
//		DWORD nBuffer	= (DWORD)min( nLength, BUFFER_SIZE );
//		DWORD nSuccess	= 0;
//		DWORD tStart	= GetTickCount();
//
//		if ( ! ReadFile( hSource, pBuffer, nBuffer, &nBuffer, NULL ) || ! nBuffer ||
//			 ! WriteFile( hTarget, pBuffer, nBuffer, &nSuccess, NULL ) || nSuccess != nBuffer )
//		{
//			m_nFileError = GetLastError();
//			break;
//		}
//
//		nLength -= nBuffer;
//
//		if ( m_pEvent != NULL ) break;
//		tStart = ( GetTickCount() - tStart ) / 2;
//		Sleep( min( tStart, 50ul ) );
//		if ( m_pEvent != NULL ) break;
//	}
//
//	delete [] pBuffer;
//
//	CloseHandle( hTarget );
//	if ( nLength > 0 )
//		DeleteFileEx( pszTarget, TRUE, FALSE, TRUE );
//
//	return ( nLength == 0 );
//}

//void CDownloadTask::CreatePathForFile(const CString& strBase, const CString& strPath)
//{
//	CString strFolder = strBase + L'\\' + strPath;
//	CreateDirectory( strFolder.Left( strFolder.ReverseFind( L'\\' ) ) );
//}
