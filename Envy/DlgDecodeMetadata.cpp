//
// DlgDecodeMetadata.cpp
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
#include "Envy.h"
#include "DlgDecodeMetadata.h"

#include "Library.h"
#include "SharedFile.h"
#include "Schema.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CDecodeMetadataDlg, CSkinDialog)
	ON_BN_CLICKED(IDC_METHOD1, OnClickedMethod1)
	ON_BN_CLICKED(IDC_METHOD2, OnClickedMethod2)
	ON_CBN_CLOSEUP(IDC_CODEPAGES, OnCloseupCodepages)
	ON_CBN_SELCHANGE(IDC_CODEPAGES, OnSelchangeCodepages)
END_MESSAGE_MAP()

const UINT CDecodeMetadataDlg::codePages[] =
{
	// arabic, baltic, centr. european, chinese simplified, chinese traditional
	1256, 1257, 1250, 936, 950,
	// cyrillic, greek, hebrew, japanese, korean, thai, turkish, vietnamese
	1251, 1253, 1255, 932, 949, 874, 1254, 1258
};

/////////////////////////////////////////////////////////////////////////////
// CDecodeMetadataDlg dialog

CDecodeMetadataDlg::CDecodeMetadataDlg(CWnd* pParent) : CSkinDialog(CDecodeMetadataDlg::IDD, pParent)
	, m_nMethod( 0 )
{
}

void CDecodeMetadataDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CODEPAGES, m_wndCodepages);
	DDX_Radio(pDX, IDC_METHOD1, m_nMethod);
	DDX_Text(pDX, IDC_PREVIEW1, m_sPreview1);
	DDX_Text(pDX, IDC_PREVIEW2, m_sPreview2);
}

/////////////////////////////////////////////////////////////////////////////
// CDecodeMetadataDlg message handlers

BOOL CDecodeMetadataDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CDecodeMetadataDlg", ID_TOOLS_LANGUAGE );	// IDI_WORLD

	UINT nID = GetACP();
	for ( int i = 0; i < _countof( codePages ); ++i )
	{
		if ( codePages[ i ] == nID )
		{
			m_wndCodepages.SetCurSel( i );
			break;
		}
	}

	if ( ! m_pFiles.IsEmpty() )
	{
		CQuickLock oLock( Library.m_pSection );
		CLibraryFile* pFile = Library.LookupFile( m_pFiles.GetHead() );
		if ( ! pFile || ! pFile->m_pMetadata || ! pFile->m_pSchema ) return TRUE;

		CXMLElement* pXML = pFile->m_pMetadata;
		m_sOriginalWords = pFile->m_pSchema->GetVisibleWords( pXML );

		m_sPreview1 = m_sOriginalWords;
		GetEncodedText( m_sPreview1, 0 );
		m_sPreview2 = m_sOriginalWords;
		GetEncodedText( m_sPreview2, 1 );
	}

	UpdateData( FALSE );

	return TRUE;
}

void CDecodeMetadataDlg::AddFile(CLibraryFile* pFile)
{
	m_pFiles.AddTail( pFile->m_nIndex );
}

void CDecodeMetadataDlg::OnOK()
{
	UpdateData();

	CProgressDialog dlgProgress( LoadString( ID_LIBRARY_REBUILD_ANSI ) + L".." );

	DWORD nCompleted = 0, nTotal = m_pFiles.GetCount();

	unsigned nCodePage = m_wndCodepages.GetCurSel();
	nCodePage = ( nCodePage < _countof( codePages ) ) ? codePages[ nCodePage ] : 1252;		// English

	// Close dialog and perform decoding in background
	CSkinDialog::OnOK();

	for ( POSITION posFiles = m_pFiles.GetHeadPosition(); posFiles; )
	{
		DWORD nIndex = m_pFiles.GetNext( posFiles );

		CQuickLock oLock( Library.m_pSection );

		if ( m_pFiles.IsEmpty() )
			break;

		if ( CLibraryFile* pFile = Library.LookupFile( nIndex ) )
		{
			dlgProgress.Progress( pFile->GetPath(), nCompleted++, nTotal );

			if ( pFile->m_pMetadata && pFile->m_pSchema )
			{
				if ( CXMLElement* pXML = pFile->m_pMetadata->Clone() )
				{
					for ( POSITION posXML = pXML->GetAttributeIterator(); posXML; )
					{
						CXMLAttribute* pAttribute = pXML->GetNextAttribute( posXML );

						CString strAttribute = pAttribute->GetValue();
						GetEncodedText( strAttribute, m_nMethod );
						pAttribute->SetValue( strAttribute );
					}

					// Make a clean copy of schema with namespace included
					if ( CXMLElement* pContainer = pFile->m_pSchema->Instantiate( TRUE ) )
					{
						pContainer->AddElement( pXML );		// Append modified metadata
						pFile->SetMetadata( pContainer );	// Save metadata by creating XML file

						delete pContainer;
					}
				}
			}
		}
	}

	Library.Update();
}

void CDecodeMetadataDlg::GetEncodedText(CString& strText, int nMethod) const
{
	int nLength = strText.GetLength();
	LPCTSTR pszSource = strText.GetBuffer( nLength );
	LPTSTR pszOutput = NULL;
	int nWide = 0;

	unsigned nCodePage = m_wndCodepages.GetCurSel();
	nCodePage = ( nCodePage < _countof( codePages ) ) ? codePages[ nCodePage ] : 1252;	// English

	if ( nMethod == 0 )
	{
		auto_array< CHAR > pszDest( new CHAR[ nLength + 1 ] );

		{
			LPCTSTR source = pszSource;
			CHAR* dest = pszDest.get();
			while ( *dest++ = static_cast< CHAR >( *source ), *source++ );
		}

		nWide = MultiByteToWideChar( nCodePage, 0, pszDest.get(), nLength, NULL, 0 );
		pszOutput = strText.GetBuffer( nWide + 1 );
		MultiByteToWideChar( nCodePage, 0, pszDest.get(), nLength, pszOutput, nWide );
	}
	else
	{
		CStringA source( pszSource );

		nWide = MultiByteToWideChar( nCodePage, 0, source, nLength, NULL, 0 );
		pszOutput = strText.GetBuffer( nWide + 1 );
		MultiByteToWideChar( nCodePage, 0, source, nLength, pszOutput, nWide );
	}

	pszOutput[ nWide ] = 0;
	strText.ReleaseBuffer();
}

void CDecodeMetadataDlg::OnClickedMethod1()
{
	UpdateData( TRUE );
}

void CDecodeMetadataDlg::OnClickedMethod2()
{
	UpdateData( TRUE );
}

void CDecodeMetadataDlg::OnCloseupCodepages()
{
	m_sPreview1 = m_sOriginalWords;
	GetEncodedText( m_sPreview1, 0 );
	m_sPreview2 = m_sOriginalWords;
	GetEncodedText( m_sPreview2, 1 );

	UpdateData( FALSE );
}

void CDecodeMetadataDlg::OnSelchangeCodepages()
{
	OnCloseupCodepages();
}
