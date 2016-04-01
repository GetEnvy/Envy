//
// DlgFileCopy.cpp
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
#include "DlgFileCopy.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "CtrlSharedFolder.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CFileCopyDlg, CSkinDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileCopyDlg dialog

CFileCopyDlg::CFileCopyDlg(CWnd* pParent, BOOL bMove)
	: CSkinDialog(CFileCopyDlg::IDD, pParent)
	, m_bMove		( bMove )
	, m_nFileProg	( 0 )
	, m_nCookie 	( 0 )
	, m_bCancel		( FALSE )
	, m_bCompleted	( false )
{
}

void CFileCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MESSAGE_MOVE, m_wndMove);
	DDX_Control(pDX, IDC_MESSAGE_COPY, m_wndCopy);
	DDX_Control(pDX, IDC_FILE_NAME, m_wndFileName);
	DDX_Control(pDX, IDC_PROGRESS_FILE, m_wndFileProg);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_PLACEHOLDER, m_wndPlaceholder);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDOK, m_wndOK);
}

/////////////////////////////////////////////////////////////////////////////
// CFileCopyDlg message handlers

void CFileCopyDlg::AddFile(CLibraryFile* pFile)
{
	m_pFiles.AddTail( pFile->m_nIndex );
}

BOOL CFileCopyDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CFileCopyDlg", IDR_LIBRARYFRAME );
	SelectCaption( this, m_bMove ? 1 : 0 );

	CString strCaption, strBase;

	CWnd* pMessage = m_bMove ? &m_wndMove : &m_wndCopy;
	pMessage->GetWindowText( strBase );
	strCaption.Format( strBase, m_pFiles.GetCount() );
	pMessage->SetWindowText( strCaption );
	pMessage->ShowWindow( SW_SHOW );

	if ( m_pFiles.GetCount() == 1 )
		GetDlgItem( IDC_PROGRESS )->EnableWindow( FALSE );

	CRect rc;
	m_wndPlaceholder.GetWindowRect( &rc );
	ScreenToClient( &rc );
	if ( ! m_wndTree.Create( WS_VISIBLE|WS_TABSTOP|WS_BORDER, rc, this, IDC_FOLDERS ) ) return -1;
	m_wndTree.SetMultiSelect( FALSE );

	{
		CQuickLock oLock( Library.m_pSection );

		m_nCookie = Library.GetCookie();
		m_wndTree.Update();

		if ( CLibraryFolder* pFolder = LibraryFolders.GetFolder( m_sTarget ) )
			m_wndTree.SelectFolder( pFolder );
	}

	if ( Settings.General.LanguageRTL )
	{
		m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_wndFileProg.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	}
	m_wndFileProg.SetRange( 0, 400 );

	PostMessage( WM_TIMER, 1 );
	SetTimer( 1, 500, NULL );

	return TRUE;
}

void CFileCopyDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	if ( m_bCompleted )
	{
		StopOperation();
		PostMessage( WM_COMMAND, IDCANCEL );
		return;
	}

	if ( ! m_wndTree.IsWindowEnabled() ) return;

	if ( m_nCookie != Library.GetCookie() )
	{
		CSingleLock pLock( &Library.m_pSection );
		if ( pLock.Lock( 500 ) )
		{
			m_nCookie = Library.GetCookie();
			m_wndTree.Update();
		}
	}

	m_wndOK.EnableWindow( m_wndTree.GetSelectedFolderIterator() != NULL );
}

BOOL CFileCopyDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNotify = (NMHDR*)lParam;

	if ( pNotify->code == NM_DBLCLK )
	{
		PostMessage( WM_COMMAND, IDOK );
		return TRUE;
	}

	return CSkinDialog::OnNotify(wParam, lParam, pResult);
}

void CFileCopyDlg::OnOK()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION pos = m_wndTree.GetSelectedFolderIterator();
	if ( pos == NULL ) return;

	CLibraryFolder* pFolder = m_wndTree.GetNextSelectedFolder( pos );

	m_sTarget = pFolder->m_sPath;

	pLock.Unlock();

	StartOperation();
}

