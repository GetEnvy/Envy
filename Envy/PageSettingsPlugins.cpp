//
// PageSettingsPlugins.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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
#include "PageSettingsPlugins.h"
#include "DlgPluginExtSetup.h"
#include "Plugins.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CPluginsSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CPluginsSettingsPage, CSettingsPage)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_PLUGINS, OnItemChangingPlugins)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PLUGINS, OnItemChangedPlugins)
	ON_NOTIFY(NM_DBLCLK, IDC_PLUGINS, OnNMDblclkPlugins)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PLUGINS, OnCustomDrawPlugins)
	ON_BN_CLICKED(IDC_PLUGINS_SETUP, OnPluginsSetup)
	ON_BN_CLICKED(IDC_PLUGINS_WEB, OnPluginsWeb)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage property page

CPluginsSettingsPage::CPluginsSettingsPage() : CSettingsPage( CPluginsSettingsPage::IDD )
{
}

CPluginsSettingsPage::~CPluginsSettingsPage()
{
}

void CPluginsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLUGINS_SETUP, m_wndSetup);
	DDX_Control(pDX, IDC_SKIN_DESC, m_wndDesc);
	DDX_Control(pDX, IDC_SKIN_NAME, m_wndName);
	DDX_Control(pDX, IDC_PLUGINS, m_wndList);
}

/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage message handlers

BOOL CPluginsSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 2, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 6, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 6, 1 );
	AddIcon( IDI_FILE, m_gdiImageList );
	AddIcon( IDI_EXECUTABLE, m_gdiImageList );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
	m_wndList.InsertColumn( 0, L"Name", LVCFMT_LEFT, 382, 0 );
	m_wndList.InsertColumn( 1, L"CLSID", LVCFMT_LEFT, 0, 1 );
	m_wndList.InsertColumn( 2, L"Extensions", LVCFMT_LEFT, 0, 2 );

	m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES );

	//LVGROUP pGroup;
	//pGroup.cbSize		= sizeof( pGroup );
	//pGroup.mask		= LVGF_ALIGN|LVGF_GROUPID|LVGF_HEADER;
	//pGroup.uAlign		= LVGA_HEADER_LEFT;
	//pGroup.pszHeader	= L"General Plugins";
	//pGroup.cchHeader	= _tcslen( pGroup.pszHeader );
	//m_wndList.InsertGroup( 0, &pGroup );

	SetTimer( 1, 100, NULL );
	m_wndSetup.EnableWindow( FALSE );

	return TRUE;
}

void CPluginsSettingsPage::OnItemChangingPlugins(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW* pNMListView = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	*pResult = 0;

	if ( m_bRunning &&
		 ( pNMListView->uOldState & LVIS_STATEIMAGEMASK ) == 0 &&
		 ( pNMListView->uNewState & LVIS_STATEIMAGEMASK ) != 0 )
		*pResult = 1;
}

void CPluginsSettingsPage::OnItemChangedPlugins(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pNMListView = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	*pResult = 0;
	if ( ! m_bRunning ) return;

	// Selected item handling
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( m_wndList.GetSelectedCount() == 1 )
	{
		CString str = m_wndList.GetItemText( nItem, 2 );
		CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
		m_wndName.SetWindowText( m_wndList.GetItemText( nItem, 0 ).Trim() );
		m_wndDesc.SetWindowText( GetPluginComments( m_wndList.GetItemText( nItem, 1 ) ) );
		m_wndSetup.EnableWindow( pPlugin != NULL && pPlugin->m_pPlugin != NULL ||
			( ! str.IsEmpty() && str != L"-" ) );
	}
	else
	{
		m_wndName.SetWindowText( L"..." );
		m_wndSetup.EnableWindow( FALSE );
	}

	// Check box handling
	nItem = pNMListView->iItem;

	if ( ( ( pNMListView->uOldState >> 12 ) & LVIS_SELECTED ) == 0 &&
		 ( ( pNMListView->uNewState >> 12 ) & LVIS_SELECTED ) != 0 )
	{
		CString strExt = m_wndList.GetItemText( nItem, 2 );
		strExt.Replace( L"-", L"" );
		m_wndList.SetItemText( nItem, 2, strExt );
	}
	else if ( ( ( pNMListView->uOldState >> 12 ) & LVIS_SELECTED ) != 0 &&
			  ( ( pNMListView->uNewState >> 12 ) & LVIS_SELECTED ) == 0 )
	{
		CString strExt = m_wndList.GetItemText( nItem, 2 );
		if ( ! strExt.IsEmpty() )
			strExt.Replace( L"-", L"" );
		else
			strExt = L"-";

		for ( int nDot = 0 ; nDot != -1 ; )
		{
			nDot = strExt.Find( L'.', nDot );
			if ( nDot != -1 )
			{
				strExt.Insert( nDot, L'-' );
				nDot += 2;
			}
		}

		m_wndList.SetItemText( nItem, 2, strExt );
	}
}

