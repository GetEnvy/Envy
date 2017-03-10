//
// LocalSearch.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
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
#include "LocalSearch.h"
#include "QuerySearch.h"
#include "QueryHit.h"

#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"

#include "EnvyURL.h"
#include "GProfile.h"
#include "Network.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "DCNeighbour.h"
#include "Datagrams.h"
#include "GGEP.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "DCPacket.h"
#include "BTClients.h"

#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "Uploads.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "ImageServices.h"

#include "Buffer.h"
#include "ZLib.h"
#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CLocalSearch construction

CLocalSearch::CLocalSearch(CQuerySearch* pSearch, PROTOCOLID nProtocol)
	: m_pSearch		( pSearch )
	, m_pEndpoint	( pSearch->m_pEndpoint )
	, m_pBuffer		( NULL )
	, m_bUDP		( TRUE )
	, m_nProtocol	( nProtocol )	// PROTOCOL_G2
{
}

CLocalSearch::CLocalSearch(CQuerySearch* pSearch, const CNeighbour* pNeighbour)
	: m_pSearch		( pSearch )
	, m_pEndpoint	( pNeighbour->m_pHost )
	, m_pBuffer		( NULL )
	, m_bUDP		( FALSE )
	, m_nProtocol	( pNeighbour->m_nProtocol )
{
}

CLocalSearch::CLocalSearch(CBuffer* pBuffer, PROTOCOLID nProtocol)
	: m_pSearch		( NULL )
	, m_pEndpoint	( )
	, m_pBuffer		( pBuffer )
	, m_bUDP		( FALSE )
	, m_nProtocol	( nProtocol )
{
}


//////////////////////////////////////////////////////////////////////
// CLocalSearch

template<>
bool CLocalSearch::IsValidForHit< CDownload >(const CDownload* pDownload) const
{
	return	// Download is shareable, active, and matches:
		pDownload->IsShared() &&
		( pDownload->IsTorrent() || pDownload->IsStarted() ) &&
		m_pSearch->Match( pDownload->GetSearchName(), NULL, NULL, pDownload );
}

template<>
bool CLocalSearch::IsValidForHit< CLibraryFile >(const CLibraryFile* pFile) const
{
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		// Browse request, or real file (not ghost)
		return Settings.Gnutella1.Enabled &&
			( ! m_pSearch || pFile->IsAvailable() );
	case PROTOCOL_G2:
		// Browse request, or comments request, or real file (not ghost)
		return Settings.Gnutella2.Enabled &&
			( ! m_pSearch || m_pSearch->m_bWantCOM || pFile->IsAvailable() );
	case PROTOCOL_DC:
		// Real file (not ghost)
		return Settings.DC.Enabled &&
			pFile->IsAvailable();
	default:
		ASSERT( FALSE );
		return false;
	}
}

// Note IsValidForHit for PROTOCOL_G1:
// Check that a free queue exists that can upload this file.
//&& ( UploadQueues.QueueRank( PROTOCOL_HTTP, pFile ) <= Settings.Gnutella1.HitQueueLimit );  // Causes Deadlock?
// NOTE: Very CPU intensive operation!
// Normally this isn't a problem, default queue length is 8-10, so this check (50) will never be activated.
// However, sometimes users configure bad settings, such as a 2000 user HTTP queue.
// Although the remote client should handle this by itself,
// give Gnutella some protection against extreme settings (to reduce un-necessary traffic).


//////////////////////////////////////////////////////////////////////
// CLocalSearch execute

bool CLocalSearch::Execute(INT_PTR nMaximum, bool bPartial /*true*/, bool bShared /*true*/)
{
	ASSERT( bPartial || bShared );

	if ( nMaximum < 0 )
		nMaximum = Settings.Gnutella.MaxHits;

	if ( m_pSearch )
		m_oGUID = m_pSearch->m_oGUID;
	else
		Network.CreateID( m_oGUID );

	INT_PTR nHits = 0;
	if ( bPartial )
	{
		if ( ! ExecutePartialFiles( nMaximum, nHits ) )
			return false;
	}

	if ( bShared && ( ! nMaximum || nHits < nMaximum ) )
	{
		if ( ! ExecuteSharedFiles( nMaximum, nHits ) )
			return false;
	}

	ASSERT( ! nMaximum || nHits <= nMaximum );

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch execute partial files

bool CLocalSearch::ExecutePartialFiles(INT_PTR nMaximum, INT_PTR& nHits)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 150 ) )
		return false;

	// Browse request, or no partials requested, or non Gnutella 2 request
	if ( ! m_pSearch || ! m_pSearch->m_bWantPFS || m_nProtocol != PROTOCOL_G2 )
		return true;

	CList< CDownload* > oFilesInPacket;

	for ( POSITION pos = Downloads.GetIterator() ;
		pos && ( ! nMaximum || ( nHits + oFilesInPacket.GetCount() < nMaximum ) ) ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( IsValidForHit( pDownload ) )
			oFilesInPacket.AddTail( pDownload );

		// Obsolete for reference:
		//	if ( ( Settings.Gnutella.HitsPerPacket && (DWORD)oFilesInPacket.GetCount() >= Settings.Gnutella.HitsPerPacket ) ||
		//		( m_pPacket && m_pPacket->m_nLength >= MAX_QUERY_PACKET_SIZE ) )
		//	{
		//		nHits += SendHits( oFilesInPacket );	// Packet full, send it
		//		oFilesInPacket.RemoveAll();
		//	}
	}

	SendHits( oFilesInPacket );

	nHits += oFilesInPacket.GetCount();

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch execute shared files

