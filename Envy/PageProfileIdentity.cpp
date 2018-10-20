//
// PageProfileIdentity.cpp
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
#include "PageProfileIdentity.h"
#include "GProfile.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CIdentityProfilePage, CSettingsPage)

//BEGIN_MESSAGE_MAP(CIdentityProfilePage, CSettingsPage)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIdentityProfilePage property page

CIdentityProfilePage::CIdentityProfilePage()
	: CSettingsPage( CIdentityProfilePage::IDD )
	, m_bBrowseUser	( FALSE )
{
}

CIdentityProfilePage::~CIdentityProfilePage()
{
}

void CIdentityProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROFILE_AGE, m_wndAge);
	DDX_CBString(pDX, IDC_PROFILE_AGE, m_sAge);
	DDX_CBString(pDX, IDC_PROFILE_GENDER, m_sGender);
	DDX_Text(pDX, IDC_PROFILE_NICK, m_sNick);
	DDX_Text(pDX, IDC_PROFILE_FIRST, m_sFirst);
	DDX_Text(pDX, IDC_PROFILE_LAST, m_sLast);
	DDX_Check(pDX, IDC_BROWSE_USER, m_bBrowseUser);
}

/////////////////////////////////////////////////////////////////////////////
// CIdentityProfilePage message handlers

BOOL CIdentityProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	if ( CXMLElement* pIdentity = MyProfile.GetXML( L"identity" ) )
	{
		if ( CXMLElement* pHandle = pIdentity->GetElementByName( L"handle" ) )
			m_sNick  = pHandle->GetAttributeValue( L"primary" );

		if ( CXMLElement* pName = pIdentity->GetElementByName( L"name" ) )
		{
			m_sFirst = pName->GetAttributeValue( L"first" );
			m_sLast  = pName->GetAttributeValue( L"last" );
		}
	}

	if ( CXMLElement* pVitals = MyProfile.GetXML( L"vitals" ) )
	{
		m_sAge = pVitals->GetAttributeValue( L"age" );

		CString strGenderMale, strGenderFemale;
		GetGenderTranslations( strGenderMale, strGenderFemale );

		m_sGender = pVitals->GetAttributeValue( L"gender" );

		if ( ! m_sGender.IsEmpty() )
		{
			CComboBox* pGender = (CComboBox*) GetDlgItem( IDC_PROFILE_GENDER );
			if ( m_sGender.CompareNoCase( L"male" ) == 0 )
				pGender->SelectString( -1, (LPCTSTR)strGenderMale );
			else if ( m_sGender.CompareNoCase( L"female" ) == 0 )
				pGender->SelectString( -1, (LPCTSTR)strGenderFemale );
			else
				m_sGender.Empty();
		}

		int nAge = 0;

		if ( _stscanf( m_sAge, L"%i", &nAge ) == 1 && nAge > 0 )
			m_sAge.Format( L"%i", nAge );
		else
			m_sAge.Empty();
	}

	CString str;
	for ( int nAge = 10; nAge < 91; nAge++ )
	{
		str.Format( L"%i", nAge );
		m_wndAge.AddString( str );
	}

	m_bBrowseUser = Settings.Community.ServeProfile;

	UpdateData( FALSE );

	return TRUE;
}

void CIdentityProfilePage::OnOK()
{
	UpdateData();

	Settings.Community.ServeProfile = m_bBrowseUser != FALSE;

	if ( CXMLElement* pIdentity = MyProfile.GetXML( L"identity", TRUE ) )
	{
		if ( CXMLElement* pHandle = pIdentity->GetElementByName( L"handle", TRUE ) )
		{
			pHandle->AddAttribute( L"primary", m_sNick );
			if ( m_sNick.IsEmpty() )
				pHandle->Delete();
		}

		if ( CXMLElement* pName = pIdentity->GetElementByName( L"name", TRUE ) )
		{
			pName->AddAttribute( L"first", m_sFirst );
			pName->AddAttribute( L"last", m_sLast );
			if ( m_sFirst.IsEmpty() && m_sLast.IsEmpty() )
				pName->Delete();
		}

		if ( pIdentity->GetElementCount() == 0 )
			pIdentity->Delete();
	}

	if ( CXMLElement* pVitals = MyProfile.GetXML( L"vitals", TRUE ) )
	{
		int nAge = 0;

		if ( _stscanf( m_sAge, L"%i", &nAge ) == 1 && nAge > 0 )
			m_sAge.Format( L"%i", nAge );
		else
			m_sAge.Empty();

		CString strGenderMale, strGenderFemale;
		GetGenderTranslations( strGenderMale, strGenderFemale );

		if ( m_sGender.CompareNoCase( strGenderMale ) == 0 || m_sGender.CompareNoCase( L"male" ) == 0 )
			pVitals->AddAttribute( L"gender", L"Male" );
		else if ( m_sGender.CompareNoCase( strGenderFemale ) == 0 || m_sGender.CompareNoCase( L"female" ) == 0 )
			pVitals->AddAttribute( L"gender", L"Female" );
		else
			pVitals->DeleteAttribute( L"gender" );

		if ( ! m_sAge.IsEmpty() )
			pVitals->AddAttribute( L"age", m_sAge );
		else
			pVitals->DeleteAttribute( L"age" );

		if ( pVitals->GetElementCount() == 0 &&
			 pVitals->GetAttributeCount() == 0 )
			pVitals->Delete();
	}
}

void CIdentityProfilePage::GetGenderTranslations(CString& pMale, CString& pFemale)
{
	// Using data from CBrowseHostProfile.1 translation since
	// the control in the dialog may change its order and it does not have its identifier.

	BOOL bCollected = FALSE;

	CXMLElement* pXML = Skin.GetDocument( L"CBrowseHostProfile.1" );

	for ( POSITION posGroup = pXML->GetElementIterator(); posGroup && ! bCollected; )
	{
		CXMLElement* pGroups = pXML->GetNextElement( posGroup );

		if ( pGroups->IsNamed( L"group" ) && pGroups->GetAttributeValue( L"id" ) == "3" )
		{
			for ( POSITION posText = pGroups->GetElementIterator(); posText && ! bCollected; )
			{
				CXMLElement* pText = pGroups->GetNextElement( posText );

				if ( pText->IsNamed( L"text" ) )
				{
					CString strTemp;
					strTemp = pText->GetAttributeValue( L"id" );
					if ( strTemp.CompareNoCase( L"gendermale" ) == 0 )
					{
						pMale = pText->GetValue();
						bCollected = pFemale.IsEmpty() ? FALSE : TRUE;
					}
					else if ( strTemp.CompareNoCase( L"genderfemale" ) == 0 )
					{
						pFemale = pText->GetValue();
						bCollected = pMale.IsEmpty() ? FALSE : TRUE;
					}
				}
			}
		}
	}
}
