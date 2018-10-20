//
// WebServices.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2011-2014
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

// Note: Consolidated library external web service interfaces
// Bitprints (Bitzi), MusicBrainz, legacy ShareMonkey
// Moved from CtrlLibraryFileView, CFileExecutor


#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "WebServices.h"
#include "FileExecutor.h"
#include "Skin.h"
#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "SharedFile.h"
#include "Library.h"
#include "DlgBitprintsDownload.h"
//#include "ShareMonkeyData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Move from CLibraryFileView?
//BEGIN_MESSAGE_MAP(CWebServices)
//	ON_UPDATE_COMMAND_UI(ID_LIBRARY_BITPRINTS_WEB, OnUpdateLibraryBitprintsWeb)
//	ON_COMMAND(ID_LIBRARY_BITPRINTS_WEB, OnLibraryBitprintsWeb)
//	ON_UPDATE_COMMAND_UI(ID_LIBRARY_BITPRINTS_DOWNLOAD, OnUpdateLibraryBitprintsDownload)
//	ON_COMMAND(ID_LIBRARY_BITPRINTS_DOWNLOAD, OnLibraryBitprintsDownload)
//	ON_UPDATE_COMMAND_UI(ID_WEBSERVICES_MUSICBRAINZ, OnUpdateMusicBrainzLookup)
//	ON_COMMAND(ID_WEBSERVICES_MUSICBRAINZ, OnMusicBrainzLookup)
//	ON_UPDATE_COMMAND_UI(ID_MUSICBRAINZ_MATCHES, OnUpdateMusicBrainzMatches)
//	ON_COMMAND(ID_MUSICBRAINZ_MATCHES, OnMusicBrainzMatches)
//	ON_UPDATE_COMMAND_UI(ID_MUSICBRAINZ_ALBUMS, OnUpdateMusicBrainzAlbums)
//	ON_COMMAND(ID_MUSICBRAINZ_ALBUMS, OnMusicBrainzAlbums)
//	ON_UPDATE_COMMAND_UI(ID_WEBSERVICES_SHAREMONKEY, OnUpdateShareMonkeyLookup)
//	ON_COMMAND(ID_WEBSERVICES_SHAREMONKEY, OnShareMonkeyLookup)
//	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_DOWNLOAD, OnUpdateShareMonkeyDownload)
//	ON_COMMAND(ID_SHAREMONKEY_DOWNLOAD, OnShareMonkeyDownload)
//	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_SAVE, OnUpdateShareMonkeySave)
//	ON_COMMAND(ID_SHAREMONKEY_SAVE, OnShareMonkeySave)
//	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_SAVE_OPTION, OnUpdateShareMonkeySaveOption)
//	ON_COMMAND(ID_SHAREMONKEY_SAVE_OPTION, OnShareMonkeySaveOption)
//	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_PREVIOUS, OnUpdateShareMonkeyPrevious)
//	ON_COMMAND(ID_SHAREMONKEY_PREVIOUS, OnShareMonkeyPrevious)
//	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_NEXT, OnUpdateShareMonkeyNext)
//	ON_COMMAND(ID_SHAREMONKEY_NEXT, OnShareMonkeyNext)
//	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_PRICES, OnUpdateShareMonkeyPrices)
//	ON_COMMAND(ID_SHAREMONKEY_PRICES, OnShareMonkeyPrices)
//	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_COMPARE, OnUpdateShareMonkeyCompare)
//	ON_COMMAND(ID_SHAREMONKEY_COMPARE, OnShareMonkeyCompare)
//	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_BUY, OnUpdateShareMonkeyBuy)
//	ON_COMMAND(ID_SHAREMONKEY_BUY, OnShareMonkeyBuy)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////
// CtrlLibraryFileView Web Services Handling
//
// Note: Moved from CLibraryFileView

//void CWebServices::ClearServicePages()
//{
//	for ( POSITION pos = m_pServiceDataPages.GetHeadPosition(); pos; )
//	{
//		CMetaList* pPanelData = m_pServiceDataPages.GetNext( pos );
//		delete pPanelData;
//	}
//
//	m_pServiceDataPages.RemoveAll();
//	m_nCurrentPage = 0;
//	m_bServiceFailed = FALSE;
//
//	GetFrame()->SetPanelData( NULL );
//}


/////////////////////////////////////////////////////////////////////
// BitprintsTicket Services

//void CWebServices::OnUpdateLibraryBitprintsWeb(CCmdUI* pCmdUI)
//{
//	if ( m_bGhostFolder )
//		pCmdUI->Enable( FALSE );
//	else
//		pCmdUI->Enable( GetSelectedCount() == 1 && ! Settings.WebServices.BitprintsWebSubmit.IsEmpty() );
//}

//void CWebServices::OnLibraryBitprintsWeb()
//{
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//
//	if ( CLibraryFile* pFile = GetSelectedFile() )
//	{
//		DWORD nIndex = pFile->m_nIndex;
//		pLock.Unlock();
//		ShowBitprintsTicket( nIndex );
//	}
//}

