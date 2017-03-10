//
// PageSettingsAdvanced.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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

// Previously "PageSettingsTraffic.cpp"

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "LiveList.h"
#include "PageSettingsAdvanced.h"
#include "CoolInterface.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define EDIT_TOKEN L'•'

IMPLEMENT_DYNCREATE(CAdvancedSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CAdvancedSettingsPage, CSettingsPage)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROPERTIES, &CAdvancedSettingsPage::OnItemChangedProperties)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROPERTIES, &CAdvancedSettingsPage::OnColumnClickProperties)
	ON_BN_CLICKED(IDC_DEFAULT_VALUE, &CAdvancedSettingsPage::OnBnClickedDefaultValue)
	ON_BN_CLICKED(IDC_CHECK, &CAdvancedSettingsPage::OnChangeValue)
	ON_EN_CHANGE(IDC_VALUE, &CAdvancedSettingsPage::OnChangeValue)
	ON_EN_CHANGE(IDC_MESSAGE, &CAdvancedSettingsPage::OnChangeValue)
	ON_CBN_SELCHANGE(IDC_FONT, &CAdvancedSettingsPage::OnChangeValue)
	ON_EN_CHANGE(IDC_FILTER_BOX, &CAdvancedSettingsPage::OnEnChangeQuickfilter)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage property page

CAdvancedSettingsPage::CAdvancedSettingsPage()
	: CSettingsPage(CAdvancedSettingsPage::IDD)
	, m_bUpdating( false )
	, m_nTimer		( 0 )
{
}

//CAdvancedSettingsPage::~CAdvancedSettingsPage()
//{
//}

void CAdvancedSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROPERTIES, m_wndList);
	DDX_Control(pDX, IDC_VALUE_SPIN, m_wndValueSpin);
	DDX_Control(pDX, IDC_VALUE, m_wndValue);
	DDX_Control(pDX, IDC_MESSAGE, m_wndText);
	DDX_Control(pDX, IDC_FONT, m_wndFonts);
	DDX_Control(pDX, IDC_CHECK, m_wndBool);
	DDX_Control(pDX, IDC_DEFAULT_VALUE, m_wndDefault);
	DDX_Control(pDX, IDC_FILTER_BOX, m_wndQuickFilter);
	DDX_Control(pDX, IDC_SEARCH, m_wndQuickFilterIcon);
}

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage message handlers

BOOL CAdvancedSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

//	for ( POSITION pos = Settings.GetHeadPosition() ; pos ; )
//	{
//		const CSettings::Item* pItem = Settings.GetNext( pos );
//
//		if ( ! pItem->m_bHidden &&
//			( pItem->m_pBool ||
//			( pItem->m_pDword && pItem->m_nScale ) ||
//			( pItem->m_pString && pItem->m_nType != CSettings::setNull ) ) )
//		{
//			m_pSettings.AddTail( new EditItem( pItem ) );
//		}
//	}

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	m_wndList.InsertColumn( 0, L"Setting", LVCFMT_LEFT, rc.right - 100, 0 );
	m_wndList.InsertColumn( 1, L"Value", LVCFMT_LEFT, 100, 1 );
	m_wndList.SetExtendedStyle( /*m_wndList.GetExtendedStyle()|*/ LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );

	m_wndQuickFilterIcon.SetIcon( CoolInterface.ExtractIcon( ID_SEARCH_SEARCH, FALSE, LVSIL_SMALL ) );

	Skin.Translate( L"CAdvancedSettingsList", m_wndList.GetHeaderCtrl() );

	AddSettings();

	//CLiveList::Sort( &m_wndList, 0 );
	//CLiveList::Sort( &m_wndList, 0 );	// Repeat?

	//UpdateInputArea();

	m_wndList.EnsureVisible( Settings.General.LastSettingsIndex, FALSE );
	m_wndList.EnsureVisible( Settings.General.LastSettingsIndex + m_wndList.GetCountPerPage() - 1, FALSE );

	return TRUE;
}

