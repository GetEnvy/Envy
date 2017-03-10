//
// WizardProfilePage.cpp
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
#include "WizardProfilePage.h"
#include "GProfile.h"
#include "Network.h"
#include "WorldGPS.h"
#include "Flags.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CWizardProfilePage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardProfilePage, CWizardPage)
	ON_WM_XBUTTONDOWN()
	ON_CBN_SELCHANGE(IDC_LOC_COUNTRY, &CWizardProfilePage::OnSelChangeCountry)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardProfilePage property page

CWizardProfilePage::CWizardProfilePage()
	: CWizardPage(CWizardProfilePage::IDD)
	, m_nAge	( 0 )
	, m_nGender	( 0 )
	, m_pWorld	( NULL )
{
}

CWizardProfilePage::~CWizardProfilePage()
{
	delete m_pWorld;
}

void CWizardProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROFILE_NICK, m_sNick);
	DDX_Control(pDX, IDC_LOC_CITY, m_wndCity);
	DDX_Control(pDX, IDC_LOC_COUNTRY, m_wndCountry);
	DDX_CBString(pDX, IDC_LOC_CITY, m_sLocCity);
	DDX_CBString(pDX, IDC_LOC_COUNTRY, m_sLocCountry);
	DDX_Control(pDX, IDC_PROFILE_AGE, m_wndAge);
	DDX_CBIndex(pDX, IDC_PROFILE_AGE, m_nAge);
	DDX_CBIndex(pDX, IDC_PROFILE_GENDER, m_nGender);
	DDX_Control(pDX, IDC_PROFILE_BIO, m_wndComments);
}

/////////////////////////////////////////////////////////////////////////////
// CWizardProfilePage message handlers

BOOL CWizardProfilePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	Skin.Apply( L"CWizardProfilePage", this );

	CString str, strYearsOld;
	LoadString( strYearsOld, IDS_WIZARD_YEARS_OLD );

	for ( int nAge = 13 ; nAge < 91 ; nAge++ )
	{
		str.Format( L" %i " + strYearsOld, nAge );
		m_wndAge.SetItemData( m_wndAge.AddString( str ), nAge );
	}

	if ( CXMLElement* pNotes = MyProfile.GetXML( L"notes" ) )
		m_wndComments.SetWindowText( pNotes->GetValue() );

	const int nFlags = Flags.GetCount();
	VERIFY( m_gdiFlags.Create( Flags.Width, 16, ILC_COLOR32|ILC_MASK, nFlags, 0 ) ||
			m_gdiFlags.Create( Flags.Width, 16, ILC_COLOR24|ILC_MASK, nFlags, 0 ) ||
			m_gdiFlags.Create( Flags.Width, 16, ILC_COLOR16|ILC_MASK, nFlags, 0 ) );
	for ( int nFlag = 0 ; nFlag < nFlags ; nFlag++ )
	{
		if ( HICON hIcon = Flags.ExtractIcon( nFlag ) )
		{
			VERIFY( m_gdiFlags.Add( hIcon ) != -1 );
			VERIFY( DestroyIcon( hIcon ) );
		}
	}
	m_wndCountry.SetImageList( &m_gdiFlags );

	m_pWorld = new CWorldGPS();
	m_pWorld->Load();

	const CWorldCountry* pCountry = m_pWorld->m_pCountry;

	int nSelect = -1;
	for ( UINT nCountry = 0 ; nCountry < m_pWorld->m_nCountry ; nCountry++, pCountry++ )
	{
	//	m_wndCountry.SetItemData( m_wndCountry.AddString( pCountry->m_sName ), (LPARAM)pCountry );

		const int nImage = Flags.GetFlagIndex( CString( pCountry->m_szID, 2 ) );
		const COMBOBOXEXITEM cbei =
		{
			CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM | CBEIF_TEXT,
			nCountry,
			(LPTSTR)(LPCTSTR)pCountry->m_sName,
			pCountry->m_sName.GetLength() + 1,
			nImage,
			nImage,
			0,
			0,
			(LPARAM)pCountry
		};
		m_wndCountry.InsertItem( &cbei );
		if ( pCountry->m_sName.CompareNoCase( m_sLocCountry ) == 0 )
			nSelect = nCountry;
	}
	m_wndCountry.SetCurSel( nSelect );