//void CWebServices::OnUpdateLibraryBitprintsDownload(CCmdUI* pCmdUI)
//{
//	if ( m_bGhostFolder || m_bRequestingService )
//		pCmdUI->Enable( FALSE );
//	else
//		pCmdUI->Enable( GetSelectedCount() > 0 && ! Settings.WebServices.BitprintsXML.IsEmpty() );
//}

//void WebServices::OnLibraryBitprintsDownload()
//{
//	GetFrame()->SetDynamicBar( NULL );
//
//	if ( ! Settings.WebServices.BitprintsOkay )
//	{
//		if ( MsgBox( IDS_BITPRINTS_MESSAGE, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
//		Settings.WebServices.BitprintsOkay = true;
//		Settings.Save();
//	}
//
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//	CBitprintsDownloadDlg dlg;
//
//	POSITION posSel = StartSelectedFileLoop();
//	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
//	{
//		if ( pFile->m_oSHA1 )
//			dlg.AddFile( pFile->m_nIndex );
//	}
//
//	pLock.Unlock();
//
//	dlg.DoModal();
//}


/////////////////////////////////////////////////////////////////////
// MusicBrainz Services

//void CWebServices::OnUpdateMusicBrainzLookup(CCmdUI* pCmdUI)
//{
//	if ( m_bGhostFolder || GetSelectedCount() != 1 || m_bRequestingService )
//	{
//		pCmdUI->Enable( FALSE );
//		return;
//	}
//
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//
//	CLibraryFile* pFile = GetSelectedFile();
//	if ( ! pFile->IsSchemaURI( CSchema::uriAudio ) || pFile->m_pMetadata == NULL )
//	{
//		pCmdUI->Enable( FALSE );
//		return;
//	}
//
//	CMetaList* pMetaList = new CMetaList();
//	pMetaList->Setup( pFile->m_pSchema, FALSE );
//	pMetaList->Combine( pFile->m_pMetadata );
//
//	pCmdUI->Enable( pMetaList->IsMusicBrainz() );
//
//	delete pMetaList;
//}

//void CWebServices::OnMusicBrainzLookup()
//{
//	CLibraryFrame* pFrame = GetFrame();
//	pFrame->SetDynamicBar( L"WebServices.MusicBrainz" );
//}

// Called when the selection changes
//void CWebServices::CheckDynamicBar()
//{
//	bool bIsMusicBrainz = false;
//	ClearServicePages();
//
//	CLibraryFrame* pFrame = GetFrame();
//	if ( _tcscmp( pFrame->GetDynamicBarName(), L"WebServices.MusicBrainz" ) == 0 )
//		bIsMusicBrainz = true;
//
//	if ( GetSelectedCount() != 1 )
//	{
//		if ( bIsMusicBrainz )
//		{
//			pFrame->SetDynamicBar( NULL );
//			m_bRequestingService = FALSE;	// ToDo: Abort operation
//		}
//		return;
//	}
//
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//	CLibraryFile* pFile = GetSelectedFile();
//
//	if ( pFile == NULL )	// Ghost file
//	{
//		pFrame->SetDynamicBar( NULL );
//		m_bRequestingService = FALSE;
//		return;
//	}
//
//	if ( ! pFile->IsSchemaURI( CSchema::uriAudio ) || pFile->m_pMetadata == NULL )
//	{
//		if ( bIsMusicBrainz )
//			pFrame->SetDynamicBar( NULL );
//
//		m_bRequestingService = FALSE;	// ToDo: Abort operation
//		return;
//	}
//
//	CMetaList* pMetaList = new CMetaList();
//	pMetaList->Setup( pFile->m_pSchema, FALSE );
//	pMetaList->Combine( pFile->m_pMetadata );
//
//	if ( ! pMetaList->IsMusicBrainz() && bIsMusicBrainz )
//		pFrame->SetDynamicBar( NULL );
//	else
//		pFrame->HideDynamicBar();
//
//	m_bRequestingService = FALSE;	// ToDo: Abort operation
//	delete pMetaList;
//
//	pLock.Unlock();
//}

//void CWebServices::OnUpdateMusicBrainzMatches(CCmdUI* pCmdUI)
//{
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//
//	CLibraryFile* pFile = GetSelectedFile();
//
//	ASSERT( pFile->m_pMetadata != NULL );
//
//	CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbpuid" );
//	pCmdUI->Enable( pAttribute != NULL && ! pAttribute->GetValue().IsEmpty() );
//}

//void CWebServices::OnMusicBrainzMatches()
//{
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//
//	CLibraryFile* pFile = GetSelectedFile();
//	ASSERT( pFile->m_pMetadata != NULL );
//
//	CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbpuid" );
//	CString strURL = L"http://musicbrainz.org/show/puid/?matchesonly=0&amp;puid=" + pAttribute->GetValue();
//
//	ShellExecute( GetSafeHwnd(), L"open", strURL, NULL, NULL, SW_SHOWNORMAL );
//}

