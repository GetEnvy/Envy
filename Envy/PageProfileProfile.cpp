//
// PageProfileProfile.cpp
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
#include "Settings.h"
#include "Envy.h"
#include "PageProfileProfile.h"
#include "GProfile.h"
#include "WorldGPS.h"
#include "Flags.h"
#include "Buffer.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CProfileProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CProfileProfilePage, CSettingsPage)
	ON_CBN_SELCHANGE(IDC_LOC_COUNTRY, &CProfileProfilePage::OnSelChangeCountry)
	ON_CBN_SELCHANGE(IDC_LOC_CITY, &CProfileProfilePage::OnSelChangeCity)
	ON_LBN_SELCHANGE(IDC_INTEREST_LIST, &CProfileProfilePage::OnSelChangeInterestList)
	ON_CBN_SELCHANGE(IDC_INTEREST_ALL, &CProfileProfilePage::OnSelChangeInterestAll)
	ON_CBN_EDITCHANGE(IDC_INTEREST_ALL, &CProfileProfilePage::OnEditChangeInterestAll)
	ON_BN_CLICKED(IDC_INTEREST_ADD, &CProfileProfilePage::OnInterestAdd)
	ON_BN_CLICKED(IDC_INTEREST_REMOVE, &CProfileProfilePage::OnInterestRemove)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProfileProfilePage property page

CProfileProfilePage::CProfileProfilePage()
	: CSettingsPage	( CProfileProfilePage::IDD )
	, m_pWorld		( NULL )
{
}

CProfileProfilePage::~CProfileProfilePage()
{
	if ( m_pWorld ) delete m_pWorld;
}

void CProfileProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INTEREST_LIST, m_wndInterestList);
	DDX_Control(pDX, IDC_INTEREST_ALL, m_wndInterestAll);
	DDX_Control(pDX, IDC_INTEREST_ADD, m_wndInterestAdd);
	DDX_Control(pDX, IDC_INTEREST_REMOVE, m_wndInterestRemove);
	DDX_Control(pDX, IDC_LOC_CITY, m_wndCity);
	DDX_Control(pDX, IDC_LOC_COUNTRY, m_wndCountry);
	DDX_CBString(pDX, IDC_LOC_CITY, m_sLocCity);
	DDX_CBString(pDX, IDC_LOC_COUNTRY, m_sLocCountry);
	DDX_Text(pDX, IDC_LOC_LAT, m_sLocLatitude);
	DDX_Text(pDX, IDC_LOC_LONG, m_sLocLongitude);
}

/////////////////////////////////////////////////////////////////////////////
// CProfileProfilePage message handlers

BOOL CProfileProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	if ( CXMLElement* pLocation = MyProfile.GetXML( L"location" ) )
	{
		if ( CXMLElement* pPolitical = pLocation->GetElementByName( L"political" ) )
		{
			m_sLocCountry	= pPolitical->GetAttributeValue( L"country" );
			m_sLocCity		= pPolitical->GetAttributeValue( L"city" );
			CString str		= pPolitical->GetAttributeValue( L"state" );
			if ( ! str.IsEmpty() && ! m_sLocCity.IsEmpty() )
				m_sLocCity += L", ";
			m_sLocCity += str;
		}

		if ( CXMLElement* pCoordinates = pLocation->GetElementByName( L"coordinates" ) )
		{
			float nValue;
			CString str = pCoordinates->GetAttributeValue( L"latitude" );

			if ( _stscanf( str, L"%f", &nValue ) == 1 )
				m_sLocLatitude.Format( nValue >= 0 ? L"%.2f\xb0 N" : L"%.2f\xb0 S", double( fabs( nValue ) ) );

			str = pCoordinates->GetAttributeValue( L"longitude" );

			if ( _stscanf( str, L"%f", &nValue ) == 1 )
				m_sLocLongitude.Format( nValue >= 0 ? L"%.1f\xb0 E" : L"%.1f\xb0 W", double( fabs( nValue ) ) );
		}
	}

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

	if ( CXMLElement* pInterests = MyProfile.GetXML( L"interests" ) )
	{
		for ( POSITION pos = pInterests->GetElementIterator() ; pos ; )
		{
			CXMLElement* pInterest = pInterests->GetNextElement( pos );

			if ( pInterest->IsNamed( L"interest" ) )
				m_wndInterestList.AddString( pInterest->GetValue() );
		}
	}

	LoadDefaultInterests();

	UpdateData( FALSE );

	OnSelChangeCountry();
