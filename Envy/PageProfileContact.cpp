//
// PageProfileContact.cpp
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
#include "PageProfileContact.h"
#include "GProfile.h"
#include "CoolInterface.h"
#include "XML.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CContactProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CContactProfilePage, CSettingsPage)
	ON_WM_PAINT()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CContactProfilePage property page

CContactProfilePage::CContactProfilePage() : CSettingsPage( CContactProfilePage::IDD )
{
}

CContactProfilePage::~CContactProfilePage()
{
}

void CContactProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROFILE_EMAIL, m_sEmail);
	DDX_Text(pDX, IDC_PROFILE_MSN, m_sMSN);
	DDX_Text(pDX, IDC_PROFILE_YAHOO, m_sYahoo);
	DDX_Text(pDX, IDC_PROFILE_AOL, m_sAOL);
	DDX_Text(pDX, IDC_PROFILE_ICQ, m_sICQ);
	DDX_Text(pDX, IDC_PROFILE_JABBER, m_sJabber);
	DDX_Text(pDX, IDC_PROFILE_TWITTER, m_sTwitter);
	DDX_Text(pDX, IDC_PROFILE_FACEBOOK, m_sFacebook);
	DDX_Text(pDX, IDC_PROFILE_GETENVY, m_sGetEnvy);
}

/////////////////////////////////////////////////////////////////////////////
// CContactProfilePage message handlers

BOOL CContactProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	if ( CXMLElement* pContacts = MyProfile.GetXML( L"contacts" ) )
	{
		for ( POSITION pos1 = pContacts->GetElementIterator() ; pos1 ; )
		{
			CXMLElement* pGroup = pContacts->GetNextElement( pos1 );

			if ( pGroup->IsNamed( L"group" ) )
			{
				CString strGroup = pGroup->GetAttributeValue( L"class" );

				if ( CXMLElement* pAddress = pGroup->GetElementByName( L"address" ) )
				{
					CString strAddress = pAddress->GetAttributeValue( L"content" );

					if ( strGroup.CompareNoCase( L"email" ) == 0 )
						m_sEmail = strAddress;
					else if ( strGroup.CompareNoCase( L"msn" ) == 0 )
						m_sMSN = strAddress;
					else if ( strGroup.CompareNoCase( L"yahoo" ) == 0 )
						m_sYahoo = strAddress;
					else if ( strGroup.CompareNoCase( L"icq" ) == 0 )
						m_sICQ = strAddress;
					else if ( strGroup.CompareNoCase( L"aol" ) == 0 )
						m_sAOL = strAddress;
					else if ( strGroup.CompareNoCase( L"google" ) == 0 )
						m_sJabber = strAddress;
					else if ( strGroup.CompareNoCase( L"jabber" ) == 0 )
						m_sJabber = strAddress;
					else if ( strGroup.CompareNoCase( L"twitter" ) == 0 )
						m_sTwitter = strAddress;
					else if ( strGroup.CompareNoCase( L"facebook" ) == 0 )
						m_sFacebook = strAddress;
					else if ( strGroup.CompareNoCase( L"getenvy.com" ) == 0 )
						m_sGetEnvy = strAddress;
					else if ( strGroup.CompareNoCase( L"envy" ) == 0 )
						m_sGetEnvy = strAddress;
					else
						MsgBox( L"\nUnrecognized contact:  " + strGroup );	// Should never happen
				}
			}
		}
	}

	if ( m_sEmail.Find( L'@' ) < 1 || m_sEmail.Find( L'.' ) < 1 )
		m_sEmail.Empty();

	UpdateData( FALSE );

	return TRUE;
}

