//
// LiveListSizer.cpp
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
#include "Settings.h"
#include "Envy.h"
#include "LiveListSizer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CLiveListSizer construction

CLiveListSizer::CLiveListSizer(CListCtrl* pCtrl)
{
	m_pCtrl		= NULL;
	m_nWidth	= 0;
	m_nColumns	= 0;
	m_pWidth	= NULL;
	m_pTake		= NULL;
	if ( pCtrl ) Attach( pCtrl );
}

CLiveListSizer::~CLiveListSizer()
{
	Detach();
}

//////////////////////////////////////////////////////////////////////
// CLiveListSizer attach and detach

void CLiveListSizer::Attach(CListCtrl* pCtrl, BOOL bScale)
{
	Detach();
	m_pCtrl = pCtrl;
	if ( pCtrl && bScale ) Resize( 0, TRUE );
}

void CLiveListSizer::Detach()
{
	m_pCtrl		= NULL;
	m_nWidth	= 0;
	m_nColumns	= 0;
	if ( m_pWidth ) delete [] m_pWidth;
	m_pWidth = NULL;
	if ( m_pTake ) delete [] m_pTake;
	m_pTake = NULL;
}

//////////////////////////////////////////////////////////////////////
// CLiveListSizer scaling resize

BOOL CLiveListSizer::Resize(int nWidth, BOOL bScale)
{
	if ( m_pCtrl == NULL ) return FALSE;
	if ( ! Settings.General.SizeLists ) return FALSE;

	if ( ! nWidth )
	{
		CRect rc;
		m_pCtrl->GetClientRect( &rc );
		nWidth = rc.right;
	}

	nWidth -= GetSystemMetrics( SM_CXVSCROLL ) - 1;

	if ( nWidth < 64 ) return FALSE;

	LV_COLUMN pColumn;
	pColumn.mask = LVCF_WIDTH;
	int nColumn = 0;
	for ( ; m_pCtrl->GetColumn( nColumn, &pColumn ); nColumn++ );

	if ( nColumn != m_nColumns )
	{
		if ( m_pWidth ) delete [] m_pWidth;
		if ( m_pTake ) delete [] m_pTake;

		m_pWidth	= new int[ m_nColumns = nColumn ];
		m_pTake		= new float[ m_nColumns ];
	}

	if ( ! m_nWidth ) m_nWidth = nWidth;

	float nTotal = 0;

	for ( nColumn = 0; nColumn < m_nColumns; nColumn++ )
	{
		m_pCtrl->GetColumn( nColumn, &pColumn );

		if ( pColumn.cx != m_pWidth[ nColumn ] )
		{
			m_pWidth[ nColumn ]	= pColumn.cx;
			m_pTake[ nColumn ]	= (float)pColumn.cx / (float)m_nWidth;
		}

		nTotal += m_pTake[ nColumn ];
	}

	if ( nTotal > 1 || ( bScale && nTotal > 0 ) )
	{
		for ( nColumn = 0; nColumn < m_nColumns; nColumn++ )
		{
			m_pTake[ nColumn ] /= nTotal;
		}
		m_nWidth = 0;
	}

	if ( m_nWidth == nWidth ) return FALSE;
	m_nWidth = nWidth;

	for ( nColumn = 0; nColumn < m_nColumns; nColumn++ )
	{
		pColumn.cx = (int)( m_pTake[ nColumn ] * (float)m_nWidth );
		m_pWidth[ nColumn ] = pColumn.cx;
		m_pCtrl->SetColumn( nColumn, &pColumn );
	}

	return TRUE;
}
