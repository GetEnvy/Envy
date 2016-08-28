//
// DlgSplash.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2008
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
#include "DlgSplash.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "FragmentBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define SPLASH_WIDTH		500
#define SPLASH_HEIGHT		200

#define COLOR_DEFAULT		RGB( 254, 186, 20 )
#define COLOR_TEXT			RGB( 255, 255, 255 )
#define COLOR_TEXT_SHADOW	RGB( 234, 164, 0 )
#define COLOR_TEXT_FADE		RGB( 244, 178, 10 )
#define COLOR_BAR_FILL		RGB( 242, 178, 10 )
#define COLOR_BAR_PROGRESS	RGB( 254, 250, 246 )
//#define COLOR_BAR_UPPEREDGE	RGB( 236, 230, 220 )
//#define COLOR_BAR_LOWEREDGE	RGB( 236, 230, 220 )

//IMPLEMENT_DYNAMIC(CSplashDlg, CDialog)

BEGIN_MESSAGE_MAP(CSplashDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYENDSESSION()
END_MESSAGE_MAP()

CBitmap CSplashDlg::m_bmSplash;		// Static


/////////////////////////////////////////////////////////////////////////////
// CSplashDlg construction

CSplashDlg::CSplashDlg(int nMax, bool bClosing)
	: CDialog( CSplashDlg::IDD, GetDesktopWindow() )
	, m_nWidth		( SPLASH_WIDTH )
	, m_nHeight		( SPLASH_HEIGHT )
	, m_nPos		( 0 )
	, m_nMax		( nMax )
	, m_bClosing	( bClosing )
	, m_sState		( Settings.SmartAgent() )	//  Was theApp.m_sSmartAgent
{
	if ( ! m_bmSplash.m_hObject )
		m_bmSplash.Attach( CImageFile::LoadBitmapFromFile( Settings.General.DataPath + L"Splash.png" ) );

	Create( IDD, GetDesktopWindow() );
}

//CSplashDlg::~CSplashDlg()
//{
//}

void CSplashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CSplashDlg message handlers

BOOL CSplashDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

//	SetClassLongPtr( GetSafeHwnd(), GCL_STYLE, GetClassLongPtr( GetSafeHwnd(), GCL_STYLE ) | CS_SAVEBITS | CS_DROPSHADOW );

	SetWindowText( m_sState );

	//CImageFile pFile;
	//if ( pFile.LoadFromFile( Settings.General.DataPath + L"Splash.png" ) )
	//{
	//	pFile.EnsureRGB();
	//	HBITMAP bmHandle = pFile.CreateBitmap();
	//	m_bmSplash.Attach( bmHandle );
	//}
	//else if ( pFile.LoadFromResource( AfxGetResourceHandle(), IDR_LARGE_LOGO, RT_PNG ) )
	//{
	//	// ToDo: Built-in media splash currently works as fallback, but this should be commented out if changed. (Note flat-color otherwise)
	//	HBITMAP bmHandle = pFile.CreateBitmap();
	//	m_bmSplash.Attach( bmHandle );
	//}

	if ( m_bmSplash.m_hObject )
	{
		BITMAP bmInfo;
		m_bmSplash.GetObject( sizeof( BITMAP ), &bmInfo );
		if ( bmInfo.bmHeight > 20 && bmInfo.bmWidth > 280 )
		{
			m_nWidth  = bmInfo.bmWidth;
			m_nHeight = bmInfo.bmHeight;
		}
	}

	SetWindowPos( NULL, 0, 0, m_nWidth, m_nHeight, SWP_NOMOVE );
	CenterWindow();

	//if ( GetSystemMetrics( SM_REMOTESESSION ) == 0 )	// Why?
		AnimateWindow( 180, AW_BLEND );

	SetWindowPos( &wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW );
	UpdateWindow();

	return TRUE;
}

BOOL CSplashDlg::OnQueryEndSession()
{
	UpdateWindow();

	CDialog::OnQueryEndSession();

	return FALSE;
}

void CSplashDlg::Step(LPCTSTR pszText)
{
	// Check if m_nMax was set high enough during construction to allow another step to take place
	ASSERT( m_nPos < m_nMax );

	m_nPos ++;
	m_sState.Format( m_bClosing ? L"%s..." : L"Starting %s...", pszText );
	SetWindowText( m_sState );

	if ( theApp.m_pfnShutdownBlockReasonCreate )	// Vista+
		theApp.m_pfnShutdownBlockReasonCreate( m_hWnd, m_sState );

	CClientDC dc( this );
	DoPaint( &dc );

	if ( IsWindowVisible() )
		Sleep( 50 );	// Allow brief text and progress bar movement  (Waste a second for the appearance of speed)
}

