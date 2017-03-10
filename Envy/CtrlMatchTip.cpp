//
// CtrlMatchTip.cpp
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
#include "CtrlCoolTip.h"
#include "CtrlMatchTip.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Library.h"
#include "SharedFile.h"
#include "MatchObjects.h"
#include "QueryHit.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "VendorCache.h"
#include "ShellIcons.h"
#include "Flags.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CMatchTipCtrl, CCoolTipCtrl)

//BEGIN_MESSAGE_MAP(CMatchTipCtrl, CCoolTipCtrl)
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl construction

CMatchTipCtrl::CMatchTipCtrl()
	: m_pFile	( NULL )
	, m_pHit	( NULL )
	, m_nIcon	( 0 )
{
}

//CMatchTipCtrl::~CMatchTipCtrl()
//{
//}

void CMatchTipCtrl::Show(CMatchFile* pFile, CQueryHit* pHit)
{
	bool bChanged = pFile != m_pFile || pHit != m_pHit;

	m_pFile	= pFile;
	m_pHit	= pHit;

	ShowImpl( bChanged );
}

BOOL CMatchTipCtrl::OnPrepare()
{
	if ( ! Settings.Interface.TipSearch )
		return FALSE;

	if ( m_pHit )
		LoadFromHit();
	else if ( m_pFile )
		LoadFromFile();
	else
		return FALSE;

	CalcSizeHelper();

	return ( m_sz.cx > 0 );
}

void CMatchTipCtrl::OnHide()
{
	m_pFile	= NULL;
	m_pHit	= NULL;
	m_pMetadata.Clear();
}

void CMatchTipCtrl::OnShow()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl load from content

void CMatchTipCtrl::LoadFromFile()
{
	m_sName = m_pFile->m_sName;
	if ( m_pFile->GetTotalHitsCount() == 1 )
	{
		m_sCountryCode = m_pFile->GetBestCountry();
		m_sCountry = theApp.GetCountryName( m_pFile->GetBestAddress() );
	}
	else
	{
		m_sCountryCode.Empty();
		m_sCountry.Empty();
	}
	m_sSize = LoadString( IDS_TIP_SIZE ) + L":  " + m_pFile->m_sSize;
	LoadTypeInfo();

	if ( Settings.General.GUIMode == GUI_BASIC )
	{
		m_sSHA1.Empty();
		m_sTiger.Empty();
		m_sED2K.Empty();
		m_sBTH.Empty();
		m_sMD5.Empty();
	}
	else
	{
		m_sSHA1 = m_pFile->m_oSHA1.toShortUrn();
		m_sTiger = m_pFile->m_oTiger.toShortUrn();
		m_sED2K = m_pFile->m_oED2K.toShortUrn();
		m_sBTH = m_pFile->m_oBTH.toShortUrn();
		m_sMD5 = m_pFile->m_oMD5.toShortUrn();
	}

	m_pFile->GetPartialTip( m_sPartial );
	m_pFile->GetQueueTip( m_sQueue );

	m_pSchema = m_pFile->AddHitsToMetadata( m_pMetadata );

	if ( m_pSchema != NULL )
	{
		m_pMetadata.Vote();
		m_pMetadata.Clean( 72 );
	}

	m_nRating = m_pFile->m_nRated ? m_pFile->m_nRating / m_pFile->m_nRated : 0;

	m_pFile->GetStatusTip( m_sStatus, m_crStatus );
	m_pFile->GetUser( m_sUser );

	if ( m_pFile->m_bBusy == 2 )
		LoadString( m_sBusy, IDS_TIP_FILE_BUSY );
	else
		m_sBusy.Empty();

	if ( m_pFile->m_bPush == 2 )
		LoadString( m_sPush, IDS_TIP_FILE_FIREWALLED );
	else
		m_sPush.Empty();

	if ( m_pFile->m_bStable == 1 )
		LoadString( m_sUnstable, IDS_TIP_FILE_UNSTABLE );
	else
		m_sUnstable.Empty();
}

