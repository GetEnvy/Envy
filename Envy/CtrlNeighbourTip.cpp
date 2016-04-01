//
// CtrlNeighbourTip.cpp
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
#include "CtrlNeighbourTip.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Network.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "GProfile.h"
#include "GraphLine.h"
#include "GraphItem.h"
#include "Flags.h"

#include "EDPacket.h"
#include "EDNeighbour.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CNeighbourTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CNeighbourTipCtrl, CCoolTipCtrl)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl construction

CNeighbourTipCtrl::CNeighbourTipCtrl()
	: m_nNeighbour	( 0 )
	, m_pGraph		( NULL )
	, m_pItemIn		( NULL )
	, m_pItemOut	( NULL )
{
}

CNeighbourTipCtrl::~CNeighbourTipCtrl()
{
	delete m_pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl prepare

BOOL CNeighbourTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;

	const CNeighbour* pNeighbour = Neighbours.Get( m_nNeighbour );
	if ( pNeighbour == NULL ) return FALSE;

	CalcSizeHelper();

//	CoolInterface.LoadIconsTo( m_pProtocols, protocolIDs, FALSE, LVSIL_NORMAL );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl show and hide

void CNeighbourTipCtrl::OnShow()
{
	delete m_pGraph;

	m_pGraph	= CreateLineGraph();
	m_pItemIn	= new CGraphItem( 0, 1.0f, RGB( 0, 0, 0xFF ) );
	m_pItemOut	= new CGraphItem( 0, 1.0f, RGB( 0xFF, 0, 0 ) );

	m_pGraph->AddItem( m_pItemIn );
	m_pGraph->AddItem( m_pItemOut );
}

void CNeighbourTipCtrl::OnHide()
{
	delete m_pGraph;
	m_pGraph = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl size

void CNeighbourTipCtrl::OnCalcSize(CDC* pDC)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return;

	const CNeighbour* pNeighbour = Neighbours.Get( m_nNeighbour );
	CString str;

	if ( pNeighbour->m_pProfile != NULL && pNeighbour->m_pProfile->IsValid() )
	{
		str = pNeighbour->m_pProfile->GetNick();
		if ( ! str.IsEmpty() )
		{
			pDC->SelectObject( &CoolInterface.m_fntBold );
			AddSize( pDC, str );
			m_sz.cy += TIP_TEXTHEIGHT;
		}

		str = pNeighbour->m_pProfile->GetLocation();
		if ( ! str.IsEmpty() )
		{
			pDC->SelectObject( &CoolInterface.m_fntNormal );
			AddSize( pDC, str );
			m_sz.cy += TIP_TEXTHEIGHT;
		}

		m_sz.cy += TIP_RULE;
	}
	else if ( ! pNeighbour->m_sServerName.IsEmpty() )	// pNeighbour->m_nProtocol == PROTOCOL_ED2K || PROTOCOL_DC
	{
		pDC->SelectObject( &CoolInterface.m_fntNormal );
		AddSize( pDC, pNeighbour->m_sServerName );
		m_sz.cy += TIP_TEXTHEIGHT;
		m_sz.cy += TIP_RULE;
	}

	pDC->SelectObject( &CoolInterface.m_fntBold );
	AddSize( pDC, pNeighbour->m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	m_sz.cy += TIP_TEXTHEIGHT;

	if ( ! pNeighbour->m_sCountryName.IsEmpty() )
	{
		AddSize( pDC, pNeighbour->m_sCountryName );
		m_sz.cy += TIP_TEXTHEIGHT + 4;
	}

	if ( ! pNeighbour->m_sUserAgent.IsEmpty() )
		str = pNeighbour->m_sUserAgent;
	else
		str = L"(" + (CString)protocolNames[ pNeighbour->m_nProtocol ] + L")";
	AddSize( pDC, str );
	m_sz.cy += TIP_TEXTHEIGHT;

	m_sz.cy += TIP_RULE;
	m_sz.cy += TIP_TEXTHEIGHT * 6 - 2;

	float nCompIn, nCompOut;
	pNeighbour->GetCompression( nCompIn, nCompOut );
	if ( nCompIn > 0 || nCompOut > 0 )
		m_sz.cy += TIP_TEXTHEIGHT;

	m_sz.cy += 40;	// Graph?
	m_sz.cx = max( m_sz.cx, 128 + 160 );
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl paint

void CNeighbourTipCtrl::OnPaint(CDC* pDC)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	const CNeighbour* pNeighbour = Neighbours.Get( m_nNeighbour );
	if ( pNeighbour == NULL ) return;

	CPoint pt( 0, 0 );
	CString str;

	if ( pNeighbour->m_pProfile != NULL && pNeighbour->m_pProfile->IsValid() )
	{
		str = pNeighbour->m_pProfile->GetNick();
		if ( ! str.IsEmpty() )
		{
			pDC->SelectObject( &CoolInterface.m_fntBold );
			DrawText( pDC, &pt, str );
			pt.y += TIP_TEXTHEIGHT;
		}

		pDC->SelectObject( &CoolInterface.m_fntNormal );
		str = pNeighbour->m_pProfile->GetLocation();
		if ( ! str.IsEmpty() )
		{
			DrawText( pDC, &pt, str );
			pt.y += TIP_TEXTHEIGHT;
		}

		DrawRule( pDC, &pt );
	}
	else if ( ! pNeighbour->m_sServerName.IsEmpty() )	// pNeighbour->m_nProtocol == PROTOCOL_ED2K || PROTOCOL_DC
	{
		pDC->SelectObject( &CoolInterface.m_fntBold );
		DrawText( pDC, &pt, pNeighbour->m_sServerName );
		pt.y += TIP_TEXTHEIGHT;
		DrawRule( pDC, &pt );
	}

	// Show large protocol icon (unused)
	//CRect rcProtocol( m_sz.cx - 32 - 4, pt.y + 4, m_sz.cx - 4, pt.y + 32 + 4 );
	//ImageList_DrawEx( m_pProtocols, pNeighbour->m_nProtocol, pDC->GetSafeHdc(),
	//	rcProtocol.left, rcProtocol.top, rcProtocol.Width(), rcProtocol.Height(),
	//	Colors.m_crTipBack, CLR_DEFAULT, ILD_NORMAL );
	//pDC->ExcludeClipRect( &rcProtocol );

	pDC->SelectObject( &CoolInterface.m_fntBold );
	DrawText( pDC, &pt, pNeighbour->m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	pt.y += TIP_TEXTHEIGHT;

	if ( ! pNeighbour->m_sCountryName.IsEmpty() )
	{
		int nFlagIndex = Flags.GetFlagIndex( pNeighbour->m_sCountry );
		if ( nFlagIndex >= 0 )
		{
			Flags.Draw( nFlagIndex, pDC->GetSafeHdc(), pt.x, pt.y,
				Images.m_bmToolTip.m_hObject ? CLR_NONE : Colors.m_crTipBack, CLR_NONE, ILD_NORMAL );
			pDC->ExcludeClipRect( pt.x, pt.y, pt.x + FLAG_WIDTH, pt.y + 16 );

			pt.y += 2;
			pt.x += FLAG_WIDTH + 9;
			DrawText( pDC, &pt, pNeighbour->m_sCountryName );
			pt.x -= FLAG_WIDTH + 9;
		}
		else
		{
			DrawText( pDC, &pt, pNeighbour->m_sCountryName );
		}
		pt.y += TIP_TEXTHEIGHT + 2;
	}

	if ( ! pNeighbour->m_sUserAgent.IsEmpty() )
		str = pNeighbour->m_sUserAgent;
	else
		str = L"(" + (CString)protocolNames[ pNeighbour->m_nProtocol ] + L")";
	DrawText( pDC, &pt, str );
	pt.y += TIP_TEXTHEIGHT;

	if ( pNeighbour->m_nState < nrsConnected )
	{
		LoadString( str, IDS_NEIGHBOUR_HANDSHAKE );
	}
	else
	{
		switch ( pNeighbour->m_nProtocol )
		{
		case PROTOCOL_G1:
			switch ( pNeighbour->m_nNodeType )
			{
			case ntNode:
				LoadString( str, IDS_NEIGHBOUR_G1PP );
				break;
			case ntHub:
				LoadString( str, IDS_NEIGHBOUR_G1LU );
				break;
			case ntLeaf:
				LoadString( str, IDS_NEIGHBOUR_G1UL );
				break;
			}
			break;
		case PROTOCOL_G2:
			switch ( pNeighbour->m_nNodeType )
			{
			case ntNode:
				LoadString( str, IDS_NEIGHBOUR_G2HH );
				break;
			case ntHub:
				LoadString( str, IDS_NEIGHBOUR_G2LH );
				break;
			case ntLeaf:
				LoadString( str, IDS_NEIGHBOUR_G2HL );
				break;
			}
			break;
		case PROTOCOL_ED2K:
			if ( CEDNeighbour* pED = (CEDNeighbour*)pNeighbour )
			{
				if ( CEDPacket::IsLowID( pED->m_nClientID ) )
					str.Format( LoadString( IDS_NEIGHBOUR_ED2K_LOW ), pED->m_nClientID );
				else
					LoadString( str, IDS_NEIGHBOUR_ED2K_HIGH );
			}
			break;
		case PROTOCOL_DC:
			LoadString( str, IDS_NEIGHBOUR_DCHUB );
			break;
		default:
			LoadString( str, IDS_NEIGHBOUR_HANDSHAKE );
			break;
		}
	}

	DrawText( pDC, &pt, str );
	pt.y += TIP_TEXTHEIGHT;

	DrawRule( pDC, &pt );

	pDC->SelectObject( &CoolInterface.m_fntBold );
	pDC->SetTextColor( m_pItemIn->m_nColor );
	LoadString( str, IDS_NEIGHBOUR_INBOUND );
	DrawText( pDC, &pt, str, 128 );
	pDC->SetTextColor( m_pItemOut->m_nColor );
	LoadString( str, IDS_NEIGHBOUR_OUTBOUND );
	DrawText( pDC, &pt, str, 128 + 80 );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	pDC->SetTextColor( 0 );

	pt.y += TIP_TEXTHEIGHT;

	LoadString( str, IDS_NEIGHBOUR_CURRENT );
	DrawText( pDC, &pt, str );
	str = Settings.SmartSpeed( pNeighbour->m_mInput.nMeasure );
	DrawText( pDC, &pt, str, 128 );
	str = Settings.SmartSpeed( pNeighbour->m_mOutput.nMeasure );
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( str, IDS_NEIGHBOUR_TOTAL );
	DrawText( pDC, &pt, str );
	str = Settings.SmartVolume( pNeighbour->m_mInput.nTotal );
	DrawText( pDC, &pt, str, 128 );
	str = Settings.SmartVolume( pNeighbour->m_mOutput.nTotal );
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	float nCompIn, nCompOut;
	pNeighbour->GetCompression( nCompIn, nCompOut );

	LoadString( str, IDS_NEIGHBOUR_COMPRESSION );
	DrawText( pDC, &pt, str );
	LoadString( str, nCompIn > 0 ? IDS_NEIGHBOUR_COMPRESSION_DF : IDS_NEIGHBOUR_COMPRESSION_NONE );
	DrawText( pDC, &pt, str, 128 );
	LoadString( str, nCompOut > 0 ? IDS_NEIGHBOUR_COMPRESSION_DF : IDS_NEIGHBOUR_COMPRESSION_NONE );
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	if ( nCompIn > 0 || nCompOut > 0 )
	{
		LoadString( str, IDS_NEIGHBOUR_RATIO );
		DrawText( pDC, &pt, str );
		( nCompIn  > 0 ) ? str.Format( L"%.2f%%", nCompIn  * 100.0 ) : str.Empty();
		DrawText( pDC, &pt, str, 128 );
		( nCompOut > 0 ) ? str.Format( L"%.2f%%", nCompOut * 100.0 ) : str.Empty();
		DrawText( pDC, &pt, str, 128 + 80 );
		pt.y += TIP_TEXTHEIGHT;
	}

	pt.y += TIP_TEXTHEIGHT - 2;

	CRect rc( pt.x, pt.y, m_sz.cx, pt.y + 40 );
	pDC->Draw3dRect( &rc, Colors.m_crTipBorder, Colors.m_crTipBorder );
	rc.DeflateRect( 1, 1 );
	m_pGraph->BufferedPaint( pDC, &rc );
	rc.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rc );
	pt.y += 40;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl message handlers

void CNeighbourTipCtrl::OnTimer(UINT_PTR nIDEvent)
{
	CCoolTipCtrl::OnTimer( nIDEvent );

	if ( m_pGraph == NULL ) return;

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	CNeighbour* pNeighbour = Neighbours.Get( m_nNeighbour );
	if ( pNeighbour == NULL ) return;

	pNeighbour->Measure();

	const DWORD nIn  = pNeighbour->m_mInput.nMeasure;
	const DWORD nOut = pNeighbour->m_mOutput.nMeasure;

	m_pItemIn->Add( nIn );
	m_pItemOut->Add( nOut );

	m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nIn );
	m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nOut );
	m_pGraph->m_nUpdates++;

	CRect rcWndTip;
	SystemParametersInfo( SPI_GETWORKAREA, 0, rcWndTip, 0 );
	rcWndTip.top += 90;
	InvalidateRect( &rcWndTip );
}
