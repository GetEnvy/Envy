//
// DlgDownloadGroup.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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
#include "DlgDownloadGroup.h"
#include "DownloadGroup.h"
#include "DownloadGroups.h"
#include "DlgHelp.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"

#include "Schema.h"
#include "SchemaCache.h"
#include "ShellIcons.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CDownloadGroupDlg, CSkinDialog)
	ON_BN_CLICKED(IDC_DOWNLOADS_BROWSE, &CDownloadGroupDlg::OnBrowse)
	ON_BN_CLICKED(IDC_FILTER_ADD, &CDownloadGroupDlg::OnFilterAdd)
	ON_BN_CLICKED(IDC_FILTER_REMOVE, &CDownloadGroupDlg::OnFilterRemove)
	ON_CBN_EDITCHANGE(IDC_FILTER_LIST, &CDownloadGroupDlg::OnEditChangeFilterList)
	ON_CBN_SELCHANGE(IDC_FILTER_LIST, &CDownloadGroupDlg::OnSelChangeFilterList)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, &CDownloadGroupDlg::OnCbnCloseupSchemas)
	ON_BN_CLICKED(IDC_DOWNLOADS_DEFAULT, &CDownloadGroupDlg::OnBnClickedDownloadDefault)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadGroupDlg dialog

CDownloadGroupDlg::CDownloadGroupDlg(CDownloadGroup* pGroup, CWnd* pParent)
	: CSkinDialog	( CDownloadGroupDlg::IDD, pParent )
	, m_pGroup		( pGroup )
//	, m_bTorrent	( FALSE )	// Obsolete
{
}

void CDownloadGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DOWNLOADS_BROWSE, m_wndBrowse);
	DDX_Control(pDX, IDC_DOWNLOADS_DEFAULT, m_wndCancel);
	DDX_Control(pDX, IDC_FOLDER, m_wndFolder);
	DDX_Control(pDX, IDC_FILTER_ADD, m_wndFilterAdd);
	DDX_Control(pDX, IDC_FILTER_REMOVE, m_wndFilterRemove);
	DDX_Control(pDX, IDC_FILTER_LIST, m_wndFilterList);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
	DDX_Text(pDX, IDC_NAME, m_sName);
	DDX_Text(pDX, IDC_FOLDER, m_sFolder);
//	DDX_Check(pDX, IDC_DOWNLOADS_TORRENT, m_bTorrent);	// Obsolete
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadGroupDlg message handlers

BOOL CDownloadGroupDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CDownloadGroupDlg", IDR_MAINFRAME );

	//m_wndSchemas.SetDroppedWidth( 200 );	// RecalcDropWidth()
	LoadString( m_wndSchemas.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );
	m_wndSchemas.Load( m_pGroup->m_sSchemaURI );
	m_sOldSchemaURI = m_pGroup->m_sSchemaURI;

	m_wndBrowse.SetCoolIcon( IDI_BROWSE, Settings.General.LanguageRTL );
	m_wndCancel.SetCoolIcon( IDI_FAKE, Settings.General.LanguageRTL );

	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );

	if ( ! DownloadGroups.Check( m_pGroup ) )
	{
		EndDialog( IDCANCEL );
		return TRUE;
	}

	for ( POSITION pos = m_pGroup->m_pFilters.GetHeadPosition(); pos; )
	{
		m_wndFilterList.AddString( m_pGroup->m_pFilters.GetNext( pos ) );
	}

	BOOL bSuper = DownloadGroups.GetSuperGroup() == m_pGroup;

	m_sName		= m_pGroup->m_sName;
	m_sFolder	= bSuper ? Settings.Downloads.CompletePath : m_pGroup->m_sFolder;
	//m_bTorrent = m_pGroup->m_bTorrent;	// Obsolete

	UpdateData( FALSE );

	if ( bSuper )
	{
		GetDlgItem( IDC_NAME )->EnableWindow( FALSE );
		m_wndFolder.EnableWindow( FALSE );
		m_wndSchemas.EnableWindow( FALSE );
		m_wndFilterList.EnableWindow( FALSE );
	}
	m_wndFilterAdd.EnableWindow( m_wndFilterList.GetWindowTextLength() > 0 );
	m_wndFilterRemove.EnableWindow( m_wndFilterList.GetCurSel() >= 0 );

	return TRUE;
}

void CDownloadGroupDlg::OnEditChangeFilterList()
{
	m_wndFilterAdd.EnableWindow( m_wndFilterList.GetWindowTextLength() > 0 );
}

void CDownloadGroupDlg::OnSelChangeFilterList()
{
	m_wndFilterRemove.EnableWindow( m_wndFilterList.GetCurSel() >= 0 );
}

