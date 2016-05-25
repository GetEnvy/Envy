//
// CtrlLibraryFileView.cpp
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
#include "CtrlLibraryFileView.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryTip.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "AlbumFolder.h"
#include "FileExecutor.h"
#include "CoolInterface.h"
#include "DlgFilePropertiesSheet.h"
#include "DlgFileCopy.h"
#include "DlgURLCopy.h"
#include "DlgURLExport.h"
#include "DlgDeleteFile.h"
#include "DlgDecodeMetadata.h"
#include "DlgBitprintsDownload.h"
#include "WebServices.h"
#include "RelatedSearch.h"
#include "Transfers.h"
#include "Security.h"
#include "Schema.h"
#include "Shell.h"
#include "Skin.h"
#include "XML.h"

//#include "ShareMonkeyData.h"	// Legacy

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CLibraryFileView, CLibraryView)

BEGIN_MESSAGE_MAP(CLibraryFileView, CLibraryView)
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_LAUNCH_FOLDER, OnUpdateLibraryLaunchFolder)
	ON_COMMAND(ID_LIBRARY_LAUNCH_FOLDER, OnLibraryLaunchFolder)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_LAUNCH, OnUpdateLibraryLaunch)
	ON_COMMAND(ID_LIBRARY_LAUNCH, OnLibraryLaunch)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_ENQUEUE, OnUpdateLibraryEnqueue)
	ON_COMMAND(ID_LIBRARY_ENQUEUE, OnLibraryEnqueue)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_URI, OnUpdateLibraryURL)
	ON_COMMAND(ID_LIBRARY_URI, OnLibraryURL)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_MOVE, OnUpdateLibraryMove)
	ON_COMMAND(ID_LIBRARY_MOVE, OnLibraryMove)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_COPY, OnUpdateLibraryCopy)
	ON_COMMAND(ID_LIBRARY_COPY, OnLibraryCopy)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_DELETE, OnUpdateLibraryDelete)
	ON_COMMAND(ID_LIBRARY_DELETE, OnLibraryDelete)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REFRESH_METADATA, OnUpdateLibraryRefreshMetadata)
	ON_COMMAND(ID_LIBRARY_REFRESH_METADATA, OnLibraryRefreshMetadata)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_SHARED_FILE, OnUpdateLibraryShared)
	ON_COMMAND(ID_LIBRARY_SHARED_FILE, OnLibraryShared)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_PROPERTIES, OnUpdateLibraryProperties)
	ON_COMMAND(ID_LIBRARY_PROPERTIES, OnLibraryProperties)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_UNLINK, OnUpdateLibraryUnlink)
	ON_COMMAND(ID_LIBRARY_UNLINK, OnLibraryUnlink)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_THIS, OnUpdateSearchForThis)
	ON_COMMAND(ID_SEARCH_FOR_THIS, OnSearchForThis)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_SIMILAR, OnUpdateSearchForSimilar)
	ON_COMMAND(ID_SEARCH_FOR_SIMILAR, OnSearchForSimilar)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_ARTIST, OnUpdateSearchForArtist)
	ON_COMMAND(ID_SEARCH_FOR_ARTIST, OnSearchForArtist)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_ALBUM, OnUpdateSearchForAlbum)
	ON_COMMAND(ID_SEARCH_FOR_ALBUM, OnSearchForAlbum)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_SERIES, OnUpdateSearchForSeries)
	ON_COMMAND(ID_SEARCH_FOR_SERIES, OnSearchForSeries)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_CREATETORRENT, OnUpdateLibraryCreateTorrent)
	ON_COMMAND(ID_LIBRARY_CREATETORRENT, OnLibraryCreateTorrent)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REBUILD_ANSI, OnUpdateLibraryRebuildAnsi)
	ON_COMMAND(ID_LIBRARY_REBUILD_ANSI, OnLibraryRebuildAnsi)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REBUILD_FILE, OnUpdateLibraryRebuild)
	ON_COMMAND(ID_LIBRARY_REBUILD_FILE, OnLibraryRebuild)
	ON_MESSAGE(WM_METADATA, OnServiceDone)

	// Web Services 	ToDo: Move Bitprints(Bitzi)/MusicBrainz out to CWebServices?
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_BITPRINTS_WEB, OnUpdateLibraryBitprintsWeb)
	ON_COMMAND(ID_LIBRARY_BITPRINTS_WEB, OnLibraryBitprintsWeb)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_BITPRINTS_DOWNLOAD, OnUpdateLibraryBitprintsDownload)
	ON_COMMAND(ID_LIBRARY_BITPRINTS_DOWNLOAD, OnLibraryBitprintsDownload)
	ON_UPDATE_COMMAND_UI(ID_WEBSERVICES_MUSICBRAINZ, OnUpdateMusicBrainzLookup)
	ON_COMMAND(ID_WEBSERVICES_MUSICBRAINZ, OnMusicBrainzLookup)
	ON_UPDATE_COMMAND_UI(ID_MUSICBRAINZ_MATCHES, OnUpdateMusicBrainzMatches)
	ON_COMMAND(ID_MUSICBRAINZ_MATCHES, OnMusicBrainzMatches)
	ON_UPDATE_COMMAND_UI(ID_MUSICBRAINZ_ALBUMS, OnUpdateMusicBrainzAlbums)
	ON_COMMAND(ID_MUSICBRAINZ_ALBUMS, OnMusicBrainzAlbums)
	// Legacy ShareMonkey for reference:
	//ON_UPDATE_COMMAND_UI(ID_WEBSERVICES_SHAREMONKEY, &CWebServices::OnUpdateShareMonkeyLookup)
	//ON_COMMAND(ID_WEBSERVICES_SHAREMONKEY, &CWebServices::OnShareMonkeyLookup)
	//ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_DOWNLOAD, &CWebServices::OnUpdateShareMonkeyDownload)
	//ON_COMMAND(ID_SHAREMONKEY_DOWNLOAD, &CWebServices::OnShareMonkeyDownload)
	//ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_SAVE, &CWebServices::OnUpdateShareMonkeySave)
	//ON_COMMAND(ID_SHAREMONKEY_SAVE, &CWebServices::OnShareMonkeySave)
	//ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_SAVE_OPTION, &CWebServices::OnUpdateShareMonkeySaveOption)
	//ON_COMMAND(ID_SHAREMONKEY_SAVE_OPTION, &CWebServices::OnShareMonkeySaveOption)
	//ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_PREVIOUS, &CWebServices::OnUpdateShareMonkeyPrevious)
	//ON_COMMAND(ID_SHAREMONKEY_PREVIOUS, &CWebServices::OnShareMonkeyPrevious)
	//ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_NEXT, &CWebServices::OnUpdateShareMonkeyNext)
	//ON_COMMAND(ID_SHAREMONKEY_NEXT, &CWebServices::OnShareMonkeyNext)
	//ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_PRICES, &CWebServices::OnUpdateShareMonkeyPrices)
	//ON_COMMAND(ID_SHAREMONKEY_PRICES, &CWebServices::OnShareMonkeyPrices)
	//ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_COMPARE, &CWebServices::OnUpdateShareMonkeyCompare)
	//ON_COMMAND(ID_SHAREMONKEY_COMPARE, &CWebServices::OnShareMonkeyCompare)
	//ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_BUY, &CWebServices::OnUpdateShareMonkeyBuy)
	//ON_COMMAND(ID_SHAREMONKEY_BUY, &CWebServices::OnShareMonkeyBuy)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryFileView construction

