//
// WndUploads.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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
#include "Uploads.h"
#include "UploadQueues.h"
#include "UploadQueue.h"
#include "UploadFiles.h"
#include "UploadFile.h"
#include "UploadTransfer.h"
#include "UploadTransferED2K.h"
#include "Transfers.h"
#include "Downloads.h"
#include "EDClient.h"
#include "Library.h"
#include "FileExecutor.h"
#include "Security.h"
#include "Skin.h"

#include "WindowManager.h"
#include "WndUploads.h"
#include "WndDownloads.h"
#include "WndMain.h"
#include "WndLibrary.h"
#include "WndBrowseHost.h"
#include "ChatWindows.h"
#include "DlgSettingsManager.h"
#include "DlgQueueProperties.h"

#include "DlgHelp.h"
#include "LibraryDictionary.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_SERIAL(CUploadsWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CUploadsWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_MDIACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_DISCONNECT, OnUpdateUploadsDisconnect)
	ON_COMMAND(ID_UPLOADS_DISCONNECT, OnUploadsDisconnect)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_LAUNCH, OnUpdateUploadsLaunch)
	ON_COMMAND(ID_UPLOADS_LAUNCH, OnUploadsLaunch)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FOLDER, OnUpdateUploadsFolder)
	ON_COMMAND(ID_UPLOADS_FOLDER, OnUploadsFolder)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_CLEAR, OnUpdateUploadsClear)
	ON_COMMAND(ID_UPLOADS_CLEAR, OnUploadsClear)
	ON_COMMAND(ID_UPLOADS_CLEAR_COMPLETED, OnUploadsClearCompleted)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_AUTO_CLEAR, OnUpdateUploadsAutoClear)
	ON_COMMAND(ID_UPLOADS_AUTO_CLEAR, OnUploadsAutoClear)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_CHAT, OnUpdateUploadsChat)
	ON_COMMAND(ID_UPLOADS_CHAT, OnUploadsChat)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_BAN, OnUpdateSecurityBan)
	ON_COMMAND(ID_SECURITY_BAN, OnSecurityBan)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_START, OnUpdateUploadsStart)
	ON_COMMAND(ID_UPLOADS_START, OnUploadsStart)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_PRIORITY, OnUpdateUploadsPriority)
	ON_COMMAND(ID_UPLOADS_PRIORITY, OnUploadsPriority)
	ON_COMMAND(ID_UPLOADS_SETTINGS, OnUploadsSettings)
	ON_COMMAND(ID_UPLOADS_HELP, OnUploadsHelp)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_ALL, OnUpdateUploadsFilterAll)
	ON_COMMAND(ID_UPLOADS_FILTER_ALL, OnUploadsFilterAll)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_ACTIVE, OnUpdateUploadsFilterActive)
	ON_COMMAND(ID_UPLOADS_FILTER_ACTIVE, OnUploadsFilterActive)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_QUEUED, OnUpdateUploadsFilterQueued)
	ON_COMMAND(ID_UPLOADS_FILTER_QUEUED, OnUploadsFilterQueued)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_HISTORY, OnUpdateUploadsFilterHistory)
	ON_COMMAND(ID_UPLOADS_FILTER_HISTORY, OnUploadsFilterHistory)
	ON_COMMAND(ID_UPLOADS_FILTER_MENU, OnUploadsFilterMenu)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_TORRENT, OnUpdateUploadsFilterTorrent)
	ON_COMMAND(ID_UPLOADS_FILTER_TORRENT, OnUploadsFilterTorrent)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUploadsWnd construction

CUploadsWnd::CUploadsWnd() : CPanelWnd( Settings.General.GUIMode == GUI_TABBED, TRUE )
{
	Create( IDR_UPLOADSFRAME );
}

CUploadsWnd::~CUploadsWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CUploadsWnd message handlers

int CUploadsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndUploads.Create( this, IDC_UPLOADS );

	m_wndUploads.ModifyStyleEx( 0, WS_EX_COMPOSITED );	// Stop flicker XP+, CPU intensive

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndToolBar.SetSyncObject( &Transfers.m_pSection );

	LoadState( NULL, TRUE );

	SetTimer( 2, Settings.Interface.RefreshRateText, NULL );
	PostMessage( WM_TIMER, 2 );

	SetTimer( 4, 5000, NULL );
	PostMessage( WM_TIMER, 4 );

	m_tSel = 0;
	m_tLastUpdate = 0;

	return 0;
}

