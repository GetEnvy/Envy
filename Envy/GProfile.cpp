//
// GProfile.cpp
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
#include "GProfile.h"
#include "G2Packet.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define WORDLIM(x)  (WORD)( ( (x) < 0 ) ? 0 : ( ( (x) > 65535 ) ? 65535 : (x) ) )

LPCTSTR CGProfile::xmlns = L"http://schemas.getenvy.com/Profile.xsd";
LPCTSTR CGProfile::xmlnsLegacy = L"http://www.shareaza.com/schemas/GProfile.xsd";

BEGIN_INTERFACE_MAP(CGProfile, CComObject)
END_INTERFACE_MAP()

CGProfile	MyProfile;


//////////////////////////////////////////////////////////////////////
// CGProfile construction

CGProfile::CGProfile() :
	m_pXML( new CXMLElement( NULL, L"profile" ) )
{
	ASSERT( m_pXML );
	VERIFY( m_pXML->AddAttribute( L"xmlns", xmlns ) );
}

//CGProfile::~CGProfile()
//{
//}

//////////////////////////////////////////////////////////////////////
// CGProfile access

BOOL CGProfile::IsValid() const
{
	// The whole identity and location tags are deleted if no attributes are present
	return ( this && m_pXML && (
		m_pXML->GetElementByName( L"identity" ) ||
		m_pXML->GetElementByName( L"location" ) ) );
}

CXMLElement* CGProfile::GetXML(LPCTSTR pszElement, BOOL bCreate)
{
	if ( pszElement )
		return m_pXML->GetElementByName( pszElement, bCreate );

	return GetPublicXML();	// Should never happen (legacy formatting)
}

CXMLElement* CGProfile::GetPublicXML(CString strClient, BOOL bChallenge)
{
	// Add/Increment browsing counter
	if ( ! bChallenge )		// Skip G2_PACKET_PROFILE_CHALLENGE
	{
		if ( CXMLElement* pStats = m_pXML->GetElementByName( L"statistics", TRUE ) )
		{
			CString strCount = pStats->GetAttributeValue( L"hitcount", L"0" );
			strCount.Format( L"%i", _tstoi( strCount ) + 1 );
			pStats->AddAttribute( L"hitcount", (LPCTSTR)strCount );
			Save();
		}
	}

	// Prepare profile for UploadTransferHTTP browse (or G2 challenge request)
	m_pXMLExport.Free();
	m_pXMLExport.Attach( m_pXML->Clone() );

	// Remove "avatar" element (exposed local path)
	if ( CXMLElement* pAvatar = m_pXMLExport->GetElementByName( L"avatar", FALSE ) )
		pAvatar->Delete();

	// Vendor-specific compatibility changes
	if ( StartsWith( strClient, L"shareaza", 8 ) )
	{
		m_pXMLExport->SetName( L"gProfile" );
		m_pXMLExport->AddAttribute( L"xmlns", xmlnsLegacy );
	}

//#ifdef _DEBUG
//	CFile pFile;
//	pFile.Open( Settings.General.DataPath + L"Profile.Public.xml", CFile::modeWrite | CFile::modeCreate );
//	CStringA strUTF8 = UTF8Encode( m_pXMLExport->ToString( TRUE, TRUE ) );
//	pFile.Write( (LPCSTR)strUTF8, strUTF8.GetLength() );
//#endif  // TESTING

	return m_pXMLExport;
}

//////////////////////////////////////////////////////////////////////
// CGProfile core

void CGProfile::Create()
{
	// Generate new Gnutella GUID
	Hashes::Guid tmp;
	VERIFY( SUCCEEDED( CoCreateGuid( reinterpret_cast< GUID* > ( &tmp[0] ) ) ) );

	VERIFY( tmp.validate() );
	oGUID = tmp;

	CXMLElement* pGnutella = m_pXML->GetElementByName( L"gnutella", TRUE );
	VERIFY( pGnutella->AddAttribute( L"guid", tmp.toString() ) );

	CreateBT();
}