void CFileCopyDlg::OnCancel()
{
	if ( IsThreadAlive() )
	{
		StopOperation();
		m_wndFileName.SetWindowText( L"Operation cancelled" );
		return;
	}

	StopOperation();

	CSkinDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg operation control

void CFileCopyDlg::StartOperation()
{
	if ( IsThreadAlive() ) return;

	m_wndTree.EnableWindow( FALSE );
	m_wndOK.EnableWindow( FALSE );

	m_wndProgress.SetRange( 0, short( m_pFiles.GetCount() ) );
	m_wndProgress.SetPos( 0 );

	m_bCancel = FALSE;
	m_bCompleted = false;
	BeginThread( "DlgFileCopy" );
}

void CFileCopyDlg::StopOperation()
{
	if ( ! IsThreadAlive() ) return;

	CWaitCursor pCursor;

	m_bCancel = TRUE;
	CloseThread();

	m_wndCancel.SetWindowText( LoadString( IDS_GENERAL_CLOSE ) );	// L"&Close"
	m_wndProgress.EnableWindow( FALSE );
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg operation thread

void CFileCopyDlg::OnRun()
{
	while ( IsThreadEnabled() )
	{
		CString strName, strPath;
		CString strComments, strShareTags;
		CSchemaPtr pSchema = NULL;
		CXMLElement* pMetadata = NULL;
		BOOL bMetadataAuto = FALSE;
		int nRating = 0;
		CLibraryFile* pFile;

		{
			CQuickLock oLock( Library.m_pSection );

			if ( m_pFiles.IsEmpty() ) break;

			DWORD nIndex = m_pFiles.RemoveHead();

			pFile = Library.LookupFile( nIndex );

			if ( pFile != NULL && pFile->IsAvailable() )
			{
				strName 	= pFile->m_sName;
				strPath 	= pFile->GetFolder();
				pSchema 	= pFile->m_pSchema;
				pMetadata	= pFile->m_pMetadata ? pFile->m_pMetadata->Clone() : NULL;
				bMetadataAuto = pFile->m_bMetadataAuto;
				nRating 	= pFile->m_nRating;
				strComments	= pFile->m_sComments;
				strShareTags = pFile->m_sShareTags;
			}
		}

		if ( pFile == NULL || ! pFile->IsAvailable() ) break;

		m_wndProgress.OffsetPos( 1 );

		m_wndFileName.SetWindowText( strName );

		if ( ProcessFile( strName, strPath ) )
		{
			CQuickLock oLock( Library.m_pSection );

			CLibraryFolder* pTargetFolder = LibraryFolders.GetFolder( m_sTarget );
			if ( pTargetFolder )
			{
				BOOL bNew;
				CLibraryFile* pTargetFile = pTargetFolder->AddFile( strName, bNew );
				if ( pSchema )
				{
					pTargetFile->m_pSchema = pSchema;
					pSchema = NULL;
				}
				pTargetFile->m_bMetadataAuto = bMetadataAuto;
				pTargetFile->m_nRating = nRating;
				pTargetFile->m_sComments = strComments;
				pTargetFile->m_sShareTags = strShareTags;
				if ( pMetadata )
				{
					if ( pTargetFile->m_pMetadata )
					{
						pMetadata->Merge( pTargetFile->m_pMetadata );
						delete pTargetFile->m_pMetadata;
					}
					pTargetFile->m_pMetadata = pMetadata;
					pMetadata = NULL;
					pTargetFile->ModifyMetadata();
				}

				if ( bNew )
					Library.AddFile( pTargetFile );
			}
		}

		delete pMetadata;

	// Alternate code to check if file is hashing first:

	//	int nRemaining;
	//	CString strCurrent;
	//	LibraryBuilder.UpdateStatus( &strCurrent, &nRemaining );
	//	CString strFile = strPath + L"\\" + strName;
	//	if ( strFile == strCurrent )
	//	{
	//		strCurrent.Format( LoadString( IDS_BITPRINTS_NOT_HASHED ), strName );
	//		theApp.Message( MSG_NOTICE, strCurrent );
	//		m_wndFileName.SetWindowText( LoadString( IDS_STATUS_FILEERROR ) );
	//	}
	//	else
	//	{
	//		m_wndFileName.SetWindowText( strName );
	//		ProcessFile( strName, strPath, bMetaData );
	//	}
	}

	m_bCompleted = true;
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg file processing

bool CFileCopyDlg::ProcessFile(const CString& strName, const CString& strPath)
{
	if ( strPath.CompareNoCase( m_sTarget ) == 0 )
		return false;

	const CString strSource = strPath + L"\\" + strName;
	const CString strTarget = m_sTarget + L"\\" + strName;

	// Check if we can move the file first
	if ( m_bMove )
	{
		HANDLE hFile = CreateFile( strSource, GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		VERIFY_FILE_ACCESS( hFile, strSource )
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_LIBRARY_MOVE_FAIL ), strName );

			switch ( MsgBox( strMessage, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) )
			{
			case IDYES:
				m_bMove = FALSE;
				break;
			case IDNO:
				break;
			default:
				return false;
			}
		}
		else
			CloseHandle( hFile );
	}

	// Copy/Move the file
	return m_bMove ?
		ProcessMove( strSource, strTarget ) :
		ProcessCopy( strSource, strTarget );
}

bool CFileCopyDlg::CheckTarget(const CString& strTarget)
{
	if ( GetFileAttributes( strTarget ) == 0xFFFFFFFF )
		return true;

	CString strMessage;
	strMessage.Format( LoadString( IDS_LIBRARY_TARGET_EXISTS ), strTarget );

	switch ( MsgBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 ) )
	{
	case IDYES:
		break;
	case IDCANCEL:
		Exit();
	case IDNO:
	default:
		return false;
	}

	if ( DeleteFileEx( strTarget, TRUE, FALSE, FALSE ) )
		return true;

	strMessage.Format( LoadString( IDS_LIBRARY_DELETE_FAIL ), strTarget );
	strMessage += L"\r\n\r\n" + GetErrorString();

	MsgBox( strMessage, MB_ICONEXCLAMATION );

	return false;
}