CLibraryFileView::CLibraryFileView()
	: m_bRequestingService	( FALSE )
	, m_bServiceFailed		( FALSE )
	, m_nCurrentPage		( 0 )
{
	m_pszToolBar = L"CLibraryFileView";
}

CLibraryFileView::~CLibraryFileView()
{
	for ( POSITION pos = m_pServiceDataPages.GetHeadPosition() ; pos ; )
	{
		delete m_pServiceDataPages.GetNext( pos );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryFileView selected item interface

BOOL CLibraryFileView::CheckAvailable(CLibraryTreeItem* pSel)
{
	if ( pSel == NULL )
		m_bAvailable = FALSE;
	else if ( pSel->m_pSelNext == NULL && pSel->m_pVirtual != NULL )
		m_bAvailable = ( pSel->m_pVirtual->GetFileCount() > 0 );
	else
		m_bAvailable = TRUE;

	return m_bAvailable;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryFileView message handlers

int CLibraryFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CLibraryView::OnCreate( lpCreateStruct ) == -1 ) return -1;
	m_bEditing = FALSE;
	return 0;
}

BOOL CLibraryFileView::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && ! m_bEditing )
	{
		switch ( pMsg->wParam )
		{
		case VK_RETURN:
			OnLibraryLaunch();
			return TRUE;
		case VK_DELETE:
			OnLibraryDelete();
			return TRUE;
		case VK_ESCAPE:
			GetParent()->PostMessage( WM_COMMAND, ID_LIBRARY_PARENT );
			return TRUE;
		}
	}
	else if ( pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_RETURN )
	{
		OnLibraryProperties();
		return TRUE;
	}

	return CLibraryView::PreTranslateMessage( pMsg );
}

