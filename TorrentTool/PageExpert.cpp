//
// PageExpert.cpp
//
// This file is part of Envy Torrent Tool (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2007
//
// Envy Torrent Tool is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Torrent Tool is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "TorrentTool.h"
#include "PageExpert.h"
#include "PagePackage.h"
#include "PageSingle.h"
#include "PageTracker.h"
#include "PageComment.h"
#include "PageOutput.h"
#include "PageFinished.h"
#include "TorrentBuilder.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CExpertPage, CWizardPage)

BEGIN_MESSAGE_MAP(CExpertPage, CWizardPage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_LIST, OnItemChangedFileList)
	ON_BN_CLICKED(IDC_BROWSE_FOLDER, OnBrowseFolder)
	ON_BN_CLICKED(IDC_ADD_FOLDER, OnAddFolder)
	ON_BN_CLICKED(IDC_ADD_FILE, OnAddFile)
	ON_BN_CLICKED(IDC_REMOVE_FILE, OnRemoveFile)
	ON_BN_CLICKED(IDC_ADD, OnAddTracker)
	ON_BN_CLICKED(IDC_REMOVE, OnRemoveTracker)
	ON_LBN_SELCHANGE(IDC_TRACKER2, OnSelectTracker)
//	ON_CBN_EDITCHANGE(IDC_TRACKER, OnUpdateTracker)
	ON_WM_XBUTTONDOWN()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CExpertPage property page

CExpertPage::CExpertPage() : CWizardPage(CExpertPage::IDD)
	, m_hImageList	( NULL )
{
}

//CExpertPage::~CExpertPage()
//{
//}

void CExpertPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_FILE_LIST, m_wndList);
	DDX_Control(pDX, IDC_FOLDER, m_wndFolders);
	DDX_Control(pDX, IDC_TRACKER2, m_wndTrackers);
	DDX_Control(pDX, IDC_REMOVE_FILE, m_wndRemoveFile);
	DDX_Control(pDX, IDC_REMOVE, m_wndRemoveTracker);
	DDX_Control(pDX, IDC_TRACKER, m_wndTracker);
	DDX_CBString(pDX, IDC_TRACKER, m_sTracker);
	DDX_CBString(pDX, IDC_FOLDER, m_sFolder);
	DDX_Text(pDX, IDC_TORRENT_NAME, m_sName);
	DDX_Text(pDX, IDC_COMMENT, m_sComment);
	DDX_Text(pDX, IDC_FILECOUNT, m_sFileCount);
	DDX_Check(pDX, IDC_PRIVATE, m_bPrivate);
}

/////////////////////////////////////////////////////////////////////////////
// CExpertPage message handlers

BOOL CExpertPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	this->DragAcceptFiles(TRUE);

	m_nTotalSize = 0;
	m_sFileCount = L"No Files.";
	m_sName = L"";
	m_sFolder = L"";
	m_sTracker = L"";
	m_sComment = L"";

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP );
	m_wndList.InsertColumn( 0, L"Filename", LVCFMT_LEFT, rc.right - 64, -1 );
	m_wndList.InsertColumn( 1, L"Size", LVCFMT_RIGHT, 64, 0 );
	m_wndList.InsertColumn( 2, L"Bytes", LVCFMT_RIGHT, 0, 0 );

	if ( theApp.m_sCommandLinePaths.GetCount() )
	{
		for ( int i = (int)theApp.m_sCommandLinePaths.GetCount() ; i ; i-- )
		{
			CString str = theApp.m_sCommandLinePaths.RemoveHead();
			if ( PathIsDirectory( str ) )
				AddFolder( (LPCTSTR)str, 2 );
			else if ( PathFileExists( str ) )
				AddFile( (LPCTSTR)str );
		}
	}
	else if ( ! theApp.m_sCommandLineSourceFile.IsEmpty() )
	{
		CStringList oDirs;
		oDirs.AddTail( theApp.m_sCommandLineSourceFile );
		theApp.m_sCommandLineSourceFile.Empty();

		while ( ! oDirs.IsEmpty() )
		{
			CString strFolder = oDirs.RemoveHead() + L"\\";
			CFileFind finder;
			BOOL bWorking = finder.FindFile( strFolder + L"*.*" );
			while ( bWorking )
			{
				bWorking = finder.FindNextFile();
				if ( ! finder.IsDots() && ! finder.IsHidden() )
				{
					CString sFilename = strFolder + finder.GetFileName();
					if ( finder.IsDirectory() )
						oDirs.AddTail( sFilename );
					else
						AddFile( sFilename );
				}
			}
		}
	}

	int nCount = theApp.GetProfileInt( L"Folders", L"Count", 0 );
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CString strName, strURL;
		strName.Format( L"%.3i.Path", nItem + 1 );
		strURL = theApp.GetProfileString( L"Folders", strName );
		if ( ! strURL.IsEmpty() )
			m_wndFolders.AddString( strURL );
	}

	GetTrackerHistory();

	m_sTracker = theApp.GetProfileString( L"Trackers", L"Last" );

	m_bPrivate = theApp.GetProfileInt( L"Comments", L"Private", FALSE );

	UpdateData( FALSE );
	return TRUE;
}