void CSplashDlg::Update(LPCTSTR pszText /*NULL*/)
{
	m_sState.Format( m_bClosing ? L"%s..." : L"Starting %s...", pszText );
	SetWindowText( m_sState );

	CClientDC dc( this );
	DoPaint( &dc );
}

void CSplashDlg::Topmost()
{
	if ( IsWindowVisible() )
		SetWindowPos( &wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW );
}

void CSplashDlg::Hide(BOOL bAbort)
{
	if ( ! bAbort )
	{
		// Check if m_nMax was set too high during construction, or if not enough steps were run
		//ASSERT( m_nPos == m_nMax );

		LoadString( m_sState, AFX_IDS_IDLEMESSAGE );	// "Ready"
		SetWindowText( m_sState );
		Invalidate();

		// MFC Windows transition effect  (Fade, etc.)
		//if ( GetSystemMetrics( SM_REMOTESESSION ) == 0 )	// Why?
			AnimateWindow( 180, AW_HIDE|AW_BLEND );
	}

	if ( theApp.m_pfnShutdownBlockReasonDestroy )		// Vista+
		theApp.m_pfnShutdownBlockReasonDestroy( m_hWnd );

	::DestroyWindow( m_hWnd );
	delete this;
}

void CSplashDlg::OnPaint()
{
	CPaintDC dc( this );
	DoPaint( &dc );
}

void CSplashDlg::DoPaint(CDC* pDC)
{
	CDC dcSplash;		// Was m_dcBuffer1
	dcSplash.CreateCompatibleDC( pDC );
	CBitmap* pOld1 = (CBitmap*)dcSplash.SelectObject( &m_bmSplash );

	CDC dcMemory;		// Was m_dcBuffer2
	dcMemory.CreateCompatibleDC( pDC );

	CBitmap bmBuffer;
	bmBuffer.CreateCompatibleBitmap( pDC, m_nWidth, m_nHeight );
	CBitmap* pOld2 = (CBitmap*)dcMemory.SelectObject( &bmBuffer );

	dcMemory.BitBlt( 0, 0, m_nWidth, m_nHeight, &dcSplash, 0, 0, SRCCOPY );

	if ( m_bmSplash.m_hObject )
		dcMemory.BitBlt( 0, 0, m_nWidth, m_nHeight, &dcSplash, 0, 0, SRCCOPY );
	else // Missing File
		dcMemory.FillSolidRect( 0, 0, SPLASH_WIDTH, SPLASH_HEIGHT, COLOR_DEFAULT );	// Default Splash Color

	CFont* pOldFont = (CFont*)dcMemory.SelectObject( &theApp.m_gdiFontBold );
	dcMemory.SetBkMode( TRANSPARENT );

	CRect rc( 8, m_nHeight - 18, m_nWidth - 98, m_nHeight - 2 );				// Text Position
	const UINT nFormat = DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX;

	dcMemory.SetTextColor( COLOR_TEXT_FADE );									// Text Outline/Fade
	rc.left--;
	rc.top += 2;
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.top -= 4;
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.left += 2;
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.left++;
	rc.top += 5;
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.left--;
	rc.top--;
	dcMemory.SetTextColor( COLOR_TEXT_SHADOW );									// Text Shadow
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.left--;
	rc.top -= 2;

	dcMemory.SetTextColor( COLOR_TEXT );										// Text Color
	dcMemory.DrawText( m_sState, &rc, nFormat );

	dcMemory.SelectObject( pOldFont );

	rc.SetRect( m_nWidth - 90, m_nHeight - 14, m_nWidth - 8, m_nHeight - 5 );	// Progress Bar Position ( 440, 222, 522, 231 )
//	dcMemory.Draw3dRect( &rc, COLOR_BAR_UPPEREDGE, COLOR_BAR_LOWEREDGE );		// Progress Bar Outline
	rc.DeflateRect( 1, 1 );
	dcMemory.FillSolidRect( &rc, COLOR_BAR_FILL ); 								// Progress Bar Background

	int nOffset = 0;
	if ( Settings.General.LanguageRTL )
		nOffset = m_nMax - min( m_nPos, m_nMax );

	CFragmentBar::DrawFragment( &dcMemory, &rc, m_nMax, nOffset, min( m_nPos, m_nMax ), COLOR_BAR_PROGRESS, FALSE );		// Progress Bar Color (No 3d edge)
	dcMemory.SelectClipRgn( NULL );

	pDC->BitBlt( 0, 0, m_nWidth, m_nHeight, &dcMemory, 0, 0, SRCCOPY );

	dcMemory.SelectObject( pOld2 );
	dcSplash.SelectObject( pOld1 );
}