void CAdvancedSettingsPage::AddSettings()
{
	m_wndList.DeleteAllItems();

	CString strQuickFilter;
	m_wndQuickFilter.GetWindowText( strQuickFilter );
	strQuickFilter.Trim();
	if ( strQuickFilter == L"*" )
		strQuickFilter = EDIT_TOKEN;

	for ( POSITION pos = Settings.GetHeadPosition() ; pos ; )
	{
		CSettings::Item* pItem = Settings.GetNext( pos );
		if ( ! pItem->m_bHidden &&
			( pItem->m_pBool ||
			( pItem->m_pDword && pItem->m_nScale ) ||
			( pItem->m_pString && pItem->m_nType != CSettings::setNull ) ) )
		{
			EditItem* pEdit = new EditItem( pItem );
			ASSERT( pEdit != NULL );
			if ( pEdit == NULL ) return;
			if ( ! strQuickFilter.IsEmpty() && _tcsistr( pEdit->m_sName, strQuickFilter ) == NULL )
				continue;

			LV_ITEM pList = {};
			pList.mask		= LVIF_PARAM|LVIF_TEXT|LVIF_IMAGE;
			pList.iItem		= m_wndList.GetItemCount();
			pList.lParam	= (LPARAM)pEdit;
			pList.iImage	= 0;
			pList.pszText	= (LPTSTR)(LPCTSTR)pEdit->m_sName;
			pList.iItem		= m_wndList.InsertItem( &pList );

			UpdateListItem( pList.iItem );
		}
	}

	CLiveList::Sort( &m_wndList );
	UpdateInputArea();
}

void CAdvancedSettingsPage::CommitAll()
{
	const int nCount = m_wndList.GetItemCount();
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Commit();
	}
}

void CAdvancedSettingsPage::UpdateAll()
{
	const int nCount = m_wndList.GetItemCount();
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Update();
		UpdateListItem( nItem );
	}

	UpdateInputArea();
}

//int CAdvancedSettingsPage::GetListItem(const EditItem* pItem)
//{
//	const int nCount = m_wndList.GetItemCount();
//	for ( int nItem = 0 ; nItem < nCount ; ++nItem )
//	{
//		if ( (const EditItem*)m_wndList.GetItemData( nItem ) == pItem )
//			return nItem;
//	}
//	return -1;
//}

void CAdvancedSettingsPage::UpdateListItem(int nItem)
{
	if ( nItem < 0 )
		return;

	EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );

	CString strValue;

	if ( pItem->m_pItem->m_pBool )
	{
		ASSERT( pItem->m_pItem->m_nScale == 1 &&
			pItem->m_pItem->m_nMin == 0 &&
			pItem->m_pItem->m_nMax == 1 );
		strValue = pItem->m_bValue ? L"True" : L"False";
	}
	else if ( pItem->m_pItem->m_pDword )
	{
		ASSERT( pItem->m_pItem->m_nScale &&
			pItem->m_pItem->m_nMin <= pItem->m_pItem->m_nMax );
		strValue.Format( L"%lu", pItem->m_nValue / pItem->m_pItem->m_nScale );
		if ( Settings.General.LanguageRTL )
			strValue = L"\x200E" + strValue + pItem->m_pItem->m_szSuffix;
		else
			strValue += pItem->m_pItem->m_szSuffix;
	}
	else if ( pItem->m_pItem->m_pString )
	{
		strValue = pItem->m_sValue;
	}

	m_wndList.SetItemText( nItem, 1, strValue );

	if ( pItem->IsDefault() )
	{
		if ( pItem->m_sName.Right( 1 ) == EDIT_TOKEN )	// "•"
		{
			pItem->m_sName.TrimRight( EDIT_TOKEN );		// "•"
			m_wndList.SetItemText( nItem, 0, pItem->m_sName );
		}
	}
	else
	{
		if ( pItem->m_sName.Right( 1 ) != EDIT_TOKEN )	// "•"
		{
			pItem->m_sName += EDIT_TOKEN;	// "•"
			m_wndList.SetItemText( nItem, 0, pItem->m_sName );
		}
	}
}

void CAdvancedSettingsPage::OnItemChangedProperties(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	UpdateInputArea();
}