bool CLocalSearch::ExecuteSharedFiles(INT_PTR nMaximum, INT_PTR& nHits)
{
	CSingleLock oLock( &Library.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return false;

	// Is it a browser request?		(ToDo: PrivateKey?)
	if ( ! m_pSearch && m_nProtocol == PROTOCOL_G2 && Settings.Gnutella2.Enabled )
	{
		// Send virtual tree
		DispatchPacket( AlbumToPacket( Library.GetAlbumRoot() ) );

		// Send physical tree
		DispatchPacket( FoldersToPacket() );
	}

	augment::auto_ptr< CFileList > pFiles( Library.Search( m_pSearch, (int)nMaximum, FALSE, m_nProtocol != PROTOCOL_G2 ) );	// Ghost files only for G2

	if ( pFiles.get() )
	{
		CFileList oFilesInPacket;

		for ( POSITION pos = pFiles->GetHeadPosition() ; pos && ( ! nMaximum || nMaximum > nHits + oFilesInPacket.GetCount() ) ; )
		{
			CLibraryFile* pFile = pFiles->GetNext( pos );

			// Select valid files
			if ( IsValidForHit( pFile ) )
				oFilesInPacket.AddTail( pFile );

			// Obsolete for reference:
			//if ( ( Settings.Gnutella.HitsPerPacket && (DWORD)oFilesInPacket.GetCount() >= Settings.Gnutella.HitsPerPacket ) ||
			//	 ( m_pPacket && m_pPacket->m_nLength >= MAX_QUERY_PACKET_SIZE ) )
			//{
			//	nHits += SendHits( oFilesInPacket );	// Packet full, send it
			//	oFilesInPacket.RemoveAll();
			//}
		}

		SendHits( oFilesInPacket );

		nHits += oFilesInPacket.GetCount();
	}

	return true;
}

template< typename T >
void CLocalSearch::SendHits(const CList< T* >& oFiles)
{
	CPacket* pPacket = NULL;
	CSchemaMap pSchemas;

	BYTE nHits = 0;
	for ( POSITION pos = oFiles.GetHeadPosition() ; pos ; )
	{
		if ( ! pPacket )
			pPacket = CreatePacket();

		AddHit( pPacket, pSchemas, oFiles.GetNext( pos ), nHits++ );

		// Send full packet
		if ( m_nProtocol == PROTOCOL_DC ||		// One hit per packet in DC++ protocol
			nHits >= Settings.Gnutella.HitsPerPacket ||
			pPacket->m_nLength >= MAX_QUERY_PACKET_SIZE )
		{
			WriteTrailer( pPacket, pSchemas, nHits );
			DispatchPacket( pPacket );
			pPacket = NULL;
			nHits = 0;
		}
	}

	if ( nHits )
	{
		WriteTrailer( pPacket, pSchemas, nHits );
		DispatchPacket( pPacket );
		pPacket = NULL;
	}

	ASSERT( pPacket == NULL );
	ASSERT( pSchemas.IsEmpty() );
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch add library file hit

template<>
void CLocalSearch::AddHit< CLibraryFile >(CPacket* pPacket, CSchemaMap& pSchemas, CLibraryFile* pFile, int nIndex)
{
	ASSERT( pPacket != NULL );

	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		AddHitG1( static_cast< CG1Packet* >( pPacket ), pSchemas, pFile, nIndex );
		break;
	case PROTOCOL_G2:
		AddHitG2( static_cast< CG2Packet* >( pPacket ), pSchemas, pFile, nIndex );
		break;
	case PROTOCOL_DC:
		AddHitDC( static_cast< CDCPacket* >( pPacket ), pSchemas, pFile, nIndex );
		break;
	default:
		ASSERT( FALSE );
	}
}

void CLocalSearch::AddHitG1(CG1Packet* pPacket, CSchemaMap& pSchemas, CLibraryFile* pFile, int nIndex)
{
	const QWORD nFileSize = pFile->GetSize();

	pPacket->WriteLongLE( pFile->m_nIndex );
	pPacket->WriteLongLE( min( (DWORD)nFileSize, (DWORD)0xFFFFFFFF ) );
	if ( Settings.Gnutella1.QueryHitUTF8 )		// Support UTF-8 Query
		pPacket->WriteStringUTF8( pFile->m_sName );
	else
		pPacket->WriteString( pFile->m_sName );

	if ( pFile->m_oSHA1 && pFile->m_oTiger )
	{
		pPacket->WriteString( L"urn:bitprint:" + pFile->m_oSHA1.toString() + L'.' + pFile->m_oTiger.toString(), FALSE );
		pPacket->WriteByte( G1_PACKET_HIT_SEP );
	}
	else if ( pFile->m_oSHA1 )
	{
		pPacket->WriteString( L"urn:sha1:" + pFile->m_oSHA1.toString(), FALSE );
		pPacket->WriteByte( G1_PACKET_HIT_SEP );
	}
	else if ( pFile->m_oTiger )
	{
		pPacket->WriteString( L"urn:ttroot:" + pFile->m_oTiger.toString(), FALSE );
		pPacket->WriteByte( G1_PACKET_HIT_SEP );
	}

	if ( pFile->m_oED2K )
	{
		pPacket->WriteString( pFile->m_oED2K.toUrn(), FALSE );
		pPacket->WriteByte( G1_PACKET_HIT_SEP );
	}
	if ( pFile->m_oMD5 )
	{
		pPacket->WriteString( pFile->m_oMD5.toUrn(), FALSE );
		pPacket->WriteByte( G1_PACKET_HIT_SEP );
	}
	if ( pFile->m_oBTH )
	{
		pPacket->WriteString( pFile->m_oBTH.toUrn(), FALSE );
		pPacket->WriteByte( G1_PACKET_HIT_SEP );
	}

	if ( Settings.Gnutella1.EnableGGEP )
	{
		CGGEPBlock pBlock;
		if ( nFileSize >= 0xFFFFFFFF )
		{
			if ( CGGEPItem* pItem = pBlock.Add( GGEP_HEADER_LARGE_FILE ) )
			{
				pItem->WriteInt64( nFileSize );
			}
		}

		if ( pFile->m_pSources.GetCount() )
		{
			if ( CGGEPItem* pItem = pBlock.Add( GGEP_HEADER_ALTS ) )
			{
				for ( POSITION pos = pFile->m_pSources.GetHeadPosition() ; pos ; )
				{
					CSharedSource* pSource = pFile->m_pSources.GetNext( pos );
					CEnvyURL oURL( pSource->m_sURL );
					if ( oURL.m_pAddress.s_addr && oURL.m_nPort &&
						! Network.IsSelfIP( oURL.m_pAddress ) )
					{
						pItem->WriteLong( oURL.m_pAddress.s_addr );
						pItem->WriteShort( oURL.m_nPort );
					}
				}
			}
		}

		// Network wide file creation time (seconds)
		if ( DWORD nCreationTime = pFile->GetCreationTime() )
		{
			if ( CGGEPItem* pItem = pBlock.Add( GGEP_HEADER_CREATE_TIME ) )
			{
				pItem->Write( &nCreationTime, 4 );
			}
		}

		pBlock.Write( pPacket );
	}

	// End of file
	pPacket->WriteByte( 0 );

	// Add Metadata XML
	if ( pFile->m_pSchema && pFile->m_pMetadata &&
		( ! m_pSearch || m_pSearch->m_bWantXML ) )
	{
		CXMLElement* pGroup = NULL;
		if ( ! pSchemas.Lookup( pFile->m_pSchema, pGroup ) )
		{
			pGroup = pFile->m_pSchema->Instantiate();
			if ( ! pGroup )
				return;
			pSchemas.SetAt( pFile->m_pSchema, pGroup );
		}

		if ( CXMLElement* pXML = pFile->m_pMetadata->Clone() )
		{
			CString strIndex;
			strIndex.Format( L"%d", nIndex );
			pXML->AddAttribute( L"index", strIndex );
			pGroup->AddElement( pXML );
		}
	}
}

void CLocalSearch::AddHitG2(CG2Packet* pPacket, CSchemaMap& /*pSchemas*/, CLibraryFile* pFile, int /*nIndex*/)
{
	// Pass 1: Calculate child group size
	// Pass 2: Write the child packet
	DWORD nGroup = 0;
	bool bCalculate = false;
	do
	{
		bCalculate = ! bCalculate;

		if ( ! bCalculate )
			pPacket->WritePacket( G2_PACKET_HIT_DESCRIPTOR, nGroup, TRUE );

		if ( pFile->m_oTiger && pFile->m_oSHA1 )
		{
			const char prefix[] = "bp";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oSHA1 );
				pPacket->Write( pFile->m_oTiger );
			}
		}
		else if ( pFile->m_oTiger )
		{
			const char prefix[] = "ttr";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::TigerHash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::TigerHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oTiger );
			}
		}
		else if ( pFile->m_oSHA1 )
		{
			const char prefix[] = "sha1";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oSHA1 );
			}
		}

		if ( pFile->m_oED2K )
		{
			const char prefix[] = "ed2k";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Ed2kHash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Ed2kHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oED2K );
			}
		}

		if ( pFile->m_oBTH )
		{
			const char prefix[] = "btih";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::BtHash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::BtHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oBTH );
			}
		}

		if ( pFile->m_oMD5 )
		{
			const char prefix[] = "md5";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Md5Hash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Md5Hash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oMD5 );
			}
		}

		if ( ! m_pSearch || m_pSearch->m_bWantDN )
		{
			if ( pFile->GetSize() <= 0xFFFFFFFF )
			{
				if ( bCalculate )
				{
					nGroup += G2_PACKET_LEN( G2_PACKET_DESCRIPTIVE_NAME, sizeof( DWORD ) + pPacket->GetStringLen( pFile->m_sName ) );
				}
				else
				{
					pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, sizeof( DWORD ) + pPacket->GetStringLen( pFile->m_sName ) );
					pPacket->WriteLongBE( (DWORD)pFile->GetSize() );
					pPacket->WriteString( pFile->m_sName, FALSE );
				}
			}
			else // size = 0xFFFFFFFF
			{
				if ( bCalculate )
				{
					nGroup += G2_PACKET_LEN( G2_PACKET_SIZE, sizeof( QWORD ) ) +
						G2_PACKET_LEN( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pFile->m_sName ) );
				}
				else
				{
					pPacket->WritePacket( G2_PACKET_SIZE, sizeof( QWORD ) );
					pPacket->WriteInt64( pFile->GetSize() );
					pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pFile->m_sName ) );
					pPacket->WriteString( pFile->m_sName, FALSE );
				}
			}

			if ( LPCTSTR pszType = _tcsrchr( pFile->m_sName, '.' ) )
			{
				if ( _tcsicmp( pszType, L".co" ) == 0 ||
					 _tcsicmp( pszType, L".collection" ) == 0 ||
					 _tcsicmp( pszType, L".emulecollection" ) == 0 )
				{
					if ( ! pFile->m_bBogus )
					{
						if ( bCalculate )
							nGroup += G2_PACKET_LEN( G2_PACKET_COLLECTION, 0 );
						else
							pPacket->WritePacket( G2_PACKET_COLLECTION, 0 );
					}
				}
			}
		}

		if ( ! m_pSearch || m_pSearch->m_bWantURL )
		{
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URL, 0 );
			else
				pPacket->WritePacket( G2_PACKET_URL, 0 );

			if ( INT_PTR nCount = pFile->m_pSources.GetCount() )
			{
				if ( bCalculate )
				{
					nGroup += G2_PACKET_LEN( G2_PACKET_CACHED_SOURCES, sizeof( WORD ) );
				}
				else
				{
					pPacket->WritePacket( G2_PACKET_CACHED_SOURCES, sizeof( WORD ) );
					pPacket->WriteShortBE( (WORD)nCount );
				}
			}

			if ( Settings.Uploads.SharePreviews &&
				( pFile->m_bCachedPreview || ( Settings.Uploads.DynamicPreviews &&
				CImageServices::IsFileViewable( (LPCTSTR)pFile->m_sName ) ) ) )
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_PREVIEW_URL, 0 );
				else
					pPacket->WritePacket( G2_PACKET_PREVIEW_URL, 0 );
			}
		}

		if ( pFile->m_pMetadata != NULL && ( ! m_pSearch || m_pSearch->m_bWantXML ) )
		{
			CString strMetadata = pFile->m_pMetadata->ToString();
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_METADATA, pPacket->GetStringLen( strMetadata ) );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_METADATA, pPacket->GetStringLen( strMetadata ) );
				pPacket->WriteString( strMetadata, FALSE );
			}
		}

		{
			CQuickLock pQueueLock( UploadQueues.m_pSection );
			CUploadQueue* pQueue = UploadQueues.SelectQueue( PROTOCOL_HTTP, pFile );
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_GROUP_ID, sizeof( BYTE ) );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_GROUP_ID, sizeof( BYTE ) );
				pPacket->WriteByte( BYTE( pQueue ? pQueue->m_nIndex + 1 : 0 ) );
			}
		}

		if ( ! m_pSearch || m_pSearch->m_bWantCOM )
		{
			if ( pFile->IsRated() )
			{
				CString strComment;
				if ( pFile->m_nRating > 0 )
					strComment.Format( L"<comment rating=\"%i\">", pFile->m_nRating - 1 );
				else
					strComment = L"<comment>";
				strComment += Escape( pFile->m_sComments );
				if ( strComment.GetLength() > 2048 ) strComment = strComment.Left( 2048 );
				strComment += L"</comment>";
				strComment.Replace( L"\r\n", L"{n}" );
				if ( bCalculate )
				{
					nGroup += G2_PACKET_LEN( G2_PACKET_COMMENT, pPacket->GetStringLen( strComment ) );
				}
				else
				{
					pPacket->WritePacket( G2_PACKET_COMMENT, pPacket->GetStringLen( strComment ) );
					pPacket->WriteString( strComment, FALSE );
				}
			}

			if ( pFile->m_bBogus )
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_BOGUS, 0 );
				else
					pPacket->WritePacket( G2_PACKET_BOGUS, 0 );
			}
		}

		if ( ! m_pSearch )
		{
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_OBJECT_ID, sizeof( DWORD ) );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_OBJECT_ID, sizeof( DWORD ) );
				pPacket->WriteLongBE( pFile->m_nIndex );
			}
		}
	}
	while ( bCalculate );
}