void CUploadsWnd::OnDestroy()
{
	KillTimer( 4 );
	SaveState();
	CPanelWnd::OnDestroy();
}

BOOL CUploadsWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndToolBar.m_hWnd &&
		 m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
		return TRUE;

	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CUploadsWnd::OnSize(UINT nType, int cx, int cy)
{
	SizeListAndBar( &m_wndUploads, &m_wndToolBar );
	CPanelWnd::OnSize( nType, cx, cy );
}

void CUploadsWnd::OnSkinChange()
{
	OnSize( 0, 0, 0 );
	CPanelWnd::OnSkinChange();
	Skin.Translate( L"CUploadCtrl", &m_wndUploads.m_wndHeader);
	Skin.CreateToolBar( L"CUploadsWnd", &m_wndToolBar );
	m_wndUploads.OnSkinChange();
}

void CUploadsWnd::OnTimer(UINT_PTR nIDEvent)
{
	// Reset Selection Timer event (posted by ctrluploads)
	if ( nIDEvent == 5 )
	{
		m_tSel = 0;
		return;
	}

	const DWORD tNow = GetTickCount();

	// Clear event (5 second timer)
	if ( nIDEvent == 4 )
	{
		CSingleLock pLock( &Transfers.m_pSection );
		if ( ! pLock.Lock( 10 ) ) return;

		//BOOL bCull = Uploads.GetCount( NULL ) > 50;	// Obsolete

		DWORD nCount = 0;

		for ( POSITION pos = Uploads.GetIterator(); pos; )
		{
			CUploadTransfer* pUpload = Uploads.GetNext( pos );

			if ( pUpload->m_nState != upsNull )
				continue;

			if ( ( tNow > pUpload->m_tConnected + Settings.Uploads.ClearDelay ) &&
				 ( Settings.Uploads.AutoClear || pUpload->m_nUploaded == 0 || nCount > Settings.Uploads.History ) )
				pUpload->Remove( FALSE );
			else
				nCount++;
		}
		return;
	}

	// Update event (2 second timer)
	if ( nIDEvent == 2 )
	{
		// If the window is visible or hasn't been updated in 10 seconds
		if ( ( IsWindowVisible() && IsActive( FALSE ) ) || tNow > m_tLastUpdate + 10*1000 )
		{
			m_wndUploads.Update();
			m_tLastUpdate = tNow;
		}
		return;
	}
}

void CUploadsWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
	{
		m_wndUploads.ClientToScreen( &point );
		Skin.TrackPopupMenu( L"CUploadsWnd.Default", point, ID_UPLOADS_HELP );
		return;
	}

	CPoint ptLocal( point );
	m_wndUploads.ScreenToClient( &ptLocal );
	m_tSel = 0;

	BOOL bHit = FALSE;

	CSingleLock pLock( &Transfers.m_pSection, FALSE );
	if ( pLock.Lock( 250 ) )
	{
		CUploadFile* pUpload;
		if ( m_wndUploads.HitTest( ptLocal, NULL, &pUpload, NULL, NULL ) && pUpload != NULL )
			bHit = TRUE;
		pLock.Unlock();
	}

	if ( bHit )
		Skin.TrackPopupMenu( L"CUploadsWnd.Upload", point, ID_UPLOADS_LAUNCH );
	else
		Skin.TrackPopupMenu( L"CUploadsWnd.Default", point, ID_UPLOADS_HELP );
}

void CUploadsWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );
	if ( bActivate )
		m_wndUploads.SetFocus();
}

BOOL CUploadsWnd::IsSelected(const CUploadFile* pFile) const
{
	if ( ! pFile->m_bSelected ) return FALSE;

	if ( const CUploadTransfer* pTransfer = pFile->GetActive() )
	{
		if ( pTransfer->m_nProtocol == PROTOCOL_BT )
		{
			if ( 0 == ( Settings.Uploads.FilterMask & ULF_TORRENT ) ) return FALSE;
		}
		else if ( pTransfer->m_pQueue != NULL )
		{
			if ( pTransfer->m_pQueue->m_bExpanded == FALSE ) return FALSE;

			if ( pTransfer->m_pQueue->IsActive( pTransfer ) )
			{
				if ( 0 == ( Settings.Uploads.FilterMask & ULF_ACTIVE ) ) return FALSE;
			}
			else
			{
				if ( 0 == ( Settings.Uploads.FilterMask & ULF_QUEUED ) ) return FALSE;
			}
		}
		else
		{
			if ( 0 == ( Settings.Uploads.FilterMask & ULF_HISTORY ) ) return FALSE;
		}
	}
	else
	{
		if ( 0 == ( Settings.Uploads.FilterMask & ULF_HISTORY ) ) return FALSE;
	}

	return TRUE;
}

