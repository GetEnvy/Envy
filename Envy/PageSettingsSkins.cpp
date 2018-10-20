//
// PageSettingsSkins.cpp
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
#include "PageSettingsSkins.h"
#include "LiveList.h"
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
	COL_NAME,
	COL_AUTHOR,
	COL_VERSION,
	COL_PATH,
	COL_URL,
	COL_EMAIL,
	COL_INFO,
	COL_DEPEND,
	COL_LAST	// Count
};

IMPLEMENT_DYNCREATE(CSkinsSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CSkinsSettingsPage, CSettingsPage)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_NOTIFY(NM_DBLCLK, IDC_SKINS, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SKINS, OnItemChangedSkins)
	ON_BN_CLICKED(IDC_SKINS_BROWSE, OnSkinsBrowse)
	ON_BN_CLICKED(IDC_SKINS_WEB, OnSkinsWeb)
	ON_BN_CLICKED(IDC_SKINS_DELETE, OnSkinsDelete)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSkinsSettingsPage property page

CSkinsSettingsPage::CSkinsSettingsPage() : CSettingsPage( CSkinsSettingsPage::IDD )
{
}

CSkinsSettingsPage::~CSkinsSettingsPage()
{
}

void CSkinsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SKINS_DELETE, m_wndDelete);
	DDX_Control(pDX, IDC_SKIN_DESC, m_wndDesc);
	DDX_Control(pDX, IDC_SKIN_NAME, m_wndName);
	DDX_Control(pDX, IDC_SKIN_AUTHOR, m_wndAuthor);
	DDX_Control(pDX, IDC_SKINS, m_wndList);
}

/////////////////////////////////////////////////////////////////////////////
// CSkinsSettingsPage message handlers

BOOL CSkinsSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 1, 1 );
	AddIcon( IDI_SKIN, m_gdiImageList );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
	m_wndList.InsertColumn( COL_NAME,	L"Name", LVCFMT_LEFT, 240, 0 );
	m_wndList.InsertColumn( COL_AUTHOR,	L"Author", LVCFMT_LEFT, 100, 1 );
	m_wndList.InsertColumn( COL_VERSION, L"Version", LVCFMT_LEFT, 42, 2 );
	m_wndList.InsertColumn( COL_PATH,	L"Path", LVCFMT_LEFT, 0, 3 );
	m_wndList.InsertColumn( COL_URL,	L"URL", LVCFMT_LEFT, 0, 4 );
	m_wndList.InsertColumn( COL_EMAIL,	L"Email", LVCFMT_LEFT, 0, 5 );
	m_wndList.InsertColumn( COL_INFO,	L"Description", LVCFMT_LEFT, 0, 6 );
	m_wndList.InsertColumn( COL_DEPEND,	L"Dependencies", LVCFMT_LEFT, 0, 7 );
	m_wndList.SetExtendedStyle( LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );

	if ( Settings.General.LanguageRTL )
		m_wndDesc.ModifyStyleEx( WS_EX_RTLREADING|WS_EX_RIGHT|WS_EX_LEFTSCROLLBAR,
			WS_EX_LTRREADING|WS_EX_LEFT|WS_EX_RIGHTSCROLLBAR, 0 );

	m_nSelected = -1;
	m_wndName.SetWindowText( L"" );
	m_wndAuthor.SetWindowText( L"" );
	m_wndDelete.EnableWindow( FALSE );

	CWaitCursor pCursor;

	EnumerateSkins();

	return TRUE;
}

void CSkinsSettingsPage::EnumerateSkins(LPCTSTR pszPath)
{
	WIN32_FIND_DATA pFind;
	HANDLE hSearch;

	CString strPath;
	strPath.Format( L"%s\\Skins\\%s*.*",
		(LPCTSTR)Settings.General.Path, pszPath ? pszPath : L"" );

	hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == '.' ) continue;

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				strPath.Format( L"%s%s\\",
					pszPath ? pszPath : L"", pFind.cFileName );

				EnumerateSkins( strPath );
			}
			else if (	_tcsistr( pFind.cFileName, L".xml" ) != NULL &&
						_tcsnicmp( pFind.cFileName, L"en.xml", 6 ) != 0 &&
						_tcsicmp( pFind.cFileName, L"Definitions.xml" ) != 0 )
			{
				AddSkin( pszPath, pFind.cFileName );
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}
}