//void CWebServices::OnUpdateMusicBrainzAlbums(CCmdUI* pCmdUI)
//{
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//	CLibraryFile* pFile = GetSelectedFile();
//
//	ASSERT( pFile->m_pMetadata != NULL );
//
//	CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbartistid" );
//	pCmdUI->Enable( pAttribute != NULL && ! pAttribute->GetValue().IsEmpty() );
//}

//void CWebServices::OnMusicBrainzAlbums()
//{
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//
//	CLibraryFile* pFile = GetSelectedFile();
//	ASSERT( pFile->m_pMetadata != NULL );
//
//	CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbartistid" );
//	CString strURL = L"http://musicbrainz.org/artist/" + pAttribute->GetValue();
//
//	ShellExecute( GetSafeHwnd(), L"open", strURL, NULL, NULL, SW_SHOWNORMAL );
//}


/////////////////////////////////////////////////////////////////////
// ShareMonkey Services  (Obsolete site, for reference)
//
// Note: Moved from CtrlLibraryFileView

//void CWebServices::OnUpdateShareMonkeyLookup(CCmdUI* pCmdUI)
//{
//	pCmdUI->Enable( GetSelectedCount() == 1 && ! m_bRequestingService );
//}

//void CWebServices::OnShareMonkeyLookup()
//{
//	GetFrame()->SetDynamicBar( L"WebServices.ShareMonkey.WithSave" );
//}

//void CWebServices::OnUpdateShareMonkeyDownload(CCmdUI* pCmdUI)
//{
//	pCmdUI->Enable( ! m_bRequestingService && m_pServiceDataPages.GetCount() == 0 );
//}

//void CWebServices::OnShareMonkeyDownload()
//{
//	if ( ! Settings.WebServices.ShareMonkeyOkay )
//	{
//		if ( MsgBox( IDS_SHAREMONKEY_MESSAGE, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
//		Settings.WebServices.ShareMonkeyOkay = true;
//		Settings.Save();
//	}
//
//	CShareMonkeyData* pPanelData = new CShareMonkeyData( m_nCurrentPage );
//
//	CString strStatus;
//	LoadString( strStatus, IDS_TIP_STATUS );
//	strStatus.TrimRight( ':' );
//	pPanelData->Add( strStatus, L"Please wait..." );
//
//	if ( m_nCurrentPage == 0 )
//		ClearServicePages();
//	GetFrame()->SetPanelData( pPanelData );
//
//	m_pServiceDataPages.AddTail( pPanelData );
//
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//	CLibraryFile* pFile = GetSelectedFile();
//
//	// Should be set to FALSE, and abort button created
//	m_bRequestingService = pFile != NULL;
//	if ( m_bRequestingService )
//	{
//		DWORD nIndex = pFile->m_nIndex;
//		pLock.Unlock();
//		pPanelData->Start( this, nIndex );
//		return;
//	}
//
//	pLock.Unlock();
//}

//void CWebServices::OnUpdateShareMonkeySave(CCmdUI* pCmdUI)
//{
//	BOOL bShow = TRUE;
//	if ( m_bServiceFailed && m_nCurrentPage == m_pServiceDataPages.GetCount() - 1 )
//		bShow = FALSE;
//	pCmdUI->Enable( bShow && ! m_bRequestingService && m_pServiceDataPages.GetCount() > 0 );
//}

//void CWebServices::OnShareMonkeySave()
//{
//	INT_PTR nCurr = 0;
//	CShareMonkeyData* pPanelData = NULL;
//
//	for ( POSITION pos = m_pServiceDataPages.GetHeadPosition(); pos; nCurr++ )
//	{
//		if ( m_nCurrentPage == nCurr )
//		{
//			pPanelData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
//			break;
//		}
//	}
//
//	if ( pPanelData == NULL ) return;
//
//	CSingleLock pLock( &Library.m_pSection, TRUE );
//
//	CLibraryFile* pFile = GetSelectedFile();
//	CSchemaPtr pSchema = pFile->m_pSchema ? pFile->m_pSchema : pPanelData->GetSchema();
//
//	if ( pSchema )
//	{
//		CXMLElement* pRoot = pSchema->Instantiate( TRUE );
//		CXMLElement* pXML = NULL;
//
//		if ( pFile->m_pMetadata )
//		{
//			pXML = pFile->m_pMetadata->Clone();
//			pRoot->AddElement( pXML );
//		}
//		else
//			pXML = pRoot->AddElement( pSchema->m_sSingular );
//
//		CXMLAttribute* pTitle = new CXMLAttribute( NULL, L"title" );
//		pXML->AddAttribute( pTitle );
//
//		CXMLAttribute* pDescription = NULL;
//		if ( pSchema->CheckURI( CSchema::uriApplication ) )
//		{
//			pDescription = new CXMLAttribute( NULL, L"fileDescription" );
//			pXML->AddAttribute( pDescription );
//		}
//		else if ( pSchema->CheckURI( CSchema::uriArchive ) )
//		{
//			// No description... There should be games
//		}
//		else
//		{
//			pDescription = new CXMLAttribute( NULL, L"description" );
//			pXML->AddAttribute( pDescription );
//		}
//
//		if ( pTitle )
//			pTitle->SetValue( pPanelData->m_sProductName );
//		if ( pDescription )
//			pDescription->SetValue( pPanelData->m_sDescription );
//
//		pFile->SetMetadata( pRoot );
//		delete pRoot;
//	}
//}

