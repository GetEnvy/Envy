//
// PageOutput.cpp
//
// This file is part of Torrent Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008,2012-2014 and Shareaza 2007
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "TorrentEnvy.h"
#include "PageOutput.h"
#include "PageWelcome.h"
#include "PageSingle.h"
#include "PagePackage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(COutputPage, CWizardPage)

BEGIN_MESSAGE_MAP(COutputPage, CWizardPage)
	ON_BN_CLICKED(IDC_BROWSE_FOLDER, OnBrowseFolder)
	ON_BN_CLICKED(IDC_CLEAR_FOLDERS, OnClearFolders)
	ON_BN_CLICKED(IDC_AUTO_PIECE_SIZE, OnClickedAutoPieceSize)
	ON_CBN_CLOSEUP(IDC_PIECE_SIZE, OnCloseupPieceSize)
	ON_WM_XBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COutputPage property page

COutputPage::COutputPage() : CWizardPage(COutputPage::IDD)
	, m_bAutoPieces	( TRUE )
	, m_nPieceIndex	( 0 )
	, m_bSHA1		( TRUE )
	, m_bED2K		( TRUE )
	, m_bMD5		( TRUE )
{
}

void COutputPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TORRENT_NAME, m_wndName);
	DDX_Control(pDX, IDC_FOLDER, m_wndFolders);
	DDX_CBString(pDX, IDC_FOLDER, m_sFolder);
	DDX_Text(pDX, IDC_TORRENT_NAME, m_sName);
	DDX_Check(pDX, IDC_AUTO_PIECE_SIZE, m_bAutoPieces);
	DDX_CBIndex(pDX, IDC_PIECE_SIZE, m_nPieceIndex);
	DDX_Control(pDX, IDC_PIECE_SIZE, m_wndPieceSize);
	DDX_Check(pDX, IDC_CHECK_SHA1, m_bSHA1);
	DDX_Check(pDX, IDC_CHECK_ED2K, m_bED2K);
	DDX_Check(pDX, IDC_CHECK_MD5, m_bMD5);
}

/////////////////////////////////////////////////////////////////////////////
// COutputPage message handlers

BOOL COutputPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	int nCount		= theApp.GetProfileInt( L"Folders", L"Count", 0 );
	m_bAutoPieces	= theApp.GetProfileInt( L"Folders", L"AutoPieceSize", TRUE );
	m_nPieceIndex	= theApp.GetProfileInt( L"Folders", L"PieceSize", 0 );
	m_bSHA1 		= theApp.GetProfileInt( L"Folders", L"SHA1", TRUE );
	m_bED2K 		= theApp.GetProfileInt( L"Folders", L"ED2K", TRUE );
	m_bMD5			= theApp.GetProfileInt( L"Folders", L"MD5", TRUE );

	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CString strName;
		strName.Format( L"%.3i.Path", nItem + 1 );
		CString strURL = theApp.GetProfileString( L"Folders", strName );
		if ( ! strURL.IsEmpty() )
			m_wndFolders.AddString( strURL );
	}

	OnReset();

	return TRUE;
}

void COutputPage::OnReset()
{
	m_sName.Empty();
	m_sFolder.Empty();
	m_wndPieceSize.EnableWindow( ! m_bAutoPieces );
	UpdateData( FALSE );
}

BOOL COutputPage::OnSetActive()
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	GET_PAGE( CWelcomePage, pWelcome );

	if ( pWelcome->m_nType == 0 )
	{
		GET_PAGE( CSinglePage, pSingle );

		CString strFile = pSingle->m_sFileName;

		if ( LPCTSTR pszSlash = _tcsrchr( strFile, '\\' ) )
		{
			m_sName = pszSlash + 1;
			m_sName += L".torrent";

			if ( m_sFolder.IsEmpty() )
				m_sFolder = strFile.Left( (int)( pszSlash - strFile ) );
		}
	}
	else
	{
		GET_PAGE( CPackagePage, pPackage );

		CString sName = pPackage->m_wndList.GetItemText( 0, 0 );

		// Get same part of first and last files
		int nCount = pPackage->m_wndList.GetItemCount();
		if ( nCount > 1 )
		{
			CString sName2 = pPackage->m_wndList.GetItemText( nCount - 1, 0 );
			LPCTSTR pszName1 = sName;
			LPCTSTR pszName2 = sName2;
			for ( int i = 0 ; *pszName1 && *pszName2 ; ++pszName1, ++pszName2, ++i )
			{
				if ( *pszName1 != *pszName2 )
				{
					sName = sName.Left( i + 1 );
					break;
				}
			}
		}

		// Use parent folder name as torrent name
		int nSlash = sName.ReverseFind( L'\\' );
		if ( nSlash != -1 )
		{
			sName = sName.Left( nSlash );
			nSlash = sName.ReverseFind( L'\\' );
			if ( nSlash != -1 )
				m_sName = sName.Mid( nSlash + 1 ) + L".torrent";
		}

		if ( m_sFolder.IsEmpty() )
			m_sFolder = theApp.GetProfileString( L"Folders", L"Last" );

		if ( ! m_sFolder.IsEmpty() && m_sName.IsEmpty() )
		{
			m_sName = PathFindFileName( m_sFolder );
			m_sName += L".torrent";
		}
	}

	if ( ! theApp.m_sCommandLineDestination.IsEmpty() )
	{
		m_sFolder = theApp.m_sCommandLineDestination;
		theApp.m_sCommandLineDestination.Empty();

		Next();
	}

	UpdateData( FALSE );

	return CWizardPage::OnSetActive();
}

