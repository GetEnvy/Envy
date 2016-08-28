//
// Emoticons.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2007
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
#include "Emoticons.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "XML.h"

#include "RichDocument.h"
#include "RichElement.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define EMOTICON_SIZE 16

CEmoticons Emoticons;


//////////////////////////////////////////////////////////////////////
// CEmoticons construction

CEmoticons::CEmoticons()
	: m_pTokens ( NULL )
{
}

CEmoticons::~CEmoticons()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CEmoticons find next token and index

LPCTSTR CEmoticons::FindNext(LPCTSTR pszText, int* pnIndex)
{
	LPCTSTR pszBest = NULL;
	int nIndex = 0, nBest = 0;

	if ( m_pTokens == NULL ) return NULL;

	for ( LPCTSTR pszToken = m_pTokens ; *pszToken ; nIndex++ )
	{
		LPCTSTR pszFind = _tcsstr( pszText, pszToken );

		if ( pszFind != NULL && ( pszBest == NULL || pszFind < pszBest ||
		   ( pszFind == pszBest && _tcslen( GetText( nBest ) ) < _tcslen( pszToken ) ) ) )
		{
			pszBest = pszFind;
			nBest = nIndex;
		}

		pszToken += _tcslen( pszToken ) + 1;
	}

	if ( pszBest && pnIndex ) *pnIndex = nBest;

	return pszBest;
}

//////////////////////////////////////////////////////////////////////
// CEmoticons lookup index from text

int CEmoticons::Lookup(LPCTSTR pszText, int nLen) const
{
	TCHAR cSave = 0;
	int nIndex = 0;

	if ( m_pTokens == NULL ) return -1;

	if ( nLen >= 0 )
	{
		cSave = pszText[ nLen ];
		( (LPTSTR)pszText )[ nLen ] = 0;
	}

	LPCTSTR pszToken = m_pTokens;
	for ( ; *pszToken ; nIndex++ )
	{
		if ( _tcscmp( pszToken, pszText ) == 0 )
			break;

		pszToken += _tcslen( pszToken ) + 1;
	}

	if ( nLen >= 0 ) ((LPTSTR)pszText)[ nLen ] = cSave;

	return ( *pszToken != 0 ) ? nIndex : -1;
}

//////////////////////////////////////////////////////////////////////
// CEmoticons get the text for an index

