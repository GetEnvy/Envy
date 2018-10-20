//
// PagePackage.cpp
//
// This file is part of Torrent Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008-2014
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "TorrentEnvy.h"
#include "PagePackage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPackagePage, CWizardPage)

BEGIN_MESSAGE_MAP(CPackagePage, CWizardPage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_LIST, OnItemChangedFileList)
	ON_BN_CLICKED(IDC_ADD_FOLDER, OnAddFolder)
	ON_BN_CLICKED(IDC_ADD_FILE, OnAddFile)
	ON_BN_CLICKED(IDC_REMOVE_FILE, OnRemoveFile)
	ON_WM_XBUTTONDOWN()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPackagePage property page

CPackagePage::CPackagePage() : CWizardPage(CPackagePage::IDD)
	, m_hImageList	( NULL )
	, m_nTotalSize	( 0 )
	, m_sTotalSize	( L"" )
	, m_sFileCount	( L"Files in this Torrent package:" )
{
}

//CPackagePage::~CPackagePage()
//{
//}

void CPackagePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_REMOVE_FILE, m_wndRemove);
	DDX_Control(pDX, IDC_FILE_LIST, m_wndList);
	DDX_Text(pDX, IDC_FILECOUNT, m_sFileCount);
	DDX_Text(pDX, IDC_TOTAL_SIZE, m_sTotalSize);
}

/////////////////////////////////////////////////////////////////////////////
// CPackagePage message handlers

BOOL CPackagePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	m_wndList.SetExtendedStyle( LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP );
	m_wndList.InsertColumn( 0, L"Filename", LVCFMT_LEFT, rc.right - 64, -1 );
	m_wndList.InsertColumn( 1, L"Size", LVCFMT_RIGHT, 64, 0 );
	m_wndList.InsertColumn( 2, L"Bytes", LVCFMT_RIGHT, 0, 0 );

	this->DragAcceptFiles(TRUE);

	return TRUE;
}

void CPackagePage::OnReset()
{
}

BOOL CPackagePage::OnSetActive()
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	if ( ! theApp.m_sCommandLineSourceFile.IsEmpty() )
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

		if ( m_wndList.GetItemCount() > 0 )
			Next();
	}

	m_wndRemove.EnableWindow( m_wndList.GetSelectedCount() > 0 );

	UpdateData( FALSE );

	return CWizardPage::OnSetActive();
}

LRESULT CPackagePage::OnWizardBack()
{
	return IDD_WELCOME_PAGE;
}

LRESULT CPackagePage::OnWizardNext()
{
	if ( m_wndList.GetItemCount() == 0 )
	{
		AfxMessageBox( IDS_PACKAGE_NEED_FILES, MB_ICONEXCLAMATION );
		return -1;
	}

	return IDD_TRACKER_PAGE;
}

void CPackagePage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

void CPackagePage::OnItemChangedFileList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	m_wndRemove.EnableWindow( m_wndList.GetSelectedCount() > 0 );

	*pResult = 0;
}

void CPackagePage::OnDropFiles( HDROP hDropInfo )
{
	CString sFilename;

	int nFiles = DragQueryFile( hDropInfo, (UINT)-1, NULL, 0 );
	for ( int i = 0; i < nFiles; i++ )
	{
		LPWSTR pszFile = sFilename.GetBuffer( _MAX_PATH );
		DragQueryFile( hDropInfo, i, pszFile, _MAX_PATH );

		if ( PathIsDirectory( sFilename ) )
			AddFolder( (LPCTSTR)pszFile, 2 );
		else
			AddFile( (LPCTSTR)pszFile );
	}

	DragFinish( hDropInfo );
}

void CPackagePage::OnAddFolder()
{
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

void CPackagePage::OnAddFile()
{
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
			for ( strFolder += '\\'; *pszFile; )
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

void CPackagePage::OnRemoveFile()
{
	CWaitCursor wc;

	for ( int nItem = m_wndList.GetItemCount() - 1; nItem >= 0; nItem-- )
	{
		if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
		{
			CString strSize = m_wndList.GetItemText( nItem, 2 );

			m_wndList.DeleteItem( nItem );

			if ( m_wndList.GetItemCount() )
			{
				m_nTotalSize -= _wtoi( strSize );
				m_sFileCount.Format( L"%i Files in this Torrent package:", m_wndList.GetItemCount() );
				m_sTotalSize.Format( L"%s", (LPCTSTR)SmartSize( m_nTotalSize ) );
			}
			else
			{
				m_sFileCount = L"Files in this Torrent package:";
				m_sTotalSize = L"";
				m_nTotalSize = 0;
			}

			UpdateData( FALSE );
			UpdateWindow();
		}
	}
}

BOOL CPackagePage::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_DELETE )
		{
			OnRemoveFile();
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage( pMsg );
}


///////////////////////////////////////////////////////////////////////////////
//	Add Files (and Folders)

void CPackagePage::AddFile(LPCTSTR pszFile)
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
	if( m_wndList.FindItem( &lvInfo, -1 ) != -1 )
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

	HIMAGELIST hIL = (HIMAGELIST)SHGetFileInfo( pszFile, 0, &pInfo, sizeof(pInfo), SHGFI_SYSICONINDEX|SHGFI_SMALLICON );

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
	strBytes.Format( L"%llu", nSize );

	m_wndList.SetItemText( nItem, 1, SmartSize( nSize ) );
	m_wndList.SetItemText( nItem, 2, strBytes );

	m_nTotalSize += nSize;
	m_sTotalSize.Format( L"%s", (LPCTSTR)SmartSize( m_nTotalSize ) );
	m_sFileCount.Format( L"%i Files in this Torrent package:", m_wndList.GetItemCount() );

	UpdateData( FALSE );
	UpdateWindow();
}

void CPackagePage::AddFolder(LPCTSTR pszPath, int nRecursive)
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;

	strPath.Format( L"%s\\*.*", pszPath );

	hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == L'.' ||
				 pFind.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
				 continue;

			strPath.Format( L"%s\\%s", pszPath, pFind.cFileName );

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( nRecursive == 0 )
				{
					const UINT nResp = AfxMessageBox( IDS_PACKAGE_RECURSIVE, MB_ICONQUESTION|MB_YESNOCANCEL );
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