void COutputPage::OnBrowseFolder()
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;

	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= L"Select folder:";
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

void COutputPage::OnClearFolders()
{
	theApp.WriteProfileInt( L"Folders", L"Count", 0 );
	theApp.WriteProfileInt( L"Folders", L"AutoPieceSize", m_bAutoPieces );
	theApp.WriteProfileInt( L"Folders", L"SHA1", m_bSHA1 );
	theApp.WriteProfileInt( L"Folders", L"ED2K", m_bED2K );
	theApp.WriteProfileInt( L"Folders", L"MD5", m_bMD5 );
	theApp.WriteProfileInt( L"Folders", L"PieceSize", m_nPieceIndex );
	m_sFolder.Empty();
	UpdateData( FALSE );
	m_wndFolders.ResetContent();
	m_wndFolders.SetFocus();
}

LRESULT COutputPage::OnWizardBack()
{
	return IDD_COMMENT_PAGE;
}

LRESULT COutputPage::OnWizardNext()
{
	UpdateData();

	if ( m_sFolder.IsEmpty() )
	{
		AfxMessageBox( IDS_OUTPUT_NEED_FOLDER, MB_ICONEXCLAMATION );
		m_wndFolders.SetFocus();
		return -1;
	}

	const CString strFolder = ( m_sFolder.GetLength() < MAX_PATH ) ?
		m_sFolder : ( CString( L"\\\\?\\" ) + m_sFolder );

	if ( GetFileAttributes( strFolder ) == 0xFFFFFFFF )
	{
		CString strMessage, strFormat;
		strFormat.LoadString( IDS_OUTPUT_CREATE_FOLDER );
		strMessage.Format( strFormat, (LPCTSTR)m_sFolder );

		if ( IDYES != AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) )
			return -1;

		if ( ! CreateDirectory( strFolder, NULL ) )
		{
			strFormat.LoadString( IDS_OUTPUT_CANT_CREATE_FOLDER );
			strMessage.Format( strFormat, (LPCTSTR)m_sFolder );

			AfxMessageBox( IDS_OUTPUT_CANT_CREATE_FOLDER, MB_ICONEXCLAMATION );
			m_wndFolders.SetFocus();
			return -1;
		}
	}

	if ( m_sName.IsEmpty() )
	{
		AfxMessageBox( IDS_OUTPUT_NEED_FILE, MB_ICONEXCLAMATION );
		m_wndName.SetFocus();
		return -1;
	}

	if ( _tcsicmp( PathFindExtension( m_sName ), L".torrent" ) != 0 )
	{
		UINT nResp = AfxMessageBox( IDS_OUTPUT_EXTENSION, MB_ICONQUESTION|MB_YESNOCANCEL );

		if ( nResp == IDYES )
		{
			m_sName += L".torrent";
			UpdateData( FALSE );
		}
		else if ( nResp != IDNO )
		{
			m_wndName.SetFocus();
			return -1;
		}
	}

	CString strPath = m_sFolder + L'\\' + m_sName;
	if ( strPath.GetLength() > MAX_PATH )
		strPath = CString( L"\\\\?\\" ) + strPath;

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

	theApp.WriteProfileString( L"Folders", L"Last", m_sFolder );
	theApp.WriteProfileInt( L"Folders", L"AutoPieceSize", m_bAutoPieces );
	theApp.WriteProfileInt( L"Folders", L"SHA1", m_bSHA1 );
	theApp.WriteProfileInt( L"Folders", L"ED2K", m_bED2K );
	theApp.WriteProfileInt( L"Folders", L"MD5", m_bMD5 );
	theApp.WriteProfileInt( L"Folders", L"PieceSize", m_nPieceIndex );

	return IDD_FINISHED_PAGE;
}

void COutputPage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

void COutputPage::OnClickedAutoPieceSize()
{
	UpdateData( TRUE );
	m_wndPieceSize.EnableWindow( !m_bAutoPieces );
}

void COutputPage::OnCloseupPieceSize()
{
	UpdateData( TRUE );
}
