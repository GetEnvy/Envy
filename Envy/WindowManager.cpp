//
// WindowManager.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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
#include "WindowManager.h"
#include "CtrlWndTabBar.h"
#include "Colors.h"
#include "Skin.h"

#include "WndHome.h"
#include "WndSystem.h"
#include "WndNeighbours.h"
#include "WndDownloads.h"
#include "WndUploads.h"
#include "WndTraffic.h"
#include "WndSearch.h"
#include "WndBrowseHost.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CWindowManager, CWnd)

BEGIN_MESSAGE_MAP(CWindowManager, CWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_STYLECHANGED()
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// CWindowManager construction

CWindowManager::CWindowManager(CMDIFrameWnd* pParent)
	: m_bIgnoreActivate ( FALSE )
{
	if ( pParent ) SetOwner( pParent );
}

CWindowManager::~CWindowManager()
{
}

//////////////////////////////////////////////////////////////////////
// CWindowManager owner

void CWindowManager::SetOwner(CMDIFrameWnd* pParent)
{
	m_pParent = pParent;
	SubclassWindow( m_pParent->m_hWndMDIClient );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager add and remove

void CWindowManager::Add(CChildWnd* pChild)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	if ( m_pWindows.Find( pChild ) == NULL )
		m_pWindows.AddTail( pChild );
}

void CWindowManager::Remove(CChildWnd* pChild)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	POSITION pos = m_pWindows.Find( pChild );
	if ( pos ) m_pWindows.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager get active

CChildWnd* CWindowManager::GetActive() const
{
	if ( m_hWnd == NULL ) return NULL;

	CWnd* pActive = m_pParent->MDIGetActive();

	if ( pActive && pActive->IsKindOf( RUNTIME_CLASS(CChildWnd) ) )
		return (CChildWnd*)pActive;

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager iteration

POSITION CWindowManager::GetIterator() const
{
	return m_pWindows.GetHeadPosition();
}

CChildWnd* CWindowManager::GetNext(POSITION& pos) const
{
	return m_pWindows.GetNext( pos );
}

BOOL CWindowManager::Check(CChildWnd* pChild) const
{
	return m_pWindows.Find( pChild ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager find

CChildWnd* CWindowManager::Find(CRuntimeClass* pClass, CChildWnd* pAfter, CChildWnd* pExcept)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	BOOL bFound = ( pAfter == NULL );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );

		if ( pChild == pExcept )
			continue;
		if ( bFound && ( ! pClass || pChild->IsKindOf( pClass ) ) )
			return pChild;
		if ( pChild == pAfter )
			bFound = TRUE;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager open / toggle

CChildWnd* CWindowManager::Open(CRuntimeClass* pClass, BOOL bToggle, BOOL bFocus)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	CChildWnd* pChild = Find( pClass );

	if ( pChild && pChild->IsIconic() )
	{
		pChild->ShowWindow( SW_SHOWNORMAL );
		bToggle = FALSE;
	}

	if ( pChild && pChild->m_bTabMode )
		bToggle = FALSE;

	if ( pChild && bToggle && GetActive() == pChild )
	{
		pChild->DestroyWindow();
		return NULL;
	}

	pLock.Unlock();

	if ( ! pChild )
		pChild = static_cast< CChildWnd* >( pClass->CreateObject() );

	if ( pChild && bFocus )
	{
		Sleep( 50 );
		pChild->BringWindowToTop();
	}

	return pChild;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager find by point

CChildWnd* CWindowManager::FindFromPoint(const CPoint& point) const
{
	CPoint pt( point );
	ScreenToClient( &pt );
	CChildWnd* pWnd = (CChildWnd*)ChildWindowFromPoint( pt );

	return ( pWnd != NULL && pWnd->IsKindOf( RUNTIME_CLASS(CChildWnd) ) ) ? pWnd : NULL;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager close

void CWindowManager::Close()
{
	CList< CChildWnd* > pClose;

	CSingleLock pLock( &theApp.m_pSection, TRUE );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );
		pClose.AddTail( pChild );

		pChild->RemoveSkin();
	}

	for ( POSITION pos = pClose.GetHeadPosition() ; pos ; )
	{
		CChildWnd* pChild = pClose.GetNext( pos );
		pChild->DestroyWindow();
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager automatic resize

void CWindowManager::AutoResize()
{
	CChildWnd* pChild;
	CRect rcSize;

	GetClientRect( &rcSize );
	if ( rcSize.right < 64 || rcSize.bottom < 64 ) return;

	for ( pChild = (CChildWnd*)GetWindow( GW_CHILD ) ; pChild ; pChild = (CChildWnd*)pChild->GetNextWindow() )
	{
		if ( ! pChild->IsKindOf( RUNTIME_CLASS(CChildWnd) ) ) continue;

		CRect rcChild;
		pChild->GetWindowRect( &rcChild );
		ScreenToClient( &rcChild );

		if ( rcChild.right == m_rcSize.right || rcChild.bottom == m_rcSize.bottom )
		{
			if ( rcChild.right == m_rcSize.right )
				rcChild.right = rcSize.right;
			if ( rcChild.bottom == m_rcSize.bottom )
				rcChild.bottom = rcSize.bottom;

			pChild->MoveWindow( &rcChild );
		}
	}

	m_rcSize = rcSize;

	if ( Settings.General.GUIMode != GUI_WINDOWED )
	{
		for ( pChild = (CChildWnd*)GetWindow( GW_CHILD ) ; pChild ; pChild = (CChildWnd*)pChild->GetNextWindow() )
		{
			if ( ! pChild->IsKindOf( RUNTIME_CLASS(CChildWnd) ) ) continue;

			if ( pChild->m_bGroupMode && pChild->m_pGroupParent )
			{
				CRect rcChild( &rcSize );
				rcChild.bottom = (int)( pChild->m_pGroupParent->m_nGroupSize * rcChild.bottom );
				pChild->m_pGroupParent->MoveWindow( &rcChild );
				rcChild.top = rcChild.bottom;
				rcChild.bottom = rcSize.bottom;
				pChild->MoveWindow( &rcChild );
			}
			else if ( pChild->m_bPanelMode && ! pChild->m_bGroupMode )
			{
				CRect rcChild;
				pChild->GetWindowRect( &rcChild );
				ScreenToClient( &rcChild );
				if ( rcChild != rcSize ) pChild->MoveWindow( &rcSize );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager customized cascade

void CWindowManager::Cascade(BOOL bActiveOnly)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	CChildWnd* pActive = bActiveOnly ? GetActive() : NULL;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );

		if ( ! pChild->m_bPanelMode && pChild->IsWindowVisible() &&
			 ! pChild->IsIconic() && ( pChild == pActive || ! pActive ) )
		{
			CRect rcClient;
			pChild->ShowWindow( SW_RESTORE );
			pChild->GetParent()->GetClientRect( &rcClient );
			pChild->MoveWindow( &rcClient );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager activate grouped windows

void CWindowManager::ActivateGrouped(CChildWnd* pExcept)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	if ( m_bIgnoreActivate ) return;
	m_bIgnoreActivate = TRUE;

	CChildWnd* pParent = pExcept->m_pGroupParent ? pExcept->m_pGroupParent : pExcept;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );

		if ( pChild != pExcept )
		{
			if ( pChild->m_bGroupMode && ( pChild == pParent || pChild->m_pGroupParent == pParent ) )
			{
				pChild->ShowWindow( SW_SHOWNORMAL );
				pChild->MDIActivate();
			}
		}
	}

	pExcept->MDIActivate();
	m_bIgnoreActivate = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager GUI mode

void CWindowManager::SetGUIMode(int nMode, BOOL bSaveState)
{
	ModifyStyle( WS_VISIBLE, 0 );

	if ( bSaveState )
	{
		SaveSearchWindows();
		SaveBrowseHostWindows();
		SaveWindowStates();
	}

	Close();

	Settings.General.GUIMode = nMode;

	if ( bSaveState )
		Settings.Save();

	LoadSearchWindows();
	LoadBrowseHostWindows();
	LoadWindowStates();

	AutoResize();
	ShowWindow( SW_SHOW );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager create tabbed windows (Obsolete)

//void CWindowManager::CreateTabbedWindows()
//{
//	CDownloadsWnd* pDownloads = new CDownloadsWnd();
//
//	if ( Settings.General.GUIMode != GUI_BASIC )
//	{
//		CUploadsWnd* pUploads = new CUploadsWnd();
//		pUploads->m_pGroupParent = pDownloads;
//
//		CNeighboursWnd* pNeighbours = new CNeighboursWnd();
//		CSystemWnd* pSystem = new CSystemWnd();
//		pSystem->m_pGroupParent = pNeighbours;
//	}
//
//	/*CHomeWnd* pHome =*/ new CHomeWnd();
//}

//////////////////////////////////////////////////////////////////////
// CWindowManager load complex window states

void CWindowManager::LoadWindowStates()
{
	CString strWindows = theApp.GetProfileString( L"Windows", L"State" );		// , L"CSystemWnd|CNeighboursWnd"

	CChildWnd* pDownloads = NULL;
	CChildWnd* pUploads = NULL;
	CChildWnd* pNeighbours = NULL;
	CChildWnd* pSystem = NULL;

	switch ( Settings.General.GUIMode )
	{
	case GUI_BASIC:
		pDownloads	=	Open( RUNTIME_CLASS( CDownloadsWnd ) );
		break;

	case GUI_TABBED:
		pDownloads	=	Open( RUNTIME_CLASS( CDownloadsWnd ) );
		pUploads	=	Open( RUNTIME_CLASS( CUploadsWnd ) );
		if ( pUploads )	pUploads->m_pGroupParent = pDownloads;
		pNeighbours	=	Open( RUNTIME_CLASS( CNeighboursWnd ) );
		pSystem 	=	Open( RUNTIME_CLASS( CSystemWnd ) );
		if ( pSystem )	pSystem->m_pGroupParent = pNeighbours;
		break;

	case GUI_WINDOWED:
		break;
	}

	if ( Settings.Interface.SaveOpenWindows && Settings.General.GUIMode != GUI_BASIC )
	{
		for ( strWindows += '|' ; strWindows.GetLength() > 1 ; )
		{
			CString strClass = strWindows.SpanExcluding( L"| ,.\t" );
			strWindows = strWindows.Mid( strClass.GetLength() + 1 );

			if ( strClass.Find( L"TG#" ) == 0 )
			{
				DWORD nUnique;

				if ( _stscanf( (LPCTSTR)strClass + 3, L"%lu", &nUnique ) == 1 )
					new CTrafficWnd( nUnique );
			}
			else if ( ! strClass.IsEmpty() &&
				strClass != L"CMediaWnd" &&		// Never?
				strClass != L"CSearchWnd" &&		// Open by LoadSearchWindows()
				strClass != L"CBrowseHostWnd" &&	// Open by LoadBrowseHostWindows()
				( ! pDownloads  || strClass != L"CDownloadsWnd" ) &&
				( ! pUploads    || strClass != L"CUploadsWnd" ) &&
				( ! pNeighbours || strClass != L"CNeighboursWnd" ) &&
				( ! pSystem     || strClass != L"CSystemWnd" ) )
			{
				CRuntimeClass* pClass = AfxClassForName( strClass );

				if ( pClass && pClass->IsDerivedFrom( RUNTIME_CLASS(CChildWnd) ) )
					Open( pClass );
			}
		}
	}

	if ( Settings.General.GUIMode != GUI_WINDOWED )
		Open( RUNTIME_CLASS( CHomeWnd ) );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager save complex window states

void CWindowManager::SaveWindowStates() const
{
	if ( ! Settings.Interface.SaveOpenWindows )
	{
		theApp.WriteProfileString( L"Windows", L"State", L"" );
		return;
	}

	CString strWindows;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );

		pChild->SaveState();

		if ( ! strWindows.IsEmpty() ) strWindows += '|';

		if ( pChild->IsKindOf( RUNTIME_CLASS(CTrafficWnd) ) )
		{
			CTrafficWnd* pTraffic = (CTrafficWnd*)pChild;
			CString strItem;
			strItem.Format( L"TG#%.4x", pTraffic->m_nUnique );
			strWindows += strItem;
		}
		else
		{
			strWindows += pChild->GetRuntimeClass()->m_lpszClassName;
		}
	}

	theApp.WriteProfileString( L"Windows", L"State", strWindows );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager search load and save

BOOL CWindowManager::LoadSearchWindows()
{
	const CString strFile = Settings.General.DataPath + L"Searches.dat";

	if ( ! Settings.Interface.SaveOpenWindows || Settings.General.GUIMode == GUI_BASIC )
	{
		DeleteFile( strFile );
		return FALSE;
	}

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
		return FALSE;

	BOOL bSuccess = FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::load, 262144 );		// 256 KB buffer
		try
		{
			while ( ar.ReadCount() == 1 )
			{
				CSearchWnd* pWnd = new CSearchWnd();
				pWnd->Serialize( ar );
			}
			ar.Close();
			bSuccess = TRUE;	// Success
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
		}
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
	}

	pFile.Close();

	if ( ! bSuccess )
	{
		theApp.Message( MSG_ERROR, L"Failed to load search windows: %s", strFile );
		return FALSE;
	}

	PostMainWndMessage( WM_SANITY_CHECK );

	return TRUE;
}

BOOL CWindowManager::SaveSearchWindows() const
{
	const CString strFile = Settings.General.DataPath + L"Searches.dat";
	const CString strTemp = Settings.General.DataPath + L"Searches.tmp";
	DWORD nCount = 0;

	if ( ! Settings.Interface.SaveOpenWindows )
	{
		DeleteFile( strFile );
		return FALSE;
	}

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save search windows: %s", strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store, 262144 );		// 256 KB buffer
		try
		{
			DWORD nTotal = 0;
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CSearchWnd* pWnd = (CSearchWnd*)GetNext( pos );
				if ( pWnd->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) && pWnd->GetLastSearch() )
					++nTotal;
			}

			DWORD nSkip = ( nTotal > Settings.Interface.SearchWindowsLimit ) ? ( nTotal - Settings.Interface.SearchWindowsLimit ) : 0;

			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CSearchWnd* pWnd = (CSearchWnd*)GetNext( pos );
				if ( pWnd->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) &&
					 pWnd->GetLastSearch() )
				{
					if ( nSkip )
					{
						--nSkip;
					}
					else
					{
						ar.WriteCount( 1 );
						pWnd->Serialize( ar );
						++nCount;
					}
				}
			}
			ar.WriteCount( 0 );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, L"Failed to save search windows: %s", strTemp );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save search windows: %s", strTemp );
		return FALSE;
	}

	if ( ! nCount )
	{
		DeleteFile( strFile );
		DeleteFile( strTemp );
	}
	else if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save search windows: %s", strFile );
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager browse host load and save

BOOL CWindowManager::LoadBrowseHostWindows()
{
	const CString strFile = Settings.General.DataPath + L"BrowseHosts.dat";

	if ( ! Settings.Interface.SaveOpenWindows || Settings.General.GUIMode == GUI_BASIC )
	{
		DeleteFile( strFile );
		return FALSE;
	}

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
		return FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::load, 262144 );		// 256 KB buffer
		try
		{
			while ( ar.ReadCount() == 1 )
			{
				CBrowseHostWnd* pWnd = new CBrowseHostWnd();
				pWnd->Serialize( ar );
			}
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, L"Failed to load browse host windows: %s", strFile );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		theApp.Message( MSG_ERROR, L"Failed to load browse host windows: %s", strFile );
		return FALSE;
	}

	return TRUE;
}