void CExpertPage::OnReset()
{
}

BOOL CExpertPage::OnSetActive()
{
	theApp.WriteProfileInt( L"", L"Expert", TRUE );
	m_wndRemoveFile.EnableWindow( m_wndList.GetSelectedCount() );
	m_wndRemoveTracker.EnableWindow( m_wndTrackers.GetSelCount() );
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
	return CWizardPage::OnSetActive();
}

LRESULT CExpertPage::OnWizardBack()
{
	theApp.WriteProfileInt( L"", L"Expert", FALSE );

	if ( m_wndTrackers.GetCount() > 0 )
		SetTrackerHistory();

	return IDD_WELCOME_PAGE;
}

LRESULT CExpertPage::OnWizardNext()
{
	UpdateData();

	if ( m_sName.IsEmpty() )
	{
		AfxMessageBox( IDS_OUTPUT_NEED_FILE, MB_ICONEXCLAMATION );
		m_wndName.SetFocus();
		return -1;
	}

	if ( m_sFolder.IsEmpty() )
	{
		AfxMessageBox( IDS_OUTPUT_NEED_FOLDER, MB_ICONEXCLAMATION );
		m_wndFolders.SetFocus();
		return -1;
	}

	if ( m_wndList.GetItemCount() == 0 )
	{
		AfxMessageBox( IDS_PACKAGE_NEED_FILES, MB_ICONEXCLAMATION );
		return -1;
	}

	BOOL bTrackerList = m_wndTrackers.GetCount() > 0;

	if ( ! bTrackerList )
	{
		OnAddTracker();
		bTrackerList = m_wndTrackers.GetCount() == 1;
		if ( ! bTrackerList &&
			 IDYES != AfxMessageBox( IDS_TRACKER_NEED_URL, MB_ICONQUESTION|MB_YESNO ) )
		{
			m_wndTracker.SetFocus();
			return -1;
		}
	}

	// Trackers:

	if ( m_sTracker.GetLength() > 15 && m_sTracker.Find( L"://" ) > 2 )
	{
		theApp.WriteProfileString( L"Trackers", L"Last", m_sTracker );
	}
	else if ( bTrackerList )
	{
		CString str;
		m_wndTrackers.GetText( 0, str );
		theApp.WriteProfileString( L"Trackers", L"Last", str );
	}

	if ( bTrackerList )
		SetTrackerHistory();

	m_wndTracker.AddString( m_sTracker );	// Populate Combo-box

	theApp.WriteProfileInt( L"Comments", L"Private", m_bPrivate );

	// Output

	if ( GetFileAttributes( m_sFolder ) == 0xFFFFFFFF )
	{
		CString strFormat, strMessage;
		strFormat.LoadString( IDS_OUTPUT_CREATE_FOLDER );
		strMessage.Format( strFormat, (LPCTSTR)m_sFolder );

		if ( IDYES != AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) )
			return -1;

		if ( ! CreateDirectory( m_sFolder, NULL ) )
		{
			strFormat.LoadString( IDS_OUTPUT_CANT_CREATE_FOLDER );
			strMessage.Format( strFormat, (LPCTSTR)m_sFolder );

			AfxMessageBox( IDS_OUTPUT_CANT_CREATE_FOLDER, MB_ICONEXCLAMATION );
			m_wndFolders.SetFocus();
			return -1;
		}
	}

	if ( m_sName.Right( 8 ).CompareNoCase( L".torrent" ) != 0 )
		m_sName += L".torrent";
		// UINT nResp = AfxMessageBox( IDS_OUTPUT_EXTENSION, MB_ICONQUESTION|MB_YESNOCANCEL );

	CString strPath = m_sFolder + '\\' + m_sName;

	if ( GetFileAttributes( strPath ) != INVALID_FILE_ATTRIBUTES )
	{
		CString strMessage;
		strMessage.LoadString( IDS_OUTPUT_REPLACE_FILE );
		strMessage.Format( strMessage, (LPCTSTR)strPath );

		if ( IDYES != AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) )
			return -1;

		DeleteFile( strPath );
	}

	if ( m_wndFolders.FindStringExact( -1, m_sFolder ) < 0 )
	{
		m_wndFolders.AddString( m_sFolder );

		CString strName;
		int nCount = theApp.GetProfileInt( L"Folders", L"Count", 0 );
		strName.Format( L"%.3i.Path", ++nCount );
		theApp.WriteProfileInt( L"Folders", L"Count", nCount );
		theApp.WriteProfileString( L"Folders", strName, m_sFolder );
	}

	GET_PAGE( COutputPage, pOutput );
	pOutput->m_sName = m_sName;
	pOutput->m_sFolder = m_sFolder;
	GET_PAGE( CTrackerPage, pTracker );
	pTracker->m_sTracker = m_sTracker;
	GET_PAGE( CCommentPage, pComment );
	pComment->m_sComment = m_sComment;

	return IDD_FINISHED_PAGE;
}

