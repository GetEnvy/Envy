//
// RichElement.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2017
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
#include "RichDocument.h"
#include "RichElement.h"
#include "RichFragment.h"

#include "CoolInterface.h"
#include "ImageServices.h"
#include "ImageFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CRichElement construction

CRichElement::CRichElement(int nType, LPCTSTR pszText, LPCTSTR pszLink, DWORD nFlags, int nGroup)
	: m_pDocument	( NULL )
	, m_nType		( nType )
	, m_nFlags		( nFlags )
	, m_nGroup		( nGroup )
	, m_hImage		( NULL )
	, m_nImageIndex ( NULL )
{
	if ( m_nType == retHeading )
	{
		m_nType = retText;
		m_nFlags |= retfHeading;
	}

	if ( pszText )
	{
		if ( ( m_nType == retBitmap || m_nType == retIcon ) && HIWORD(pszText) == 0 )
			m_sText.Format( L"%Iu", (size_t)pszText );
		else
			m_sText = pszText;
	}

	if ( pszLink ) m_sLink = pszLink;
}

CRichElement::CRichElement(HBITMAP hBitmap, LPCTSTR pszLink, DWORD nFlags, int nGroup)
	: m_pDocument	( NULL )
	, m_nType		( retBitmap )
	, m_nFlags		( nFlags )
	, m_nGroup		( nGroup )
	, m_hImage		( (HANDLE)hBitmap )
	, m_nImageIndex ( NULL )
{
	if ( pszLink ) m_sLink = pszLink;
}

CRichElement::CRichElement(HICON hIcon, LPCTSTR pszLink, DWORD nFlags, int nGroup)
	: m_pDocument	( NULL )
	, m_nType		( retIcon )
	, m_nFlags		( nFlags )
	, m_nGroup		( nGroup )
	, m_hImage		( (HANDLE)hIcon )
	, m_nImageIndex ( NULL )
{
	if ( pszLink ) m_sLink = pszLink;
}

