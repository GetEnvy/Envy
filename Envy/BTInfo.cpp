//
// BTInfo.cpp
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
#include "BTInfo.h"
#include "BENode.h"
#include "Buffer.h"
#include "Download.h"
#include "Downloads.h"
#include "DownloadTask.h"
#include "FragmentedFile.h"
#include "Transfers.h"
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "DlgProgressBar.h"
#include "DownloadWithTorrent.h"	// For scrape m_pPeerId
#include "HttpRequest.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Check if a string is a valid path/file name.
static bool IsValid(const CString& str)
{
	return ! str.IsEmpty() && ( str.Find( L'?' ) == -1 ) && ( str != L"#ERROR#" );
}

//////////////////////////////////////////////////////////////////////
// CBTInfo construction

CBTInfo::CBTInfo()
	: m_nBlockSize		( 0ul )
	, m_nBlockCount		( 0ul )
	, m_pBlockBTH		( NULL )
	, m_nTotalUpload	( 0ull )
	, m_nTotalDownload	( 0ull )
	, m_nTrackerSeeds	( 0 )
	, m_nTrackerPeers	( 0 )
	, m_nTrackerWait	( 90 * 1000 )
	, m_nTrackerIndex	( -1 )
	, m_nTrackerMode	( tNull )
	, m_tTrackerScrape	( 0ul )
	, m_tCreationDate	( 0ul )
	, m_nTestByte		( 0ul )
	, m_nInfoSize		( 0ul )
	, m_nInfoStart		( 0ul )
	, m_bPrivate		( FALSE )
	, m_nStartDownloads	( dtAlways )
	, m_bEncodingError	( false )
	, m_nEncoding		( Settings.BitTorrent.TorrentCodePage )
{
	CBENode::m_nDefaultCP = Settings.BitTorrent.TorrentCodePage;
}

CBTInfo::CBTInfo(const CBTInfo& oSource)
	: m_nBlockSize		( 0ul )
	, m_nBlockCount		( 0ul )
	, m_pBlockBTH		( NULL )
	, m_nTotalUpload	( 0ull )
	, m_nTotalDownload	( 0ull )
	, m_nTrackerIndex	( -1 )
	, m_nTrackerMode	( tNull )
	, m_nTestByte		( 0ul )
	, m_nInfoSize		( 0ul )
	, m_nInfoStart		( 0ul )
	, m_tCreationDate	( 0ul )
	, m_bPrivate		( FALSE )
	, m_nStartDownloads	( dtAlways )
	, m_bEncodingError	( false )
	, m_nEncoding		( Settings.BitTorrent.TorrentCodePage )
{
	*this = oSource;

	CBENode::m_nDefaultCP = Settings.BitTorrent.TorrentCodePage;
}

CBTInfo::~CBTInfo()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CBTFile construction

CBTInfo::CBTFile::CBTFile(const CBTInfo* pInfo, const CBTFile* pBTFile)
	: m_pInfo		( pInfo )
	, m_nOffset		( pBTFile ? pBTFile->m_nOffset : 0 )
{
	if ( pBTFile )
		CEnvyFile::operator=( *pBTFile );
}

