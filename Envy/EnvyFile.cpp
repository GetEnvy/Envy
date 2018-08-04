//
// EnvyFile.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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
#include "EnvyFile.h"
#include "Network.h"
#include "DlgURLCopy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

inline bool validAndEqual(QWORD nLeft, QWORD nRight)
{
	return ( nLeft != SIZE_UNKNOWN && nRight != SIZE_UNKNOWN && nLeft == nRight );
}

inline bool validAndUnequal(QWORD nLeft, QWORD nRight)
{
	return ( nLeft != SIZE_UNKNOWN && nRight != SIZE_UNKNOWN && nLeft != nRight );
}

inline bool validAndEqual(const CString& sLeft, const CString& sRight)
{
	return ( sLeft.GetLength() && sRight.GetLength() && ! sLeft.CompareNoCase( sRight ) );
}

inline bool validAndUnequal(const CString& sLeft, const CString& sRight)
{
	return ( sLeft.GetLength() && sRight.GetLength() && sLeft.CompareNoCase( sRight ) );
}

// CEnvyFile

IMPLEMENT_DYNAMIC(CEnvyFile, CComObject)

BEGIN_INTERFACE_MAP(CEnvyFile, CComObject)
	INTERFACE_PART(CEnvyFile, IID_IEnvyFile, EnvyFile)
END_INTERFACE_MAP()

CEnvyFile::CEnvyFile()
	: m_nSize	( SIZE_UNKNOWN )
{
	EnableDispatch( IID_IEnvyFile );
}

CEnvyFile::CEnvyFile(const CEnvyFile& pFile)
	: m_sName	( pFile.m_sName )
	, m_nSize	( pFile.m_nSize )
	, m_oSHA1	( pFile.m_oSHA1 )
	, m_oTiger	( pFile.m_oTiger )
	, m_oED2K	( pFile.m_oED2K )
	, m_oBTH	( pFile.m_oBTH )
	, m_oMD5	( pFile.m_oMD5 )
	, m_sPath	( pFile.m_sPath )
	, m_sURL	( pFile.m_sURL )
{
	EnableDispatch( IID_IEnvyFile );
}

CEnvyFile& CEnvyFile::operator=(const CEnvyFile& pFile)
{
	m_sName = pFile.m_sName;
	m_nSize = pFile.m_nSize;
	m_oSHA1 = pFile.m_oSHA1;
	m_oTiger = pFile.m_oTiger;
	m_oED2K = pFile.m_oED2K;
	m_oBTH  = pFile.m_oBTH;
	m_oMD5  = pFile.m_oMD5;
	m_sPath = pFile.m_sPath;
	m_sURL  = pFile.m_sURL;
	return *this;
}

bool CEnvyFile::operator==(const CEnvyFile& pFile) const
{
	if ( this == &pFile )
		return true;	// Same object

	if ( validAndUnequal( m_nSize,  pFile.m_nSize  ) ||
		 validAndUnequal( m_oSHA1,  pFile.m_oSHA1  ) ||
		 validAndUnequal( m_oTiger, pFile.m_oTiger ) ||
		 validAndUnequal( m_oED2K,  pFile.m_oED2K  ) ||
		 validAndUnequal( m_oMD5,   pFile.m_oMD5   ) )
		return false;	// Different sizes or hashes (excluding BitTorrent)

	if ( validAndEqual( m_oSHA1,  pFile.m_oSHA1  ) ||
		 validAndEqual( m_oTiger, pFile.m_oTiger ) ||
		 validAndEqual( m_oED2K,  pFile.m_oED2K  ) ||
		 validAndEqual( m_oMD5,   pFile.m_oMD5   ) )
		return true;	// Same hash (excluding BitTorrent)

	if ( validAndEqual( m_oBTH,  pFile.m_oBTH  ) &&
		 validAndEqual( m_sName, pFile.m_sName ) )
		return true;	// Same name and BitTorrent hash

	return false;		// Insufficient data
}