BOOL CWindowManager::SaveBrowseHostWindows() const
{
	const CString strFile = Settings.General.DataPath + L"BrowseHosts.dat";
	const CString strTemp = Settings.General.DataPath + L"BrowseHosts.tmp";
	DWORD nCount = 0;

	if ( ! Settings.Interface.SaveOpenWindows )
	{
		DeleteFile( strFile );
		return FALSE;
	}

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save browse host windows: %s", strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store, 262144 );		// 256 KB buffer
		try
		{
			DWORD nTotal = 0;
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CBrowseHostWnd* pWnd = (CBrowseHostWnd*) GetNext( pos );
				if ( pWnd->IsKindOf( RUNTIME_CLASS(CBrowseHostWnd) ) )
					++nTotal;
			}

			DWORD nSkip = ( nTotal > Settings.Interface.BrowseWindowsLimit ) ? ( nTotal - Settings.Interface.BrowseWindowsLimit ) : 0;

			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CBrowseHostWnd* pWnd = (CBrowseHostWnd*) GetNext( pos );
				if ( pWnd->IsKindOf( RUNTIME_CLASS(CBrowseHostWnd) ) )
				{
					if ( nSkip )
					{
						--nSkip;
					}
					else
					{
						ar.WriteCount( 1 );
						pWnd->Serialize( ar );
						++nCount;
					}
				}
			}
			ar.WriteCount( 0 );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, L"Failed to save browse host windows: %s", strTemp );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save browse host windows: %s", strTemp );
		return FALSE;
	}

	//theApp.Message( MSG_DEBUG, L"Browses successfully saved to: %s", strFile );

	if ( ! nCount )
	{
		DeleteFile( strTemp );
		DeleteFile( strFile );
	}
	else if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, L"Failed to save browse host windows: %s", strFile );
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager new blank search window (or toggle existing tabs)