void CLibraryFileView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	GetToolTip()->Hide();

	CStringList oFiles;
	{
		CQuickLock pLock( Library.m_pSection );
		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			oFiles.AddTail( pFile->GetPath() );
		}
	}

	if ( oFiles.GetCount() == 0 )
	{
		// No files were selected, try folder itself
		if ( CLibraryTreeItem* pRoot = GetFolderSelection() )
		{
			if ( pRoot->m_pPhysical )
				oFiles.AddTail( pRoot->m_pPhysical->m_sPath );
		}
	}

	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	CString strName( m_pszToolBar );
//	strName += Settings.Library.ShowVirtual ? L".Virtual" : L".Physical";		// For now, CLibraryFileView.Virtual = CLibraryFileView.Physical

	Skin.TrackPopupMenu( strName, point, ID_LIBRARY_LAUNCH, oFiles );
}

void CLibraryFileView::OnMouseMove(UINT nFlags, CPoint point)
{
	CLibraryView::OnMouseMove( nFlags, point );

	if ( DWORD nFile = (DWORD)HitTestIndex( point ) )	// DWORD_PTR
		GetToolTip()->Show( nFile );
	else
		GetToolTip()->Hide();
}

// Inherit from CLibraryView
//void CLibraryFileView::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
//{
//	GetToolTip()->Hide();
//
//	if ( nButton == 1 )
//		GetParent()->SendMessage( WM_COMMAND, ID_LIBRARY_PARENT );
//}

void CLibraryFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CLibraryView::OnKeyDown( nChar, nRepCnt, nFlags );

	CheckDynamicBar();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryFileView command handlers

void CLibraryFileView::OnUpdateLibraryLaunch(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryLaunch()
{
	GetToolTip()->Hide();

	CStringList oList;

	{
		CSingleLock oLock( &Library.m_pSection );
		if ( ! oLock.Lock( 250 ) ) return;

		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			oList.AddTail( pFile->GetPath() );
		}
	}

	CFileExecutor::Execute( oList );
}

void CLibraryFileView::OnUpdateLibraryEnqueue(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryEnqueue()
{
	CStringList pList;

	{
		CSingleLock oLock( &Library.m_pSection );
		if ( ! oLock.Lock( 250 ) ) return;

		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			pList.AddTail( pFile->GetPath() );
		}
	}

	CFileExecutor::Enqueue( pList );
}

void CLibraryFileView::OnUpdateLibraryLaunchFolder(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryLaunchFolder()
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		CString strPath = pFile->GetPath();
		pLock.Unlock();
		ShellExecute( GetSafeHwnd(), NULL, L"Explorer.exe", L"/select, " + strPath, NULL, SW_SHOWNORMAL );
		pLock.Lock();
	}
}

void CLibraryFileView::OnUpdateLibraryURL(CCmdUI* pCmdUI)
{
	const bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
	const INT_PTR nCount = GetSelectedCount();

	pCmdUI->Enable( nCount > 0 );
	pCmdUI->SetText( LoadString( ( nCount == 1 && ! bShift ) ? IDS_LIBRARY_URI_COPY : IDS_LIBRARY_URI_EXPORT ) );
}