bool CEnvyFile::operator!=(const CEnvyFile& pFile) const
{
	if ( this == &pFile )
		return false;	// Same object

	if ( validAndUnequal( m_nSize,  pFile.m_nSize  ) ||
		 validAndUnequal( m_oSHA1,  pFile.m_oSHA1  ) ||
		 validAndUnequal( m_oTiger, pFile.m_oTiger ) ||
		 validAndUnequal( m_oED2K,  pFile.m_oED2K  ) ||
		 validAndUnequal( m_oMD5,   pFile.m_oMD5   ) )
		return true;	// Different sizes or hashes (excluding BitTorrent)

	return false;		// Insufficient data
}

CString CEnvyFile::GetURL(const IN_ADDR& nAddress, WORD nPort) const
{
	CString strURL;
	if ( m_oSHA1 )
	{
		strURL.Format( L"http://%s:%i/uri-res/N2R?%s",
			(LPCTSTR)CString( inet_ntoa( nAddress ) ), nPort, (LPCTSTR)m_oSHA1.toUrn() );
	}
	else if ( m_oTiger )
	{
		strURL.Format( L"http://%s:%i/uri-res/N2R?%s",
			(LPCTSTR)CString( inet_ntoa( nAddress ) ), nPort, (LPCTSTR)m_oTiger.toUrn() );
	}
	else if ( m_oED2K )
	{
		strURL.Format( L"http://%s:%i/uri-res/N2R?%s",
			(LPCTSTR)CString( inet_ntoa( nAddress ) ), nPort, (LPCTSTR)m_oED2K.toUrn() );
	}
	else if ( m_oMD5 )
	{
		strURL.Format( L"http://%s:%i/uri-res/N2R?%s",
			(LPCTSTR)CString( inet_ntoa( nAddress ) ), nPort, (LPCTSTR)m_oMD5.toUrn() );
	}
	else if ( m_oBTH )
	{
		strURL.Format( L"http://%s:%i/uri-res/N2R?%s",
			(LPCTSTR)CString( inet_ntoa( nAddress ) ), nPort, (LPCTSTR)m_oBTH.toUrn() );
	}
	return strURL;
}

CString CEnvyFile::GetBitprint() const
{
	if ( m_oSHA1 || m_oTiger )
		return GetURN();

	return CString();
}

CString CEnvyFile::GetURN() const
{
	if ( m_oSHA1 && m_oTiger )	// L"urn:bitprint:"
		return Hashes::TigerHash::urns[ 2 ].signature + m_oSHA1.toString() + L'.' + m_oTiger.toString();
	if ( m_oSHA1 )
		return m_oSHA1.toUrn();
	if ( m_oTiger )
		return m_oTiger.toUrn();
	if ( m_oED2K )
		return m_oED2K.toUrn();
	if ( m_oBTH )
		return m_oBTH.toUrn();
	if ( m_oMD5 )
		return m_oMD5.toUrn();

	return CString();
}

CString CEnvyFile::GetShortURN() const
{
	if ( m_oSHA1 && m_oTiger )
		return Hashes::TigerHash::urns[ 3 ].signature + m_oSHA1.toString() + L'.' + m_oTiger.toString();
	if ( m_oSHA1 )
		return m_oSHA1.toShortUrn();
	if ( m_oTiger )
		return m_oTiger.toShortUrn();
	if ( m_oED2K )
		return m_oED2K.toShortUrn();
	if ( m_oMD5 )
		return m_oMD5.toShortUrn();
	if ( m_oBTH )
		return m_oBTH.toShortUrn();

	return CString();
}

CString CEnvyFile::GetFilename() const
{
	CString strFilename;
	if ( m_oTiger )
		strFilename = L"ttr_"  + m_oTiger.toString();
	else if ( m_oSHA1 )
		strFilename = L"sha1_" + m_oSHA1.toString();
	else if ( m_oED2K )
		strFilename = L"ed2k_" + m_oED2K.toString();
	else if ( m_oBTH )
		strFilename = L"btih_" + m_oBTH.toString();
	else if ( m_oMD5 )
		strFilename = L"md5_"  + m_oMD5.toString();
	else if ( m_sName.GetLength() > 1 )			// Note need to avoid conflicts, but random numbers can leave orphan partials, different .pd and .partial
		strFilename.Format( L"file_%s.%s", (LPCTSTR)m_sName, (LPCTSTR)CTime::GetCurrentTime().Format( L"%M%S" ).Mid( 1, 2 ) );
	else
		strFilename.Format( L"file_%.2i%.2i%.2i", GetRandomNum( 0, 99 ), GetRandomNum( 0, 99 ), GetRandomNum( 0, 99 ) );
	return strFilename;
}

