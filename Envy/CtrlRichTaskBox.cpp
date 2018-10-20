//
// CtrlRichTaskBox.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2010
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
#include "Envy.h"
#include "RichDocument.h"
#include "CtrlRichTaskBox.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CRichTaskBox, CTaskBox)

BEGIN_MESSAGE_MAP(CRichTaskBox, CTaskBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRichTaskBox construction

CRichTaskBox::CRichTaskBox()
	: m_pDocument	( NULL )
	, m_nWidth		( -1 )
{
}

CRichTaskBox::~CRichTaskBox()
{
	if ( m_pDocument ) delete m_pDocument;
}

/////////////////////////////////////////////////////////////////////////////
// CRichTaskBox message handlers

BOOL CRichTaskBox::Create(CTaskPanel* pPanel, LPCTSTR pszCaption, UINT nIDIcon)
{
	return CTaskBox::Create( pPanel, 0, pszCaption, nIDIcon );
}

int CRichTaskBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc;
	GetClientRect( &rc );
	m_wndView.Create( WS_CHILD|WS_VISIBLE, rc, this, 1 );
	m_wndView.SetOwner( GetPanel()->GetOwner() );

	return 0;
}

void CRichTaskBox::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	if ( cx != m_nWidth )
	{
		m_nWidth = cx;
		SetSize( m_wndView.FullHeightMove( 0, 0, cx ) );
	}
}

void CRichTaskBox::SetDocument(CRichDocument* pDocument)
{
	m_wndView.SetDocument( pDocument );

	if ( pDocument->m_crBackground == Colors.m_crRichdocBack )
		pDocument->m_crBackground = Colors.m_crTaskBoxClient;
	if ( pDocument->m_crText == Colors.m_crRichdocText )
		pDocument->m_crText = Colors.m_crTaskBoxText;

	Update();
}

void CRichTaskBox::Update()
{
	if ( m_wndView.IsModified() )
	{
		CRect rc;
		GetClientRect( &rc );
		m_nWidth = rc.Width();

		SetSize( m_wndView.FullHeightMove( 0, 0, m_nWidth ) );
		m_wndView.Invalidate();
	}
}
