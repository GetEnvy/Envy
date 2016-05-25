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
#include "Flags.h"
#include "Colors.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define AVATAR_COLUMN 148

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
	: m_pDocumentLeft ( NULL )
	, m_pDocumentRight ( NULL )
{
}

CBrowseProfileCtrl::~CBrowseProfileCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl operations

BOOL CBrowseProfileCtrl::Create(CWnd* pParentWnd)
{
	return CWnd::Create( NULL, NULL, WS_CHILD, CRect(0), pParentWnd, IDC_BROWSE_PROFILE, NULL );
}

void CBrowseProfileCtrl::OnSkinChange()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pDocumentLeft != NULL )
		m_pDocumentLeft->Clear();
	else
		m_pDocumentLeft = new CRichDocument();

	m_pdNick = m_pdFullName = m_pdFullLocation = NULL;
	m_pdGenderMale = m_pdGenderFemale = m_pdAge = NULL;
	m_pdContactEmail = m_pdContactGetEnvy = m_pdContactFacebook = m_pdContactTwitter = NULL;
	m_pdContactAOL = m_pdContactYahoo = m_pdContactSkype = m_pdContactJabber = m_pdContactICQ = NULL;
	m_pdBioText = m_pdInterests = m_pdVendor = m_pdAddress = NULL;

	if ( CXMLElement* pXML = Skin.GetDocument( L"CBrowseHostProfile.1" ) )
	{
		CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;
		m_pDocumentLeft->LoadXML( pXML, &pMap );

		pMap.Lookup( L"Nick", m_pdNick );
		pMap.Lookup( L"FullName", m_pdFullName );
		pMap.Lookup( L"FullLocation", m_pdFullLocation );
		pMap.Lookup( L"GenderMale", m_pdGenderMale );
		pMap.Lookup( L"GenderFemale", m_pdGenderFemale );
		pMap.Lookup( L"Age", m_pdAge );
		pMap.Lookup( L"Vendor", m_pdVendor );
		pMap.Lookup( L"Address", m_pdAddress );
		pMap.Lookup( L"BioText", m_pdBioText );
		pMap.Lookup( L"Interests", m_pdInterests );
		pMap.Lookup( L"ContactEmail", m_pdContactEmail );
		pMap.Lookup( L"ContactSkype", m_pdContactSkype );
		pMap.Lookup( L"ContactYahoo", m_pdContactYahoo );
		pMap.Lookup( L"ContactAOL", m_pdContactAOL );
		pMap.Lookup( L"ContactICQ", m_pdContactICQ );
		pMap.Lookup( L"ContactJabber", m_pdContactJabber );
		pMap.Lookup( L"ContactTwitter", m_pdContactTwitter );
		pMap.Lookup( L"ContactFacebook", m_pdContactFacebook );
		pMap.Lookup( L"ContactGetEnvy", m_pdContactGetEnvy );
	}

	if ( m_pDocumentRight != NULL )
		m_pDocumentRight->Clear();
	else
		m_pDocumentRight = new CRichDocument();

	m_pdBookmarks = NULL;

	if ( CXMLElement* pXML = Skin.GetDocument( L"CBrowseHostProfile.2" ) )
	{
		CMap< CString, const CString&, CRichElement*, CRichElement* > pMap;
		m_pDocumentRight->LoadXML( pXML, &pMap );

		pMap.Lookup( L"Bookmarks", m_pdBookmarks );
	}

	m_wndDoc1.SetDocument( m_pDocumentLeft );
	m_wndDoc2.SetDocument( m_pDocumentRight );
}

void CBrowseProfileCtrl::Update(CHostBrowser* pBrowser)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pDocumentLeft == NULL || m_pDocumentRight == NULL ) return;

	CGProfile* pProfile = pBrowser->m_pProfile;
	if ( pProfile == NULL || pProfile->IsValid() == FALSE ) return;

	if ( ! pBrowser->IsBrowsing() && pBrowser->m_nHits > 0 && ! m_imgHead.m_bLoaded )
		LoadDefaultHead();

	UpdateDocumentLeft( pBrowser, pProfile );
	UpdateDocumentRight( pBrowser, pProfile );

	if ( m_wndDoc1.IsWindowVisible() == FALSE ) m_wndDoc1.ShowWindow( SW_SHOW );
	if ( m_wndDoc2.IsWindowVisible() == FALSE ) m_wndDoc2.ShowWindow( SW_SHOW );
}