void CWindowManager::OpenNewSearchWindow()
{
	BOOL bEmptySearch = FALSE;
	POSITION posSearch = NULL;
	int nCount = 0;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CSearchWnd* pChild = (CSearchWnd*)GetNext( pos );

		if ( ! pChild->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) )
			continue;

		nCount++;

		if ( ! pChild->GetLastSearch() )			// Empty search should always be found last:
		{
			if ( ! posSearch )
			{
				if ( pChild != GetActive() )		// Always show empty search on first press
				{
					pChild->BringWindowToTop();
					if ( pChild->IsIconic() )
						pChild->ShowWindow( SW_SHOWNORMAL );
					return;
				}

				if ( nCount == 1 )					// Only tab already shown, do nothing
					return;

				posSearch = GetIterator();			// Set toggle loop from beginning
			}

			bEmptySearch = TRUE;
			break;
		}

		if ( ! posSearch && pChild == GetActive() )
			posSearch = pos;						// Set toggle from current tab
	}

	if ( bEmptySearch == FALSE )					// Always show empty search if needed
	{
		new CSearchWnd();
		return;
	}

	// Toggle to next existing search tab
	for ( ; posSearch ; )
	{
		CSearchWnd* pChild = (CSearchWnd*)GetNext( posSearch );

		if ( ! pChild->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) )	// || pChild == GetActive()
			continue;

		pChild->BringWindowToTop();
		if ( pChild->IsIconic() )
			pChild->ShowWindow( SW_SHOWNORMAL );
		return;
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager skin change