CString CBTInfo::CBTFile::FindFile() const
{
	CQuickLock oLock( Library.m_pSection );

	// Try complete folder
	CString strFile = Settings.Downloads.CompletePath + L"\\" + m_sPath;
	if ( GetFileSize( SafePath( strFile ) ) == m_nSize )
		return strFile;

	// Try folder of original .torrent
	CString strTorrentPath = m_pInfo->m_sPath.Left( m_pInfo->m_sPath.ReverseFind( L'\\' ) + 1 );
	strFile = SafePath( strTorrentPath + m_sPath );
	if ( GetFileSize( SafePath( strFile ) ) == m_nSize )
		return strFile;

	int nSlash = m_sPath.Find( L'\\' );
	if ( nSlash != -1 )
	{
		CString strShortPath = m_sPath.Mid( nSlash + 1 );

		// Try complete folder without outer file directory
		strFile = Settings.Downloads.CompletePath + L"\\" + strShortPath;
		if ( GetFileSize( SafePath( strFile ) ) == m_nSize )
			return strFile;

		// Try folder of original .torrent without outer file directory
		strFile = strTorrentPath + strShortPath;
		if ( GetFileSize( SafePath( strFile ) ) == m_nSize )
			return strFile;
	}

	// Try find by name only
	if ( const CLibraryFile* pShared = LibraryMaps.LookupFileByName( m_sName, m_nSize, FALSE, TRUE ) )
	{
		strFile = pShared->GetPath();
		if ( GetFileSize( SafePath( strFile ) ) == m_nSize )
			return strFile;
	}

	// Try find by hash
	if ( const CLibraryFile* pShared = LibraryMaps.LookupFileByHash( this, FALSE, TRUE ) )
	{
		strFile = pShared->GetPath();
		if ( GetFileSize( SafePath( strFile ) ) == m_nSize )
			return strFile;
	}

	return m_sPath;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo clear

void CBTInfo::Clear()
{
	delete [] m_pBlockBTH;
	m_pBlockBTH			= NULL;

	m_nTotalUpload		= 0;
	m_nTotalDownload	= 0;

	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
		delete m_pFiles.GetNext( pos );
	m_pFiles.RemoveAll();

	m_nEncoding			= Settings.BitTorrent.TorrentCodePage;
	m_tCreationDate		= 0;
	m_sCreatedBy.Empty();
	m_sComment.Empty();
	m_bPrivate			= FALSE;
	m_nStartDownloads	= dtAlways;
	m_oTrackers.RemoveAll();
	m_nTrackerIndex		= -1;
	m_nTrackerMode		= tNull;
	m_bEncodingError	= false;
	m_pTestSHA1.Reset();
	m_nTestByte			= 0;
	m_pSource.Clear();
	m_nInfoSize			= 0;
	m_nInfoStart		= 0;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo copy

CBTInfo& CBTInfo::operator=(const CBTInfo& oSource)
{
	Clear();

	CEnvyFile::operator=( oSource );

	for ( POSITION pos = oSource.m_sURLs.GetHeadPosition() ; pos ; )
		m_sURLs.AddTail( oSource.m_sURLs.GetNext( pos ) );

	m_nBlockSize		= oSource.m_nBlockSize;
	m_nBlockCount		= oSource.m_nBlockCount;

	if ( oSource.m_pBlockBTH )
	{
		m_pBlockBTH = new Hashes::BtPureHash[ m_nBlockCount ];
		std::copy( oSource.m_pBlockBTH, oSource.m_pBlockBTH + m_nBlockCount,
			stdext::make_checked_array_iterator( m_pBlockBTH, m_nBlockCount ) );
	}

	m_nTotalUpload		= oSource.m_nTotalUpload;
	m_nTotalDownload	= oSource.m_nTotalDownload;

	for ( POSITION pos = oSource.m_pFiles.GetHeadPosition() ; pos ; )
		m_pFiles.AddTail( new CBTFile( this, oSource.m_pFiles.GetNext( pos ) ) );

	m_nEncoding			= oSource.m_nEncoding;
	m_sComment			= oSource.m_sComment;
	m_tCreationDate		= oSource.m_tCreationDate;
	m_sCreatedBy		= oSource.m_sCreatedBy;
	m_bPrivate			= oSource.m_bPrivate;
	m_nStartDownloads	= oSource.m_nStartDownloads;

	for ( INT_PTR i = 0 ; i < oSource.m_oTrackers.GetCount() ; ++i )
		m_oTrackers.Add( oSource.m_oTrackers[ i ] );

	for ( POSITION pos = oSource.m_oNodes.GetHeadPosition() ; pos ; )
		m_oNodes.AddTail( oSource.m_oNodes.GetNext( pos ) );

	m_nTrackerIndex		= oSource.m_nTrackerIndex;
	m_nTrackerMode		= oSource.m_nTrackerMode;
	m_bEncodingError	= oSource.m_bEncodingError;
	m_pTestSHA1			= oSource.m_pTestSHA1;
	m_nTestByte			= oSource.m_nTestByte;

	m_pSource.Add( oSource.m_pSource.m_pBuffer, oSource.m_pSource.m_nLength );
	m_nInfoSize			= oSource.m_nInfoSize;
	m_nInfoStart		= oSource.m_nInfoStart;

	return *this;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo serialize

// Set at INTERNAL_VERSION on change:
#define BTINFO_SER_VERSION 1

// nVersion History:
//  7 - redesigned tracker list (ryo-oh-ki)
//  8 - removed m_nFilePriority (ryo-oh-ki)
//  9 - added m_sName (ryo-oh-ki)
// 10 - added m_pSource (ivan386) (Shareaza 2.5.2)
// 11 - added m_nInfoStart & m_nInfoSize (ivan386)
// 1000 - (11)
// 1 - (Envy 1.0)

void CBTInfo::Serialize(CArchive& ar)
{
	int nVersion = BTINFO_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;

		SerializeOut( ar, m_oBTH );
		if ( ! m_oBTH ) return;

		ar << m_nSize;
		ar << m_nBlockSize;
		ar << m_nBlockCount;
		for ( DWORD i = 0 ; i < m_nBlockCount ; ++i )
		{
			ar.Write( &*m_pBlockBTH[ i ].begin(), m_pBlockBTH->byteCount );
		}

		ar << m_nTotalUpload;
		ar << m_nTotalDownload;

		ar << m_sName;

		ar << m_nEncoding;
		ar << m_sComment;
		ar << m_tCreationDate;
		ar << m_sCreatedBy;
		ar << m_bPrivate;

		ar.WriteCount( m_pFiles.GetCount() );
		for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
			m_pFiles.GetNext( pos )->Serialize( ar, nVersion );

		ar << m_nTrackerIndex;
		ar << m_nTrackerMode;

		int nTrackers = (int)m_oTrackers.GetCount();
		ar.WriteCount( nTrackers );
		for ( int nTracker = 0 ; nTracker < nTrackers ; nTracker++ )
		{
			m_oTrackers[ nTracker ].Serialize( ar, nVersion );
		}

		if ( m_pSource.m_nLength && m_nInfoSize )
		{
			ar << m_pSource.m_nLength;
			ar.Write( m_pSource.m_pBuffer, m_pSource.m_nLength );
			ar << m_nInfoStart;
			ar << m_nInfoSize;
		}
		else
		{
			ar << (DWORD)0;
		}
	}
	else // Loading
	{
		// ToDo: Are any BTINFO_SER_VERSION nVersions still necessary for Shareaza imports?

		ar >> nVersion;
		if ( nVersion > INTERNAL_VERSION && nVersion != 1000 )
			AfxThrowUserException();

		SerializeIn( ar, m_oBTH, nVersion );
		if ( ! m_oBTH ) return;

		ar >> m_nSize;
		ar >> m_nBlockSize;
		ar >> m_nBlockCount;

		if ( m_nBlockCount )
		{
			m_pBlockBTH = new Hashes::BtPureHash[ (DWORD)m_nBlockCount ];

			for ( DWORD i = 0 ; i < m_nBlockCount ; ++i )
			{
				ReadArchive( ar, &*m_pBlockBTH[ i ].begin(), m_pBlockBTH->byteCount );
			}
		}

		ar >> m_nTotalUpload;
		ar >> m_nTotalDownload;

		ar >> m_sName;

		ar >> m_nEncoding;
		ar >> m_sComment;
		ar >> m_tCreationDate;
		ar >> m_sCreatedBy;

		ar >> m_bPrivate;

		int nFiles = (int)ar.ReadCount();
		QWORD nOffset = 0;
		for ( int nFile = 0 ; nFile < nFiles ; nFile++ )
		{
			CAutoPtr< CBTFile >pBTFile( new CBTFile( this ) );
			if ( ! pBTFile )
				AfxThrowUserException();	// Out Of Memory

			pBTFile->Serialize( ar, nVersion );

			pBTFile->m_nOffset = nOffset;
			nOffset += pBTFile->m_nSize;

			m_pFiles.AddTail( pBTFile.Detach() );
		}

		//if ( nVersion < 7 )	// Shareaza 2.3
		//{
		//	CString strTracker;
		//	ar >> strTracker;
		//	SetTracker( strTracker );
		//}

		ar >> m_nTrackerIndex;
		ar >> m_nTrackerMode;

		//if ( nVersion < 7 )	// Shareaza 2.3
		//{
		//	int nTrackers = (int)ar.ReadCount();
		//	if ( nTrackers )
		//	{
		//		CBTTracker oTracker;
		//		oTracker.Serialize( ar, nVersion );
		//		AddTracker( oTracker );
		//	}
		//}

		int nTrackers = (int)ar.ReadCount();
		if ( nTrackers )
		{
			for ( int nTracker = 0 ; nTracker < nTrackers ; nTracker++ )
			{
				CBTTracker oTracker;
				oTracker.Serialize( ar, nVersion );
				AddTracker( oTracker );
			}
		}

		//if ( nVersion >= 10 )	// Shareaza 2.5.2.0+
		{
			DWORD nLength;
			ar >> nLength;
			if ( nLength )
			{
				m_pSource.EnsureBuffer( nLength );
				ar.Read( m_pSource.m_pBuffer, nLength );
				m_pSource.m_nLength = nLength;
				ar >> m_nInfoStart;
				ar >> m_nInfoSize;
			}
		}

		SetTrackerNext();

		// Imported Partial from Shareaza 2.4
		//if ( nVersion < 8 )
		//	ConvertOldTorrents();
	}
}

//void CBTInfo::ConvertOldTorrents()
//{
//	// For importing Shareaza 2.4 multifile partials
//	// Legacy for reference only.
//
//	if ( m_pFiles.GetCount() < 2 )
//		return;
//
//	if ( ! Downloads.IsSpaceAvailable( m_nSize, Downloads.dlPathComplete ) )
//		AfxThrowFileException( CFileException::diskFull );
//
//	CString strSource;
//	strSource.Format( L"%s\\%s.partial", Settings.Downloads.IncompletePath, GetFilename() );
//	if ( strSource.GetLength() > MAX_PATH ) strSource = L"\\\\?\\" + strSource;
//
//	if ( GetFileAttributes( strSource ) == INVALID_FILE_ATTRIBUTES )
//		return;
//
//	CFile oSource( strSource, CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone );
//
//	const DWORD BUFFER_SIZE = 8ul * 1024ul * 1024ul;
//	BYTE* pBuffer = new BYTE[ BUFFER_SIZE ];
//	if ( ! pBuffer )
//		AfxThrowMemoryException();
//
//	CString strTargetTemplate;
//	strTargetTemplate.Format( L"%s\\%s", Settings.Downloads.IncompletePath, GetFilename() );
//	if ( strTargetTemplate.GetLength() > MAX_PATH ) strTargetTemplate = L"\\\\?\\" + strTargetTemplate;
//
//	CString strText;
//	CProgressBarDlg oProgress( CWnd::GetDesktopWindow() );
//	strText.LoadString( IDS_BT_UPDATE_TITLE );
//	oProgress.SetWindowText( strText );
//	strText.LoadString( IDS_BT_UPDATE_CONVERTING );
//	oProgress.SetActionText( strText );
//	oProgress.SetEventText( m_sName );
//	oProgress.SetEventRange( 0, int( m_nSize / 1024ull ) );
//	oProgress.CenterWindow();
//	oProgress.ShowWindow( SW_SHOW );
//	oProgress.UpdateWindow();
//	oProgress.UpdateData( FALSE );
//
//	CString strOf;
//	strOf.LoadString( IDS_GENERAL_OF );
//	QWORD nTotal = 0ull;
//	DWORD nCount = 0ul;
//	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; ++nCount )
//	{
//		CBTFile* pFile = m_pFiles.GetNext( pos );
//
//		CString strTarget;
//		strTarget.Format( L"%s_%lu.partial", strTargetTemplate, nCount );
//
//		CFile oTarget( strTarget, CFile::modeCreate | CFile::modeWrite | CFile::osSequentialScan );
//
//		strText.Format( L"%lu %s %i", nCount + 1ul, strOf, m_pFiles.GetCount() );
//
//		if ( Settings.General.LanguageRTL )
//			strText = L"\x202B" + strText;
//
//		oProgress.SetSubActionText( strText );
//		oProgress.SetSubEventText( pFile->m_sPath );
//		oProgress.SetSubEventRange( 0, int( pFile->m_nSize / 1024ull ) );
//		oProgress.UpdateData( FALSE );
//		oProgress.UpdateWindow();
//
//		QWORD nLength = pFile->m_nSize;
//		while ( nLength )
//		{
//			DWORD nBuffer = min( nLength, BUFFER_SIZE );
//
//			nBuffer = oSource.Read( pBuffer, nBuffer );
//			if ( nBuffer )
//				oTarget.Write( pBuffer, nBuffer );
//
//			nLength -= nBuffer;
//			nTotal += nBuffer;
//
//			oProgress.StepSubEvent( int( nBuffer / 1024ul ) );
//			oProgress.SetEventPos( int( nTotal / 1024ull ) );
//			oProgress.UpdateWindow();
//		}
//	}
//	ASSERT( nTotal == m_nSize );
//
//	delete [] pBuffer;
//}

//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTFile serialize

void CBTInfo::CBTFile::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_nSize;
		ar << m_sPath;
		ar << m_sName;
		SerializeOut( ar, m_oSHA1 );
		SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oTiger );
		SerializeOut( ar, m_oMD5 );
	}
	else // Loading
	{
		ar >> m_nSize;
		ar >> m_sPath;
		ar >> m_sName;

		//if ( nVersion < 8 ) // Upgrade old Shareaza import
		//	m_sName = PathFindFileName( m_sPath );

		SerializeIn( ar, m_oSHA1, nVersion );
		SerializeIn( ar, m_oED2K, nVersion );
		SerializeIn( ar, m_oTiger, nVersion );
		//if ( nVersion < 8 )
		//{
		//	int nFilePriority;
		//	ar >> nFilePriority;
		//}
		SerializeIn( ar, m_oMD5, nVersion );
	}
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load .torrent file

