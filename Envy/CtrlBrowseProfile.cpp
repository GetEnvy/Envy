//
// CtrlBrowseProfile.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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
#include "CtrlBrowseProfile.h"
#include "GProfile.h"
#include "G2Packet.h"
#include "HostBrowser.h"

#include "RichElement.h"
#include "RichDocument.h"
#include "Colors.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CBrowseProfileCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_NOTIFY(RVN_CLICK, IDC_BROWSE_PROFILE, OnClickView)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl construction

CBrowseProfileCtrl::CBrowseProfileCtrl()
	: m_pDocument1 ( NULL )
	, m_pDocument2 ( NULL )
{
}

CBrowseProfileCtrl::~CBrowseProfileCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl operations

BOOL CBrowseProfileCtrl::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::Create( NULL, NULL, WS_CHILD,
		rect, pParentWnd, IDC_BROWSE_PROFILE, NULL );
}

void CBrowseProfileCtrl::OnSkinChange()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pDocument1 != NULL )
		m_pDocument1->Clear();
	else
		m_pDocument1 = new CRichDocument();

	m_pdNick = m_pdFullName = m_pdFullLocation = NULL;
	m_pdGenderMale = m_pdGenderFemale = m_pdAge = NULL;
	m_pdContactEmail = m_pdContactMSN = m_pdContactYahoo = m_pdContactICQ = m_pdContactAOL = m_pdContactJabber = NULL;
	m_pdBioText = m_pdInterests = NULL;

	if ( CXMLElement* pXML = Skin.GetDocument( L"CBrowseHostProfile.1" ) )
	{
		CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;
		m_pDocument1->LoadXML( pXML, &pMap );

	//	pMap.Lookup( L"Vendor", m_pdVendor );
		pMap.Lookup( L"Nick", m_pdNick );
		pMap.Lookup( L"FullName", m_pdFullName );
		pMap.Lookup( L"FullLocation", m_pdFullLocation );
		pMap.Lookup( L"GenderMale", m_pdGenderMale );
		pMap.Lookup( L"GenderFemale", m_pdGenderFemale );
		pMap.Lookup( L"Age", m_pdAge );
		pMap.Lookup( L"BioText", m_pdBioText );
		pMap.Lookup( L"Interests", m_pdInterests );
		pMap.Lookup( L"ContactEmail", m_pdContactEmail );
		pMap.Lookup( L"ContactMSN", m_pdContactMSN );
		pMap.Lookup( L"ContactYahoo", m_pdContactYahoo );
		pMap.Lookup( L"ContactAOL", m_pdContactAOL );
		pMap.Lookup( L"ContactICQ", m_pdContactICQ );
		pMap.Lookup( L"ContactJabber", m_pdContactJabber );
		pMap.Lookup( L"ContactTwitter", m_pdContactTwitter );
		pMap.Lookup( L"ContactFacebook", m_pdContactFacebook );
		pMap.Lookup( L"ContactGetEnvy", m_pdContactGetEnvy );
	}

	if ( m_pDocument2 != NULL )
		m_pDocument2->Clear();
	else
		m_pDocument2 = new CRichDocument();

	m_pdBookmarks = NULL;

	if ( CXMLElement* pXML = Skin.GetDocument( L"CBrowseHostProfile.2" ) )
	{
		CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;
		m_pDocument2->LoadXML( pXML, &pMap );

		pMap.Lookup( L"Bookmarks", m_pdBookmarks );
	}

	m_wndDoc1.SetDocument( m_pDocument1 );
	m_wndDoc2.SetDocument( m_pDocument2 );
}

void CBrowseProfileCtrl::Update(CHostBrowser* pBrowser)
{
	CSingleLock pLock( &m_pSection, TRUE );

	CGProfile* pProfile = pBrowser->m_pProfile;

	if ( m_pDocument1 == NULL || m_pDocument2 == NULL ) return;
	if ( pProfile == NULL || pProfile->IsValid() == FALSE ) return;

	if ( ! pBrowser->IsBrowsing() && pBrowser->m_nHits > 0 && ! m_imgHead.m_bLoaded )
		LoadDefaultHead();

	UpdateDocument1( pProfile );
	UpdateDocument2( pBrowser );

	if ( m_wndDoc1.IsWindowVisible() == FALSE ) m_wndDoc1.ShowWindow( SW_SHOW );
	if ( m_wndDoc2.IsWindowVisible() == FALSE ) m_wndDoc2.ShowWindow( SW_SHOW );
}