void CUploadsWnd::Prepare()
{
	if ( GetTickCount() < m_tSel + Settings.Interface.RefreshRateUI )
		return;

	CSingleLock pLock( &Transfers.m_pSection, FALSE );
	if ( ! pLock.Lock( Settings.Interface.RefreshRateUI ) )
		return;

	m_tSel = GetTickCount();
	m_nSelected = 0;

	m_bSelFile = m_bSelUpload = FALSE;
	m_bSelActive = m_bSelQueued = FALSE;
	m_bSelChat = m_bSelBrowse = FALSE;
	m_bSelSourceAcceptConnections = FALSE;
	m_bSelPartial = TRUE;

	for ( POSITION posFile = UploadFiles.GetIterator(); posFile; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( posFile );

		if ( ! pFile->m_bSelected || ! IsSelected( pFile ) )
			continue;

		m_bSelFile = TRUE;

		if ( CUploadTransfer* pTransfer = pFile->GetActive() )
		{
			m_nSelected++;
			m_bSelUpload = TRUE;

			if ( pTransfer->m_bClientExtended )
			{
				m_bSelChat = TRUE;
				m_bSelBrowse = TRUE;
			}
			else if ( pTransfer->m_nProtocol == PROTOCOL_ED2K )
			{
				m_bSelChat = TRUE;
				CUploadTransferED2K* pTransferED2K = static_cast< CUploadTransferED2K* >( pTransfer );
				if ( pTransferED2K->m_pClient && pTransferED2K->m_pClient->m_bEmBrowse )
					m_bSelBrowse = TRUE;
			}

			if ( pTransfer->m_pQueue != NULL )
			{
				if ( pTransfer->m_pQueue->IsActive( pTransfer ) )
					m_bSelActive = TRUE;
				else
					m_bSelQueued = TRUE;
			}
			else if ( pTransfer->m_nState != upsNull )
			{
				m_bSelActive = TRUE;
			}

			if ( m_bSelPartial == TRUE )
			{
				if ( StartsWith( pFile->m_sPath, Settings.Downloads.IncompletePath ) )
					continue;

				if ( ! pFile->m_sPath.IsEmpty() )	// Not multifile torrent
					m_bSelPartial = FALSE;
				else if ( PathIsDirectory( Settings.Downloads.TorrentPath + L"\\" + pFile->m_sName ) )		// Try multifile torrent default,  ToDo: Need better detection
					m_bSelPartial = FALSE;

				//CEnvyFile oFile = *pFile;
				//CSingleLock pLibraryLock( &Library.m_pSection );
				//if ( pLibraryLock.Lock( 100 ) )
				//{
				//	if ( CLibraryFile* pLibFile = LibraryMaps.LookupFileByHash( &oFile, FALSE, TRUE ) )
				//		m_bSelPartial = FALSE;
				//	else if ( pFile->m_sPath.Find( pFile->m_sName ) > 3 )	// Not multifile torrent  (Not in library, but shared seed from untracked download group)
				//		m_bSelPartial = FALSE;
				//	else if ( PathIsDirectory( Settings.Downloads.TorrentPath + L"\\" + pFile->m_sName ) ) 	// Try multifile torrent default, need better detection
				//		m_bSelPartial = FALSE;
				//}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CUploadsWnd command handlers

void CUploadsWnd::OnUpdateUploadsDisconnect(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelActive );
}

void CUploadsWnd::OnUploadsDisconnect()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CUploadFile*> pList;
	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) )
			pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		if ( ! SafeLock( pLock ) ) continue;

		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) && pFile->GetActive() != NULL )
		{
			CUploadTransfer* pUpload = pFile->GetActive();

			if ( pUpload->m_nProtocol == PROTOCOL_ED2K && pUpload->m_nState != upsNull )
			{
				CString strFormat, strMessage;
				LoadString( strFormat, IDS_UPLOAD_CANCEL_ED2K );
				strMessage.Format( strFormat, (LPCTSTR)pUpload->m_sName );
				pLock.Unlock();
				INT_PTR nResp = MsgBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
				if ( nResp == IDCANCEL )
					break;
				if ( nResp != IDYES || ! Uploads.Check( pUpload ) )
					continue;
				pLock.Lock();
			}

			pUpload->Close( TRUE );
		}
	}
}