LPCTSTR CEmoticons::GetText(int nIndex) const
{
	if ( m_pTokens == NULL ) return NULL;

	for ( LPCTSTR pszToken = m_pTokens ; *pszToken ; )
	{
		if ( nIndex-- <= 0 ) return pszToken;

		pszToken += _tcslen( pszToken ) + 1;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CEmoticons draw

void CEmoticons::Draw(CDC* pDC, int nIndex, int nX, int nY, COLORREF crBack)
{
	if ( m_pTokens == NULL ) return;
	ImageList_DrawEx( m_pImage.m_hImageList, nIndex, pDC->GetSafeHdc(),
		nX, nY, 16, 16, crBack, CLR_DEFAULT, ILD_NORMAL );
	// if ( crBack != CLR_NONE ) pDC->ExcludeClipRect( nX, nY, nX + 16, nY + 16 );
}

//////////////////////////////////////////////////////////////////////
// CEmoticons menu

CMenu* CEmoticons::CreateMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->CreatePopupMenu();

	int nCount = 0;
	for ( int nPos = 0 ; nPos < m_pButtons.GetSize() ; nPos++ )
	{
		int nIndex = m_pButtons.GetAt( nPos );

		if ( nCount > 0 && ( nCount % 12 ) == 0 )
			pMenu->AppendMenu( MF_OWNERDRAW|MF_MENUBREAK, nIndex + 1, (LPCTSTR)NULL );
		else
			pMenu->AppendMenu( MF_OWNERDRAW, nIndex + 1, (LPCTSTR)NULL );

		nCount++;
	}

	return pMenu;
}

//////////////////////////////////////////////////////////////////////
// CEmoticons load

BOOL CEmoticons::Load()
{
	Clear();
	m_pImage.Create( EMOTICON_SIZE, EMOTICON_SIZE, ILC_COLOR32|ILC_MASK, 1, 8 ) ||
		m_pImage.Create( EMOTICON_SIZE, EMOTICON_SIZE, ILC_COLOR24|ILC_MASK, 1, 8 ) ||
		m_pImage.Create( EMOTICON_SIZE, EMOTICON_SIZE, ILC_COLOR16|ILC_MASK, 1, 8 );

	if ( ! LoadXML( Settings.General.DataPath + L"Emoticons.xml" ) )
		return FALSE;

	BuildTokens();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEmoticons clear

void CEmoticons::Clear()
{
	if ( m_pImage.m_hImageList != NULL ) m_pImage.DeleteImageList();

	if ( m_pTokens != NULL ) delete [] m_pTokens;
	m_pTokens = NULL;

	m_pIndex.RemoveAll();
	m_pButtons.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CEmoticons add an emoticon

int CEmoticons::AddEmoticon(LPCTSTR pszText, CImageFile* pImage, CRect* pRect, COLORREF crBack, BOOL bButton)
{
	ASSERT( pImage->m_bLoaded && pImage->m_nComponents == 3 );	// ToDo: Allow alpha?

	if ( pRect->left < 0 || pRect->left + EMOTICON_SIZE > pImage->m_nWidth ) return -1;
	if ( pRect->top < 0 || pRect->top > pImage->m_nHeight + EMOTICON_SIZE ) return -1;
	if ( pRect->right != pRect->left + EMOTICON_SIZE ) return -1;
	if ( pRect->bottom != pRect->top + EMOTICON_SIZE ) return -1;

	DWORD nPitch = pImage->m_nWidth * pImage->m_nComponents;
	while ( nPitch & 3 ) nPitch++;

	BYTE* pSource = pImage->m_pImage;
	pSource += pRect->top * nPitch + pRect->left * pImage->m_nComponents;

	HDC hDCMem1, hDCMem2;
	if ( HDC hDC = GetDC( NULL ) )	// Get screen DC
	{
		hDCMem1 = CreateCompatibleDC( hDC );	// Create memory DC for the source
		if ( ! hDCMem1 )
		{
			ReleaseDC( NULL, hDC );
			return -1;
		}

		hDCMem2 = CreateCompatibleDC( hDC );	// Create memory DC for the destination
		if ( ! hDCMem2 )
		{
			DeleteDC( hDCMem1 );
			ReleaseDC( NULL, hDC );
			return -1;
		}

		CBitmap bmOriginal, bmMoved;
		CDC* pDC = CDC::FromHandle( hDC );

		if ( ! bmOriginal.CreateCompatibleBitmap( pDC, EMOTICON_SIZE, EMOTICON_SIZE ) )	// Source bitmap
		{
			ReleaseDC( NULL, hDC );
			DeleteDC( hDCMem1 );
			DeleteDC( hDCMem2 );
			return -1;
		}

		if ( ! bmMoved.CreateCompatibleBitmap( pDC, EMOTICON_SIZE, EMOTICON_SIZE ) )	// Destination bitmap
		{
			ReleaseDC( NULL, hDC );
			DeleteDC( hDCMem1 );
			DeleteDC( hDCMem2 );
			bmOriginal.DeleteObject();
			return -1;
		}

		BITMAPINFOHEADER pInfo;
		pInfo.biSize		= sizeof( BITMAPINFOHEADER );
		pInfo.biWidth		= EMOTICON_SIZE;
		pInfo.biHeight		= EMOTICON_SIZE;
		pInfo.biPlanes		= 1;
		pInfo.biBitCount	= 24;
		pInfo.biCompression	= BI_RGB;
		pInfo.biSizeImage	= EMOTICON_SIZE * EMOTICON_SIZE * 3;

		for ( int nY = EMOTICON_SIZE - 1 ; nY >= 0 ; nY-- )
		{
			SetDIBits( hDCMem1, bmOriginal, nY, 1, pSource, (BITMAPINFO*)&pInfo, DIB_RGB_COLORS );
			pSource += nPitch;
		}

		HBITMAP hOld_bm1, hOld_bm2;
		hOld_bm1 = (HBITMAP)SelectObject( hDCMem1, bmOriginal.m_hObject );
		hOld_bm2 = (HBITMAP)SelectObject( hDCMem2, bmMoved.m_hObject );

		if ( Settings.General.LanguageRTL )
			SetLayout( hDCMem2, LAYOUT_RTL );
		StretchBlt( hDCMem2, 0, 0, EMOTICON_SIZE, EMOTICON_SIZE, hDCMem1, 0, 0, EMOTICON_SIZE, EMOTICON_SIZE, SRCCOPY );

		SelectObject( hDCMem1, hOld_bm1 );
		SelectObject( hDCMem2, hOld_bm2 );
		DeleteDC( hDCMem1 );
		DeleteDC( hDCMem2 );
		ReleaseDC( NULL, hDC );
		int nIndex = m_pImage.Add( &bmMoved, crBack );
		bmMoved.DeleteObject();
		bmOriginal.DeleteObject();

		m_pIndex.Add( pszText );
		if ( bButton )
			m_pButtons.Add( nIndex );

		return nIndex;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////
// CEmoticons build tokens

void CEmoticons::BuildTokens()
{
	int nLength = 2;

	for ( int nIndex = 0 ; nIndex < m_pIndex.GetSize() ; nIndex++ )
	{
		nLength += m_pIndex.GetAt( nIndex ).GetLength() + 1;
	}

	delete [] m_pTokens;
	m_pTokens = new TCHAR[ nLength ];

	DWORD nStart = 0;
	for ( int nIndex = 0 ; nIndex < m_pIndex.GetSize() ; nIndex++ )
	{
		_tcscpy_s( &m_pTokens[ nStart ], nLength - nStart, m_pIndex.GetAt( nIndex ) );
		nStart += m_pIndex.GetAt( nIndex ).GetLength() + 1;
	}
	m_pTokens[ nStart ] = 0;
}

//////////////////////////////////////////////////////////////////////
// CEmoticons load Trillian XML

BOOL CEmoticons::LoadXML(LPCTSTR pszFile)
{
	CString strPath, strValue;

	CXMLElement* pXML = CXMLElement::FromFile( pszFile, TRUE );
	if ( pXML == NULL ) return FALSE;

	strPath = pszFile;
	int nSlash = strPath.ReverseFind( L'\\' );
	if ( nSlash >= 0 ) strPath = strPath.Left( nSlash + 1 );

	CXMLElement* pBitmap = pXML->GetElementByName( L"bitmap" );

	if ( pBitmap == NULL )
	{
		delete pXML;
		return FALSE;
	}

	strValue = pBitmap->GetAttributeValue( L"file" );

	nSlash = strValue.ReverseFind( L'/' );
	if ( nSlash >= 0 ) strValue = strValue.Mid( nSlash + 1 );
	strValue = strPath + strValue;

	CImageFile pImage;

	if ( ! pImage.LoadFromFile( strValue ) ||
		 ! pImage.EnsureRGB( GetSysColor( COLOR_WINDOW ) ) ||
		 ! pImage.SwapRGB() )
	{
		delete pXML;
		return FALSE;
	}

	COLORREF crBack = RGB( pImage.m_pImage[2], pImage.m_pImage[1], pImage.m_pImage[0] );

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pEmoticon = pXML->GetNextElement( pos );
		if ( ! pEmoticon->IsNamed( L"emoticon" ) ) continue;

		CXMLElement* pSource = pEmoticon->GetElementByName( L"source" );
		CString strText = pEmoticon->GetAttributeValue( L"text" );
		CRect rc( 0, 0, 0, 0 );

		strValue = pSource->GetAttributeValue( L"left", L"0" );
		_stscanf( strValue, L"%li", &rc.left );
		strValue = pSource->GetAttributeValue( L"top", L"0" );
		_stscanf( strValue, L"%li", &rc.top );
		strValue = pSource->GetAttributeValue( L"right", L"0" );
		_stscanf( strValue, L"%li", &rc.right );
		strValue = pSource->GetAttributeValue( L"bottom", L"0" );
		_stscanf( strValue, L"%li", &rc.bottom );

		BOOL bButton = pEmoticon->GetAttributeValue( L"button" ).CompareNoCase( L"yes" ) == 0;

		AddEmoticon( strText, &pImage, &rc, crBack, bButton );
	}

	delete pXML;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEmoticons rich text formatting

void CEmoticons::FormatText(CRichDocument* pDocument, LPCTSTR pszBody, BOOL bNewlines, COLORREF cr)
{
	static LPCTSTR pszURLs[] = { L"\r", L"\n", L"http://", L"https://", L"ftp://", L"mailto:", L"aim:", L"www.", L"magnet:?", L"ed2k://", L"dchub://", L"gnutella:", L"gnutella1:", L"gnutella2:", L"g2://", L"gnet:", L"envy:", L"peer:", L"peerproject:", L"shareaza:", L"raza:", L"gwc:", L"uhc:", L"ukhl:", L"mp2p:", L"sig2dat:", NULL };
	BOOL bBold = FALSE, bItalic = FALSE, bUnderline = FALSE;
	CString str;

	while ( *pszBody )
	{
		LPCTSTR pszToken = _tcschr( pszBody, '[' );

		for ( int nURL = 0 ; pszURLs[ nURL ] != NULL ; nURL++ )
		{
			LPCTSTR pszFind = _tcsistr( pszBody, pszURLs[ nURL ] );
			if ( pszFind != NULL && ( pszToken == NULL || pszFind < pszToken ) ) pszToken = pszFind;
		}

		int nEmoticon = -1;
		LPCTSTR pszEmoticon = FindNext( pszBody, &nEmoticon );

		if ( pszEmoticon != NULL && ( pszToken == NULL || pszEmoticon < pszToken ) )
			pszToken = pszEmoticon;

		if ( pszToken != pszBody )
		{
			if ( pszToken != NULL )
			{
				TCHAR cSave = *pszToken;
				*(LPTSTR)pszToken = 0;
				str = pszBody;
				*(LPTSTR)pszToken = cSave;
			}
			else
			{
				str = pszBody;
			}

			pDocument->Add( retText, str, NULL,
				( bBold ? retfBold : 0 ) |
				( bItalic ? retfItalic : 0 ) |
				( bUnderline ? retfUnderline : 0 ) |
				( cr ? retfColor : 0 ) )->m_cColor = cr;
		}

		if ( pszToken == NULL ) break;

		pszBody = pszToken;
		if ( *pszBody == 0 ) break;

		if ( pszEmoticon == pszBody )
		{
			str.Format( L"%i", nEmoticon );
			pDocument->Add( retEmoticon, str );
			pszBody += _tcslen( GetText( nEmoticon ) );
			continue;
		}
		else if ( pszBody[0] == '\r' || pszBody[0] == '\n' )
		{
			if ( bNewlines )
				pDocument->Add( retNewline, L"4" );

			pszBody ++;
			continue;
		}
		else if ( *pszBody != '[' )
		{
			for ( ; *pszToken ; pszToken++ )
			{
				if ( ! _istalnum( *pszToken ) &&
					_tcschr( L":@/?=&%._-+;~#", *pszToken ) == NULL )
				{
					break;
				}
			}

			TCHAR cSave = *pszToken;
			*(LPTSTR)pszToken = 0;
			str = pszBody;
			*(LPTSTR)pszToken = cSave;

			if ( _tcsnicmp( str, L"www.", 4 ) == 0 )
				str = L"http://" + str;

			pDocument->Add( retLink, str, str,
				( bBold ? retfBold : 0 ) |
				( bItalic ? retfItalic : 0 ) |
				( bUnderline ? retfUnderline : 0 ) );

			pszBody = pszToken;
		}
		else if ( _tcsnicmp( pszBody, L"[b]", 3 ) == 0 )
		{
			bBold = TRUE;
		}
		else if ( _tcsnicmp( pszBody, L"[/b]", 4 ) == 0 )
		{
			bBold = FALSE;
		}
		else if ( _tcsnicmp( pszBody, L"[i]", 3 ) == 0 )
		{
			bItalic = TRUE;
		}
		else if ( _tcsnicmp( pszBody, L"[/i]", 4 ) == 0 )
		{
			bItalic = FALSE;
		}
		else if ( _tcsnicmp( pszBody, L"[u]", 3 ) == 0 )
		{
			bUnderline = TRUE;
		}
		else if ( _tcsnicmp( pszBody, L"[/u]", 4 ) == 0 )
		{
			bUnderline = FALSE;
		}
		else if ( _tcsnicmp( pszBody, L"[/c]", 4 ) == 0 )
		{
			cr = 0;
		}
		else if ( _tcsnicmp( pszBody, L"[c:#", 4 ) == 0 && _tcslen( pszBody ) >= 4 + 6 + 1 )
		{
			_tcsncpy_s( str.GetBuffer( 7 ), 7, pszBody + 4, 6 );
			str.ReleaseBuffer( 6 );
			int nRed = 0, nGreen = 0, nBlue = 0;
			_stscanf( str.Mid( 0, 2 ), L"%x", &nRed );
			_stscanf( str.Mid( 2, 2 ), L"%x", &nGreen );
			_stscanf( str.Mid( 4, 2 ), L"%x", &nBlue );
			cr = RGB( nRed, nGreen, nBlue );
		}

		if ( *pszBody == '[' )
		{
			pszToken = _tcschr( pszBody, ']' );
			if ( pszToken != NULL ) pszBody = pszToken + 1;
			else pszBody ++;
		}
	}
}
