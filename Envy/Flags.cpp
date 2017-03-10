//
// Flags.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
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
#include "Flags.h"
#include "Skin.h"
#include "ImageServices.h"
#include "ImageFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Legacy defaults, use Flags.Height & Flags.Width
//#define SOURCE_FLAG_WIDTH 	18
//#define SOURCE_FLAG_HEIGHT	12
//#define IMAGELIST_FLAG_WIDTH	FLAG_WIDTH	// 18
//#define IMAGELIST_FLAG_HEIGHT	16			// Settings.Skin.RowSize - 1


CFlags Flags;

CFlags::CFlags()
{
}

CFlags::~CFlags()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CFlags load

BOOL CFlags::Load()
{
	Clear();

	const CString strDefault = Settings.General.DataPath + L"Flags.png";

	CImageFile pImage;
	HBITMAP hBitmap = Skin.GetWatermark( L"CFlags" );
	if ( ! hBitmap ||
		 ! pImage.LoadFromBitmap( hBitmap ) ||
		 pImage.m_nWidth < 6 * 26 ||
		 pImage.m_nHeight < 6 * 26 ||
		 pImage.m_nHeight > 32 * 26 ||
		 pImage.m_nWidth % 26 != 0 ||
		 pImage.m_nHeight % 26 != 0 ||
		 ! pImage.EnsureRGB( GetSysColor( COLOR_WINDOW ) ) ||
		 ! pImage.SwapRGB() )
	{
		if ( ! pImage.LoadFromFile( strDefault ) ||
			pImage.m_nWidth < 6 * 26 ||
			pImage.m_nHeight < 6 * 26 ||
			pImage.m_nHeight > 32 * 26 ||
		//	pImage.m_nWidth % 26 != 0 ||
		//	pImage.m_nHeight % 26 != 0 ||
			! pImage.EnsureRGB( GetSysColor( COLOR_WINDOW ) ) ||
			! pImage.SwapRGB() )
		{
			return FALSE;
		}
	}

	Height = pImage.m_nHeight / 26;
	Width  = pImage.m_nWidth / 26;
	m_nImagelistWidth  = max( Width, 16 );
	m_nImagelistHeight = max( Height, ( Settings.Skin.RowSize > 17 ? (int)Settings.Skin.RowSize - 1 : 16 ) );

	m_pImage.Create( m_nImagelistWidth, m_nImagelistHeight, ILC_COLOR32|ILC_MASK, 26 * 26, 8 ) ||
	m_pImage.Create( m_nImagelistWidth, m_nImagelistHeight, ILC_COLOR24|ILC_MASK, 26 * 26, 8 ) ||
	m_pImage.Create( m_nImagelistWidth, m_nImagelistHeight, ILC_COLOR16|ILC_MASK, 26 * 26, 8 );

	const COLORREF crMask = RGB( 0, 255, 0 );

	for ( int i = 0 ; i < 26 ; i++ )
	{
		for ( int j = 0 ; j < 26 ; j++ )
		{
			CRect rc( i * Width, j * Height, i * Width + Width, j * Height + Height );
			AddFlag( &pImage, &rc, crMask );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFlags add a flag

void CFlags::AddFlag(CImageFile* pImage, CRect* pRect, COLORREF crBack)
{
	//ASSERT( pImage->m_bLoaded && pImage->m_nComponents == 3 );	// Allow alpha?
	//ASSERT( pRect->left >= 0 && pRect->left + SOURCE_FLAG_WIDTH <= pImage->m_nWidth );
	//ASSERT( pRect->top >= 0 && pRect->top <= pImage->m_nHeight + SOURCE_FLAG_HEIGHT );
	//ASSERT( pRect->right == pRect->left + SOURCE_FLAG_WIDTH );
	//ASSERT( pRect->bottom == pRect->top + SOURCE_FLAG_HEIGHT );

	DWORD nPitch = pImage->m_nWidth * pImage->m_nComponents;
	while ( nPitch & 3 ) nPitch++;

	BYTE* pSource = pImage->m_pImage;
	pSource += pRect->top * nPitch + pRect->left * pImage->m_nComponents;

	HDC hDC = GetDC( NULL );					// Get screen DC
	HDC hDCMem1 = CreateCompatibleDC( hDC );	// Create source memory DC
	HDC hDCMem2 = CreateCompatibleDC( hDC );	// Create destination memory DC

	CBitmap bmOriginal, bmMoved;
	CDC* pDC = CDC::FromHandle( hDC );

	bmOriginal.CreateCompatibleBitmap( pDC, Width, Height );	// Source bitmap
	bmMoved.CreateCompatibleBitmap( pDC, m_nImagelistWidth, m_nImagelistHeight );	// Destination bitmap

	BITMAPINFOHEADER pInfo = {};
	pInfo.biSize		= sizeof( BITMAPINFOHEADER );
	pInfo.biWidth		= Width;
	pInfo.biHeight		= Height;
	pInfo.biPlanes		= 1;
	pInfo.biBitCount	= 24;
	pInfo.biCompression	= BI_RGB;
	pInfo.biSizeImage	= Width * Height * 3;

	for ( int nY = Height - 1 ; nY >= 0 ; nY-- )
	{
		SetDIBits( hDCMem1, bmOriginal, nY, 1, pSource, (BITMAPINFO*)&pInfo, DIB_RGB_COLORS );
		pSource += nPitch;
	}

	HBITMAP hOld_bm1 = (HBITMAP)SelectObject( hDCMem1, bmOriginal.m_hObject );
	HBITMAP hOld_bm2 = (HBITMAP)SelectObject( hDCMem2, bmMoved.m_hObject );
	CDC* pDC2 = CDC::FromHandle( hDCMem2 );
	pDC2->SetBkMode( TRANSPARENT );
	pDC2->FillSolidRect( 0, 0, m_nImagelistWidth, m_nImagelistHeight, crBack );

	if ( Settings.General.LanguageRTL )
		SetLayout( hDCMem2, LAYOUT_RTL );

	int nOffset = ( m_nImagelistHeight - Height ) / 2;
	StretchBlt( hDCMem2, 0, nOffset, Width, Height,
				hDCMem1, 0, 0, Width, Height, SRCCOPY );

	SelectObject( hDCMem1, hOld_bm1 );
	SelectObject( hDCMem2, hOld_bm2 );
	VERIFY( DeleteDC( hDCMem1 ) );
	VERIFY( DeleteDC( hDCMem2 ) );
	ReleaseDC( NULL, hDC );
	m_pImage.Add( &bmMoved, crBack );
	bmMoved.DeleteObject();
	bmOriginal.DeleteObject();
}

//////////////////////////////////////////////////////////////////////
// CFlags clear

void CFlags::Clear()
{
	if ( m_pImage.m_hImageList )
		m_pImage.DeleteImageList();
}

int CFlags::GetCount() const
{
	if ( m_pImage.m_hImageList )
		return m_pImage.GetImageCount();

	return NULL;	// Startup
}

int CFlags::GetFlagIndex(const CString& sCountryCode) const
{
	if ( sCountryCode.GetLength() != 2 )
		return -1;

	char nFirstLetter  = (char)( sCountryCode[0] - 'A' );
	char nSecondLetter = (char)( sCountryCode[1] - 'A' );
	// Currently only the letters A-Z are in the flag matrix
	// but GeoIP can also return some combinations that aren't all letters (A1, A2, etc.)
	if ( nFirstLetter >= 0 && nFirstLetter < 26 && nSecondLetter >= 0 && nSecondLetter < 26 )
		return nFirstLetter * 26 + nSecondLetter;
	if ( nFirstLetter == 0 && nSecondLetter < 0 )
		return 0;

	return -1;
}

HICON CFlags::ExtractIcon(int i)
{
	return m_pImage.ExtractIcon( i );
}

BOOL CFlags::Draw(int i, HDC hdcDst, int x, int y, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
{
	return ImageList_DrawEx( m_pImage, i, hdcDst, x, y,
		m_nImagelistWidth, m_nImagelistHeight, rgbBk, rgbFg, fStyle );
}