void CLocalSearch::AddHitDC(CDCPacket* pPacket, CSchemaMap& /*pSchemas*/, CLibraryFile* pFile, int /*nIndex*/)
{
	// Active user / Passive user:
	// $SR Nick FileName<0x05>FileSize FreeSlots/TotalSlots<0x05>HubName (HubIP:HubPort)|
	// $SR Nick FileName<0x05>FileSize FreeSlots/TotalSlots<0x05>HubName (HubIP:HubPort)<0x05>User|

	if ( ! m_pSearch )
		return;

	CUploadQueue* pQueue = UploadQueues.SelectQueue(
		PROTOCOL_DC, NULL, 0, CUploadQueue::ulqBoth, NULL );
	int nTotalSlots = pQueue ? pQueue->m_nMaxTransfers : 0;
	int nActiveSlots = pQueue ? pQueue->GetActiveCount() : 0;
	int nFreeSlots = nTotalSlots > nActiveSlots ? ( nTotalSlots - nActiveSlots ) : 0;

	CString strHubName;
	if ( pFile->m_oTiger )	// It's TTH search
		strHubName = L"TTH:" + pFile->m_oTiger.toString();
	else
		strHubName = m_pSearch->m_sMyHub;

	CBuffer pAnswer;
	pAnswer.Add( _P("$SR ") );
	pAnswer.Print( m_pSearch->m_sMyNick );
	pAnswer.Add( _P(" ") );
	pAnswer.Print( pFile->m_sName );
	pAnswer.Add( _P("\x05") );
	CString strSize;
	strSize.Format( L"%I64u %d/%d", pFile->m_nSize, nFreeSlots, nTotalSlots );
	pAnswer.Print( strSize );
	pAnswer.Add( _P("\x05") );
	pAnswer.Print( strHubName );
	pAnswer.Add( _P(" (") );
	pAnswer.Print( HostToString( &m_pSearch->m_pMyHub ) );
	pAnswer.Add( _P(")") );
	if ( ! m_pSearch->m_bUDP )
	{
		pAnswer.Add( _P("\x05") );
		pAnswer.Print( m_pSearch->m_sUserNick );
	}
	pAnswer.Add( _P("|") );

	pPacket->Write( pAnswer.m_pBuffer, pAnswer.m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch add download hit

template<>
void CLocalSearch::AddHit< CDownload >(CPacket* pPacket, CSchemaMap& pSchemas, CDownload* pDownload, int nIndex)
{
	ASSERT( pPacket != NULL );

	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		AddHitG1( static_cast< CG1Packet* >( pPacket ), pSchemas, pDownload, nIndex );
		break;
	case PROTOCOL_G2:
		AddHitG2( static_cast< CG2Packet* >( pPacket ), pSchemas, pDownload, nIndex );
		break;
	case PROTOCOL_DC:
		AddHitDC( static_cast< CDCPacket* >( pPacket ), pSchemas, pDownload, nIndex );
		break;
	default:
		ASSERT( FALSE );
	}
}

void CLocalSearch::AddHitG1(CG1Packet* /*pPacket*/, CSchemaMap& /*pSchemas*/, CDownload* /*pDownload*/, int /*nIndex*/)
{
	// ToDo: Add Gnutella(1) active-download hit packet!
}

void CLocalSearch::AddHitG2(CG2Packet* pPacket, CSchemaMap& /*pSchemas*/, CDownload* pDownload, int /*nIndex*/)
{
	// Pass 1: Calculate child group size
	// Pass 2: Write the child packet
	DWORD nGroup = 0;
	bool bCalculate = false;
	do
	{
		bCalculate = ! bCalculate;

		if ( ! bCalculate )
			pPacket->WritePacket( G2_PACKET_HIT_DESCRIPTOR, nGroup, TRUE );

		if ( pDownload->m_oTiger && pDownload->m_oSHA1 )
		{
			const char prefix[] = "bp";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oSHA1 );
				pPacket->Write( pDownload->m_oTiger );
			}
		}
		else if ( pDownload->m_oTiger )
		{
			const char prefix[] = "ttr";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::TigerHash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::TigerHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oTiger );
			}
		}
		else if ( pDownload->m_oSHA1 )
		{
			const char prefix[] = "sha1";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oSHA1 );
			}
		}

		if ( pDownload->m_oED2K )
		{
			const char prefix[] = "ed2k";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Ed2kHash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Ed2kHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oED2K );
			}
		}

		if ( pDownload->m_oBTH )
		{
			const char prefix[] = "btih";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::BtHash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::BtHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oBTH );
			}
		}

		if ( pDownload->m_oMD5 )
		{
			const char prefix[] = "md5";
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Md5Hash::byteCount );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Md5Hash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oMD5 );
			}
		}

		if ( m_pSearch->m_bWantDN )
		{
			if ( pDownload->m_nSize <= 0xFFFFFFFF )
			{
				if ( bCalculate )
				{
					nGroup += G2_PACKET_LEN( G2_PACKET_DESCRIPTIVE_NAME, sizeof( DWORD ) + pPacket->GetStringLen( pDownload->m_sName ) );
				}
				else
				{
					pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, sizeof( DWORD ) + pPacket->GetStringLen( pDownload->m_sName ) );
					pPacket->WriteLongBE( (DWORD)pDownload->m_nSize );
					pPacket->WriteString( pDownload->m_sName, FALSE );
				}
			}
			else // Size > 0xFFFFFFFF
			{
				if ( bCalculate )
				{
					nGroup += G2_PACKET_LEN( G2_PACKET_SIZE, sizeof( QWORD ) ) +
						G2_PACKET_LEN( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pDownload->m_sName ) );
				}
				else
				{
					pPacket->WritePacket( G2_PACKET_SIZE, sizeof( QWORD ) );
					pPacket->WriteInt64( pDownload->m_nSize );
					pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pDownload->m_sName ) );
					pPacket->WriteString( pDownload->m_sName, FALSE );
				}
			}
		}

		if ( m_pSearch->m_bWantURL )
		{
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URL, 0 );
			else
				pPacket->WritePacket( G2_PACKET_URL, 0 );
		}

		QWORD nComplete = pDownload->GetVolumeComplete();
		if ( nComplete <= 0xFFFFFFFF )
		{
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_PARTIAL, sizeof( DWORD ) );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_PARTIAL, sizeof( DWORD ) );
				pPacket->WriteLongBE( (DWORD)nComplete );
			}
		}
		else // Downloaded size > 0xFFFFFFFF
		{
			if ( bCalculate )
			{
				nGroup += G2_PACKET_LEN( G2_PACKET_PARTIAL, sizeof( QWORD ) );
			}
			else
			{
				pPacket->WritePacket( G2_PACKET_PARTIAL, sizeof( QWORD ) );
				pPacket->WriteInt64( nComplete );
			}
		}
	}
	while ( bCalculate );
}

