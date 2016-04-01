//
// WndLibrary.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2007
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
#include "WndLibrary.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "LibraryBuilder.h"
#include "CollectionFile.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "WndMain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_SERIAL(CLibraryWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CLibraryWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryWnd construction

CLibraryWnd::CLibraryWnd() : CPanelWnd( TRUE, FALSE )
{
	Create( IDR_LIBRARYFRAME );
}

CLibraryWnd::~CLibraryWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryWnd operations

CLibraryWnd* CLibraryWnd::GetLibraryWindow(BOOL bToggle, BOOL bFocus)
{
	if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
	{
		return static_cast< CLibraryWnd* >( pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd), bToggle, bFocus ) );
	}
	return NULL;
}

BOOL CLibraryWnd::Display(const CLibraryFile* pFile)
{
	if ( ! theApp.SafeMainWnd() )
		return FALSE;

	theApp.SafeMainWnd()->OpenFromTray();

	m_wndFrame.Update( TRUE );

	return m_wndFrame.Display( pFile );
}

BOOL CLibraryWnd::Display(const CAlbumFolder* pFolder)
{
	if ( ! theApp.SafeMainWnd() )
		return FALSE;

	theApp.SafeMainWnd()->OpenFromTray();

	m_wndFrame.Update( TRUE );

	return m_wndFrame.Display( pFolder );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryWnd message handlers

int CLibraryWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	m_tLast = 0;

	m_wndFrame.Create( this );

	LoadState();

	return 0;
}

void CLibraryWnd::OnDestroy()
{
	SaveState();
	CPanelWnd::OnDestroy();
}

BOOL CLibraryWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndFrame.m_hWnd )
	{
		if ( m_wndFrame.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}

	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CLibraryWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );
	if ( m_wndFrame.m_hWnd )
		m_wndFrame.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CLibraryWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );

	//if ( bActivate )
	//{
	//	m_wndFrame.Update();
	//	m_wndFrame.SetFocus();
	//}
}

void CLibraryWnd::OnTimer(UINT_PTR nIDEvent)
{
	const DWORD tNow = GetTickCount();

	if ( nIDEvent == 1 )
	{
		if ( IsPartiallyVisible() )
		{
			CWaitCursor pCursor;
			m_wndFrame.Update( FALSE );
			m_tLast = tNow;
		}
	}
	else if ( tNow > m_tLast + 30000 )
	{
		m_wndFrame.Update( FALSE );
		m_tLast = tNow;
	}
}

void CLibraryWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	m_wndFrame.OnSkinChange();
}


/////////////////////////////////////////////////////////////////////////////
// CLibraryWnd events

HRESULT CLibraryWnd::GetGenericView(IGenericView** ppView)
{
	if ( m_wndFrame.m_hWnd == NULL ) return S_FALSE;
	CLibraryListPtr pList( m_wndFrame.GetViewSelection() );
	*ppView = (IGenericView*)pList->GetInterface( IID_IGenericView, TRUE );
	return S_OK;
}

