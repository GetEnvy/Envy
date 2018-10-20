//
// IEProtocol.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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
#include "IEProtocol.h"

#include "Buffer.h"
#include "Colors.h"
#include "Connection.h"
#include "Library.h"
#include "SharedFile.h"
#include "AlbumFolder.h"
#include "LibraryFolders.h"
#include "LibraryHistory.h"
#include "CollectionFile.h"
#include "ZIPFile.h"
#include "ImageFile.h"
#include "ThumbCache.h"
#include "ShellIcons.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

/////////////////////////////////////////////////////////////////////////////
// System

IMPLEMENT_DYNAMIC(CIEProtocol, CComObject)

IMPLEMENT_DYNAMIC(CIEProtocolRequest, CComObject)

// {18D11ED9-1264-48A1-9E14-20F2C633242B}
IMPLEMENT_OLECREATE_FLAGS(CIEProtocol, "Envy.IEProtocol",
	afxRegFreeThreading|afxRegApartmentThreading,
	0x18d11ed9, 0x1264, 0x48a1, 0x9e, 0x14, 0x20, 0xf2, 0xc6, 0x33, 0x24, 0x2b)

// {E1A67AE5-7041-4AE1-94F7-DE03EF759E27}
IMPLEMENT_OLECREATE_FLAGS(CIEProtocolRequest, "Envy.IEProtocolRequest",
	afxRegFreeThreading|afxRegApartmentThreading,
	0xe1a67ae5, 0x7041, 0x4ae1, 0x94, 0xf7, 0xde, 0x03, 0xef, 0x75, 0x9e, 0x27)

BEGIN_INTERFACE_MAP(CIEProtocol, CComObject)
	INTERFACE_PART(CIEProtocol, IID_IClassFactory, ClassFactory)
END_INTERFACE_MAP()

BEGIN_INTERFACE_MAP(CIEProtocolRequest, CComObject)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternet, InternetProtocol)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocol, InternetProtocol)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocolRoot, InternetProtocol)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocolInfo, InternetProtocolInfo)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Global Instance

LPCWSTR CIEProtocol::pszProtocols[] = { L"p2p-col", L"p2p-file", L"p2p-app", NULL };

CIEProtocol IEProtocol;


/////////////////////////////////////////////////////////////////////////////
// CIEProtocol construction

CIEProtocol::CIEProtocol()
{
}

CIEProtocol::~CIEProtocol()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol session control

BOOL CIEProtocol::Create()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pSession )
		return TRUE;

	CComPtr<IInternetSession> pSession;

	if ( FAILED( CoInternetGetSession( 0, &pSession, 0 ) ) )
		return FALSE;

	for ( int nProtocol = 0; pszProtocols[ nProtocol ] != NULL; nProtocol++ )
	{
		if ( FAILED( pSession->RegisterNameSpace( &m_xClassFactory, CLSID_EnvyIEProtocol, pszProtocols[ nProtocol ], 0, NULL, 0 ) ) )
			return FALSE;
	}

	m_pSession = pSession;

	return TRUE;
}

void CIEProtocol::Close()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pSession )
	{
		for ( int nProtocol = 0; pszProtocols[ nProtocol ] != NULL; nProtocol++ )
		{
			m_pSession->UnregisterNameSpace( &m_xClassFactory, pszProtocols[ nProtocol ] );
		}
		m_pSession.Release();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol IClassFactory implementation

IMPLEMENT_UNKNOWN(CIEProtocol, ClassFactory)

STDMETHODIMP CIEProtocol::XClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	METHOD_PROLOGUE(CIEProtocol, ClassFactory)

	if ( pUnkOuter != NULL )
		return CLASS_E_NOAGGREGATION;

	CIEProtocolRequest* pRequest = new CIEProtocolRequest();
	HRESULT hr = pRequest->ExternalQueryInterface( &riid, ppvObject );
	pRequest->ExternalRelease();

	return hr;
}