void CLocalSearch::AddHitDC(CDCPacket* /*pPacket*/, CSchemaMap& /*pSchemas*/, CDownload* /*pDownload*/, int /*nIndex*/)
{
	// ToDo: Add DC++ active-download hit packet
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch create packet

CPacket* CLocalSearch::CreatePacket()
{
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		return static_cast< CPacket* >( CreatePacketG1() );
	case PROTOCOL_G2:
		return static_cast< CPacket* >( CreatePacketG2() );
	case PROTOCOL_DC:
		return static_cast< CPacket* >( CreatePacketDC() );
	default:
		ASSERT( FALSE );
	}
	return NULL;
}

CG1Packet* CLocalSearch::CreatePacketG1()
{
	DWORD nTTL = m_bUDP ? 1 :
		( m_pSearch ? ( m_pSearch->m_nHops + 2 ) : Settings.Gnutella1.SearchTTL );

	CG1Packet* pPacket = CG1Packet::New( G1_PACKET_HIT, nTTL, m_oGUID );

	pPacket->WriteByte( 0 );	// Hit count will be set latter
	pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.s_addr );

	if ( Uploads.m_bStable )
		pPacket->WriteLongLE( Uploads.m_nBestSpeed * 8 / 1024 );
	else
		pPacket->WriteLongLE( Settings.Connection.OutSpeed );

	return pPacket;
}