void CDownloadGroupDlg::OnFilterAdd()
{
	CString strType;
	m_wndFilterList.GetWindowText( strType );

	strType.Trim();
	if ( strType.IsEmpty() ) return;

	if ( m_wndFilterList.FindString( -1, strType ) >= 0 ) return;

	m_wndFilterList.AddString( strType );
	m_wndFilterList.SetWindowText( L"" );
}

void CDownloadGroupDlg::OnFilterRemove()
{
	int nItem = m_wndFilterList.GetCurSel();

	int nNewSelected = CB_ERR;
	if ( nItem > CB_ERR )
	{
		nNewSelected = m_wndFilterList.DeleteString( nItem );
		if ( nItem == 0 && nNewSelected > 0 )
			nNewSelected = 0;	// First one
		else
			nNewSelected = nItem - 1;
	}

	m_wndFilterList.SetCurSel( nNewSelected );
	m_wndFilterRemove.EnableWindow( nNewSelected != CB_ERR );
}

void CDownloadGroupDlg::OnCbnCloseupSchemas()
{
	if ( m_sOldSchemaURI == m_wndSchemas.GetSelectedURI() )
		return;		// No change

	CString strNameOld, strNameNew, strFilter;

	UpdateData();	// For current m_sName

	// Get filters
	CList< CString > oList;
	for ( int nItem = 0; nItem < m_wndFilterList.GetCount(); nItem++ )
	{
		m_wndFilterList.GetLBText( nItem, strFilter );
		if ( oList.Find( strFilter ) == NULL )
			oList.AddTail( strFilter );
	}

	// Remove old schema filters (preserve custom ones)
	if ( CSchemaPtr pOldSchema = SchemaCache.Get( m_sOldSchemaURI ) )
	{
		for ( POSITION pos = pOldSchema->GetFilterIterator(); pos; )
		{
			CString strFilter;
			BOOL bResult;
			pOldSchema->GetNextFilter( pos, strFilter, bResult );
			if ( bResult )
			{
				strFilter.Insert( 0, L'.' );
				while ( POSITION pos = oList.Find( strFilter ) )
					oList.RemoveAt( pos );
			}
		}

		strNameOld = ( ! pOldSchema->m_sHeaderTitle.IsEmpty() ?
					pOldSchema->m_sHeaderTitle : pOldSchema->m_sTitle );
	}

	// Add new schema filters
	if ( CSchemaPtr pNewSchema = SchemaCache.Get( m_wndSchemas.GetSelectedURI() ) )
	{
		for ( POSITION pos = pNewSchema->GetFilterIterator(); pos; )
		{
			CString strFilter;
			BOOL bResult;
			pNewSchema->GetNextFilter( pos, strFilter, bResult );
			if ( bResult )
			{
				strFilter.Insert( 0, L'.' );
				oList.AddTail( strFilter );
			}
		}

		strNameNew = ( ! pNewSchema->m_sHeaderTitle.IsEmpty() ?
			pNewSchema->m_sHeaderTitle : pNewSchema->m_sTitle );
	}

	// Refill interface filters list
	m_wndFilterList.ResetContent();
	for ( POSITION pos = oList.GetHeadPosition(); pos; )
	{
		m_wndFilterList.AddString( oList.GetNext( pos ) );
	}

	// Set default group name if appropriate
	LoadString( strFilter, IDS_DOWNLOAD_NEW_GROUP );
	if ( m_sName == strNameOld || m_sName == strFilter || m_sName.IsEmpty() )
	{
		m_sName = strNameNew;
		UpdateData( FALSE );
	}

	m_sOldSchemaURI = m_wndSchemas.GetSelectedURI();
}

void CDownloadGroupDlg::OnBrowse()
{
	UpdateData();

	CString strPath( BrowseForFolder( L"Select folder for downloads:", m_sFolder ) );
	if ( strPath.IsEmpty() )
		return;

	// Make sure download/incomplete folders aren't the same
	if ( _tcsicmp( strPath, Settings.Downloads.IncompletePath ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_FILEPATH_NOT_SAME );
		MsgBox( strMessage, MB_ICONEXCLAMATION );
		return;
	}

	// If the group folder and download folders are the same, use the default download folder
	CString strSchema = m_wndSchemas.GetSelectedURI();
	if ( strSchema != CSchema::uriBitTorrent &&
		 strSchema != CSchema::uriCollection &&
		! strPath.CompareNoCase( Settings.Downloads.CompletePath ) )
		m_sFolder.Empty();
	else
		m_sFolder = strPath;

	UpdateData( FALSE );
}