void CWindowManager::PostSkinChange()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChildWnd = GetNext( pos );
		pChildWnd->OnSkinChange();
	}
}

void CWindowManager::PostSkinRemove()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->RemoveSkin();
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager message handlers

void CWindowManager::OnSize(UINT nType, int cx, int cy)
{
	if ( ! Settings.Skin.FrameEdge )
		ModifyStyleEx( WS_EX_CLIENTEDGE, 0 );	// Required here for OnStyleChanged?

	if ( nType != 1982 )
		CWnd::OnSize( nType, cx, cy );

	AutoResize();
}

BOOL CWindowManager::OnEraseBkgnd(CDC* pDC)
{
//	if ( Colors.m_crPanelBack == RGB_DEFAULT_CASE )
//		return TRUE;

	CRect rc;
	GetClientRect( &rc );
	pDC->FillSolidRect( &rc, Colors.m_crPanelBack );

	return TRUE;
}

void CWindowManager::OnPaint()
{
	CPaintDC dc( this );

	// Draw background logo
//	if ( Settings.General.GUIMode != GUI_WINDOWED )
//		return;
//
//	CRect rc;
//	GetClientRect( &rc );
//	COLORREF crBackground = Colors.m_crMediaWindow;
//
//	if ( HBITMAP hLogo = Skin.GetWatermark( L"LargeLogo", TRUE ) )
//	{
//		BITMAP pInfo = {};
//		GetObject( hLogo, sizeof( BITMAP ), &pInfo );
//		CPoint pt = rc.CenterPoint();
//		pt.x -= pInfo.bmWidth / 2;
//		pt.y -= pInfo.bmHeight / 2;
//		CDC dcMem;
//		dcMem.CreateCompatibleDC( &dc );
//		HBITMAP pOldBmp = (HBITMAP)dcMem.SelectObject( hLogo );
//		dc.BitBlt( pt.x, pt.y, pInfo.bmWidth, pInfo.bmHeight, &dcMem, 0, 0, SRCCOPY );
//		crBackground = dc.GetPixel( pt.x, pt.y );
//		dc.ExcludeClipRect( pt.x, pt.y, pt.x + pInfo.bmWidth, pt.y + pInfo.bmHeight );
//		dcMem.SelectObject( pOldBmp );
//	}
//
//	dc.FillSolidRect( &rc, crBackground );
}

void CWindowManager::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	// Remove 3d bevel
	if ( ! Settings.Skin.FrameEdge && nStyleType == GWL_EXSTYLE && ( lpStyleStruct->styleNew & WS_EX_CLIENTEDGE ) )
		ModifyStyleEx( WS_EX_CLIENTEDGE, 0 );
}
