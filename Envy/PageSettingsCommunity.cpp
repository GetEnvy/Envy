//
// PageSettingsCommunity.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2007
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
#include "PageSettingsCommunity.h"
#include "DlgProfileManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CCommunitySettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CCommunitySettingsPage, CSettingsPage)
	ON_BN_CLICKED(IDC_EDIT_PROFILE, OnEditProfile)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCommunitySettingsPage property page

CCommunitySettingsPage::CCommunitySettingsPage()
	: CSettingsPage(CCommunitySettingsPage::IDD)
	, m_bProfileEnable	( FALSE )
	, m_bChatEnable 	( FALSE )
	, m_bChatAllNetworks ( FALSE )
	, m_bChatFilter 	( FALSE )
	, m_bChatCensor 	( FALSE )
{
}

CCommunitySettingsPage::~CCommunitySettingsPage()
{
}

void CCommunitySettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ENABLE, m_bProfileEnable);
	DDX_Check(pDX, IDC_CHAT_ENABLE, m_bChatEnable);
	DDX_Check(pDX, IDC_CHAT_ALLNETWORKS, m_bChatAllNetworks);
	DDX_Check(pDX, IDC_CHAT_FILTER, m_bChatFilter);
	DDX_Check(pDX, IDC_CHAT_CENSOR, m_bChatCensor);
}

/////////////////////////////////////////////////////////////////////////////
// CCommunitySettingsPage message handlers

BOOL CCommunitySettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bProfileEnable	= Settings.Community.ServeProfile;
	m_bChatEnable		= Settings.Community.ChatEnable;
	m_bChatAllNetworks	= Settings.Community.ChatAllNetworks;
	m_bChatFilter		= Settings.Community.ChatFilter;
	m_bChatCensor		= Settings.Community.ChatCensor;

	UpdateData( FALSE );

	return TRUE;
}

void CCommunitySettingsPage::OnEditProfile()
{
	CProfileManagerDlg dlg;
	dlg.DoModal();
}

void CCommunitySettingsPage::OnOK()
{
	UpdateData();

	Settings.Community.ServeProfile		= m_bProfileEnable != FALSE;
	Settings.Community.ChatEnable		= m_bChatEnable != FALSE;
	Settings.Community.ChatAllNetworks	= m_bChatAllNetworks != FALSE;
	Settings.Community.ChatFilter		= m_bChatFilter != FALSE;
	Settings.Community.ChatCensor		= m_bChatCensor != FALSE;

	CSettingsPage::OnOK();
}