void CMatchTipCtrl::LoadFromHit()
{
	m_sName = m_pHit->m_sName;
	m_sSize = LoadString( IDS_TIP_SIZE ) + L":  " + Settings.SmartVolume( m_pHit->m_nSize );
	LoadTypeInfo();

	if ( Settings.General.GUIMode == GUI_BASIC )
	{
		m_sSHA1.Empty();
		m_sTiger.Empty();
		m_sED2K.Empty();
		m_sBTH.Empty();
		m_sMD5.Empty();
	}
	else
	{
		m_sSHA1 = m_pHit->m_oSHA1.toShortUrn();
		m_sTiger = m_pHit->m_oTiger.toShortUrn();
		m_sED2K = m_pHit->m_oED2K.toShortUrn();
		m_sBTH = m_pHit->m_oBTH.toShortUrn();
		m_sMD5 = m_pHit->m_oMD5.toShortUrn();
	}

	if ( m_pHit->m_nPartial )
		m_sPartial.Format( LoadString( IDS_TIP_PARTIAL ), 100.0f * (float)m_pHit->m_nPartial / (float)m_pHit->m_nSize );
	else
		m_sPartial.Empty();

	if ( m_pHit->m_nUpSlots )
		m_sQueue.Format( LoadString( IDS_TIP_QUEUE ), m_pHit->m_nUpSlots, max( 0, m_pHit->m_nUpQueue - m_pHit->m_nUpSlots ) );
	else
		m_sQueue.Empty();

	m_pSchema = m_pHit->m_pSchema;

	m_pMetadata.Setup( m_pSchema );

	if ( m_pSchema != NULL )
	{
		if ( m_pHit->m_pXML && m_pSchema->Equals( m_pHit->m_pSchema ) )
			m_pMetadata.Combine( m_pHit->m_pXML );
		m_pMetadata.Clean( 72 );
	}

	m_nRating = m_pHit->m_nRating;

	m_sStatus.Empty();

	if ( m_pFile->GetLibraryStatus() == TRI_FALSE )
	{
		LoadString( m_sStatus, IDS_TIP_EXISTS_LIBRARY );
		m_crStatus = Colors.m_crTextStatus;
	}
	else if ( m_pFile->m_bDownload || m_pHit->m_bDownload )
	{
		LoadString( m_sStatus, IDS_TIP_EXISTS_DOWNLOAD );
		m_crStatus = Colors.m_crTextStatus;
	}
	else if ( m_pHit->m_bBogus )
	{
		LoadString( m_sStatus, IDS_TIP_BOGUS );
		m_crStatus = Colors.m_crTextAlert;
	}
	else if ( ! m_pHit->m_sComments.IsEmpty() )
	{
		if ( m_pHit->m_nRating == 1 )
			LoadString( m_sStatus, IDS_TIP_EXISTS_BLACKLISTED );
		m_sStatus += m_pHit->m_sComments;
		m_crStatus = Colors.m_crTextAlert;
	}
	else if ( m_pFile->GetLibraryStatus() == TRI_TRUE )		// Ghost rated
	{
		LoadString( m_sStatus, IDS_TIP_EXISTS_DELETED );
		m_crStatus = Colors.m_crTextAlert;
	}

	// Is this a firewalled eDonkey client
	if ( m_pHit->m_nProtocol == PROTOCOL_ED2K && m_pHit->m_bPush == TRI_TRUE )
	{
		m_sUser.Format( L"%lu@%s - %s",
			m_pHit->m_oClientID.begin()[2],
			(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)*m_pHit->m_oClientID.begin() ) ),
			(LPCTSTR)m_pHit->m_pVendor->m_sName );
	}
	else
	{
		m_sUser.Format( L"%s:%u - %s",
			(LPCTSTR)CString( inet_ntoa( m_pHit->m_pAddress ) ),
			m_pHit->m_nPort,
			(LPCTSTR)m_pHit->m_pVendor->m_sName );
	}

	// Add the Nickname if there is one and they are being shown
	if ( Settings.Search.ShowNames && ! m_pHit->m_sNick.IsEmpty() )
		m_sUser = m_pHit->m_sNick + L" (" + m_sUser + L")";

	m_sCountryCode = m_pHit->m_sCountry;
	m_sCountry = theApp.GetCountryName( m_pHit->m_pAddress );

	if ( m_pHit->m_bBusy == 2 )
		LoadString( m_sBusy, IDS_TIP_SOURCE_BUSY );
	else
		m_sBusy.Empty();

	if ( m_pHit->m_bPush == 2 )
		LoadString( m_sPush, IDS_TIP_SOURCE_FIREWALLED );
	else
		m_sPush.Empty();

	if ( m_pHit->m_bStable == 1 )
		LoadString( m_sUnstable, IDS_TIP_SOURCE_UNSTABLE );
	else
		m_sUnstable.Empty();
}

