//
// CtrlHomePanel.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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
#include "CtrlHomePanel.h"

#include "Statistics.h"
#include "Network.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "GraphItem.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"

#include "CoolInterface.h"
#include "ShellIcons.h"
#include "RichDocument.h"
#include "RichElement.h"
#include "FragmentBar.h"
#include "FileExecutor.h"
#include "Colors.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CHomePanel, CTaskPanel)
BEGIN_MESSAGE_MAP(CHomePanel, CTaskPanel)
	ON_WM_CREATE()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeConnectionBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeConnectionBox, CRichTaskBox)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeLibraryBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeLibraryBox, CRichTaskBox)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeDownloadsBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeDownloadsBox, CRichTaskBox)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeUploadsBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeUploadsBox, CRichTaskBox)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHomePanel construction

CHomePanel::CHomePanel()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomePanel message handlers

BOOL CHomePanel::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, Settings.Skin.SidebarWidth, 0 );
	return CTaskPanel::Create( L"CHomePanel", WS_VISIBLE, rect, pParentWnd, IDC_HOME_PANEL );
}

int CHomePanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskPanel::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_boxConnection.Create( this, L"Connection", IDR_NEIGHBOURSFRAME );
	m_boxLibrary.Create( this, L"Library", IDR_LIBRARYFRAME );
	m_boxDownloads.Create( this, L"Downloads", IDR_DOWNLOADSFRAME );
	m_boxUploads.Create( this, L"Uploads", IDR_UPLOADSFRAME );

	AddBox( &m_boxConnection );
	AddBox( &m_boxLibrary );
	AddBox( &m_boxDownloads );
	AddBox( &m_boxUploads );
	// SetStretchBox( &m_boxLibrary );

	return 0;
}

void CHomePanel::OnSkinChange()
{
	SetWatermark( L"CHomePanel" );
	SetFooter( L"CHomePanel.Footer" );

	m_boxConnection.OnSkinChange();
	m_boxLibrary.OnSkinChange();
	m_boxDownloads.OnSkinChange();
	m_boxUploads.OnSkinChange();
	Update();
	Invalidate();

	// ToDo: Fix need for duplicate code workaround
	m_boxConnection.OnSkinChange();
	m_boxLibrary.OnSkinChange();
	m_boxDownloads.OnSkinChange();
	m_boxUploads.OnSkinChange();
	Update();
	Invalidate();
}

void CHomePanel::Update()
{
	m_boxDownloads.Update();
	m_boxUploads.Update();
	m_boxConnection.Update();
	m_boxLibrary.Update();
}


/////////////////////////////////////////////////////////////////////////////
// CHomeConnectionBox construction

CHomeConnectionBox::CHomeConnectionBox()
	: m_pdConnectedHours ( NULL )
	, m_pdConnectedMinutes ( NULL )
{
	ZeroMemory( m_pdCount, sizeof( m_pdCount ) );
	SetPrimary( TRUE );
}

CHomeConnectionBox::~CHomeConnectionBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomeConnectionBox message handlers

void CHomeConnectionBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdConnectedHours = m_pdConnectedMinutes = NULL;

	for ( int nP = PROTOCOL_NULL ; nP < PROTOCOL_LAST ; ++nP )	// PROTOCOLID
	{
		for ( int nT = ntNode ; nT <= ntLeaf ; nT++ )
		{
			m_pdCount[ nP ][ nT ] = NULL;
			m_sCount[ nP ][ nT ].Empty();
		}
	}

	SetCaptionmark( L"CHomeConnectionBox.Caption" );

	CXMLElement* pXML = Skin.GetDocument( L"CHomeConnectionBox" );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( L"title", L"Connection" ) );
	HICON hIcon = CoolInterface.ExtractIcon( IDR_NEIGHBOURSFRAME, Settings.General.LanguageRTL );
	if ( hIcon ) SetIcon( hIcon );

	m_pDocument = new CRichDocument();

	CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( L"ConnectedHours", m_pdConnectedHours );
	pMap.Lookup( L"ConnectedMinutes", m_pdConnectedMinutes );

	pMap.Lookup( L"G1Peers", m_pdCount[PROTOCOL_G1][ntNode] );
	pMap.Lookup( L"G1Hubs", m_pdCount[PROTOCOL_G1][ntHub] );
	pMap.Lookup( L"G1Leaves", m_pdCount[PROTOCOL_G1][ntLeaf] );

	pMap.Lookup( L"G2Peers", m_pdCount[PROTOCOL_G2][ntNode] );
	pMap.Lookup( L"G2Hubs", m_pdCount[PROTOCOL_G2][ntHub] );
	pMap.Lookup( L"G2Leaves", m_pdCount[PROTOCOL_G2][ntLeaf] );

	pMap.Lookup( L"EDServers", m_pdCount[PROTOCOL_ED2K][ntHub] );

	pMap.Lookup( L"DCHubs", m_pdCount[PROTOCOL_DC][ntHub] );

	for ( int nP = PROTOCOL_NULL ; nP < PROTOCOL_LAST ; ++nP )	// PROTOCOLID
	{
		for ( int nT = ntNode ; nT < ntLast ; nT++ )
		{
			if ( m_pdCount[ nP ][ nT ] != NULL )
				m_sCount[ nP ][ nT ] = m_pdCount[ nP ][ nT ]->m_sText;
		}
	}

	SetDocument( m_pDocument );

	Update();
}

void CHomeConnectionBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	int nCount[ PROTOCOL_LAST ][ ntLast + 1 ] = {};
	int nTotal = 0;
	CString str;

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );

		if ( pNeighbour->m_nState >= nrsConnected )
		{
			nCount[ pNeighbour->m_nProtocol ][ pNeighbour->m_nNodeType + 1 ] ++;
			nTotal ++;
		}
		else
		{
			nCount[ pNeighbour->m_nProtocol ][ ntNode ] ++;
		}
	}

	nCount[ PROTOCOL_G1 ][ ntNode ] += nCount[ PROTOCOL_NULL ][ ntNode ];	// 0
	nCount[ PROTOCOL_G2 ][ ntNode ] += nCount[ PROTOCOL_NULL ][ ntNode ];	// 0

	BOOL bConnected = Network.IsConnected();
	m_pDocument->ShowGroup( 1, ! bConnected );
	m_pDocument->ShowGroup( 2, bConnected );

	static const bool* pEnable[ PROTOCOL_LAST ] =
	{
		NULL,							// PROTOCOL_NULL
		&Settings.Gnutella1.Enabled,	// PROTOCOL_G1		(Group 10)
		&Settings.Gnutella2.Enabled,	// PROTOCOL_G2		(Group 20)
		&Settings.eDonkey.Enabled,		// PROTOCOL_ED2K	(Group 30)
		NULL,							// PROTOCOL_HTTP
		NULL,							// PROTOCOL_FTP
		&Settings.DC.Enabled,			// PROTOCOL_DC		(Group 60)
		NULL,							// PROTOCOL_BT
		NULL							// PROTOCOL_KAD
	};

	const BOOL bDetail = Settings.General.GUIMode != GUI_BASIC;

	for ( int nProtocol = PROTOCOL_G1 ; nProtocol < PROTOCOL_LAST ; ++nProtocol )	// PROTOCOLID
	{
		if ( ! pEnable[ nProtocol ] )
			continue;

		const int nBase = nProtocol * 10;

		if ( bConnected && *pEnable[ nProtocol ] )
		{
			m_pDocument->ShowGroup( nBase, TRUE );

			if ( nCount[ nProtocol ][ ntLeaf + 1 ] )
			{
				if ( m_pdCount[ nProtocol ][ ntLeaf ] && bDetail )
				{
					str.Format( m_sCount[ nProtocol ][ ntLeaf ], nCount[ nProtocol ][ ntLeaf + 1 ] );
					m_pdCount[ nProtocol ][ ntLeaf ]->SetText( str );

					m_pDocument->ShowGroup( nBase + 3, FALSE );
					m_pDocument->ShowGroup( nBase + 4, FALSE );
					m_pDocument->ShowGroup( nBase + 5, TRUE );
				}
				else // Basic Mode Hub
				{
					m_pDocument->ShowGroup( nBase + 3, TRUE );
					m_pDocument->ShowGroup( nBase + 4, FALSE );
					m_pDocument->ShowGroup( nBase + 5, FALSE );
				}
			}
			else if ( nCount[ nProtocol ][ ntHub + 1 ] )
			{
				if ( m_pdCount[ nProtocol ][ ntHub ] && bDetail )
				{
					str.Format( m_sCount[ nProtocol ][ ntHub ], nCount[ nProtocol ][ ntHub + 1 ] );
					m_pdCount[ nProtocol ][ ntHub ]->SetText( str );

					m_pDocument->ShowGroup( nBase + 3, FALSE );
					m_pDocument->ShowGroup( nBase + 4, TRUE );
					m_pDocument->ShowGroup( nBase + 5, FALSE );
				}
				else // Basic Mode Leaf
				{
					m_pDocument->ShowGroup( nBase + 3, TRUE );
					m_pDocument->ShowGroup( nBase + 4, FALSE );
					m_pDocument->ShowGroup( nBase + 5, FALSE );
				}
			}
			else if ( nCount[ nProtocol ][ ntNode + 1 ] )
			{
				m_pDocument->ShowGroup( nBase + 3, TRUE );
				m_pDocument->ShowGroup( nBase + 4, FALSE );
				m_pDocument->ShowGroup( nBase + 5, FALSE );
			}
			else
			{
				m_pDocument->ShowGroup( nBase + 3, FALSE );
				m_pDocument->ShowGroup( nBase + 4, FALSE );
				m_pDocument->ShowGroup( nBase + 5, FALSE );
			}

			if ( nCount[ nProtocol ][ ntNode + 1 ] ||
				 nCount[ nProtocol ][ ntHub  + 1 ] ||
				 nCount[ nProtocol ][ ntLeaf + 1 ] )
			{
				m_pDocument->ShowGroup( nBase + 1, FALSE );
				m_pDocument->ShowGroup( nBase + 2, FALSE );
			}
			else
			{
				m_pDocument->ShowGroup( nBase + 1, nCount[ nProtocol ][ ntNode ] == 0 );
				m_pDocument->ShowGroup( nBase + 2, nCount[ nProtocol ][ ntNode ] != 0 );
			}
		}
		else
		{
			m_pDocument->ShowGroupRange( nBase, nBase + 9, FALSE );
		}
	}

	if ( m_pdConnectedHours )
	{
		str.Format( L"%I64u ", Statistics.Today.Timer.Connected / ( 60 * 60 ) );
		m_pdConnectedHours->SetText( str );
	}
	if ( m_pdConnectedMinutes )
	{
		str.Format( L"%I64u ", ( Statistics.Today.Timer.Connected / 60 ) % 60 );
		m_pdConnectedMinutes->SetText( str );
	}

	OnSize( 0, 0, 0 );		// Fix all taskbox heights if Sidebar width changed

	CRichTaskBox::Update();
}


