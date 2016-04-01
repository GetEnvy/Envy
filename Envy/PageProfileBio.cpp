//
// PageProfileBio.cpp : implementation file
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
#include "PageProfileBio.h"
#include "GProfile.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CBioProfilePage, CSettingsPage)

//BEGIN_MESSAGE_MAP(CBioProfilePage, CSettingsPage)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBioProfilePage property page

CBioProfilePage::CBioProfilePage() : CSettingsPage( CBioProfilePage::IDD )
{
}

CBioProfilePage::~CBioProfilePage()
{
}

void CBioProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROFILE_BIO, m_wndText);
}

/////////////////////////////////////////////////////////////////////////////
// CBioProfilePage message handlers

BOOL CBioProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	if ( CXMLElement* pNotes = MyProfile.GetXML( L"notes" ) )
		m_wndText.SetWindowText( pNotes->GetValue() );

	UpdateData( FALSE );

	return TRUE;
}

void CBioProfilePage::OnOK()
{
	if ( CXMLElement* pNotes = MyProfile.GetXML( L"notes", TRUE ) )
	{
		CString str;
		m_wndText.GetWindowText( str );

		if ( ! str.IsEmpty() )
			pNotes->SetValue( str );
		else
			pNotes->Delete();
	}

	CSettingsPage::OnOK();
}