void CGProfile::CreateBT()
{
	// Convert Gnutella GUID (128 bits) to BitTorrent GUID (160 bits)
	Hashes::BtGuid tmp_bt;
	CopyMemory( &tmp_bt[0], &((Hashes::Guid)oGUID)[0], ((Hashes::Guid)oGUID).byteCount );
	for ( size_t nByte = ((Hashes::Guid)oGUID).byteCount ; nByte < tmp_bt.byteCount ; nByte++ )
		tmp_bt[ nByte ] = GetRandomNum( 0ui8, _UI8_MAX );

	VERIFY( tmp_bt.validate() );
	oGUIDBT = tmp_bt;

	CXMLElement* pBitTorrent = m_pXML->GetElementByName( L"bittorrent", TRUE );
	VERIFY( pBitTorrent->AddAttribute( L"guid", tmp_bt.toString() ) );
}

//////////////////////////////////////////////////////////////////////
// CGProfile loading and saving

BOOL CGProfile::Load()
{
	const CString strPath = Settings.General.DataPath + L"Profile.xml";

	const CXMLElement* pXML = CXMLElement::FromFile( strPath, TRUE );
	if ( pXML == NULL )
	{
		Create();

		return FALSE;
	}

	if ( ! FromXML( pXML ) )
	{
		delete pXML;
		return FALSE;
	}

	if ( ! (Hashes::BtGuid)oGUIDBT )
	{
		// Upgrade
		CreateBT();
		Save();
	}

	return TRUE;
}

BOOL CGProfile::Save()
{
	const CString strPath = Settings.General.DataPath + L"Profile.xml";

	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeWrite | CFile::modeCreate ) )
		return FALSE;

	CStringA sUTF8 = UTF8Encode( m_pXML->ToString( TRUE, TRUE ) );
	pFile.Write( (LPCSTR)sUTF8, sUTF8.GetLength() );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGProfile XML assignment