void CLibraryFileView::OnLibraryURL()
{
	const bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
	const INT_PTR nCount = GetSelectedCount();
	if ( ! nCount ) return;

	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	if ( nCount == 1 && ! bShift )
	{
		const CLibraryFile* pFile = GetSelectedFile();
		if ( ! pFile ) return;

		CURLCopyDlg dlg;

		dlg.Add( pFile );

		pLock.Unlock();

		dlg.DoModal();
	}
	else //if ( GetSelectedCount() > 0 )
	{
		CURLExportDlg dlg;

		POSITION posSel = StartSelectedFileLoop();
		while ( const CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			dlg.Add( pFile );
		}

		pLock.Unlock();

		dlg.DoModal();
	}
}

void CLibraryFileView::OnUpdateLibraryMove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryMove()
{
	CFileCopyDlg dlg( NULL, TRUE );

	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		dlg.AddFile( pFile );
	}

	pLock.Unlock();

	dlg.DoModal();
}

void CLibraryFileView::OnUpdateLibraryCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryCopy()
{
	CFileCopyDlg dlg( NULL, FALSE );

	CSingleLock pLock( &Library.m_pSection, TRUE );
	if ( ! SafeLock( pLock ) ) return;

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		dlg.AddFile( pFile );
	}

	pLock.Unlock();

	dlg.DoModal();
}

void CLibraryFileView::OnUpdateLibraryDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryDelete()
{
	CSingleLock pLibraryLock( &Library.m_pSection, TRUE );

	CLibraryListPtr pList( new CLibraryList() );

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel, FALSE, ! m_bGhostFolder ) )
	{
		pList->AddTail( pFile );
	}

	pLibraryLock.Unlock();
	CSingleLock pTransfersLock( &Transfers.m_pSection );	// Can clear uploads and downloads
	if ( ! SafeLock( pTransfersLock ) ) return;
	if ( ! SafeLock( pLibraryLock ) ) return;

	while ( ! pList->IsEmpty() )
	{
		CLibraryFile* pFile = Library.LookupFile( pList->GetHead(), FALSE, ! m_bGhostFolder );
		if ( pFile == NULL )
		{
			pList->RemoveHead();	// Remove item from list to avoid endless loop
			continue;
		}

		if ( m_bGhostFolder )
		{
			for ( INT_PTR nProcess = pList->GetCount() ; nProcess > 0 && pList->GetCount() > 0 ; nProcess-- )
			{
				if ( ( pFile = Library.LookupFile( pList->RemoveHead() ) ) != NULL )
					pFile->Delete( TRUE );
			}
		}
		else
		{
			CDeleteFileDlg dlg( this );
			dlg.m_sName = pFile->m_sName;
			dlg.m_sComments = pFile->m_sComments;
			dlg.m_nRateValue = pFile->m_nRating;
			dlg.m_bAll = pList->GetCount() > 1;

			pLibraryLock.Unlock();
			pTransfersLock.Unlock();

			if ( dlg.DoModal() != IDOK ) break;

			pTransfersLock.Lock();
			pLibraryLock.Lock();

			for ( INT_PTR nProcess = dlg.m_bAll ? pList->GetCount() : 1 ; nProcess > 0 && pList->GetCount() > 0 ; nProcess-- )
			{
				if ( ( pFile = Library.LookupFile( pList->RemoveHead(), FALSE, TRUE ) ) != NULL )
				{
					dlg.Apply( pFile );
					pFile->Delete();
				}
			}
		}

		Library.Update( true );
	}
}

void CLibraryFileView::OnUpdateLibraryCreateTorrent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && Settings.BitTorrent.TorrentCreatorPath.GetLength() > 6 && GetSelectedCount() < 2 );
}

