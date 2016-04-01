//
// DlgURLExport.cpp
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
#include "DlgURLExport.h"
#include "DlgURLCopy.h"
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "Transfer.h"
#include "Network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

static const LPCTSTR pszPresets[] =
{
	L"[Magnet]",
	L"[LinkED2K]",
	L"<a href=\"[MagnetURI]\">[Name]</a><br>",
	L"<a href=\"[LinkED2K]\">[Name]</a><br>"
};

static const LPCTSTR pszTokens[] =
{
	L"[TIGER]",		// 0
	L"[SHA1]",		// 1
	L"[MD5]",		// 2
	L"[ED2K]",		// 3
	L"[BTH]",		// 4
	L"[Name]",		// 5
	L"[NameURI]",	// 6
	L"[FileBase]",	// 7
	L"[FileExt]",	// 8
	L"[Size]",		// 9
	L"[ByteSize]",	// 10
	L"[Path]",		// 11
	L"[LocalHost]",	// 12
	L"[LocalPort]",	// 13
	L"[Link]",		// 14
	L"[LinkURI]",	// 15
	L"[Magnet]",	// 16
	L"[MagnetURI]",	// 17
	L"[LinkED2K]"	// 18
};

IMPLEMENT_DYNAMIC(CURLExportDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CURLExportDlg, CSkinDialog)
	ON_CBN_CLOSEUP(IDC_URL_TOKEN, OnCloseUpUrlToken)
	ON_CBN_SELCHANGE(IDC_URL_PRESET, OnSelChangeUrlPreset)
	ON_CBN_KILLFOCUS(IDC_URL_PRESET, OnKillFocusUrlPreset)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CURLExportDlg dialog

CURLExportDlg::CURLExportDlg(CWnd* pParent) : CSkinDialog(CURLExportDlg::IDD, pParent)
{
}

void CURLExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_COPY, m_wndCopy);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_URL_TOKEN, m_wndToken);
	DDX_Control(pDX, IDC_URL_PRESET, m_wndPreset);
	DDX_Control(pDX, IDC_URL_FORMAT, m_wndFormat);
	DDX_Control(pDX, IDC_MESSAGE, m_wndMessage);
	DDX_Text(pDX, IDC_URL_FORMAT, m_sFormat);
}

/////////////////////////////////////////////////////////////////////////////
// CURLExportDlg message handlers

BOOL CURLExportDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CURLExportDlg", IDI_WEB_URL );

	if ( Settings.General.LanguageRTL ) m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	CString strFormat, strMessage;
	m_wndMessage.GetWindowText( strFormat );
	strMessage.Format( strFormat, m_pFiles.GetCount() );
	m_wndMessage.SetWindowText( strMessage );

	m_sFormat = Settings.Library.URLExportFormat;

	if ( m_sFormat.GetLength() < 6 )
		m_sFormat = L"<a href=\"magnet:?xt=urn:bitprint:[SHA1].[TIGER]&amp;xt=urn:ed2khash:[ED2K]&amp;xt=urn:md5:[MD5]&amp;xl=[ByteSize]&amp;dn=[NameURI]\">[Name]</a><br>";

	UpdateData( FALSE );

	return TRUE;
}

void CURLExportDlg::Add(const CEnvyFile* pFile)
{
	ASSERT( pFile != NULL );
	m_pFiles.AddTail( *pFile );
}

void CURLExportDlg::OnKillFocusUrlPreset()
{
	m_wndPreset.SetCurSel( -1 );
}

void CURLExportDlg::OnCloseUpUrlToken()
{
	int nToken = m_wndToken.GetCurSel();
	m_wndToken.SetCurSel( -1 );
	if ( nToken < 0 || nToken >= _countof( pszTokens ) ) return;

	m_wndFormat.ReplaceSel( pszTokens[ nToken ] );
	m_wndFormat.SetFocus();
}

void CURLExportDlg::OnSelChangeUrlPreset()
{
	int nPreset = m_wndPreset.GetCurSel();
	if ( nPreset < 0 || nPreset >= _countof( pszPresets ) ) return;

	m_wndFormat.SetWindowText( pszPresets[ nPreset ] );
}

