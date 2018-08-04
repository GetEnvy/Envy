//
// QRCode.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// All work here is original and released as-is under Persistent Public Domain [PPD]
//

// LibQREncode DLL Interface

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "QRCode.h"

extern "C"
{
	#include "qrencode.h"
	#include "qrencode.c"
	//#include <QREncode\qrencode.h>	// Full DLL
}

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#define new DEBUG_NEW
//#endif	// Debug

//#define REMOTECODE_SIZE 116

CQRCode QREncode;


//////////////////////////////////////////////////////////////////////
// CQRCode construction

CQRCode::CQRCode()
{
}

//CQRCode::~CQRCode()
//{
//}

//////////////////////////////////////////////////////////////////////
// CQRCode Bitmap Objects

HBITMAP CQRCode::GetBitmap(CString str, UINT nPixelsize /*4*/)
{
	QRcode* pQRC = QRcode_encodeString( (CStringA)str );
	if ( ! pQRC )
		return NULL;

	const UINT nUnitWidth = pQRC->width;	// Not REMOTECODE_SIZE
	UINT nBitWidth = nUnitWidth * nPixelsize * 3;
	if ( nBitWidth % 4 )
		nBitWidth = ( nBitWidth / 4 + 1 ) * 4;
	const UINT nDataBytes = nBitWidth * nBitWidth;

	BITMAPINFO bmi;
	//bmi.bmiColors[0] = { 0xFF,0xFF,0xFF,0 };		// BGRA
	//bmi.bmiColors[1] = { 0,0,0,0 };
	//BITMAPINFOHEADER bmiHeader;
	BITMAPINFOHEADER& bmiHeader = bmi.bmiHeader;
	bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 24;
	bmiHeader.biWidth = nUnitWidth * nPixelsize;	// REMOTECODE_SIZE
	bmiHeader.biHeight = nUnitWidth * nPixelsize;	// REMOTECODE_SIZE
	//bmi.bmiHeader = bmiHeader;

	// Convert QrCode bits to bmp pixels
	unsigned char* pDestData;
	unsigned char* pSourceData = pQRC->data;
	unsigned char* pRGBData = (unsigned char*)malloc( nDataBytes );
	memset( pRGBData, 0xff, nDataBytes );

	// ToDo: Fix resulting image appears rotated 90° counter-clockwise

	for( UINT y = 0 ; y < nUnitWidth ; y++ )
	{
		pDestData = pRGBData + nBitWidth * y * nPixelsize;
		for( UINT  x = 0 ; x < nUnitWidth ; x++ )
		{
			if ( *pSourceData & 1 )
			{
				for( UINT l = 0 ; l < nPixelsize ; l++ )
				{
					for( UINT n = 0 ; n < nPixelsize ; n++ )
					{
						*(pDestData +     n * 3 + nBitWidth * l) = 0;	// PIXEL_COLOR_B
						*(pDestData + 1 + n * 3 + nBitWidth * l) = 0;	// PIXEL_COLOR_G
						*(pDestData + 2 + n * 3 + nBitWidth * l) = 0;	// PIXEL_COLOR_R
					}
				}
			}
			pDestData += 3 * nPixelsize;
			pSourceData++;
		}
	}

	HBITMAP hbitmap = CreateDIBitmap( GetDC( 0 ), &bmiHeader, CBM_INIT, pRGBData, &bmi, DIB_RGB_COLORS );

	FILE* file;
	if ( !( fopen_s( &file, (CStringA)( Settings.General.DataPath + L"QRCodeRemote.bmp" ), "wb" ) ) )
	{
		BITMAPFILEHEADER FileHeader = {};
		FileHeader.bfType = 0x4d42;  // "BM"
		FileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nDataBytes;
		FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		FileHeader.bfReserved1 = 0;
		FileHeader.bfReserved2 = 0;

		fwrite( &FileHeader, sizeof(BITMAPFILEHEADER), 1, file );
		fwrite( &bmiHeader, sizeof(BITMAPINFOHEADER), 1, file );
		fwrite( pRGBData, sizeof(unsigned char), ( bmiHeader.biWidth * bmiHeader.biHeight * 3 ), file );
		fclose( file );
	}

	QRcode_free( pQRC );
	free( pRGBData );

	return hbitmap;
}

void CQRCode::UpdateRemote(CString str)
{
	static CString strCurrent;

	// 53 char max for 29x29 code (version 3)
	if ( str.GetLength() > 52 )
		str = str.Left( str.Find( L'?' ) );
	ASSERT( str.GetLength() <= 52 );

	if ( strCurrent == str ) return;

	if ( HBITMAP hbm = GetBitmap( str ) )
	{
		if ( m_bmRemote.m_hObject )
			m_bmRemote.DeleteObject();
		m_bmRemote.Attach( hbm );
		strCurrent == str;
	}
}