void CPluginsSettingsPage::OnCustomDrawPlugins(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;
	*pResult = CDRF_DODEFAULT;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( pDraw->nmcd.lItemlParam == 0 )
			pDraw->clrText = RGB( 110, 110, 110 );	// Hidden Plugin		//Colors.m_crNetworkNull
		//else
		//	pDraw->clrText = 0;						// Interface Elements	//Colors.m_crText
	}
}

void CPluginsSettingsPage::OnPluginsSetup()
{
	CString strExt;
	if ( m_wndList.GetSelectedCount() != 1 ) return;

	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
	strExt = m_wndList.GetItemText( nItem, 2 );

	if ( pPlugin != NULL && pPlugin->m_pPlugin != NULL )
	{
		pPlugin->m_pPlugin->Configure();
	}
	else if ( ! strExt.IsEmpty() )
	{
		CPluginExtSetupDlg dlg( &m_wndList, strExt );
		m_bRunning = FALSE;
		dlg.DoModal();
		m_bRunning = TRUE;
	}
}

void CPluginsSettingsPage::OnPluginsWeb()
{
	const CString strWebSite( WEB_SITE );

	ShellExecute( GetSafeHwnd(), L"open",
		strWebSite + L"?id=addon&Version=" + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CPluginsSettingsPage::OnOK()
{
	BOOL bChanged = FALSE;

	int nCount = m_wndList.GetItemCount();
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
		CString strCLSID = m_wndList.GetItemText( nItem, 1 );

		TRISTATE bEnabled = static_cast< TRISTATE >(
			m_wndList.GetItemState( nItem, LVIS_STATEIMAGEMASK ) >> 12 );

		if ( bEnabled != TRI_UNKNOWN && IsWindowVisible() )
			theApp.WriteProfileString( L"Plugins", strCLSID, m_wndList.GetItemText( nItem, 2 ) );

		if ( pPlugin != NULL && ( bEnabled == TRI_TRUE ) != ( pPlugin->m_pPlugin != NULL ) )
		{
			bChanged = TRUE;

			if ( bEnabled == TRI_TRUE )
				pPlugin->Start();
			else
				pPlugin->Stop();
		}
	}

	if ( bChanged )
		PostMainWndMessage( WM_SKINCHANGED );

	CSettingsPage::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage plugin enumeration

void CPluginsSettingsPage::InsertPlugin(LPCTSTR pszCLSID, LPCTSTR pszName, int nImage, TRISTATE bEnabled, LPVOID pPlugin, LPCTSTR pszExtension)
{
	int nItem = 0;
	CString strCurrAssoc, strAssocAdd;

	int nCount = m_wndList.GetItemCount();
	for ( ; nItem < nCount ; nItem++ )
	{
		LPVOID pExisting = (LPVOID)m_wndList.GetItemData( nItem );
		CString strExisting = m_wndList.GetItemText( nItem, 0 );

		if ( pPlugin != NULL && pExisting == NULL ) break;
		if ( pPlugin == NULL && pExisting != NULL ) continue;
		if ( strExisting.Compare( pszName ) == 0 )
		{
			if ( pszExtension && _tcslen( pszExtension ) )
			{
				strCurrAssoc = m_wndList.GetItemText( nItem, 2 );
				strAssocAdd.Format( L"|%s|", pszExtension );

				if ( strCurrAssoc.Find( strAssocAdd ) == -1 )
				{
					strCurrAssoc.Append( strAssocAdd );
					strCurrAssoc.Replace( L"||", L"|" );
					m_wndList.SetItemText( nItem, 2, strCurrAssoc );
				}
			}
			return;
		}
		if ( strExisting.Compare( pszName ) > 0 ) break;
	}

	nItem = m_wndList.InsertItem( LVIF_IMAGE|LVIF_TEXT|LVIF_PARAM, nItem,
		pszName, 0, 0, nImage, (LPARAM)pPlugin );

	m_wndList.SetItemText( nItem, 1, pszCLSID );

	if ( pszExtension && _tcslen( pszExtension ) )
	{
		strAssocAdd.Format( L"|%s|", pszExtension );
		m_wndList.SetItemText( nItem, 2, strAssocAdd );
	}
	else
		m_wndList.SetItemText( nItem, 2, bEnabled < TRI_TRUE ? L"-" : L"" );

	if ( bEnabled != TRI_UNKNOWN )
		m_wndList.SetItemState( nItem, bEnabled << 12, LVIS_STATEIMAGEMASK );
}

void CPluginsSettingsPage::EnumerateGenericPlugins()
{
	Plugins.Enumerate();

	for ( POSITION pos = Plugins.GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = Plugins.GetNext( pos );

		if ( ! pPlugin->m_sName.IsEmpty() )
		{
			int nImage = AddIcon( pPlugin->LookupIcon(), m_gdiImageList );

			InsertPlugin( pPlugin->GetStringCLSID(), pPlugin->m_sName,
				( ( nImage == -1 ) ? 0 : nImage ),
				pPlugin->m_pPlugin != NULL ? TRI_TRUE : TRI_FALSE, pPlugin );
		}
	}
}

void CPluginsSettingsPage::EnumerateMiscPlugins()
{
	HKEY hPlugins = NULL;

	if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_CURRENT_USER, REGISTRY_KEY L"\\Plugins", 0, KEY_READ, &hPlugins ) )
		return;

	for ( DWORD nIndex = 0 ; ; nIndex++ )
	{
		HKEY hCategory = NULL;
		TCHAR szName[128];
		DWORD nName = 128;

		if ( ERROR_SUCCESS != RegEnumKeyEx( hPlugins, nIndex, szName, &nName, NULL, NULL, NULL, NULL ) )
			break;

		if ( _tcsicmp( szName, L"General" ) != 0 )
		{
			if ( ERROR_SUCCESS == RegOpenKeyEx( hPlugins, szName, 0, KEY_READ, &hCategory ) )
			{
				EnumerateMiscPlugins( szName, hCategory );
				RegCloseKey( hCategory );
			}
		}
	}

	RegCloseKey( hPlugins );
}

