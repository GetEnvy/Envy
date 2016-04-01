//
// GraphBase.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
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
#include "GraphBase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CGraphBase construction

CGraphBase::CGraphBase()
{
	m_hOldImage = NULL;
	m_szImage	= CSize( 0, 0 );
}

CGraphBase::~CGraphBase()
{
	if ( m_hOldImage )
	{
		m_pDC.SelectObject( CBitmap::FromHandle( m_hOldImage ) );
		m_pDC.DeleteDC();
		m_pImage.DeleteObject();
		m_hOldImage = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CGraphBase defaults

void CGraphBase::CreateDefaults()
{
}

//////////////////////////////////////////////////////////////////////
// CGraphBase serialize

void CGraphBase::Serialize(CArchive& /*ar*/)
{
}

//////////////////////////////////////////////////////////////////////
// CGraphBase update

BOOL CGraphBase::Update()
{
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CGraphBase clear

void CGraphBase::Clear()
{
}

//////////////////////////////////////////////////////////////////////
// CGraphBase paint

void CGraphBase::Paint(CDC* /*pDC*/, CRect* /*pRect*/)
{
}

//////////////////////////////////////////////////////////////////////
// CGraphBase buffered paint

void CGraphBase::BufferedPaint(CDC* pDC, CRect* pRect)
{
	CSize sz = pRect->Size();

	if ( sz.cx < 0 || sz.cy < 0 ) return;

	if ( sz != m_szImage || m_hOldImage == NULL )
	{
		if ( m_hOldImage != NULL )
		{
			m_pDC.SelectObject( CBitmap::FromHandle( m_hOldImage ) );
			m_pDC.DeleteDC();
			m_pImage.DeleteObject();
		}

		m_szImage = sz;
		m_pImage.CreateCompatibleBitmap( pDC, sz.cx, sz.cy );
		m_pDC.CreateCompatibleDC( pDC );
		m_hOldImage = (HBITMAP)m_pDC.SelectObject( &m_pImage )->GetSafeHandle();
	}

	CRect rc( 0, 0, sz.cx, sz.cy );
	Paint( &m_pDC, &rc );

	pDC->BitBlt( pRect->left, pRect->top, sz.cx, sz.cy, &m_pDC, 0, 0, SRCCOPY );
}