//	RecalcDropWidth( &m_wndCountry );	// Not ComboBoxEx

	m_wndInterestAdd.EnableWindow( FALSE );
	m_wndInterestRemove.EnableWindow( FALSE );

	return TRUE;
}

int CProfileProfilePage::LoadDefaultInterests()
{
	int nCount = 0;
	const CString strFile = Settings.General.DataPath + L"Interests.dat";

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeRead ) )
		return nCount;

	try
	{
		CString strLine;
		CString strLang = L" " + Settings.General.Language;
		CBuffer pBuffer;

		pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
		pBuffer.m_nLength = (DWORD)pFile.GetLength();
		pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
		pFile.Close();

		// Format: Delineated List, enabled by prespecified #languages:	#start en ... #end en
		// (Allows multiple/nested/overlapped languages, all applicable results displayed alphabetically)

		BOOL bActive = FALSE;

		while ( pBuffer.ReadLine( strLine ) )
		{
			if ( strLine.GetLength() < 2 ) continue;		// Blank line

			if ( strLine.GetAt( 0 ) == '#' )				// Language start/end line
			{
				if ( strLine.Find( strLang, 4 ) < 1 && strLine.Find( L" all", 4 ) < 1 )
				{
					if ( strLine.Left( 10 ) == L"#languages" )
						strLang = L" en";
				}
				else if ( strLine.Left( 6 ) == L"#start" || strLine.Left( 6 ) == L"#begin" )
					bActive = TRUE;
				else if ( strLine.Left( 4 ) == L"#end" )
					bActive = FALSE;	//break;

				continue;
			}

			if ( ! bActive ) continue;						// Disinterested language

			if ( strLine.Find( L"\t" ) > 0 ) 			// Trim at whitespace (remove any comments)
				strLine.Left( strLine.Find( L"\t" ) );

			nCount++;
			m_wndInterestAll.AddString( strLine );
		}
	}
	catch ( CException* pException )
	{
		if ( pFile.m_hFile != CFile::hFileNull )
			pFile.Close();	// File is still open so close it
		pException->Delete();
	}

	return nCount;
}

void CProfileProfilePage::OnSelChangeCountry()
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
			strCity = pCity->m_sName + L", " + pCity->m_sState;		// ToDo: Sort by state?
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

void CProfileProfilePage::OnSelChangeCity()
{
	int nSel = m_wndCity.GetCurSel();
	if ( nSel < 0 ) return;
	CWorldCity* pCity = (CWorldCity*)m_wndCity.GetItemData( nSel );
	if ( ! pCity ) return;

	m_sLocLatitude.Format( pCity->m_nLatitude >= 0 ? L"%.2f° N" : L"%.2f° S",
		double( fabs( pCity->m_nLatitude ) ) );

	m_sLocLongitude.Format( pCity->m_nLongitude >= 0 ? L"%.1f° E" : L"%.1f° W",
		double( fabs( pCity->m_nLongitude ) ) );

	GetDlgItem( IDC_LOC_LAT )->SetWindowText( m_sLocLatitude );
	GetDlgItem( IDC_LOC_LONG )->SetWindowText( m_sLocLongitude );
}

void CProfileProfilePage::OnSelChangeInterestList()
{
	m_wndInterestRemove.EnableWindow( m_wndInterestList.GetCurSel() >= 0 );
}

