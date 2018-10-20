//
// DlgPluginExtSetup.cpp
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
#include "Envy.h"
#include "DlgPluginExtSetup.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CPluginExtSetupDlg, CDialog)

BEGIN_MESSAGE_MAP(CPluginExtSetupDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_ASSOCIATIONS, OnChangingAssociations)
	ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

CPluginExtSetupDlg::CPluginExtSetupDlg(CWnd* pParent, LPCTSTR pszExt)
	: CDialog(CPluginExtSetupDlg::IDD, pParent)
{
	m_sExtensions	= (CString)pszExt;
	m_pParent		= (CListCtrl*)pParent;
}

CPluginExtSetupDlg::~CPluginExtSetupDlg()
{
}

void CPluginExtSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ASSOCIATIONS, m_wndList);
}

BOOL CPluginExtSetupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	m_wndList.InsertColumn( 0, L"Extension", LVCFMT_LEFT, rc.right, 0 );
	m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES|LVS_EX_LABELTIP );

	CStringArray oTokens;
	Split( m_sExtensions, L'|', oTokens );

	m_bRunning = FALSE;

	INT_PTR nTotal = oTokens.GetCount();
	INT_PTR nChecked = 0;

	for ( INT_PTR nToken = 0; nToken < nTotal; nToken++ )
	{
		CString strToken = oTokens.GetAt( nToken );
		if ( strToken.IsEmpty() ) continue;		// Shouldn't happen

		BOOL bChecked = ( strToken[ 0 ] != L'-' );
		int nItem = m_wndList.InsertItem( LVIF_TEXT, m_wndList.GetItemCount(),
			! bChecked ? strToken.Mid( 1 ) : strToken, 0, 0, 0, 0 );

		if ( bChecked )
		{
			m_wndList.SetItemState( nItem, 2 << 12, LVIS_STATEIMAGEMASK );
			nChecked++;
		}
	}

	if ( nChecked == nTotal ) m_bParentState = TRI_TRUE;
	else if ( nChecked == 0 ) m_bParentState = TRI_FALSE;
	else m_bParentState = TRI_UNKNOWN;

	m_bRunning = TRUE;

	UpdateData( FALSE );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPluginExtSetupDlg message handlers

void CPluginExtSetupDlg::OnChangingAssociations(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if ( m_bRunning &&
		 ( pNMLV->uOldState & LVIS_STATEIMAGEMASK ) == 0 &&
		 ( pNMLV->uNewState & LVIS_STATEIMAGEMASK ) != 0 )
		*pResult = 1;
}

void CPluginExtSetupDlg::OnOK()
{
	CString strCurrExt, strExt;
	int nTotal = m_wndList.GetItemCount();
	int nChecked = 0;
	TRISTATE bCurrState = m_bParentState;

	for ( int nItem = 0; nItem < nTotal; nItem++ )
	{
		TRISTATE bEnabled = static_cast< TRISTATE >(
			m_wndList.GetItemState( nItem, LVIS_STATEIMAGEMASK ) >> 12 );

		if ( bEnabled == TRI_TRUE )
		{
			nChecked++;
			strExt = m_wndList.GetItemText( nItem, 0 );
		}
		else
			strExt = L"-" + m_wndList.GetItemText( nItem, 0 );

		// Invert the order since the extension map becomes inversed
		strCurrExt.Insert( 0, L"|" );
		strCurrExt.Insert( 0, strExt );
	}
	if ( ! strCurrExt.IsEmpty() )
		strCurrExt.Insert( 0, L"|" );

	if ( nChecked == nTotal ) bCurrState = TRI_TRUE;
	else if ( nChecked == 0 ) bCurrState = TRI_FALSE;
	else bCurrState = TRI_UNKNOWN;

	if ( strCurrExt != m_sExtensions )
	{
		int nItem = m_pParent->GetNextItem( -1, LVNI_SELECTED );
		m_pParent->SetItemText( nItem, 2, strCurrExt );
		if ( bCurrState != m_bParentState )
		{
			if ( bCurrState != TRI_UNKNOWN )	// 0 state removes checkbox, we don't need that
				m_pParent->SetItemState( nItem, bCurrState << 12, LVIS_STATEIMAGEMASK );
			else
				m_pParent->SetItemState( nItem, 2 << 12, LVIS_STATEIMAGEMASK );
		}
		m_bParentState = bCurrState;
		m_sExtensions = strCurrExt;
	}

	CDialog::OnOK();
}