BOOL CMatchTipCtrl::LoadTypeInfo()
{
	m_nIcon = ShellIcons.Get( m_sName, 32 );
	m_sType = LoadString( IDS_TIP_TYPE ) + L": " + ShellIcons.GetTypeString( m_sName );
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl layout

void CMatchTipCtrl::OnCalcSize(CDC* pDC)
{
	AddSize( pDC, m_sName );
	m_sz.cy += TIP_ICONHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( ! m_sUser.IsEmpty() )
	{
		AddSize( pDC, m_sUser );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	if ( ! m_sCountry.IsEmpty() )
	{
		AddSize( pDC, m_sCountry, 18 + 2 );
		m_sz.cy += TIP_ICONHEIGHT;
	}

	if ( ! m_sStatus.IsEmpty() )
	{
		m_sz.cy += TIP_RULE;
		pDC->SelectObject( &CoolInterface.m_fntBold );
		AddSize( pDC, m_sStatus );
		pDC->SelectObject( &CoolInterface.m_fntNormal );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	m_sz.cy += TIP_RULE;

	// Icon
	m_sz.cy += 32;	// max( 32, TIP_TEXTHEIGHT * 2 );
	AddSize( pDC, m_sSize, 40 );
	AddSize( pDC, m_sType, 40 );

	if ( Settings.General.GUIMode != GUI_BASIC &&
		( ! m_sSHA1.IsEmpty() || ! m_sTiger.IsEmpty() || ! m_sED2K.IsEmpty() || ! m_sBTH.IsEmpty() || ! m_sMD5.IsEmpty() ) )
	{
		m_sz.cy += TIP_RULE;

		if ( ! m_sSHA1.IsEmpty() )
		{
			AddSize( pDC, m_sSHA1 );
			m_sz.cy += TIP_TEXTHEIGHT;
		}

		if ( ! m_sTiger.IsEmpty() )
		{
			AddSize( pDC, m_sTiger );
			m_sz.cy += TIP_TEXTHEIGHT;
		}

		if ( ! m_sED2K.IsEmpty() )
		{
			AddSize( pDC, m_sED2K );
			m_sz.cy += TIP_TEXTHEIGHT;
		}

		if ( ! m_sBTH.IsEmpty() )
		{
			AddSize( pDC, m_sBTH );
			m_sz.cy += TIP_TEXTHEIGHT;
		}

		if ( ! m_sMD5.IsEmpty() )
		{
			AddSize( pDC, m_sMD5 );
			m_sz.cy += TIP_TEXTHEIGHT;
		}
	}

	// Busy/Firewalled/unstable warnings. Queue info.
	if ( ! m_sBusy.IsEmpty() || ! m_sPush.IsEmpty() || ! m_sUnstable.IsEmpty() || ! m_sQueue.IsEmpty() )
	{
		m_sz.cy += TIP_RULE;

		if ( ! m_sBusy.IsEmpty() )
		{
			pDC->SelectObject( &CoolInterface.m_fntBold );
			AddSize( pDC, m_sBusy, 20 );
			pDC->SelectObject( &CoolInterface.m_fntNormal );
			m_sz.cy += TIP_ICONHEIGHT;
		}

		if ( ! m_sQueue.IsEmpty() )
		{
			if ( ! m_sBusy.IsEmpty() || ! m_sPush.IsEmpty() || ! m_sUnstable.IsEmpty() )	// Align queue info with above (if present)
				AddSize( pDC, m_sQueue, 20 );
			else
				AddSize( pDC, m_sQueue );
			m_sz.cy += TIP_ICONHEIGHT;
		}

		if ( ! m_sPush.IsEmpty() )
		{
			pDC->SelectObject( &CoolInterface.m_fntBold );
			AddSize( pDC, m_sPush, 20 );
			pDC->SelectObject( &CoolInterface.m_fntNormal );
			m_sz.cy += TIP_ICONHEIGHT;
		}

		if ( ! m_sUnstable.IsEmpty() )
		{
			pDC->SelectObject( &CoolInterface.m_fntBold );
			AddSize( pDC, m_sUnstable, 20 );
			pDC->SelectObject( &CoolInterface.m_fntNormal );
			m_sz.cy += TIP_ICONHEIGHT;
		}
	}

	// Partial warning
	if ( ! m_sPartial.IsEmpty() )
	{
		m_sz.cy += TIP_RULE;
		AddSize( pDC, m_sPartial );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	// Metadata
	if ( int nCount = (int)m_pMetadata.GetCount( TRUE ) )	// INT_PTR
	{
		m_sz.cy += TIP_RULE - 1;

		int nMetaHeight = nCount * TIP_TEXTHEIGHT;
		int nValueWidth = 0;
		m_nKeyWidth = 40;

		m_pMetadata.ComputeWidth( pDC, m_nKeyWidth, nValueWidth );

		if ( m_nKeyWidth ) m_nKeyWidth += TIP_GAP;
		m_sz.cx  = min( max( m_sz.cx, (LONG)m_nKeyWidth + nValueWidth ), (LONG)GetSystemMetrics( SM_CXSCREEN ) / 2 );
		m_sz.cy += nMetaHeight;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl painting

void CMatchTipCtrl::OnPaint(CDC* pDC)
{
	if ( ! IsWindow( GetSafeHwnd() ) || ! IsWindowVisible() ) return;

	CPoint pt( 0, 0 );
	//CSize sz( m_sz.cx, TIP_TEXTHEIGHT );

	const COLORREF crBack = Images.m_bmToolTip.m_hObject ? CLR_NONE : Colors.m_crTipBack;

	DrawText( pDC, &pt, m_sName );
	pt.y += TIP_ICONHEIGHT;

	pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( ! m_sUser.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sUser );
		pt.y += TIP_TEXTHEIGHT;
	}

	if ( ! m_sCountry.IsEmpty() )
	{
		int nFlagIndex = Flags.GetFlagIndex( m_sCountryCode );
		if ( nFlagIndex >= 0 )
		{
			Flags.Draw( nFlagIndex, *pDC, pt.x, pt.y, crBack, crBack );
			if ( ! Images.m_bmToolTip.m_hObject )
				pDC->ExcludeClipRect( pt.x, pt.y, pt.x + Flags.Width, pt.y + 16 );
			pt.x += Flags.Width + 4;
			pt.y += 2;
		}
		DrawText( pDC, &pt, m_sCountry );
		if ( nFlagIndex >= 0 )
		{
			pt.x -= Flags.Width + 4;
			pt.y -= 2;
		}
		pt.y += TIP_ICONHEIGHT;
	}

	if ( ! m_sStatus.IsEmpty() )
	{
		DrawRule( pDC, &pt );

		pDC->SetTextColor( m_crStatus );
		pDC->SelectObject( &CoolInterface.m_fntBold );
		DrawText( pDC, &pt, m_sStatus );
		pDC->SelectObject( &CoolInterface.m_fntNormal );
		pDC->SetTextColor( Colors.m_crTipText );
		pt.y += TIP_TEXTHEIGHT;
	}

	DrawRule( pDC, &pt );

	pt.y--;
	ShellIcons.Draw( pDC, m_nIcon, 32, pt.x, pt.y, crBack );
	pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );
	pt.y++;

	if ( m_nRating > 1 )
	{
		CPoint ptStar( m_sz.cx - 3, pt.y - 2 );

		for ( int nRating = m_nRating - 1 ; nRating ; nRating-- )
		{
			ptStar.x -= 16;
			CoolInterface.Draw( pDC, IDI_STAR, 16, ptStar.x, ptStar.y, crBack );
			if ( ! Images.m_bmToolTip.m_hObject )
				pDC->ExcludeClipRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 );
		}
	}

	pt.x += 40;
	DrawText( pDC, &pt, m_sSize );
	if ( m_sSize.Find( L" B" ) < 1 && m_nRating < 2 )
	{
		CString strSize;
		strSize.Format( L"(%I64i bytes)", m_pFile->m_nSize );
		CSize szText = pDC->GetTextExtent( strSize );

		int nPos = pt.x;
		pt.x = m_sz.cx - szText.cx - 1;
		DrawText( pDC, &pt, strSize );
		pt.x = nPos;
	}
	pt.y += TIP_ICONHEIGHT;
	DrawText( pDC, &pt, m_sType );
	pt.y += TIP_TEXTHEIGHT;
	pt.x -= 40;

	// Hashes
	if ( Settings.General.GUIMode != GUI_BASIC &&
		( ! m_sSHA1.IsEmpty() || ! m_sTiger.IsEmpty() || ! m_sED2K.IsEmpty() || ! m_sBTH.IsEmpty() || ! m_sMD5.IsEmpty() ) )
	{
		DrawRule( pDC, &pt );

		if ( ! m_sSHA1.IsEmpty() )
		{
			DrawText( pDC, &pt, m_sSHA1 );
			pt.y += TIP_TEXTHEIGHT;
		}

		if ( ! m_sTiger.IsEmpty() )
		{
			DrawText( pDC, &pt, m_sTiger );
			pt.y += TIP_TEXTHEIGHT;
		}

		if ( ! m_sED2K.IsEmpty() )
		{
			DrawText( pDC, &pt, m_sED2K );
			pt.y += TIP_TEXTHEIGHT;
		}

		if ( ! m_sBTH.IsEmpty() )
		{
			DrawText( pDC, &pt, m_sBTH );
			pt.y += TIP_TEXTHEIGHT;
		}

		if ( ! m_sMD5.IsEmpty() )
		{
			DrawText( pDC, &pt, m_sMD5 );
			pt.y += TIP_TEXTHEIGHT;
		}
	}

	// Busy, firewalled, unstabled warnings. Queue info
	if ( ! m_sBusy.IsEmpty() || ! m_sPush.IsEmpty() || ! m_sUnstable.IsEmpty() || ! m_sQueue.IsEmpty() )
	{
		DrawRule( pDC, &pt );

		pDC->SetTextColor( Colors.m_crTipWarnings );
		pDC->SelectObject( &CoolInterface.m_fntBold );

		// Source busy warning
		if ( ! m_sBusy.IsEmpty() )
		{
			CoolInterface.Draw( pDC, IDI_BUSY, 16, pt.x, pt.y, crBack );
			if ( ! Images.m_bmToolTip.m_hObject )
				pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 16, pt.y + 16 );

			pt.x += 20;
			pt.y++;
			DrawText( pDC, &pt, m_sBusy );
			pt.x -= 20;
			pt.y += TIP_ICONHEIGHT;
		}

		pDC->SetTextColor( Colors.m_crTipText );
		pDC->SelectObject( &CoolInterface.m_fntNormal );

		// Queue info
		if ( ! m_sQueue.IsEmpty() )
		{
			if ( ! m_sBusy.IsEmpty() || ! m_sPush.IsEmpty() || ! m_sUnstable.IsEmpty() )	// Align queue info with above (if present)
			{
				pt.x += 20;
				DrawText( pDC, &pt, m_sQueue );
				pt.x -= 20;
			}
			else
				DrawText( pDC, &pt, m_sQueue );

			pt.y += TIP_ICONHEIGHT;
		}

		pDC->SetTextColor( Colors.m_crTipWarnings );
		pDC->SelectObject( &CoolInterface.m_fntBold );

		// Source firewalled warning
		if ( ! m_sPush.IsEmpty() )
		{
			CoolInterface.Draw( pDC, IDI_FIREWALLED, 16, pt.x, pt.y, crBack );
			if ( ! Images.m_bmToolTip.m_hObject )
				pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 16, pt.y + 16 );

			pt.y++;
			pt.x += 20;
			DrawText( pDC, &pt, m_sPush );
			pt.x -= 20;
			pt.y += TIP_ICONHEIGHT;
		}

		// Source unstable warning
		if ( ! m_sUnstable.IsEmpty() )
		{
			CoolInterface.Draw( pDC, IDI_UNSTABLE, 16, pt.x, pt.y, crBack );
			if ( ! Images.m_bmToolTip.m_hObject )
				pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 16, pt.y + 16 );

			pt.y++;
			pt.x += 20;
			DrawText( pDC, &pt, m_sUnstable );
			pt.x -= 20;
			pt.y += TIP_ICONHEIGHT;
		}
		pDC->SetTextColor( Colors.m_crTipText );
		pDC->SelectObject( &CoolInterface.m_fntNormal );
	}

	// Partial warning
	if ( ! m_sPartial.IsEmpty() )
	{
		DrawRule( pDC, &pt );
		DrawText( pDC, &pt, m_sPartial );
		pt.y += TIP_TEXTHEIGHT;
	}

	// Metadata
	if ( m_pMetadata.GetCount( TRUE ) )
	{
		DrawRule( pDC, &pt );

		for ( POSITION pos = m_pMetadata.GetIterator() ; pos ; )
		{
			const CMetaItem* pItem = m_pMetadata.GetNext( pos );
			if ( pItem->m_pMember && pItem->m_pMember->m_bHidden ) continue;

			DrawText( pDC, &pt, Settings.General.LanguageRTL ? L':' + pItem->m_sKey : pItem->m_sKey + L':' );
			pt.x += m_nKeyWidth;
			DrawText( pDC, &pt, pItem->m_sValue );
			pt.x -= m_nKeyWidth;
			pt.y += TIP_TEXTHEIGHT;
		}
	}
}