void CLibraryFileView::OnLibraryCreateTorrent()
{
	if ( GetSelectedCount() == 1 && Settings.BitTorrent.DefaultTracker.GetLength() > 10 )
	{
		CSingleLock pLock( &Library.m_pSection, TRUE );

		if ( CLibraryFile* pFile = GetSelectedFile() )
		{
			CString strPath = pFile->GetPath();
			pLock.Unlock();

			if ( ! strPath.IsEmpty() )
			{
				CString strCommandLine =
					L" -sourcefile \"" + strPath +
					L"\" -destination \"" + Settings.Downloads.TorrentPath +
					L"\" -tracker \"" + Settings.BitTorrent.DefaultTracker +
					L"\"";

				ShellExecute( GetSafeHwnd(), L"open", Settings.BitTorrent.TorrentCreatorPath, strCommandLine, NULL, SW_SHOWNORMAL );

				return;
			}
		}
	}

	ShellExecute( GetSafeHwnd(), L"open", Settings.BitTorrent.TorrentCreatorPath, NULL, NULL, SW_SHOWNORMAL );
}

void CLibraryFileView::OnUpdateLibraryRebuildAnsi(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() );
}

// Obsolete:
//void CLibraryFileView::OnUpdateLibraryRebuildAnsi(CCmdUI* pCmdUI)
//{
//	if ( m_bGhostFolder )
//	{
//		pCmdUI->Enable( FALSE );
//		return;
//	}
//
//	CQuickLock oLock( Library.m_pSection );
//
//	// Count only selected mp3's which have no custom metadata in XML format
//	INT_PTR nSelected = GetSelectedCount();
//	POSITION posSel = StartSelectedFileLoop();
//	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
//	{
//		CString strExtension = pFile->m_sName.Right(3);
//		ToLower( strExtension );
//
//		BOOL bXmlPossiblyModified = FALSE;
//		if ( ! pFile->m_bMetadataAuto )
//		{
//			WIN32_FIND_DATA fd = { 0 };
//			if ( GetFileAttributesEx( pFile->GetPath(), GetFileExInfoStandard, &fd ) )
//			{
//				ULARGE_INTEGER nMetaDataTime;
//				ULARGE_INTEGER nFileDataTime;
//
//				nFileDataTime.HighPart = fd.ftLastWriteTime.dwHighDateTime;
//				nFileDataTime.LowPart = fd.ftLastWriteTime.dwLowDateTime;
//				// Convert 100 ns into seconds
//				nFileDataTime.QuadPart /= 10000000;
//
//				nMetaDataTime.HighPart = pFile->m_pMetadataTime.dwHighDateTime;
//				nMetaDataTime.LowPart = pFile->m_pMetadataTime.dwLowDateTime;
//				nMetaDataTime.QuadPart /= 10000000;
//
//				// Assume that XML was not modified during the first 10 sec. of creation
//				if ( nMetaDataTime.HighPart == nFileDataTime.HighPart &&
//					nMetaDataTime.LowPart - nFileDataTime.LowPart > 10 )
//					bXmlPossiblyModified = TRUE;
//			}
//		}
//		if ( ( strExtension != L"mp3" && strExtension != L"avi" &&
//			   strExtension != L"pdf" && strExtension != L"mpc" &&
//			   strExtension != L"mpp" && strExtension != L"mp+" )
//			  || bXmlPossiblyModified )
//			nSelected--;
//	}
//
//	pCmdUI->Enable( nSelected > 0 );
//}

void CLibraryFileView::OnLibraryRebuildAnsi()
{
	CDecodeMetadataDlg dlg;

	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		dlg.AddFile( pFile );
	}

	pLock.Unlock();

	if ( dlg.m_pFiles.GetCount() )
		dlg.DoModal();
}

void CLibraryFileView::OnUpdateLibraryRebuild(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryRebuild()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION posSel = StartSelectedFileLoop();

	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		pFile->Rebuild();
	}

	Library.Update( true );
}

void CLibraryFileView::OnUpdateLibraryRefreshMetadata(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryRefreshMetadata()
{
	const DWORD nTotal = (DWORD)GetSelectedCount();		// INT_PTR

	if ( nTotal == 1 )
	{
		CQuickLock pLock( Library.m_pSection );
		LibraryBuilder.RefreshMetadata( GetSelectedFile()->GetPath() );
		return;
	}

	CProgressDialog dlgProgress( LoadString( ID_LIBRARY_REFRESH_METADATA ) + L"..." );

	CQuickLock pLock( Library.m_pSection );

	DWORD nCompleted = 0;
	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		CString strPath = pFile->GetPath();

		dlgProgress.Progress( strPath, nCompleted++, nTotal );

		LibraryBuilder.RefreshMetadata( strPath );
	}
}