BOOL CSkinsSettingsPage::AddSkin(LPCTSTR pszPath, LPCTSTR pszName)
{
	CString strXML;
	CFile pFile;

	strXML = Settings.General.Path + L"\\Skins\\";
	if ( pszPath ) strXML += pszPath;
	strXML += pszName;

	if ( ! pFile.Open( strXML, CFile::modeRead ) ) return FALSE;

	DWORD nSource = (DWORD)pFile.GetLength();
	if ( nSource > 4096*1024 ) return FALSE;

	CHAR* pSource = new CHAR[ nSource ];
	pFile.Read( pSource, nSource );
	pFile.Close();

	BYTE* pByte = (BYTE*)pSource;
	DWORD nByte = nSource;

	if ( nByte >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
	{
		nByte = nByte / 2 - 1;

		if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
		{
			pByte += 2;

			for ( DWORD nSwap = 0; nSwap < nByte; nSwap ++ )
			{
				register CHAR nTemp = pByte[ ( nSwap << 1 ) + 0 ];
				pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
				pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
			}
		}
		else
		{
			pByte += 2;
		}

		CopyMemory( strXML.GetBuffer( nByte ), pByte, nByte * sizeof( TCHAR ) );
		strXML.ReleaseBuffer( nByte );
	}
	else
	{
		if ( nByte >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF )
		{
			pByte += 3;
			nByte -= 3;
		}

		strXML = UTF8Decode( (LPCSTR)pByte, nByte );
	}

	delete [] pSource;

	CXMLElement* pXML = NULL;

	int nManifest = strXML.Find( L"<manifest" );

	if ( nManifest > 0 )
	{
		CString strManifest = strXML.Mid( nManifest ).SpanExcluding( L">" ) + L'>';

		if ( CXMLElement* pManifest = CXMLElement::FromString( strManifest ) )
		{
			pXML = new CXMLElement( NULL, L"skin" );
			pXML->AddElement( pManifest );
		}
	}

	if ( pXML == NULL )
	{
		pXML = CXMLElement::FromString( strXML, TRUE );
		if ( pXML == NULL ) return FALSE;
	}

	strXML.Empty();

	CXMLElement* pManifest = pXML->GetElementByName( L"manifest" );

	// Hide Non-Skins
	if ( ! pXML->IsNamed( L"skin" ) || pManifest == NULL ||
		 ! pManifest->GetAttributeValue( L"type" ).CompareNoCase( L"language" ) )
	{
		delete pXML;
		return FALSE;
	}

	// Hide Wrong-Language Skins
	if ( pManifest->GetAttributeValue( L"language" ).GetLength() &&
		 pManifest->GetAttributeValue( L"language" ).CompareNoCase( Settings.General.Language.Left(2) ) )
	{
		delete pXML;
		return FALSE;
	}

//	CString	strType		= pManifest->GetAttributeValue( L"type", L"Unknown" );
	CString	strName		= pManifest->GetAttributeValue( L"name", pszName );
	CString strAuthor	= pManifest->GetAttributeValue( L"author", L"Unknown" );
	CString strVersion	= pManifest->GetAttributeValue( L"version", L"Unknown" );
	CString strIcon		= pManifest->GetAttributeValue( L"icon" );
	CString strURL		= pManifest->GetAttributeValue( L"link" );
	CString strEmail	= pManifest->GetAttributeValue( L"email" );
	CString strDesc		= pManifest->GetAttributeValue( L"description" );
	CString strDepends	= pManifest->GetAttributeValue( L"dependencies" );
//	CString strPlatform	= pManifest->GetAttributeValue( L"platform" );
//	CString strLicense	= pManifest->GetAttributeValue( L"license" );

	if ( Settings.General.LanguageRTL )
	{
		strName = L"\x202A" + strName;
		strAuthor = L"\x202A" + strAuthor;
	}

	delete pXML;

	if ( ! strIcon.IsEmpty() )
	{
		if ( pszPath != NULL )
			strIcon = Settings.General.Path + L"\\Skins\\" + pszPath + strIcon;
		else
			strIcon = Settings.General.Path + L"\\Skins\\" + strIcon;
	}
	else
	{
		if ( pszPath != NULL )
			strIcon = Settings.General.Path + L"\\Skins\\" + pszPath + strIcon + pszName;
		else
			strIcon = Settings.General.Path + L"\\Skins\\" + strIcon + pszName;

		strIcon = strIcon.Left( strIcon.GetLength() - 3 ) + L"ico";
	}

	if ( StartsWith( strURL, _P( L"http://" ) ) )
		; // Do nothing
	else if ( StartsWith( strURL, _P( L"www." ) ) )
		strURL = L"http://" + strURL;
	else
		strURL.Empty();

	if ( strEmail.Find( L'@' ) < 0 ) strEmail.Empty();

	CLiveItem pItem( COL_LAST, 0 );
	HICON hIcon = NULL;
	ExtractIconEx( strIcon, 0, NULL, &hIcon, 1 );
	if ( hIcon )
	{
		pItem.SetImage( m_gdiImageList.Add( hIcon ) );
		DestroyIcon( hIcon );
	}
	else
	{
		pItem.SetImage( 0 );
	}

	//strDepend.Trim( L" ,;\t" );
	if ( IsText( strDepends, _P( L"none" ) ) )
		strDepends.Empty();

	pItem.Set( COL_NAME, strName );
	pItem.Set( COL_AUTHOR, strAuthor );
	pItem.Set( COL_VERSION, strVersion );
	pItem.Set( COL_URL, strURL );
	pItem.Set( COL_EMAIL, strEmail );
	pItem.Set( COL_INFO, strDesc );
	pItem.Set( COL_DEPEND, strDepends );

	strName.Format( L"%s%s", pszPath ? pszPath : L"", pszName );
	pItem.Set( COL_PATH, strName );

	int nItem = pItem.Add( &m_wndList, -1, COL_LAST );

	if ( theApp.GetProfileInt( L"Skins", strName, FALSE ) )
		m_wndList.SetItemState( nItem, 2 << 12, LVIS_STATEIMAGEMASK );
	else
		m_wndList.SetItemState( nItem, 1 << 12, LVIS_STATEIMAGEMASK );

	return TRUE;
}

void CSkinsSettingsPage::CheckDependencies(CString sPaths)
{
	if ( sPaths.GetLength() < 5 ) return;

	CString strPath;
	CStringArray oPaths;
	INT_PTR nTotal = 1;

	if ( sPaths.Find( L',', 4 ) < 4 )
	{
		strPath = sPaths;
		strPath.Trim();
	}
	else
	{
		CStringArray oTokens;
		Split( sPaths, L',', oTokens );

		nTotal = oTokens.GetCount();
		for ( INT_PTR nToken = 0; nToken < nTotal; nToken++ )
		{
			CString strToken = oTokens.GetAt( nToken );
			strToken.Trim( L" \t\r\n;,'" );
			if ( strToken.GetLength() < 5 )
				continue;
			if ( EndsWith( strToken, _P( L".xml" ) ) )
				oPaths.Add( strToken );
		}

		nTotal = oPaths.GetCount();
		if ( ! nTotal ) return;
		if ( nTotal == 1 )
			strPath = oTokens.GetAt( 0 );
	}

	const int nCount = m_wndList.GetItemCount();
	for ( int nItem = 0; nItem < nCount; nItem++ )
	{
		CString strName = m_wndList.GetItemText( nItem, COL_PATH );
		strName = strName.Mid( strName.ReverseFind( L'\\' ) + 1 );

		if ( nTotal == 1 )
		{
			if ( strName.CompareNoCase( strPath ) == 0 )
				m_wndList.SetItemState( nItem, UINT( 2 << 12 ), LVIS_STATEIMAGEMASK );
			continue;
		}

		for ( INT_PTR nToken = 0; nToken < nTotal; nToken++ )
		{
			 if ( strName.CompareNoCase( oPaths.GetAt( nToken ) ) == 0 )
				 m_wndList.SetItemState( nItem, UINT( 2 << 12 ), LVIS_STATEIMAGEMASK );
		}
	}
}

void CSkinsSettingsPage::OnItemChangedSkins(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	m_nSelected = m_wndList.GetNextItem( -1, LVNI_SELECTED );

	if ( m_nSelected >= 0 )
	{
		m_wndName.SetWindowText( m_wndList.GetItemText( m_nSelected, COL_NAME ) );
		m_wndAuthor.SetWindowText( m_wndList.GetItemText( m_nSelected, COL_AUTHOR ) );
		m_wndDesc.SetWindowText( m_wndList.GetItemText( m_nSelected, COL_INFO ) );
		m_wndDelete.EnableWindow( TRUE );
	}
	else
	{
		m_wndName.SetWindowText( L"" );
		m_wndAuthor.SetWindowText( L"" );
		m_wndDesc.SetWindowText( L"" );
		m_wndDelete.EnableWindow( FALSE );
	}

	// Checkbox
	const BOOL bChecked = (BOOL)( ( ( pNMListView->uNewState & LVIS_STATEIMAGEMASK ) >> 12 ) - 1 );
	if ( ! bChecked || bChecked < 0 ) return;	// Non-checkbox notifications

	const BOOL bPrevState = (BOOL)( ( ( pNMListView->uOldState & LVIS_STATEIMAGEMASK ) >> 12 ) - 1 );
	if ( bPrevState < 0 ) return;				// No previous state at startup

	if ( bChecked == bPrevState ) return;		// No change

	CheckDependencies( m_wndList.GetItemText( pNMListView->iItem, COL_DEPEND ) );
}

HBRUSH CSkinsSettingsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSettingsPage::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( m_nSelected >= 0 )
	{
		if ( pWnd == &m_wndName )
		{
			if ( m_wndList.GetItemText( m_nSelected, 4 ).GetLength() )
			{
				pDC->SetTextColor( Colors.m_crTextLink );
				pDC->SelectObject( &theApp.m_gdiFontLine );
			}
		}
		else if ( pWnd == &m_wndAuthor )
		{
			if ( m_wndList.GetItemText( m_nSelected, 5 ).GetLength() )
			{
				pDC->SetTextColor( Colors.m_crTextLink );
				pDC->SelectObject( &theApp.m_gdiFontLine );
			}
		}
	}

	return hbr;
}