CRichElement::~CRichElement()
{
	if ( m_hImage != NULL )
	{
		if ( m_nType == retBitmap )
		{
			DeleteObject( (HBITMAP)m_hImage );
			m_hImage = NULL;
		}
		else if ( m_nType == retIcon )
		{
			DestroyIcon( (HICON)m_hImage );
			m_hImage = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CRichElement editing

void CRichElement::Show(BOOL bShow)
{
	if ( bShow == ( ( m_nFlags & retfHidden ) > 0 ) )
	{
		m_nFlags |= retfHidden;
		if ( bShow ) m_nFlags &= ~retfHidden;
		m_pDocument->m_nCookie++;
	}
}

void CRichElement::SetText(LPCTSTR pszText)
{
	if ( m_sText != pszText )
	{
		m_sText = pszText;
		m_pDocument->m_nCookie++;
	}
}

void CRichElement::SetFlags(DWORD nFlags, DWORD nMask)
{
	DWORD nNew = ( m_nFlags & ~nMask ) | ( nFlags & nMask );

	if ( nNew != m_nFlags )
	{
		m_nFlags = nNew;
		m_pDocument->m_nCookie++;
	}
}

//////////////////////////////////////////////////////////////////////
// CRichElement delete

void CRichElement::Delete()
{
	if ( m_pDocument ) m_pDocument->Remove( this );
	delete this;
}

//////////////////////////////////////////////////////////////////////
// CRichElement setup for paint

void CRichElement::PrePaint(CDC* pDC, BOOL bHover)
{
	ASSERT( m_pDocument->m_fntNormal.m_hObject != NULL );

	CFont* pFont = &m_pDocument->m_fntNormal;

	switch ( m_nType )
	{
	case retText:
		if ( m_nFlags & retfColor )
			pDC->SetTextColor( m_cColor );
		else
			pDC->SetTextColor( m_pDocument->m_crText );
		pFont = &m_pDocument->m_fntNormal;
		break;
	case retLink:
		if ( m_nFlags & retfColor )
			pDC->SetTextColor( m_cColor );
		else
			pDC->SetTextColor( bHover ? m_pDocument->m_crHover : m_pDocument->m_crLink );
		pFont = &m_pDocument->m_fntUnder;
		break;
	case retBitmap:
		PrePaintBitmap( pDC );
		pFont = NULL;
		break;
	case retIcon:
		PrePaintIcon( pDC );
		pFont = NULL;
		break;
	case retEmoticon:
		_stscanf( m_sText, L"%i", &m_nImageIndex );			// ToDo:
		m_hImage = NULL;
		pFont = NULL;
		break;
	case retCmdIcon:
		if ( UINT nID = CoolInterface.NameToID( m_sText ) )
		{
			m_nImageIndex = CoolInterface.ImageForID( nID );	// ToDo:
			m_hImage = NULL;
		}
		break;
	default:
		pFont = NULL;
		break;
	}

	if ( m_nFlags & retfBold )
	{
		pFont = ( m_nFlags & retfUnderline ) ? &m_pDocument->m_fntBoldUnder : &m_pDocument->m_fntBold;
	}
	else if ( m_nFlags & retfItalic )
	{
		pFont = &m_pDocument->m_fntItalic;
	}
	else if ( m_nFlags & retfUnderline )
	{
		pFont = &m_pDocument->m_fntUnder;
	}
	else if ( m_nFlags & retfHeading )
	{
		pFont = &m_pDocument->m_fntHeading;
		pDC->SetTextColor( m_pDocument->m_crHeading );
	}

	if ( pFont ) pDC->SelectObject( pFont );
}

void CRichElement::PrePaintBitmap(CDC* /*pDC*/)
{
	if ( m_hImage != NULL ) return;

	if ( _tcsnicmp( m_sText, L"res:", 4 ) == 0 )
	{
		UINT nID = 0;
		if ( _stscanf( (LPCTSTR)m_sText + 4, L"%u", &nID ) != 1 )
			return;

		m_hImage = CImageFile::LoadBitmapFromResource( nID );
	}
	else
	{
	//	CString strFile = Settings.General.Path + '\\' + m_sText;
	//
	//	CImageFile pFile;
	//	if ( ! pFile.LoadFromFile( strFile ) ) return;
	//	if ( ! pFile.EnsureRGB() ) return;	// ToDo: Support Alpha?
	//	m_hImage = pFile.CreateBitmap();

		m_hImage = CImageFile::LoadBitmapFromFile( Settings.General.Path + L'\\' + m_sText );
	}
}

void CRichElement::PrePaintIcon(CDC* /*pDC*/)
{
	if ( m_hImage != NULL || m_sText.IsEmpty() ) return;

	UINT nID = 0, nWidth = 16, nHeight = 16;
	_stscanf( m_sText, L"%u.%u.%u", &nID, &nWidth, &nHeight );
	//ASSERT( ( nWidth == 16 && nHeight == 16 ) || ( nWidth == 32 && nHeight == 32 ) );
	ASSERT( nWidth >= 16 );		// Allow for custom (re)sizes

	m_hImage = CoolInterface.ExtractIcon( nID, Settings.General.LanguageRTL,
		( nWidth < 20 ) ? LVSIL_SMALL : ( nWidth > 44 ) ? LVSIL_BIG : LVSIL_NORMAL );
	ASSERT( m_hImage != NULL );
}

//////////////////////////////////////////////////////////////////////
// CRichElement dimensions

CSize CRichElement::GetSize() const
{
	CSize sz( 0, 0 );

	if ( m_nType == retGap )
	{
		_stscanf( m_sText, L"%lu", &sz.cx );
	}
	else if ( m_nType == retBitmap && m_hImage != NULL )
	{
		BITMAP pInfo = {};
		GetObject( (HBITMAP)m_hImage, sizeof(pInfo), &pInfo );

		sz.cx = pInfo.bmWidth;
		sz.cy = pInfo.bmHeight;
	}
	else if ( m_nType == retIcon )
	{
		sz.cx = sz.cy = 16;

		ICONINFO ii;
		if ( GetIconInfo( (HICON)m_hImage, &ii ) )
		{
			BITMAP bmInfo = {};
			GetObject( ii.hbmColor, sizeof(BITMAP), &bmInfo );
			if ( bmInfo.bmWidth > 16 )
				sz.cx = bmInfo.bmWidth;
			if ( bmInfo.bmHeight > 16 )
				sz.cy = bmInfo.bmHeight;
			DeleteObject( ii.hbmColor );
			DeleteObject( ii.hbmMask );
		}

		UINT nID = 0;
		_stscanf( m_sText, L"%u.%li.%li", &nID, &sz.cx, &sz.cy );
	}
	else if ( m_nType == retEmoticon || m_nType == retCmdIcon )
	{
		sz.cx = sz.cy = 16;
	}
	else if ( m_nType == retAnchor )
	{
		sz.cx = sz.cy = 16;
		_stscanf( m_sText, L"%li.%li", &sz.cx, &sz.cy );
	}

	return sz;
}