CG2Packet* CLocalSearch::CreatePacketG2()
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_HIT, TRUE );

	pPacket->WritePacket( G2_PACKET_NODE_GUID, Hashes::Guid::byteCount );
	pPacket->Write( Hashes::Guid( MyProfile.oGUID ) );

	pPacket->WritePacket( G2_PACKET_NODE_ADDRESS, 6 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.s_addr );
	pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );

	pPacket->WritePacket( G2_PACKET_VENDOR, 4 );
	pPacket->WriteString( VENDOR_CODE, FALSE );

	if ( Network.IsFirewalled() )
		pPacket->WritePacket( G2_PACKET_PEER_FIREWALLED, 0 );

	{
		CSingleLock pNetworkLock( &Network.m_pSection );
		if ( pNetworkLock.Lock( 50 ) )
		{
			for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
			{
				CNeighbour* pNeighbour = Neighbours.GetNext( pos );

				if ( pNeighbour->m_nNodeType != ntLeaf &&
					 pNeighbour->m_nProtocol == PROTOCOL_G2 )
				{
					pPacket->WritePacket( G2_PACKET_NEIGHBOUR_HUB, 6 );
					pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.s_addr );
					pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );
				}
			}
		}
	}

	if ( ! Uploads.m_bStable )
		pPacket->WritePacket( G2_PACKET_PEER_UNSTABLE, 0 );

	CSingleLock pQueueLock( &UploadQueues.m_pSection );
	if ( pQueueLock.Lock( 2000 ) )
	{
		int nQueue = 1;
		for ( POSITION pos = UploadQueues.GetIterator() ; pos ; nQueue++ )
		{
			CUploadQueue* pQueue = UploadQueues.GetNext( pos );
			pPacket->WritePacket( G2_PACKET_HIT_GROUP, ( 4 + 7 ) + 2, TRUE );
			pPacket->WritePacket( G2_PACKET_PEER_STATUS, 7 );
			pPacket->WriteShortBE( WORD( pQueue->GetQueuedCount() + pQueue->GetTransferCount() ) );
			pPacket->WriteByte( BYTE( pQueue->GetTransferCount( TRUE ) ) );
			pPacket->WriteLongBE( pQueue->GetPredictedBandwidth() * 8 / 1024 );
			pPacket->WriteByte( 0 );
			pPacket->WriteByte( BYTE( nQueue ) );
		}

		pQueueLock.Unlock();
	}

	CString strNick = MyProfile.GetNick();
	if ( ! strNick.IsEmpty() )
	{
		if ( strNick.GetLength() > 32 )
			strNick = strNick.Left( 32 );

		int nNick = pPacket->GetStringLen( strNick );
		pPacket->WritePacket( G2_PACKET_PROFILE, nNick + 6, TRUE );
		pPacket->WritePacket( G2_PACKET_NICK, nNick );
		pPacket->WriteString( strNick, FALSE );
	}

	if ( Settings.Community.ServeProfile ) pPacket->WritePacket( G2_PACKET_BROWSE_PROFILE, 0 );
	if ( Settings.Community.ServeFiles ) pPacket->WritePacket( G2_PACKET_BROWSE_HOST, 0 );
	if ( Settings.Community.ChatEnable ) pPacket->WritePacket( G2_PACKET_PEER_CHAT, 0 );

	return pPacket;
}