void CLibraryFileView::OnUpdateLibraryProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryProperties()
{
	CFilePropertiesSheet dlg;
	//CStringList oFiles;

	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel, FALSE, FALSE ) )
	{
		dlg.Add( pFile );
		//oFiles.AddTail( pFile->GetPath() );
	}

	pLock.Unlock();

	//HRESULT hr;
	//CComPtr< IDataObject > pDataObject;
	//{
	//	// Convert path string list to PIDL list
	//	auto_array< PIDLIST_ABSOLUTE > pShellFileAbs( new PIDLIST_ABSOLUTE [ oFiles.GetCount() ] );
	//	for ( int i = 0 ; i < oFiles.GetCount() ; ++i )
	//	  pShellFileAbs[ i ] = ILCreateFromPath( oFiles.GetHead() );
	//
	//	PIDLIST_ABSOLUTE pShellParent = ILCloneFull( pShellFileAbs[ 0 ] );
	//	ILRemoveLastID( pShellParent );
	//
	//	auto_array< LPCITEMIDLIST > pShellFiles( new LPCITEMIDLIST [ oFiles.GetCount() ] );
	//	POSITION pos = oFiles.GetHeadPosition();
	//	for ( int i = 0 ; i < oFiles.GetCount() ; ++i )
	//		pShellFiles[ i ] = ILFindChild( pShellParent, pShellFileAbs[ i ] );
	//
	//	hr = CIDLData_CreateFromIDArray( pShellParent, oFiles.GetCount(),
	//		pShellFiles.get(), &pDataObject );
	//
	//	ILFree( pShellParent );
	//
	//	for ( int i = 0 ; i < oFiles.GetCount() ; ++i )
	//		ILFree( (LPITEMIDLIST)pShellFileAbs[ i ] );
	//}
	//if ( SUCCEEDED( hr ) )
	//	hr = SHMultiFileProperties( pDataObject, 0 );

	dlg.DoModal();
}

void CLibraryFileView::OnUpdateLibraryShared(CCmdUI* pCmdUI)
{
	if ( GetSelectedCount() < 1 )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
		return;
	}

	TRISTATE bShared = TRI_UNKNOWN;

	CSingleLock pLock( &Library.m_pSection );
	if ( pLock.Lock( 100 ) )
	{
		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			if ( bShared == TRI_UNKNOWN )
			{
				bShared = pFile->IsShared() ? TRI_TRUE : TRI_FALSE;
			}
			else if ( ( bShared == TRI_TRUE ) != pFile->IsShared() )
			{
				pCmdUI->Enable( FALSE );
				return;
			}
		}
		pLock.Unlock();
	}

	pCmdUI->Enable( TRUE );
	pCmdUI->SetCheck( bShared == TRI_TRUE );
}

void CLibraryFileView::OnLibraryShared()
{
	CQuickLock oLock( Library.m_pSection );

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		pFile->SetShared( ! pFile->IsShared() );
	}

	Library.Update();
}

void CLibraryFileView::OnUpdateLibraryUnlink(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
	{
		pCmdUI->Enable( FALSE );
		return;
	}
	CLibraryTreeItem* pItem = GetFolderSelection();
	pCmdUI->Enable( GetSelectedCount() > 0 && pItem && pItem->m_pVirtual && pItem->m_pSelNext == NULL );
}

void CLibraryFileView::OnLibraryUnlink()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryTreeItem* pItem = GetFolderSelection();

	if ( pItem == NULL || pItem->m_pVirtual == NULL || pItem->m_pSelNext != NULL ) return;

	CAlbumFolder* pFolder = pItem->m_pVirtual;
	if ( ! LibraryFolders.CheckAlbum( pFolder ) ) return;

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		pFolder->RemoveFile( pFile );
	}
}

void CLibraryFileView::OnUpdateSearchForThis(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForThis() );
}

