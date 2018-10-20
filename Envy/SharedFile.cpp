//
// SharedFile.cpp
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
#include "SharedFile.h"
#include "SharedFolder.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryDictionary.h"
#include "LibraryFolders.h"
#include "LibraryHistory.h"
#include "HashDatabase.h"
#include "Plugins.h"
#include "WndChild.h"
#include "WndMain.h"

#include "Buffer.h"
#include "Network.h"
#include "Download.h"
#include "Downloads.h"
#include "EnvyURL.h"
#include "FileExecutor.h"
#include "ThumbCache.h"

#include "XML.h"
#include "XMLCOM.h"
#include "Schema.h"
#include "SchemaCache.h"

#include "Application.h"
#include "VersionChecker.h"
#include "DlgFolderScan.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CLibraryFile, CEnvyFile)

BEGIN_INTERFACE_MAP(CLibraryFile, CEnvyFile)
	INTERFACE_PART(CLibraryFile, IID_ILibraryFile, LibraryFile)
END_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////
// CLibraryFile construction

CLibraryFile::CLibraryFile(CLibraryFolder* pFolder, LPCTSTR pszName)
	: m_pNextSHA1		( NULL )
	, m_pNextTiger		( NULL )
	, m_pNextED2K		( NULL )
	, m_pNextBTH		( NULL )
	, m_pNextMD5		( NULL )
	, m_nScanCookie		( 0ul )
	, m_nUpdateCookie	( 0ul )
	, m_nSelectCookie	( 0ul )
	, m_nListCookie		( 0ul )
	, m_pFolder			( pFolder )
	, m_nIndex			( 0ul )
	, m_nVirtualBase	( 0ull )
	, m_nVirtualSize	( 0ull )
	, m_bVerify			( TRI_UNKNOWN )
	, m_bShared			( TRI_UNKNOWN )
	, m_pSchema			( NULL )
	, m_pMetadata		( NULL )
	, m_bMetadataAuto	( FALSE )
	, m_bMetadataModified ( FALSE )
	, m_bCachedPreview	( FALSE )
	, m_bBogus			( FALSE )
	, m_nRating			( 0 )
//	, m_bShareTag		( TRI_UNKNOWN )		// ToDo: Implement ShareTag
	, m_nHitsToday		( 0ul )
	, m_nHitsTotal		( 0ul )
	, m_nUploadsToday	( 0ul )
	, m_nUploadsTotal	( 0ul )
	, m_nSearchCookie	( 0ul )
	, m_nSearchWords	( 0ul )
	, m_pNextHit		( NULL )
	, m_nCollIndex		( 0ul )
	, m_nIcon16			( -1 )
	, m_bNewFile		( FALSE )
	, m_tCreateTime		( 0 )
{
	ZeroMemory( &m_pTime, sizeof( m_pTime ) );
	ZeroMemory( &m_pMetadataTime, sizeof( m_pMetadataTime ) );
	if ( pszName )
	{
		m_sName = pszName;
		m_pSchema = SchemaCache.GuessByFilename( m_sName );
	}
	if ( pFolder )
		m_sPath = pFolder->m_sPath;

	EnableDispatch( IID_ILibraryFile );
}