void CBrowseProfileCtrl::UpdateDocumentLeft(CHostBrowser* pBrowser, CGProfile* pProfile)
{
	CSingleLock pLock( &m_pDocumentLeft->m_pSection, TRUE );

	if ( m_pdNick != NULL )
		m_pdNick->SetText( pProfile->GetNick() );

	// 0 Client/Flag
	if ( m_pdVendor != NULL && ( ! pBrowser->m_sServer.IsEmpty() || ! pBrowser->m_sCountry.IsEmpty() ) )
	{
		// Note: pBrowser->m_sAddress and pBrowser->m_sCountry are empty at this point, but available in 2nd colunm (Invalidate cookie?)
		CString str = Settings.General.GUIMode == GUI_BASIC ?
			theApp.GetCountryName( pBrowser->m_pAddress ) : (LPCTSTR)CString( inet_ntoa( pBrowser->m_pAddress ) );
		m_pdAddress->SetText( str );
		m_pdVendor->SetText( (LPCTSTR)pBrowser->m_sServer );
		str.Format( L"gnutella:browse:%s:%u", (LPCTSTR)str, pBrowser->m_nPort );			// ToDo: Append Private Key?
		m_pdVendor->m_sLink = L"command:copy:" + str;	

		if ( ! m_pdAddress->m_hImage )	// Add Flag Once
		{
			if ( int nFlagImage = Flags.GetFlagIndex( theApp.GetCountryCode( pBrowser->m_pAddress ) ) )
			{
				if ( POSITION pos = m_pDocumentLeft->Find( m_pdAddress ) )
				{
					if ( HICON hFlag = Flags.ExtractIconW( nFlagImage ) )
					{
						m_pdAddress->m_hImage = hFlag;
					//	m_pDocumentLeft->GetNext( pos );	// Place after text
						m_pDocumentLeft->Add( hFlag, m_pdVendor->m_sLink, 0, 0, pos );
						m_pDocumentLeft->Add( retGap, L"5", NULL, 0, 0, pos );
					}
				}
			}
		}

		m_pDocumentRight->ShowGroup( 0, TRUE );
	}
	else
		m_pDocumentRight->ShowGroup( 0, FALSE );

	// 1 Name
	if ( CXMLElement* pIdentity = pProfile->GetXML( L"identity" ) )
	{
		if ( CXMLElement* pName = pIdentity->GetElementByName( L"name" ) )
		{
			CString strFirst = pName->GetAttributeValue( L"first" );
			CString strLast  = pName->GetAttributeValue( L"last" );

			if ( m_pdFullName != NULL && ( ! strFirst.IsEmpty() || ! strLast.IsEmpty() ) )
				m_pdFullName->SetText( strFirst + L' ' + strLast );
		}
	}

	if ( ! m_pdFullName || m_pdFullName->m_sText.IsEmpty() )
		m_pDocumentLeft->ShowGroup( 1, FALSE );
	else
		m_pDocumentLeft->ShowGroup( 1, TRUE );

	// 2 Location
	CString str = pProfile->GetLocation();
	m_pDocumentLeft->ShowGroup( 2, ! str.IsEmpty() );
	if ( m_pdFullLocation != NULL )
		m_pdFullLocation->SetText( str );

	// 3 Age/Gender
	if ( CXMLElement* pVitals = pProfile->GetXML( L"vitals" ) )
	{
		str = pVitals->GetAttributeValue( L"gender" );
		m_pDocumentLeft->ShowGroup( 3, ! str.IsEmpty() );

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
		m_pDocumentLeft->ShowGroup( 3, FALSE );
	}

	// 4 Social
	BOOL bContact = FALSE;

	str = pProfile->GetContact( L"Email" );
	m_pDocumentLeft->ShowGroup( 40, ! str.IsEmpty() );
	if ( m_pdContactEmail != NULL )
	{
		bContact = TRUE;
		m_pdContactEmail->SetText( str );
		m_pdContactEmail->m_sLink = L"mailto:" + str;
	}

	str = pProfile->GetContact( L"GetEnvy.com" );
	m_pDocumentLeft->ShowGroup( 48, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactGetEnvy != NULL )
	{
		bContact = TRUE;
		m_pdContactGetEnvy->SetText( str );
		m_pdContactGetEnvy->m_sLink = L"http://getenvy.com/users/" + str;	// ToDo: Update user profile link
	}

	str = pProfile->GetContact( L"Facebook" );
	m_pDocumentLeft->ShowGroup( 47, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactFacebook != NULL )
	{
		bContact = TRUE;
		m_pdContactFacebook->SetText( str );
		m_pdContactFacebook->m_sLink = L"http://facebook.com/" + str;
	}

	str = pProfile->GetContact( L"Twitter" );
	m_pDocumentLeft->ShowGroup( 46, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactTwitter != NULL )
	{
		bContact = TRUE;
		m_pdContactTwitter->SetText( str );
		m_pdContactTwitter->m_sLink = L"http://twitter.com/" + str;
	}

	str = pProfile->GetContact( L"Skype" );
	if ( str.IsEmpty() )
		str = pProfile->GetContact( L"MSN" );	// Legacy
	m_pDocumentLeft->ShowGroup( 44, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactSkype != NULL )
	{
		bContact = TRUE;
		m_pdContactSkype->SetText( str );
		m_pdContactSkype->m_sLink = L"skype:" + str + L"?chat";
	}

	str = pProfile->GetContact( L"Yahoo" );
	m_pDocumentLeft->ShowGroup( 41, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactYahoo != NULL )
	{
		bContact = TRUE;
		m_pdContactYahoo->SetText( str );
		m_pdContactYahoo->m_sLink = L"ymsgr:sendim?" + str;
	}

	str = pProfile->GetContact( L"AOL" );
	m_pDocumentLeft->ShowGroup( 43, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactAOL != NULL )
	{
		bContact = TRUE;
		m_pdContactAOL->SetText( str );
		m_pdContactAOL->m_sLink = L"aim:goim?screenname=" + str;
	}

	str = pProfile->GetContact( L"ICQ" );
	m_pDocumentLeft->ShowGroup( 42, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactICQ != NULL )
	{
		bContact = TRUE;
		m_pdContactICQ->SetText( str );
		m_pdContactICQ->m_sLink = L"http://icq.com/people/" + str;
	}

	str = pProfile->GetContact( L"Google" );
	if ( str.IsEmpty() )
		str = pProfile->GetContact( L"Jabber" );	// Legacy
	m_pDocumentLeft->ShowGroup( 45, ! str.IsEmpty() );
	if ( ! str.IsEmpty() && m_pdContactJabber != NULL )
	{
		bContact = TRUE;
		m_pdContactJabber->SetText( str );
	//	m_pdContactJabber->m_sLink = L"xmpp:" + str;
	}

	m_pDocumentLeft->ShowGroup( 4, bContact );

	str.Empty();

	// 5 Interests
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

	m_pDocumentLeft->ShowGroup( 5, ! str.IsEmpty() );
	if ( m_pdInterests != NULL ) m_pdInterests->SetText( str );

	str.Empty();

	// 6 Comments
	if ( CXMLElement* pBio = pProfile->GetXML( L"notes" ) )
		str = pBio->GetValue();
	m_pDocumentLeft->ShowGroup( 6, ! str.IsEmpty() );
	if ( m_pdBioText != NULL ) m_pdBioText->SetText( str );

	m_wndDoc1.InvalidateIfModified();
}