void CLibraryFileView::OnSearchForThis()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForThis();
}

void CLibraryFileView::OnUpdateSearchForSimilar(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForSimilar() );
}

void CLibraryFileView::OnSearchForSimilar()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForSimilar();
}

void CLibraryFileView::OnUpdateSearchForArtist(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForArtist() );
}

void CLibraryFileView::OnSearchForArtist()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForArtist();
}

void CLibraryFileView::OnUpdateSearchForAlbum(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForAlbum() );
}

void CLibraryFileView::OnSearchForAlbum()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForAlbum();
}

void CLibraryFileView::OnUpdateSearchForSeries(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForSeries() );
}

void CLibraryFileView::OnSearchForSeries()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForSeries();
}


/////////////////////////////////////////////////////////////////////
// Web Services Handling

// ToDo: Move below Bitprints(Bitzi)/MusicBrainz to WebServices?

void CLibraryFileView::ClearServicePages()
{
	for ( POSITION pos = m_pServiceDataPages.GetHeadPosition() ; pos ; )
	{
		delete m_pServiceDataPages.GetNext( pos );
	}

	m_pServiceDataPages.RemoveAll();
	m_nCurrentPage = 0;
	m_bServiceFailed = FALSE;

	GetFrame()->SetPanelData( NULL );
}


/////////////////////////////////////////////////////////////////////
// Bitprints listing Services

void CLibraryFileView::OnUpdateLibraryBitprintsWeb(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() == 1 && Settings.WebServices.BitprintsWebSubmit.GetLength() );
}

void CLibraryFileView::OnLibraryBitprintsWeb()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		DWORD nIndex = pFile->m_nIndex;
		pLock.Unlock();
		CWebServices::ShowBitprintsTicket( nIndex );
	}
}

void CLibraryFileView::OnUpdateLibraryBitprintsDownload(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && ! m_bRequestingService && GetSelectedCount() && Settings.WebServices.BitprintsXML.GetLength() );
}

void CLibraryFileView::OnLibraryBitprintsDownload()
{
	GetFrame()->SetDynamicBar( NULL );

	if ( ! Settings.WebServices.BitprintsOkay )
	{
		if ( MsgBox( IDS_BITPRINTS_MESSAGE, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
		Settings.WebServices.BitprintsOkay = true;
		Settings.Save();
	}

	CBitprintsDownloadDlg dlg;

	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		if ( pFile->m_oSHA1 )
			dlg.AddFile( pFile->m_nIndex );
	}

	pLock.Unlock();

	dlg.DoModal();
}

/////////////////////////////////////////////////////////////////////
// MusicBrainz Services

void CLibraryFileView::OnUpdateMusicBrainzLookup(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder || GetSelectedCount() != 1 || m_bRequestingService )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	CLibraryFile* pFile = GetSelectedFile();
	if ( ! pFile->IsSchemaURI( CSchema::uriAudio ) || pFile->m_pMetadata == NULL )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	CMetaList* pMetaList = new CMetaList();
	pMetaList->Setup( pFile->m_pSchema, FALSE );
	pMetaList->Combine( pFile->m_pMetadata );

	pLock.Unlock();

	pCmdUI->Enable( pMetaList->IsMusicBrainz() );

	delete pMetaList;
}

void CLibraryFileView::OnMusicBrainzLookup()
{
	CLibraryFrame* pFrame = GetFrame();
	pFrame->SetDynamicBar( L"WebServices.MusicBrainz" );
}