STDMETHODIMP CIEProtocol::XClassFactory::LockServer(BOOL fLock)
{
	METHOD_PROLOGUE(CIEProtocol, ClassFactory)

	if ( fLock )
		AfxOleLockApp();
	else
		AfxOleUnlockApp();

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest construction

CIEProtocolRequest::CIEProtocolRequest()
{
}

CIEProtocolRequest::~CIEProtocolRequest()
{
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest transfer handler

HRESULT CIEProtocolRequest::OnStart(LPCTSTR pszURL, IInternetProtocolSink* pSink, IInternetBindInfo* /*pBindInfo*/, DWORD dwFlags)
{
	HRESULT hr = IEProtocol.OnRequest( pszURL, m_oBuffer, m_strMimeType, ( dwFlags & PI_PARSE_URL ) != 0 );

	if ( ( dwFlags & PI_PARSE_URL ) || hr == INET_E_INVALID_URL )
		return hr;

	m_pSink = pSink;

	if ( SUCCEEDED( hr ) )
	{
		if ( ! m_strMimeType.IsEmpty() )
		{
			hr = m_pSink->ReportProgress( BINDSTATUS_MIMETYPEAVAILABLE, CComBSTR( m_strMimeType ) );
			ASSERT( SUCCEEDED( hr ) );

			hr = m_pSink->ReportProgress( BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, CComBSTR( m_strMimeType ) );
			ASSERT( SUCCEEDED( hr ) );
		}

		hr = m_pSink->ReportData( BSCF_FIRSTDATANOTIFICATION, 0, m_oBuffer.m_nLength );
		ASSERT( SUCCEEDED( hr ) );

		hr = m_pSink->ReportData( BSCF_LASTDATANOTIFICATION, m_oBuffer.m_nLength, m_oBuffer.m_nLength );
		ASSERT( SUCCEEDED( hr ) );

		hr = m_pSink->ReportResult( S_OK, 200, NULL );
		ASSERT( SUCCEEDED( hr ) );
	}
	else
	{
		hr = m_pSink->ReportResult( INET_E_OBJECT_NOT_FOUND, 404, NULL );
		ASSERT( SUCCEEDED( hr ) );
	}

	return hr;
}

HRESULT CIEProtocolRequest::OnRead(void* pv, ULONG cb, ULONG* pcbRead)
{
	cb = min( cb, m_oBuffer.m_nLength );
	if ( pcbRead != NULL )
		*pcbRead = cb;

	if ( cb > 0 )
	{
		CopyMemory( pv, m_oBuffer.m_pBuffer, cb );
		m_oBuffer.Remove( cb );
	}

	return ( cb > 0 || m_oBuffer.m_nLength > 0 ) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest IInternetProtocol implementation

IMPLEMENT_UNKNOWN(CIEProtocolRequest, InternetProtocol)

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Abort(HRESULT /*hrReason*/, DWORD /*dwOptions*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Continue(PROTOCOLDATA* /*pProtocolData*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Resume()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return E_NOTIMPL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Start(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR /*dwReserved*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return pThis->OnStart( (LPCTSTR)CW2T( szUrl ), pOIProtSink, pOIBindInfo, grfPI );
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Suspend()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return E_NOTIMPL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Terminate(DWORD /*dwOptions*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::LockRequest(DWORD /*dwOptions*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return pThis->OnRead( pv, cb, pcbRead );
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Seek(LARGE_INTEGER /*dlibMove*/, DWORD /*dwOrigin*/, ULARGE_INTEGER* /*plibNewPosition*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return E_FAIL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::UnlockRequest()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	pThis->m_pSink = NULL;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest IInternetProtocolInfo implementation

IMPLEMENT_UNKNOWN(CIEProtocolRequest, InternetProtocolInfo)

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::CombineUrl(LPCWSTR /*pwzBaseUrl*/, LPCWSTR /*pwzRelativeUrl*/, DWORD /*dwCombineFlags*/, LPWSTR /*pwzResult*/, DWORD /*cchResult*/, DWORD* /*pcchResult*/, DWORD /*dwReserved*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::CompareUrl(LPCWSTR /*pwzUrl1*/, LPCWSTR /*pwzUrl2*/, DWORD /*dwCompareFlags*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::ParseUrl(LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD /*dwParseFlags*/, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD /*dwReserved*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	UNUSED_ALWAYS( pwzUrl );

	// HACK: Security bypass
	switch ( ParseAction )
	{
	case PARSE_SECURITY_URL:
	case PARSE_SECURITY_DOMAIN:
		*pcchResult = lstrlen( WEB_SITE ) + 1;
		if ( cchResult < *pcchResult || pwzResult == NULL ) return S_FALSE;
		_tcscpy_s( pwzResult, cchResult, WEB_SITE );
		return S_OK;
	default:
		return INET_E_DEFAULT_ACTION;
	}
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::QueryInfo(LPCWSTR pwzUrl, QUERYOPTION OueryOption, DWORD /*dwQueryFlags*/, LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD /*dwReserved*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	UNUSED_ALWAYS( pwzUrl );

	switch ( OueryOption )
	{
	case QUERY_USES_NETWORK:
	case QUERY_IS_SECURE:
	case QUERY_IS_SAFE:
		*pcbBuf = sizeof( DWORD );
		if ( cbBuffer < *pcbBuf || pBuffer == NULL ) return S_FALSE;
		*(DWORD*)pBuffer = 0;
		return S_OK;
	default:
		return INET_E_DEFAULT_ACTION;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CIEProtocol request handler

HRESULT CIEProtocol::OnRequest(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly)
{
	CSingleLock pLock( &m_pSection, TRUE );

	TRACE( "Requested URL: %s\n", pszURL );

	if ( _tcsnicmp( pszURL, _P( L"p2p-col:" ) ) == 0 )		// p2p-col:[//]{SHA1}/{relative path inside zip}
		return OnRequestCollection( SkipSlashes( pszURL, 8 ), oBuffer, sMimeType, bParseOnly );

	if ( _tcsnicmp( pszURL, _P( L"p2p-file:" ) ) == 0 )		// p2p-file:[//]{SHA1}/{preview|meta}
		return OnRequestFile( SkipSlashes( pszURL, 9 ), oBuffer, sMimeType, bParseOnly );

	if ( _tcsnicmp( pszURL, _P( L"p2p-app:" ) ) == 0 )		// p2p-app:[//]{history}
		return OnRequestApplication( SkipSlashes( pszURL, 8 ), oBuffer, sMimeType, bParseOnly );

	return INET_E_INVALID_URL;
}

HRESULT CIEProtocol::OnRequestCollection(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly)
{
	CString strURL = pszURL;
	CString strURN = strURL.SpanExcluding( L"/" );
	if ( strURN.IsEmpty() || _tcslen( pszURL ) < 30 )
		return INET_E_INVALID_URL;

	Hashes::Sha1Hash oSHA1;
	oSHA1.fromString( strURN );

	CSingleLock oLock( &Library.m_pSection, FALSE );
	if ( ! oLock.Lock( 500 ) )
		return INET_E_OBJECT_NOT_FOUND;

	// Render simple collection as HTML
	if ( oSHA1 )
	{
		CAlbumFolder* pCollAlbum = LibraryFolders.GetCollection( oSHA1 );
		if ( pCollAlbum )
		{
			CCollectionFile* pCollFile = pCollAlbum->GetCollection();
			if ( pCollFile && pCollFile->IsType( CCollectionFile::SimpleCollection ) )
			{
				if ( ! bParseOnly )
				{
					CString strBuffer;
					pCollFile->Render( strBuffer );
					oBuffer.Print( strBuffer, CP_UTF8 );
					sMimeType = L"text/html";
				}
				return S_OK;
			}
		}
	}

	// Load file directly from ZIP
	CLibraryFile* pCollFile = LibraryMaps.LookupFileBySHA1( oSHA1, FALSE, TRUE );
	if ( ! pCollFile )
	{
		// Load as sha1
		if ( ! oSHA1 )
			return INET_E_INVALID_URL;
		pCollFile = LibraryMaps.LookupFileBySHA1( oSHA1, FALSE, TRUE );
		if ( ! pCollFile )
			return INET_E_INVALID_URL;
	}

	CString strCollPath = pCollFile->GetPath();

	oLock.Unlock();

	CZIPFile oCollZIP;
	if ( ! oCollZIP.Open( strCollPath ) )
		return INET_E_OBJECT_NOT_FOUND;

	CString strPath = URLDecode( strURL.Mid( strURN.GetLength() + 1 ) );
	bool bDir = strPath.IsEmpty() || ( strPath.GetAt( strPath.GetLength() - 1 ) == L'/' );

	CString strFile = ( bDir ? ( strPath + L"index.htm" ) : strPath );
	CZIPFile::File* pFile = oCollZIP.GetFile( strFile, TRUE );
	if ( ! pFile )
	{
		if ( ! bDir )
			return INET_E_OBJECT_NOT_FOUND;

		strFile = strPath + L"collection.xml";
		pFile = oCollZIP.GetFile( strFile, TRUE );
		if ( ! pFile )
			return INET_E_OBJECT_NOT_FOUND;
	}

	CAutoPtr< CBuffer > pSource( pFile->Decompress() );
	if ( ! pSource )
		return INET_E_OBJECT_NOT_FOUND;

	if ( ! bParseOnly )
	{
		oBuffer.AddBuffer( pSource );
		sMimeType = ShellIcons.GetMIME( PathFindExtension( strFile ) );
	}

	return S_OK;
}

HRESULT CIEProtocol::OnRequestFile(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly)
{
	CString strURL = pszURL;
	CString strURN = strURL.SpanExcluding( L"/" );
	if ( strURN.IsEmpty() || _tcslen( pszURL ) < 31 )
		return INET_E_INVALID_URL;

	CString strVerb = URLDecode( strURL.Mid( strURN.GetLength() + 1 ) );
	if ( strVerb.IsEmpty() )
		return INET_E_INVALID_URL;

	CSingleLock oLock( &Library.m_pSection, FALSE );
	if ( ! oLock.Lock( 500 ) )
		return INET_E_OBJECT_NOT_FOUND;

	// Try to load as urn first
	CLibraryFile* pFile = LibraryMaps.LookupFileByURN( strURN, FALSE, TRUE );
	if ( ! pFile )
	{
		// Load as sha1
		Hashes::Sha1Hash oSHA1;
		if ( ! oSHA1.fromString( strURN ) )
			return INET_E_INVALID_URL;
		pFile = LibraryMaps.LookupFileBySHA1( oSHA1, FALSE, TRUE );
		if ( ! pFile )
			return INET_E_INVALID_URL;
	}

	if ( strVerb.CompareNoCase( L"preview" ) == 0 )
	{
		if ( bParseOnly )
			return S_OK;

		CImageFile pImage;
		if ( CThumbCache::Cache( pFile->GetPath(), &pImage ) )
		{
			CAutoVectorPtr< BYTE > pBuffer;
			DWORD nImageSize = 0;
			if ( pImage.SaveToMemory( L".jpg", 90, &pBuffer.m_p, &nImageSize ) )
			{
				oBuffer.Add( pBuffer, nImageSize );
				sMimeType = L"image/jpeg";
				return S_OK;
			}
		}
	}
	else if ( strVerb.CompareNoCase( L"meta" ) == 0 )
	{
		if ( bParseOnly )
			return S_OK;

		CString strXML;
		if ( pFile->m_pMetadata )
			strXML = pFile->m_pMetadata->ToString( TRUE, FALSE, TRUE );
		else
			strXML = L"<?xml version=\"1.0\"?>";
		oBuffer.Print( strXML, CP_UTF8 );
		sMimeType = L"text/xml";
		return S_OK;
	}
	else if ( strVerb.Left( 4 ).CompareNoCase( L"icon" ) == 0 )
	{
		if ( bParseOnly )
			return S_OK;

		int cx = max( min( _tstoi( strVerb.Mid( 4 ) ), 256 ), 16 );
		if ( HICON hIcon = ShellIcons.ExtractIcon( ShellIcons.Get( pFile->GetPath(), cx ), cx ) )
		{
			if ( SaveIcon( hIcon, oBuffer ) )
			{
				sMimeType = L"image/x-icon";
				DeleteObject( hIcon );
				return S_OK;
			}
			DeleteObject( hIcon );
		}
	}

	return INET_E_INVALID_URL;
}

HRESULT CIEProtocol::OnRequestApplication(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly)
{
	if ( _tcsnicmp( pszURL, _P( L"history" ) ) == 0 )
	{
		if ( bParseOnly )
			return S_OK;

		CString strXML;
		strXML.Format( L"<html>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n<style type=\"text/css\">\n"
			L"body { font-family: %s; font-size: %upx; margin: 0; padding: 0; background-color: %s; color: %s; }\n"
			L"h1 { font-size: 120%%; font-weight: bold; color: %s; background-color: %s; margin: 0; }\n"
			L"table { width: 100%%; font-size: 100%%; margin: 0; padding: 0; table-layout: fixed; }\n"
			L".name0 { width: 41%%; background-color: %s; color: %s; cursor: hand; }\n"
			L".time0 { width: 8%%; background-color: %s; text-align: right; }\n"
			L".name1 { width: 41%%; background-color: %s; color: %s; cursor: hand; }\n"
			L".time1 { width: 8%%; background-color: %s; text-align: right; }\n"
			L".icon { width: 16px; height: 16px; border-style: none; }\n"
			L"</style>\n</head>\n<body onmousemove=\"window.external.hover(''); event.cancel\">\n<h1> %s </h1>\n<table>\n",
			/*body*/	(LPCTSTR)Settings.Fonts.DefaultFont, Settings.Fonts.DefaultSize, ToCSSColor( Colors.m_crWindow ), ToCSSColor( Colors.m_crDisabled ),
			/*h1*/		ToCSSColor( Colors.m_crBannerText ), ToCSSColor( Colors.m_crBannerBack ),
			/*.name0*/	ToCSSColor( Colors.m_crSchemaRow[ 0 ] ), ToCSSColor( Colors.m_crTextLink ),
			/*.time0*/	ToCSSColor( Colors.m_crSchemaRow[ 0 ] ),
			/*.name1*/	ToCSSColor( Colors.m_crSchemaRow[ 1 ] ), ToCSSColor( Colors.m_crTextLink ),
			/*.time1*/	ToCSSColor( Colors.m_crSchemaRow[ 1 ] ),
			/*h1*/		(LPCTSTR)Escape( LoadString( IDS_LIBPANEL_RECENT_ADDITIONS ) ) );

		CSingleLock oLock( &Library.m_pSection, FALSE );
		if ( ! oLock.Lock( 500 ) )
			return INET_E_OBJECT_NOT_FOUND;

		int nCount = 0;
		for ( POSITION pos = LibraryHistory.GetIterator(); pos; )
		{
			const CLibraryRecent* pRecent = LibraryHistory.GetNext( pos );
			if ( ! pRecent->m_pFile )
				continue;

			CString strURN = pRecent->m_pFile->GetURN();
			if ( strURN.IsEmpty() )
				continue;

			CString strTime;
			SYSTEMTIME tAdded;
			FileTimeToSystemTime( &pRecent->m_tAdded, &tAdded );
			SystemTimeToTzSpecificLocalTime( NULL, &tAdded, &tAdded );
			GetDateFormat( LOCALE_USER_DEFAULT, NULL, &tAdded, L"ddd',' MMM dd", strTime.GetBuffer( 64 ), 64 );
			strTime.ReleaseBuffer();

			if ( ( nCount & 1 ) == 0 )
				strXML += L"<tr>";

			strXML.AppendFormat(
				L"<td class=\"name%d\" onclick=\"window.external.display('%s');\" onmousemove=\"window.external.hover('%s'); window.event.cancelBubble = true;\">"
				L"<img class=\"icon\" src=\"p2p-file://%s/icon16\"> %s </a></td>"
				L"<td class=\"time%d\"> %s </td>",
				( nCount & 2 ) >> 1, (LPCTSTR)Escape( strURN ), (LPCTSTR)Escape( strURN ),
				(LPCTSTR)Escape( strURN ), (LPCTSTR)Escape( pRecent->m_pFile->m_sName ),
				( nCount & 2 ) >> 1, (LPCTSTR)Escape( strTime ) );

			if ( ( nCount & 1 ) != 0 )
				strXML += L"</tr>\n";

			nCount++;
		}

		if ( nCount && ( nCount & 1 ) != 0 )
			strXML += L"<td></td><td></td></tr>\n";

		strXML += L"</table>\n</body>\n</html>";

		oBuffer.Print( strXML, CP_UTF8 );
		sMimeType = L"text/html";

		return S_OK;
	}

	return INET_E_INVALID_URL;
}

CString CIEProtocol::ToCSSColor(COLORREF rgb)
{
	CString strColor;
	strColor.Format( L"#%02x%02x%02x", GetRValue( rgb ), GetGValue( rgb ), GetBValue( rgb ) );
	return strColor;
}