void CUploadsWnd::OnUpdateUploadsStart(CCmdUI* pCmdUI)
{
	Prepare();

	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pItem->Show( ! m_bSelActive || m_bSelQueued );

	pCmdUI->Enable( m_bSelQueued );
}

void CUploadsWnd::OnUploadsStart()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );

		if ( IsSelected( pFile ) && pFile->GetActive() != NULL )
			pFile->GetActive()->Promote();
	}
}

void CUploadsWnd::OnUpdateUploadsPriority(CCmdUI* pCmdUI)
{
	Prepare();

	// Default skin visible="false"
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pItem->Show( m_bSelActive && ! m_bSelQueued );

	BOOL bPriority = FALSE;

	if ( m_bSelActive && ! m_bSelQueued )
	{
		for ( POSITION pos = UploadFiles.GetIterator(); pos; )
		{
			CUploadFile* pFile = UploadFiles.GetNext( pos );

			if ( IsSelected( pFile ) && pFile->GetActive()->m_bPriority && pFile->GetActive()->m_nState != upsNull )
			{
				bPriority = TRUE;
				break;
			}
		}
	}

	pCmdUI->Enable( ( m_bSelActive && m_nSelected == 1 ) || bPriority );
	pCmdUI->SetCheck( bPriority );
}

void CUploadsWnd::OnUploadsPriority()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );

		if ( pFile->GetActive() != NULL )
		{
			if ( m_nSelected == 1 && IsSelected( pFile ) && ! pFile->GetActive()->m_bPriority )
				pFile->GetActive()->Promote( TRUE );
			else if ( pFile->GetActive()->m_nState != upsNull )
				pFile->GetActive()->m_bPriority = FALSE;
		}
	}
}

void CUploadsWnd::OnUpdateUploadsClear(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelFile );
}

void CUploadsWnd::OnUploadsClear()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CUploadFile*> pList;
	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) )
			pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) )
		{
			CUploadTransfer* pUpload = pFile->GetActive();

			if ( pUpload != NULL && pUpload->m_nProtocol == PROTOCOL_ED2K && pUpload->m_nState != upsNull )
			{
				CString strMessage;
				strMessage.Format( LoadString( IDS_UPLOAD_CANCEL_ED2K ), (LPCTSTR)pUpload->m_sName );
				pLock.Unlock();
				INT_PTR nResp = MsgBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
				pLock.Lock();
				if ( nResp == IDCANCEL )
					break;
				if ( nResp != IDYES || ! UploadFiles.Check( pFile ) )
					continue;
			}

			pFile->Remove();
		}
	}
}

void CUploadsWnd::OnUpdateUploadsLaunch(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelFile && ! m_bSelPartial );
}