void CDownloadGroupDlg::OnBnClickedDownloadDefault()
{
	UpdateData();

	CString strSchema = m_wndSchemas.GetSelectedURI();
	if ( strSchema == CSchema::uriBitTorrent )
		m_sFolder = Settings.Downloads.TorrentPath;
	else if ( strSchema == CSchema::uriCollection )
		m_sFolder = Settings.Downloads.CollectionPath;
	else if ( m_pGroup == DownloadGroups.GetSuperGroup() )
		m_sFolder = theApp.GetDownloadsFolder();
	else
		m_sFolder.Empty();

	UpdateData( FALSE );
}

void CDownloadGroupDlg::OnOK()
{
	UpdateData();

	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );

	// Validate Path
	if ( m_sFolder.GetLength() && m_sFolder != m_pGroup->m_sFolder )
	{
		if ( m_sFolder.Find( L"\"" ) > -1 || m_sFolder.Find( L"\t" ) > -1 ||
			 m_sFolder.Find( L"<" ) > -1 || m_sFolder.Find( L">" ) > -1 ||
			 m_sFolder.Find( L"?" ) > -1 || m_sFolder.Find( L"*" ) > -1 ||
			 m_sFolder.Find( L"|" ) > -1 || m_sFolder.Find( L":" ) != 1 )
			m_sFolder.Empty();
	}

	if ( DownloadGroups.Check( m_pGroup ) )
	{
		if ( m_pGroup != DownloadGroups.GetSuperGroup() )
		{
			m_pGroup->m_sName = m_sName;
			m_pGroup->m_sFolder  = m_sFolder;
		//	m_pGroup->m_bTorrent = m_bTorrent;
		}
		else if ( m_sFolder.GetLength() && m_sFolder.CompareNoCase( Settings.Downloads.CompletePath ) != 0 )	 // New Default Path
		{
			m_pGroup->m_sFolder = m_sFolder;
			Settings.Downloads.CompletePath = m_sFolder;
		}

		m_pGroup->m_pFilters.RemoveAll();
		for ( int nItem = 0; nItem < m_wndFilterList.GetCount(); nItem++ )
		{
			CString str;
			m_wndFilterList.GetLBText( nItem, str );
			m_pGroup->AddFilter( str );
		}

		// Change schema and remove old schema filters (preserve custom ones)
		m_pGroup->SetSchema( m_wndSchemas.GetSelectedURI(), TRUE );

		if ( m_pGroup->m_sName.IsEmpty() )
			m_pGroup->m_sName = m_sName;

		// Why should we force users to have groups named after the schema?
		// Because we add new schema related types without asking?
		//if ( m_sName.GetLength() && m_pGroup->m_sName != m_sName )
		//	m_pGroup->m_sName = m_sName;	// Reset Group name to schema type
	}

	if ( ! m_sFolder.IsEmpty() )
	{
		if ( m_sFolder.Find( L":\\" ) != 1 )
			m_sFolder = Settings.Downloads.CompletePath + L"\\" + m_sFolder;

		if ( ! LibraryFolders.IsFolderShared( m_sFolder ) )
		{
			CString strFormat, strMessage;

			LoadString( strFormat, IDS_LIBRARY_DOWNLOADS_ADD );
			strMessage.Format( strFormat, (LPCTSTR)m_sFolder );

			BOOL bAdd = ( MsgBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES );
			if ( bAdd )
			{
				if ( LibraryFolders.IsSubFolderShared( m_sFolder ) )
				{
					LoadString( strFormat, IDS_LIBRARY_SUBFOLDER_IN_LIBRARY );
					strMessage.Format( strFormat, (LPCTSTR)m_sFolder );

					bAdd = ( MsgBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES );
					if ( bAdd )
					{
						CLibraryFolder* pFolder;
						while ( ( pFolder = LibraryFolders.IsSubFolderShared( m_sFolder ) ) != NULL )
						{
							LibraryFolders.RemoveFolder( pFolder );
						}
					}
				}
				if ( bAdd )
				{
					if ( ! LibraryFolders.IsShareable( m_sFolder ) )
					{
						pLock.Unlock();
						CHelpDlg::Show( L"ShareHelp.BadShare" );
						bAdd = FALSE;
					}
				}
				if ( bAdd )
				{
					if ( CLibraryFolder* pFolder = LibraryFolders.AddFolder( m_sFolder ) )
					{
						BOOL bShare = MsgBox( IDS_LIBRARY_DOWNLOADS_SHARE, MB_ICONQUESTION|MB_YESNO ) == IDYES;

						CQuickLock oLock( Library.m_pSection );
						if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
							pFolder->SetShared( bShare ? TRI_TRUE : TRI_FALSE );
						Library.Update();
					}
				}
			}
		}
	}

	CSkinDialog::OnOK();
}
