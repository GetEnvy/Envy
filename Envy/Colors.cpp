//
// Colors.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2020
// Portions copyright PeerProject 2009-2014
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
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CColors Colors;


//////////////////////////////////////////////////////////////////////
// CColors construction

CColors::CColors()
{
}

//CColors::~CColors()
//{
//}

void CColors::Load()
{
	CalculateColors();
}

//////////////////////////////////////////////////////////////////////
// CColors Default Colors

void CColors::CalculateColors(BOOL bCustom)
{
	if ( ( m_bCustom = bCustom ) == FALSE )
	{
		m_crWindow				= GetSysColor( COLOR_WINDOW );
		m_crMidtone				= GetSysColor( COLOR_BTNFACE );
		m_crText				= GetSysColor( COLOR_WINDOWTEXT );
		m_crHiText				= GetSysColor( COLOR_HIGHLIGHTTEXT );
		m_crHighlight			= GetSysColor( COLOR_HIGHLIGHT );
		m_crHiBorder			= m_crHighlight;
		m_crHiBorderIn			= NULL;
	}

	m_crSysWindow				= GetSysColor( COLOR_WINDOW );
	m_crSysBtnFace				= GetSysColor( COLOR_BTNFACE );
	m_crSysDisabled				= GetSysColor( COLOR_GRAYTEXT );	// Borders
	m_crSys3DShadow 			= GetSysColor( COLOR_3DSHADOW );
	m_crSys3DHighlight			= GetSysColor( COLOR_3DHIGHLIGHT );
	m_crSysActiveCaption		= GetSysColor( COLOR_ACTIVECAPTION );
	m_crSysMenuText 			= GetSysColor( COLOR_MENUTEXT );
	m_crStatusBar				= RGB_DEFAULT_CASE;					// GetSysColor( COLOR_WINDOWFRAME );
//	m_crStatusBarText			= 0;

	m_crDialog					= CalculateColor( m_crSysBtnFace, m_crSysWindow, 100 );
	m_crDialogText				= RGB( 0, 0, 0 );
	m_crDialogMenu				= m_crWindow;
	m_crDialogMenuText			= m_crText;
	m_crDialogPanel 			= m_crWindow;
	m_crDialogPanelText			= m_crText;

	m_crBackNormal				= CalculateColor( m_crMidtone, m_crWindow, 215 );
	m_crBackSel					= CalculateColor( m_crHighlight, m_crWindow, 180 );
	m_crBackCheck				= CalculateColor( m_crHighlight, m_crWindow, 200 );
	m_crBackCheckSel			= CalculateColor( m_crHighlight, m_crWindow, 130 );
	m_crMargin					= CalculateColor( m_crMidtone, m_crWindow, 40 );
	m_crShadow					= CalculateColor( m_crHighlight, m_crSys3DShadow, 200 );
	m_crBorder					= m_crHighlight;
	m_crDisabled				= m_crSysDisabled;
	m_crCmdText					= m_crSysMenuText;
	m_crCmdTextSel				= m_crCmdText;

	m_crPanelBorder				= RGB( 0, 0, 0 );
	m_crPanelText				= RGB( 255, 255, 255 );
	m_crPanelBack				= RGB_DEFAULT_CASE;
	m_crBannerText				= RGB( 250, 250, 255 );
	m_crBannerBack				= RGB_DEFAULT_CASE;
	m_crSchemaRow[0]			= RGB( 245, 245, 255 );
	m_crSchemaRow[1]			= RGB( 215, 225, 245 );

	m_crTipBack					= GetSysColor( COLOR_INFOBK );
	m_crTipText					= GetSysColor( COLOR_INFOTEXT );
	m_crTipBorder				= CalculateColor( m_crTipBack, (COLORREF)0, 100 );
	m_crTipGraph				= CalculateColor( RGB( 255, 255, 255 ), m_crTipBack, 80 );
	m_crTipGraphGrid			= CalculateColor( Colors.m_crTipBorder, m_crTipGraph, 180 );
	m_crTipWarnings				= RGB( 64, 64, 64 );		// Grey warning messages

	m_crTaskPanelBack			= RGB( 122, 160, 230 );		// Not RGB_DEFAULT_CASE
	m_crTaskBoxPrimaryBack		= RGB_DEFAULT_CASE;			// Was RGB( 30, 87, 199 )
	m_crTaskBoxPrimaryText		= RGB( 255, 255, 255 );
	m_crTaskBoxCaptionBack		= RGB( 250, 250, 255 );
	m_crTaskBoxCaptionText		= RGB( 34, 94, 218 );
	m_crTaskBoxCaptionHover		= RGB( 84, 144, 255 );
	m_crTaskBoxClient			= RGB( 214, 224, 248 );
	m_crTaskBoxText 			= m_crText;

	m_crMediaWindowBack			= RGB( 0, 0, 0 );
	m_crMediaWindowText			= RGB( 0xDD, 0xDD, 0xDD );
	m_crMediaStatusBack			= RGB( 0x1B, 0x1D, 0x20 );
	m_crMediaStatusText			= RGB( 254, 254, 254 );
	m_crMediaPanelBack 			= RGB( 0x1A, 0x18, 0x16 );
	m_crMediaPanelText			= RGB( 0x68, 0x62, 0x5A );
	m_crMediaPanelActiveBack	= RGB( 0x1C, 0x1B, 0x1A );
	m_crMediaPanelActiveText	= RGB( 0xFF, 0xFF, 0xFF );
	m_crMediaPanelCaptionBack	= RGB( 0x34, 0x32, 0x32 );
	m_crMediaPanelCaptionText	= RGB( 0xFD, 0xFC, 0xFA );

	m_crTrafficWindowBack		= RGB( 0, 0, 0 );
	m_crTrafficWindowText		= RGB( 192, 196, 255 );
	m_crTrafficWindowGrid		= RGB( 0, 0, 128 );

	m_crMonitorHistoryBack		= RGB( 0, 0, 0 );
	m_crMonitorHistoryBackMax	= RGB( 80, 0, 0 );
	m_crMonitorHistoryText		= RGB( 255, 0, 0 );
	m_crMonitorDownloadLine		= RGB( 0, 0xFF, 0 );
	m_crMonitorUploadLine		= RGB( 0xFF, 0xFF, 0 );
	m_crMonitorDownloadBar		= RGB( 0, 0xBB, 0 );
	m_crMonitorUploadBar		= RGB( 0xBB, 0xBB, 0 );
	m_crMonitorGraphBorder		= RGB( 20, 15, 10 );
	m_crMonitorGraphBack		= RGB( 255, 254, 248 );
	m_crMonitorGraphGrid		= RGB( 230, 230, 180 );
	m_crMonitorGraphLine		= RGB( 252, 20, 10 );

	m_crRatingNull				= RGB( 0x4A, 0x44, 0x40 );
	m_crRating0					= RGB( 0xEA, 0x4A, 0x60 );
	m_crRating1					= RGB( 0xC8, 0xC4, 0xC0 );
	m_crRating2					= RGB( 0xB4, 0xB2, 0xB0 );
	m_crRating3					= RGB( 0x84, 0x82, 0x80 );
	m_crRating4					= RGB( 0x60, 0x5A, 0x55 );
	m_crRating5					= RGB( 0, 0, 0 );

	m_crRichdocBack 			= m_crWindow;
	m_crRichdocText 			= m_crText;
	m_crRichdocHeading 			= RGB( 180, 50, 10 );
	m_crTextAlert 				= RGB( 255, 0, 0 );
	m_crTextStatus 				= RGB( 0, 128, 0 );
	m_crTextLink				= RGB( 0, 0, 255 );
	m_crTextLinkHot				= RGB( 255, 0, 0 );

	m_crChatIn					= RGB( 0, 0, 255 );
	m_crChatOut					= RGB( 255, 0, 0 );
	m_crChatNull				= RGB( 128, 128, 128 );
	m_crSearchExists			= RGB( 0, 128, 0 );
	m_crSearchExistsHit			= RGB( 0, 64, 0 );
	m_crSearchExistsSelected	= RGB( 0, 255, 0 );
	m_crSearchGhostrated		= RGB( 200, 90, 0 );
	m_crSearchQueued 			= RGB( 0, 0, 160 );
	m_crSearchQueuedHit			= RGB( 0, 0, 100 );
	m_crSearchQueuedSelected	= GetSysColor( COLOR_HIGHLIGHTTEXT );
	m_crSearchNull				= m_crSys3DShadow;
	//m_crSearchTorrent 		= CalculateColor( m_crWindow, RGB( 244, 242, 240 ), 10 );
	//m_crSearchCollection 		= CalculateColor( m_crWindow, RGB( 254, 120, 10 ), 25 );
	//m_crSearchHighrated 		= CalculateColor( m_crWindow, RGB( 255, 250, 50 ), 20 );
	m_crTransferSource			= RGB( 30, 30, 30 );
	m_crTransferRanges			= RGB( 220, 240, 220 );
	m_crTransferCompleted		= RGB( 0, 128, 0 );
	m_crTransferVerifyPass		= RGB( 0, 0, 128 );
	m_crTransferVerifyFail		= RGB( 255, 0, 0 );
	m_crTransferCompletedSelected	= RGB( 0, 255, 0 );
	m_crTransferVerifyPassSelected	= RGB( 0, 255, 0 );
	m_crTransferVerifyFailSelected	= RGB( 255, 0, 0 );
	m_crLibraryShared			= RGB( 0, 0, 0 );
	m_crLibraryUnshared 		= RGB( 192, 192, 192 );
	m_crLibraryUnscanned 		= RGB( 128, 128, 128 );
	m_crLibraryUnsafe			= RGB( 255, 0, 0 );
	m_crLogDebug				= RGB( 128, 128, 128 );
	m_crLogInfo 				= RGB( 0, 0, 0 );
	m_crLogNotice				= RGB( 0, 0, 128 );
	m_crLogWarning				= RGB( 250, 128, 64 );
	m_crLogError				= RGB( 255, 0, 0 );
	m_crNetworkNull 			= RGB( 192, 192, 192 );
	m_crNetworkG1				= RGB( 80, 80, 80 );
	m_crNetworkG2				= RGB( 100, 100, 255 );
	m_crNetworkED2K				= RGB( 128, 128, 0 );
	m_crNetworkDC				= RGB( 128, 192, 20 );
	m_crNetworkUp				= RGB( 128, 0, 0 );
	m_crNetworkDown				= RGB( 0, 0, 128 );
	m_crSecurityAllow			= RGB( 0, 128, 0 );
	m_crSecurityDeny			= RGB( 255, 0, 0 );

	m_crDropdownText			= m_crSysMenuText;
	m_crDropdownBox				= m_crSysWindow;
	m_crResizebarEdge			= m_crSysBtnFace;
	m_crResizebarFace			= m_crSysBtnFace;
	m_crResizebarShadow			= m_crSys3DShadow;
	m_crResizebarHighlight		= m_crSys3DHighlight;
	m_crFragmentShaded			= m_crSysBtnFace;
	m_crFragmentComplete		= RGB( 90, 240, 80 );
	m_crFragmentSource1			= RGB( 0, 150, 255 );
	m_crFragmentSource2			= RGB( 0, 150, 0 );
	m_crFragmentSource3			= RGB( 255, 100, 0 );
	m_crFragmentSource4			= RGB( 255, 200, 0 );
	m_crFragmentSource5			= RGB( 150, 150, 255 );
	m_crFragmentSource6			= RGB( 200, 150, 0 );
	m_crFragmentSource7			= RGB( 0, 40, 240 );
	m_crFragmentSource8			= RGB( 200, 190, 180 );
	m_crFragmentPass			= RGB( 0, 220, 0 );
	m_crFragmentFail			= RGB( 220, 0, 0 );
	m_crFragmentRequest			= RGB( 255, 255, 0 );
	m_crFragmentBorder			= RGB( 50, 50, 50 );
	m_crFragmentBorderSelected	= RGB( 50, 50, 50 );
}

void CColors::OnSysColorChange()
{
	if ( ! m_bCustom ) CalculateColors();
}

COLORREF CColors::CalculateColor(COLORREF crFore, COLORREF crBack, int nAlpha)
{
	int nRed	= GetRValue( crFore ) * ( 255 - nAlpha ) / 255 + GetRValue( crBack ) * nAlpha / 255;
	int nGreen	= GetGValue( crFore ) * ( 255 - nAlpha ) / 255 + GetGValue( crBack ) * nAlpha / 255;
	int nBlue	= GetBValue( crFore ) * ( 255 - nAlpha ) / 255 + GetBValue( crBack ) * nAlpha / 255;

	return RGB( nRed, nGreen, nBlue );
}
