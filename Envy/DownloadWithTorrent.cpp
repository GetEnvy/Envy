//
// DownloadWithTorrent.cpp
//
// This file is part of Envy (getenvy.com) © 2016
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
#include "DownloadWithTorrent.h"
#include "Download.h"
#include "Downloads.h"
#include "DownloadTask.h"
#include "DownloadSource.h"
#include "DownloadGroups.h"
#include "DownloadTransferBT.h"
#include "UploadTransferBT.h"
#include "UploadTransfer.h"
#include "Uploads.h"
#include "Transfers.h"
#include "Network.h"
#include "HostCache.h"
#include "Buffer.h"
#include "BTPacket.h"
#include "BTClient.h"
#include "BTClients.h"
#include "BTTrackerRequest.h"
#include "Library.h"
#include "LibraryMaps.h"
#include "LibraryFolders.h"
#include "FragmentedFile.h"
#include "SharedFile.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent construction

CDownloadWithTorrent::CDownloadWithTorrent()
	: m_bTorrentRequested	( FALSE )
	, m_bTorrentStarted		( FALSE )
	, m_tTorrentTracker		( 0 )
	, m_nTorrentUploaded	( 0 )
	, m_nTorrentDownloaded	( 0 )
	, m_bTorrentEndgame		( false )
	, m_bTorrentTrackerError ( FALSE )

	, m_nTorrentBlock		( 0 )
	, m_nTorrentSize		( 0 )
	, m_nTorrentSuccess		( 0 )
	, m_bSeeding			( FALSE )

	, m_tTorrentChoke		( 0 )
	, m_tTorrentSources		( 0 )
{
	// Generate random Key value
	for ( int nChar = 1 ; nChar < 6 ; nChar++ )
	{
		m_sKey += GenerateCharacter();
	}
}

CDownloadWithTorrent::~CDownloadWithTorrent()
{
	if ( m_bTorrentRequested )
		SendStopped();

	m_pPeerID.clear();

	CloseTorrentUploads();
}

bool CDownloadWithTorrent::IsSeeding() const
{
	return m_bSeeding != 0;
}

bool CDownloadWithTorrent::IsTorrent() const
{
	return m_pTorrent.IsAvailable();
}

bool CDownloadWithTorrent::IsSingleFileTorrent() const
{
	return m_pTorrent.IsAvailable() && m_pTorrent.GetCount() == 1;
}

bool CDownloadWithTorrent::IsMultiFileTorrent() const
{
	return m_pTorrent.IsAvailable() && m_pTorrent.GetCount() > 1;
}

// Obsolete:
//void CDownloadWithTorrent::AddRequest(CBTTrackerRequest* pRequest)
//{
//	CQuickLock oLock( m_pRequestsSection );
//
//	if ( m_pRequests.Find( pRequest ) == NULL )
//		m_pRequests.AddTail( pRequest );
//}
//
//void CDownloadWithTorrent::RemoveRequest(CBTTrackerRequest* pRequest)
//{
//	CQuickLock oLock( m_pRequestsSection );
//
//	if ( POSITION pos = m_pRequests.Find( pRequest ) )
//	{
//		m_pRequests.RemoveAt( pos );
//	}
//}
//
//void CDownloadWithTorrent::CancelRequest(CBTTrackerRequest* pRequest)
//{
//	CQuickLock oLock( m_pRequestsSection );
//
//	if ( POSITION pos = m_pRequests.Find( pRequest ) )
//	{
//		pRequest->Cancel();
//	}
//}

