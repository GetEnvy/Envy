//
// WndSecurity.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
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
#include "WndSecurity.h"
#include "DlgSecureRule.h"
#include "SecureRule.h"
#include "Security.h"
#include "LiveList.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

const static UINT nImageIDs[] =
{
	ID_SECURITY_EDIT,		// IDR_SECURITYFRAME,
	IDI_SECURITY_ACCEPT,
	IDI_SECURITY_DENY,
	NULL
};

IMPLEMENT_SERIAL(CSecurityWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSecurityWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_RULES, OnCustomDrawList)
	ON_NOTIFY(NM_DBLCLK, IDC_RULES, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_RULES, OnSortList)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_EDIT, OnUpdateSecurityEdit)
	ON_COMMAND(ID_SECURITY_EDIT, OnSecurityEdit)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_RESET, OnUpdateSecurityReset)
	ON_COMMAND(ID_SECURITY_RESET, OnSecurityReset)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_REMOVE, OnUpdateSecurityRemove)
	ON_COMMAND(ID_SECURITY_REMOVE, OnSecurityRemove)
	ON_COMMAND(ID_SECURITY_ADD, OnSecurityAdd)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_POLICY_ACCEPT, OnUpdateSecurityPolicyAccept)
	ON_COMMAND(ID_SECURITY_POLICY_ACCEPT, OnSecurityPolicyAccept)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_POLICY_DENY, OnUpdateSecurityPolicyDeny)
	ON_COMMAND(ID_SECURITY_POLICY_DENY, OnSecurityPolicyDeny)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_MOVE_UP, OnUpdateSecurityMoveUp)
	ON_COMMAND(ID_SECURITY_MOVE_UP, OnSecurityMoveUp)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_MOVE_DOWN, OnUpdateSecurityMoveDown)
	ON_COMMAND(ID_SECURITY_MOVE_DOWN, OnSecurityMoveDown)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_EXPORT, OnUpdateSecurityExport)
	ON_COMMAND(ID_SECURITY_EXPORT, OnSecurityExport)
	ON_COMMAND(ID_SECURITY_IMPORT, OnSecurityImport)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSecurityWnd construction

CSecurityWnd::CSecurityWnd()
{
	Create( IDR_SECURITYFRAME );
}

CSecurityWnd::~CSecurityWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSecurityWnd message handlers

int CSecurityWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT|LVS_SHOWSELALWAYS, rectDefault, this, IDC_RULES );
	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );

	m_wndList.InsertColumn( COL_SECURITY_CONTENT, L"Address / Content", LVCFMT_LEFT, 200 );
	m_wndList.InsertColumn( COL_SECURITY_HITS, L"Hits", LVCFMT_CENTER, 60 );
	m_wndList.InsertColumn( COL_SECURITY_NUM, L"#", LVCFMT_CENTER, 30 );
	m_wndList.InsertColumn( COL_SECURITY_ACTION, L"Action", LVCFMT_CENTER, 60 );
	m_wndList.InsertColumn( COL_SECURITY_EXPIRES, L"Expires", LVCFMT_CENTER, 60 );
	m_wndList.InsertColumn( COL_SECURITY_TYPE, L"Match", LVCFMT_CENTER, 60 );
	m_wndList.InsertColumn( COL_SECURITY_COMMENT, L"Comment", LVCFMT_LEFT, 200 );

	m_pSizer.Attach( &m_wndList );

	// Obsolete:
//	CBitmap bmBase;
//	bmBase.LoadBitmap( IDB_SECURITY );
//	if ( Settings.General.LanguageRTL )
//		bmBase.m_hObject = CreateMirroredBitmap( (HBITMAP) bmBase.m_hObject );

