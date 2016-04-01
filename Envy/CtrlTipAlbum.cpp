//
// CtrlTipAlbum.cpp
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
#include "CtrlTipAlbum.h"

#include "Library.h"
#include "LibraryFolders.h"
#include "AlbumFolder.h"
#include "Schema.h"
#include "Colors.h"
#include "CoolInterface.h"
#include "ShellIcons.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CAlbumTipCtrl, CCoolTipCtrl)

//BEGIN_MESSAGE_MAP(CAlbumTipCtrl, CCoolTipCtrl)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl construction

CAlbumTipCtrl::CAlbumTipCtrl()
	: m_pAlbumFolder( NULL )
	, m_nIcon32 	( 0 )
	, m_nIcon48 	( 0 )
	, m_nKeyWidth	( 0 )
	, m_bCollection	( FALSE )
	, m_crLight 	( CColors::CalculateColor( Colors.m_crTipBack, RGB( 255, 255, 255 ), 128 ) )
{
}

CAlbumTipCtrl::~CAlbumTipCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl prepare

BOOL CAlbumTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	if ( ! m_pAlbumFolder || ! LibraryFolders.CheckAlbum( m_pAlbumFolder ) ) return FALSE;

	// Basic data

	m_sName	= m_pAlbumFolder->m_sName;
	m_sType	= L"Virtual Folder";

	m_nIcon32 = 0;
	m_nIcon48 = 0;
	m_bCollection = bool( m_pAlbumFolder->m_oCollSHA1 );

	// Metadata

	m_pMetadata.Clear();

	if ( m_pAlbumFolder->m_pSchema != NULL )
	{
		CString strText;
		LPCTSTR pszColon = _tcschr( m_pAlbumFolder->m_pSchema->m_sTitle, ':' );
		m_sType = pszColon ? pszColon + 1 : m_pAlbumFolder->m_pSchema->m_sTitle;
		LoadString( strText, IDS_TIP_FOLDER );

		if ( Settings.General.LanguageRTL )
			m_sType = L"\x202A" + m_sType + L" \x200E" + strText;
		else
			m_sType += L" " + strText;

		m_nIcon48 = m_pAlbumFolder->m_pSchema->m_nIcon48;
		m_nIcon32 = m_pAlbumFolder->m_pSchema->m_nIcon32;

		if ( m_pAlbumFolder->m_pXML != NULL )
		{
			m_pMetadata.Setup( m_pAlbumFolder->m_pSchema, FALSE );
			m_pMetadata.Combine( m_pAlbumFolder->m_pXML );
			m_pMetadata.Clean();
		}
	}

	CalcSizeHelper();

	return m_sz.cx > 0;
}

/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl compute size

void CAlbumTipCtrl::OnCalcSize(CDC* pDC)
{
	AddSize( pDC, m_sName );
	m_sz.cy += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	AddSize( pDC, m_sType );
	m_sz.cy += TIP_TEXTHEIGHT;

	m_sz.cy += TIP_RULE;

	int nMetaHeight = static_cast< int >( m_pMetadata.GetCount() * TIP_TEXTHEIGHT );
	int nValueWidth = 0;
	m_nKeyWidth = 0;

	m_pMetadata.ComputeWidth( pDC, m_nKeyWidth, nValueWidth );

	if ( m_nKeyWidth ) m_nKeyWidth += TIP_GAP;
	m_sz.cx = max( m_sz.cx, m_nKeyWidth + nValueWidth + 102 );
	m_sz.cy += max( nMetaHeight, 96 );
}

/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl painting

void CAlbumTipCtrl::OnPaint(CDC* pDC)
{
	CPoint pt( 0, 0 );

	DrawText( pDC, &pt, m_sName );
	pt.y += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	DrawText( pDC, &pt, m_sType );
	pt.y += TIP_TEXTHEIGHT;

	DrawRule( pDC, &pt );

	CRect rcThumb( pt.x, pt.y, pt.x + 96, pt.y + 96 );
	CRect rcWork( &rcThumb );
	DrawThumb( pDC, rcWork );
	pDC->ExcludeClipRect( &rcThumb );

	int nCount = 0;

	for ( POSITION pos = m_pMetadata.GetIterator() ; pos ; )
	{
		CMetaItem* pItem = m_pMetadata.GetNext( pos );

		DrawText( pDC, &pt, pItem->m_sKey + ':', 100 );
		DrawText( pDC, &pt, pItem->m_sValue, 100 + m_nKeyWidth );
		pt.y += TIP_TEXTHEIGHT;

		if ( ++nCount == 5 )
		{
			pt.x += 98;
			pt.y -= 2;
			DrawRule( pDC, &pt, TRUE );
			pt.x -= 98;
			pt.y -= 2;
		}
	}
}

void CAlbumTipCtrl::DrawThumb(CDC* pDC, CRect& rcThumb)
{
	pDC->Draw3dRect( &rcThumb, Colors.m_crTipBorder, Colors.m_crTipBorder );
	rcThumb.DeflateRect( 1, 1 );

	CPoint pt(	( rcThumb.left + rcThumb.right ) / 2 - 24,
				( rcThumb.top + rcThumb.bottom ) / 2 - 24 );

	if ( m_nIcon48 >= 0 )
	{
		ShellIcons.Draw( pDC, m_nIcon48, 48, pt.x, pt.y, m_crLight );
		if ( m_bCollection )
			CoolInterface.Draw( pDC, IDI_COLLECTION_MASK, 16, pt.x, pt.y );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 48, pt.y + 48 );
	}
	else if ( m_nIcon32 >= 0 )
	{
		pt.x += 8;
		pt.y += 8;
		ShellIcons.Draw( pDC, m_nIcon32, 32, pt.x, pt.y );
		if ( m_bCollection )
			CoolInterface.Draw( pDC, IDI_COLLECTION_MASK, 16, pt.x, pt.y );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );
	}

	pDC->FillSolidRect( &rcThumb, m_crLight );
}
