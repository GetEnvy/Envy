//
// WndScheduler.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2010 and PeerProject 2010-2014
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
#include "Scheduler.h"
#include "WndScheduler.h"
#include "DlgScheduleTask.h"
#include "Network.h"
#include "LiveList.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Set Column Order
enum {
	COL_TASK,
	COL_DATE,
	COL_TIME,
	COL_STATUS,
	COL_ACTIVITY,
	COL_COMMENT,
	COL_LAST	// Column Count
};

const static UINT nImageIDs[] =
{
	IDR_SCHEDULERFRAME,
	IDI_NOTASK,
	ID_SCHEDULER_ACTIVATE,
	ID_SCHEDULER_DEACTIVATE,
	NULL
};

IMPLEMENT_SERIAL(CSchedulerWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSchedulerWnd, CPanelWnd)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SCHEDULE, OnCustomDrawList)
	ON_NOTIFY(NM_DBLCLK, IDC_SCHEDULE, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SCHEDULE, OnSortList)
	ON_COMMAND(ID_SCHEDULER_ADD, OnSchedulerAdd)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_ACTIVATE, OnUpdateSchedulerActivate)
	ON_COMMAND(ID_SCHEDULER_ACTIVATE, OnSchedulerActivate)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_DEACTIVATE, OnUpdateSchedulerDeactivate)
	ON_COMMAND(ID_SCHEDULER_DEACTIVATE, OnSchedulerDeactivate)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_EDIT, OnUpdateSchedulerEdit)
	ON_COMMAND(ID_SCHEDULER_EDIT, OnSchedulerEdit)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_REMOVE, OnUpdateSchedulerRemove)
	ON_COMMAND(ID_SCHEDULER_REMOVE, OnSchedulerRemove)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_REMOVE_ALL, OnUpdateSchedulerRemoveAll)
	ON_COMMAND(ID_SCHEDULER_REMOVE_ALL, OnSchedulerRemoveAll)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_EXPORT, OnUpdateSchedulerExport)
	ON_COMMAND(ID_SCHEDULER_EXPORT, OnSchedulerExport)
	ON_COMMAND(ID_SCHEDULER_IMPORT, OnSchedulerImport)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSchedulerWnd construction

CSchedulerWnd::CSchedulerWnd()
	: m_tLastUpdate ( 0 )
{
	Create( IDR_SCHEDULERFRAME );
}

CSchedulerWnd::~CSchedulerWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerWnd message handlers

int CSchedulerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT|LVS_SHOWSELALWAYS, rectDefault, this, IDC_SCHEDULE );
	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );

	m_pSizer.Attach( &m_wndList );

	m_wndList.InsertColumn( COL_TASK, L"Task", LVCFMT_LEFT, 240 );
	m_wndList.InsertColumn( COL_DATE, L"Date", LVCFMT_CENTER, 220 );
	m_wndList.InsertColumn( COL_TIME, L"Time", LVCFMT_CENTER, 90 );
	m_wndList.InsertColumn( COL_STATUS, L"Status", LVCFMT_CENTER, 90 );
	m_wndList.InsertColumn( COL_ACTIVITY, L"Activity", LVCFMT_CENTER, 90 );
	m_wndList.InsertColumn( COL_COMMENT,  L"Comment", LVCFMT_LEFT, 280 );

//	CoolInterface.LoadIconsTo( m_gdiImageList, nImageIDs );
//	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.SetFont( &theApp.m_gdiFont );

	LoadState( L"CSchedulerWnd", TRUE );

	Update();

	return 0;
}