//	m_gdiImageList.Create( 16, 16, ILC_MASK|ILC_COLOR32, 3, 1 ) ||
//	m_gdiImageList.Create( 16, 16, ILC_MASK|ILC_COLOR24, 3, 1 ) ||
//	m_gdiImageList.Create( 16, 16, ILC_MASK|ILC_COLOR16, 3, 1 );
//	m_gdiImageList.Add( &bmBase, RGB( 0, 255, 0 ) );
//	AddIcon( IDR_SECURITYFRAME, m_gdiImageList );
//	AddIcon( IDI_SECURITY_ACCEPT, m_gdiImageList );
//	AddIcon( IDI_SECURITY_DENY, m_gdiImageList );

//	CoolInterface.LoadIconsTo( m_gdiImageList, nImageIDs );
//	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.SetFont( &theApp.m_gdiFont );

	LoadState( L"CSecurityWnd", TRUE );

	Update();

	return 0;
}

void CSecurityWnd::OnDestroy()
{
	Security.Save();

	Settings.SaveList( L"CSecurityWnd", &m_wndList );
	SaveState( L"CSecurityWnd" );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CSecurityWnd operations

void CSecurityWnd::Update(int nColumn, BOOL bSort)
{
	Security.Expire();

	// Columns set in CSecureRule::ToList()
	CAutoPtr< CLiveList > pLiveList( Security.GetList() );

	if ( nColumn >= 0 )
		SetWindowLongPtr( m_wndList.GetSafeHwnd(), GWLP_USERDATA, 0 - nColumn - 1 );

	pLiveList->Apply( &m_wndList, bSort );

	m_tLastUpdate = GetTickCount();		// Update time after it's done doing work
}

CSecureRule* CSecurityWnd::GetItem(int nItem)
{
	if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
	{
		CSecureRule* pRule = (CSecureRule*)m_wndList.GetItemData( nItem );
		if ( Security.Check( pRule ) ) return pRule;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CSecurityWnd message handlers

void CSecurityWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( ! m_wndList ) return;

	CPanelWnd::OnSize( nType, cx, cy );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy - Settings.Skin.ToolbarHeight, SWP_NOZORDER );
	SizeListAndBar( &m_wndList, &m_wndToolBar );
}

void CSecurityWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 && IsPartiallyVisible() )
	{
		const DWORD tTicks = GetTickCount();
		const DWORD tDelay = max( ( 2 * (DWORD)Security.GetCount() ), 1000ul );	// Delay based on size of list

		if ( ( tTicks - m_tLastUpdate ) > tDelay )
		{
			if ( tDelay < 2000 )		// Sort if list is under 1000
				Update();
			else						// Otherwise just refresh values
				Update( -1, FALSE );
		}
	}
}

void CSecurityWnd::OnSkinChange()
{
	OnSize( 0, 0, 0 );
	CPanelWnd::OnSkinChange();

	Settings.LoadList( L"CSecurityWnd", &m_wndList, -3 );
	Skin.CreateToolBar( L"CSecurityWnd", &m_wndToolBar );

	CoolInterface.LoadIconsTo( m_gdiImageList, nImageIDs );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	if ( m_wndList.SetBkImage( Skin.GetWatermark( L"CSecurityWnd" ) ) || m_wndList.SetBkImage( Skin.GetWatermark( L"System.Windows" ) ) )		// Images.m_bmSystemWindow.m_hObject
		m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );	// No LVS_EX_DOUBLEBUFFER
	else
		m_wndList.SetBkColor( Colors.m_crWindow );
}

void CSecurityWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	if ( ! ::IsWindow( m_wndList.GetSafeHwnd() ) ) return;

	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		LV_ITEM pItem;
		pItem.mask		= LVIF_IMAGE;
		pItem.iItem		= static_cast< int >( pDraw->nmcd.dwItemSpec );
		pItem.iSubItem	= 0;
		m_wndList.GetItem( &pItem );

		if ( m_wndList.GetBkColor() == Colors.m_crWindow )
			pDraw->clrTextBk = Colors.m_crWindow;

		switch ( Settings.General.LanguageRTL ? 2 - pItem.iImage : pItem.iImage )
		{
		case CSecureRule::srAccept:
			pDraw->clrText = Colors.m_crSecurityAllow;
			break;
		case CSecureRule::srDeny:
			pDraw->clrText = Colors.m_crSecurityDeny;
			break;
		}

		*pResult = CDRF_DODEFAULT;
	}
}

