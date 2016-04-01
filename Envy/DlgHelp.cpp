//
// DlgHelp.cpp
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
#include "Envy.h"
#include "DlgHelp.h"
#include "Colors.h"
#include "Skin.h"
#include "RichElement.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CHelpDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CHelpDlg, CSkinDialog)
	ON_NOTIFY(RVN_CLICK, IDC_HELP_VIEW, OnClickView)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHelpDlg setup

BOOL CHelpDlg::Show(LPCTSTR pszName, CWnd* pParent)
{
	CHelpDlg dlg( pszName, pParent );
	return dlg.DoModal() == IDOK;
}

/////////////////////////////////////////////////////////////////////////////
// CHelpDlg construction

CHelpDlg::CHelpDlg(LPCTSTR pszName, CWnd* pParent)
	: CSkinDialog( CHelpDlg::IDD, pParent )
{
	m_sDocument = pszName;
}

void CHelpDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CHelpDlg message handlers

BOOL CHelpDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	if ( CXMLElement* pXML = Skin.GetDocument( m_sDocument ) )
		m_pDocument.LoadXML( pXML );
	else
		PostMessage( WM_CLOSE );

	m_pDocument.m_crBackground = Colors.m_crDialog;

	CRect rcClient;
	GetClientRect( &rcClient );
	rcClient.bottom -= 32;	// Dialog Button Area  ToDo: Set Constant?
	//rcClient.top += Skin.m_nBanner;

	m_wndView.Create( WS_CHILD | WS_VISIBLE, rcClient, this, IDC_HELP_VIEW );
	m_wndView.SetDocument( &m_pDocument );
	m_wndView.SetSelectable( TRUE );

	SkinMe( L"CHelpDlg", ID_HELP_ABOUT );

	return TRUE;
}

void CHelpDlg::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	if ( CRichElement* pElement = ((RVN_ELEMENTEVENT*)pNotify)->pElement )
		theApp.InternalURI( pElement->m_sLink );
}