//void CWebServices::OnUpdateShareMonkeySaveOption(CCmdUI* pCmdUI)
//{
//	OnUpdateShareMonkeySave( pCmdUI );
//}

//void CWebServices::OnShareMonkeySaveOption()
//{
//	Settings.WebServices.ShareMonkeySaveThumbnail = ! Settings.WebServices.ShareMonkeySaveThumbnail;
//}

//void CWebServices::OnUpdateShareMonkeyPrevious(CCmdUI* pCmdUI)
//{
//	pCmdUI->Enable( ! m_bRequestingService && m_nCurrentPage > 0 );
//}

//void CWebServices::OnShareMonkeyPrevious()
//{
//	INT_PTR nCurr = m_nCurrentPage--;
//	CShareMonkeyData* pPanelData = NULL;
//
//	POSITION pos = m_pServiceDataPages.GetHeadPosition();
//
//	while ( pos && nCurr-- )
//	{
//		pPanelData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
//	}
//
//	GetFrame()->SetPanelData( pPanelData );
//}

//void CWebServices::OnUpdateShareMonkeyNext(CCmdUI* pCmdUI)
//{
//	BOOL bShow = TRUE;
//	if ( m_bServiceFailed && m_nCurrentPage == m_pServiceDataPages.GetCount() - 1 )
//		bShow = FALSE;
//	pCmdUI->Enable( bShow && ! m_bRequestingService && m_pServiceDataPages.GetCount() > 0 );
//}

//void CWebServices::OnShareMonkeyNext()
//{
//	INT_PTR nCurr = ++m_nCurrentPage;
//	nCurr++;
//
//	if ( m_nCurrentPage > m_pServiceDataPages.GetCount() - 1 )
//	{
//		OnShareMonkeyDownload();
//	}
//	else
//	{
//		CShareMonkeyData* pPanelData = NULL;
//		POSITION pos = m_pServiceDataPages.GetHeadPosition();
//
//		while ( pos && nCurr-- )
//		{
//			pPanelData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
//		}
//
//		GetFrame()->SetPanelData( pPanelData );
//	}
//}

//void CWebServices::OnUpdateShareMonkeyPrices(CCmdUI* pCmdUI)
//{
//	BOOL bShow = TRUE;
//	if ( m_bServiceFailed && m_pServiceDataPages.GetCount() == 1 || m_pServiceDataPages.GetCount() == 0 )
//		bShow = FALSE;
//	pCmdUI->Enable( ! m_bRequestingService && bShow );
//}

//void CWebServices::OnShareMonkeyPrices()
//{
//	POSITION pos = m_pServiceDataPages.GetHeadPosition();
//	CShareMonkeyData* pData = NULL;
//
//	// ToDo: Change m_pServiceDataPages to CMap. Now it's stupid
//	for ( INT_PTR nPage = 0; nPage <= m_nCurrentPage; nPage++ )
//	{
//		pData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
//	}
//
//	if ( pData && ! pData->m_pChild )
//	{
//		CShareMonkeyData* pChild = new CShareMonkeyData( 0, CShareMonkeyData::stStoreMatch );
//		pData->m_pChild = pChild;
//		CString strStatus;
//		LoadString( strStatus, IDS_TIP_STATUS );
//		strStatus.TrimRight( ':' );
//		pChild->Add( strStatus, L"Please wait..." );
//		pChild->m_sSessionID = pData->m_sSessionID;
//		pChild->m_sProductID = pData->m_sProductID;
//		pChild->m_sThumbnailURL = pData->m_sThumbnailURL;
//
//		GetFrame()->SetPanelData( pChild );
//		pChild->Start( this, 0 );
//	}
//	else
//	{
//		GetFrame()->SetPanelData( pData ? pData->m_pChild : NULL );
//	}
//}

//void CWebServices::OnUpdateShareMonkeyCompare(CCmdUI* pCmdUI)
//{
//	BOOL bShow = TRUE;
//	if ( m_bServiceFailed && m_pServiceDataPages.GetCount() == 1 || m_pServiceDataPages.GetCount() == 0 )
//		bShow = FALSE;
//	pCmdUI->Enable( ! m_bRequestingService && bShow );
//}