/////////////////////////////////////////////////////////////////////////////
// CHomeLibraryBox construction

CHomeLibraryBox::CHomeLibraryBox()
	: m_pdLibraryFiles	( NULL )
	, m_pdLibraryVolume	( NULL )
	, m_pdLibraryHashRemaining	( NULL )
	, m_hHand			( NULL )
	, m_pHover			( NULL )
	, m_nIndex			( -1 )
{
	SetPrimary( FALSE );
}

CHomeLibraryBox::~CHomeLibraryBox()
{
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		delete m_pList.GetAt( nItem );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeLibraryBox message handlers

int CHomeLibraryBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CRichTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_pFont.CreateFont( -(int)(Settings.Fonts.DefaultSize - 1), 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_hHand = theApp.LoadCursor( IDC_HAND );

	m_wndTip.Create( this, &Settings.Interface.TipLibrary );

	return 0;
}

void CHomeLibraryBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdLibraryFiles = m_pdLibraryVolume = m_pdLibraryHashRemaining = NULL;

	SetCaptionmark( L"CHomeLibraryBox.Caption" );

	CXMLElement* pXML = Skin.GetDocument( L"CHomeLibraryBox" );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( L"title", L"Library" ) );
	HICON hIcon = CoolInterface.ExtractIcon( IDR_LIBRARYFRAME, Settings.General.LanguageRTL );
	if ( hIcon ) SetIcon( hIcon );

	m_pDocument = new CRichDocument();

	CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( L"LibraryFiles", m_pdLibraryFiles );
	pMap.Lookup( L"LibraryVolume", m_pdLibraryVolume );
	pMap.Lookup( L"LibraryHashRemaining", m_pdLibraryHashRemaining );

	SetDocument( m_pDocument );

	Update();

	// Update Dropshadow	(Note: Caused app freeze when allowing hovered item during skin change)
	m_wndTip.DestroyWindow();
	m_wndTip.Create( this, &Settings.Interface.TipLibrary );
}

void CHomeLibraryBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	BOOL bChanged = FALSE;
	int nCount = 0;
	CString str;

	for ( INT_PTR nItem = m_pList.GetSize() - 1 ; nItem >= 0 ; nItem-- )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( ! LibraryHistory.Check( pItem->m_pRecent, 6 ) ||
			NULL == pItem->m_pRecent->m_pFile )
		{
			delete pItem;
			m_pList.RemoveAt( nItem );
			bChanged = TRUE;
		}
	}

	for ( POSITION pos = LibraryHistory.GetIterator() ; pos && nCount < 6 ; )
	{
		CLibraryRecent* pRecent = LibraryHistory.GetNext( pos );
		if ( pRecent->m_pFile == NULL ) continue;

		INT_PTR nItem = m_pList.GetSize() - 1;
		for ( ; nItem >= 0 ; nItem-- )
		{
			Item* pItem = m_pList.GetAt( nItem );
			if ( pItem->m_pRecent == pRecent ) break;
		}

		if ( nItem < 0 )
		{
			Item* pItem			= new Item();
			pItem->m_pRecent	= pRecent;
			pItem->m_nIndex		= pRecent->m_pFile->m_nIndex;
			pItem->m_sText		= pRecent->m_pFile->m_sName;
			pItem->m_nIcon16	= ShellIcons.Get( pRecent->m_pFile->GetPath(), 16 );

			m_pList.InsertAt( nCount, pItem );
			bChanged = TRUE;
		}

		nCount++;
	}

	nCount = static_cast< int >( m_pList.GetSize() * 18 );
	if ( nCount ) nCount += 6;

	m_pDocument->ShowGroup( 2, m_pList.GetSize() == 0 );

	QWORD nVolume;
	DWORD nFiles;

	LibraryMaps.GetStatistics( &nFiles, &nVolume );

	if ( m_pdLibraryFiles )
	{
		str.Format( L"%lu ", nFiles );
		if ( m_pdLibraryFiles->m_sText.Compare( str ) != 0 )
		{
			m_pdLibraryFiles->SetText( str );
			bChanged = TRUE;
		}
	}

	if ( m_pdLibraryVolume )
	{
		str = Settings.SmartVolume( nVolume, KiloBytes );
		if ( m_pdLibraryVolume->m_sText.Compare( str ) != 0 )
		{
			m_pdLibraryVolume->SetText( str );
			bChanged = TRUE;
		}
	}

	size_t nHashing = LibraryBuilder.GetRemaining();

	if ( nHashing > 0 )
	{
		str.Format( L"%lu ", nHashing );
		if ( m_pdLibraryHashRemaining && m_pdLibraryHashRemaining->m_sText.Compare( str ) != 0 )
		{
			m_pdLibraryHashRemaining->SetText( str );
			bChanged = TRUE;
		}
		m_pDocument->ShowGroup( 1, TRUE );

		BOOL bPriority = LibraryBuilder.GetBoostPriority();
		m_pDocument->ShowGroup( 3, bPriority == FALSE );
		m_pDocument->ShowGroup( 4, bPriority == TRUE );
	}
	else
	{
		m_pDocument->ShowGroup( 1, FALSE );
		m_pDocument->ShowGroup( 3, FALSE );
		m_pDocument->ShowGroup( 4, FALSE );
	}