void CAdvancedSettingsPage::UpdateInputArea()
{
	if ( m_bUpdating ) return;
	m_bUpdating = true;

	const int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );

	m_wndValueSpin.ShowWindow( SW_HIDE );
	m_wndValueSpin.EnableWindow( FALSE );

	m_wndValue.ShowWindow( SW_HIDE );
	m_wndValue.EnableWindow( FALSE );

	m_wndText.ShowWindow( SW_HIDE );
	m_wndText.EnableWindow( FALSE );

	m_wndFonts.ShowWindow( SW_HIDE );
	m_wndFonts.EnableWindow( TRUE );

	m_wndBool.ShowWindow( SW_HIDE );
	m_wndBool.EnableWindow( FALSE );

	m_wndDefault.EnableWindow( FALSE );

	if ( nItem >= 0 )
	{
		const EditItem* pItem = (const EditItem*)m_wndList.GetItemData( nItem );

		const BOOL bEnable = pItem->m_pItem->m_nType != CSettings::setReadOnly;

		if ( pItem->m_pItem->m_pDword )
		{
			CString strValue;
			strValue.Format( L"%lu", pItem->m_nValue / pItem->m_pItem->m_nScale );
			m_wndValue.ShowWindow( SW_SHOW );
			m_wndValue.EnableWindow( bEnable );
			m_wndValueSpin.ShowWindow( SW_SHOW );
			m_wndValueSpin.EnableWindow( bEnable );
			pItem->m_pItem->SetRange( m_wndValueSpin );
			m_wndValue.SetWindowText( strValue );
		}
		else if ( pItem->m_pItem->m_pBool )
		{
			m_wndBool.SetCheck( pItem->m_bValue ? BST_CHECKED : BST_UNCHECKED );
			m_wndBool.ShowWindow( SW_SHOW );
			m_wndBool.EnableWindow( bEnable );
		}
		else if ( pItem->m_pItem->m_pString && pItem->m_pItem->m_nType == CSettings::setFont )
		{
			m_wndFonts.SelectFont( pItem->m_sValue );
			m_wndFonts.ShowWindow( SW_SHOW );
			m_wndFonts.EnableWindow( TRUE );
		}
		else if ( pItem->m_pItem->m_pString /*&& pItem->m_pItem->m_nType == CSettings::setString*/ )
		{
			m_wndText.SetWindowText( pItem->m_sValue );
			m_wndText.ShowWindow( SW_SHOW );
			m_wndText.EnableWindow( bEnable );
		}

		if ( bEnable && ! pItem->IsDefault() )
			m_wndDefault.EnableWindow( TRUE );

		//GetDlgItem( IDC_DEFAULT_VALUE )->EnableWindow( ! pItem->IsDefault() );
	}

	m_bUpdating = false;
}

void CAdvancedSettingsPage::OnChangeValue()
{
	if ( m_wndList.m_hWnd == NULL || m_bUpdating ) return;

	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );

	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );

		if ( pItem->m_pItem->m_nType == CSettings::setReadOnly )
			return;

		if ( pItem->m_pItem->m_pDword )
		{
			DWORD nValue;
			CString strValue;
			m_wndValue.GetWindowText( strValue );
			if ( _stscanf( strValue, L"%lu", &nValue ) != 1 || nValue > pItem->m_pItem->m_nMax || nValue < pItem->m_pItem->m_nMin )
				return;
			pItem->m_nValue = nValue * pItem->m_pItem->m_nScale;
		}
		else if ( pItem->m_pItem->m_pBool )
		{
			pItem->m_bValue = ( m_wndBool.GetCheck() == BST_CHECKED );
		}
		else if ( pItem->m_pItem->m_pString && pItem->m_pItem->m_nType == CSettings::setFont )
		{
			pItem->m_sValue = m_wndFonts.GetSelectedFont();
		}
		else if ( pItem->m_pItem->m_pString )	// && pItem->m_pItem->m_nType == CSettings::setString
		{
			m_wndText.GetWindowText( pItem->m_sValue );
		}
		else
			return;

		UpdateListItem( nItem );

		m_wndDefault.EnableWindow( ! pItem->IsDefault() );
	}
}

void CAdvancedSettingsPage::OnColumnClickProperties(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CAdvancedSettingsPage::OnOK()
{
	CommitAll();

	UpdateAll();

	m_wndFonts.Invalidate();

	CSettingsPage::OnOK();
}

void CAdvancedSettingsPage::OnDestroy()
{
	if ( m_nTimer )
		KillTimer( m_nTimer );

	CString strFilter;
	m_wndQuickFilter.GetWindowText( strFilter );
	strFilter.Trim();

	if ( strFilter.IsEmpty() )
		Settings.General.LastSettingsIndex = m_wndList.GetTopIndex();

	const int nCount = m_wndList.GetItemCount();
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		delete (EditItem*)m_wndList.GetItemData( nItem );
	}

	CSettingsPage::OnDestroy();
}