void CUploadsWnd::OnUploadsLaunch()
{
	const BOOL bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 );

	CSingleLock pTransfersLock( &Transfers.m_pSection );
	if ( ! SafeLock( pTransfersLock ) ) return;

	CList<CUploadFile*> pList;

	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) )
			pList.AddTail( pFile );
	}

	pTransfersLock.Unlock();

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		// Multifile torrent always opens folder
		if ( pFile->m_sPath.IsEmpty() )				// ToDo: Update this path assumption when fixed elsewhere
		{
			const CString strPath = Settings.Downloads.TorrentPath + L"\\" + pFile->m_sName;		// Try default multifile torrent folder  (Need better detection)
			if ( PathIsDirectory( strPath ) )
				ShellExecute( GetSafeHwnd(), L"open", strPath, NULL, NULL, SW_SHOWNORMAL );
			continue;
		}

		// Launch directly with Shift key
		if ( bShift && SafeLock( pTransfersLock ) )
		{
			if ( UploadFiles.Check( pFile ) )
			{
				pTransfersLock.Unlock();
				CFileExecutor::Execute( pFile->m_sPath );
				continue;
			}
			pTransfersLock.Unlock();
		}

		CEnvyFile& oFile = *pFile;

		// Show in Library by default
		CSingleLock pLibraryLock( &Library.m_pSection );
		if ( SafeLock( pLibraryLock ) )
		{
			CLibraryFile* pLibFile = LibraryMaps.LookupFileByHash( &oFile );
			if ( ! pLibFile ) pLibFile = LibraryMaps.LookupFileByPath( oFile.m_sPath );
			if ( pLibFile )
			{
				if ( CLibraryWnd* pLibrary = CLibraryWnd::GetLibraryWindow() )		// (CLibraryWnd*)( pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd) ) ) )
				{
					if ( pLibrary->Display( pLibFile ) )
						continue;
				}
			}
			pLibraryLock.Unlock();
		}

		// Show in Downloads as fallback (torrents)
		if ( SafeLock( pTransfersLock ) )
		{
			if ( CDownload* pDownload = Downloads.FindByPath( oFile.m_sPath ) )
			{
				pTransfersLock.Unlock();
				if ( CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd() )
				{
					if ( CDownloadsWnd* pDownWnd = (CDownloadsWnd*)pMainWnd->m_pWindows.Find( RUNTIME_CLASS(CDownloadsWnd) ) )
					{
						pDownWnd->Select( pDownload );

						pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_DOWNLOADS );
						pMainWnd->PostMessage( WM_SYSCOMMAND, SC_RESTORE );

						continue;
					}
				}
			}
			else
				pTransfersLock.Unlock();
		}
	}
}

void CUploadsWnd::OnUpdateUploadsFolder(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelFile && ! m_bSelPartial );
}

void CUploadsWnd::OnUploadsFolder()
{
	CQuickLock oLock( UploadQueues.m_pSection );

	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) )
		{
			CString strPath;	// *pFile->m_sPath
			CEnvyFile oFile = *pFile;
			if ( CLibraryFile* pLibFile = LibraryMaps.LookupFileByHash( &oFile, FALSE, TRUE ) )
			{
				strPath = pLibFile->GetPath();	// = pLibFile->GetFolder() + L"\\" + pFile->m_sName;
			}
			else //if ( strPath.IsEmpty() )
			{
				// ToDo: Fix non-default multifile torrents
				strPath = Settings.Downloads.TorrentPath + L"\\" + pFile->m_sName;
			}

			// Show corresponding download with Shift key  (ToDo: Fix multifile torrents. Note holding Shift may cause many files to highlight.)
			if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
			{
				if ( CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd() )
				{
					if ( CDownloadsWnd* pDownWnd = (CDownloadsWnd*)pMainWnd->m_pWindows.Find( RUNTIME_CLASS(CDownloadsWnd) ) )
						pDownWnd->Select( Downloads.FindByPath( strPath ) );
				}
				continue;
			}

			if ( PathIsDirectory( strPath ) )
				ShellExecute( GetSafeHwnd(), L"open", strPath, NULL, NULL, SW_SHOWNORMAL );
			else if ( PathFileExists( strPath ) )
				ShellExecute( GetSafeHwnd(), NULL, L"Explorer.exe", L"/select, " + strPath, NULL, SW_SHOWNORMAL );
		}
	}
}

void CUploadsWnd::OnUpdateUploadsChat(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelChat && Settings.Community.ChatEnable );
}

void CUploadsWnd::OnUploadsChat()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );

		if ( ! IsSelected( pFile ) )
			continue;

		if ( CUploadTransfer* pTransfer = pFile->GetActive() )
		{
			PROTOCOLID nProtocol = pTransfer->m_nProtocol;
			SOCKADDR_IN pAddress = pTransfer->m_pHost;
			BOOL bClientExtended = pTransfer->m_bClientExtended;
		//	CString strNick = pTransfer->m_sRemoteNick;

			pLock.Unlock();

			if ( nProtocol == PROTOCOL_HTTP )		// HTTP chat. (G2, G1)
				ChatWindows.OpenPrivate( Hashes::Guid(), &pAddress, FALSE, PROTOCOL_HTTP );
			else if ( bClientExtended )				// Client accepts G2 chat
				ChatWindows.OpenPrivate( Hashes::Guid(), &pAddress, FALSE, PROTOCOL_G2 );
			else if ( nProtocol == PROTOCOL_ED2K )	// ED2K chat.
				ChatWindows.OpenPrivate( Hashes::Guid(), &pAddress, FALSE, PROTOCOL_ED2K );
			//else		// Should never be called
			//	theApp.Message( MSG_DEBUG, L"Error while initiating chat- Unable to select protocol" );

			if ( ! SafeLock( pLock ) ) return;
		}
	}
}