//void CWebServices::OnShareMonkeyCompare()
//{
//	POSITION pos = m_pServiceDataPages.GetHeadPosition();
//	CShareMonkeyData* pData = NULL;
//
//	// ToDo: Change m_pServiceDataPages to CMap. Now it's stupid
//	for ( INT_PTR nPage = 0; nPage <= m_nCurrentPage; nPage++ )
//	{
//		pData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
//	}
//
//	if ( pData && pData->m_sComparisonURL.GetLength() )
//		ShellExecute( GetSafeHwnd(), L"open", pData->m_sComparisonURL, NULL, NULL, SW_SHOWNORMAL );
//}

//void CWebServices::OnUpdateShareMonkeyBuy(CCmdUI* pCmdUI)
//{
//	pCmdUI->Enable( ! m_bServiceFailed && ! m_bRequestingService && m_pServiceDataPages.GetCount() > 0 );
//}

//void CWebServices::OnShareMonkeyBuy()
//{
//	POSITION pos = m_pServiceDataPages.GetHeadPosition();
//	CShareMonkeyData* pData = NULL;
//
//	// ToDo: Change m_pServiceDataPages to CMap. Now it's stupid
//	for ( INT_PTR nPage = 0; nPage <= m_nCurrentPage; nPage++ )
//	{
//		pData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
//	}
//
//	if ( pData && pData->m_sBuyURL.GetLength() )
//		ShellExecute( GetSafeHwnd(), L"open", pData->m_sBuyURL, NULL, NULL, SW_SHOWNORMAL );
//}


//////////////////////////////////////////////////////////////////////
// CWebServices show Bitprints listing	(Format URL)
//
// Note: Moved from CFileExecutor

BOOL CWebServices::ShowBitprintsTicket(DWORD nIndex)
{
	if ( ! Settings.WebServices.BitprintsOkay )
	{
		if ( MsgBox( IDS_BITPRINTS_MESSAGE, MB_ICONQUESTION|MB_YESNO ) != IDYES )
			return FALSE;
		Settings.WebServices.BitprintsOkay = true;
		Settings.Save();
	}

	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryFile* pFile = Library.LookupFile( nIndex );
	if ( pFile == NULL ) return FALSE;

	if ( ! pFile->m_oSHA1 || ! pFile->m_oTiger || ! pFile->m_oED2K )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_BITPRINTS_NOT_HASHED ), (LPCTSTR)pFile->m_sName );
		pLock.Unlock();
		MsgBox( strMessage, MB_ICONINFORMATION );
		return FALSE;
	}

	CString str, strURL = Settings.WebServices.BitprintsWebView;

	CFile hFile;
	if ( hFile.Open( pFile->GetPath(), CFile::modeRead|CFile::shareDenyNone ) )
	{
		strURL = Settings.WebServices.BitprintsWebSubmit;

		if ( hFile.GetLength() > 0 )
		{
			static LPCTSTR pszHex = L"0123456789ABCDEF";
			BYTE nBuffer[20];
			int nPeek = hFile.Read( nBuffer, 20 );
			hFile.Close();

			for ( int nByte = 0; nByte < nPeek; nByte++ )
			{
				str += pszHex[ (BYTE)nBuffer[ nByte ] >> 4 ];
				str += pszHex[ (BYTE)nBuffer[ nByte ] & 15 ];
			}

			strURL.Replace( L"(FIRST20)", str );
		}
		else
			strURL.Replace( L"(FIRST20)", L"0" );
	}
	else
		strURL.Replace( L"(URN)", pFile->m_oSHA1.toString() + L"." + pFile->m_oTiger.toString() );

	CString strName = pFile->m_sName;
	LPCTSTR pszExt = _tcsrchr( strName, '.' );
	int nExtLen = pszExt ? static_cast< int >( _tcslen( pszExt ) - 1 ) : 0;
	const CString strExt = strName.Right( nExtLen ).Trim().MakeUpper();

	strURL.Replace( L"(NAME)", URLEncode( strName ) );
	strURL.Replace( L"(SHA1)", pFile->m_oSHA1.toString() );
	strURL.Replace( L"(TTH)", pFile->m_oTiger.toString() );
	strURL.Replace( L"(ED2K)", pFile->m_oED2K.toString() );
	strURL.Replace( L"(AGENT)", URLEncode( Settings.SmartAgent() ) );

	str.Format( L"%I64i", pFile->GetSize() );
	strURL.Replace( L"(SIZE)", str );

	CString strINFO = L"&tag.tiger.tree=" + pFile->m_oTiger.toString();
	if ( pFile->m_oMD5 )
		strINFO += L"&tag.md5.md5=" + pFile->m_oMD5.toString();
	if ( ! pFile->m_sComments.Trim().IsEmpty() )
		strINFO += L"&tag.subjective.comment=" + URLEncode( pFile->m_sComments );