//	OnSize( 0, 0, 0 );		// Duplicate fix all taskbox heights if Sidebar width changed

	if ( GetView().IsModified() )
	{
		CRect rc;
		GetClientRect( &rc );
		m_nWidth = rc.Width();
		nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
	}
	else
	{
		CRect rc;
		m_wndView.GetWindowRect( &rc );
		ScreenToClient( &rc );

		if ( rc.top != nCount )
		{
			m_nWidth = rc.Width();
			nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
		}
		else
		{
			m_wndView.GetClientRect( &rc );
			nCount += rc.Height();
		}
	}

	SetSize( nCount );

	if ( bChanged )
	{
		m_pHover = NULL;
		KillTimer( 2 );
		m_wndView.Invalidate();
	}
}

void CHomeLibraryBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );

	if ( m_nWidth != cx )
	{
		m_nWidth = cx;
		int nCount = static_cast< int >( m_pList.GetSize() * 18 );
		if ( nCount ) nCount += 6;
		nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
		SetSize( nCount );
	}
}

void CHomeLibraryBox::OnPaint()
{
	CRect rcClient, rcIcon, rcText;
	CPaintDC dc( this );

	GetClientRect( &rcClient );
	m_wndView.GetClientRect( &rcIcon );
	rcClient.bottom -= rcIcon.Height();
	rcClient.top += 6;

	rcIcon.SetRect( 4, rcClient.top, 4 + 16, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );

	dc.SetBkMode( OPAQUE );		// ToDo: Transparent for skinning
	dc.SetBkColor( Colors.m_crTaskBoxClient );	// Was Colors.m_crRichdocBack
	dc.SetTextColor( Colors.m_crTextLink );

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFont );

	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		ShellIcons.Draw( &dc, pItem->m_nIcon16, 16, rcIcon.left, rcIcon.top, Colors.m_crTaskBoxClient );	// Was Colors.m_crRichdocBack

		CString str = pItem->m_sText;

		if ( dc.GetTextExtent( str ).cx > rcText.Width() - 8 )
		{
			while ( str.GetLength() && dc.GetTextExtent( str + L'\x2026' ).cx > rcText.Width() - 8 )
			{
				str = str.Left( str.GetLength() - 1 );
			}
			str += L'\x2026';
		}

		dc.SetTextColor( m_pHover == pItem ? Colors.m_crTextLinkHot : Colors.m_crTextLink );
		dc.ExtTextOut( rcText.left + 4, rcText.top + 2, ETO_CLIPPED|ETO_OPAQUE, &rcText, str, NULL );

		dc.ExcludeClipRect( &rcIcon );
		dc.ExcludeClipRect( &rcText );

		rcIcon.OffsetRect( 0, 18 );
		rcText.OffsetRect( 0, 18 );
	}

	rcClient.top = 0;
	dc.FillSolidRect( &rcClient, Colors.m_crTaskBoxClient );	// Was Colors.m_crRichdocBack
	dc.SelectObject( pOldFont );
}

CHomeLibraryBox::Item* CHomeLibraryBox::HitTest(const CPoint& point) const
{
	CRect rcClient, rcIcon, rcText;

	GetClientRect( &rcClient );
	rcClient.top += 6;

	rcIcon.SetRect( 4, rcClient.top, 4 + 16, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );

	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( rcIcon.PtInRect( point ) || rcText.PtInRect( point ) ) return pItem;

		rcIcon.OffsetRect( 0, 18 );
		rcText.OffsetRect( 0, 18 );
	}

	return NULL;
}

BOOL CHomeLibraryBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	Item* pItem = HitTest( point );

	if ( pItem != NULL && Settings.Interface.TipLibrary && ! Skin.m_bSkinChanging )		// App freeze workaround
		m_wndTip.Show( pItem->m_nIndex );
	else
		m_wndTip.Hide();

	if ( pItem != m_pHover )
	{
		if ( pItem != NULL && m_pHover == NULL )
			SetTimer( 2, 200, NULL );
		else if ( pItem == NULL && m_pHover != NULL )
			KillTimer( 2 );

		m_pHover = pItem;
		Invalidate();
	}

	if ( m_pHover != NULL )
	{
		::SetCursor( m_hHand );
		return TRUE;
	}

	return CTaskBox::OnSetCursor( pWnd, nHitTest, message );
}

void CHomeLibraryBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_wndTip.Hide();

	if ( Item* pItem = HitTest( point ) )
	{
		CSingleLock pLock( &Library.m_pSection );
		if ( ! SafeLock( pLock ) ) return;

		if ( CLibraryFile* pFile = Library.LookupFile( pItem->m_nIndex ) )
		{
			if ( pFile->IsAvailable() )
			{
				CString strPath = pFile->GetPath();
				pLock.Unlock();
				CFileExecutor::Execute( strPath );
			}
			else
			{
				pLock.Unlock();
			}
			m_pHover = NULL;
			KillTimer( 2 );
			Invalidate();
		}
	}

	CTaskBox::OnLButtonUp( nFlags, point );
}

void CHomeLibraryBox::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	m_wndTip.Hide();