void CSchedulerWnd::OnDestroy()
{
	Scheduler.Save();

	Settings.SaveList( L"CSchedulerWnd", &m_wndList );
	SaveState( L"CSchedulerWnd" );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerWnd operations

void CSchedulerWnd::Update(int nColumn, BOOL bSort)
{
	CQuickLock oLock( Scheduler.m_pSection );

	CLiveList pLiveList( COL_LAST, Scheduler.GetCount() + Scheduler.GetCount() / 4u );

	int nCount = 1;
	for ( POSITION pos = Scheduler.GetIterator(); pos; nCount++ )
	{
		CScheduleTask* pSchTask = Scheduler.GetNext( pos );

		// Adding tasks we got from Scheduler to temp list and getting a handle
		// to modify their properties according to scheduler item.
		CLiveItem* pItem = pLiveList.Add( pSchTask );

		if ( pSchTask->m_bActive )
			pItem->SetImage( SCHEDULE_ITEM_ACTIVE );
		else
			pItem->SetImage( SCHEDULE_ITEM_INACTIVE );

		// Action column
		switch ( pSchTask->m_nAction )
		{
		case BANDWIDTH_FULL:
			pItem->Set( COL_TASK, LoadString( IDS_SCHEDULER_BANDWIDTH_FULL ) );
			break;
		case BANDWIDTH_LIMITED:
			pItem->Set( COL_TASK, LoadString( IDS_SCHEDULER_BANDWIDTH_LIMITED ) );
			break;
		case BANDWIDTH_STOP:
			pItem->Set( COL_TASK, LoadString( IDS_SCHEDULER_BANDWIDTH_STOP ) );
			break;
		case SYSTEM_EXIT:
			pItem->Set( COL_TASK, LoadString( IDS_SCHEDULER_SYSTEM_EXIT ) );
			break;
		case SYSTEM_SHUTDOWN:
			pItem->Set( COL_TASK, LoadString( IDS_SCHEDULER_SYSTEM_SHUTDOWN ) );
			break;
		case SYSTEM_DISCONNECT:
			pItem->Set( COL_TASK, LoadString( IDS_SCHEDULER_SYSTEM_DIALUP_OFF ) );
			break;
		case SYSTEM_NOTICE:
			pItem->Set( COL_TASK, LoadString( IDS_SCHEDULER_SYSTEM_NOTICE ) );
			break;
		}

		// Date column
		if ( ! pSchTask->m_bSpecificDays )		// One time event
		{
			pItem->Set( COL_DATE, pSchTask->m_tScheduleDateTime.Format( L"%A, %B %m, %Y" ) );
		}
		else if ( pSchTask->m_nDays == 0x7F )	// All days flagged
		{
			pItem->Set( COL_DATE, LoadString( IDS_DAY_EVERYDAY ) );
		}
		else	// Specific days flagged
		{
			CString strDays;
			if ( pSchTask->m_nDays & MONDAY )
				strDays += LoadString( IDS_DAY_MONDAY ) + L" ";
			if ( pSchTask->m_nDays & TUESDAY )
				strDays += LoadString( IDS_DAY_TUESDAY ) + L" ";
			if ( pSchTask->m_nDays & WEDNESDAY )
				strDays += LoadString( IDS_DAY_WEDNESDAY ) + L" ";
			if ( pSchTask->m_nDays & THURSDAY )
				strDays += LoadString( IDS_DAY_THURSDAY ) + L" ";
			if ( pSchTask->m_nDays & FRIDAY )
				strDays += LoadString( IDS_DAY_FRIDAY ) + L" ";
			if ( pSchTask->m_nDays & SATURDAY )
				strDays += LoadString( IDS_DAY_SATURDAY ) + L" ";
			if ( pSchTask->m_nDays & SUNDAY )
				strDays += LoadString( IDS_DAY_SUNDAY );

			pItem->Set( COL_DATE, strDays );
		}

		// Time column
		pItem->Set( COL_TIME, pSchTask->m_tScheduleDateTime.Format( L"%I:%M:%S %p" ) );

		// Status column
		if ( pSchTask->m_bActive )
			pItem->Set( COL_STATUS, LoadString( pSchTask->m_bExecuted ? IDS_SCHEDULER_TASK_DONETODAY : IDS_SCHEDULER_TASK_WAITING ) );
		else
			pItem->Set( COL_STATUS, LoadString( pSchTask->m_bExecuted ? IDS_SCHEDULER_TASK_DONE : IDS_STATUS_INACTIVE ) );

		// Active column
		pItem->Set( COL_ACTIVITY, LoadString( pSchTask->m_bActive ? IDS_STATUS_ACTIVE : IDS_STATUS_INACTIVE ) );

		// Description column
		pItem->Set( COL_COMMENT, pSchTask->m_sDescription );
	}

	// In case scheduler gave nothing
	if ( nCount == 1 )
	{
		CLiveItem* pDefault = pLiveList.Add( (LPVOID)0 );
		pDefault->Set( COL_TASK, LoadString( IDS_SCHEDULER_TASK_NONE ) );
		pDefault->SetImage( SCHEDULE_NO_ITEM );
	}

	if ( nColumn >= 0 )
		SetWindowLongPtr( m_wndList.GetSafeHwnd(), GWLP_USERDATA, 0 - nColumn - 1 );

	pLiveList.Apply( &m_wndList, bSort );	// Put items in the main list

	m_tLastUpdate = GetTickCount();			// Update time after done doing work
}

CScheduleTask* CSchedulerWnd::GetItem(int nItem)
{
	if ( nItem > -1 )
	{
		CScheduleTask* pSchTask = (CScheduleTask*)m_wndList.GetItemData( nItem );
		if ( Scheduler.Check( pSchTask ) )
			return pSchTask;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerWnd message handlers

void CSchedulerWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( ! m_wndList ) return;

	CPanelWnd::OnSize( nType, cx, cy );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy - Settings.Skin.ToolbarHeight, SWP_NOZORDER );
	SizeListAndBar( &m_wndList, &m_wndToolBar );
}

void CSchedulerWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 && IsPartiallyVisible() )
	{
		const DWORD tDelay = max( ( 2 * (DWORD)Scheduler.GetCount() ), 1000ul );	// Delay based on size of list

		if ( ( GetTickCount() - m_tLastUpdate ) > tDelay )
		{
			if ( tDelay < 2000 )
				Update();				// Sort if list is under 1000
			else
				Update( -1, FALSE );	// Otherwise just refresh values
		}
	}
}

