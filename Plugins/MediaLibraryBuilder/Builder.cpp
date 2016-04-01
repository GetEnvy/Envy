//
// Builder.cpp : Implementation of CBuilder
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Nikolay Raspopov 2005-2007
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "Builder.h"
//#include <strsafe.h>

STDMETHODIMP CBuilder::Process(
	/*[in]*/ BSTR sFile,
	/*[in]*/ ISXMLElement* pXML)
{
	HRESULT hr = E_FAIL;
	__try
	{
		hr = SafeProcess( sFile, pXML );
	}
	__except( GetExceptionCode() != EXCEPTION_CONTINUE_EXECUTION )
	{
		return E_FAIL;
	}
	return hr;
}

HRESULT CBuilder::SafeProcess(BSTR sFile, ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements(&pISXMLRootElements);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create(CComBSTR("videos"), &pXMLRootElement);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes(&pISXMLRootAttributes);
	if ( FAILED( hr ) )
		return hr;
	pISXMLRootAttributes->Add( CComBSTR("xmlns:xsi"),
		CComBSTR("http://www.w3.org/2001/XMLSchema-instance") );
	pISXMLRootAttributes->Add( CComBSTR("xsi:noNamespaceSchemaLocation"),
		CComBSTR("http://schemas.getenvy.com/Video.xsd") );

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements(&pISXMLElements);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create(CComBSTR("video"), &pXMLElement);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes(&pISXMLAttributes);
	if ( FAILED( hr ) )
		return hr;

	CComPtr< IMediaDet > pDet;
	hr = pDet.CoCreateInstance( CLSID_MediaDet );
	if ( FAILED( hr ) )
	{
		ATLTRACE( "Cannot instantiate MediaDet object: 0x%08x\n", hr );
		return hr;
	}

	hr = pDet->put_Filename(sFile);
	if ( FAILED( hr ) )
	{
		ATLTRACE( "Cannot open file: 0x%08x\n", hr );
		return hr;
	}

	long lStreams = 0;
	hr = pDet->get_OutputStreams(&lStreams);
	if ( FAILED( hr ) )
	{
		ATLTRACE( "Cannot get streams: 0x%08x\n", hr );
		return hr;
	}

	CString strTemp;
	bool bFound = false;

	AM_MEDIA_TYPE mt = {};
	for ( long i = 0; i < lStreams; i++ )
	{
		hr = pDet->put_CurrentStream(i);
		if ( SUCCEEDED( hr ) )
		{
			GUID major_type = {};
			hr = pDet->get_StreamType(&major_type);
			if ( major_type == MEDIATYPE_Video )
			{
				hr = pDet->get_StreamMediaType(&mt);
				if ( SUCCEEDED( hr ) &&
					mt.formattype == FORMAT_VideoInfo &&
					mt.cbFormat >= sizeof(VIDEOINFOHEADER) &&
					mt.pbFormat != NULL )
				{
					bFound = true;
					break;
				}

				// ! bFound
				if ( mt.cbFormat != 0 )
					CoTaskMemFree(mt.pbFormat);
				if ( mt.pUnk != NULL )
					mt.pUnk->Release();
				ZeroMemory( &mt, sizeof(AM_MEDIA_TYPE) );
			}
			//else if ( major_type == MEDIATYPE_Audio )
		}
	}

	if ( bFound )	// Video stream
	{
		// ToDo: Get proper codec info - http://msdn.microsoft.com/library/windows/desktop/dd373477.aspx

		CString strCodec;

		const VIDEOINFOHEADER *pVih = (const VIDEOINFOHEADER*)mt.pbFormat;

		if ( DWORD nCodec = pVih->bmiHeader.biCompression )
		{
			strCodec.Format( L"%c%c%c%c",
				LOBYTE(LOWORD(nCodec)),
				HIBYTE(LOWORD(nCodec)),
				LOBYTE(HIWORD(nCodec)),
				HIBYTE(HIWORD(nCodec)) );
		}
		else if ( mt.subtype == MEDIASUBTYPE_H264 )
		{
			strCodec = L"H264";
		}
		else if ( mt.subtype == MEDIASUBTYPE_Avi )
		{
			strCodec = L"AVI";
		}
		else if ( mt.subtype == MEDIASUBTYPE_MPEG1Video )
		{
			strCodec = L"MPEG1";
		}
		else if ( mt.subtype == MEDIASUBTYPE_MJPG )
		{
			strCodec = L"MJPG";
		}
		else if (	// GUID DDCCBBAA-0000-0010-8000-00AA00389B71
			mt.subtype.Data2 == 0x0000 &&
			mt.subtype.Data3 == 0x0010 &&
			mt.subtype.Data4[0] == 0x80 &&
			mt.subtype.Data4[1] == 0x00 &&
			mt.subtype.Data4[2] == 0x00 &&
			mt.subtype.Data4[3] == 0xAA &&
			mt.subtype.Data4[4] == 0x00 &&
			mt.subtype.Data4[5] == 0x38 &&
			mt.subtype.Data4[6] == 0x9B &&
			mt.subtype.Data4[7] == 0x71 )
		{
			strCodec.Format(L"%c%c%c%c",
				LOBYTE(LOWORD(mt.subtype.Data1)),
				HIBYTE(LOWORD(mt.subtype.Data1)),
				LOBYTE(HIWORD(mt.subtype.Data1)),
				HIBYTE(HIWORD(mt.subtype.Data1)));
		}

//		// Video Format (Not a codec)
//		if ( mt.subtype == MEDIASUBTYPE_Y41P )
//		{
//			strCodec = L"MPEG";
//		}
//		else
//		{
//#ifdef _DEBUG
//			LPWSTR clsid = NULL;
//			if ( SUCCEEDED( StringFromCLSID( mt.subtype, &clsid ) ) )
//			{
//				ATLTRACE( "Video format: %s\n", CW2A( clsid ) );
//				CoTaskMemFree(clsid);
//			}
//#endif // _DEBUG
//			codec = L"Unknown";
//		}

		if ( ! strCodec.IsEmpty() )
		{
			if ( strCodec.Compare( L"Y41P" ) == 0 )
				strCodec = L"MPEG1";
			else if ( strCodec.Compare( L"NV12" ) == 0 )
				strCodec.Empty();	// ToDo: Why common MEDIASUBTYPE_NV12 result?

			pISXMLAttributes->Add( CComBSTR("codec"), CComBSTR(strCodec) );
		}

		int nWidth  = pVih->bmiHeader.biWidth;
		int nHeight = pVih->bmiHeader.biHeight;
		if ( nHeight < 0 )
			nHeight = -nHeight;
		if ( nHeight > 6000 )
			nHeight = 0;
		strTemp.Format( L"%lu", nWidth );
		pISXMLAttributes->Add( CComBSTR("width"), CComBSTR(strTemp) );
		strTemp.Format( L"%lu", nHeight );
		pISXMLAttributes->Add( CComBSTR("height"), CComBSTR(strTemp) );

		if ( mt.cbFormat )
			CoTaskMemFree( mt.pbFormat );
		if ( mt.pUnk )
			mt.pUnk->Release();
	}

	double total_time = 0.0;
	hr = pDet->get_StreamLength(&total_time);
	if ( SUCCEEDED( hr ) && total_time != .0 )
	{
		StringCbPrintf( strTemp.GetBuffer( 32 ), 32 * sizeof( TCHAR ),
			L"%.3f", total_time / 60.0 );
		strTemp.ReleaseBuffer();
		pISXMLAttributes->Add( CComBSTR("minutes"), CComBSTR(strTemp) );
	}

	double fps = 0.0;
	hr = pDet->get_FrameRate(&fps);
	if ( SUCCEEDED( hr ) && fps != .0 )
	{
		StringCbPrintf( strTemp.GetBuffer( 32 ), 32 * sizeof( TCHAR ),
			L"%.2f", fps );
		strTemp.ReleaseBuffer();
		pISXMLAttributes->Add( CComBSTR("frameRate"), CComBSTR(strTemp) );
	}

	return S_OK;
}