BOOL CSkinsSettingsPage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_nSelected >= 0 )
	{
		CRect rc;
		CPoint point;
		GetCursorPos( &point );

		m_wndName.GetWindowRect( &rc );
		if ( rc.PtInRect( point ) )
		{
			if ( m_wndList.GetItemText( m_nSelected, 4 ).GetLength() )
			{
				SetCursor( theApp.LoadCursor( IDC_HAND ) );
				return TRUE;
			}
		}

		m_wndAuthor.GetWindowRect( &rc );
		if ( rc.PtInRect( point ) )
		{
			if ( m_wndList.GetItemText( m_nSelected, 5 ).GetLength() )
			{
				SetCursor( theApp.LoadCursor( IDC_HAND ) );
				return TRUE;
			}
		}
	}

	return CSettingsPage::OnSetCursor( pWnd, nHitTest, message );
}

void CSkinsSettingsPage::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if ( m_nSelected < 0 ) return;

	CRect rc;
	ClientToScreen( &point );

	m_wndName.GetWindowRect( &rc );
	if ( rc.PtInRect( point ) )
	{
		CString strURL = m_wndList.GetItemText( m_nSelected, COL_URL );

		if ( ! strURL.IsEmpty() )
			ShellExecute( GetSafeHwnd(), L"open", strURL, NULL, NULL, SW_SHOWNORMAL );

		return;
	}

	m_wndAuthor.GetWindowRect( &rc );
	if ( rc.PtInRect( point ) )
	{
		CString strEmail = m_wndList.GetItemText( m_nSelected, COL_EMAIL );

		if ( ! strEmail.IsEmpty() )
			ShellExecute( GetSafeHwnd(), L"open", L"mailto:" + strEmail, NULL, NULL, SW_SHOWNORMAL );

		return;
	}
}