void CExpertPage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

BOOL CExpertPage::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_DELETE && GetFocus() == &m_wndList )
		{
			OnRemoveFile();
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage( pMsg );
}

void CExpertPage::OnBrowseFolder()
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;

	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= L"Select output folder:";
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	pPath = SHBrowseForFolder( &pBI );
	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	pMalloc->Release();

	UpdateData( TRUE );
	m_sFolder = szPath;
	UpdateData( FALSE );
}

void CExpertPage::OnItemChangedFileList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;	// For reference

	m_wndRemoveFile.EnableWindow( m_wndList.GetSelectedCount() > 0 );

	*pResult = 0;
}

void CExpertPage::OnDropFiles(HDROP hDropInfo)
{
	UpdateData();

	CString strFilename;

	const int nFiles = DragQueryFile( hDropInfo, (UINT)-1, NULL, 0 );
	for ( int i = 0 ; i < nFiles ; i++ )
	{
		LPWSTR pszFile = strFilename.GetBuffer( _MAX_PATH );
		DragQueryFile( hDropInfo, i, pszFile, _MAX_PATH );

		if ( PathIsDirectory( strFilename ) )
			AddFolder( (LPCTSTR)pszFile, 2 );
		else
			AddFile( (LPCTSTR)pszFile );
	}

	DragFinish( hDropInfo );
}

void CExpertPage::OnAddFolder()
{
	UpdateData();

	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;

	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= L"Add a folder:";
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	pPath = SHBrowseForFolder( &pBI );
	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	pMalloc->Release();

	CWaitCursor wc;
	AddFolder( szPath, 0 );
}