void CProfileProfilePage::OnSelChangeInterestAll()
{
	m_wndInterestAdd.EnableWindow( m_wndInterestAll.GetCurSel() >= 0 || m_wndInterestAll.GetWindowTextLength() > 0 );
}

void CProfileProfilePage::OnEditChangeInterestAll()
{
	m_wndInterestAdd.EnableWindow( m_wndInterestAll.GetCurSel() >= 0 || m_wndInterestAll.GetWindowTextLength() > 0 );
}

void CProfileProfilePage::OnInterestAdd()
{
	CString str;
	m_wndInterestAll.GetWindowText( str );
	m_wndInterestList.AddString( str );
}

void CProfileProfilePage::OnInterestRemove()
{
	int nItem = m_wndInterestList.GetCurSel();
	if ( nItem >= 0 ) m_wndInterestList.DeleteString( nItem );
	m_wndInterestRemove.EnableWindow( FALSE );
}

void CProfileProfilePage::OnOK()
{
	UpdateData();

	if ( CXMLElement* pLocation = MyProfile.GetXML( L"location", TRUE ) )
	{
		if ( CXMLElement* pPolitical = pLocation->GetElementByName( L"political", TRUE ) )
		{
			if ( ! m_sLocCountry.IsEmpty() )
			{
				pPolitical->AddAttribute( L"country", m_sLocCountry );
			}
			else if ( CXMLAttribute* pAttr = pPolitical->GetAttribute( L"country" ) )
			{
				pAttr->Delete();
			}

			int nPos = m_sLocCity.Find( L", " );

			if ( nPos >= 0 )
			{
				pPolitical->AddAttribute( L"city", m_sLocCity.Left( nPos ) );
				pPolitical->AddAttribute( L"state", m_sLocCity.Mid( nPos + 2 ) );
			}
			else if ( ! m_sLocCity.IsEmpty() )
			{
				pPolitical->AddAttribute( L"city", m_sLocCity );
				if ( CXMLAttribute* pAttr = pPolitical->GetAttribute( L"state" ) )
					pAttr->Delete();
			}
			else
			{
				if ( CXMLAttribute* pAttr = pPolitical->GetAttribute( L"city" ) )
					pAttr->Delete();
				if ( CXMLAttribute* pAttr = pPolitical->GetAttribute( L"state" ) )
					pAttr->Delete();
			}

			if ( pPolitical->GetElementCount() == 0 && pPolitical->GetAttributeCount() == 0 )
				pPolitical->Delete();
		}

		if ( CXMLElement* pCoordinates = pLocation->GetElementByName( L"coordinates", TRUE ) )
		{
			CString strValue;
			float nValue = 0;

			if ( _stscanf( m_sLocLatitude, L"%f", &nValue ) == 1 )
			{
				if ( m_sLocLatitude.Find( L'S' ) >= 0 ) nValue *= -1;
				strValue.Format( L"%f", double( nValue ) );
				pCoordinates->AddAttribute( L"latitude", strValue );
			}

			if ( _stscanf( m_sLocLongitude, L"%f", &nValue ) == 1 )
			{
				if ( m_sLocLongitude.Find( L'W' ) >= 0 ) nValue *= -1;
				strValue.Format( L"%f", double( nValue ) );
				pCoordinates->AddAttribute( L"longitude", strValue );
			}

			if ( nValue == 0 )
				pCoordinates->Delete();
		}

		if ( pLocation->GetElementCount() == 0 )
			pLocation->Delete();
	}

	if ( CXMLElement* pInterests = MyProfile.GetXML( L"interests", TRUE ) )
	{
		pInterests->DeleteAllElements();

		for ( int nItem = 0 ; nItem < m_wndInterestList.GetCount() ; nItem++ )
		{
			CString str;
			m_wndInterestList.GetText( nItem, str );
			CXMLElement* pInterest = pInterests->AddElement( L"interest" );
			pInterest->SetValue( str );
		}

		if ( pInterests->GetElementCount() == 0 )
			pInterests->Delete();
	}

	CSettingsPage::OnOK();
}