BOOL CBTInfo::LoadTorrentFile(LPCTSTR pszFile)
{
	CFile pFile;
	if ( pFile.Open( pszFile, CFile::modeRead|CFile::shareDenyNone ) )
	{
		DWORD nLength = (DWORD)pFile.GetLength();
		m_sPath = pszFile;

		if ( nLength < 20 * 1024 * 1024 && nLength != 0 )
		{
			m_pSource.Clear();
			if ( m_pSource.EnsureBuffer( nLength ) )
			{
				pFile.Read( m_pSource.m_pBuffer, nLength );
				m_pSource.m_nLength = nLength;

				return LoadTorrentBuffer( &m_pSource );
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo save .torrent file

BOOL CBTInfo::SaveTorrentFile(const CString& sFolder)
{
	if ( ! IsAvailable() )
		return FALSE;

	if ( m_pSource.m_nLength == 0 )
		return FALSE;

	CString strPath = sFolder + L"\\" + SafeFilename( m_sName + L".torrent" );
	if ( m_sPath.CompareNoCase( strPath ) == 0 )
		return TRUE;	// Same file

	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeWrite | CFile::modeCreate ) )
		return FALSE;

	pFile.Write( m_pSource.m_pBuffer, m_pSource.m_nLength );
	pFile.Close();

	//m_sPath = strPath;	// ToDo?

	return TRUE;
}

#define MAX_PIECE_SIZE (16 * 1024)
BOOL CBTInfo::LoadInfoPiece(BYTE *pPiece, DWORD nPieceSize, DWORD nInfoSize, DWORD nInfoPiece)
{
	ASSERT( nPieceSize <= MAX_PIECE_SIZE );
	if ( nPieceSize > MAX_PIECE_SIZE )
		return FALSE;

	if ( m_pSource.m_nLength == 0 && nInfoPiece == 0 )
	{
		CBENode oRoot;
		if ( GetTrackerCount() > 0 )
		{
			// Create .torrent file with tracker if needed

			oRoot.Add( "announce" )->SetString( GetTrackerAddress() );	// "8:announce%d:%s"

			if ( GetTrackerCount() > 1 )
			{
				CBENode* pList = oRoot.Add("announce-list")->Add(); 	// "13:announce-listll"
				for ( int i = 0 ; i < GetTrackerCount() ; i++ )
				{
					pList->Add()->SetString( GetTrackerAddress( i ) );
				}
			}
		}
		oRoot.Add( "info" )->SetInt(0); 	// "d4:info"
		oRoot.Encode( &m_pSource );
		m_pSource.m_nLength -= 4;
		m_nInfoStart = m_pSource.m_nLength;
	}

	QWORD nPieceStart = nInfoPiece * MAX_PIECE_SIZE;

	if ( nPieceStart == ( m_pSource.m_nLength - m_nInfoStart ) )
	{
		m_pSource.Add( pPiece, nPieceSize );

		if ( m_pSource.m_nLength - m_nInfoStart == nInfoSize )
		{
			m_pSource.Add( "e", 1 );
			return LoadTorrentBuffer( &m_pSource );
		}
	}
	return FALSE;
}

int CBTInfo::NextInfoPiece() const
{
	if ( m_pSource.m_nLength == 0 )
		return 0;

	if ( ! m_nInfoSize && m_pSource.m_nLength > m_nInfoStart )
		return ( m_pSource.m_nLength - m_nInfoStart ) / MAX_PIECE_SIZE;

	return -1;
}

DWORD CBTInfo::GetInfoPiece(DWORD nPiece, BYTE **pInfoPiece) const
{
	const DWORD nPieceStart = MAX_PIECE_SIZE * nPiece;
	if ( m_nInfoSize && m_nInfoStart &&
		m_pSource.m_nLength > m_nInfoStart + m_nInfoSize &&
		nPieceStart < m_nInfoSize )
	{
		*pInfoPiece = &m_pSource.m_pBuffer[ m_nInfoStart + nPieceStart ];
		DWORD nPieceSize = m_nInfoSize - nPieceStart;
		return nPieceSize > MAX_PIECE_SIZE ? MAX_PIECE_SIZE : nPieceSize;
	}
	return 0;
}

DWORD CBTInfo::GetInfoSize() const
{
	return m_nInfoSize;
}

BOOL CBTInfo::CheckInfoData()
{
	ASSERT( m_pSource.m_nLength );

	if ( ! m_pSource.m_nLength ) return FALSE;

	auto_ptr< CBENode > pNode ( CBENode::Decode( &m_pSource ) );
	if ( ! pNode.get() )
		return FALSE;

	const CBENode* pRoot = pNode.get();
	const CBENode* pInfo = pRoot->GetNode( "info" );

	if ( pInfo && pInfo->m_nSize &&
		 pInfo->m_nPosition + pInfo->m_nSize < m_pSource.m_nLength )
	{
		Hashes::BtHash oBTH;
		CSHA pBTH;
		pBTH.Add( &m_pSource.m_pBuffer[pInfo->m_nPosition], pInfo->m_nSize );
		pBTH.Finish();
		pBTH.GetHash( &oBTH[0] );

		if ( oBTH == m_oBTH )
		{
			m_nInfoStart = pInfo->m_nPosition;
			m_nInfoSize  = pInfo->m_nSize;
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from buffer

BOOL CBTInfo::LoadTorrentBuffer(const CBuffer* pBuffer)
{
	auto_ptr< CBENode > pNode ( CBENode::Decode( pBuffer ) );
	if ( ! pNode.get() )
	{
		theApp.Message( MSG_ERROR, L"[BT] Failed to decode torrent data: %s", pBuffer->ReadString( (size_t)-1 ) );
		return FALSE;
	}

	return LoadTorrentTree( pNode.get() );
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from tree (keys)

BOOL CBTInfo::LoadTorrentTree(const CBENode* pRoot)
{
	//ASSERT( m_sName.IsEmpty() && m_nSize == SIZE_UNKNOWN );	// Assume empty object
	ASSERT( ! m_pBlockBTH );

	theApp.Message( MSG_DEBUG, L"[BT] Loading torrent tree: %s", (LPCTSTR)pRoot->Encode() );

	if ( ! pRoot->IsType( CBENode::beDict ) )
		return FALSE;

	// Get the info node
	const CBENode* pInfo = pRoot->GetNode( "info" );
	if ( ! pInfo || ! pInfo->IsType( CBENode::beDict ) )
		return FALSE;

	if ( m_oBTH )
	{
		CSHA oSHA = pInfo->GetSHA1();
		Hashes::BtHash oBTH;
		oSHA.GetHash( &oBTH[ 0 ] );
		oBTH.validate();

		if ( oBTH != m_oBTH )
			return FALSE;
	}

	// Get the encoding (from torrents that have it)
	m_nEncoding = 0;
	const CBENode* pEncoding = pRoot->GetNode( "codepage" );
	if ( pEncoding && pEncoding->IsType( CBENode::beInt ) )
	{
		// "codepage" style (UNIT giving the exact Windows code page)
		m_nEncoding = (UINT)pEncoding->GetInt();
	}
	else
	{
		// "encoding" style (String representing the encoding to use)
		pEncoding = pRoot->GetNode( "encoding" );
		if ( pEncoding && pEncoding->IsType( CBENode::beString ) )
		{
			CString strEncoding = pEncoding->GetString();

			if ( strEncoding.GetLength() < 3 )
				theApp.Message( MSG_ERROR, L"Torrent 'encoding' node too short" );
			else if ( _tcsistr( strEncoding.GetString(), L"UTF-8" ) != NULL ||
					  _tcsistr( strEncoding.GetString(), L"UTF8" ) != NULL )
				m_nEncoding = CP_UTF8;
			else if ( _tcsistr( strEncoding.GetString(), L"ANSI" ) != NULL )
				m_nEncoding = CP_ACP;
			else if ( _tcsistr( strEncoding.GetString(), L"BIG5" ) != NULL )
				m_nEncoding = 950;
			else if ( _tcsistr( strEncoding.GetString(), L"Korean" ) != NULL )
				m_nEncoding = 949;
			else if ( _tcsistr( strEncoding.GetString(), L"UHC" ) != NULL )
				m_nEncoding = 949;
			else if ( _tcsistr( strEncoding.GetString(), L"Chinese" ) != NULL )
				m_nEncoding = 936;
			else if ( _tcsistr( strEncoding.GetString(), L"GB2312" ) != NULL )
				m_nEncoding = 936;
			else if ( _tcsistr( strEncoding.GetString(), L"GBK" ) != NULL )
				m_nEncoding = 936;
			else if ( _tcsistr( strEncoding.GetString(), L"Japanese" ) != NULL )
				m_nEncoding = 932;
			else if ( _tcsistr( strEncoding.GetString(), L"Shift-JIS" ) != NULL )
				m_nEncoding = 932;
			else if ( _tcsnicmp( strEncoding.GetString(), L"Windows-", 8 ) == 0 )
			{
				UINT nEncoding = 0;
				strEncoding = strEncoding.Mid( 8 );
				if ( ( _stscanf( strEncoding, L"%u", &nEncoding ) == 1 ) && ( nEncoding > 0 ) )
					m_nEncoding = nEncoding;
			}
			else if ( _tcsnicmp( strEncoding.GetString(), L"CP", 2 ) == 0 )
			{
				UINT nEncoding = 0;
				strEncoding = strEncoding.Mid( 2 );
				if ( ( _stscanf( strEncoding, L"%u", &nEncoding ) == 1 ) && ( nEncoding > 0 ) )
					m_nEncoding = nEncoding;
			}
		}
	}

	// Get the comments (if present)
	m_sComment = pRoot->GetStringFromSubNode( "comment", m_nEncoding );

	// Get the creation date (if present)
	const CBENode* pDate = pRoot->GetNode( "creation date" );
	if ( pDate && pDate->IsType( CBENode::beInt ) )
		m_tCreationDate = (DWORD)pDate->GetInt();
		// CTime pTime( (time_t)m_tCreationDate );
		// theApp.Message( MSG_NOTICE, pTime.Format( L"%Y-%m-%d %H:%M:%S" ) );

	// Get the creator (if present)
	m_sCreatedBy = pRoot->GetStringFromSubNode( "created by", m_nEncoding );

	// Get nodes for DHT (if present) BEP 0005
	const CBENode* pNodeList = pRoot->GetNode( "nodes" );
	if ( pNodeList && pNodeList->IsType( CBENode::beList ) )
	{
		for ( int i = 0 ; i < pNodeList->GetCount() ; ++i )
		{
			const CBENode* pNode = pNodeList->GetNode( i );
			if ( pNode && pNode->IsType( CBENode::beList ) && pNode->GetCount() == 2 )
			{
				const CBENode* pHost = pNode->GetNode( 0 );
				const CBENode* pPort = pNode->GetNode( 1 );
				if ( pHost && pHost->IsType( CBENode::beString ) && pPort && pPort->IsType( CBENode::beInt ) )
				{
					CString strHost;
					strHost.Format( L"%s:%u", pHost->GetString(), (WORD)pPort->GetInt() );
					m_oNodes.AddTail( strHost );
				//	HostCache.BitTorrent.Add( pHost->GetString(), (WORD)pPort->GetInt() );	// Obsolete
				}
			}
		}
	}

	// Get announce-list (if present)
	CBENode* pAnnounceList = pRoot->GetNode( "announce-list" );
	if ( pAnnounceList && pAnnounceList->IsType( CBENode::beList ) )
	{
		m_nTrackerMode = tMultiFinding;

		CList< CString > pTrackers, pBadTrackers;

		// Loop through all the tiers
		for ( int nTier = 0 ; nTier < pAnnounceList->GetCount() ; nTier++ )
		{
			const CBENode* pSubList = pAnnounceList->GetNode( nTier );
			if ( pSubList && pSubList->IsType( CBENode::beList ) )
			{
				// Read in the trackers
				for ( int nTracker = 0 ; nTracker < pSubList->GetCount() ; nTracker++ )
				{
					const CBENode* pTracker = pSubList->GetNode( nTracker );
					if ( pTracker && pTracker->IsType( CBENode::beString ) )
					{
						CString strTracker = pTracker->GetString();		// Get the tracker

						// Unescape if needed
						if ( strTracker.Find( L"%3A", 2 ) > 2 )
						{
							strTracker.Replace( L"%3A", L":" );
							strTracker.Replace( L"%2F", L"/" );
						}

						// Check tracker is valid
						if ( ! StartsWith( strTracker, _P( L"http://" ) ) &&
							 ! StartsWith( strTracker, _P( L"udp://" ) ) )					// ToDo: Handle rare HTTPS etc?
							pBadTrackers.AddTail( BAD_TRACKER_TOKEN + strTracker );			// Store unknown tracker for display (*https://)
						else if ( IsDeadTracker( strTracker ) )
							pBadTrackers.AddTail( BAD_TRACKER_TOKEN + strTracker );			// Store common dead trackers for display
						else
							pTrackers.AddTail( strTracker );								// Store TCP tracker
					}
				}

				if ( ! pTrackers.IsEmpty() )
				{
					// Randomize the tracker order in this tier
					if ( pTrackers.GetCount() > 1 )
					{
						for ( POSITION pos = pTrackers.GetHeadPosition() ; pos ; )
						{
							if ( GetRandomNum( 0, 1 ) )
							{
								CString strTemp = pTrackers.GetAt( pos );
								pTrackers.RemoveAt( pos );

								if ( GetRandomNum( 0, 1 ) )
									pTrackers.AddHead( strTemp );
								else
									pTrackers.AddTail( strTemp );
							}
							pTrackers.GetNext( pos );
						}
					}

					// Store the trackers
					for ( POSITION pos = pTrackers.GetHeadPosition() ; pos ; )
					{
						// Create the tracker and add it to the list
						AddTracker( CBTTracker( pTrackers.GetNext( pos ), nTier ) );
					}
					// Delete temporary storage
					pTrackers.RemoveAll();
				}
			}
		}

		// Catch unsupported trackers for display at end of list.
		if ( ! pBadTrackers.IsEmpty() )
		{
			for ( POSITION pos = pBadTrackers.GetHeadPosition() ; pos ; )
			{
				// Create the tracker and add it to the list
				AddTracker( CBTTracker( pTrackers.GetNext( pos ), 99 ) );

			//	CBTTracker oTracker;
			//	oTracker.m_sAddress = BAD_TRACKER_TOKEN + pTrackers.GetNext( pos );				// Mark for display only: *https://...
			//	oTracker.m_nFailures = 1;
			//	oTracker.m_nTier = 99;
			//	AddTracker( oTracker );
			}
		}

		SetTrackerNext();
	}

	// Announce node is ignored by multi-tracker torrents
	if ( m_oTrackers.IsEmpty() )
	{
		// Get announce
		const CBENode* pAnnounce = pRoot->GetNode( "announce" );
		if ( pAnnounce && pAnnounce->IsType( CBENode::beString ) )
		{
			// Get the tracker
			CString strTracker = pAnnounce->GetString();

			// Unescape if needed
			if ( strTracker.Find( L"%3A", 2 ) > 2 )
			{
				strTracker.Replace( L"%2F", L"/" );
				strTracker.Replace( L"%3A", L":" );
			}

			// Store it if it's valid. (Some torrents have invalid trackers)
			if ( strTracker.GetLength() < 10 ||
				 ( ! StartsWith( strTracker, _P( L"http://" ) ) &&	// (ToDo: Support https://)
				   ! StartsWith( strTracker, _P( L"udp://" ) ) ) ||
				 IsDeadTracker( strTracker ) )
			{
				// Catch unknown and common defunct trackers  (ToDo: Get Private tag first)
				strTracker = Settings.BitTorrent.DefaultTracker;	// L"udp://tracker.openbittorrent.com:80/announce"

				// Set the torrent to be a single-tracker torrent
				m_nTrackerMode = tSingle;
				SetTracker( strTracker );

				// Backup tracker
				//CBTTracker oTracker;
				//oTracker.m_sAddress = Settings.BitTorrent.DefaultTracker;
				//oTracker.m_nTier = 0;
				//m_nTrackerMode = tMultiFinding;
				//AddTracker( oTracker );
			}

			SetTracker( strTracker );
			m_nTrackerMode = tSingle;
		}
	}

	// Get the info node (As above, for reference)
	//CBENode* pInfo = pRoot->GetNode( "info" );
	//if ( ! pInfo || ! pInfo->IsType( CBENode::beDict ) )
	//	return FALSE;

	// Get the private flag (if present)
	const CBENode* pPrivate = pInfo->GetNode( "private" );
	if ( pPrivate && pPrivate->IsType( CBENode::beInt ) )
		m_bPrivate = pPrivate->GetInt() != 0;

	// Get the name
	m_sName = pInfo->GetStringFromSubNode( "name", m_nEncoding );

	CString strURL;		// Root for path, if given

	// Get Web Seed  (http://bittorrent.org/beps/bep_0019.html)
	if ( const CBENode* pURL = pRoot->GetNode( "url-list" ) )
	{
		// Get the source
		if ( pURL->IsType( CBENode::beString ) )
		{
			strURL = pURL->GetString();
		}
		else if ( pURL->IsType( CBENode::beList ) )
		{
			// ToDo: Handle full list
			if ( const CBENode* pSource = pURL->GetNode( 0 ) )
			{
				if ( pSource->IsType( CBENode::beString ) )
					strURL = pSource->GetString();
			}
		}

		if ( ! strURL.IsEmpty() )
		{
			// Unescape if needed
			if ( strURL.Find( L"%2F", 3 ) > 0 )
			{
				strURL.Replace( L"%3A", L":" );
				strURL.Replace( L"%2F", L"/" );
			}

			// Ensure it's valid
			if ( ! StartsWith( strURL, _P( L"http://" ) ) )
				strURL.Empty();
			else if ( strURL.Right( 1 ) == L'/' && ! m_sName.IsEmpty() )
				strURL += m_sName + L'/';

			// Note: Handle http source below
		}
	}

	// If we still don't have a name, generate one
	if ( m_sName.IsEmpty() )
		m_sName.Format( L"Torrent-%i", GetRandomNum( 0i32, _I32_MAX ) );

	// Get the piece stuff
	const CBENode* pPL = pInfo->GetNode( "piece length" );
	if ( ! pPL || ! pPL->IsType( CBENode::beInt ) ) return FALSE;
	m_nBlockSize = (DWORD)pPL->GetInt();
	if ( ! m_nBlockSize ) return FALSE;

	const CBENode* pHash = pInfo->GetNode( "pieces" );
	if ( ! pHash || ! pHash->IsType( CBENode::beString ) ) return FALSE;
	if ( pHash->m_nValue % Hashes::Sha1Hash::byteCount ) return FALSE;
	m_nBlockCount = (DWORD)( pHash->m_nValue / Hashes::Sha1Hash::byteCount );
	if ( ! m_nBlockCount || m_nBlockCount > 209716 ) return FALSE;

	m_pBlockBTH = new Hashes::BtPureHash[ m_nBlockCount ];

	std::copy( static_cast< const Hashes::BtHash::RawStorage* >( pHash->m_pValue ),
		static_cast< const Hashes::BtHash::RawStorage* >( pHash->m_pValue ) + m_nBlockCount,
		stdext::make_checked_array_iterator( m_pBlockBTH, m_nBlockCount ) );

	// Hash info
	if ( const CBENode* pSHA1 = pInfo->GetNode( "sha1" ) )
	{
		if ( ! pSHA1->IsType( CBENode::beString ) || pSHA1->m_nValue != Hashes::Sha1Hash::byteCount ) return FALSE;
		m_oSHA1 = *static_cast< const Hashes::BtHash::RawStorage* >( pSHA1->m_pValue );
	}
	else if ( const CBENode* pSHA1Base16 = pInfo->GetNode( "filehash" ) )
	{
		if ( ! pSHA1Base16->IsType( CBENode::beString ) ||
			pSHA1Base16->m_nValue != Hashes::BtGuid::byteCount ) return FALSE;
		m_oSHA1 = *static_cast< const Hashes::BtGuid::RawStorage* >( pSHA1Base16->m_pValue );
	}

	if ( const CBENode* pED2K = pInfo->GetNode( "ed2k" ) )
	{
		if ( ! pED2K->IsType( CBENode::beString ) || pED2K->m_nValue != Hashes::Ed2kHash::byteCount ) return FALSE;
		m_oED2K = *static_cast< const Hashes::Ed2kHash::RawStorage* >( pED2K->m_pValue );
	}

	if ( const CBENode* pMD5 = pInfo->GetNode( "md5sum" ) )
	{
		if ( ! pMD5->IsType( CBENode::beString ) ) return FALSE;

		else if ( pMD5->m_nValue == Hashes::Md5Hash::byteCount )
		{
			m_oMD5 = *static_cast< const Hashes::Md5Hash::RawStorage* >( pMD5->m_pValue );
		}
		else if ( pMD5->m_nValue == Hashes::Md5Hash::byteCount * 2 )
		{
			CStringA tmp;
			tmp.Append( (const char*)pMD5->m_pValue, (int)pMD5->m_nValue );
			m_oMD5.fromString( CA2W( tmp ) );
		}
		else
		{
			return FALSE;
		}
	}

	if ( const CBENode* pTiger = pInfo->GetNode( "tiger" ) )
	{
		if ( ! pTiger->IsType( CBENode::beString ) || pTiger->m_nValue != Hashes::TigerHash::byteCount ) return FALSE;
		m_oTiger = *static_cast< const Hashes::TigerHash::RawStorage* >( pTiger->m_pValue );
	}

	// Details on file (or files).
	if ( const CBENode* pSingleLength = pInfo->GetNode( "length" ) )
	{
		if ( ! pSingleLength->IsType( CBENode::beInt ) )
			return FALSE;
		m_nSize = (QWORD)pSingleLength->GetInt();
		if ( ! m_nSize )
			return FALSE;

		CAutoPtr< CBTFile >pBTFile( new CBTFile( this ) );
		if ( ! pBTFile )		// Out of memory
			return FALSE;

		pBTFile->m_sPath = m_sName;
		pBTFile->m_sName = PathFindFileName( m_sName );
		pBTFile->m_nSize = m_nSize;
		pBTFile->m_oSHA1 = m_oSHA1;
		pBTFile->m_oTiger = m_oTiger;
		pBTFile->m_oED2K = m_oED2K;
		pBTFile->m_oMD5 = m_oMD5;
		m_pFiles.AddTail( pBTFile.Detach() );

		// Add sources from torrents - DWK
		const CBENode* pSources = pRoot->GetNode( "sources" );
		if ( pSources && pSources->IsType( CBENode::beList ) )
		{
			int m_nSources = pSources->GetCount();
			for ( int nSource = 0 ; nSource < m_nSources ; nSource++ )
			{
				CBENode* pSource = pSources->GetNode( nSource );
				if ( ! pSource || ! pSource->IsType( CBENode::beString ) ) continue;
				m_sURLs.AddTail( pSource->GetString() );
			}
		}

		// Add http web seed
		if ( ! strURL.IsEmpty() )
		{
			if ( strURL.Right( 1 ) == L'/' )
				strURL += pBTFile->m_sName;
			m_sURLs.AddTail( strURL );
		}
	}
	else if ( const CBENode* pFiles = pInfo->GetNode( "files" ) )
	{
		CString strPath;

		if ( ! pFiles->IsType( CBENode::beList ) ) return FALSE;
		int nFiles = pFiles->GetCount();
		if ( ! nFiles || nFiles > 8192 * 8 ) return FALSE;

		m_nSize = 0;

		QWORD nOffset = 0;
		for ( int nFile = 0 ; nFile < nFiles ; nFile++ )
		{
			CAutoPtr< CBTFile > pBTFile( new CBTFile( this ) );
			if ( ! pBTFile )		// Out of Memory
				return FALSE;

			const CBENode* pFile = pFiles->GetNode( nFile );
			if ( ! pFile || ! pFile->IsType( CBENode::beDict ) ) return FALSE;

			const CBENode* pLength = pFile->GetNode( "length" );
			if ( ! pLength || ! pLength->IsType( CBENode::beInt ) ) return FALSE;
			pBTFile->m_nSize = (QWORD)pLength->GetInt();

			pBTFile->m_nOffset = nOffset;

			strPath.Empty();

			// Try path.utf8 if it's set  (Was Settings.BitTorrent.TorrentExtraKeys)
			const CBENode* pPath = pFile->GetNode( "path.utf-8" );
			if ( pPath && pPath->IsType( CBENode::beList ) )
			{
				const CBENode* pPart = pPath->GetNode( 0 );
				if ( pPart && pPart->IsType( CBENode::beString ) )
					strPath = pPart->GetString();
			}

			// Get the regular path
			pPath = pFile->GetNode( "path" );

			if ( ! pPath || ! pPath->IsType( CBENode::beList ) ) return FALSE;

			const CBENode* pPathPart = pPath->GetNode( 0 );
			if ( pPathPart && pPathPart->IsType( CBENode::beString ) )
			{
				if ( ! IsValid( strPath ) )
				{
					strPath = pPathPart->GetString();	// Get the path
				}
				else
				{
					// Check the path matches the .utf path
					CString strCheck = pPathPart->GetString();
					if ( strPath != strCheck )
						m_bEncodingError = true;
					// Switch back to the UTF-8 path
					pPath = pFile->GetNode( "path.utf-8" );
				}
			}

			// If that didn't work, try decoding the path
			if ( ! IsValid( strPath ) )
			{
				// There was an error reading the path
				m_bEncodingError = true;
				// Open path node
				pPath = pFile->GetNode( "path" );
				if ( pPath )
				{
					const CBENode* pPart = pPath->GetNode( 0 );
					if ( pPart->IsType( CBENode::beString ) )
						strPath = pPart->DecodeString(m_nEncoding);
				}
			}

			if ( ! pPath || ! pPath->IsType( CBENode::beList ) ) return FALSE;
			if ( strPath.CompareNoCase( L"#ERROR#" ) == 0 ) return FALSE;

			pBTFile->m_sName = PathFindFileName( strPath );

			// Hack to prefix all
			pBTFile->m_sPath = SafeFilename( m_sName );

			for ( int nPath = 0 ; nPath < pPath->GetCount() ; nPath++ )
			{
				const CBENode* pPart = pPath->GetNode( nPath );
				if ( ! pPart || ! pPart->IsType( CBENode::beString ) ) return FALSE;

				if ( ! pBTFile->m_sPath.IsEmpty() )
					pBTFile->m_sPath += '\\';
				const int nPathLength = pBTFile->m_sPath.GetLength();
				if ( nPathLength && pBTFile->m_sPath.GetAt( nPathLength - 1 ) != L'\\' )
					pBTFile->m_sPath += L'\\';

				// Get the path

				// Check for encoding error
				if ( pPart->GetString().CompareNoCase( L"#ERROR#" ) == 0 )
					strPath = SafeFilename( pPart->DecodeString( m_nEncoding ), true );
				else
					strPath = SafeFilename( pPart->GetString(), true );

				pBTFile->m_sPath += strPath;
			}

			if ( const CBENode* pSHA1 = pFile->GetNode( "sha1" ) )
			{
				if ( ! pSHA1->IsType( CBENode::beString ) )
					return FALSE;

				if ( pSHA1->m_nValue == Hashes::Sha1Hash::byteCount )
				{
					pBTFile->m_oSHA1 = *static_cast< Hashes::Sha1Hash::RawStorage* >( pSHA1->m_pValue );
				}
				else if ( pSHA1->m_nValue == Hashes::Sha1Hash::byteCount * 2 )
				{
					CStringA tmp;
					tmp.Append( (const char*)pSHA1->m_pValue, (int)pSHA1->m_nValue );
					pBTFile->m_oSHA1.fromString( CA2W( tmp ) );
				}
				else
				{
					return FALSE;
				}
			}

			if ( const CBENode* pED2K = pFile->GetNode( "ed2k" ) )
			{
				if ( pED2K->IsType( CBENode::beString ) && pED2K->m_nValue != Hashes::Ed2kHash::byteCount )
					pBTFile->m_oED2K = *static_cast< Hashes::Ed2kHash::RawStorage* >( pED2K->m_pValue );
			}

			if ( const CBENode* pMD5 = pFile->GetNode( "md5sum" ) )
			{
				if ( ! pMD5->IsType( CBENode::beString ) )
				{
					return FALSE;
				}
				else if ( pMD5->m_nValue == Hashes::Md5Hash::byteCount )
				{
					pBTFile->m_oMD5 = *static_cast< const Hashes::Md5Hash::RawStorage* >( pMD5->m_pValue );
				}
				else if ( pMD5->m_nValue == Hashes::Md5Hash::byteCount * 2 )
				{
					CStringA tmp;
					tmp.Append( (const char*)pMD5->m_pValue, (int)pMD5->m_nValue );
					pBTFile->m_oMD5.fromString( CA2W( tmp ) );
				}
				else
				{
					return FALSE;
				}
			}

			if ( const CBENode* pTiger = pFile->GetNode( "tiger" ) )
			{
				if ( pTiger->IsType( CBENode::beString ) && pTiger->m_nValue != Hashes::TigerHash::byteCount )
					pBTFile->m_oTiger = *static_cast< Hashes::TigerHash::RawStorage* >( pTiger->m_pValue );
			}

			m_nSize += pBTFile->m_nSize;
			nOffset += pBTFile->m_nSize;

			// Add http web seed
			// ToDo: Handle additonal multi-file http web seeds
			if ( nFile == 0 && ! strURL.IsEmpty() )	// First file only (no offset?)
			{
				if ( strURL.Right( 1 ) == L'/' )
					strURL += PathFindFileName( pBTFile->m_sName );
				m_sURLs.AddTail( strURL );
			}

			m_pFiles.AddTail( pBTFile.Detach() );
		}

		if ( nFiles == 1 )
		{
			// Single file in a multi-file torrent
			CBTFile* pSingleFile = m_pFiles.GetHead();

			// Reset the name
			m_sName = strPath;

			// Add http web seed (added above)
			//if ( ! strURL.IsEmpty )
			//{
			//	if ( strURL.Right( 1 ) == L'/' )
			//		strURL += PathFindFileName( m_sName );
			//	m_sURLs.AddTail( strURL );
			//}

			// Set data/file hashes (if they aren't)
			if ( pSingleFile->m_oSHA1 )
				m_oSHA1 = pSingleFile->m_oSHA1;
			else if ( m_oSHA1 )
				pSingleFile->m_oSHA1 = m_oSHA1;

			if ( pSingleFile->m_oED2K )
				m_oED2K = pSingleFile->m_oED2K;
			else if ( m_oED2K )
				pSingleFile->m_oED2K = m_oED2K;

			if ( pSingleFile->m_oMD5 )
				m_oMD5 = pSingleFile->m_oMD5;
			else if ( m_oMD5 )
				pSingleFile->m_oMD5 = m_oMD5;

			if ( pSingleFile->m_oTiger )
				m_oTiger = pSingleFile->m_oTiger;
			else if ( m_oTiger )
				pSingleFile->m_oTiger = m_oTiger;
		}
	}
	else
	{
		return FALSE;
	}

	if ( ( m_nSize + m_nBlockSize - 1 ) / m_nBlockSize != m_nBlockCount )
		return FALSE;

	if ( ! CheckFiles() )
		return FALSE;

	CSHA oSHA = pInfo->GetSHA1();
	oSHA.GetHash( &m_oBTH[ 0 ] );
	m_oBTH.validate();

	if ( m_pSource.m_nLength > 0 && pInfo->m_nSize
		 && pInfo->m_nPosition + pInfo->m_nSize < m_pSource.m_nLength )
	{
		Hashes::BtHash oBTH;
		CSHA pBTH;
		pBTH.Add( &m_pSource.m_pBuffer[pInfo->m_nPosition], pInfo->m_nSize );
		pBTH.Finish();
		pBTH.GetHash( &oBTH[0] );

		if ( oBTH == m_oBTH )
		{
			m_nInfoStart = pInfo->m_nPosition;
			m_nInfoSize  = pInfo->m_nSize;
		}
	}

	return TRUE;
}

BOOL CBTInfo::IsDeadTracker(const CString& sTracker)
{
	// Known common defunct URLs, keep updated  (ToDo: Trackers.dat file?)
	// ToDo: Check DNS headers too - http://bittorrent.org/beps/bep_0034.html
	return
		sTracker.Find( L"piratebay.org", 6 ) > 0 ||
		sTracker.Find( L"publicbt.com", 6 ) > 0 ||
		sTracker.Find( L"demonii.com", 6 ) > 0 ||
		sTracker.Find( L"denis.stalker.h3q.com", 5 ) > 0 ||
		( StartsWith( sTracker, _P( L"http://" ) ) &&
			sTracker.Find( L".1337x.", 8 ) > 8 );
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from tree

BOOL CBTInfo::CheckFiles()
{
	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
	{
		CBTFile* pBTFile = m_pFiles.GetNext( pos );
		pBTFile->m_sPath.Trim();

		LPCTSTR pszPath = pBTFile->m_sPath;

		if ( pszPath == NULL || *pszPath == 0 ) return FALSE;
		if ( pszPath[1] == L':' ) return FALSE;
		if ( *pszPath == L'\\' || *pszPath == L'/' ) return FALSE;
		if ( _tcsstr( pszPath, L"..\\" ) != NULL ) return FALSE;
		if ( _tcsstr( pszPath, L"../" ) != NULL ) return FALSE;
	}

	return m_pFiles.GetCount() > 0;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo block testing

void CBTInfo::BeginBlockTest()
{
	ASSERT( IsAvailable() );
	ASSERT( m_pBlockBTH != NULL );

	m_pTestSHA1.Reset();
	m_nTestByte = 0;
}

void CBTInfo::AddToTest(LPCVOID pInput, DWORD nLength)
{
	if ( nLength == 0 ) return;

	ASSERT( IsAvailable() );
	ASSERT( m_pBlockBTH != NULL );
	ASSERT( m_nTestByte + nLength <= m_nBlockSize );

	m_pTestSHA1.Add( pInput, nLength );
	m_nTestByte += nLength;
}

BOOL CBTInfo::FinishBlockTest(DWORD nBlock)
{
	ASSERT( IsAvailable() );

	if ( m_pBlockBTH == NULL || nBlock >= m_nBlockCount )
		return FALSE;

	Hashes::BtHash oBTH;
	m_pTestSHA1.Finish();
	m_pTestSHA1.GetHash( &oBTH[ 0 ] );
	oBTH.validate();

	return m_pBlockBTH[ nBlock ] == oBTH;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo tracker handling

void CBTInfo::SetTrackerAccess(DWORD tNow)
{
	// Check that there should be a tracker
	if ( m_oTrackers.IsEmpty() ) return;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	// Set the current tracker's access time
	m_oTrackers[ m_nTrackerIndex ].m_tLastAccess = tNow;
}

void CBTInfo::SetTrackerSucceeded(DWORD tNow)
{
	// Check that there should be a tracker
	if ( m_oTrackers.IsEmpty() ) return;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	// Set the current tracker's success time
	m_oTrackers[ m_nTrackerIndex ].m_tLastSuccess = tNow;

	// Reset the failure count
	m_oTrackers[ m_nTrackerIndex ].m_nFailures = 0;
}

void CBTInfo::SetTrackerRetry(DWORD tTime)
{
	// Check that there should be a tracker
	if ( ! HasTracker() ) return;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	// Set the current tracker's next allowable access attempt time
	m_oTrackers[ m_nTrackerIndex ].m_tNextTry = tTime;
}

void CBTInfo::SetTrackerNext(DWORD tTime)
{
	if ( m_oTrackers.IsEmpty() )
	{
		m_nTrackerMode = tNull;
		m_nTrackerIndex = -1;
		return;
	}

	if ( m_nTrackerMode == tNull || m_nTrackerMode == tSingle )
		return;

	// Make sure this is a multitracker torrent
	if ( m_oTrackers.GetCount() < 2 )
	{
		m_nTrackerMode = tSingle;
		m_nTrackerIndex = 0;
		return;
	}

	// Get current time
	if ( ! tTime )
		tTime = GetTickCount();

	// Set current mode to searching
	m_nTrackerMode = tMultiFinding;

	// Start with the first tracker in the list
	m_nTrackerIndex = 0;

	// Search through the list for an available tracker, or the first one that will become available
	for ( int nTracker = 0 ; nTracker < m_oTrackers.GetCount() ; ++nTracker )
	{
		// Check for bad trackers displayed at end of list (but user-added may follow)
		// m_oTrackers[ nTracker ].m_sAddress.GetAt( 0 ) == BAD_TRACKER_TOKEN		// *https://
		if ( ! StartsWith( m_oTrackers[ nTracker ].m_sAddress, _P( L"http://" ) ) &&
			 ! StartsWith( m_oTrackers[ nTracker ].m_sAddress, _P( L"udp://" ) ) )
			continue;	//break;

		// Get the next tracker in the list
		CBTTracker& oTracker = m_oTrackers[ nTracker ];

		// If it's available, reset the retry time
		if ( oTracker.m_tNextTry < tTime )
			oTracker.m_tNextTry = 0;

		// If this tracker will become available before the current one, make it the current tracker
		if ( m_oTrackers[ m_nTrackerIndex ].m_tNextTry > oTracker.m_tNextTry )
			m_nTrackerIndex = nTracker;
	}
}

CString CBTInfo::GetTrackerAddress(int nTrackerIndex) const
{
	if ( m_oTrackers.IsEmpty() )
		return CString();

	if ( nTrackerIndex == -1 )
		nTrackerIndex = m_nTrackerIndex;

	if ( nTrackerIndex == -1 )
		return CString();

	ASSERT( nTrackerIndex >= 0 && nTrackerIndex < m_oTrackers.GetCount() );

	return m_oTrackers[ nTrackerIndex ].m_sAddress;
}

TRISTATE CBTInfo::GetTrackerStatus(int nTrackerIndex) const
{
	if ( m_oTrackers.IsEmpty() )
		return TRI_UNKNOWN;

	if ( nTrackerIndex == -1 )
		nTrackerIndex = m_nTrackerIndex;

	if ( nTrackerIndex == -1 )
		return TRI_UNKNOWN;

	ASSERT( nTrackerIndex >= 0 && nTrackerIndex < m_oTrackers.GetCount() );

	if ( ! m_oTrackers[ nTrackerIndex ].m_tNextTry &&
		 ! m_oTrackers[ nTrackerIndex ].m_tLastSuccess )
		return TRI_UNKNOWN;

	if ( m_oTrackers[ nTrackerIndex ].m_tNextTry >
		m_oTrackers[ nTrackerIndex ].m_tLastSuccess )
		return TRI_FALSE;

	return TRI_TRUE;
}

int CBTInfo::GetTrackerTier(int nTrackerIndex) const
{
	if ( m_oTrackers.IsEmpty() )
		return 0;

	if ( nTrackerIndex == -1 )
		nTrackerIndex = m_nTrackerIndex;

	if ( nTrackerIndex == -1 )
		return 0;

	ASSERT( nTrackerIndex >= 0 && nTrackerIndex < m_oTrackers.GetCount() );

	return m_oTrackers[ nTrackerIndex ].m_nTier;
}

DWORD CBTInfo::GetTrackerNextTry() const
{
	if ( ! HasTracker() )
		return (DWORD)-1;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	return m_oTrackers[ m_nTrackerIndex ].m_tNextTry;
}

DWORD CBTInfo::GetTrackerFailures() const
{
	if ( ! HasTracker() )
		return 0;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	// Return the # of failures
	return m_oTrackers[ m_nTrackerIndex ].m_nFailures;
}

void CBTInfo::OnTrackerFailure()
{
	if ( ! HasTracker() )
		return;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	m_oTrackers[ m_nTrackerIndex ].m_nFailures++;
}

void CBTInfo::SetTracker(const CString& sTracker)
{
	m_nTrackerIndex = AddTracker( CBTTracker( sTracker ) );
}

void CBTInfo::SetNode(const CString& sNode)
{
	m_oNodes.AddTail( sNode );
}

void CBTInfo::SetTrackerMode(int nTrackerMode)
{
	// Check it's valid
	INT_PTR nCount = m_oTrackers.GetCount();
	if ( ( nTrackerMode == CBTInfo::tMultiFound		&& nCount > 1 ) ||
		 ( nTrackerMode == CBTInfo::tMultiFinding	&& nCount > 1 ) ||
		 ( nTrackerMode == CBTInfo::tSingle			&& nCount > 0 ) ||
		   nTrackerMode == CBTInfo::tNull )
	{
		m_nTrackerMode = nTrackerMode;

		if ( nTrackerMode == CBTInfo::tNull )
			m_nTrackerIndex = -1;
		else if ( m_nTrackerIndex == -1 )
			SetTrackerNext();
	}
}

int CBTInfo::AddTracker(const CBTTracker& oTracker)
{
	for ( int i = 0 ; i < (int)m_oTrackers.GetCount() ; ++i )
	{
		if ( m_oTrackers[ i ] == oTracker )
			return i;	// Already have
	}

	return (int)m_oTrackers.Add( oTracker );
}

void CBTInfo::RemoveAllTrackers()
{
	m_nTrackerIndex = -1;
	m_nTrackerMode = CBTInfo::tNull;
	m_oTrackers.RemoveAll();
}

// For Reference, use BTTrackerRequest
//BOOL CBTInfo::ScrapeTracker()
//{
//	ASSUME_NO_LOCK( Transfers.m_pSection );		// Custom Debug !ASSUME_LOCK
//
//	if ( m_tTrackerScrape )
//	{
//		// Support rare min_request_interval flag,  Low default throttle is enough in practice
//		if ( ( GetTickCount() - m_tTrackerScrape ) < m_nTrackerWait )
//			return ( m_nTrackerSeeds > 0 || m_nTrackerPeers > 0 );
//	}
//
//	m_tTrackerScrape = GetTickCount();
//
//	if ( theApp.m_bClosing )
//		return FALSE;
//
//	CString strURL = GetTrackerAddress();
//	if ( ! StartsWith( strURL, _P( L"http://" ) ) )
//		return FALSE;	// ToDo: Support UDP Tracker scrape & handle rare HTTPS?
//
//	if ( strURL.Replace( L"/announce", L"/scrape" ) != 1 )
//		return FALSE;
//
//	// Fetch scrape only for the given info hash
//	strURL = strURL.TrimRight( L'&' ) + ( ( strURL.Find( L'?' ) != -1 ) ? L'&' : L'?' ) + L"info_hash=";
//
//	// m_oBTH must be protected by Transfers.m_pSection
//	CSingleLock oLock( &Transfers.m_pSection );
//	if ( ! oLock.Lock( 500 ) ) return FALSE;
//
//	strURL += CBTTrackerRequest::Escape( m_oBTH );
//		// + L"&peer_id=" + CBTTrackerRequest::Escape( pDownload.m_pPeerID ); 	// ToDo: Is this needed?
//
//	LPBYTE nKey = &m_oBTH[ 0 ];
//
//	oLock.Unlock();
//
//	CHttpRequest pRequest;
//	pRequest.SetURL( strURL );
//	pRequest.AddHeader( L"Accept-Encoding", L"deflate, gzip" );
//	pRequest.EnableCookie( false );
//
//	// Don't wait for thread
//	if ( ! pRequest.Execute( TRUE ) || ! pRequest.InflateResponse() )
//		return FALSE;
//
//	CBuffer* pResponse = pRequest.GetResponseBuffer();
//	if ( pResponse == NULL || pResponse->m_pBuffer == NULL )
//		return FALSE;
//
//	if ( const CBENode* pNode = CBENode::Decode( pResponse ) )
//	{
//		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, L"[BT] Received BitTorrent tracker response: %s", pNode->Encode() );
//
//		//if ( ! oLock.Lock( 500 ) ) return FALSE;
//		//LPBYTE nKey = &m_oBTH[ 0 ];		// Above
//		//oLock.Unlock();
//
//		CBENode* pFiles = pNode->GetNode( "files" );
//		CBENode* pFile = pFiles->GetNode( nKey, Hashes::BtHash::byteCount );
//		if ( ! pFile->IsType( CBENode::beDict ) ) return FALSE;
//
//		if ( const CBENode* pSeeds = pFile->GetNode( "complete" ) )
//		{
//			if ( pSeeds->IsType( CBENode::beInt ) )
//				m_nTrackerSeeds = (int)( pSeeds->GetInt() & ~0xFFFF0000 );		// QWORD Caution: Don't get negative values from buggy trackers
//		}
//
//		if ( const CBENode* pPeers = pFile->GetNode( "incomplete" ) )
//		{
//			if ( pPeers->IsType( CBENode::beInt ) )
//				m_nTrackerPeers = (int)( pPeers->GetInt() & ~0xFFFF0000 );
//		}
//
//		//if ( const CBENode* pHistory = pFile->GetNode( "downloaded" ) )		// ToDo: Use stat of all completed downloads ?
//		//{
//		//	if ( pHistory->IsType( CBENode::beInt ) )
//		//		m_nTrackerHistory = (int)( pHistory->GetInt() & ~0xFFFF0000 );
//		//}
//
//		// Unofficial min_request_interval
//		if ( m_nTrackerWait < 200 * 1000 )
//		{
//			if ( const CBENode* pFlags = pNode->GetNode( "flags" ) )
//			{
//				if ( const CBENode* pWait = pFlags->GetNode( "min_request_interval" ) )
//				{
//					if ( pWait->IsType( CBENode::beInt ) )
//						m_nTrackerWait = pWait->GetInt() * 1000;
//
//					if ( m_nTrackerWait < 60 * 1000 )
//						m_nTrackerWait = 90 * 1000;
//					else if ( m_nTrackerWait > 6 * 60 * 60 * 1000 )
//						m_nTrackerWait = 30 * 60 * 1000;
//				}
//			}
//		}
//
//		delete pNode;
//	}
//
//	return ( m_nTrackerSeeds > 0 || m_nTrackerPeers > 0 );
//}

CString CBTInfo::GetTrackerHash() const
{
	// Produce encoded tracker list for LT_TEX extension.
	CStringA sAddress;

	// Get concatenated tracker URLs list sorted in alphabetical order
	string_set oAddr;
	const int nCount = (int)m_oTrackers.GetCount();
	for ( int i = 0 ; i < nCount ; ++i )
	{
		oAddr.insert( m_oTrackers[ i ].m_sAddress );
	}
	for ( string_set::const_iterator i = oAddr.begin() ; i != oAddr.end() ; i++ )
	{
		sAddress += CT2A( (*i) );
	}

	// Get SHA1 of it
	CSHA oSHA;
	oSHA.Add( (LPCSTR)sAddress, sAddress.GetLength() );
	oSHA.Finish();

	Hashes::Sha1Hash oSHA1;
	oSHA.GetHash( &oSHA1[ 0 ] );
	oSHA1.validate();

	// Return hex-encoded hash
	return oSHA1.toString< Hashes::base16Encoding >();
}


//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTTracker construction

CBTInfo::CBTTracker::CBTTracker(LPCTSTR szAddress, INT nTier)
	: m_sAddress		( szAddress ? szAddress : L"" )
	, m_tLastAccess		( 0 )
	, m_tLastSuccess	( 0 )
	, m_tNextTry		( 0 )
	, m_nFailures		( 0 )
	, m_nType			( 0 )
	, m_nTier			( nTier )
{
}

CBTInfo::CBTTracker::CBTTracker(const CBTTracker& oSource)
	: m_sAddress		( oSource.m_sAddress )
	, m_tLastAccess		( oSource.m_tLastAccess )
	, m_tLastSuccess	( oSource.m_tLastSuccess )
	, m_tNextTry		( oSource.m_tNextTry )
	, m_nFailures		( oSource.m_nFailures )
	, m_nType			( oSource.m_nType )
	, m_nTier			( oSource.m_nTier )
{
}

CBTInfo::CBTTracker& CBTInfo::CBTTracker::operator=(const CBTTracker& oSource)
{
	m_sAddress		= oSource.m_sAddress;
	m_tLastAccess	= oSource.m_tLastAccess;
	m_tLastSuccess	= oSource.m_tLastSuccess;
	m_tNextTry		= oSource.m_tNextTry;
	m_nFailures		= oSource.m_nFailures;
	m_nType			= oSource.m_nType;
	m_nTier 		= oSource.m_nTier;

	return *this;
}

bool CBTInfo::CBTTracker::operator==(const CBTTracker& oSource)
{
	return ( m_sAddress == oSource.m_sAddress );
}


//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTTracker serialize

void CBTInfo::CBTTracker::Serialize(CArchive& ar, int /*nVersion*/)
{
	if ( ar.IsStoring() )
	{
		ar << m_sAddress;
		ar << m_tLastAccess;
		ar << m_tLastSuccess;
		ar << m_tNextTry;
		ar << m_nFailures;
		ar << m_nTier;
		ar << m_nType;
	}
	else // Loading
	{
		ar >> m_sAddress;
		ar >> m_tLastAccess;
		ar >> m_tLastSuccess;
		ar >> m_tNextTry;
		ar >> m_nFailures;
		ar >> m_nTier;
		ar >> m_nType;
	}
}