CDCPacket* CLocalSearch::CreatePacketDC()
{
	return CDCPacket::New();
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch core trailer

void CLocalSearch::WriteTrailer(CPacket* pPacket, CSchemaMap& pSchemas, BYTE nHits)
{
	ASSERT( pPacket != NULL );

	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		WriteTrailerG1( static_cast< CG1Packet* >( pPacket ), pSchemas, nHits );
		break;
	case PROTOCOL_G2:
		WriteTrailerG2( static_cast< CG2Packet* >( pPacket ), pSchemas, nHits );
		break;
	case PROTOCOL_DC:
		WriteTrailerDC( static_cast< CDCPacket* >( pPacket ), pSchemas, nHits );
		break;
	default:
		ASSERT( FALSE );
	}
}

void CLocalSearch::WriteTrailerG1(CG1Packet* pPacket, CSchemaMap& pSchemas, BYTE nHits)
{
	// Prepare XML
	CStringA sXML;
	for ( POSITION pos1 = pSchemas.GetStartPosition() ; pos1 ; )
	{
		CXMLElement* pGroup;
		CSchemaPtr pSchema;
		pSchemas.GetNextAssoc( pos1, pSchema, pGroup );
		sXML += UTF8Encode( pGroup->ToString( TRUE, FALSE ) );
		delete pGroup;
	}
	pSchemas.RemoveAll();

	// Compress XML
	DWORD nXMLLength = sXML.GetLength();
	DWORD nCompressedXMLLength = 0;
	auto_array< BYTE > pCompressedXML;
	if ( nXMLLength )
		pCompressedXML = CZLib::Compress( (LPCSTR)sXML, nXMLLength, &nCompressedXMLLength );

	// Flags: 'I understand' first byte, 'Yes/No' - second byte
	// REMEMBER THAT THE PUSH BIT IS SET OPPOSITE THAN THE OTHERS
	BYTE nFlags[ 2 ] =
	{
		G1_QHD_BUSY | G1_QHD_STABLE | G1_QHD_SPEED | G1_QHD_GGEP,
		G1_QHD_PUSH
	};
	if ( Network.IsFirewalled() )
		nFlags[ 0 ] |= G1_QHD_PUSH;
	if ( Uploads.m_bStable )
		nFlags[ 1 ] |= G1_QHD_STABLE;
	if ( Uploads.m_bStable )
		nFlags[ 1 ] |= G1_QHD_SPEED;
	if ( ! UploadQueues.IsTransferAvailable() )
		nFlags[ 1 ] |= G1_QHD_BUSY;
	if ( Settings.Gnutella1.EnableGGEP )
		nFlags[ 1 ] |= G1_QHD_GGEP;

	// Correct the number of files sent
	pPacket->m_pBuffer[ 0 ] = nHits;

	// Write client vendor code
	pPacket->WriteString( _T(VENDOR_CODE), FALSE );

	// Write public info
	pPacket->WriteByte( 4 );	// Public size: flags (2 bytes) + xml size (2 bytes)
	pPacket->WriteByte( nFlags[ 0 ] );
	pPacket->WriteByte( nFlags[ 1 ] );
	if ( pCompressedXML.get() && nCompressedXMLLength + 9 < nXMLLength + 2 )
	{
		// "{deflate}" (9 bytes) + NUL
		pPacket->WriteShortLE( (WORD)( nCompressedXMLLength + 9 + 1 ) );
	}
	else if ( nXMLLength )
	{
		// "{}" (2 bytes) + NUL
		pPacket->WriteShortLE( WORD( nXMLLength + 2 + 1 ) );
		pCompressedXML.reset();
		nCompressedXMLLength = 0;
	}
	else
	{
		// NUL
		pPacket->WriteShortLE( WORD( 1 ) );
	}

	// Write Chat flag
	pPacket->WriteByte( Settings.Community.ChatEnable ? G1_QHD_CHAT : 0 );

	// Write GGEP block
	if ( Settings.Gnutella1.EnableGGEP )
	{
		CGGEPBlock pBlock;
		// Write Browse flag
		if ( Settings.Community.ServeFiles )
			pBlock.Add( GGEP_HEADER_BROWSE_HOST );

		if ( Settings.Community.ChatEnable )
			pBlock.Add( GGEP_HEADER_CHAT );

		if ( m_bUDP && m_pSearch->m_nHops == 0 )
			pBlock.Add( GGEP_HEADER_MULTICAST_RESPONSE );

		pBlock.Write( pPacket );
	}

	// Write XML
	if ( nCompressedXMLLength )
	{
		pPacket->Write( "{deflate}", 9 );
		pPacket->Write( pCompressedXML.get(), nCompressedXMLLength );
		pPacket->WriteByte( 0 );
	}
	else if ( nXMLLength )
	{
		pPacket->Write( "{}", 2 );
		pPacket->Write( (LPCSTR)sXML, nXMLLength );
		pPacket->WriteByte( 0 );
	}
	else
	{
		pPacket->WriteByte( 0 );
	}

	// Client GUID
	pPacket->Write( Hashes::Guid( MyProfile.oGUID ) );

#ifdef _DEBUG
	// Test created hit
	if ( CQueryHit* pDebugHit = CQueryHit::FromG1Packet( pPacket ) )
	{
		pDebugHit->Delete();
		pPacket->m_nPosition = 0;
	}
	else
		theApp.Message( MSG_ERROR | MSG_FACILITY_SEARCH, L"[G1] Envy produced search packet above but cannot parse it back." );
#endif	// _DEBUG
}