void CBrowseProfileCtrl::UpdateDocument1(CGProfile* pProfile)
{
	CSingleLock pLock( &m_pDocument1->m_pSection, TRUE );

	CString str;

	if ( m_pdNick != NULL )
		m_pdNick->SetText( pProfile->GetNick() );

	if ( CXMLElement* pIdentity = pProfile->GetXML( L"identity" ) )
	{
		if ( CXMLElement* pName = pIdentity->GetElementByName( L"name" ) )
		{
			CString strFirst = pName->GetAttributeValue( L"first" );
			CString strLast  = pName->GetAttributeValue( L"last" );

			if ( m_pdFullName != NULL && ( ! strFirst.IsEmpty() || ! strLast.IsEmpty() ) )
			{
				m_pDocument1->ShowGroup( 1, TRUE );
				m_pdFullName->SetText( strFirst + L' ' + strLast );
			}
			else
			{
				m_pDocument1->ShowGroup( 1, FALSE );
			}
		}
		else
		{
			m_pDocument1->ShowGroup( 1, FALSE );
		}
	}
	else
	{
		m_pDocument1->ShowGroup( 1, FALSE );
	}

	str = pProfile->GetLocation();
	m_pDocument1->ShowGroup( 2, ! str.IsEmpty() );
	if ( m_pdFullLocation != NULL )
		m_pdFullLocation->SetText( str );

	if ( CXMLElement* pVitals = pProfile->GetXML( L"vitals" ) )
	{
		str = pVitals->GetAttributeValue( L"gender" );
		m_pDocument1->ShowGroup( 3, ! str.IsEmpty() );

		if ( m_pdGenderMale != NULL && m_pdGenderFemale != NULL )
		{
			m_pdGenderMale->Show( str.CompareNoCase( L"male" ) == 0 );
			m_pdGenderFemale->Show( str.CompareNoCase( L"female" ) == 0 );
		}

		str = pVitals->GetAttributeValue( L"age" );
		if ( m_pdAge != NULL ) m_pdAge->SetText( str );
	}
	else
	{
		m_pDocument1->ShowGroup( 3, FALSE );
	}

	BOOL bContact = FALSE;

	str = pProfile->GetContact( L"Email" );
	bContact |= ( ! str.IsEmpty() );
	m_pDocument1->ShowGroup( 40, ! str.IsEmpty() );
	if ( m_pdContactEmail != NULL )
	{
		m_pdContactEmail->SetText( str );
		m_pdContactEmail->m_sLink = L"mailto:" + str;
	}

	str = pProfile->GetContact( L"MSN" );
	bContact |= ( ! str.IsEmpty() );
	m_pDocument1->ShowGroup( 44, ! str.IsEmpty() );
	if ( m_pdContactMSN != NULL )
	{
		m_pdContactMSN->SetText( str );
		m_pdContactMSN->m_sLink = L"msn:" + str;
	}

	str = pProfile->GetContact( L"Yahoo" );
	bContact |= ( ! str.IsEmpty() );
	m_pDocument1->ShowGroup( 41, ! str.IsEmpty() );
	if ( m_pdContactYahoo != NULL )
	{
		m_pdContactYahoo->SetText( str );
		m_pdContactYahoo->m_sLink = L"ymsgr:sendim?" + str;
	}

	str = pProfile->GetContact( L"AOL" );
	bContact |= ( ! str.IsEmpty() );
	m_pDocument1->ShowGroup( 43, ! str.IsEmpty() );
	if ( m_pdContactAOL != NULL )
	{
		m_pdContactAOL->SetText( str );
		m_pdContactAOL->m_sLink = L"aim:goim?screenname=" + str;
	}

	str = pProfile->GetContact( L"ICQ" );
	bContact |= ( ! str.IsEmpty() );
	m_pDocument1->ShowGroup( 42, ! str.IsEmpty() );
	if ( m_pdContactICQ != NULL )
	{
		m_pdContactICQ->SetText( str );
		m_pdContactICQ->m_sLink = L"http://people.icq.com/people/about_me.php?uin=" + str;
	}

	str = pProfile->GetContact( L"Google" );
	if ( str.IsEmpty() ) str = pProfile->GetContact( L"Jabber" );
	bContact |= ( ! str.IsEmpty() );
	m_pDocument1->ShowGroup( 45, ! str.IsEmpty() );
	if ( m_pdContactJabber != NULL )
	{
		m_pdContactJabber->SetText( str );
	//	m_pdContactJabber->m_sLink = L"xmpp:" + str;
	}

	// Custom Extended Fields:

	str = pProfile->GetContact( L"Twitter" );
	m_pDocument1->ShowGroup( 46, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactTwitter != NULL )
	{
		bContact = TRUE;
		m_pdContactTwitter->SetText( str );
		m_pdContactTwitter->m_sLink = L"http://twitter.com/" + str;
	}

	str = pProfile->GetContact( L"Facebook" );
	m_pDocument1->ShowGroup( 47, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactFacebook != NULL )
	{
		bContact = TRUE;
		m_pdContactFacebook->SetText( str );
		m_pdContactFacebook->m_sLink = L"http://facebook.com/" + str;
	}

	str = pProfile->GetContact( L"GetEnvy.com" );
	m_pDocument1->ShowGroup( 48, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactGetEnvy != NULL )
	{
		bContact = TRUE;
		m_pdContactGetEnvy->SetText( str );
		m_pdContactGetEnvy->m_sLink = L"http://getenvy.com/users/" + str;	// ToDo: Update user profile link
	}

	m_pDocument1->ShowGroup( 4, bContact );

	str.Empty();

	if ( CXMLElement* pInterests = pProfile->GetXML( L"interests" ) )
	{
		for ( POSITION pos = pInterests->GetElementIterator() ; pos ; )
		{
			CXMLElement* pInterest = pInterests->GetNextElement( pos );

			if ( pInterest->IsNamed( L"interest" ) )
			{
				if ( ! str.IsEmpty() ) str += L", ";
				str += pInterest->GetValue();
			}
		}
	}

	m_pDocument1->ShowGroup( 5, ! str.IsEmpty() );
	if ( m_pdInterests != NULL ) m_pdInterests->SetText( str );

	str.Empty();
	if ( CXMLElement* pBio = pProfile->GetXML( L"notes" ) )
		str = pBio->GetValue();
	m_pDocument1->ShowGroup( 6, ! str.IsEmpty() );
	if ( m_pdBioText != NULL ) m_pdBioText->SetText( str );

	m_wndDoc1.InvalidateIfModified();
}