//	if ( m_pHover )
	{
		m_nIndex = -1;
		POINT pTest( point );
		ScreenToClient( &pTest );
		if ( Item* pItem = HitTest( pTest ) )
		{
			m_nIndex = pItem->m_nIndex;

			// Test only. (Prefer context menu)
			if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0 )
			{
				CSingleLock pLock( &Library.m_pSection );
				if ( ! SafeLock( pLock ) ) return;

				if ( CLibraryFile* pFile = Library.LookupFile( pItem->m_nIndex ) )
				{
					pLock.Unlock();
					if ( MsgBox( L"Remove item from history?", MB_ICONQUESTION|MB_YESNO|MB_SETFOREGROUND, 0, NULL, 30 ) == IDYES )
					{
						LibraryHistory.OnFileDelete( pFile );
						m_pHover = NULL;
						KillTimer( 2 );
						Invalidate();
						return;
					}
				}
			}
		}

		Skin.TrackPopupMenu( L"CLibraryHistoryPanel", point, ID_LIBRARY_CLEAR_HISTORY );

		// Note command handlers in WndHome
	}
}

void CHomeLibraryBox::OnTimer(UINT_PTR nIDEvent)
{
	CTaskBox::OnTimer( nIDEvent );

	CPoint point;
	CRect rect;

	GetCursorPos( &point );
	GetWindowRect( &rect );

	if ( rect.PtInRect( point ) ) return;

	KillTimer( 2 );

	if ( m_pHover != NULL )
	{
		m_pHover = NULL;
		Invalidate();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CHomeDownloadsBox construction

CHomeDownloadsBox::CHomeDownloadsBox()
	: m_pdDownloadsNone	( NULL )
	, m_pdDownloadsOne	( NULL )
	, m_pdDownloadsMany	( NULL )
	, m_pdDownloadedNone ( NULL )
	, m_pdDownloadedOne	( NULL )
	, m_pdDownloadedMany ( NULL )
	, m_pdDownloadedVolume ( NULL )
	, m_hHand			( NULL )
	, m_pHover			( NULL )
{
	SetPrimary( FALSE );
}

CHomeDownloadsBox::~CHomeDownloadsBox()
{
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		delete m_pList.GetAt( nItem );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeDownloadsBox message handlers

int CHomeDownloadsBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CRichTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_pFont.CreateFont( -(int)(Settings.Fonts.DefaultSize - 1), 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_hHand = theApp.LoadCursor( IDC_HAND );

	m_wndTip.Create( this, &Settings.Interface.TipDownloads );

	return 0;
}

void CHomeDownloadsBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdDownloadsNone = m_pdDownloadsOne = m_pdDownloadsMany = NULL;
	m_pdDownloadedNone = m_pdDownloadedOne = m_pdDownloadedMany = m_pdDownloadedVolume = NULL;

	SetCaptionmark( L"CHomeDownloadsBox.Caption" );

	CXMLElement* pXML = Skin.GetDocument( L"CHomeDownloadsBox" );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( L"title", L"Downloads" ) );
	HICON hIcon = CoolInterface.ExtractIcon( IDR_DOWNLOADSFRAME, Settings.General.LanguageRTL );
	if ( hIcon ) SetIcon( hIcon );

	m_pDocument = new CRichDocument();

	CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( L"DownloadsNone", m_pdDownloadsNone );
	pMap.Lookup( L"DownloadsOne", m_pdDownloadsOne );
	pMap.Lookup( L"DownloadsMany", m_pdDownloadsMany );
	pMap.Lookup( L"DownloadedNone", m_pdDownloadedNone );
	pMap.Lookup( L"DownloadedOne", m_pdDownloadedOne );
	pMap.Lookup( L"DownloadedMany", m_pdDownloadedMany );
	pMap.Lookup( L"DownloadedVolume", m_pdDownloadedVolume );

	if ( m_pdDownloadsMany ) m_sDownloadsMany = m_pdDownloadsMany->m_sText;
	if ( m_pdDownloadedMany ) m_sDownloadedMany = m_pdDownloadedMany->m_sText;

	SetDocument( m_pDocument );		// Was GetView().SetDocument( m_pDocument );

	Update();

	// Update Dropshadow
	m_wndTip.DestroyWindow();
	m_wndTip.Create( this, &Settings.Interface.TipDownloads );
}

void CHomeDownloadsBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	BOOL bChanged = FALSE;
	CString str;

	for ( INT_PTR nItem = m_pList.GetSize() - 1 ; nItem >= 0 ; nItem-- )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( ! Downloads.CheckActive( pItem->m_pDownload, 6 ) )
		{
			delete pItem;
			m_pList.RemoveAt( nItem );
			bChanged = TRUE;
			m_pHover = NULL;
			KillTimer( 2 );
		}
	}

	int nCount = 0;
	int nInsert = 0;

	for ( POSITION pos = Downloads.GetReverseIterator() ; pos && nCount < 6 ; )
	{
		CDownload* pDownload = Downloads.GetPrevious( pos );
		if ( pDownload->IsPaused() ) continue;
		if ( pDownload->IsCompleted() ) continue;

		Item* pItem = NULL;

		for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
		{
			pItem = m_pList.GetAt( nItem );
			if ( pItem->m_pDownload == pDownload ) break;
			pItem = NULL;
		}

		if ( pItem == NULL )
		{
			pItem				= new Item();
			pItem->m_pDownload	= pDownload;
			pItem->m_sText		= pDownload->GetDisplayName();
			pItem->m_nIcon16	= ShellIcons.Get( pItem->m_sText, 16 );
			m_pList.InsertAt( nInsert++, pItem );
			bChanged = TRUE;
			m_pHover = NULL;
			KillTimer( 2 );
		}

		QWORD nComplete	= pDownload->GetVolumeComplete();
		BOOL bPaused	= pDownload->GetFirstTransfer() == NULL;

		if ( pItem->m_nComplete != nComplete ||
			 pItem->m_bPaused != bPaused ||
			 pItem->m_nSize != pDownload->m_nSize )
		{
			pItem->m_nSize		= pDownload->m_nSize;
			pItem->m_nComplete	= nComplete;
			pItem->m_bPaused	= bPaused;
			bChanged = TRUE;
		}

		nCount++;
	}

	nCount = static_cast< int >( m_pList.GetSize() * 18 );
	if ( nCount ) nCount += 6;

	m_pDocument->ShowGroup( 1, m_pList.GetSize() == 0 );

	int nActive = (int)CGraphItem::GetValue( GRC_DOWNLOADS_FILES );

	if ( nActive > 1 )
	{
		if ( m_pdDownloadsMany )
		{
			str.Format( m_sDownloadsMany, nActive );
			m_pdDownloadsMany->SetText( str );
			m_pdDownloadsMany->Show( TRUE );
		}
		if ( m_pdDownloadsOne )  m_pdDownloadsOne->Show( FALSE );
		if ( m_pdDownloadsNone ) m_pdDownloadsNone->Show( FALSE );
	}
	else if ( nActive == 1 )
	{
		if ( m_pdDownloadsMany ) m_pdDownloadsMany->Show( FALSE );
		if ( m_pdDownloadsOne )  m_pdDownloadsOne->Show( TRUE );
		if ( m_pdDownloadsNone ) m_pdDownloadsNone->Show( FALSE );
	}
	else
	{
		if ( m_pdDownloadsMany ) m_pdDownloadsMany->Show( FALSE );
		if ( m_pdDownloadsOne )  m_pdDownloadsOne->Show( FALSE );
		if ( m_pdDownloadsNone ) m_pdDownloadsNone->Show( TRUE );
	}

	m_pDocument->ShowGroup( 1, nActive == 0 );

	// Download Stats Count
	if ( Statistics.Today.Downloads.Files == 0 )
	{
		if ( m_pdDownloadedNone ) m_pdDownloadedNone->Show( TRUE );
		if ( m_pdDownloadedOne )  m_pdDownloadedOne->Show( FALSE );
		if ( m_pdDownloadedMany ) m_pdDownloadedMany->Show( FALSE );
	}
	else if ( Statistics.Today.Downloads.Files == 1 )
	{
		if ( m_pdDownloadedNone ) m_pdDownloadedNone->Show( FALSE );
		if ( m_pdDownloadedOne )  m_pdDownloadedOne->Show( TRUE );
		if ( m_pdDownloadedMany ) m_pdDownloadedMany->Show( FALSE );
	}
	else
	{
		str.Format( m_sDownloadedMany, (int)Statistics.Today.Downloads.Files );

		if ( m_pdDownloadedNone ) m_pdDownloadedNone->Show( FALSE );
		if ( m_pdDownloadedOne )  m_pdDownloadedOne->Show( FALSE );

		if ( m_pdDownloadedMany )
		{
			m_pdDownloadedMany->SetText( str );
			m_pdDownloadedMany->Show( TRUE );
		}
	}

	if ( m_pdDownloadedVolume )
	{
		str = Settings.SmartVolume( Statistics.Today.Downloads.Volume, KiloBytes );
		m_pdDownloadedVolume->SetText( str );
		if ( Statistics.Last.Bandwidth.Incoming > 120 )		// More activity than idle connection
			GetView().Invalidate();
	}

	if ( GetView().IsModified() )
	{
		CRect rc;
		GetClientRect( &rc );
		m_nWidth = rc.Width();
		nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
	}
	else
	{
		CRect rc;
		m_wndView.GetWindowRect( &rc );
		ScreenToClient( &rc );

		if ( rc.top != nCount )
		{
			m_nWidth = rc.Width();
			nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
		}
		else
		{
			m_wndView.GetClientRect( &rc );
			nCount += rc.Height();
		}
	}

	SetSize( nCount );
}

void CHomeDownloadsBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );

	if ( m_nWidth != cx )
	{
		m_nWidth = cx;
		int nCount = static_cast< int >( m_pList.GetSize() * 18 );
		if ( nCount ) nCount += 6;
		nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
		SetSize( nCount );
	}
}

void CHomeDownloadsBox::OnPaint()
{
	CRect rcClient, rcIcon, rcText;
	CPaintDC dc( this );

	GetClientRect( &rcClient );
	m_wndView.GetClientRect( &rcIcon );
	rcClient.bottom -= rcIcon.Height();
	rcClient.top += 6;

	rcIcon.SetRect( 4, rcClient.top, 4 + 20, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );
	rcIcon.DeflateRect( 0, 2 );

	dc.SetBkMode( OPAQUE ); 	// ToDo: Transparent for skinning
	dc.SetBkColor( Colors.m_crTaskBoxClient );	// Was Colors.m_crRichdocBack
	dc.SetTextColor( Colors.m_crTextLink );

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFont );

	COLORREF crAlt[3] = { RGB( 0, 150, 255 ), RGB( 190, 0, 0 ), RGB( 0, 150, 0 ) };

	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( pItem->m_nComplete == 0 || pItem->m_nSize == SIZE_UNKNOWN )
		{
			CRect rc( rcIcon.left, rcIcon.top, rcIcon.left + 16, rcIcon.top + 16 );
			ShellIcons.Draw( &dc, pItem->m_nIcon16, 16, rc.left, rc.top, Colors.m_crTaskBoxClient );	// Was Colors.m_crRichdocBack
			dc.ExcludeClipRect( &rc );
		}
		else
		{
			COLORREF cr = pItem->m_bPaused ? Colors.m_crNetworkNull : crAlt[ nItem % 3 ];
			dc.Draw3dRect( &rcIcon, Colors.m_crFragmentBorder, Colors.m_crFragmentBorder );
			rcIcon.DeflateRect( 1, 1 );
			CFragmentBar::DrawFragment( &dc, &rcIcon, pItem->m_nSize, 0, pItem->m_nComplete, cr, TRUE, TRUE );
			dc.FillSolidRect( &rcIcon, Colors.m_crTaskBoxClient );	// Was Colors.m_crRichdocBack
			rcIcon.InflateRect( 1, 1 );
			dc.ExcludeClipRect( &rcIcon );
		}

		CString str = pItem->m_sText;

		if ( dc.GetTextExtent( str ).cx > rcText.Width() - 8 )
		{
			while ( str.GetLength() && dc.GetTextExtent( str + L'\x2026' ).cx > rcText.Width() - 8 )
			{
				str = str.Left( str.GetLength() - 1 );
			}
			str += L'\x2026';
		}

		dc.SetTextColor( m_pHover == pItem ? Colors.m_crTextLinkHot : Colors.m_crTextLink );
		dc.ExtTextOut( rcText.left + 4, rcText.top + 2, ETO_CLIPPED|ETO_OPAQUE, &rcText, str, NULL );

		dc.ExcludeClipRect( &rcText );

		rcIcon.OffsetRect( 0, 18 );
		rcText.OffsetRect( 0, 18 );
	}

	rcClient.top = 0;
	dc.FillSolidRect( &rcClient, Colors.m_crTaskBoxClient );	// Was Colors.m_crRichdocBack
	dc.SelectObject( pOldFont );
}