void CLocalSearch::WriteTrailerG2(CG2Packet* pPacket, CSchemaMap& /*pSchemas*/, BYTE /*nHits*/)
{
	pPacket->WriteByte( 0 );	// End of packet
	pPacket->WriteByte( 0 );	// nHops
	pPacket->Write( m_oGUID );	// SearchID

#ifdef _DEBUG
	// Test created hit
	if ( CQueryHit* pDebugHit = CQueryHit::FromG2Packet( pPacket ) )
	{
		pDebugHit->Delete();
		pPacket->m_nPosition = 0;
	}
	else
		theApp.Message( MSG_ERROR | MSG_FACILITY_SEARCH, L"[G2] Envy produced search packet above but cannot parse it back." );
#endif // _DEBUG
}

void CLocalSearch::WriteTrailerDC(CDCPacket* /*pPacket*/, CSchemaMap& /*pSchemas*/, BYTE /*nHits*/)
{
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch dispatch packet

void CLocalSearch::DispatchPacket(CPacket* pPacket)
{
	if ( ! pPacket )
		return;

	if ( m_pBuffer )
	{
		pPacket->ToBuffer( m_pBuffer );
	}
	else if ( m_bUDP )
	{
		Datagrams.Send( &m_pEndpoint, pPacket, FALSE );
	}
	else
	{
		CQuickLock oLock( Network.m_pSection );
		if ( CNeighbour* pNeighbour = Neighbours.Get( m_pEndpoint.sin_addr ) )
		{
			if ( pNeighbour->m_nProtocol == m_nProtocol )
				pNeighbour->Send( pPacket, FALSE, TRUE );
		}
	}

	pPacket->Release();
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch physical and virtual folder tree

// Obsolete (for deletion)
//void CLocalSearch::WriteVirtualTree()
//{
//	CSingleLock oLock( &Library.m_pSection );
//	if ( oLock.Lock( 1000 ) )
//	{
//		CG2Packet* pPacket = AlbumToPacket( Library.GetAlbumRoot() );
//		oLock.Unlock();
//		if ( pPacket ) DispatchPacket( pPacket );
//	}
//	if ( oLock.Lock( 1000 ) )
//	{
//		CG2Packet* pPacket = FoldersToPacket();
//		oLock.Unlock();
//		if ( pPacket ) DispatchPacket( pPacket );
//	}
//}

CG2Packet* CLocalSearch::AlbumToPacket(CAlbumFolder* pFolder)
{
	if ( pFolder == NULL ) return NULL;
	if ( pFolder->m_pSchema != NULL && pFolder->m_pSchema->m_bPrivate ) return NULL;
	if ( pFolder->GetSharedCount() == 0 ) return NULL;

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_VIRTUAL_FOLDER, TRUE );
	if ( ! pPacket ) return NULL;

	if ( pFolder->m_pSchema != NULL )
	{
		augment::auto_ptr< CXMLElement > pXML( pFolder->m_pSchema->Instantiate( TRUE ) );
		if ( ! pXML.get() ) return NULL;

		if ( pFolder->m_pXML != NULL )
		{
			pXML->AddElement( pFolder->m_pXML->Clone() );
		}
		else if ( CXMLElement* pBody = pXML->AddElement( pFolder->m_pSchema->m_sSingular ) )
		{
			pBody->AddAttribute( pFolder->m_pSchema->GetFirstMemberName(), pFolder->m_sName );
		}

		CString strXML = pXML->ToString();

		pPacket->WritePacket( G2_PACKET_METADATA, pPacket->GetStringLen( strXML ) );
		pPacket->WriteString( strXML, FALSE );
	}

	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		if ( CG2Packet* pChild = AlbumToPacket( pFolder->GetNextFolder( pos ) ) )
		{
			pPacket->WritePacket( pChild );
			pChild->Release();
		}
	}

	if ( DWORD nFiles = pFolder->GetSharedCount( FALSE ) )
	{
		pPacket->WritePacket( G2_PACKET_FILES, nFiles * 4u );

		for ( POSITION pos = pFolder->GetFileIterator() ; pos ; )
		{
			CLibraryFile* pFile = pFolder->GetNextFile( pos );
			pPacket->WriteLongBE( pFile->m_nIndex );
		}
	}

	return pPacket;
}