void CSecurityWnd::OnDblClkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnSecurityEdit();
	*pResult = 0;
}

void CSecurityWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CSecurityWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	Skin.TrackPopupMenu( L"CSecurityWnd", point, ID_SECURITY_EDIT );
}

void CSecurityWnd::OnUpdateSecurityEdit(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 && m_wndList.GetItemCount() > 1 );
}

void CSecurityWnd::OnSecurityEdit()
{
	if ( m_wndList.GetSelectedCount() != 1 || m_wndList.GetItemCount() < 2 )
		return;

	CSecureRule* pEditableRule;
	{
		CQuickLock oLock( Security.m_pSection );

		CSecureRule* pRule = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
		if ( ! pRule ) return;
		pEditableRule = new CSecureRule( *pRule );
	}

	CSecureRuleDlg dlg( NULL, pEditableRule );
	if ( dlg.DoModal() == IDOK )
	{
		Security.Save();
		Update();
	}
	else
		delete pEditableRule;
}

void CSecurityWnd::OnUpdateSecurityReset(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSecurityWnd::OnSecurityReset()
{
	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CQuickLock oLock( Security.m_pSection );

		if ( CSecureRule* pRule = GetItem( nItem ) )
			pRule->Reset();
	}

	Security.Save();
	Update();
}

void CSecurityWnd::OnUpdateSecurityRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 && m_wndList.GetItemCount() > 1 );
}

void CSecurityWnd::OnSecurityRemove()
{
	if ( m_wndList.GetSelectedCount() < 1 || m_wndList.GetItemCount() <= 1 )
		return;

	if ( MsgBox( IDS_SECURITY_REMOVE_CONFIRM, MB_ICONQUESTION|MB_YESNO ) != IDYES )
		return;

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CQuickLock oLock( Security.m_pSection );

		if ( CSecureRule* pRule = GetItem( nItem ) )
			Security.Remove( pRule );
	}

	Security.Save();
	Update();
}

void CSecurityWnd::OnUpdateSecurityMoveUp(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSecurityWnd::OnSecurityMoveUp()
{
	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CQuickLock oLock( Security.m_pSection );

		if ( CSecureRule* pRule = GetItem( nItem ) )
			Security.MoveUp( pRule );
	}

	Security.Save();
	Update( 2 );
}

void CSecurityWnd::OnUpdateSecurityMoveDown(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSecurityWnd::OnSecurityMoveDown()
{
	CQuickLock oLock( Security.m_pSection );

	CList< CSecureRule* > pList;

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		pList.AddHead( GetItem( nItem ) );
	}

	while ( pList.GetCount() )
	{
		CSecureRule* pRule = pList.RemoveHead();
		if ( pRule ) Security.MoveDown( pRule );
	}

	Security.Save();
	Update( 2 );
}

void CSecurityWnd::OnSecurityAdd()
{
	CSecureRuleDlg dlg;

	if ( dlg.DoModal() == IDOK )
	{
		Security.Save();
		Update();
	}
}