void CSkinsSettingsPage::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	// Checkmark only selected skin (from first click), clear all others
	for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
	{
		m_wndList.SetItemState( nItem, UINT( ( nItem != m_nSelected ? 1 : 2 ) << 12 ), LVIS_STATEIMAGEMASK );
	}

	CheckDependencies( m_wndList.GetItemText( m_nSelected, COL_DEPEND ) );
}

void CSkinsSettingsPage::OnSkinsBrowse()
{
	CFileDialog dlg( TRUE, L"envy", L"*.envy", OFN_FILEMUSTEXIST,
		L"Skin Packages|*.envy;*.env;*.psk;*.sks|" + LoadString( IDS_FILES_ALL ) + L"|*.*||", this );

	if ( dlg.DoModal() != IDOK ) return;

	CString strFile = dlg.GetPathName();

	ShellExecute( GetSafeHwnd(), L"open", strFile, NULL, NULL, SW_SHOWNORMAL );
}

void CSkinsSettingsPage::OnSkinsWeb()
{
	ShellExecute( GetSafeHwnd(), L"open",
		CString( WEB_SITE ) + L"Skins?Version=" + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CSkinsSettingsPage::OnOK()
{
	BOOL bChanged = FALSE;

	for ( int nItem = 0; nItem < m_wndList.GetItemCount(); nItem++ )
	{
		CString strSkin = m_wndList.GetItemText( nItem, 3 );

		BOOL bOn = m_wndList.GetItemState( nItem, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );

		if ( theApp.GetProfileInt( L"Skins", strSkin, FALSE ) != (UINT)bOn )
			bChanged = TRUE;

		theApp.WriteProfileInt( L"Skins", strSkin, bOn );
	}

	if ( bChanged )
		PostMainWndMessage( WM_SKINCHANGED );

	CSettingsPage::OnOK();
}

void CSkinsSettingsPage::OnSkinsDelete()
{
	if ( m_nSelected < 0 ) return;

	CString strName = m_wndList.GetItemText( m_nSelected, COL_NAME );
	CString strBase = m_wndList.GetItemText( m_nSelected, COL_PATH );

	CString strFormat, strPrompt;

	Skin.LoadString( strFormat, IDS_SKIN_DELETE );
	strPrompt.Format( strFormat, (LPCTSTR)strName );

	if ( MsgBox( strPrompt, MB_ICONQUESTION|MB_OKCANCEL|MB_DEFBUTTON2 ) != IDOK ) return;

	theApp.WriteProfileString( L"Skins", strBase, NULL );

	CString strPath;
	strPath.Format( L"%s\\Skins\\%s",
		(LPCTSTR)Settings.General.Path, (LPCTSTR)strBase );

	DeleteFileEx( strPath, FALSE, TRUE, TRUE );

	int nSlash = strPath.ReverseFind( L'\\' );
	strPath = strPath.Left( nSlash ) + L"\\*.xml";

	WIN32_FIND_DATA pFind;
	HANDLE hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		FindClose( hSearch );
	}
	else
	{
		strPath = strPath.Left( strPath.GetLength() - 3 ) + L"*";
		hSearch = FindFirstFile( strPath, &pFind );

		if ( hSearch != INVALID_HANDLE_VALUE )
		{
			strPath = strPath.Left( strPath.GetLength() - 3 );

			do
			{
				if ( pFind.cFileName[0] == '.' ) continue;
				DeleteFileEx( strPath + pFind.cFileName, FALSE, TRUE, TRUE );
			}
			while ( FindNextFile( hSearch, &pFind ) );

			FindClose( hSearch );
		}

		strPath = strPath.Left( strPath.GetLength() - 1 );
		RemoveDirectory( strPath );
	}

	m_wndList.DeleteItem( m_nSelected );
	m_wndName.SetWindowText( L"" );
	m_wndAuthor.SetWindowText( L"" );
	m_wndDesc.SetWindowText( L"" );
	m_wndDelete.EnableWindow( FALSE );
	m_nSelected = -1;
}