void CSchedulerWnd::OnSkinChange()
{
	OnSize( 0, 0, 0 );
	CPanelWnd::OnSkinChange();

	Settings.LoadList( L"CSchedulerWnd", &m_wndList, -3 );
	Skin.CreateToolBar( L"CSchedulerWnd", &m_wndToolBar );

// Obsolete for reference: (Predefined nImageIDs above)
//	m_gdiImageList.Create( 16, 16, ILC_MASK|ILC_COLOR32, 3, 1 );
//	m_gdiImageList.Add( CoolInterface.ExtractIcon( IDR_SCHEDULERFRAME, FALSE ) );
//	m_gdiImageList.Add( CoolInterface.ExtractIcon( IDI_NOTASK, FALSE ) );
//	m_gdiImageList.Add( CoolInterface.ExtractIcon( ID_SCHEDULER_ACTIVATE, FALSE ) );
//	m_gdiImageList.Add( CoolInterface.ExtractIcon( ID_SCHEDULER_DEACTIVATE, FALSE ) );

	CoolInterface.LoadIconsTo( m_gdiImageList, nImageIDs );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	if ( m_wndList.SetBkImage( Skin.GetWatermark( L"CSchedulerWnd" ) ) || m_wndList.SetBkImage( Skin.GetWatermark( L"System.Windows" ) ) )	// Images.m_bmSystemWindow.m_hObject
		m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );	// No LVS_EX_DOUBLEBUFFER
	else
		m_wndList.SetBkColor( Colors.m_crWindow );
}

void CSchedulerWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
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

		switch ( pItem.iImage )
		{
		case 2:
			pDraw->clrText = RGB( 0, 127, 0 );	// ToDo: Color?
			break;
		case 3:
			pDraw->clrText = RGB( 255, 0, 0 );	// ToDo: Color?
			break;
		default:
			pDraw->clrText = Colors.m_crText;
		}

		*pResult = CDRF_DODEFAULT;
	}
}

void CSchedulerWnd::OnDblClkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnSchedulerEdit();
	*pResult = 0;
}

void CSchedulerWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CSchedulerWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ( point.x == -1 && point.y == -1 )	// Keyboard fix
		ClientToScreen( &point );

	Skin.TrackPopupMenu( L"CSchedulerWnd", point, ID_SCHEDULER_EDIT );
}

void CSchedulerWnd::OnUpdateSchedulerEdit(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CSchedulerWnd::OnSchedulerEdit()
{
	CScheduleTask* pEditableItem;
	{
		CQuickLock oLock( Scheduler.m_pSection );

		CScheduleTask* pSchTask = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
		if ( ! pSchTask ) return;
		pEditableItem = new CScheduleTask( *pSchTask );
	}

	CScheduleTaskDlg dlg( NULL, pEditableItem );
	if ( dlg.DoModal() == IDOK )
	{
		Scheduler.Save();
		Update();
	}
	else
		delete pEditableItem;
}

void CSchedulerWnd::OnUpdateSchedulerRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSchedulerWnd::OnSchedulerRemove()
{
	CQuickLock oLock( Scheduler.m_pSection );

	for ( int nItem = -1; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0; )
	{
		if ( CScheduleTask* pSchTask = GetItem( nItem ) )
			Scheduler.Remove( pSchTask );
	}

	Scheduler.Save();
	Update();
}

void CSchedulerWnd::OnSchedulerAdd()
{
	CScheduleTaskDlg dlg;

	if ( dlg.DoModal() == IDOK )
	{
		Scheduler.Save();
		Update();
	}
}

void CSchedulerWnd::OnUpdateSchedulerDeactivate(CCmdUI* pCmdUI)
{
	CQuickLock oLock( Scheduler.m_pSection );

	CScheduleTask* pSchTask = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );

	if ( ! pSchTask )
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 && pSchTask->m_bActive );
}