void CUploadsWnd::OnUpdateSecurityBan(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelUpload );
}

void CUploadsWnd::OnSecurityBan()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CList<CUploadFile*> pList;

	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) ) pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) && pFile->GetActive() != NULL )
		{
			CUploadTransfer* pUpload = pFile->GetActive();

			IN_ADDR pAddress = pUpload->m_pHost.sin_addr;
			pUpload->Remove( FALSE );
			pLock.Unlock();
			Security.Ban( &pAddress, banSession );
			pLock.Lock();
		}
	}
}

void CUploadsWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelBrowse );
}

void CUploadsWnd::OnBrowseLaunch()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	CList<CUploadFile*> pList;

	for ( POSITION pos = UploadFiles.GetIterator(); pos; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) ) pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) && pFile->GetActive() != NULL )
		{
			CUploadTransfer* pTransfer = pFile->GetActive();
			PROTOCOLID nProtocol = pTransfer->m_nProtocol;
			SOCKADDR_IN pAddress = pTransfer->m_pHost;
			pLock.Unlock();
			new CBrowseHostWnd( nProtocol, &pAddress );
			pLock.Lock();
		}
	}
}

void CUploadsWnd::OnUploadsClearCompleted()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return;

	for ( POSITION pos = Uploads.GetIterator(); pos; )
	{
		CUploadTransfer* pUpload = Uploads.GetNext( pos );
		if ( pUpload->m_nState == upsNull ) pUpload->Remove( FALSE );
	}

	pLock.Unlock();
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsAutoClear(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.Uploads.AutoClear );
}

void CUploadsWnd::OnUploadsAutoClear()
{
	Settings.Uploads.AutoClear = ! Settings.Uploads.AutoClear;
	if ( Settings.Uploads.AutoClear ) OnTimer( 4 );
}

// Removed redundant function:
//void CUploadsWnd::OnEditQueue()

void CUploadsWnd::OnUploadsSettings()
{
	CSettingsManagerDlg::Run( L"CUploadsSettingsPage" );
}

void CUploadsWnd::OnUploadsHelp()
{
	CHelpDlg::Show( L"UploadHelp" );
}

BOOL CUploadsWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_TAB )		// Toggle window focus to Downloads
		{
			GetManager()->Open( RUNTIME_CLASS(CDownloadsWnd) );
			return TRUE;
		}
		if ( pMsg->wParam == VK_DELETE )	// Cancel uploads
		{
			OnUploadsClear();
			return TRUE;
		}
		if ( pMsg->wParam == VK_ESCAPE )	// Clear selections
		{
			for ( POSITION posFile = UploadFiles.GetIterator(); posFile; )
			{
				CUploadFile* pFile = UploadFiles.GetNext( posFile );
				pFile->m_bSelected = FALSE;
			}

			m_wndUploads.Update();
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}

void CUploadsWnd::OnUploadsFilterMenu()
{
	CMenu* pMenu = Skin.GetMenu( L"CUploadsWnd.Filter" );
	m_wndToolBar.ThrowMenu( ID_UPLOADS_FILTER_MENU, pMenu, NULL, FALSE, TRUE );
}

void CUploadsWnd::OnUpdateUploadsFilterAll(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_ALL ) == ULF_ALL );
}

void CUploadsWnd::OnUploadsFilterAll()
{
	Settings.Uploads.FilterMask |= ULF_ALL;
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsFilterActive(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_ACTIVE ) > 0 );
}

void CUploadsWnd::OnUploadsFilterActive()
{
	Settings.Uploads.FilterMask ^= ULF_ACTIVE;
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsFilterQueued(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_QUEUED ) > 0 );
}

void CUploadsWnd::OnUploadsFilterQueued()
{
	Settings.Uploads.FilterMask ^= ULF_QUEUED;
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsFilterHistory(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_HISTORY ) > 0 );
}

void CUploadsWnd::OnUploadsFilterHistory()
{
	Settings.Uploads.FilterMask ^= ULF_HISTORY;
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsFilterTorrent(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_TORRENT ) > 0 );
}

void CUploadsWnd::OnUploadsFilterTorrent()
{
	Settings.Uploads.FilterMask ^= ULF_TORRENT;
	m_wndUploads.Update();
}