void CBrowseProfileCtrl::UpdateDocument2(CHostBrowser* pBrowser)
{
	int nBookmarks = 0;
	CGProfile* pProfile = pBrowser->m_pProfile;

	CSingleLock pLock( &m_pDocument2->m_pSection, TRUE );

	m_pDocument2->ShowGroup( 2, pBrowser->m_bCanChat );

//  ToDo: Add Flag/ClientVersion to Profile
//	CString str = pBrowser->m_sm_sCountry;
//	int nFlagImage = Flags.GetFlagIndex( pBrowser->m_sCountry );
//	if ( int )
//	{
//		m_pdCountryName->SetText( str );
//		m_pDocument2->ShowGroup( 4, TRUE );
//	}
//	else
//		m_pDocument2->ShowGroup( 4, FALSE );
//
//	str = pBrowser->m_sUserAgent;
//	if ( str != NULL )
//	{
//		m_pdVendor->SetText( str );
//		m_pDocument2->ShowGroup( 5, TRUE );
//	}
//	else
//		m_pDocument2->ShowGroup( 5, FALSE );

	if ( m_pdBookmarks != NULL )
	{
		POSITION pos = m_pDocument2->Find( m_pdBookmarks );
		if ( pos == NULL ) return;

		POSITION posSave = pos;
		m_pDocument2->GetNext( pos );

		while ( pos )
		{
			CRichElement* pElement = m_pDocument2->GetNext( pos );
			if ( pElement->m_nGroup != 1 ) break;
			pElement->Delete();
		}

		m_pDocument2->GetNext( posSave );

		if ( CXMLElement* pBookmarks = pProfile->GetXML( L"bookmarks" ) )
		{
			for ( pos = pBookmarks->GetElementIterator() ; pos ; )
			{
				CXMLElement* pBookmark = pBookmarks->GetNextElement( pos );
				if ( ! pBookmark->IsNamed( L"bookmark" ) ) continue;

				CString strTitle	= pBookmark->GetAttributeValue( L"title" );
				CString strURL		= pBookmark->GetAttributeValue( L"url" );

				if ( strTitle.IsEmpty() ) continue;
				if ( strURL.Find( L"http://" ) != 0 ) continue;

				m_pDocument2->Add( retIcon, MAKEINTRESOURCE(IDI_WEB_URL), strURL, 0, 1, posSave );
				m_pDocument2->Add( retGap, L"5", NULL, 0, 1, posSave );
				m_pDocument2->Add( retLink, strTitle, strURL, retfMiddle, 1, posSave );
				m_pDocument2->Add( retNewline, L"2", NULL, 0, 1, posSave );

				nBookmarks ++;
			}
		}
	}

	m_pDocument2->ShowGroup( 3, nBookmarks ? TRUE : FALSE );

	m_wndDoc2.InvalidateIfModified();
}