bool CEnvyFile::SplitStringToURLs(LPCTSTR pszURLs, CMapStringToFILETIME& oUrls) const
{
	CString strURLs( pszURLs );

	// Fix buggy URLs
	strURLs.Replace( L"Zhttp://", L"Z, http://" );
	strURLs.Replace( L"Z%2C http://", L"Z, http://" );

	// Temporary replace quoted commas
	bool bQuote = false;
	for ( int nScan = 0 ; nScan < strURLs.GetLength() ; nScan++ )
	{
		if ( strURLs[ nScan ] == '\"' )
		{
			bQuote = ! bQuote;
			strURLs.SetAt( nScan, ' ' );
		}
		else if ( strURLs[ nScan ] == ',' && bQuote )
		{
			strURLs.SetAt( nScan, '\x1f' );
		}
	}

	int nStart = 0;
	for ( ;; )
	{
		CString strURL = strURLs.Tokenize( L",", nStart );
		if ( strURL.IsEmpty() )
			break;
		strURL.Replace( '\x1f', ',' );	// Restore quoted commas
		strURL.Trim();

		// Get time
		FILETIME tSeen = { 0, 0 };
		int nPos = strURL.ReverseFind( L' ' );
		if ( nPos > 8 && TimeFromString( strURL.Mid( nPos + 1 ).TrimLeft(), &tSeen ) )
			strURL = strURL.Left( nPos ).TrimRight();

		// Convert short "h.o.s.t:port" to full source URL
		nPos = strURL.Find( L':' );
		if ( nPos > 6 && strURL.GetLength() > nPos + 1 &&
			strURL.GetAt( nPos + 1 ) != '/' )
		{
			int nPort;
			if ( _stscanf( strURL.Mid( nPos + 1 ), L"%i", &nPort ) != 1 )
				nPort = 0;
			DWORD nAddress = inet_addr( CT2CA( strURL.Left( nPos ) ) );
			if ( nPort > 0 && nPort <= USHRT_MAX && nAddress != INADDR_NONE &&
				! Network.IsFirewalledAddress( (IN_ADDR*)&nAddress, TRUE ) &&
				! Network.IsReserved( (IN_ADDR*)&nAddress ) )
			{
				strURL = GetURL( *(IN_ADDR*)&nAddress, static_cast< WORD >( nPort ) );
			}
			else
			{
				strURL.Empty();
			}
		}

		if ( ! strURL.IsEmpty() )
		{
			strURL.Replace( L"%2C", L"," );
			oUrls.SetAt( strURL, tSeen );
		}
	}

	return ! oUrls.IsEmpty();
}

//////////////////////////////////////////////////////////////////////
// CEnvyFile Automation

IMPLEMENT_DISPATCH(CEnvyFile, EnvyFile)

STDMETHODIMP CEnvyFile::XEnvyFile::get_Path(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CEnvyFile, EnvyFile )
	*psPath = CComBSTR( pThis->m_sPath ).Detach();
	return S_OK;
}

STDMETHODIMP CEnvyFile::XEnvyFile::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CEnvyFile, EnvyFile )
	*psName = CComBSTR( pThis->m_sName ).Detach();
	return S_OK;
}

STDMETHODIMP CEnvyFile::XEnvyFile::get_Size(ULONGLONG FAR* pnSize)
{
	METHOD_PROLOGUE( CEnvyFile, EnvyFile )
	*pnSize = pThis->m_nSize;
	return S_OK;
}