void CURLExportDlg::OnSave()
{
	UpdateData();

	if ( m_sFormat.IsEmpty() ) return;

//	theApp.WriteProfileString( L"Library", L"URLExportFormat", m_sFormat );
	Settings.Library.URLExportFormat = m_sFormat;

	LPCTSTR pszExt = ( m_sFormat.Find( L'<' ) >= 0 ) ? L"htm" : L"txt";
	LPCTSTR pszFilter = ( m_sFormat.Find( L'<' ) >= 0 ) ?
		L"HTML Files|*.htm;*.html|Text Files|*.txt|" + LoadString( IDS_FILES_ALL ) + L"|*.*||" :
		L"Text Files|*.txt|HTML Files|*.htm;*.html|" + LoadString( IDS_FILES_ALL ) + L"|*.*||";

	CFileDialog dlg( FALSE, pszExt, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		pszFilter, this );

	if ( dlg.DoModal() != IDOK ) return;

	CFile pOutput;

	if ( ! pOutput.Open( dlg.GetPathName(), CFile::modeWrite|CFile::modeCreate ) )
		return;

	CWaitCursor pCursor;

	m_wndProgress.SetRange( 0, short( m_pFiles.GetCount() ) );
	m_wndCopy.EnableWindow( FALSE );
	m_wndSave.EnableWindow( FALSE );

	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
	{
		m_wndProgress.OffsetPos( 1 );

		CString strLine = m_sFormat;

		MakeURL( m_pFiles.GetNext( pos ), strLine );

		int nBytes = WideCharToMultiByte( CP_ACP, 0, strLine, strLine.GetLength(), NULL, 0, NULL, NULL );
		LPSTR pBytes = new CHAR[nBytes];
		WideCharToMultiByte( CP_ACP, 0, strLine, strLine.GetLength(), pBytes, nBytes, NULL, NULL );
		pOutput.Write( pBytes, nBytes );
		delete [] pBytes;
	}

	pOutput.Close();

	EndDialog( IDOK );
}

void CURLExportDlg::OnCopy()
{
	UpdateData();

	if ( m_sFormat.IsEmpty() ) return;

	Settings.Library.URLExportFormat = m_sFormat;

	CWaitCursor pCursor;
	CString strOutput;

	m_wndProgress.SetRange( 0, short( m_pFiles.GetCount() ) );
	m_wndCopy.EnableWindow( FALSE );
	m_wndSave.EnableWindow( FALSE );

	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
	{
		m_wndProgress.OffsetPos( 1 );

		CString strLine = m_sFormat;

		MakeURL( m_pFiles.GetNext( pos ), strLine );

		strOutput += strLine;
	}

	theApp.SetClipboard( strOutput );

	EndDialog( IDOK );
}

void CURLExportDlg::MakeURL(CEnvyFile pFile, CString& strLine)
{
	CString strMagnet = CURLCopyDlg::CreateMagnet( pFile );
	strLine.Replace( L"[Magnet]", strMagnet );

	strMagnet.Replace( L"&", L"&amp;" );
	strLine.Replace( L"[MagnetURI]", strMagnet );

	CString strED2K;
	if ( pFile.m_oED2K &&
		pFile.m_nSize != 0 && pFile.m_nSize != SIZE_UNKNOWN &&
		pFile.m_sName.GetLength() )
	{
		strED2K.Format( L"ed2k://|file|%s|%I64u|%s|/",
			(LPCTSTR)URLEncode( pFile.m_sName ),
			pFile.m_nSize,
			(LPCTSTR)pFile.m_oED2K.toString() );
	}
	strLine.Replace( L"[LinkED2K]", strED2K );

	strLine.Replace( L"[Name]", pFile.m_sName );
	strLine.Replace( L"[NameURI]", URLEncode( pFile.m_sName ) );

	strLine.Replace( L"[Link]", pFile.m_sURL );
	strLine.Replace( L"[LinkURI]", URLEncode( pFile.m_sURL ) );

	strLine.Replace( L"[Path]", pFile.m_sPath );

	if ( pFile.m_nSize != 0 && pFile.m_nSize != SIZE_UNKNOWN )
	{
		CString strItem;
		strItem.Format( L"%I64u", pFile.m_nSize );
		strLine.Replace( L"[ByteSize]", strItem );
		strLine.Replace( L"[Size]", Settings.SmartVolume( pFile.m_nSize ) );
	}
	else
	{
		strLine.Replace( L"[Size]", L"" );
		strLine.Replace( L"[ByteSize]", L"" );
	}

	strLine.Replace( L"[TIGER]", pFile.m_oTiger.toString() );
	strLine.Replace( L"[SHA1]",	pFile.m_oSHA1.toString() );
	strLine.Replace( L"[MD5]",	pFile.m_oMD5.toString() );
	strLine.Replace( L"[ED2K]",	pFile.m_oED2K.toString() );
	strLine.Replace( L"[BTH]",	pFile.m_oBTH.toString() );

	const int nDot = pFile.m_sName.ReverseFind( L'.' );
	if ( nDot > 0 )
	{
		strLine.Replace( L"[FileBase]", pFile.m_sName.Left( nDot ) );
		strLine.Replace( L"[FileExt]", pFile.m_sName.Mid( nDot + 1 ) );
	}
	else
	{
		strLine.Replace( L"[FileBase]", pFile.m_sName );
		strLine.Replace( L"[FileExt]", L"" );
	}

	if ( Network.IsListening() )
	{
		CString strItem = CString( inet_ntoa( Network.m_pHost.sin_addr ) );
		strLine.Replace( L"[LocalHost]", strItem );
		strItem.Format( L"%lu", htons( Network.m_pHost.sin_port ) );
		strLine.Replace( L"[LocalPort]", strItem );
	}

	strLine += L"\r\n";
}