void CBrowseProfileCtrl::OnHeadPacket(CG2Packet* pPacket)
{
	CString strFile;
	G2_PACKET nType;
	DWORD nLength;

	CSingleLock pLock( &m_pSection, TRUE );

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_NAME )
		{
			strFile = pPacket->ReadString( nLength );
		}
		else if ( nType == G2_PACKET_BODY )
		{
			if ( m_imgHead.LoadFromMemory( PathFindExtension( strFile ),
				 (LPCVOID)( pPacket->m_pBuffer + pPacket->m_nPosition ), nLength ) &&
				 m_imgHead.EnsureRGB( Colors.m_crWindow ) &&
				 m_imgHead.Resample( 128, 128 ) )
			{
				// Ok
			}
		}

		pPacket->m_nPosition = nNext;
	}

	PostMessage( WM_TIMER, 1 );
}

void CBrowseProfileCtrl::LoadDefaultHead()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_imgHead.m_bLoaded ) return;

	if ( m_imgHead.LoadFromFile( Settings.General.DataPath + L"DefaultAvatar.png" ) &&
		 m_imgHead.EnsureRGB( Colors.m_crWindow ) &&
		 m_imgHead.Resample( 128, 128 ) )
	{
		Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl message handlers

int CBrowseProfileCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rect;
	m_wndDoc1.Create( WS_CHILD, rect, this, IDC_BROWSE_PROFILE );
	m_wndDoc2.Create( WS_CHILD, rect, this, IDC_BROWSE_PROFILE );
	m_wndDoc1.SetSelectable( TRUE );

	return 0;
}

void CBrowseProfileCtrl::OnDestroy()
{
	CWnd::OnDestroy();

	if ( m_pDocument2 != NULL ) delete m_pDocument2;
	m_pDocument2 = NULL;

	if ( m_pDocument1 != NULL ) delete m_pDocument1;
	m_pDocument1 = NULL;
}

void CBrowseProfileCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	CRect rc;
	GetClientRect( &rc );
	rc.left += 148;

	m_wndDoc1.SetWindowPos( NULL, rc.left, rc.top, rc.Width() / 2,
		rc.Height(), SWP_NOZORDER );

	m_wndDoc2.SetWindowPos( NULL, ( rc.left + rc.right ) / 2, rc.top,
		rc.Width() - rc.Width() / 2, rc.Height(), SWP_NOZORDER );
}

void CBrowseProfileCtrl::OnPaint()
{
	CSingleLock pLock( &m_pSection, TRUE );

	CPaintDC dc( this );
	CRect rcPanel;

	GetClientRect( &rcPanel );

	if ( m_imgHead.m_bLoaded )
	{
		CRect rcHead( 10, 10, 138, 138 );
		CDC dcMem;
		CBitmap bmHead;
		bmHead.Attach( m_imgHead.CreateBitmap() );
		dcMem.CreateCompatibleDC( &dc );
		CBitmap* pOldBmp = (CBitmap*)dcMem.SelectObject( &bmHead );
		dc.BitBlt( rcHead.left, rcHead.top, rcHead.Width(), rcHead.Height(),
			&dcMem, 0, 0, SRCCOPY );
		dc.ExcludeClipRect( &rcHead );
		dcMem.SelectObject( pOldBmp );
		bmHead.DeleteObject();
	}

	if ( m_wndDoc1.IsWindowVisible() )
		rcPanel.right = rcPanel.left + 148;

	dc.FillSolidRect( &rcPanel, Colors.m_crWindow );
}

void CBrowseProfileCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
		Invalidate();
}

void CBrowseProfileCtrl::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	if ( CRichElement* pElement = ((RVN_ELEMENTEVENT*) pNotify)->pElement )
		theApp.InternalURI( pElement->m_sLink );
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl serialize

void CBrowseProfileCtrl::Serialize(CArchive& ar, int /*nVersion*/)	// BROWSER_SER_VERSION
{
	m_imgHead.Serialize( ar );

	if ( ar.IsLoading() )
		PostMessage( WM_TIMER, 1 );
}