CHomeDownloadsBox::Item* CHomeDownloadsBox::HitTest(const CPoint& point) const
{
	CRect rcClient, rcIcon, rcText;

	GetClientRect( &rcClient );
	rcClient.top += 6;

	rcIcon.SetRect( 4, rcClient.top, 4 + 20, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );

	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( rcIcon.PtInRect( point ) || rcText.PtInRect( point ) ) return pItem;

		rcIcon.OffsetRect( 0, 18 );
		rcText.OffsetRect( 0, 18 );
	}

	return NULL;
}

BOOL CHomeDownloadsBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	Item* pItem = HitTest( point );

	if ( pItem != NULL )
		m_wndTip.Show( pItem->m_pDownload );
	else
		m_wndTip.Hide();

	if ( pItem != m_pHover )
	{
		if ( pItem != NULL && m_pHover == NULL )
			SetTimer( 2, 200, NULL );
		else if ( pItem == NULL && m_pHover != NULL )
			KillTimer( 2 );

		m_pHover = pItem;
		Invalidate();
	}

	if ( m_pHover != NULL )
	{
		::SetCursor( m_hHand );
		return TRUE;
	}

	return CTaskBox::OnSetCursor( pWnd, nHitTest, message );
}

void CHomeDownloadsBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_wndTip.Hide();

	if ( Item* pItem = HitTest( point ) )
	{
		m_pHover = NULL;
		KillTimer( 2 );
		Invalidate();

		ExecuteDownload( pItem->m_pDownload );
	}

	CTaskBox::OnLButtonUp( nFlags, point );
}

void CHomeDownloadsBox::OnTimer(UINT_PTR nIDEvent)
{
	CTaskBox::OnTimer( nIDEvent );

	CPoint point;
	CRect rect;

	GetCursorPos( &point );
	GetWindowRect( &rect );

	if ( rect.PtInRect( point ) ) return;

	KillTimer( 2 );

	if ( m_pHover != NULL )
	{
		m_pHover = NULL;
		Invalidate();
	}
}

BOOL CHomeDownloadsBox::ExecuteDownload(CDownload* pDownload)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! SafeLock( pLock ) ) return FALSE;
	if ( ! Downloads.Check( pDownload ) ) return FALSE;

	if ( ! pDownload->Launch( -1, &pLock, FALSE ) )
		PostMainWndMessage( WM_COMMAND, ID_VIEW_DOWNLOADS );

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CHomeUploadsBox construction

CHomeUploadsBox::CHomeUploadsBox()
	: m_pdUploadsNone	( NULL )
	, m_pdUploadsOne	( NULL )
	, m_pdUploadsMany	( NULL )
	, m_pdUploadedNone	( NULL )
	, m_pdUploadedOne	( NULL )
	, m_pdUploadedMany	( NULL )
	, m_pdTorrentsOne	( NULL )
	, m_pdTorrentsMany	( NULL )
{
	SetPrimary( FALSE );
}

CHomeUploadsBox::~CHomeUploadsBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomeUploadsBox message handlers

void CHomeUploadsBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdUploadsNone = m_pdUploadsOne = m_pdUploadsMany = NULL;
	m_pdUploadedNone = m_pdUploadedOne = m_pdUploadedMany = NULL;
	m_pdTorrentsOne = m_pdTorrentsMany = NULL;

	SetCaptionmark( L"CHomeUploadsBox.Caption" );

	CXMLElement* pXML = Skin.GetDocument( L"CHomeUploadsBox" );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( L"title", L"Uploads" ) );
	HICON hIcon = CoolInterface.ExtractIcon( IDR_UPLOADSFRAME, Settings.General.LanguageRTL );
	if ( hIcon ) SetIcon( hIcon );

	m_pDocument = new CRichDocument();

	CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( L"UploadsNone", m_pdUploadsNone );
	pMap.Lookup( L"UploadsOne", m_pdUploadsOne );
	pMap.Lookup( L"UploadsMany", m_pdUploadsMany );
	pMap.Lookup( L"UploadedNone", m_pdUploadedNone );
	pMap.Lookup( L"UploadedOne", m_pdUploadedOne );
	pMap.Lookup( L"UploadedMany", m_pdUploadedMany );
	pMap.Lookup( L"TorrentsOne", m_pdTorrentsOne );
	pMap.Lookup( L"TorrentsMany", m_pdTorrentsMany );

	if ( m_pdUploadedOne ) m_sUploadedOne = m_pdUploadedOne->m_sText;
	if ( m_pdUploadedMany ) m_sUploadedMany = m_pdUploadedMany->m_sText;

	if ( m_pdUploadsMany ) m_sUploadsMany = m_pdUploadsMany->m_sText;
	if ( m_pdTorrentsMany ) m_sTorrentsMany = m_pdTorrentsMany->m_sText;

	SetDocument( m_pDocument );

	Update();
}

void CHomeUploadsBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CString str;

	// Active Uploads Count
	int nCount = (int)CGraphItem::GetValue( GRC_UPLOADS_TRANSFERS );

	if ( nCount > 1 )
	{
		if ( m_pdUploadsMany )
		{
			str.Format( m_sUploadsMany, nCount );
			m_pdUploadsMany->SetText( str );
			m_pdUploadsMany->Show( TRUE );
		}
		if ( m_pdUploadsOne )  m_pdUploadsOne->Show( FALSE );
		if ( m_pdUploadsNone ) m_pdUploadsNone->Show( FALSE );
	}
	else if ( nCount == 1 )
	{
		if ( m_pdUploadsMany ) m_pdUploadsMany->Show( FALSE );
		if ( m_pdUploadsOne )  m_pdUploadsOne->Show( TRUE );
		if ( m_pdUploadsNone ) m_pdUploadsNone->Show( FALSE );
	}
	else
	{
		if ( m_pdUploadsMany ) m_pdUploadsMany->Show( FALSE );
		if ( m_pdUploadsOne )  m_pdUploadsOne->Show( FALSE );
		if ( m_pdUploadsNone ) m_pdUploadsNone->Show( TRUE );
	}

	// Torrent Seed Count
	nCount = Downloads.GetSeedCount();
	m_pDocument->ShowGroup( 1, TRUE );

	if ( nCount > 1 )
	{
		if ( m_pdTorrentsMany )
		{
			str.Format( m_sTorrentsMany, nCount );
			m_pdTorrentsMany->SetText( str );
			m_pdTorrentsMany->Show( TRUE );
		}
		if ( m_pdTorrentsOne )  m_pdTorrentsOne->Show( FALSE );
	}
	else if ( nCount == 1 )
	{
		if ( m_pdTorrentsMany ) m_pdTorrentsMany->Show( FALSE );
		if ( m_pdTorrentsOne )  m_pdTorrentsOne->Show( TRUE );
	}
	else
	{
		m_pDocument->ShowGroup( 1, FALSE );
	}

	// Upload Stats Count
	if ( Statistics.Today.Uploads.Files == 0 )
	{
		if ( m_pdUploadedNone ) m_pdUploadedNone->Show( TRUE );
		if ( m_pdUploadedOne )  m_pdUploadedOne->Show( FALSE );
		if ( m_pdUploadedMany ) m_pdUploadedMany->Show( FALSE );
	}
	else if ( Statistics.Today.Uploads.Files == 1 )
	{
		str.Format( m_sUploadedOne, Settings.SmartVolume( Statistics.Today.Uploads.Volume, KiloBytes ) );

		if ( m_pdUploadedNone ) m_pdUploadedNone->Show( FALSE );
		if ( m_pdUploadedMany ) m_pdUploadedMany->Show( FALSE );

		if ( m_pdUploadedOne )
		{
			m_pdUploadedOne->SetText( str );
			m_pdUploadedOne->Show( TRUE );
		}
	}
	else
	{
		str.Format( m_sUploadedMany, (int)Statistics.Today.Uploads.Files,
			Settings.SmartVolume( Statistics.Today.Uploads.Volume, KiloBytes ) );

		if ( m_pdUploadedNone ) m_pdUploadedNone->Show( FALSE );
		if ( m_pdUploadedOne )  m_pdUploadedOne->Show( FALSE );

		if ( m_pdUploadedMany )
		{
			m_pdUploadedMany->SetText( str );
			m_pdUploadedMany->Show( TRUE );
		}
	}

	CRichTaskBox::Update();
}