CG2Packet* CLocalSearch::FoldersToPacket()
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PHYSICAL_FOLDER, TRUE );
	if ( ! pPacket ) return NULL;

	for ( POSITION pos = LibraryFolders.GetFolderIterator() ; pos ; )
	{
		if ( CG2Packet* pChild = FolderToPacket( LibraryFolders.GetNextFolder( pos ) ) )
		{
			pPacket->WritePacket( pChild );
			pChild->Release();
		}
	}

	return pPacket;
}

CG2Packet* CLocalSearch::FolderToPacket(CLibraryFolder* pFolder)
{
	if ( pFolder == NULL ) return NULL;
	if ( pFolder->GetSharedCount() == 0 ) return NULL;

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PHYSICAL_FOLDER, TRUE );
	if ( ! pPacket ) return NULL;

	pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pFolder->m_sName ) );
	pPacket->WriteString( pFolder->m_sName, FALSE );

	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		if ( CG2Packet* pChild = FolderToPacket( pFolder->GetNextFolder( pos ) ) )
		{
			pPacket->WritePacket( pChild );
			pChild->Release();
		}
	}

	if ( DWORD nFiles = pFolder->GetSharedCount( FALSE ) )
	{
		pPacket->WritePacket( G2_PACKET_FILES, nFiles * 4u );

		for ( POSITION pos = pFolder->GetFileIterator() ; pos ; )
		{
			const CLibraryFile* pFile = pFolder->GetNextFile( pos );
			if ( pFile->IsShared() )
				pPacket->WriteLongBE( pFile->m_nIndex );
		}
	}

	return pPacket;
}