void CPluginsSettingsPage::EnumerateMiscPlugins(LPCTSTR pszType, HKEY hRoot)
{
	CMap< CString, const CString&, CString, CString& >	pCLSIDs;
	CString strPath = REGISTRY_KEY L"\\Plugins";

	for ( DWORD nIndex = 0 ; ; nIndex++ )
	{
		CWaitCursor pCursor;

		DWORD nName = 128, nValue = 128, nType = REG_SZ;
		TCHAR szName[128], szValue[128];

		if ( ERROR_SUCCESS != RegEnumValue( hRoot, nIndex, szName, &nName, NULL, &nType, (LPBYTE)szValue, &nValue ) )
			break;

		if ( nType == REG_SZ && szValue[0] == '{' )
		{
			CString strExts, strEnabledExt, strDisabledExt, strCurrExt;

			if ( *szName == '.' )
			{
				if ( pCLSIDs.Lookup( szValue, strExts ) == FALSE )
				{
					DWORD nLength = 0;
					HKEY hUserPlugins = NULL;

					if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, strPath, 0, KEY_READ, &hUserPlugins ) )
					{
						if ( ERROR_SUCCESS == RegQueryValueEx( hUserPlugins, (LPCTSTR)szValue, NULL, &nType, NULL, &nLength ) &&
							 nType == REG_SZ && nLength )
						{
							TCHAR* pszExtValue = new TCHAR[ nLength ];
							if ( ERROR_SUCCESS == RegQueryValueEx( hUserPlugins, (LPCTSTR)szValue, NULL, &nType, (LPBYTE)pszExtValue, &nLength ) )
								strExts.SetString( pszExtValue );	// Found under user options

							delete [] pszExtValue;
						}
						else if ( nType == REG_DWORD )	// Upgrade from REG_DWORD to REG_SZ
						{
							BOOL bEnabled = theApp.GetProfileInt( L"Plugins", szValue, TRUE );
							strExts = bEnabled ? L"" : L"-";
						}
						RegCloseKey( hUserPlugins );
					}
				}

				// Disabled extensions have '-' sign before their names
				strEnabledExt.Format( L"|%s|", szName );
				strDisabledExt.Format( L"|-%s|", szName );
				if ( strExts.Find( strEnabledExt ) == -1 )
				{
					if ( strExts.Find( strDisabledExt ) == -1 )
					{
						// Missing extension under user options; append to the list
						// Leave "-" if upgrading: it will be removed eventually when user applies settings
						strCurrExt = ( strExts[ 0 ] == L'-' ) ? strDisabledExt : strEnabledExt;
						strExts.Append( strCurrExt );
					}
					else
						strCurrExt = strDisabledExt;
				}
				else strCurrExt = strEnabledExt;
			}

			strExts.Replace( L"||", L"|" );
			pCLSIDs.SetAt( szValue, strExts );
			if ( ! strExts.IsEmpty() ) theApp.WriteProfileString( L"Plugins", szValue, strExts );
			strCurrExt.Replace( L"|", L"" );
			AddMiscPlugin( pszType, szValue, strCurrExt );
		}
	}
}