//	BOOL bAudioSchema = FALSE, bVideoSchema = FALSE, bImageSchema = FALSE, bAppSchema = FALSE;
//	if ( pFile->m_pSchema->CheckURI( CSchema::uriAudio ) )
//		bAudioSchema = TRUE;
//	else if ( pFile->m_pSchema->CheckURI( CSchema::uriVideo ) )
//		bVideoSchema = TRUE;
//	else if ( pFile->m_pSchema->CheckURI( CSchema::uriImage ) )
//		bImageSchema = TRUE;
//	else if ( pFile->m_pSchema->CheckURI( CSchema::uriApplication ) )
//		bAppSchema = TRUE;

	if ( pFile->m_pMetadata != NULL && pFile->m_pSchema != NULL )
	{
		CXMLElement* pMetadata = pFile->m_pMetadata;
		int nTemp, nAudioTag = 0, nImageTag = 0;
		CString strDescription, strAudioTag, strImageTag;

		for ( POSITION pos = pMetadata->GetAttributeIterator(); pos; )
		{
			CString strReplace;
			CXMLNode* pNode = pMetadata->GetNextAttribute( pos );
			str = pNode->GetName();
			strReplace = pNode->GetValue();
			str.MakeLower();

			if ( str == L"link" )
			{
				strINFO += L"&tag.url.url=" + URLEncode( strReplace );
			}
			else if ( str == L"description" )
			{
				strDescription = URLEncode( strReplace.Trim() );	// L"&tag.objective.description=" +
			}
			else if ( pFile->m_pSchema->CheckURI( CSchema::uriAudio ) )
			{
				if ( str == L"title" )
					strINFO += L"&tag.audiotrack.title=" + URLEncode( strReplace.Trim() );
				else if ( str == L"artist" )
					strINFO += L"&tag.audiotrack.artist=" + URLEncode( strReplace.Trim() );
				else if ( str == L"album" )
					strINFO += L"&tag.audiotrack.album=" + URLEncode( strReplace.Trim() );
				else if ( str == L"track" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( L"%d", nTemp );

					strINFO += L"&tag.audiotrack.tracknumber=" + strReplace;
				}
				else if ( str == L"year" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( L"%d", nTemp );

					strINFO += L"&tag.audiotrack.year=" + strReplace;
				}
				else if ( strExt == L"MP3" || strExt == L"OGG" || strExt == L"WAV" )
				{
					// ToDo: Read WAV information in FileExecutor.cpp, Bitprints submit is already ready
					if ( str == L"bitrate" )
					{
						if ( strExt == L"MP3" )
						{
							strAudioTag += L"&tag.mp3.vbr=";

							if ( _tcsstr( strReplace, L"~" ) )
								strAudioTag += L"y";
							else
								strAudioTag += L"n";
						}

						nTemp = _ttoi( strReplace );
						strReplace.Format( L"%d", nTemp );

						if ( strExt == L"MP3" )
							strAudioTag += L"&tag.mp3.bitrate=";
						else if ( strExt == L"OGG" )
							strAudioTag += L"&tag.vorbis.bitrate=";
						else
							strReplace.Empty();

						if ( ! strReplace.IsEmpty() )
						{
							strAudioTag += strReplace;
							nAudioTag++;
						}
					}
					// ToDo: Read sampleSize of WAV in FileExecutor.cpp, Bitprints submit is already ready
					else if ( str == L"sampleSize" )
					{
						nTemp = _ttoi( strReplace );
						strReplace.Format( L"%d", nTemp );

						if ( strExt == L"WAV" )
						{
							strAudioTag += L"&tag.wav.samplesize=" + strReplace;
							nAudioTag++;
						}
					}
					else if ( str == L"seconds" )
					{
						nTemp = (int)( _wtof( strReplace ) * 1000 );
						strReplace.Format( L"%d", nTemp );

						if ( strExt == L"MP3" )
							strAudioTag += L"&tag.mp3.duration=";
						else if ( strExt == L"OGG" )
							strAudioTag += L"&tag.vorbis.duration=";
						else if ( strExt == L"WAV" )
							strAudioTag += L"&tag.wav.duration=";
						else
							strReplace.Empty();

						if ( ! strReplace.IsEmpty() )
						{
							strAudioTag += strReplace;
							nAudioTag++;
						}
					}
					else if ( str == L"sampleRate" )
					{
						nTemp = _ttoi( strReplace );
						strReplace.Format( L"%d", nTemp );

						if ( strExt == L"MP3" )
							strAudioTag += L"&tag.mp3.samplerate=";
						else if ( strExt == L"OGG" )
							strAudioTag += L"&tag.vorbis.samplerate=";
						else if ( strExt == L"WAV" )
							strAudioTag += L"&tag.wav.samplerate=";
						else
							strReplace.Empty();

						if ( ! strReplace.IsEmpty() )
						{
							strAudioTag += strReplace;
							nAudioTag++;
						}
					}
					else if ( str == L"channels" )
					{
						nTemp = _ttoi( strReplace );
						strReplace.Format( L"%d", nTemp );

						if ( strExt == L"OGG" )
							strAudioTag += L"&tag.vorbis.channels=";
						else if ( strExt == L"WAV" )
							strAudioTag += L"&tag.wav.channels=";
						else
							strReplace.Empty();

						if ( ! strReplace.IsEmpty() )
						{
							strAudioTag += strReplace;
							nAudioTag++;
						}
					}
					else if ( str == L"soundType" )
					{
						if ( strExt == L"MP3" )
						{
							if ( strReplace.CompareNoCase( L"Stereo" ) == 0 || strReplace.CompareNoCase( L"Joint Stereo" ) == 0 || strReplace.CompareNoCase( L"Dual Channel" ) == 0 )
								strAudioTag += L"&tag.mp3.stereo=y";
							else if ( strReplace.CompareNoCase( L"Single Channel" ) == 0 || strReplace.CompareNoCase( L"Mono" ) == 0 )
								strAudioTag += L"&tag.mp3.stereo=n";
							else
								strReplace.Empty();
						}

						if ( ! strReplace.IsEmpty() )
							nAudioTag++;
					}
					else if ( str == L"encoder" )
					{
						if ( strExt == L"MP3" )
							strAudioTag += L"&tag.mp3.encoder=";
						else if ( strExt == L"OGG" )
							strAudioTag += L"&tag.vorbis.encoder=";
						else
							strReplace.Empty();

						if ( ! strReplace.IsEmpty() )
							strAudioTag += URLEncode( strReplace );
					}
				}
			}
			else if ( pFile->m_pSchema->CheckURI( CSchema::uriImage ) )
			{
				if ( str == L"width" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( L"%d", nTemp );

					strImageTag += L"&tag.image.width=" + strReplace;
					nImageTag++;
				}
				else if ( str == L"height" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( L"%d", nTemp );

					strImageTag += L"&tag.image.height=" + strReplace;
					nImageTag++;
				}
				else if ( str == L"colors" )
				{
					if ( strReplace == L"2" ) strReplace = L"1";
					else if ( strReplace == L"16" ) strReplace = L"4";
					else if ( strReplace == L"256" || strReplace == L"Greyscale" ) strReplace = L"8";
					else if ( strReplace == L"64K" ) strReplace = L"16";
					else if ( strReplace == L"16.7M" ) strReplace = L"24";
					else strReplace.Empty();

					if ( ! strReplace.IsEmpty() )
					{
						strImageTag += L"&tag.image.bpp=" + strReplace;
						nImageTag++;
					}
				}
			}
			else if ( pFile->m_pSchema->CheckURI( CSchema::uriVideo ) )
			{
				if ( str == L"realdescription" )
				{
					strDescription = URLEncode( strReplace.Trim() );	// L"&tag.objective.description=" +
				}
				else if ( str == L"width" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( L"%d", nTemp );

					strINFO += L"&tag.video.width=" + strReplace;
				}
				else if ( str == L"height" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( L"%d", nTemp );

					strINFO += L"&tag.video.height=" + strReplace;
				}
				else if ( str == L"frameRate" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( L"%d", nTemp );

					strINFO += L"&tag.video.fps=" + strReplace;
				}
				else if ( str == L"minutes" )
				{
					nTemp = (int)( _wtof( strReplace ) * 60 * 1000 );
					strReplace.Format( L"%d", nTemp );

					strINFO += L"&tag.video.duration=" + strReplace;
				}
				else if ( str == L"bitrate" )
				{
					// ToDo: Read video's bitrate in FileExecutor.cpp, Bitprints submit is already ready
					nTemp = _ttoi( strReplace );
					strReplace.Format( L"%d", nTemp );

					strINFO += L"&tag.video.bitrate=" + strReplace;
				}
				else if ( str == L"codec" )
				{
					strReplace.MakeUpper();
					strINFO += L"&tag.video.codec=" + URLEncode( strReplace );
				}
			}
			else if ( pFile->m_pSchema->CheckURI( CSchema::uriApplication ) )
			{
				if ( str == L"filedescription" )
					strDescription = URLEncode( strReplace.Trim() );
				else if ( str == L"title" && strDescription.IsEmpty() )
					strDescription = URLEncode( strReplace.Trim() );
			}
		} // end for pos

		if ( nAudioTag == 4 )
			strINFO += strAudioTag;
		else if ( nImageTag == 3 )
			strINFO += strImageTag;		// Handle extensions below

		if ( ! strDescription.IsEmpty() )
			strINFO += L"&tag.objective.description=" + strDescription;
	} // end if

	// Video Extensions		// +Images	ToDo: Handle any other types?
	if ( ! strExt.IsEmpty() )
	{
		SwitchMap( FileExt )
		{
			FileExt[ L"AVI" ]	= 'a';
			FileExt[ L"DIVX" ]	= 'v';
			FileExt[ L"XVID" ]	= 'x';
			FileExt[ L"MKV" ]	= 'k';
			FileExt[ L"WEBM" ]	= 'e';
			FileExt[ L"HDMOV" ]	= 'q';
			FileExt[ L"MOV" ]	= 'q';
			FileExt[ L"QT" ] 	= 'q';
			FileExt[ L"MPG" ]	= 'm';
			FileExt[ L"MPEG" ]	= 'm';
			FileExt[ L"MPE" ]	= 'm';
			FileExt[ L"M1V" ]	= '1';
			FileExt[ L"MP2" ]	= '2';
			FileExt[ L"M2V" ]	= '2';
			FileExt[ L"M2TS" ]	= '2';
			FileExt[ L"MPV2" ]	= '2';
			FileExt[ L"MP4" ]	= '4';
			FileExt[ L"M4V" ]	= '4';
			FileExt[ L"WMV" ]	= 'w';
			FileExt[ L"WM" ] 	= 'w';
			FileExt[ L"ASF" ]	= 'w';
			FileExt[ L"RM" ] 	= 'r';
			FileExt[ L"RMVB" ]	= 'r';
			FileExt[ L"RAM" ]	= 'r';
			FileExt[ L"OGM" ]	= 'o';
			FileExt[ L"OGV" ]	= 'o';
			FileExt[ L"VOB" ]	= 'b';
			FileExt[ L"VP6" ]	= '6';
			FileExt[ L"3GP" ]	= 'g';
			FileExt[ L"IVF" ]	= 'i';
			FileExt[ L"FLV" ]	= 'f';
			FileExt[ L"SWF" ]	= 'f';

			FileExt[ L"PNG" ]	= 'P';
			FileExt[ L"GIF" ]	= 'P';
			FileExt[ L"BMP" ]	= 'P';
			FileExt[ L"PSD" ]	= 'P';
			FileExt[ L"TIF" ]	= 'P';
			FileExt[ L"ICO" ]	= 'P';
			FileExt[ L"WEBP" ]	= 'P';
			FileExt[ L"SVG" ]	= 'P';
			FileExt[ L"JPG" ]	= 'J';
			FileExt[ L"JPEG" ]	= 'J';
			FileExt[ L"JPE" ]	= 'J';
			FileExt[ L"JFIF" ]	= 'J';

			FileExt[ L"MP3" ]	= 'X';
			FileExt[ L"AAC" ]	= 'X';
			FileExt[ L"FLAC" ]	= 'X';
			FileExt[ L"PDF" ]	= 'X';
			FileExt[ L"ZIP" ]	= 'X';
			FileExt[ L"RAR" ]	= 'X';
			FileExt[ L"EXE" ]	= 'X';
		}

		switch ( FileExt[ strExt ] )
		{
		case 'a':		// avi
			strINFO += L"&tag.video.format=AVI";
			break;
		case 'k':		// mkv
			strINFO += L"&tag.video.format=Matroska";
			break;
		case 'q':		// mov hdmov qt
			strINFO += L"&tag.video.format=QuickTime";
			break;
		case 'r':		// rm rmvb rv ram rpm?
			strINFO += L"&tag.video.format=Real";
			break;
		case 'm':		// mpg mpeg mpe
			strINFO += L"&tag.video.format=MPEG";
			break;
		case '1':		// m1v
			strINFO += L"&tag.video.format=MPEG-1";
			break;
		case '2':		// mp2 m2p mpv2 m2ts
			strINFO += L"&tag.video.format=MPEG-2";
			break;
		case '4':		// mp4 m4v
			strINFO += L"&tag.video.format=MPEG-4";
			break;
		case 'v':		// divx (div/tix?)
			strINFO += L"&tag.video.format=DivX";
			break;
		case 'x':		// xvid
			strINFO += L"&tag.video.format=XviD";
			break;
		case 'w':		// wmv wm wmd? asf
			strINFO += L"&tag.video.format=" + URLEncode( L"Windows Media" );
			break;
		case 'o':		// ogm ogv
			strINFO += L"&tag.video.format=" + URLEncode( L"Ogg Media File" );
			break;
		case 'g':		// 3gp
			strINFO += L"&tag.video.format=3GP";
			break;
		case '6':		// vp6
			strINFO += L"&tag.video.format=VP6";
			break;
		case 'b':		// vob
			strINFO += L"&tag.video.format=DVD";
			break;
		case 'i':		// ivf
			strINFO += L"&tag.video.format=Indeo";
			break;
		case 'f':		// swf flv
			strINFO += L"&tag.video.format=Flash";
			break;
		case 'e':		// webm
			strINFO += L"&tag.video.format=WebM";
			break;

		case 'P':		// png gif bmp psd tif webp ico svg
			strINFO += L"&tag.image.format=" + strExt;
			break;
		case 'J':		// jpg jpeg jpe jfif
			strINFO += L"&tag.image.format=JPEG";
			break;

		case 'X':		// Skip common non-video types
			break;

		default:		// Unknown extension, try video schema
			if ( pFile->m_pSchema != NULL && pFile->m_pSchema->CheckURI( CSchema::uriVideo ) )
				strINFO += L"&tag.video.format=" + URLEncode( strExt );
		}
	}

	pLock.Unlock();

	strURL.Replace( L"&(INFO)", strINFO );

	ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), L"open", strURL, NULL, NULL, SW_SHOWNORMAL );

	return TRUE;
}