void CBrowseProfileCtrl::UpdateDocumentRight(CHostBrowser* pBrowser, CGProfile* pProfile)
{
	int nBookmarks = 0;

	CSingleLock pLock( &m_pDocumentRight->m_pSection, TRUE );

	m_pDocumentRight->ShowGroup( 2, pBrowser->m_bCanChat );

	if ( m_pdBookmarks != NULL )
	{
		POSITION pos = m_pDocumentRight->Find( m_pdBookmarks );
		if ( pos == NULL ) return;

		POSITION posSave = pos;
		m_pDocumentRight->GetNext( pos );

		while ( pos )
		{
			CRichElement* pElement = m_pDocumentRight->GetNext( pos );
			if ( pElement->m_nGroup != 1 ) break;
			pElement->Delete();
		}

		if ( pProfile != NULL && pProfile->IsValid() )
		{
			m_pDocumentRight->GetNext( posSave );

			if ( CXMLElement* pBookmarks = pProfile->GetXML( L"bookmarks" ) )
			{
				for ( pos = pBookmarks->GetElementIterator() ; pos ; )
				{
					CXMLElement* pBookmark = pBookmarks->GetNextElement( pos );
					if ( ! pBookmark->IsNamed( L"bookmark" ) ) continue;

					CString strTitle	= pBookmark->GetAttributeValue( L"title" );
					CString strURL		= pBookmark->GetAttributeValue( L"url" );

					if ( strTitle.IsEmpty() ) continue;
					if ( ! StartsWith( strURL, _P( L"http://" ) ) && ! StartsWith( strURL, _P( L"https://" ) ) ) continue;

					m_pDocumentRight->Add( retIcon, MAKEINTRESOURCE(IDI_WEB_URL), strURL, 0, 1, posSave );
					m_pDocumentRight->Add( retGap, L"5", NULL, 0, 1, posSave );
					m_pDocumentRight->Add( retLink, strTitle, strURL, retfMiddle, 1, posSave );
					m_pDocumentRight->Add( retNewline, L"2", NULL, 0, 1, posSave );

					nBookmarks++;
				}
			}
		}
	}

	m_pDocumentRight->ShowGroup( 3, nBookmarks ? TRUE : FALSE );

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

	if ( m_pDocumentRight != NULL ) delete m_pDocumentRight;
	m_pDocumentRight = NULL;

	if ( m_pDocumentLeft != NULL ) delete m_pDocumentLeft;
	m_pDocumentLeft = NULL;
}

void CBrowseProfileCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	CRect rc;
	GetClientRect( &rc );
	rc.left += AVATAR_COLUMN;

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
		dc.BitBlt( rcHead.left, rcHead.top, rcHead.Width(), rcHead.Height(), &dcMem, 0, 0, SRCCOPY );
		dc.ExcludeClipRect( &rcHead );
		dcMem.SelectObject( pOldBmp );
		bmHead.DeleteObject();
	}

	if ( m_wndDoc1.IsWindowVisible() )
		rcPanel.right = rcPanel.left + AVATAR_COLUMN;

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