BOOL CGProfile::FromXML(const CXMLElement* pXML)
{
	// Checking XML validity
	if ( pXML == NULL )
		return FALSE;

	if ( pXML->GetAttributeValue( L"xmlns" ).CompareNoCase( xmlns ) == 0 )				// http://schemas.getenvy.com/Profile.xsd defined above
	{
		if ( ! pXML->IsNamed( L"profile" ) )
			return FALSE;
	}
	else if ( pXML->GetAttributeValue( L"xmlns" ).CompareNoCase( xmlnsLegacy ) == 0 )	// http://www.shareaza.com/schemas/GProfile.xsd defined above
	{
		if ( ! pXML->IsNamed( L"gProfile" ) )
			return FALSE;
	}
	//else if ( ! pXML->GetAttributeValue( L"xmlns" ).CompareNoCase( L"http://www.limewire.com/schemas/person.xsd" ) )
	//{
	//	// Never reached from G1.  ToDo: Is this even useful to acquire some Limewire-type xml profile data?
	//	theApp.Message( MSG_INFO, L"Limewire Profiles Currently Unsupported." );
	//	//if ( ! pXML->IsNamed( L"person" ) )
	//		return FALSE;
	//}
	else
	{
		theApp.Message( MSG_INFO, L"Unknown Profile Type:  %s", pXML->GetAttributeValue( L"xmlns" ) );
		if ( ! pXML->IsNamed( L"profile" ) && ! pXML->IsNamed( L"gProfile" ) )
			return FALSE;
	}

	// Loading Gnutella GUID (required)
	const CXMLElement* pGnutella = pXML->GetElementByName( L"gnutella" );
	if ( pGnutella == NULL )
		return FALSE;

	Hashes::Guid tmp;
	if ( ! tmp.fromString( pGnutella->GetAttributeValue( L"guid" ) ) )
		return FALSE;

	oGUID = tmp;

	// Loading BitTorrent GUID (optional)
	if ( const CXMLElement* pBitTorrent = pXML->GetElementByName( L"bittorrent" ) )
	{
		Hashes::BtGuid tmp_bt;
		if ( tmp_bt.fromString( pBitTorrent->GetAttributeValue( L"guid" ) ) )
			oGUIDBT = tmp_bt;
	}

	// Replace XML
	m_pXML.Free();
	m_pXML.Attach( const_cast< CXMLElement* >( pXML ) );

	// Insecure legacy method
	//delete m_pXML;
	//m_pXML = const_cast< CXMLElement* >( pXML );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGProfile field access

CString CGProfile::GetNick() const
{
	if ( const CXMLElement* pIdentity = m_pXML->GetElementByName( L"identity" ) )
	{
		if ( const CXMLElement* pHandle = pIdentity->GetElementByName( L"handle" ) )
			return pHandle->GetAttributeValue( L"primary" );
	}
	return CString();
}

CString CGProfile::GetLocation() const
{
	if ( const CXMLElement* pLocation = m_pXML->GetElementByName( L"location" ) )
	{
		if ( const CXMLElement* pPolitical = pLocation->GetElementByName( L"political" ) )
		{
			CString strCity = pPolitical->GetAttributeValue( L"city" );
			CString strState = pPolitical->GetAttributeValue( L"state" );
			CString strCountry = pPolitical->GetAttributeValue( L"country" );

			CString str = strCity;
			if ( ! strState.IsEmpty() )
			{
				if ( ! str.IsEmpty() )
					str += L", ";
				str += strState;
			}
			if ( ! strCountry.IsEmpty() )
			{
				if ( ! str.IsEmpty() )
					str += L", ";
				str += strCountry;
			}

			return str;
		}
	}
	return CString();
}

CString CGProfile::GetContact(LPCTSTR pszType) const
{
	if ( const CXMLElement* pContacts = m_pXML->GetElementByName( L"contacts" ) )
	{
		for ( POSITION pos = pContacts->GetElementIterator() ; pos ; )
		{
			const CXMLElement* pGroup = pContacts->GetNextElement( pos );

			if ( pGroup->IsNamed( L"group" ) &&
				 pGroup->GetAttributeValue( L"class" ).CompareNoCase( pszType ) == 0 )
			{
				if ( const CXMLElement* pAddress = pGroup->GetElementByName( L"address" ) )
					return pAddress->GetAttributeValue( L"content" );
			}
		}
	}
	return CString();
}

DWORD CGProfile::GetPackedGPS() const
{
	if ( const CXMLElement* pLocation = m_pXML->GetElementByName( L"location" ) )
	{
		if ( const CXMLElement* pCoordinates = pLocation->GetElementByName( L"coordinates" ) )
		{
			float nLatitude = 0, nLongitude = 0;
			if ( _stscanf( pCoordinates->GetAttributeValue( L"latitude" ),  L"%f", &nLatitude ) == 1 &&
				 _stscanf( pCoordinates->GetAttributeValue( L"longitude" ), L"%f", &nLongitude ) == 1 )
			{
				return ( (DWORD)WORDLIM( ( nLatitude  + 90.0f )  * 65535.0f / 180.0f ) << 16 ) +
						 (DWORD)WORDLIM( ( nLongitude + 180.0f ) * 65535.0f / 360.0f );
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CGProfile create avatar packet

CG2Packet* CGProfile::CreateAvatar() const
{
	const CXMLElement* pAvatar = m_pXML->GetElementByName( L"avatar" );
	if ( pAvatar == NULL )
		return NULL;

	CString strPath = pAvatar->GetAttributeValue( L"path" );
	if ( strPath.IsEmpty() )
		return NULL;

	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeRead ) )
		return NULL;

	int nPos = strPath.ReverseFind( L'\\' );
	if ( nPos >= 0 )
		strPath = strPath.Mid( nPos + 1 );

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PROFILE_AVATAR );
	if ( ! pPacket )
		return NULL;

	pPacket->WritePacket( G2_PACKET_NAME, pPacket->GetStringLen( strPath ) );
	pPacket->WriteString( strPath, FALSE );

	pPacket->WritePacket( G2_PACKET_BODY, (DWORD)pFile.GetLength() );
	if ( LPBYTE pBody = pPacket->WriteGetPointer( (DWORD)pFile.GetLength() ) )
		pFile.Read( pBody, (DWORD)pFile.GetLength() );

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CGProfile serialize

void CGProfile::Serialize(CArchive& ar, int /*nVersion*/)	// BROWSER_SER_VERSION
{
	BOOL bXMLPresent = FALSE;

	if ( ar.IsStoring() )
	{
		bXMLPresent = ( m_pXML->GetElementCount() != 0 );
		ar << bXMLPresent;
	}
	else // Loading
	{
		ar >> bXMLPresent;
	}

	if ( m_pXML && bXMLPresent )
		m_pXML->Serialize( ar );
}