CLibraryFile::~CLibraryFile()
{
	Library.RemoveFile( this );

	delete m_pMetadata;

	for ( POSITION pos = m_pSources.GetHeadPosition(); pos; )
	{
		delete m_pSources.GetNext( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile path computation

const CLibraryFolder* CLibraryFile::GetFolderPtr() const
{
	ASSUME_LOCK( Library.m_pSection );
	ASSERT_VALID( this );

	return m_pFolder;
}

CString CLibraryFile::GetFolder() const
{
	if ( m_pFolder )
		return m_pFolder->m_sPath;

	return CString();
}

CString CLibraryFile::GetPath() const
{
	if ( m_pFolder )
		return m_pFolder->m_sPath + L'\\' + m_sName;

	return m_sName;
}

CString CLibraryFile::GetSearchName() const
{
	int nBase = 0;
	CString str;

	if ( m_pFolder && m_pFolder->m_pParent )
	{
		for ( const CLibraryFolder* pFolder = m_pFolder; ; pFolder = pFolder->m_pParent )
		{
			if ( pFolder->m_pParent == NULL )
			{
				nBase = pFolder->m_sPath.GetLength();
				break;
			}
		}
	}

	if ( nBase <= 0 )
	{
		str = m_sName;
	}
	else
	{
		ASSERT( m_pFolder->m_sPath.GetLength() > nBase );
		str = m_pFolder->m_sPath.Mid( nBase + 1 ) + L'\\' + m_sName;
	}

	ToLower( str );
	return str;
}

CXMLElement* CLibraryFile::CreateXML(CXMLElement* pRoot, BOOL bSharedOnly, XmlType nType /*0*/) const
{
	if ( bSharedOnly && ! IsShared() )
		return NULL;

	// ToDo: Support private passkey

	// Special case-sensitive http://adc.sourceforge.net/ADC.html#_file_list
	if ( nType == xmlDC )
	{
		CXMLElement* pFile = pRoot->AddElement( L"File" );;
		if ( pFile )
		{
			pFile->AddAttribute( L"Name", m_sName );
			pFile->AddAttribute( L"Size", m_nSize );
			pFile->AddAttribute( L"TTH", m_oTiger.toString() );
		}
		return pFile;
	}

	CXMLElement* pFile = pRoot->AddElement( L"file" );
	if ( pFile )
	{
		if ( m_oSHA1 && m_oTiger )
			pFile->AddElement( L"id" )->SetValue( L"urn:bitprint:" + m_oSHA1.toString() + L'.' + m_oTiger.toString() );
		else if ( m_oSHA1 )
			pFile->AddElement( L"id" )->SetValue( m_oSHA1.toUrn() );
		else if ( m_oTiger )
			pFile->AddElement( L"id" )->SetValue( m_oTiger.toUrn() );

		if ( m_oMD5 )
			pFile->AddElement( L"id" )->SetValue( m_oMD5.toUrn() );

		if ( m_oED2K )
			pFile->AddElement( L"id" )->SetValue( m_oED2K.toUrn() );

		if ( m_oBTH )
			pFile->AddElement( L"id" )->SetValue( m_oBTH.toUrn() );

		if ( CXMLElement* pDescription = pFile->AddElement( L"description" ) )
		{
			pDescription->AddElement( L"name" )->SetValue( m_sName );

			CString str;
			str.Format( L"%I64u", GetSize() );
			pDescription->AddElement( L"size" )->SetValue( str );
		}

		if ( m_pMetadata && m_pSchema )
		{
			if ( CXMLElement* pMetadata = pFile->AddElement( L"metadata" ) )
			{
				m_pMetadata->m_bOrdered = FALSE;	// Workaround
				pMetadata->AddAttribute( L"xmlns:s", m_pSchema->GetURI() );
				pMetadata->AddElement( m_pMetadata->Prefix( L"s:" ) );
			}
		}
	}

	return pFile;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile shared check

bool CLibraryFile::IsShared(bool bIgnoreOverride /*false*/) const
{
	ASSUME_LOCK( Library.m_pSection );

	// Use override shared flag of file
	if ( m_bShared != TRI_UNKNOWN && ! bIgnoreOverride )
		return ( m_bShared == TRI_TRUE );

	// Don't share offline files
	if ( m_pFolder && m_pFolder->IsOffline() )
		return false;

	// Don't share private torrents
	if ( IsPrivateTorrent() )
		return false;

	// Ghost files by default shared, then use folder shared flag
	return ! m_pFolder || m_pFolder->IsShared();
}

void CLibraryFile::SetShared(bool bShared, bool bOverride)
{
	TRISTATE bNewShare = bOverride ? ( bShared ? TRI_TRUE : TRI_FALSE ) : TRI_UNKNOWN;

	// Don't share not verified files
	if ( m_bVerify == TRI_FALSE )
		bNewShare = TRI_FALSE;

	// Don't share private torrents
	if ( IsPrivateTorrent() )
		bNewShare = TRI_FALSE;

	bool bFolderShared = ! m_pFolder || m_pFolder->IsShared();

	// Use folder default shared flag
	if ( bNewShare == TRI_UNKNOWN )
		bNewShare = bShared ?
			( bFolderShared ? TRI_UNKNOWN : TRI_TRUE ) :
			( bFolderShared ? TRI_FALSE : TRI_UNKNOWN );

	if ( m_bShared != bNewShare )
	{
		m_bShared = bNewShare;
		m_nUpdateCookie++;

		LibraryDictionary.Invalidate();
	}
}

bool CLibraryFile::IsPrivateTorrent() const
{
	return m_pSchema && m_pMetadata &&
		m_pSchema->CheckURI( CSchema::uriBitTorrent ) &&
		m_pMetadata->GetAttributeValue( L"privateflag", L"false" ).Compare( L"true" ) == 0;		// Set "true" in LibraryBuilderInternals if "private=1"
}

DWORD CLibraryFile::GetCreationTime()
{
	if ( m_tCreateTime )
		return m_tCreateTime;

	if ( m_pFolder && m_pFolder->IsOffline() )
		return 0;

	HANDLE hFile = CreateFile( SafePath( GetPath() ), FILE_READ_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		return 0;

	FILETIME ftLastWriteTime;
	BOOL bResult = GetFileTime( hFile, NULL, NULL, &ftLastWriteTime );

	CloseHandle( hFile );

	if ( ! bResult )
		return 0;

	return m_tCreateTime = ( ( MAKEQWORD( ftLastWriteTime.dwLowDateTime,
		ftLastWriteTime.dwHighDateTime ) ) / 10000000ui64 - 11644473600ui64 );
}

BOOL CLibraryFile::SetCreationTime(DWORD tTime)
{
	if ( tTime > static_cast< DWORD >( time( NULL ) ) )
		return FALSE;

	m_tCreateTime = tTime;

	if ( m_pFolder && m_pFolder->IsOffline() )
		return FALSE;

	HANDLE hFile = CreateFile( SafePath( GetPath() ), FILE_WRITE_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		return FALSE;

	QWORD t = ( (QWORD)m_tCreateTime + 11644473600ui64 ) * 10000000ui64;
	FILETIME ftLastWriteTime = { (DWORD)t, (DWORD)( t >> 32 ) };
	BOOL bResult = SetFileTime( hFile, NULL, NULL, &ftLastWriteTime );

	CloseHandle( hFile );

	return bResult;
}

BOOL CLibraryFile::CheckFileAttributes(QWORD nSize, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	return ( nSize == SIZE_UNKNOWN || nSize == 0 || nSize == m_nSize ) &&
		( ! bAvailableOnly || m_pFolder ) &&
		( ! bSharedOnly || IsShared() );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile schema URI test

BOOL CLibraryFile::IsSchemaURI(LPCTSTR pszURI) const
{
	if ( m_pSchema == NULL )
		return ( pszURI == NULL || *pszURI == NULL );

	return m_pSchema->CheckURI( pszURI );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rated or commented (or no metadata)

BOOL CLibraryFile::IsRated() const
{
	return ( m_nRating || ! m_sComments.IsEmpty() );
}

BOOL CLibraryFile::IsRatedOnly() const
{
	return IsRated() && ( m_pSchema == NULL || m_pMetadata == NULL );
}

// ToDo: Implement ShareTags (Permissiveness)
//BOOL CLibraryFile::IsShareTagged() const
//{
//	return ( m_bShareTag != TRI_UNKNOWN );
//}

BOOL CLibraryFile::IsHashed() const
{
	return m_oSHA1 && m_oTiger && m_oMD5 && m_oED2K;	// m_oBTH ignored
}

BOOL CLibraryFile::IsNewFile() const
{
	return m_bNewFile;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rebuild hashes and metadata

BOOL CLibraryFile::Rebuild()
{
	if ( ! m_pFolder ) return FALSE;	// Ghost file

	Library.RemoveFile( this );

	m_oSHA1.clear();
	m_oTiger.clear();
	m_oED2K.clear();
	m_oMD5.clear();
	m_oBTH.clear();

	m_nVirtualBase = m_nVirtualSize = 0;

	if ( m_pMetadata && m_bMetadataAuto )
	{
		delete m_pMetadata;
		m_pMetadata = NULL;
	}

	Library.AddFile( this );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rename

BOOL CLibraryFile::Rename(LPCTSTR pszName)
{
	if ( ! m_pFolder ) return FALSE;	// Ghost file
	if ( ! pszName || ! *pszName ) return FALSE;
	if ( _tcschr( pszName, '\\' ) ) return FALSE;

	CString strNew = m_pFolder->m_sPath + '\\' + pszName;

	// Close the file handle
	theApp.OnRename( GetPath() );

	if ( MoveFile( GetPath(), strNew ) )
	{
		// Success. Tell the file to use its new name
		theApp.OnRename( GetPath(), strNew );
	}
	else
	{
		// Failure. Continue using its old name
		theApp.OnRename( GetPath(), GetPath() );
		return FALSE;
	}

	//if ( m_pMetadata )
	//{
	//	CString strMetaOld = m_pFolder->m_sPath + L"\\Metadata\\" + m_sName + L".xml";
	//	CString strMetaNew = m_pFolder->m_sPath + L"\\Metadata\\" + pszName + L".xml";
	//	MoveFile( strMetaOld, strMetaNew );
	//}

	Library.RemoveFile( this );

	m_sName = pszName;

	m_pFolder->OnFileRename( this );

	Library.AddFile( this );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile delete

BOOL CLibraryFile::Delete(BOOL bDeleteGhost)
{
	if ( m_pFolder )
	{
		// Delete file
		BOOL bToRecycleBin = ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 );
		if ( ! DeleteFileEx( GetPath(), TRUE, bToRecycleBin, TRUE ) )
			return FALSE;
	}

	OnDelete( bDeleteGhost );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile metadata access

void CLibraryFile::UpdateMetadata(const CDownload* pDownload)
{
	// Disable sharing of incomplete files
	if ( pDownload->m_bVerify == TRI_FALSE )
	{
		m_bVerify = m_bShared = TRI_FALSE;
		m_bBogus = TRUE;
	}

	// Get BTIH of recently downloaded file
	if ( ! m_oBTH && pDownload->IsSingleFileTorrent() )
		m_oBTH = pDownload->m_oBTH;

	// Get metadata of recently downloaded file
	if ( pDownload->HasMetadata() )
	{
		if ( m_pMetadata )
		{
			// Update existing
			BOOL bMetadataAuto = m_bMetadataAuto;
			if ( MergeMetadata( pDownload->m_pXML ) )
				m_bMetadataAuto = bMetadataAuto;	// Preserve flag
		}
		else if ( CXMLElement* pBody = pDownload->m_pXML->GetFirstElement() )
		{
			// Recreate metadata
			TRACE( "Using download XML:%s", pBody->ToString( FALSE, TRUE ) );
			m_pSchema = SchemaCache.Get( pDownload->m_pXML->GetAttributeValue( CXMLAttribute::schemaName ) );
			m_pMetadata = pBody->Clone();
			m_bMetadataAuto = TRUE;
			ModifyMetadata();
		}
	}
}

BOOL CLibraryFile::SetMetadata(CXMLElement*& pXML, BOOL bMerge, BOOL bOverwrite)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pMetadata == NULL && pXML == NULL )
		return TRUE;	// No need

	CSchemaPtr pSchema = NULL;

	if ( pXML != NULL )
	{
		// Try fully specified XML first, for example <videos ...><video ... /></videos>
		pSchema = SchemaCache.Get( pXML->GetAttributeValue( CXMLAttribute::schemaName ) );
		if ( pSchema == NULL )
		{
			// Then try short version, for example <video ... />
			pSchema = SchemaCache.Guess( pXML->GetName() );
			if ( ! pSchema )
				pSchema = SchemaCache.GuessByFilename( m_sName );

			if ( pSchema )
			{
				// Recreate full XML
				if ( CXMLElement* pFullXML = pSchema->Instantiate( TRUE ) )
				{
					if ( pFullXML->AddElement( pXML ) )
						pXML = pFullXML;
					else
						delete pFullXML;
				}
			}
		}

		if ( pSchema == NULL || ! pSchema->Validate( pXML, TRUE ) )
			return FALSE;	// Invalid XML

		if ( m_pMetadata && bMerge )
			pXML->GetFirstElement()->Merge( m_pMetadata, ! bOverwrite );

		if ( m_pMetadata && m_pSchema == pSchema &&
			m_pMetadata->Equals( pXML->GetFirstElement() ) )
			return TRUE;	// No need
	}
	else
	{
		pSchema = SchemaCache.GuessByFilename( m_sName );
	}

	Library.RemoveFile( this );

	delete m_pMetadata;

	m_pSchema		= pSchema;
	m_pMetadata		= pXML ? pXML->GetFirstElement()->Detach() : NULL;
	m_bMetadataAuto	= FALSE;

	delete pXML;
	pXML = NULL;

	ModifyMetadata();

	Library.AddFile( this );

	return TRUE;
}

BOOL CLibraryFile::MergeMetadata(CXMLElement*& pXML, BOOL bOverwrite)
{
	return SetMetadata( pXML, TRUE, bOverwrite );
}

BOOL CLibraryFile::MergeMetadata(const CXMLElement* pXML)
{
	BOOL bResult = FALSE;
	if ( CXMLElement* pCloned = pXML->Clone() )
	{
		bResult = SetMetadata( pCloned, TRUE, FALSE );
		delete pCloned;
	}
	return bResult;
}

void CLibraryFile::ClearMetadata()
{
	CXMLElement* pXML = NULL;
	SetMetadata( pXML );
}

void CLibraryFile::ModifyMetadata()
{
	m_bMetadataModified = TRUE;
	GetSystemTimeAsFileTime( &m_pMetadataTime );
}

CString CLibraryFile::GetMetadataWords() const
{
	if ( m_pSchema != NULL && m_pMetadata != NULL )
		return m_pSchema->GetIndexedWords( m_pMetadata );

	return CString();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile hash volume lookups

CTigerTree* CLibraryFile::GetTigerTree()
{
	if ( ! m_oTiger )
		return NULL;	// Not hashed yet

	if ( ! m_pFolder )
		return NULL;	// Virtual file

	CAutoPtr< CTigerTree > pTiger( new CTigerTree() );
	if ( ! pTiger )
		return NULL;	// Out of memory

	if ( ! LibraryHashDB.GetTiger( m_nIndex, pTiger ) )
		return NULL;	// Database error

	pTiger->SetupParameters( m_nSize );

	Hashes::TigerHash oRoot;
	pTiger->GetRoot( &oRoot[ 0 ] );
	oRoot.validate();
	if ( m_oTiger != oRoot )
	{
		// Wrong hash
		LibraryHashDB.DeleteTiger( m_nIndex );

		Library.RemoveFile( this );
		m_oTiger.clear();
		Library.AddFile( this );

		return NULL;
	}

	// OK
	return pTiger.Detach();
}

CED2K* CLibraryFile::GetED2K()
{
	if ( ! m_oED2K ) return NULL;
	if ( ! m_pFolder ) return NULL;

	CED2K* pED2K = new CED2K();

	if ( LibraryHashDB.GetED2K( m_nIndex, pED2K ) )
	{
		Hashes::Ed2kHash oRoot;
		pED2K->GetRoot( &oRoot[ 0 ] );
		oRoot.validate();
		if ( m_oED2K == oRoot ) return pED2K;

		LibraryHashDB.DeleteED2K( m_nIndex );
	}

	delete pED2K;
	Library.RemoveFile( this );
	m_oED2K.clear();
	Library.AddFile( this );

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile alternate sources

CSharedSource* CLibraryFile::AddAlternateSources(LPCTSTR pszURLs)
{
	if ( pszURLs == NULL ) return NULL;
	if ( *pszURLs == 0 ) return NULL;

	CSharedSource* pFirst = NULL;

	CMapStringToFILETIME oUrls;
	SplitStringToURLs( pszURLs, oUrls );

	for ( POSITION pos = oUrls.GetStartPosition(); pos; )
	{
		CString strURL;
		FILETIME tSeen = {};
		oUrls.GetNextAssoc( pos, strURL, tSeen );
		if ( CSharedSource* pSource = AddAlternateSource( strURL, &tSeen ) )
			pFirst = pSource;
	}

	return pFirst;
}

CSharedSource* CLibraryFile::AddAlternateSource(LPCTSTR pszURL, FILETIME* tSeen)
{
	if ( pszURL == NULL ) return NULL;
	if ( *pszURL == 0 ) return NULL;

	CString strURL( pszURL );
	CEnvyURL pURL;

	BOOL bSeen;
	FILETIME tSeenLocal = { 0, 0 };
	if ( tSeen && tSeen->dwLowDateTime && tSeen->dwHighDateTime )
	{
		bSeen = TRUE;
	}
	else
	{
		tSeen = &tSeenLocal;
		bSeen = FALSE;
	}

	int nPos = strURL.ReverseFind( L' ' );
	if ( nPos > 0 )
	{
		CString strTime = strURL.Mid( nPos + 1 );
		strURL = strURL.Left( nPos );
		strURL.TrimRight();
		bSeen = TimeFromString( strTime, tSeen );
	}

	if ( ! pURL.Parse( strURL ) ) return NULL;

	if ( Network.IsFirewalledAddress( &pURL.m_pAddress, Settings.Connection.IgnoreOwnIP ) ||
		 Network.IsReserved( (IN_ADDR*)&pURL.m_pAddress ) ) return NULL;

	if ( pURL != *this ) return NULL;

	for ( POSITION pos = m_pSources.GetHeadPosition(); pos; )
	{
		CSharedSource* pSource = m_pSources.GetNext( pos );

		if ( pSource->m_sURL.CompareNoCase( strURL ) == 0 )
		{
			pSource->Freshen( bSeen ? tSeen : NULL );
			return pSource;
		}
	}

	CSharedSource* pSource = new CSharedSource( strURL, bSeen ? tSeen : NULL );
	m_pSources.AddTail( pSource );

	return pSource;
}

CString CLibraryFile::GetAlternateSources(CList< CString >* pState, int nMaximum, PROTOCOLID nProtocol)
{
	CString strSources;
	SYSTEMTIME stNow;
	FILETIME ftNow;

	GetSystemTime( &stNow );
	SystemTimeToFileTime( &stNow, &ftNow );

	for ( POSITION pos = m_pSources.GetHeadPosition(); pos; )
	{
		CSharedSource* pSource = m_pSources.GetNext( pos );

		if ( ! pSource->IsExpired( ftNow ) &&
			 ( pState == NULL || pState->Find( pSource->m_sURL ) == NULL ) )
		{
			if ( ( nProtocol == PROTOCOL_HTTP ) && ( _tcsncmp( pSource->m_sURL, L"http://", 7 ) != 0 ) )
				continue;

			if ( pState != NULL )
				pState->AddTail( pSource->m_sURL );

			if ( pSource->m_sURL.Find( L"Zhttp://" ) >= 0 ||
				pSource->m_sURL.Find( L"Z%2C http://" ) >= 0 )
			{
				// Ignore buggy URLs
				TRACE( "CLibraryFile::GetAlternateSources() Bad URL: %s\n", (LPCSTR)CT2A( pSource->m_sURL ) );
			}
			else
			{
				CString strURL = pSource->m_sURL;
				strURL.Replace( L",", L"%2C" );

				if ( ! strSources.IsEmpty() ) strSources += L", ";
				strSources += strURL;
				strSources += ' ';
				strSources += TimeToString( &pSource->m_pTime );
			}

			if ( nMaximum == 1 )
				break;
			else if ( nMaximum > 1 )
				nMaximum--;
		}
	}

	return strSources;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile serialize

void CLibraryFile::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ASSERT( ! m_sName.IsEmpty() );
		ar << m_sName;
		ar << m_nIndex;
		ar << m_nSize;
		ar.Write( &m_pTime, sizeof( m_pTime ) );
		ar << m_bShared;

		ar << m_nVirtualSize;
		if ( m_nVirtualSize > 0 )
			ar << m_nVirtualBase;

		SerializeOut( ar, m_oSHA1 );
		SerializeOut( ar, m_oTiger );
		SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oMD5 );
		SerializeOut( ar, m_oBTH );
		ar << m_bVerify;

		if ( m_pSchema != NULL && m_pMetadata != NULL )
		{
			ar << m_pSchema->GetURI();
			m_pMetadata->Serialize( ar );
		}
		else
		{
			CString strURI;
			ar << strURI;
		}

		ar << m_nRating;
		ar << m_sComments;
		ar << m_sShareTags;
		//ar << m_bShareTag;

		//if ( nVersion > 1000 )
		//{
		//	// ShareTags Support?
		//	ar << m_nShareTag;
		//}
		//else
		//{
		//	DWORD nShareTag;
		//	ar >> nShareTag;
		//	m_nShareTag = nShareTag;
		//}

		ar << m_bMetadataAuto;
		ar.Write( &m_pMetadataTime, sizeof( m_pMetadataTime ) );
		m_bMetadataModified = FALSE;

		ar << m_nHitsTotal;
		ar << m_nUploadsTotal;
		ar << m_bCachedPreview;
		ar << m_bBogus;

		ar.WriteCount( m_pSources.GetCount() );

		for ( POSITION pos = m_pSources.GetHeadPosition(); pos; )
		{
			CSharedSource* pSource = m_pSources.GetNext( pos );
			pSource->Serialize( ar, nVersion );
		}
	}
	else // Loading
	{
		ar >> m_sName;
		ASSERT( ! m_sName.IsEmpty() );

		ar >> m_nIndex;
		ar >> m_nSize;

		ReadArchive( ar, &m_pTime, sizeof( m_pTime ) );

		ar >> m_bShared;

		ar >> m_nVirtualSize;
		if ( m_nVirtualSize > 0 )
			ar >> m_nVirtualBase;

		SerializeIn( ar, m_oSHA1, nVersion );
		SerializeIn( ar, m_oTiger, nVersion );
		SerializeIn( ar, m_oED2K, nVersion );
		SerializeIn( ar, m_oMD5,  nVersion );
		SerializeIn( ar, m_oBTH, nVersion );

		ar >> m_bVerify;

		CString strURI;
		ar >> strURI;

		if ( ! strURI.IsEmpty() )
		{
			m_pMetadata = new CXMLElement();
			if ( ! m_pMetadata )
				AfxThrowMemoryException();
			m_pMetadata->Serialize( ar );
			m_pSchema = SchemaCache.Get( strURI );
			if ( m_pSchema == NULL )
			{
				delete m_pMetadata;
				m_pMetadata = NULL;
			}
			// else schema URI changed
		}
		if ( m_pSchema == NULL )
			m_pSchema = SchemaCache.GuessByFilename( m_sName );

		ar >> m_nRating;
		ar >> m_sComments;
		ar >> m_sShareTags;
		//ar >> m_bShareTag;

		ar >> m_bMetadataAuto;
		ReadArchive( ar, &m_pMetadataTime, sizeof( m_pMetadataTime ) );

		m_bMetadataModified = FALSE;

		ar >> m_nHitsTotal;
		ar >> m_nUploadsTotal;
		ar >> m_bCachedPreview;
		//if ( nVersion >= 20 )
		ar >> m_bBogus;

		if ( nVersion > 0 )
		{
			SYSTEMTIME stNow;
			FILETIME ftNow;

			GetSystemTime( &stNow );
			SystemTimeToFileTime( &stNow, &ftNow );

			for ( DWORD_PTR nSources = ar.ReadCount(); nSources > 0; nSources-- )
			{
				CSharedSource* pSource = new CSharedSource();
				if ( pSource == NULL )
					break;
					// theApp.Message( MSG_DEBUG, L"Memory allocation error in CLibraryFile::Serialize" );

				pSource->Serialize( ar, nVersion );

				if ( pSource->IsExpired( ftNow ) )
					delete pSource;
				else
					m_pSources.AddTail( pSource );
			}
		}

		// Rehash old audio files
		//if ( nVersion < 22 && m_pSchema != NULL && m_pSchema->CheckURI( CSchema::uriAudio ) )
		//{
		//	m_oSHA1.clear();
		//	m_oTiger.clear();
		//	m_oMD5.clear();
		//	m_oED2K.clear();
		//}

		//Library.AddFile( this );	// Added in SharedFolder Serialize
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile threaded scan

BOOL CLibraryFile::ThreadScan(DWORD nScanCookie, QWORD nSize, FILETIME* pTime/*, LPCTSTR pszMetaData*/)
{
	ASSUME_LOCK( Library.m_pSection );
	ASSERT( m_pFolder );	// No ghosts

	m_nScanCookie = nScanCookie;

	// If file is already in library but hashing was delayed, hash it again
	if ( m_nSize == nSize && CompareFileTime( &m_pTime, pTime ) == 0 )
	{
		if ( m_nIndex && ! IsHashed() )
			LibraryBuilder.Add( this );

		CFolderScanDlg::Update( m_sName,
			( m_nSize == SIZE_UNKNOWN ) ? 0 : (DWORD)( m_nSize / 1024 ) );

		return m_bMetadataModified;
	}

	Library.RemoveFile( this );

	CopyMemory( &m_pTime, pTime, sizeof( FILETIME ) );
	m_nSize = nSize;

	m_oSHA1.clear();
	m_oTiger.clear();
	m_oMD5.clear();
	m_oED2K.clear();

	Library.AddFile( this );

	m_nUpdateCookie++;

	CFolderScanDlg::Update( m_sName,
		( m_nSize == SIZE_UNKNOWN ) ? 0 : (DWORD)( m_nSize / 1024 ) );

	return TRUE;
}

BOOL CLibraryFile::IsReadable() const
{
	if ( ! m_pFolder )
		return FALSE;

	HANDLE hFile = CreateFile( SafePath( GetPath() ), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile delete handler
// bDeleteGhost is used only when deleting ghost files

void CLibraryFile::OnDelete(BOOL bDeleteGhost, TRISTATE bCreateGhost)
{
	if ( m_pFolder )
	{
		CThumbCache::Delete( GetPath() );

		if ( bCreateGhost == TRI_TRUE )
		{
			if ( ! IsRated() )
			{
				m_bShared = TRI_FALSE;

				CString strTransl;
				CString strUntransl = L"Ghost File";
				LoadString( strTransl, IDS_LIBRARY_GHOST_FILE );
				if ( strTransl == strUntransl )
					m_sComments = strUntransl;
				else
					m_sComments = strTransl + L" (" + strUntransl + L")";
			}
			Ghost();
			return;
		}

		if ( IsRated() && bCreateGhost != TRI_FALSE )
		{
			Ghost();
			return;
		}
	}

	// Remove file from all albums and folders
	LibraryFolders.OnFileDelete( this, bDeleteGhost );

	// Remove file from library history
	LibraryHistory.OnFileDelete( this );

	// Remove tiger/ed2k hash trees
	LibraryHashDB.DeleteAll( m_nIndex );

	delete this;
}

void CLibraryFile::Ghost()
{
	ASSUME_LOCK( Library.m_pSection );

	SYSTEMTIME pTime;
	GetSystemTime( &pTime );
	SystemTimeToFileTime( &pTime, &m_pTime );

	// Remove file from library maps, builder and dictionaries
	Library.RemoveFile( this );

	// Remove file from all albums and folders (skipping ghost files)
	LibraryFolders.OnFileDelete( this, FALSE );

	// Remove file from library history
	LibraryHistory.OnFileDelete( this );

	// Remove tiger/ed2k hash trees
	LibraryHashDB.DeleteAll( m_nIndex );

	m_pFolder = NULL;
	m_sPath.Empty();
	Library.AddFile( this );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile download verification

BOOL CLibraryFile::OnVerifyDownload(const CLibraryRecent* pRecent)
{
	ASSERT( IsAvailable() );
	ASSERT( IsHashed() );

	if ( Settings.Downloads.VerifyFiles && m_bVerify == TRI_UNKNOWN && m_nVirtualSize == 0 )
	{
		if ( (bool)m_oSHA1 && (bool)pRecent->m_oSHA1 && pRecent->m_oSHA1.isTrusted() )
			m_bVerify = ( m_oSHA1 == pRecent->m_oSHA1 ) ? TRI_TRUE : TRI_FALSE;

		if ( m_bVerify != TRI_FALSE && (bool)m_oTiger && (bool)pRecent->m_oTiger && pRecent->m_oTiger.isTrusted() )
			m_bVerify = ( m_oTiger == pRecent->m_oTiger ) ? TRI_TRUE : TRI_FALSE;

		if ( m_bVerify != TRI_FALSE && (bool)m_oED2K && (bool)pRecent->m_oED2K && pRecent->m_oED2K.isTrusted() )
			m_bVerify = ( m_oED2K == pRecent->m_oED2K ) ? TRI_TRUE : TRI_FALSE;

		if ( m_bVerify != TRI_FALSE && (bool)m_oMD5 && (bool)pRecent->m_oMD5 && pRecent->m_oMD5.isTrusted() )
			m_bVerify = ( m_oMD5 == pRecent->m_oMD5 ) ? TRI_TRUE : TRI_FALSE;

		if ( m_bVerify != TRI_FALSE && (bool)m_oBTH && (bool)pRecent->m_oBTH && pRecent->m_oBTH.isTrusted() )
			m_bVerify = ( m_oBTH == pRecent->m_oBTH ) ? TRI_TRUE : TRI_FALSE;

		Downloads.OnVerify( this, m_bVerify );

		if ( m_bVerify == TRI_TRUE )
		{
			theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_VERIFY_SUCCESS, (LPCTSTR)m_sName );
		}
		else if ( m_bVerify == TRI_FALSE )
		{
			m_bShared = TRI_FALSE;

			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_VERIFY_FAIL, (LPCTSTR)m_sName );

			return FALSE;
		}
	}

	AddAlternateSources( pRecent->m_sSources );

	// Notify library plugins
	if ( Plugins.OnNewFile( this ) )
		return TRUE;

	// Notify all windows about this file
	if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
	{
		CChildWnd* pChildWnd = NULL;
		while ( ( pChildWnd = pMainWnd->m_pWindows.Find( NULL, pChildWnd ) ) != NULL )
		{
			if ( pChildWnd->OnNewFile( this ) )		// ILibraryFile plugin notification
				return TRUE;
		}
	}

	// Notify version checker
	if ( VersionChecker.CheckUpgradeHash( this ) )
		return TRUE;

	return TRUE;
}

// PrepareDoc is used for collections "multi-filepicker" item processing.

BOOL CLibraryFile::PrepareDoc(LPCTSTR pszTemplate, CArray< CString >& oDocs) const
{
	ASSUME_LOCK( Library.m_pSection );

	CString strDoc( pszTemplate );

	if ( m_pMetadata && m_pSchema )
	{
		// Should be all meta data replacement
		const CXMLElement* pMetadata = m_pMetadata;
		for ( POSITION pos = pMetadata->GetAttributeIterator(); pos; )
		{
			const CXMLNode* pNode = pMetadata->GetNextAttribute( pos );
			CString str = pNode->GetName();
			CString strReplace = pNode->GetValue();
			if ( str == L"seconds" || str == L"minutes" )
			{
				double nTotalSecs = ( str == L"minutes" ) ?
					_tstof( (LPCTSTR)strReplace ) * 60 : _tstof( (LPCTSTR)strReplace );
				int nSecs = (int)nTotalSecs;
				int nHours = nSecs / 3600;
				nSecs -= nHours * 3600;
				int nMins = nSecs / 60;
				nSecs -= nMins * 60;

				str.Format( L"%d", nHours );
				ReplaceNoCase( strDoc, L"$meta:hours$", str );
				str.Format( L"%d", nMins );
				ReplaceNoCase( strDoc, L"$meta:minutes$", str );
				str.Format( L"%d", nSecs );
				ReplaceNoCase( strDoc, L"$meta:seconds$", str );

				if ( nHours )
					str.Format( L"%d:%d:%.2d", nHours, nMins, nSecs );
				else
					str.Format( L"%d:%.2d", nMins, nSecs );
				ReplaceNoCase( strDoc, L"$meta:time$", str );
			}
			else if ( str == "track" )
			{
				int nTrack = _ttoi( (LPCTSTR)strReplace );
				str.Format( L"%d", nTrack );
				ReplaceNoCase( strDoc, L"$meta:track$", str );
			}
			else
			{
				CString strOld;
				strOld.Format( L"$meta:%s$", (LPCTSTR)str );
				ReplaceNoCase( strDoc, strOld, strReplace );
			}
		}
	}

	CString strFileName, strNameURI, strSize, strMagnet;

	if ( m_sName )
	{
		strFileName = m_sName;
		strNameURI = URLEncode( m_sName );
	}

	if ( m_nSize != SIZE_UNKNOWN )
	{
		strSize.Format( L"%I64u", m_nSize ); // bytes
		ReplaceNoCase( strDoc, L"$meta:sizebytes$", strSize );

		CString strHumanSize;
		if ( m_nSize / ( 1024*1024 ) > 1 )
			strHumanSize.Format( L"%.2f MB", (float)m_nSize / 1024 / 1024 );
		else
			strHumanSize.Format( L"%.2f KB", (float)m_nSize / 1024 );

		ReplaceNoCase( strDoc, L"$meta:size$", strHumanSize );
	}

	if ( m_oSHA1 )
	{
		strMagnet = L"xt=urn:sha1:" + m_oSHA1.toString();

		ReplaceNoCase( strDoc, L"$meta:sha1$", m_oSHA1.toString() );
		ReplaceNoCase( strDoc, L"$meta:gnutella$", L"gnutella://urn:sha1:" + m_oSHA1.toString() + L'/' + strNameURI + L'/' );
	}

	if ( m_oTiger )
	{
		strMagnet = L"xt=urn:tree:tiger/:" + m_oTiger.toString();

		ReplaceNoCase( strDoc, L"$meta:tiger$", m_oTiger.toString() );
	}

	if ( m_oSHA1 && m_oTiger )
	{
		strMagnet = L"xt=urn:bitprint:" + m_oSHA1.toString() + L'.' + m_oTiger.toString();

		ReplaceNoCase( strDoc, L"$meta:bitprint$", m_oSHA1.toString() + L'.' + m_oTiger.toString() );
	}

	if ( m_oED2K )
	{
		if ( ! strMagnet.IsEmpty() ) strMagnet += L"&amp;";
		strMagnet += L"xt=urn:ed2khash:" + m_oED2K.toString();

		ReplaceNoCase( strDoc, L"$meta:ed2khash$", m_oED2K.toString() );
		if ( ! strSize.IsEmpty() )
			ReplaceNoCase( strDoc, L"$meta:ed2k$", L"ed2k://|file|" + strNameURI + L'|' + strSize + L'|' + m_oED2K.toString() + L"|/" );
	}

	if ( m_oMD5 )
	{
		if ( ! strMagnet.IsEmpty() ) strMagnet += L"&amp;";
		strMagnet += L"xt=urn:md5:" + m_oMD5.toString();

		ReplaceNoCase( strDoc, L"$meta:md5$", m_oMD5.toString() );
	}

	if ( m_oBTH )
	{
		if ( ! strMagnet.IsEmpty() ) strMagnet += L"&amp;";
		strMagnet += L"xt=urn:btih:" + m_oMD5.toString();

		ReplaceNoCase( strDoc, L"$meta:btih$", m_oBTH.toString() );
	}

	if ( ! strMagnet.IsEmpty() ) strMagnet += L"&amp;xl=" + strSize;
	strMagnet = L"magnet:?" + strMagnet + L"&amp;dn=" + strNameURI;
	ReplaceNoCase( strDoc, L"$meta:magnet$", strMagnet );

	ReplaceNoCase( strDoc, L"$meta:name$", strFileName );
	if ( ! m_sComments.IsEmpty() )
		ReplaceNoCase( strDoc, L"$meta:comments$", m_sComments );

	CString strNumber;
	strNumber.Format( L"%d", oDocs.GetCount() + 1 );
	ReplaceNoCase( strDoc, L"$meta:number$", strNumber );

	// Replace all "$meta:xxx$" which were left in the file to "--"
	while ( LPCTSTR szStart = StrStrI( strDoc, L"$meta:" ) )
	{
		if ( LPCTSTR szEnd = StrChr( szStart + 6, L'$' ) )
			strDoc.Replace( CString( szStart, szEnd - szStart + 1 ), L"--" );
		else
			break;
	}

	oDocs.Add( strDoc );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSharedSource construction

CSharedSource::CSharedSource(LPCTSTR pszURL, FILETIME* pTime)
{
	ZeroMemory( &m_pTime, sizeof( m_pTime ) );

	if ( pszURL != NULL )
	{
		m_sURL = pszURL;
		Freshen( pTime );
	}
}

void CSharedSource::Serialize(CArchive& ar, int /*nVersion*/)
{
	if ( ar.IsStoring() )
	{
		ar << m_sURL;
		ar.Write( &m_pTime, sizeof( FILETIME ) );
	}
	else // Loading
	{
		ar >> m_sURL;
		ReadArchive( ar, &m_pTime, sizeof( FILETIME ) );
	}
}

void CSharedSource::Freshen(FILETIME* pTime)
{
	SYSTEMTIME tNowSys;
	GetSystemTime( &tNowSys);

	if ( pTime != NULL )
	{
		FILETIME tNowFile;
		SystemTimeToFileTime( &tNowSys, &tNowFile );
		(LONGLONG&)tNowFile += 10000000;

		if ( CompareFileTime( pTime, &tNowFile ) <= 0 )
			m_pTime = *pTime;
		else
			SystemTimeToFileTime( &tNowSys, &m_pTime );
	}
	else
	{
		SystemTimeToFileTime( &tNowSys, &m_pTime );
	}
}

BOOL CSharedSource::IsExpired(FILETIME& tNow)
{
	LONGLONG nElapse = *((LONGLONG*)&tNow) - *((LONGLONG*)&m_pTime);
	return nElapse > (LONGLONG)Settings.Library.SourceExpire * 10000000;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile automation

IMPLEMENT_DISPATCH(CLibraryFile, LibraryFile)

STDMETHODIMP CLibraryFile::XLibraryFile::get_Hash(URN_TYPE nType, ENCODING nEncoding, BSTR FAR* psURN)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xEnvyFile.get_Hash( nType, nEncoding, psURN );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_URL(BSTR FAR* psURL)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xEnvyFile.get_URL( psURL );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Magnet(BSTR FAR* psMagnet)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xEnvyFile.get_Magnet( psMagnet );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Folder(ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	if ( pThis->m_pFolder )
		*ppFolder = (ILibraryFolder*)pThis->m_pFolder->GetInterface( IID_ILibraryFolder, TRUE );
	else
		*ppFolder = NULL;
	return *ppFolder != NULL ? S_OK : S_FALSE;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Path(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*psPath = CComBSTR( pThis->GetPath() ).Detach();
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xEnvyFile.get_Name( psName );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Shared(TRISTATE FAR* pnValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pnValue = pThis->m_bShared;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::put_Shared(TRISTATE nValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	pThis->m_bShared = nValue;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_EffectiveShared(VARIANT_BOOL FAR* pbValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pbValue = pThis->IsShared() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Size(ULONGLONG FAR* pnSize)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pnSize = pThis->GetSize();
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Index(LONG FAR* pnIndex)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pnIndex = (LONG)pThis->m_nIndex;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_URN(BSTR sURN, BSTR FAR* psURN)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xEnvyFile.get_URN( sURN, psURN );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_MetadataAuto(VARIANT_BOOL FAR* pbValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pbValue = pThis->m_bMetadataAuto ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Metadata(ISXMLElement FAR* FAR* ppXML)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*ppXML = NULL;

	CQuickLock oLock( Library.m_pSection );

	if ( pThis->m_pSchema == NULL )
		return S_OK;

	CXMLElement* pXML	= pThis->m_pSchema->Instantiate( TRUE );
	*ppXML				= (ISXMLElement*)CXMLCOM::Wrap( pXML, IID_ISXMLElement );

	if ( pThis->m_pMetadata )
		pXML->AddElement( pThis->m_pMetadata->Clone() );

	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::put_Metadata(ISXMLElement FAR* pXML)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )

	CQuickLock oLock( Library.m_pSection );

	if ( CXMLElement* pReal = CXMLCOM::Unwrap( pXML ) )
		return pThis->SetMetadata( pReal ) ? S_OK : E_FAIL;

	pThis->ClearMetadata();
	return S_OK;
}

// Note no SmartExecute needed

STDMETHODIMP CLibraryFile::XLibraryFile::Execute()
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return CFileExecutor::Execute( pThis->GetPath() ) ? S_OK : E_FAIL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Delete()
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->Delete() ? S_OK : E_FAIL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Rename(BSTR sNewName)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->Rename( CString( sNewName ) ) ? S_OK : E_FAIL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Copy(BSTR /*sNewPath*/)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Move(BSTR /*sNewPath*/)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::MergeMetadata(ISXMLElement* pXML, VARIANT_BOOL bOverwrite, VARIANT_BOOL* pbValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )

	if ( ! pbValue )
		return E_POINTER;

	if ( CXMLElement* pReal = CXMLCOM::Unwrap( pXML ) )
	{
		*pbValue = pThis->MergeMetadata( pReal, bOverwrite ) ?
			VARIANT_TRUE : VARIANT_FALSE;
	}
	else
	{
		*pbValue = VARIANT_FALSE;
	}

	return S_OK;
}