bool CFileCopyDlg::ProcessMove(const CString& strSource, const CString& strTarget)
{
	if ( ! CheckTarget( strTarget ) )
		return false;

	// Close the file handle
	theApp.OnRename( strSource );

	// Try moving the file
	if ( MoveFile( strSource, strTarget ) )
	{
		// Success. Tell the file to use its new name
		theApp.OnRename( strSource, strTarget );
		return true;
	}

	// Try a copy/delete. (Will usually make a duplicate of the file)
	if ( ProcessCopy( strSource, strTarget ) )
	{
		// Success. Tell the file to use its new name
		theApp.OnRename( strSource, strTarget );
		return DeleteFileEx( strSource, TRUE, FALSE, FALSE ) != 0;
	}

	// Failure. Continue using its old name
	theApp.OnRename( strSource, strSource );

	return false;
}

bool CFileCopyDlg::ProcessCopy(const CString& strSource, const CString& strTarget)
{
	if ( ! CheckTarget( strTarget ) )
		return false;

	m_wndFileProg.SetPos( 0 );
	m_nFileProg = 0;

	if ( CopyFileEx( strSource, strTarget, CopyCallback, this, &m_bCancel, COPY_FILE_FAIL_IF_EXISTS ) != 0 )
		return true;

	if ( ! IsThreadAlive() )
		DeleteFileEx( strTarget, TRUE, FALSE, FALSE );

	return false;
}

DWORD WINAPI CFileCopyDlg::CopyCallback(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER /*StreamSize*/, LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/, DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/, LPVOID lpData)
{
	CFileCopyDlg* pDlg = (CFileCopyDlg*)lpData;

	if ( ! pDlg->IsThreadAlive() ) return 1;

	if ( TotalFileSize.LowPart )
	{
		double nProgress = ( (double)TotalBytesTransferred.LowPart / (double)TotalFileSize.LowPart );
		int iProgress = (int)( nProgress * 400 );

		if ( iProgress != pDlg->m_nFileProg )
			pDlg->m_wndFileProg.SetPos( pDlg->m_nFileProg = iProgress );
	}

	return 0;
}