void CContactProfilePage::OnPaint()
{
	CPaintDC dc( this );
	CRect rc( 0,0,0,0 );

	GetDlgItem( IDC_PROFILE_EMAIL )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDI_MAIL, 16, rc.left - 20, rc.top + 2 );

	GetDlgItem( IDC_PROFILE_GETENVY )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDR_MAINFRAME, 16, rc.left - 20, rc.top + 2 );

	GetDlgItem( IDC_PROFILE_FACEBOOK )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDI_CONTACT_FACEBOOK, 16, rc.left - 20, rc.top + 2 );

	GetDlgItem( IDC_PROFILE_TWITTER )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDI_CONTACT_TWITTER, 16, rc.left - 20, rc.top + 2 );

	GetDlgItem( IDC_PROFILE_JABBER )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDI_CONTACT_GOOGLE, 16, rc.left - 20, rc.top + 2 );

	GetDlgItem( IDC_PROFILE_MSN )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDI_CONTACT_MSN, 16, rc.left - 20, rc.top + 2 );

	GetDlgItem( IDC_PROFILE_YAHOO )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDI_CONTACT_YAHOO, 16, rc.left - 20, rc.top + 2 );

	GetDlgItem( IDC_PROFILE_AOL )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDI_CONTACT_AOL, 16, rc.left - 20, rc.top + 2 );

	GetDlgItem( IDC_PROFILE_ICQ )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	CoolInterface.Draw( &dc, IDI_CONTACT_ICQ, 16, rc.left - 20, rc.top + 2 );
}

void CContactProfilePage::OnOK()
{
	UpdateData();

	if ( m_sEmail.Find( L'@' ) < 0 || m_sEmail.Find( L'.' ) < 0 )
		m_sEmail.Empty();

	AddAddress( L"Email", L"Primary", m_sEmail );
	AddAddress( L"MSN", L"Primary", m_sMSN );
	AddAddress( L"Yahoo", L"Primary", m_sYahoo );
	AddAddress( L"ICQ", L"Primary", m_sICQ );
	AddAddress( L"AOL", L"Primary", m_sAOL );
	AddAddress( L"Jabber", L"Primary", m_sJabber );
	AddAddress( L"Twitter", L"Primary", m_sTwitter );
	AddAddress( L"Facebook", L"Primary", m_sFacebook );
	AddAddress( L"GetEnvy.com", L"Primary", m_sGetEnvy );
}

void CContactProfilePage::AddAddress(LPCTSTR pszClass, LPCTSTR pszName, LPCTSTR pszAddress)
{
	if ( CXMLElement* pContacts = MyProfile.GetXML( L"contacts", TRUE ) )
	{
		for ( POSITION pos1 = pContacts->GetElementIterator() ; pos1 ; )
		{
			CXMLElement* pGroup = pContacts->GetNextElement( pos1 );

			if ( pGroup->IsNamed( L"group" ) &&
				 pGroup->GetAttributeValue( L"class" ).CompareNoCase( pszClass ) == 0 )
			{
				for ( POSITION pos2 = pGroup->GetElementIterator() ; pos2 ; )
				{
					CXMLElement* pAddress = pGroup->GetNextElement( pos2 );

					if ( pAddress->IsNamed( L"address" ) &&
						 pAddress->GetAttributeValue( L"name" ).CompareNoCase( pszName ) == 0 )
					{
						if ( pszAddress && *pszAddress )
						{
							pAddress->AddAttribute( L"content", pszAddress );
							return;
						}
						else
						{
							pAddress->Delete();
							break;
						}
					}
				}

				if ( pszAddress && *pszAddress )
				{
					CXMLElement* pAddress = pGroup->AddElement( L"address" );
					pAddress->AddAttribute( L"name", pszName );
					pAddress->AddAttribute( L"content", pszAddress );
				}
				else if ( pGroup->GetElementCount() == 0 )
				{
					pGroup->Delete();
				}
				break;
			}
		}

		if ( pszAddress && *pszAddress )
		{
			CXMLElement* pGroup = pContacts->AddElement( L"group" );
			pGroup->AddAttribute( L"class", pszClass );

			CXMLElement* pAddress = pGroup->AddElement( L"address" );
			pAddress->AddAttribute( L"name", pszName );
			pAddress->AddAttribute( L"content", pszAddress );
		}
		else if ( pContacts->GetElementCount() == 0 )
		{
			pContacts->Delete();
		}
	}
}