void CPluginsSettingsPage::AddMiscPlugin(LPCTSTR /*pszType*/, LPCTSTR pszCLSID, LPCTSTR pszExtension)
{
	HKEY hClass = NULL;
	CString strClass;
	CLSID pCLSID;

	strClass.Format( L"CLSID\\%s", pszCLSID );

	if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CLASSES_ROOT, strClass, 0, KEY_READ, &hClass ) )
	{
		DWORD nValue = MAX_PATH * sizeof( TCHAR ), nType = REG_SZ;
		TCHAR szValue[ MAX_PATH ];

		if ( ERROR_SUCCESS == RegQueryValueEx( hClass, NULL, NULL, &nType, (LPBYTE)szValue, &nValue ) )
		{
			if ( Hashes::fromGuid( pszCLSID, &pCLSID ) )
			{
				TRISTATE bEnabled = TRI_UNKNOWN;
				bEnabled = Plugins.LookupEnable( pCLSID ) ? TRI_TRUE : TRI_FALSE;
				InsertPlugin( pszCLSID, szValue, 1, bEnabled, NULL, pszExtension );
			}
		}

		RegCloseKey( hClass );
	}
}

// Plugin descriptions are taken from Comments in its code resources. Authors can put any information here.
CString CPluginsSettingsPage::GetPluginComments(LPCTSTR pszCLSID) const
{
	CString strPath;
	HKEY hClassServer = NULL;

	strPath.Format( L"CLSID\\%s\\InProcServer32", pszCLSID );

	if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_CLASSES_ROOT, strPath, 0, KEY_READ, &hClassServer ) )
		return CString();

	DWORD nValue = MAX_PATH * sizeof( TCHAR ), nType = REG_SZ;
	TCHAR szPluginPath[ MAX_PATH ];

	if ( ERROR_SUCCESS != RegQueryValueEx( hClassServer, NULL, NULL, &nType,(LPBYTE)szPluginPath, &nValue ) || nType != REG_SZ )
		return CString();

	strPath.SetString( szPluginPath );

	DWORD nSize = GetFileVersionInfoSize( strPath, &nSize );
	BYTE* pBuffer = new BYTE[ nSize ];

	if ( ! GetFileVersionInfo( strPath, NULL, nSize, pBuffer ) )
	{
		delete [] pBuffer;
		return CString();
	}

	WCHAR* pLanguage = (WCHAR*)pBuffer + 20 + 26 + 18 + 3;

	if ( wcslen( pLanguage ) != 8 )
	{
		delete [] pBuffer;
		return CString();
	}

	CString strKey, strValue;

	strKey = L"\\StringFileInfo\\";
	strKey.Append( pLanguage );
	strKey.Append( L"\\Comments" );

	BYTE* pValue = NULL;
	nSize = 0;

	if ( VerQueryValue( pBuffer, (LPTSTR)(LPCTSTR)strKey, (void**)&pValue, (UINT*)&nSize ) )
	{
		if ( pValue[1] )
			strValue = (LPCSTR)pValue;
		else
			strValue = (LPCTSTR)pValue;
	}

	delete [] pBuffer;

	return strValue;
}

void CPluginsSettingsPage::UpdateList()
{
	if ( m_hWnd == NULL ) return;

	if ( ! IsWindowVisible() || m_wndList.GetItemCount() == 0 )
	{
		m_bRunning = FALSE;

		CWaitCursor wc;

		m_wndList.DeleteAllItems();
		EnumerateGenericPlugins();
		EnumerateMiscPlugins();

		m_bRunning = TRUE;
	}
}

void CPluginsSettingsPage::OnTimer(UINT_PTR /*nIDEvent*/)
{
	if ( IsWindowVisible() )
	{
		KillTimer( 1 );
		UpdateList();
	}
}

void CPluginsSettingsPage::OnNMDblclkPlugins(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnPluginsSetup();
	*pResult = 0;
}