void CExpertPage::OnAddFile()
{
	UpdateData();

	CFileDialog dlg( TRUE, NULL, NULL,
		OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|OFN_ENABLESIZING,
		L"All Files|*.*||", this );

	const DWORD nFilesSize( 81920 );
	LPTSTR szFiles = new TCHAR [ nFilesSize ];
	ZeroMemory( szFiles, nFilesSize * sizeof( TCHAR ) );
	dlg.m_ofn.lpstrFile	= szFiles;
	dlg.m_ofn.nMaxFile	= nFilesSize;

	if ( dlg.DoModal() == IDOK )
	{
		CWaitCursor wc;
		CString strFolder	= szFiles;
		LPCTSTR pszFile		= szFiles + strFolder.GetLength() + 1;

		if ( *pszFile )
		{
			for ( strFolder += '\\' ; *pszFile ; )
			{
				AddFile( strFolder + pszFile );
				pszFile += _tcslen( pszFile ) + 1;
			}
		}
		else
		{
			AddFile( strFolder );
		}
	}

	delete [] szFiles;
}

void CExpertPage::OnRemoveFile()
{
	CWaitCursor wc;

	UpdateData();

	for ( int nItem = m_wndList.GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
	{
		if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
		{
			CString strSize = m_wndList.GetItemText( nItem, 2 );

			m_wndList.DeleteItem( nItem );

			if ( m_wndList.GetItemCount() )
			{
				m_nTotalSize -= _wtoi( strSize );
				m_sFileCount.Format( L"%i Files, %s", m_wndList.GetItemCount(), SmartSize( m_nTotalSize ) );
			}
			else
			{
				m_sFileCount = L"No Files.";
				m_nTotalSize = 0;
			}

			UpdateData( FALSE );
			UpdateWindow();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Add Files (and Folders)

void CExpertPage::AddFile(LPCTSTR pszFile)
{
	LPCTSTR szFilepath = ( _tcsclen( pszFile ) < MAX_PATH ) ?
		pszFile : (LPCTSTR)( CString( L"\\\\?\\" ) + pszFile );

	HANDLE hFile = CreateFile( szFilepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		CString strMessage;
		strMessage.LoadString( IDS_PACKAGE_CANT_OPEN );
		strMessage.Format( strMessage, pszFile );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return;
	}

	// Duplicate Check
	LVFINDINFO lvInfo;
	lvInfo.flags = LVFI_STRING;
	lvInfo.psz = pszFile;
	if ( m_wndList.FindItem( &lvInfo, -1 ) != -1 )
	{
		//CString strMessage;
		//strMessage.Format( L"Duplicate filename denied:  %s", pszFile );
		//AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		CloseHandle( hFile );
		return;
	}

	DWORD nLow, nHigh;
	nLow = GetFileSize( hFile, &nHigh );
	QWORD nSize = ( (QWORD)nHigh << 32 ) + (QWORD)nLow;
	CloseHandle( hFile );

	SHFILEINFO pInfo = {};
	ZeroMemory( &pInfo, sizeof(pInfo) );

	HIMAGELIST hIL = (HIMAGELIST)SHGetFileInfo( pszFile, 0, &pInfo, sizeof(pInfo),
		SHGFI_SYSICONINDEX|SHGFI_SMALLICON );

	if ( hIL != NULL && m_hImageList == NULL )
	{
		m_hImageList = hIL;
		CImageList pTemp;
		pTemp.Attach( hIL );
		m_wndList.SetImageList( &pTemp, LVSIL_SMALL );
		pTemp.Detach();
	}

	const int nItem = m_wndList.InsertItem( LVIF_TEXT|LVIF_IMAGE, m_wndList.GetItemCount(),
		pszFile, 0, 0, pInfo.iIcon, NULL );

	CString strBytes;
	strBytes.Format( L"%i", nSize );

	m_wndList.SetItemText( nItem, 1, SmartSize( nSize ) );
	m_wndList.SetItemText( nItem, 2, strBytes );

	m_nTotalSize += nSize;
	m_sFileCount.Format( L"%i Files,  %s", m_wndList.GetItemCount(), SmartSize( m_nTotalSize ) );

	UpdateData( FALSE );
	UpdateWindow();
}

void CExpertPage::AddFolder(LPCTSTR pszPath, int nRecursive)
{
	WIN32_FIND_DATA pFind;

	CString strPath;
	strPath.Format( L"%s\\*.*", pszPath );

	HANDLE hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == '.' ||
				 pFind.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
				continue;

			strPath.Format( L"%s\\%s", pszPath, pFind.cFileName );

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( nRecursive == 0 )
				{
					UINT nResp = AfxMessageBox( IDS_PACKAGE_RECURSIVE, MB_ICONQUESTION|MB_YESNOCANCEL );
					if ( nResp == IDYES )
						nRecursive = 2;
					else if ( nResp == IDNO )
						nRecursive = 1;
					else
						break;
				}

				if ( nRecursive == 2 )
					AddFolder( strPath, 2 );
			}
			else
			{
				AddFile( strPath );
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}
}

void CExpertPage::OnAddTracker()
{
	CString str;
	m_wndTracker.GetWindowText( str );
	str.Trim();

	if ( str.GetLength() < 16 ||
		m_wndTrackers.GetCount() > 20 ||
		m_wndTrackers.FindStringExact( -1, str ) >= 0 ||
		( str.Left( 6 ).CompareNoCase( L"udp://" ) != 0 &&
		  str.Left( 7 ).CompareNoCase( L"http://" ) != 0 &&
		  str.Left( 8 ).CompareNoCase( L"https://" ) != 0 ) )
		return;

	if ( str.Right( 9 ).CompareNoCase( L"/announce" ) != 0 &&
		 IDYES != AfxMessageBox( IDS_TRACKER_NEED_ANNOUNCE, MB_ICONQUESTION|MB_YESNO ) )
		return;

	m_wndTrackers.AddString( str );
}

void CExpertPage::OnRemoveTracker()
{
	int nItem = m_wndTrackers.GetCurSel();
	if ( nItem >= 0 ) m_wndTrackers.DeleteString( nItem );
	m_wndRemoveTracker.EnableWindow( m_wndTrackers.GetSelCount() );
}

void CExpertPage::OnSelectTracker()
{
	m_wndRemoveTracker.EnableWindow( m_wndTrackers.GetSelCount() );
}

void CExpertPage::GetTrackerHistory()
{
	int nCount = theApp.GetProfileInt( L"Trackers", L"Count", 0 );
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CString strName, strURL;
		strName.Format( L"%.3i.URL", nItem + 1 );
		strURL = theApp.GetProfileString( L"Trackers", strName );
		if ( ! strURL.IsEmpty() )
			m_wndTracker.AddString( strURL );
	}
	if ( ! m_wndTracker.GetCount() )
		 m_wndTracker.AddString( L"udp://tracker.openbittorrent.com:80/announce" );
}

void CExpertPage::SetTrackerHistory()
{
	CString str;
	CStringList	pList;

	int nCount = m_wndTrackers.GetCount();
	if ( ! nCount ) return;

	for ( int nIndex = 0 ; nIndex < nCount ; nIndex++ )
	{
		m_wndTrackers.GetText( nIndex, str );
		pList.AddTail( str );
	}

	nCount = theApp.GetProfileInt( L"Trackers", L"Count", 0 );

	for ( int nIndex = 1 ; nIndex <= nCount ; nIndex++ )
	{
		str.Format( L"%.3i.URL", nIndex );
		str = theApp.GetProfileString( L"Trackers", str );
		if ( ! pList.Find( str ) )
			pList.AddTail( str );
	}

	nCount = 1;
	theApp.WriteProfileInt( L"Trackers", L"Count", (int)pList.GetCount() );

	for ( POSITION pos = pList.GetHeadPosition() ; pos ; nCount++ )
	{
		str.Format( L"%.3i.URL", nCount );
		theApp.WriteProfileString( L"Trackers", str, pList.GetNext( pos ) );
	}
}