void CSchedulerWnd::OnSchedulerDeactivate()
{
	CQuickLock oLock( Scheduler.m_pSection );

	CScheduleTask* pSchTask = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );

	if ( ! pSchTask ) return;

	pSchTask->m_bActive = false;

	// PUT HERE (MoJo)

	Update();
}

void CSchedulerWnd::OnUpdateSchedulerActivate(CCmdUI* pCmdUI)
{
	CQuickLock oLock( Scheduler.m_pSection );

	CScheduleTask* pSchTask = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );

	if ( ! pSchTask )
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 && ! pSchTask->m_bActive );
}

void CSchedulerWnd::OnSchedulerActivate()
{
	CQuickLock oLock( Scheduler.m_pSection );

	CScheduleTask* pSchTask = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( ! pSchTask ) return;

	if ( ! Scheduler.IsScheduledTimePassed( pSchTask ) || pSchTask->m_bSpecificDays )
	{
		pSchTask->m_bActive = true;
		pSchTask->m_bExecuted = false;
	}
	else
	{
		OnSchedulerEdit();
	}

	Update();
}

void CSchedulerWnd::OnUpdateSchedulerRemoveAll(CCmdUI* pCmdUI)
{
	CScheduleTask *pSchTask = (CScheduleTask *)m_wndList.GetItemData (0);
	pCmdUI->Enable( pSchTask != NULL );
}

void CSchedulerWnd::OnSchedulerRemoveAll()
{
	CString strMessage;
	LoadString( strMessage, IDS_SCHEDULER_REMOVEALL_CONFIRM );
	if ( MsgBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CQuickLock oLock( Scheduler.m_pSection );

	for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
	{
		if ( CScheduleTask* pSchTask = GetItem( nItem ) )
			Scheduler.Remove( pSchTask );
	}

	Scheduler.Save();
	Update();
}

void CSchedulerWnd::OnUpdateSchedulerExport(CCmdUI* pCmdUI)
{
//	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
	CScheduleTask *pSchTask = (CScheduleTask *)m_wndList.GetItemData (0);
	pCmdUI->Enable( pSchTask != NULL );
}

void CSchedulerWnd::OnSchedulerExport()
{
	CFileDialog dlg( FALSE, L"xml", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		L"XML Scheduler Files|*.xml|" );

	if ( dlg.DoModal() != IDOK ) return;

	CString strText;
	CFile pFile;

	if ( ! pFile.Open( dlg.GetPathName(), CFile::modeWrite|CFile::modeCreate ) )
	{
		MsgBox( L"Error: Can not export Scheduler list to file.", MB_ICONSTOP|MB_OK );	// ToDo: Translate?
		return;
	}

	CWaitCursor pCursor;

	CXMLElement* pXML = new CXMLElement( NULL, L"scheduler" );

	pXML->AddAttribute( L"xmlns", CScheduler::xmlns );

	const BOOL bSelection = m_wndList.GetNextItem( -1, LVIS_SELECTED ) >= 0;
	for ( int nItem = -1; ( nItem = m_wndList.GetNextItem( nItem, bSelection ? LVIS_SELECTED : 0 ) ) >= 0; )
	{
		CQuickLock oLock( Scheduler.m_pSection );

		if ( CScheduleTask* pTask = GetItem( nItem ) )
			pXML->AddElement( pTask->ToXML() );
	}

	strText = pXML->ToString( TRUE, TRUE );

	int nBytes = WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), NULL, 0, NULL, NULL );
	LPSTR pBytes = new CHAR[nBytes];
	WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), pBytes, nBytes, NULL, NULL );
	pFile.Write( pBytes, nBytes );
	delete [] pBytes;

	delete pXML;

	pFile.Close();
}

void CSchedulerWnd::OnSchedulerImport()
{
	CFileDialog dlg( TRUE, L"xml", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		L"XML Scheduler Files|*.xml|" + LoadString( IDS_FILES_ALL ) + L"|*.*||" );

	if ( dlg.DoModal() != IDOK ) return;

	CWaitCursor pCursor;

	if ( Scheduler.Import( dlg.GetPathName() ) )
		Scheduler.Save();
	else
		MsgBox( L"Error: Can not import Scheduler list from file.", MB_ICONSTOP|MB_OK );	// ToDo: Translate?
}

BOOL CSchedulerWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_DELETE )
		{
			OnSchedulerRemove();
			return TRUE;
		}
		if ( pMsg->wParam == VK_INSERT )
		{
			PostMessage( WM_COMMAND, ID_SCHEDULER_ADD );
			return TRUE;
		}
		if ( pMsg->wParam == 'A' && GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
				m_wndList.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
			return TRUE;
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}