//	OnSelChangeCountry();
//	RecalcDropWidth( &m_wndCountry );	// Not ComboBoxEx

	UpdateData( FALSE );

	return TRUE;
}

BOOL CWizardProfilePage::OnSetActive()
{
	// Wizard Window Caption Workaround
	CString strCaption;
	GetWindowText( strCaption );
	GetParent()->SetWindowText( strCaption );

	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	m_sNick = MyProfile.GetNick();

	if ( m_sNick.IsEmpty() )
	{
		TCHAR pBuffer[64];
		DWORD nSize = 64;
		if ( GetUserNameW( pBuffer, &nSize ) )
			m_sNick = pBuffer;
	}

	if ( CXMLElement* pVitals = MyProfile.GetXML( L"vitals" ) )
	{
		CString strGender	= pVitals->GetAttributeValue( L"gender" );
		CString strAge		= pVitals->GetAttributeValue( L"age" );

		if ( strGender.CompareNoCase( L"male" ) == 0 )
			m_nGender = 1;
		else if ( strGender.CompareNoCase( L"female" ) == 0 )
			m_nGender = 2;

		int nAge = 0;
		_stscanf( strAge, L"%i", &nAge );

		for ( int nAgeItem = 0 ; nAgeItem < m_wndAge.GetCount() ; nAgeItem ++ )
		{
			if ( m_wndAge.GetItemData( nAgeItem ) == (DWORD)nAge )
			{
				m_nAge = nAgeItem;
				break;
			}
		}
	}

	if ( CXMLElement* pLocation = MyProfile.GetXML( L"location" ) )
	{
		if ( CXMLElement* pPolitical = pLocation->GetElementByName( L"political" ) )
		{
			m_sLocCountry	= pPolitical->GetAttributeValue( L"country" );
			m_sLocCity		= pPolitical->GetAttributeValue( L"city" ) + L", "
							+ pPolitical->GetAttributeValue( L"state" );
		}
	}

	if ( m_sLocCountry.IsEmpty() && Network.m_pHost.sin_addr.S_un.S_addr )
		m_sLocCountry = theApp.GetCountryName( Network.m_pHost.sin_addr );

	CString strTest;
	for ( UINT nCountry = 0 ; nCountry < m_pWorld->m_nCountry ; nCountry++ )
	{
		m_wndCountry.GetLBText( nCountry, strTest );
		if ( strTest.CompareNoCase( m_sLocCountry ) == 0 )
		{
			m_wndCountry.SetCurSel( nCountry );
			break;
		}
	}

	OnSelChangeCountry();

	UpdateData( FALSE );

	return CWizardPage::OnSetActive();
}

void CWizardProfilePage::OnSelChangeCountry()
{
	CWaitCursor pCursor;

	int nSel = m_wndCountry.GetCurSel();
	if ( nSel < 0 ) return;
	CWorldCountry* pCountry = (CWorldCountry*)m_wndCountry.GetItemData( nSel );
	if ( ! pCountry ) return;

	if ( m_wndCity.GetCount() )
		m_wndCity.ResetContent();

	CWorldCity* pCity = pCountry->m_pCity;
	CString strCity;

	for ( int nCity = pCountry->m_nCity ; nCity ; nCity--, pCity++ )
	{
		if ( ! pCity->m_sName.IsEmpty() && ! pCity->m_sState.IsEmpty() )
			strCity = pCity->m_sName + L", " + pCity->m_sState;
		else if ( ! pCity->m_sName.IsEmpty() )
			strCity = pCity->m_sName;
		else if ( ! pCity->m_sState.IsEmpty() )
			strCity = pCity->m_sState;
		else
			continue;

		m_wndCity.SetItemData( m_wndCity.AddString( strCity ), (LPARAM)pCity );
	}
	RecalcDropWidth( &m_wndCity );

//	UpdateData();
}

void CWizardProfilePage::OnXButtonDown(UINT /*nFlags*/, UINT nButton, CPoint /*point*/)
{
	if ( nButton == 1 )
		GetSheet()->PressButton( PSBTN_BACK );
	else if ( nButton == 2 )
		GetSheet()->PressButton( PSBTN_NEXT );
}

LRESULT CWizardProfilePage::OnWizardBack()
{
	return CWizardPage::OnWizardBack();
}