STDMETHODIMP CEnvyFile::XEnvyFile::get_URN(BSTR sURN, BSTR FAR* psURN)
{
	METHOD_PROLOGUE( CEnvyFile, EnvyFile )

	CString strURN = sURN ? sURN : L"";
	CComBSTR bstrURN;

	if ( strURN.IsEmpty() )
	{
		bstrURN = pThis->GetURN();
	}
	else if ( strURN.CompareNoCase( L"urn:bitprint" ) == 0 )
	{
		if ( pThis->m_oSHA1 && pThis->m_oTiger ) bstrURN = L"urn:bitprint:" + pThis->m_oSHA1.toString() + L'.' + pThis->m_oTiger.toString();
	}
	else if ( strURN.CompareNoCase( L"urn:sha1" ) == 0 )
	{
		if ( pThis->m_oSHA1 ) bstrURN = pThis->m_oSHA1.toUrn();
	}
	else if ( strURN.CompareNoCase( L"urn:tree:tiger/" ) == 0 )
	{
		if ( pThis->m_oTiger ) bstrURN = pThis->m_oTiger.toUrn();
	}
	else if ( strURN.CompareNoCase( L"urn:md5" ) == 0 )
	{
		if ( pThis->m_oMD5 ) bstrURN = pThis->m_oMD5.toUrn();
	}
	else if ( strURN.CompareNoCase( L"urn:ed2k" ) == 0 )
	{
		if ( pThis->m_oED2K ) bstrURN = pThis->m_oED2K.toUrn();
	}
	else if ( strURN.CompareNoCase( L"urn:btih" ) == 0 )
	{
		if ( pThis->m_oBTH ) bstrURN = pThis->m_oBTH.toUrn();
	}

	*psURN = bstrURN.Detach();

	return S_OK;
}

STDMETHODIMP CEnvyFile::XEnvyFile::get_Hash(URN_TYPE nType, ENCODING nEncoding, BSTR FAR* psURN)
{
	METHOD_PROLOGUE( CEnvyFile, EnvyFile )

	CComBSTR bstrURN;

	switch ( nType )
	{
	case URN_SHA1:
		if ( pThis->m_oSHA1 )
		{
			switch ( nEncoding )
			{
			case ENCODING_GUID:
				bstrURN = pThis->m_oSHA1.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				bstrURN = pThis->m_oSHA1.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				bstrURN = pThis->m_oSHA1.toString< Hashes::base32Encoding >();
				break;
			//default:
			//	;
			}
		}
		break;

	case URN_TIGER:
		if ( pThis->m_oTiger )
		{
			switch ( nEncoding )
			{
			case ENCODING_GUID:
				bstrURN = pThis->m_oTiger.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				bstrURN = pThis->m_oTiger.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				bstrURN = pThis->m_oTiger.toString< Hashes::base32Encoding >();
				break;
			//default:
			//	;
			}
		}
		break;

	case URN_ED2K:
		if ( pThis->m_oED2K )
		{
			switch ( nEncoding )
			{
			case ENCODING_GUID:
				bstrURN = pThis->m_oED2K.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				bstrURN = pThis->m_oED2K.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				bstrURN = pThis->m_oED2K.toString< Hashes::base32Encoding >();
				break;
			//default:
			//	;
			}
		}
		break;

	case URN_MD5:
		if ( pThis->m_oMD5 )
		{
			switch ( nEncoding )
			{
			case ENCODING_GUID:
				bstrURN = pThis->m_oMD5.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				bstrURN = pThis->m_oMD5.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				bstrURN = pThis->m_oMD5.toString< Hashes::base32Encoding >();
				break;
			//default:
			//	;
			}
		}
		break;

	case URN_BTIH:
		if ( pThis->m_oBTH )
		{
			switch ( nEncoding )
			{
			case ENCODING_GUID:
				bstrURN = pThis->m_oBTH.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				bstrURN = pThis->m_oBTH.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				bstrURN = pThis->m_oBTH.toString< Hashes::base32Encoding >();
				break;
			//default:
			//	;
			}
		}
		break;

	//default:
	//	;
	}

	*psURN = bstrURN.Detach();

	return S_OK;
}

STDMETHODIMP CEnvyFile::XEnvyFile::get_URL(BSTR FAR* psURL)
{
	METHOD_PROLOGUE( CEnvyFile, EnvyFile )
	*psURL = CComBSTR( pThis->m_sURL ).Detach();
	return S_OK;
}

STDMETHODIMP CEnvyFile::XEnvyFile::get_Magnet(BSTR FAR* psMagnet)
{
	METHOD_PROLOGUE( CEnvyFile, EnvyFile )
	*psMagnet = CComBSTR( CURLCopyDlg::CreateMagnet( *pThis ) ).Detach();
	return S_OK;
}