BOOL CLibraryWnd::OnCollection(LPCTSTR pszPath)
{
	CAlbumFolder* pFolder = NULL;
	CLibraryFolder* pLibFolder;
	CCollectionFile pCollection;
	CString strMessage;

	if ( ! pCollection.Open( pszPath ) )	// Verify specified collection is valid
	{
		// User clicked an invalid collection
		strMessage.Format( LoadString( IDS_LIBRARY_COLLECTION_INVALID ), pszPath );
		MsgBox( strMessage, MB_ICONEXCLAMATION );
		return FALSE;
	}

	// Create the collection folder (in case it doesn't exist)
	CreateDirectory( Settings.Downloads.CollectionPath );
	// Add the collection folder to the library (in case it isn't there)
	pLibFolder = LibraryFolders.AddFolder( Settings.Downloads.CollectionPath );
	// Force a scan of it (in case watch library folders is disabled)
	pLibFolder = LibraryFolders.GetFolder( Settings.Downloads.CollectionPath );
	if ( pLibFolder != NULL ) pLibFolder->Scan();

	CSingleLock oLock( &Library.m_pSection, TRUE );
	if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( pszPath, FALSE, TRUE ) )
	{
		// Collection IS already in the library

		// Re-mount the collection
		LibraryFolders.MountCollection( pFile->m_oSHA1, &pCollection );
		pFolder = LibraryFolders.GetCollection( pFile->m_oSHA1 );
		oLock.Unlock();
	}
	else	// Collection is not already in the main library
	{
		oLock.Unlock();
		// Check the collection folder
		CString strSource( pszPath ), strTarget;

		const int nName = strSource.ReverseFind( L'\\' );
		if ( nName >= 0 )
		{
			strTarget = Settings.Downloads.CollectionPath + strSource.Mid( nName );
			LibraryBuilder.RequestPriority( strTarget );
		}

		oLock.Lock();
		if ( CLibraryFile* pTargetFile = LibraryMaps.LookupFileByPath( strTarget, FALSE, TRUE ) )
		{
			// Collection is already in the collection folder

			// Re-mount the collection
			LibraryFolders.MountCollection( pTargetFile->m_oSHA1, &pCollection );
			pFolder = LibraryFolders.GetCollection( pTargetFile->m_oSHA1 );
			oLock.Unlock();
		}
		else	// Collection is not already in collection folder
		{
			oLock.Unlock();

			if ( ! strTarget.IsEmpty() && CopyFile( strSource, strTarget, TRUE ) )
			{
				// Collection was copied into the collection folder

				// Force a scan of collection folder (in case watch library folders is disabled)
				if ( pLibFolder != NULL ) pLibFolder->Scan();

				strMessage.Format( LoadString( IDS_LIBRARY_COLLECTION_INSTALLED ), (LPCTSTR)pCollection.GetTitle() );
				MsgBox( strMessage, MB_ICONINFORMATION );

				oLock.Lock();
				if ( CLibraryFolder* pCollectionFolder = LibraryFolders.GetFolder(Settings.Downloads.CollectionPath ) )
				{
					pCollectionFolder->Scan();
				}
				if ( CLibraryFile* pTargetFile1 = LibraryMaps.LookupFileByPath( strTarget, FALSE, TRUE ) )
				{
					// Re-mount the collection
					LibraryFolders.MountCollection( pTargetFile1->m_oSHA1, &pCollection );
					pFolder = LibraryFolders.GetCollection( pTargetFile1->m_oSHA1 );
				}
				oLock.Unlock();
			}
			else if ( GetLastError() == ERROR_FILE_EXISTS )
			{
				// File with this name already exists:
				// We cannot copy the collection because it's already there, but it doesn't appear in the library.
				// The greatest probablility is that the file is there, but hasn't been added yet.
				// Best bet is to pretend everything is okay, since the delay it takes the user to respond may fix everything.

				strMessage.Format( LoadString( IDS_LIBRARY_COLLECTION_INSTALLED ), (LPCTSTR)pCollection.GetTitle() );
				MsgBox( strMessage, MB_ICONINFORMATION );

				oLock.Lock();
				if ( CLibraryFile* pTargetFile1 = LibraryMaps.LookupFileByPath( strTarget, FALSE, TRUE ) )
				{
					// Collection was already there: Re-mount the collection
					LibraryFolders.MountCollection( pTargetFile1->m_oSHA1, &pCollection );
					pFolder = LibraryFolders.GetCollection( pTargetFile1->m_oSHA1 );
					oLock.Unlock();
				}
				else	// File of this name exists in the folder, but does not appear in the library.
				{
					// Most likely cause- Corrupt file in collection folder.
					oLock.Unlock();
					strMessage.Format( LoadString( IDS_LIBRARY_COLLECTION_CANT_INSTALL ), (LPCTSTR)pCollection.GetTitle(), (LPCTSTR)Settings.Downloads.CollectionPath );
					MsgBox( strMessage, MB_ICONEXCLAMATION );
				}
			}
			else	// Was not able to copy collection to the collection folder for Unknown reason -Display an error message
			{
				strMessage.Format( LoadString( IDS_LIBRARY_COLLECTION_CANT_INSTALL ), (LPCTSTR)pCollection.GetTitle(), (LPCTSTR)Settings.Downloads.CollectionPath );
				MsgBox( strMessage, MB_ICONEXCLAMATION );
			}
		}
	}

	if ( pFolder )
		Display( pFolder ); 	// Show the collection

	return ( pFolder != NULL );
}