// Called when the selection changes
void CLibraryFileView::CheckDynamicBar()
{
	bool bIsMusicBrainz = false;
	ClearServicePages();

	CLibraryFrame* pFrame = GetFrame();
	if ( _tcscmp( pFrame->GetDynamicBarName(), L"WebServices.MusicBrainz" ) == 0 )
		bIsMusicBrainz = true;

	if ( GetSelectedCount() != 1 )
	{
		if ( bIsMusicBrainz )
		{
			pFrame->SetDynamicBar( NULL );
			m_bRequestingService = FALSE;	// ToDo: Abort operation
		}
		return;
	}

	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryFile* pFile = GetSelectedFile();

	if ( pFile == NULL )	// Ghost file
	{
		pFrame->SetDynamicBar( NULL );
		m_bRequestingService = FALSE;
		return;
	}

	if ( ! pFile->IsSchemaURI( CSchema::uriAudio ) || pFile->m_pMetadata == NULL )
	{
		if ( bIsMusicBrainz )
			pFrame->SetDynamicBar( NULL );

		m_bRequestingService = FALSE;	// ToDo: Abort operation
		return;
	}

	CMetaList* pMetaList = new CMetaList();
	pMetaList->Setup( pFile->m_pSchema, FALSE );
	pMetaList->Combine( pFile->m_pMetadata );

	pLock.Unlock();

	if ( ! pMetaList->IsMusicBrainz() && bIsMusicBrainz )
		pFrame->SetDynamicBar( NULL );
	else
		pFrame->HideDynamicBar();

	m_bRequestingService = FALSE;	// ToDo: Abort operation
	delete pMetaList;
}

void CLibraryFileView::OnUpdateMusicBrainzMatches(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		if ( pFile->m_pMetadata )
		{
			if ( CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbpuid" ) )
			{
				pCmdUI->Enable( pAttribute != NULL && ! pAttribute->GetValue().IsEmpty() );
				return;
			}
		}
	}
	pCmdUI->Enable( FALSE );
}

void CLibraryFileView::OnMusicBrainzMatches()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		if ( pFile->m_pMetadata )
		{
			if ( CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbpuid" ) )
			{
				pLock.Unlock();
				CString mbpuid = pAttribute->GetValue();
				if ( ! mbpuid.IsEmpty() )
				{
					CString strURL = L"http://musicbrainz.org/show/puid/?matchesonly=0&amp;puid=" + mbpuid;
					ShellExecute( GetSafeHwnd(), L"open", strURL, NULL, NULL, SW_SHOWNORMAL );
				}
			}
		}
	}
}

void CLibraryFileView::OnUpdateMusicBrainzAlbums(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 200 ) ) return;

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		if ( pFile->m_pMetadata )
		{
			if ( CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbartistid" ) )
			{
				pCmdUI->Enable( pAttribute != NULL && ! pAttribute->GetValue().IsEmpty() );
				return;
			}
		}
	}
	pCmdUI->Enable( FALSE );
}

void CLibraryFileView::OnMusicBrainzAlbums()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		if ( pFile->m_pMetadata )
		{
			if ( CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbartistid" ) )
			{
				pLock.Unlock();
				CString mbartistid = pAttribute->GetValue();
				if ( ! mbartistid.IsEmpty() )
					ShellExecute( GetSafeHwnd(), L"open", L"http://musicbrainz.org/artist/" + mbartistid, NULL, NULL, SW_SHOWNORMAL );
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////
// ShareMonkey Services (Obsolete, moved to WebServices for reference)


////////////////////////////////////////////////////////////////////////////////
// Set pszMessage to NULL when the work is done, to remove the status item from meta panel.
// Don't do that in the middle or the order will change.
// The status must be the first item in the meta panel.

LRESULT CLibraryFileView::OnServiceDone(WPARAM wParam, LPARAM lParam)
{
	CString strStatus;
	LoadString( strStatus, IDS_TIP_STATUS );
	strStatus.TrimRight( ':' );

	LPCTSTR pszMessage = (LPCTSTR)lParam;
	CMetaList* pPanelData = (CMetaList*)wParam;

	m_bServiceFailed = FALSE;

	if ( pPanelData == NULL )
	{
		m_nCurrentPage = 0;
		m_bRequestingService = FALSE;
		ClearServicePages();
	}
	else if ( pszMessage == NULL )
	{
		pPanelData->Remove( strStatus );
	}
	else
	{
		CMetaItem* pItem = pPanelData->Find( strStatus );
		if ( pItem ) pItem->m_sValue = pszMessage;

		m_bServiceFailed = TRUE;
	}

	CLibraryFrame* pFrame = GetFrame();
	if ( pFrame->GetPanelData() != NULL )
		pFrame->SetPanelData( pPanelData );

	m_bRequestingService = FALSE;

	return 0;
}