void CAdvancedSettingsPage::OnBnClickedDefaultValue()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Default();

		UpdateListItem( nItem );
		UpdateInputArea();
	}
}

bool CAdvancedSettingsPage::IsModified() const
{
	const int nCount = m_wndList.GetItemCount();
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		const EditItem* pItem = (const EditItem*)m_wndList.GetItemData( nItem );
		if ( pItem->IsModified() )
			return true;
	}
	return false;
}

void CAdvancedSettingsPage::OnEnChangeQuickfilter()
{
	if ( m_nTimer )
		KillTimer( m_nTimer );

	m_nTimer = SetTimer( 1, 600, NULL );
}

void CAdvancedSettingsPage::OnTimer(UINT_PTR nIDEvent)
{
	if ( m_nTimer == nIDEvent )
	{
		KillTimer( m_nTimer );
		AddSettings();
	}

	CSettingsPage::OnTimer( nIDEvent );
}

/////////////////////////////////////////////////////////////////////////////
// CSettingEdit construction

CAdvancedSettingsPage::EditItem::EditItem(const CSettings::Item* pItem)
	: m_pItem			( const_cast<CSettings::Item*>( pItem ) )
	, m_nValue			( pItem->m_pDword ? *pItem->m_pDword : 0 )
	, m_bValue			( pItem->m_pBool ? *pItem->m_pBool : false )
	, m_sValue			( pItem->m_pString ? *pItem->m_pString : CString() )
	, m_nOriginalValue	( pItem->m_pDword ? *pItem->m_pDword : 0 )
	, m_bOriginalValue	( pItem->m_pBool ? *pItem->m_pBool : false )
	, m_sOriginalValue	( pItem->m_pString ? *pItem->m_pString : CString() )
	, m_sName			( ( ! *pItem->m_szSection ||							// Settings.Name -> General.Name
							! lstrcmpi( pItem->m_szSection, L"Settings" ) )		// .Name -> General.Name
							? L"General" : pItem->m_szSection )
{
	m_sName += L".";
	m_sName += pItem->m_szName;
	if ( ! IsDefault() )
		m_sName += EDIT_TOKEN;	// "•"
}

void CAdvancedSettingsPage::EditItem::Update()
{
	if ( m_pItem->m_pDword )
		m_nOriginalValue = m_nValue = *m_pItem->m_pDword;
	else if ( m_pItem->m_pBool )
		m_bOriginalValue = m_bValue = *m_pItem->m_pBool;
	else if ( m_pItem->m_pString )
		m_sOriginalValue = m_sValue = *m_pItem->m_pString;
}

void CAdvancedSettingsPage::EditItem::Commit()
{
	if ( m_pItem->m_pDword )
	{
		if ( m_nValue != m_nOriginalValue )
			*m_pItem->m_pDword = m_nOriginalValue = m_nValue;
	}
	else if ( m_pItem->m_pBool )
	{
		if ( m_bValue != m_bOriginalValue )
			*m_pItem->m_pBool= m_bOriginalValue = m_bValue;
	}
	else if ( m_pItem->m_pString )
	{
		if ( m_sValue != m_sOriginalValue )
			*m_pItem->m_pString= m_sOriginalValue = m_sValue;
	}
}

bool CAdvancedSettingsPage::EditItem::IsModified() const
{
	if ( m_pItem->m_pDword )
		return ( m_nValue != m_nOriginalValue );
	if ( m_pItem->m_pBool )
		return ( m_bValue != m_bOriginalValue );
	if ( m_pItem->m_pString )
		return ( m_sValue != m_sOriginalValue );

	return false;
}

bool CAdvancedSettingsPage::EditItem::IsDefault() const
{
	if ( m_pItem->m_pDword )
		return ( m_pItem->m_DwordDefault == m_nValue );
	if ( m_pItem->m_pBool )
		return ( m_pItem->m_BoolDefault == m_bValue );
	if ( m_pItem->m_pString )
		return ( m_pItem->m_StringDefault == NULL || m_sValue == m_pItem->m_StringDefault );

	return true;
}

void CAdvancedSettingsPage::EditItem::Default()
{
	if ( m_pItem->m_pDword )
		m_nValue = m_pItem->m_DwordDefault;
	else if ( m_pItem->m_pBool )
		m_bValue = m_pItem->m_BoolDefault;
	else if ( m_pItem->m_pString )
		m_sValue = m_pItem->m_StringDefault;
}
