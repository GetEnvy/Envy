//
// PageSettingsRich.cpp
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
#include "WndSettingsSheet.h"
#include "PageSettingsRich.h"
#include "RichDocument.h"
#include "RichElement.h"
#include "Colors.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CRichSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CRichSettingsPage, CSettingsPage)
	ON_NOTIFY(RVN_CLICK, IDC_RICH_VIEW, OnClickView)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRichSettingsPage property page

CRichSettingsPage::CRichSettingsPage(LPCTSTR pszName)
	: CSettingsPage(CRichSettingsPage::IDD, pszName )
	, m_pDocument ( NULL )
{
	// Early caption load for settings tree items
	if ( CXMLElement* pXML = Skin.GetDocument( m_sName ) )
		m_sCaption = pXML->GetAttributeValue( L"title", m_sName );
}

CRichSettingsPage::~CRichSettingsPage()
{
	delete m_pDocument;
}

void CRichSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CRichSettingsPage message handlers

BOOL CRichSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	CRect rc;
	GetClientRect( &rc );
	m_wndView.Create( WS_VISIBLE, rc, this, IDC_RICH_VIEW );

	return TRUE;
}

void CRichSettingsPage::OnSkinChange()
{
	CSettingsPage::OnSkinChange();

	if ( ! IsWindow( GetSafeHwnd() ) )
		return;		// No page created yet

	// (Re)Load document
	if ( CXMLElement* pXML = Skin.GetDocument( m_sName ) )
	{
		m_sCaption = pXML->GetAttributeValue( L"title", m_sName );
		SetWindowText( m_sCaption );

		delete m_pDocument;
		m_pDocument = new CRichDocument();
		m_pDocument->LoadXML( pXML );
		m_pDocument->m_crBackground = Colors.m_crDialog;
		m_wndView.SetDocument( m_pDocument );
	}
}

void CRichSettingsPage::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	CRichElement* pElement = ( (RVN_ELEMENTEVENT*)pNotify )->pElement;
	if ( ! pElement ) return;

	if ( _tcsncmp( pElement->m_sLink, L"page:", 5 ) == 0 )
	{
		CString strPage = pElement->m_sLink.Mid( 5 );
		GetSheet()->SetActivePage( GetSheet()->GetPage( strPage ) );
	}
}