void CSecurityWnd::OnUpdateSecurityExport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSecurityWnd::OnSecurityExport()
{
	CFileDialog dlg( FALSE, L"xml", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		L"XML Security Files|*.xml|NET Security Files|*.net|" + LoadString( IDS_FILES_ALL ) + L"|*.*||" );

	if ( dlg.DoModal() != IDOK ) return;

	CFile pFile;

	if ( ! pFile.Open( dlg.GetPathName(), CFile::modeWrite|CFile::modeCreate ) )
	{
		MsgBox( L"Error" );	// ToDo: Security Export Error
		return;
	}

	CWaitCursor pCursor;

	CString strText;

	if ( dlg.GetFileExt().CompareNoCase( L"net" ) == 0 )
	{
		for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
		{
			CQuickLock oLock( Security.m_pSection );

			if ( CSecureRule* pRule = GetItem( nItem ) )
			{
				if ( pRule->m_nType != CSecureRule::srAddress )
					continue;	// IP only for .net files?

				strText = pRule->ToGnucleusString();

				if ( ! strText.IsEmpty() )
				{
					strText += L"\r\n";

					const int nBytes = WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), NULL, 0, NULL, NULL );
					LPSTR pBytes = new CHAR[nBytes];
					WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), pBytes, nBytes, NULL, NULL );
					pFile.Write( pBytes, nBytes );
					delete [] pBytes;
				}
			}
		}
	}
	else	// Generate .XML
	{
		augment::auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, L"security" ) );

		pXML->AddAttribute( L"xmlns", CSecurity::xmlns );

		for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
		{
			CQuickLock oLock( Security.m_pSection );

			if ( CSecureRule* pRule = GetItem( nItem ) )
				pXML->AddElement( pRule->ToXML() );
		}

		strText = pXML->ToString( TRUE, TRUE );

		const int nBytes = WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), NULL, 0, NULL, NULL );
		auto_array< CHAR > pBytes( new CHAR[ nBytes ] );
		WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), pBytes.get(), nBytes, NULL, NULL );
		pFile.Write( pBytes.get(), nBytes );
	}

	pFile.Close();
}

void CSecurityWnd::OnSecurityImport()
{
	CFileDialog dlg( TRUE, L"xml", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		L"Security Rules|*.xml;*.net|XML Files|*.xml|NET Files|*.net|" + LoadString( IDS_FILES_ALL ) + L"|*.*||" );

	if ( dlg.DoModal() != IDOK ) return;

	CWaitCursor pCursor;

	if ( Security.Import( dlg.GetPathName() ) )
		Security.Save();
	else
		MsgBox( L"Import Failed." );	// ToDo: Error message, unable to import rules
}

void CSecurityWnd::OnUpdateSecurityPolicyAccept(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Security.m_bDenyPolicy == FALSE );
}

void CSecurityWnd::OnSecurityPolicyAccept()
{
	Security.m_bDenyPolicy = FALSE;
	Update();
	m_wndList.RedrawItems( 0, m_wndList.GetItemCount() - 1 );
}

void CSecurityWnd::OnUpdateSecurityPolicyDeny(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Security.m_bDenyPolicy == TRUE );
}

void CSecurityWnd::OnSecurityPolicyDeny()
{
	Security.m_bDenyPolicy = TRUE;
	Update();
	m_wndList.RedrawItems( 0, m_wndList.GetItemCount() - 1 );
}

BOOL CSecurityWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			if ( pMsg->wParam == VK_UP )
			{
				PostMessage( WM_COMMAND, ID_SECURITY_MOVE_UP ); 	// OnSecurityMoveUp()
				return TRUE;
			}
			if ( pMsg->wParam == VK_DOWN )
			{
				PostMessage( WM_COMMAND, ID_SECURITY_MOVE_DOWN );	// OnSecurityMoveDown()
				return TRUE;
			}
			if ( pMsg->wParam == 'A' )
			{
				for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
				{
					if ( CSecureRule* pRule = (CSecureRule*)m_wndList.GetItemData( nItem ) )	// Skip Default Policy
						m_wndList.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
				}
				return TRUE;
			}
		}
		else if ( pMsg->wParam == VK_DELETE )
		{
			PostMessage( WM_COMMAND, ID_SECURITY_REMOVE );			// OnSecurityRemove()
			return TRUE;
		}
		else if ( pMsg->wParam == VK_INSERT )
		{
			PostMessage( WM_COMMAND, ID_SECURITY_ADD );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_RETURN )
		{
			PostMessage( WM_COMMAND, ID_SECURITY_EDIT );
			return TRUE;
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}