TCHAR CDownloadWithTorrent::GenerateCharacter() const
{
	switch ( GetRandomNum( 0, 2 ) )
	{
	case 0:
		return static_cast< TCHAR >( 'a' + ( GetRandomNum( 0, 25 ) ) );
	case 1:
		return static_cast< TCHAR >( 'A' + ( GetRandomNum( 0, 25 ) ) );
	default:
		return static_cast< TCHAR >( '0' + ( GetRandomNum( 0, 9 ) ) );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent serialize

void CDownloadWithTorrent::Serialize(CArchive& ar, int nVersion)
{
	CDownloadWithFile::Serialize( ar, nVersion );

	if ( nVersion < 40 )	// Old Shareaza style conversion disabled
		return;

	m_pTorrent.Serialize( ar );

	if ( ! IsTorrent() )
		return;

	if ( ar.IsStoring() )
	{
		ar << m_nTorrentSuccess;
		if ( m_pTorrentBlock )	//  || nVersion < 1020
			ar.Write( m_pTorrentBlock, sizeof( BYTE ) * m_nTorrentBlock );
		ar << BOOL( m_bSeeding && Settings.BitTorrent.AutoSeed );
	}
	else // Loading
	{
		m_nTorrentSize	= m_pTorrent.m_nBlockSize;
		m_nTorrentBlock	= m_pTorrent.m_nBlockCount;

		ar >> m_nTorrentSuccess;
		m_pTorrentBlock.Free();
		if ( m_nTorrentBlock )
		{
			m_pTorrentBlock.Attach( new BYTE[ m_nTorrentBlock ] );
			memset( m_pTorrentBlock, TRI_UNKNOWN, m_nTorrentBlock );
			ReadArchive( ar, m_pTorrentBlock, sizeof( BYTE ) * m_nTorrentBlock );
		}

		ar >> m_bSeeding;

		//if ( nVersion < 41 )
		//{
		//	CString strServingFileName;
		//	ar >> strServingFileName;
		//	GetFile()->Delete();
		//}

		GenerateTorrentDownloadID();

		if ( ! m_oTiger && m_pTorrent.m_oTiger )
		{
			m_oTiger = m_pTorrent.m_oTiger;
			m_bTigerTrusted = true;
		}
		if ( ! m_oSHA1 && m_pTorrent.m_oSHA1 )
		{
			m_oSHA1 = m_pTorrent.m_oSHA1;
			m_bSHA1Trusted = true;
		}
		if ( ! m_oED2K && m_pTorrent.m_oED2K )
		{
			m_oED2K = m_pTorrent.m_oED2K;
			m_bED2KTrusted = true;
		}
		if ( ! m_oMD5 && m_pTorrent.m_oMD5 )
		{
			m_oMD5 = m_pTorrent.m_oMD5;
			m_bMD5Trusted = true;
		}
		if ( ! m_oBTH && m_pTorrent.m_oBTH )
		{
			m_oBTH = m_pTorrent.m_oBTH;
			m_bBTHTrusted = true;
		}

		// Convert old Shareaza multifile torrents (Shareaza < 2.4.0.4)
		//if ( nVersion < 40 )
		//{
		//	// Get old file name
		//	ASSERT( GetFileCount() == 1 );
		//	CString strPath = GetPath( 0 );
		//	CString strServingFileName = strPath;
		//
		//	CProgressBarDlg oProgress( AfxGetMainWnd() );
		//	oProgress.SetWindowText( LoadString( IDS_BT_UPDATE_TITLE ) );
		//	oProgress.SetActionText( LoadString( IDS_BT_UPDATE_CONVERTING ) );
		//	oProgress.SetEventText( m_sName );
		//	oProgress.SetEventRange( 0, (int)( m_pTorrent.m_nSize / 1024ull ) );
		//	oProgress.SetSubEventText( strServingFileName );
		//	oProgress.SetSubEventRange( 0, (int)( m_pTorrent.m_nSize / 1024ull ) );
		//	oProgress.CenterWindow();
		//	oProgress.ShowWindow( SW_SHOW );
		//	oProgress.UpdateWindow();
		//	oProgress.UpdateData( FALSE );
		//
		//	// Create a bunch of new empty files
		//	ClearFile();	// Close old files
		//	auto_ptr< CFragmentedFile > pFragFile = GetFile();
		//	if ( ! pFragFile.get() )
		//		AfxThrowMemoryException();
		//	if ( ! pFragFile->Open( m_pTorrent, ! IsSeeding() ) )
		//		AfxThrowFileException( CFileException::genericException );
		//
		//	if ( ! IsSeeding() )
		//	{
		//		// Check for free space, then open file
		//		if ( ! Downloads.IsSpaceAvailable( m_pTorrent.m_nSize, Downloads.dlPathIncomplete ) )
		//			AfxThrowFileException( CFileException::diskFull );
		//		CFile oSource( strServingFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan );
		//
		//		// Copy data from old file to new files
		//		const QWORD BUFFER_SIZE = 4ul * 1024ul * 1024ul;	// 4 MB
		//		auto_array< BYTE > pBuffer( new BYTE[ BUFFER_SIZE ] );
		//		if ( ! pBuffer.get() ) AfxThrowMemoryException();
		//		// Optimize this by reading only available data
		//		QWORD nTotal = 0ull;
		//		for ( QWORD nLength = m_pTorrent.m_nSize ; nLength ; )
		//		{
		//			DWORD nBuffer = (DWORD)min( nLength, BUFFER_SIZE );
		//			DWORD nRead = oSource.Read( pBuffer.get(), nBuffer );
		//			if ( nRead )
		//			{
		//				if ( ! pFragFile->Write( nTotal, pBuffer.get(), nRead ) )
		//					AfxThrowFileException( CFileException::genericException );
		//			}
		//			if ( nRead != nBuffer )
		//				break;	// EOF
		//			nLength -= nBuffer;
		//			nTotal += nBuffer;
		//
		//			CString strText;
		//			strText.Format( L"%s %s %s",
		//				(LPCTSTR)Settings.SmartVolume( nTotal, KiloBytes ),
		//				(LPCTSTR)LoadString( IDS_GENERAL_OF ),
		//				(LPCTSTR)Settings.SmartVolume( m_pTorrent.m_nSize, KiloBytes ) );
		//			oProgress.SetSubActionText( strText );
		//			oProgress.StepSubEvent( (int)( nBuffer / 1024ul ) );
		//			oProgress.SetEventPos( (int)( nTotal / 1024ull ) );
		//			oProgress.UpdateWindow();
		//			AfxGetMainWnd()->UpdateWindow();
		//			Sleep( 50 );
		//		}
		//	}
		//
		//	// Delete old multifile
		//	DeleteFileEx( strServingFileName, FALSE, FALSE, TRUE );
		//	if ( strServingFileName != strPath )
		//		DeleteFileEx( strPath, FALSE, FALSE, TRUE );
		//}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent submit data

BOOL CDownloadWithTorrent::SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength)
{
	if ( IsTorrent() )
	{
		CSingleLock oLock( &Transfers.m_pSection );
		if ( oLock.Lock( 250 ) )
		{
			for ( CDownloadTransfer* pTransfer = GetFirstTransfer() ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
			{
				if ( pTransfer->m_nProtocol == PROTOCOL_BT )
					pTransfer->UnrequestRange( nOffset, nLength );
			}
		}
	}

	return CDownloadWithFile::SubmitData( nOffset, pData, nLength );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent set torrent

BOOL CDownloadWithTorrent::SetTorrent(const CBTInfo* pTorrent /*NULL*/)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( IsMoving() || IsCompleted() )
		return FALSE;

	if ( pTorrent )
	{
		if ( ! pTorrent->IsAvailable() )
			return FALSE;

		m_pTorrent = *pTorrent;
	}

	// Check conflicts first

	if ( m_nSize != SIZE_UNKNOWN &&			// Single file download
		 m_pTorrent.IsAvailableInfo() &&
		 m_pTorrent.m_nSize != m_nSize )
		return FALSE;

	if ( m_bBTHTrusted && m_oBTH && m_pTorrent.m_oBTH && m_oBTH != m_pTorrent.m_oBTH )
		return FALSE;

	if ( m_bTigerTrusted && m_oTiger && m_pTorrent.m_oTiger && m_oTiger != m_pTorrent.m_oTiger )
		return FALSE;

	if ( m_bSHA1Trusted && m_oSHA1 && m_pTorrent.m_oSHA1 && m_oSHA1 != m_pTorrent.m_oSHA1 )
		return FALSE;

	if ( m_bED2KTrusted && m_oED2K && m_pTorrent.m_oED2K && m_oED2K != m_pTorrent.m_oED2K )
		return FALSE;

	if ( m_bMD5Trusted && m_oMD5 && m_pTorrent.m_oMD5 && m_oMD5 != m_pTorrent.m_oMD5)
		return FALSE;

	// Update

	if ( ! m_pTorrent.m_sName.IsEmpty() )
		Rename( m_pTorrent.m_sName );

	if ( m_pTorrent.m_nSize != SIZE_UNKNOWN )
		m_nSize = m_pTorrent.m_nSize;

	if ( m_pTorrent.m_nBlockSize )
		m_nTorrentSize = m_pTorrent.m_nBlockSize;

	if ( m_pTorrent.m_nBlockCount )
	{
		m_nTorrentBlock = m_pTorrent.m_nBlockCount;

		m_pTorrentBlock.Free();
		m_pTorrentBlock.Attach( new BYTE[ m_nTorrentBlock ] );
		memset( m_pTorrentBlock, TRI_UNKNOWN, m_nTorrentBlock );
	}

	if ( m_pTorrent.m_oTiger )
	{
		m_oTiger = m_pTorrent.m_oTiger;
		m_bTigerTrusted = true;
	}

	if ( m_pTorrent.m_oSHA1 )
	{
		m_oSHA1 = m_pTorrent.m_oSHA1;
		m_bSHA1Trusted = true;
	}

	if ( m_pTorrent.m_oED2K )
	{
		m_oED2K = m_pTorrent.m_oED2K;
		m_bED2KTrusted = true;
	}

	if ( m_pTorrent.m_oMD5 )
	{
		m_oMD5 = m_pTorrent.m_oMD5;
		m_bMD5Trusted = true;
	}

	if ( m_pTorrent.m_oBTH )
	{
		m_oBTH = m_pTorrent.m_oBTH;
		m_bBTHTrusted = true;
	}

	m_nTorrentSuccess = 0;

	if ( CreateDirectory( Settings.Downloads.TorrentPath ) )
	{
		LibraryFolders.AddFolder( Settings.Downloads.TorrentPath, FALSE );
		m_pTorrent.SaveTorrentFile( Settings.Downloads.TorrentPath );
	}

	// Add sources from torrents - DWK
	for ( POSITION pos = m_pTorrent.m_sURLs.GetHeadPosition() ; pos ; )
	{
		AddSourceURLs( m_pTorrent.m_sURLs.GetNext( pos ) );
	}

	// Add DHT nodes to host cache
	for ( POSITION pos = m_pTorrent.m_oNodes.GetHeadPosition() ; pos ; )
	{
		HostCache.BitTorrent.Add( m_pTorrent.m_oNodes.GetNext( pos ) );
	}

	SetModified();

	// Re-link Download Group
	DownloadGroups.Link( static_cast< CDownload* >( this ) );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent run

void CDownloadWithTorrent::RunTorrent(DWORD tNow)
{
	if ( ! IsTorrent() || ! Settings.BitTorrent.Enabled || ! Network.IsConnected() )
		return;

	// Return if disk is full
	if ( GetFileError() != ERROR_SUCCESS )
		return;

	// Choke torrents every 10 seconds
	if ( tNow > m_tTorrentChoke && tNow - m_tTorrentChoke >= 10000ul )
		ChokeTorrent( tNow );

	// Check if the torrent file exists and has been opened
	if ( ! OpenFile() )
		return;

	// Return if this download is waiting for a download task to finish
	if ( IsTasking() )
		return;

	// Generate a peerid if there isn't one
	if ( ! m_pPeerID )
		GenerateTorrentDownloadID();

	// Store some values for later
	DWORD nSourcesCount = 0ul;
	DWORD nSourcesMax = 0ul;
	DWORD nSourcesWanted = 0ul;

	// Check if a tracker has already been locked onto
	if ( ! m_bTorrentStarted )
	{
		// Check if download is active, isn't already waiting for a request reply, and is allowed to try and contact this tracker
		if ( ! m_bTorrentRequested && ! IsPaused() && ( IsTrying() || IsSeeding() ) && tNow > m_tTorrentTracker )
		{
			// Get the # of sources that can be connected to
			nSourcesCount = GetBTSourceCount( TRUE );

			// Calculate how many new sources are wanted, expect a high failure rate
			nSourcesMax = Settings.BitTorrent.DownloadConnections * 4ul;
			if ( nSourcesCount < nSourcesMax )
				nSourcesWanted = nSourcesMax - nSourcesCount;

			// Initial announce to tracker
			SendStarted( nSourcesWanted );

			// Deselect default unwanted files
			if ( ( Settings.BitTorrent.SkipPaddingFiles || Settings.BitTorrent.SkipTrackerFiles ) && ! IsSeeding() )
			{
				CDownload* pDownload = Downloads.FindByBTH( m_oBTH );

				if ( pDownload && pDownload->GetVolumeRemaining() == m_nSize )	// First load only
				{
					auto_ptr< CFragmentedFile > pFragFile( pDownload->GetFile() );
					if ( pFragFile.get() )
					{
						const DWORD nCount = pFragFile->GetCount();
						if ( nCount > 1 )
						{
							CString strName;
							for ( DWORD i = 0 ; i < nCount ; i++ )
							{
								strName = pFragFile->GetName( i );
								strName = strName.Mid( strName.ReverseFind( L'\\' ) + 1 );

								if ( strName[0] == L'_' && Settings.BitTorrent.SkipPaddingFiles &&
										  StartsWith( strName, L"_____padding_file_", 18 ) )
									pFragFile->SetPriority( i, CFragmentedFile::prUnwanted );
								else if ( Settings.BitTorrent.SkipTrackerFiles && strName.Right( 4 ) == L".txt" &&
										( StartsWith( strName, L"Torrent downloaded from ", 24 ) || StartsWith( strName, L"Torrent_downloaded_from_", 24 ) ) )
									pFragFile->SetPriority( i, CFragmentedFile::prUnwanted );
							}
						}
					}
				}
			}
		}

		return;
	}

	// Store if this is a regular update or not
	bool bRegularUpdate = tNow > m_tTorrentTracker;

	// Check if an update needs to be sent to the tracker.
	// This can be a regular update, or a request for more sources if the number of known sources is getting too low.
	if ( bRegularUpdate || tNow - m_tTorrentSources > Settings.BitTorrent.DefaultTrackerPeriod )
	{
		// Check if the torrent is seeding
		if ( IsSeeding() )
		{
			// Use the upload count values
			nSourcesCount = Uploads.GetTorrentUploadCount();
			nSourcesMax = Settings.BitTorrent.UploadCount;
		}
		else
		{
			// Use the download count values
			nSourcesCount = GetBTSourceCount();
			nSourcesMax = Settings.BitTorrent.DownloadConnections;
		}

		// Request more sources, more often, for regular updates
		if ( bRegularUpdate )
			nSourcesMax *= 4ul;

		// Calculate the # of sources needed
		if ( nSourcesCount < nSourcesMax )
			nSourcesWanted = nSourcesMax - nSourcesCount;

		// Check if an update needs to be sent
		if ( bRegularUpdate || nSourcesWanted )
			SendUpdate( nSourcesWanted );	// Send tracker update
		else
			m_tTorrentSources = tNow;		// Record time that source counts checked even if no update sent
	}

	//return;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent Create Peer ID

// The 'Peer ID' is different for each download and is not retained between sessions.
// -EN0010-############

BOOL CDownloadWithTorrent::GenerateTorrentDownloadID()
{
	theApp.Message( MSG_DEBUG, L"Creating BitTorrent Peer ID" );

	// Check ID is not in use
	ASSERT( ! m_pPeerID );
	if ( m_pPeerID )
	{
		theApp.Message( MSG_ERROR, L"Attempted to re-create an in-use Peer ID" );
		return FALSE;
	}

	// Client ID
	// Azureus style "-SSVVVV-" http://bittorrent.org/beps/bep_0020.html
	m_pPeerID[ 0 ] = '-';
	m_pPeerID[ 1 ] = BT_ID1;
	m_pPeerID[ 2 ] = BT_ID2;
	m_pPeerID[ 3 ] = theApp.m_szVersion[0];		// 0	static_cast< BYTE >( '0' + theApp.m_nVersion[0] )
	m_pPeerID[ 4 ] = theApp.m_szVersion[1];		// X
	m_pPeerID[ 5 ] = theApp.m_szVersion[2];		// X
	m_pPeerID[ 6 ] = theApp.m_szVersion[3];		// 0
	m_pPeerID[ 7 ] = '-';

	// Random characters for the rest of the Client ID
	for ( int nByte = 8 ; nByte < 20 ; nByte++ )
	{
		m_pPeerID[ nByte ] = GetRandomNum( 0ui8, _UI8_MAX );
	}
	m_pPeerID.validate();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Send BT Tracker Request actions

void CDownloadWithTorrent::SendStarted(DWORD nNumWant)
{
	if ( ! Network.IsConnected() || ! Settings.BitTorrent.Enabled )
		return;

	// Record that the start request has been sent
	m_bTorrentRequested = TRUE;
	m_tTorrentTracker = m_tTorrentSources = GetTickCount();
	m_tTorrentTracker += Settings.BitTorrent.DefaultTrackerPeriod;
	m_nTorrentDownloaded = m_nTorrentUploaded = 0ull;

	DHT.Search( m_oBTH );

	// Return if there is no tracker
	if ( ! m_pTorrent.HasTracker() )
		return;

	// Create and run tracker request
	TrackerRequests.Request( static_cast< CDownload* >( this ), BTE_TRACKER_STARTED, nNumWant, this );	// "started"
}

void CDownloadWithTorrent::SendUpdate(DWORD nNumWant)
{
	if ( ! Network.IsConnected() || ! Settings.BitTorrent.Enabled )
		return;

	// Record that an update has been sent
	m_tTorrentTracker = m_tTorrentSources = GetTickCount();
	m_tTorrentTracker += Settings.BitTorrent.DefaultTrackerPeriod;

	DHT.Search( m_oBTH );

	// Return if there is no tracker
	if ( ! m_pTorrent.HasTracker() )
		return;

	// Create and run tracker request
	TrackerRequests.Request( static_cast< CDownload* >( this ), BTE_TRACKER_UPDATE, nNumWant, this );	// "NULL"
}

void CDownloadWithTorrent::SendCompleted()
{
	if ( ! Network.IsConnected() || ! Settings.BitTorrent.Enabled )
		return;

	// Return if there is no tracker
	if ( ! m_pTorrent.HasTracker() )
		return;

	// Create and run tracker request
	TrackerRequests.Request( static_cast< CDownload* >( this ), BTE_TRACKER_COMPLETED, 0, this );		// "completed"
}

void CDownloadWithTorrent::SendStopped()
{
	if ( ! Settings.BitTorrent.Enabled || ! Network.IsConnected() )
		return;

	if ( ! m_pTorrent.HasTracker() )
		return;		// There is no tracker

	// Log the 'stop' event
	theApp.Message( MSG_DEBUG, L"[BT] Sending final tracker announce for %s", m_pTorrent.m_sName );

	// Update download to indicate it has been stopped
	m_bTorrentStarted = m_bTorrentRequested = FALSE;

	// Create and run tracker request
	TrackerRequests.Request( static_cast< CDownload* >( this ), BTE_TRACKER_STOPPED );					// "stopped"
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent tracker event handler

void CDownloadWithTorrent::OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip, CBTTrackerRequest* pEvent)
{
	ASSUME_LOCK( Transfers.m_pSection );

	const DWORD tNow = GetTickCount();

	if ( bSuccess )
	{
		// Success: Reset error conditions
		if ( pEvent->GetEvent() == BTE_TRACKER_STARTED ||
			 pEvent->GetEvent() == BTE_TRACKER_UPDATE ||
			 pEvent->GetEvent() == BTE_TRACKER_COMPLETED )
		{
			// Abort if the download has been paused after the request was sent but before a reply was received
			if ( ! m_bTorrentRequested )
				return;

			m_bTorrentStarted = TRUE;
			m_tTorrentTracker = tNow + pEvent->GetInterval() * 1000;
		}
		m_bTorrentTrackerError = FALSE;
		m_sTorrentTrackerError.Empty();
		m_pTorrent.SetTrackerSucceeded( tNow );

		// Get new sources
		//int nMax = Settings.Downloads.SourcesWanted;
		for ( POSITION pos = pEvent->GetSources() ; pos ; )
		{
			const CBTTrackerSource& pSource = pEvent->GetNextSource( pos );
			AddSourceBT( pSource.m_pPeerID, &pSource.m_pAddress.sin_addr, ntohs( pSource.m_pAddress.sin_port ) );
			//if ( nMax-- < 0 && GetEffectiveSourceCount() >= Settings.Downloads.SourcesWanted )
			//	break;
		}

		// Lock on this tracker if we were searching for one
		if ( m_pTorrent.GetTrackerMode() == CBTInfo::tMultiFinding )
		{
			theApp.Message( MSG_DEBUG, L"[BT] Locked onto tracker %s", m_pTorrent.GetTrackerAddress() );
			m_pTorrent.SetTrackerMode( CBTInfo::tMultiFound );
		}
	}
	else
	{
		// There was a problem with the tracker
		m_bTorrentTrackerError = TRUE;
		m_sTorrentTrackerError = ( pszTip ? pszTip : pszReason );
		m_pTorrent.OnTrackerFailure();
		m_bTorrentRequested = m_bTorrentStarted = FALSE;
		m_tTorrentTracker = tNow + GetRetryTime();
		m_pTorrent.SetTrackerRetry( m_tTorrentTracker );

		theApp.Message( MSG_INFO, L"%s", pszReason );

		if ( m_pTorrent.IsMultiTracker() )
		{
			// Try the next one
			m_pTorrent.SetTrackerNext( tNow );

			// Set retry time
			m_tTorrentTracker = m_pTorrent.GetTrackerNextTry();

			// Load the error message string
			CString strFormat, strErrorMessage;
			LoadString( strFormat, IDS_BT_TRACKER_MULTI );
			strErrorMessage.Format( strFormat, m_pTorrent.GetTrackerIndex() + 1, m_pTorrent.GetTrackerCount() );
			m_sTorrentTrackerError = m_sTorrentTrackerError + L" | " + strErrorMessage;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent tracker retry calculation

DWORD CDownloadWithTorrent::GetRetryTime() const
{
	// 0..2 - 1 min, 3..5 - 4 min, 6..8 - 9 min, ..., > 20 - 1 hour
	DWORD tRetryTime = m_pTorrent.GetTrackerFailures() / 3ul + 1ul;
	tRetryTime *= tRetryTime * 60ul * 1000ul;
	if ( tRetryTime > 60ul * 60ul * 1000ul )
		tRetryTime = 60ul * 60ul * 1000ul;
	return tRetryTime;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent download transfer linking

CDownloadTransferBT* CDownloadWithTorrent::CreateTorrentTransfer(CBTClient* pClient)
{
	if ( IsMoving() || IsPaused() ) return NULL;

	CDownloadSource* pSource = NULL;

	Hashes::Guid tmp = transformGuid( pClient->m_oGUID );
	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		pSource = GetNext( posSource );

		if ( pSource->m_nProtocol == PROTOCOL_BT &&
			validAndEqual( pSource->m_oGUID, tmp ) ) break;

		pSource = NULL;
	}

	if ( pSource == NULL )
	{
		pSource = new CDownloadSource( static_cast< CDownload* >( this ),
			pClient->m_oGUID, &pClient->m_pHost.sin_addr, htons( pClient->m_pHost.sin_port ) );
		pSource->m_bPushOnly = ! pClient->m_bInitiated;

		if ( ! AddSourceInternal( pSource ) )
			return NULL;
	}

	if ( ! pSource->IsIdle() )
		return NULL;	// A download transfer already exists

	return (CDownloadTransferBT*)pSource->CreateTransfer( pClient );
}

void CDownloadWithTorrent::OnFinishedTorrentBlock(DWORD nBlock)
{
	for ( CDownloadTransferBT* pTransfer = (CDownloadTransferBT*)GetFirstTransfer() ; pTransfer ; pTransfer = (CDownloadTransferBT*)pTransfer->m_pDlNext )
	{
		if ( pTransfer->m_nProtocol == PROTOCOL_BT )
			pTransfer->SendFinishedBlock( nBlock );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent create bitfield

CBTPacket* CDownloadWithTorrent::CreateBitfieldPacket()
{
	if ( ! m_pTorrentBlock )
		return NULL;

	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_BITFIELD );
	int nCount = 0;

	for ( QWORD nBlock = 0 ; nBlock < m_nTorrentBlock ; )
	{
		BYTE nByte = 0;

		for ( int nBit = 7 ; nBit >= 0 && nBlock < m_nTorrentBlock ; nBit--, nBlock++ )
		{
			if ( m_pTorrentBlock[ nBlock ] == TRI_TRUE )
			{
				nByte |= ( 1 << nBit );
				nCount++;
			}
		}

		pPacket->WriteByte( nByte );
	}

	if ( nCount > 0 ) return pPacket;
	pPacket->Release();

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent upload linking

void CDownloadWithTorrent::AddUpload(CUploadTransferBT* pUpload)
{
	if ( m_pTorrentUploads.Find( pUpload ) == NULL )
		m_pTorrentUploads.AddTail( pUpload );
}

void CDownloadWithTorrent::RemoveUpload(CUploadTransferBT* pUpload)
{
	if ( POSITION pos = m_pTorrentUploads.Find( pUpload ) )
		m_pTorrentUploads.RemoveAt( pos );
}

void CDownloadWithTorrent::CloseTorrentUploads()
{
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pUpload = m_pTorrentUploads.GetNext( pos );
		pUpload->Close();
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent choking

void CDownloadWithTorrent::ChokeTorrent(DWORD tNow)
{
	BOOL bChooseRandom = TRUE;
	int nTotalRandom = 0;
	CList< void* > pSelected;

	if ( ! tNow ) tNow = GetTickCount();
	if ( tNow > m_tTorrentChoke && tNow - m_tTorrentChoke < 2000 ) return;
	m_tTorrentChoke = tNow;

	// Check if a seeding torrent needs to start some new connections
	if ( IsSeeding() )
	{
		// We might need to 'push' a connection if we don't have enough upload connections
		if ( (DWORD)m_pTorrentUploads.GetCount() < Settings.BitTorrent.UploadCount * 2 &&
			 (DWORD)m_pTorrentUploads.GetCount() != GetBTSourceCount() &&
			CanStartTransfers( tNow ) )
		{
			theApp.Message( MSG_DEBUG, L"Attempting to push-start a BitTorrent upload for %s", m_pTorrent.m_sName );
			StartNewTransfer( tNow );
		}
	}

	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );
		if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;

		if ( pTransfer->m_nRandomUnchoke == 2 )
		{
			if ( tNow >= pTransfer->m_tRandomUnchoke + Settings.BitTorrent.RandomPeriod )
				pTransfer->m_nRandomUnchoke = 1;
			else
				bChooseRandom = FALSE;
		}

		if ( pTransfer->m_bInterested )
			nTotalRandom += ( pTransfer->m_nRandomUnchoke == 0 ) ? 3 : 1;
	}

	if ( bChooseRandom && nTotalRandom > 0 )
	{
		nTotalRandom = GetRandomNum( 0, nTotalRandom - 1 );

		for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
		{
			CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );
			if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
			if ( pTransfer->m_bInterested == FALSE ) continue;

			const int nWeight = ( pTransfer->m_nRandomUnchoke == 0 ) ? 3 : 1;

			if ( nTotalRandom < nWeight )
			{
				pTransfer->m_nRandomUnchoke = 2;
				pTransfer->m_tRandomUnchoke = tNow;
				pSelected.AddTail( pTransfer );
				break;
			}
			else
			{
				nTotalRandom -= nWeight;
			}
		}
	}

	while ( (DWORD)pSelected.GetCount() < Settings.BitTorrent.UploadCount )
	{
		CUploadTransferBT* pBest = NULL;
		DWORD nBest = 0;

		for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
		{
			CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );

			if ( pTransfer->m_nProtocol == PROTOCOL_BT &&
				 pTransfer->m_bInterested &&
				 pSelected.Find( pTransfer->m_pClient ) == NULL &&
				 pTransfer->GetAverageSpeed() >= nBest )
			{
				pBest = pTransfer;
				nBest = pTransfer->GetAverageSpeed();
			}
		}

		if ( pBest == NULL ) break;
		pSelected.AddTail( pBest->m_pClient );
	}

	while ( (DWORD)pSelected.GetCount() < Settings.BitTorrent.UploadCount )
	{
		CDownloadTransferBT* pBest = NULL;
		DWORD nBest = 0;

		for ( CDownloadTransferBT* pTransfer = (CDownloadTransferBT*)GetFirstTransfer() ;
				pTransfer ; pTransfer = (CDownloadTransferBT*)pTransfer->m_pDlNext )
		{
			if ( pTransfer->m_nProtocol == PROTOCOL_BT &&
				 pSelected.Find( pTransfer->m_pClient ) == NULL &&
				 pTransfer->m_nState == dtsDownloading &&
				 pTransfer->m_pClient->m_pUploadTransfer->m_bInterested &&
				 pTransfer->GetAverageSpeed() >= nBest )
			{
				pBest = pTransfer;
				nBest = pTransfer->GetAverageSpeed();
			}
		}

		if ( pBest == NULL ) break;
		pSelected.AddTail( pBest->m_pClient );
	}

	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );
		if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;

		pTransfer->SetChoke( pTransfer->m_bInterested == TRUE &&
			pSelected.Find( pTransfer->m_pClient ) == NULL );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent search -> tracker link

BOOL CDownloadWithTorrent::FindMoreSources()
{
	if ( m_bTorrentRequested )
	{
		ASSERT( IsTorrent() );

		if ( GetTickCount() > m_tTorrentSources + 15000 )
		{
			SendUpdate( min( Settings.BitTorrent.DownloadConnections * 4ul, 100ul ) );
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent Close

void CDownloadWithTorrent::CloseTorrent()
{
	if ( m_bTorrentRequested )
		SendStopped();

	CloseTorrentUploads();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent stats

float CDownloadWithTorrent::GetRatio() const
{
	return float( m_pTorrent.m_nTotalUpload * 10000 /
		( m_pTorrent.m_nTotalDownload ? m_pTorrent.m_nTotalDownload :
		m_pTorrent.m_nSize ) ) / 100.0f;	// ToDo: Fix m_nSize
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent Check if it's okay to start a new download transfer

BOOL CDownloadWithTorrent::CheckTorrentRatio() const
{
	if ( ! IsTorrent() ) return TRUE;

	if ( m_pTorrent.m_nStartDownloads == CBTInfo::dtAlways ) return TRUE;	// Torrent is set to download as needed

	if ( m_pTorrent.m_nStartDownloads == CBTInfo::dtWhenRatio )				// Torrent is set to download only when ratio is okay
	{
		if ( m_nTorrentUploaded > m_nTorrentDownloaded ) return TRUE;		// Ratio OK
		if ( GetVolumeComplete() < 5 * 1024 * 1024 ) return TRUE;			// Always get at least 5 MB so you have something to upload
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent check if upload exists

BOOL CDownloadWithTorrent::UploadExists(in_addr* pIP) const
{
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );

		if ( pTransfer->m_nProtocol == PROTOCOL_BT &&
			 pTransfer->m_nState != upsNull &&
			 pTransfer->m_pHost.sin_addr.S_un.S_addr == pIP->S_un.S_addr )
			return TRUE;
	}
	return FALSE;
}

BOOL CDownloadWithTorrent::UploadExists(const Hashes::BtGuid& oGUID) const
{
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );

		if ( pTransfer->m_nProtocol == PROTOCOL_BT &&
			 pTransfer->m_nState != upsNull &&
			 validAndEqual( oGUID, pTransfer->m_pClient->m_oGUID ) )
			return TRUE;
	}
	return FALSE;
}