LRESULT CWizardProfilePage::OnWizardNext()
{
	UpdateData( TRUE );

	if ( CXMLElement* pIdentity = MyProfile.GetXML( L"identity", TRUE ) )
	{
		if ( CXMLElement* pHandle = pIdentity->GetElementByName( L"handle", TRUE ) )
		{
			pHandle->AddAttribute( L"primary", m_sNick );
			if ( m_sNick.IsEmpty() )
				pHandle->Delete();
			else if ( Settings.Remote.Username.IsEmpty() )
				Settings.Remote.Username = m_sNick;
		}
	}

	if ( CXMLElement* pNotes = MyProfile.GetXML( L"notes", TRUE ) )
	{
		CString str;
		m_wndComments.GetWindowText( str );

		if ( ! str.IsEmpty() )
			pNotes->SetValue( str );
		//else
		//	pNotes->Delete();
	}

	if ( CXMLElement* pVitals = MyProfile.GetXML( L"vitals", TRUE ) )
	{
		if ( m_nAge < 10 )
		{
			pVitals->DeleteAttribute( L"age" );
		}
		else
		{
			CString strAge;
			strAge.Format( L"%i", m_wndAge.GetItemData( m_nAge ) );
			pVitals->AddAttribute( L"age", strAge );
		}

		if ( m_nGender == 1 || m_nGender == 3 )
			pVitals->AddAttribute( L"gender", L"male" );
		else if ( m_nGender == 2 || m_nGender == 4 )
			pVitals->AddAttribute( L"gender", L"female" );
		else
			pVitals->DeleteAttribute( L"gender" );

		if ( pVitals->GetElementCount() == 0 &&
			 pVitals->GetAttributeCount() == 0 )
			pVitals->Delete();
	}

	if ( CXMLElement* pLocation = MyProfile.GetXML( L"location", TRUE ) )
	{
		if ( CXMLElement* pPolitical = pLocation->GetElementByName( L"political", TRUE ) )
		{
			if ( ! m_sLocCountry.IsEmpty() )
				pPolitical->AddAttribute( L"country", m_sLocCountry );
			else
				pPolitical->DeleteAttribute( L"country" );

			int nPos = m_sLocCity.Find( L", " );

			if ( nPos >= 0 )
			{
				pPolitical->AddAttribute( L"city", m_sLocCity.Left( nPos ) );
				pPolitical->AddAttribute( L"state", m_sLocCity.Mid( nPos + 2 ) );
			}
			else if ( ! m_sLocCity.IsEmpty() )
			{
				pPolitical->AddAttribute( L"city", m_sLocCity );
				pPolitical->DeleteAttribute( L"state" );
			}
			else
			{
				pPolitical->DeleteAttribute( L"city" );
				pPolitical->DeleteAttribute( L"state" );
			}

			if ( pPolitical->GetElementCount() == 0 && pPolitical->GetAttributeCount() == 0 )
				pPolitical->Delete();
		}

		if ( CXMLElement* pCoordinates = pLocation->GetElementByName( L"coordinates", TRUE ) )
		{
			int nSel = m_wndCity.GetCurSel();
			CWorldCity* pCity = (CWorldCity*)m_wndCity.GetItemData( m_wndCity.GetCurSel() );

			if ( nSel >= 0 && pCity != NULL )
			{
				CString strValue;

				strValue.Format( L"%f", pCity->m_nLatitude );
				pCoordinates->AddAttribute( L"latitude", strValue );

				strValue.Format( L"%f", pCity->m_nLongitude );
				pCoordinates->AddAttribute( L"longitude", strValue );
			}
			else
			{
				pCoordinates->Delete();
			}
		}

		if ( pLocation->GetElementCount() == 0 )
			pLocation->Delete();
	}

	// Annoying popups.  This information is underutilized, do not put so much emphasis.

	//if ( MyProfile.GetNick().IsEmpty() )
	//{
	//	MsgBox( IDS_PROFILE_NO_NICK, MB_ICONEXCLAMATION );
	//	return -1;
	//}
	//if ( MyProfile.GetXML( L"vitals" ) == NULL )
	//{
	//	if ( MsgBox( IDS_PROFILE_NO_VITALS, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES ) return -1;
	//}
	//if ( MyProfile.GetLocation().IsEmpty() )
	//{
	//	if ( MsgBox( IDS_PROFILE_NO_LOCATION, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES ) return -1;
	//}

	MyProfile.Save();

	return 0;
}
