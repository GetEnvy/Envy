//
// DlgFileProperties.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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
#include "DlgFileProperties.h"
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "ShellIcons.h"
#include "Schema.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CFilePropertiesDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CFilePropertiesDlg, CSkinDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_LBUTTONUP()
	ON_WM_GETMINMAXINFO()
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, OnCloseUpSchemas)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesDlg dialog

CFilePropertiesDlg::CFilePropertiesDlg(CWnd* pParent, DWORD nIndex)
	: CSkinDialog( 0, pParent )
	, m_nIndex		( nIndex )
	, m_bHexHash	( FALSE )
	, m_nWidth		( 0 )
{
}

void CFilePropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_HASH_LABEL, m_wndHash);
	DDX_Control(pDX, IDC_FILE_ICON, m_wndIcon);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
	DDX_Text(pDX, IDC_FILE_NAME, m_sName);
	DDX_Text(pDX, IDC_FILE_SIZE, m_sSize);
	DDX_Text(pDX, IDC_FILE_TYPE, m_sType);
	DDX_Text(pDX, IDC_FILE_PATH, m_sPath);
	DDX_Text(pDX, IDC_FILE_INDEX, m_sIndex);
	DDX_Text(pDX, IDC_FILE_SHA1, m_sSHA1);
	DDX_Text(pDX, IDC_FILE_TIGER, m_sTiger);
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesDlg message handlers

BOOL CFilePropertiesDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CFilePropertiesDlg", IDI_PROPERTIES );

	CRect rc;
	GetWindowRect( &rc );
	m_nWidth = rc.Width();

	m_wndSchema.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rc, this, IDC_METADATA );

	Update();

	if ( ! Settings.LoadWindow( L"CFilePropertiesDlg", this ) )
	{
		GetWindowRect( &rc );
		rc.bottom++;
		MoveWindow( &rc );
	}

	PostMessage( WM_TIMER, 1 );

	return TRUE;
}

void CFilePropertiesDlg::Update()
{
	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = Library.LookupFile( m_nIndex );

	if ( pFile == NULL )
	{
		PostMessage( WM_COMMAND, IDCANCEL );
		return;
	}

	m_sName = pFile->m_sName;
	m_sPath = pFile->GetFolder();
	m_sSize = Settings.SmartVolume( pFile->GetSize() );
	m_sIndex.Format( L"# %lu", pFile->m_nIndex );

	if ( pFile->m_oSHA1 )
	{
		if ( m_bHexHash )
			m_sSHA1 = L"sha1:" + pFile->m_oSHA1.toString< Hashes::base16Encoding >();
		else
			m_sSHA1 = pFile->m_oSHA1.toShortUrn();
	}
	else
	{
		LoadString( m_sSHA1, IDS_NO_URN_AVAILABLE );
	}

	m_sTiger = pFile->m_oTiger.toShortUrn();
	m_wndIcon.SetIcon( ShellIcons.ExtractIcon( ShellIcons.Get( pFile->m_sName, 32 ), 32 ) );
	m_sType = ShellIcons.GetTypeString( pFile->m_sName );

	UpdateData( FALSE );

	m_wndSchemas.m_sNoSchemaText = LoadString( IDS_SEARCH_NO_METADATA );
	m_wndSchemas.Load( pFile->m_pSchema ? (LPCTSTR)pFile->m_pSchema->GetURI() : NULL );

	OnSelChangeSchemas();

	if ( pFile->m_pMetadata )
	{
		CXMLElement* pXML = pFile->m_pMetadata->Clone();

		if ( pFile->m_oSHA1 )
			pXML->AddAttribute( L"SHA1", pFile->m_oSHA1.toString() );
		else if ( CXMLAttribute* pSHA1 = pXML->GetAttribute( L"SHA1" ) )
			pSHA1->Delete();

		m_wndSchema.UpdateData( pXML, FALSE );

		delete pXML;
	}
}

void CFilePropertiesDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CSkinDialog::OnGetMinMaxInfo( lpMMI );

	if ( m_nWidth )
	{
		lpMMI->ptMinTrackSize.x = m_nWidth;
		lpMMI->ptMinTrackSize.y = 256;
		lpMMI->ptMaxTrackSize.x = m_nWidth;
	}
}

void CFilePropertiesDlg::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 ) CSkinDialog::OnSize( nType, cx, cy );

	if ( ! IsWindow( m_wndSchema.m_hWnd ) ) return;

	CRect rc;

	m_wndSchemas.GetWindowRect( &rc );
	ScreenToClient( &rc );

	m_wndSchema.SetWindowPos( NULL, rc.left, rc.bottom + 8, rc.Width(),
		cy - 24 - 16 - ( rc.bottom + 8 ), SWP_NOZORDER );

	m_wndOK.GetWindowRect( &rc );
	ScreenToClient( &rc );

	m_wndOK.SetWindowPos( NULL, rc.left, cy - 32, 0, 0, SWP_NOZORDER|SWP_NOSIZE );
	m_wndCancel.SetWindowPos( NULL, rc.right + 8, cy - 32, 0, 0, SWP_NOZORDER|SWP_NOSIZE );
}

void CFilePropertiesDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CRect rc;
	GetClientRect( &rc );
	OnSize( 1982, rc.Width(), rc.Height() );
}

void CFilePropertiesDlg::OnSelChangeSchemas()
{
	CSchemaPtr pSchema = m_wndSchemas.GetSelected();
	m_wndSchema.SetSchema( pSchema );
}

void CFilePropertiesDlg::OnCloseUpSchemas()
{
	if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
		PostMessage( WM_KEYDOWN, VK_TAB );
}

void CFilePropertiesDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rc;

	m_wndHash.GetWindowRect( &rc );
	ScreenToClient( &rc );

	if ( rc.PtInRect( point ) )
	{
		m_bHexHash = ! m_bHexHash;
		Update();
	}

	CSkinDialog::OnLButtonUp(nFlags, point);
}

void CFilePropertiesDlg::OnOK()
{
	{
		CQuickLock oLock( Library.m_pSection );

		if ( CLibraryFile* pFile = Library.LookupFile( m_nIndex ) )
		{
			if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
			{
				CXMLElement* pXML		= pSchema->Instantiate( TRUE );
				CXMLElement* pSingular	= pXML->AddElement( pSchema->m_sSingular );

				m_wndSchema.UpdateData( pSingular, TRUE );

				if ( CXMLAttribute* pSHA1 = pSingular->GetAttribute( L"SHA1" ) )
					pSHA1->Delete();

				pFile->SetMetadata( pXML );
				delete pXML;
			}
			else
			{
				pFile->ClearMetadata();
			}

			Library.Update();
		}
	}

	CSkinDialog::OnOK();
}

void CFilePropertiesDlg::OnDestroy()
{
	Settings.SaveWindow( L"CFilePropertiesDlg", this );
	CSkinDialog::OnDestroy();
}
