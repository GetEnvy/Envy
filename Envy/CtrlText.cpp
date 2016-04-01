//
// CtrlText.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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

// System Window Log Display (Network Tab)

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "CtrlText.h"

#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define LINE_BUFFER_LIMIT		4096
#define LINE_BUFFER_BLOCK		64

#define LINE_GAP				1	// px
#define OFFSET					4	// px


IMPLEMENT_DYNCREATE(CTextCtrl, CWnd)

BEGIN_MESSAGE_MAP(CTextCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextCtrl construction

CTextCtrl::CTextCtrl()
	: m_nPosition	( 0 )
	, m_nTotal		( 0 )
	, m_nHeight		( 0 )
	, m_bProcess	( TRUE )
	, m_nLastClicked ( -1 )
{
	// Severity (Text)
	//m_crText[0] = Colors.m_crLog...
	OnSkinChange();

	m_pFont.CreateFont( -(int)Settings.Fonts.DefaultSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.SystemLogFont );
}

CTextCtrl::~CTextCtrl()
{
	Clear( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CTextCtrl operations

BOOL CTextCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VSCROLL;
	return CWnd::Create( NULL, NULL, dwStyle, rect, pParentWnd, nID, NULL );
}

void CTextCtrl::Add(const CLogMessage* pMsg)
{
	CString strTime;
	if ( Settings.General.ShowTimestamp )
		strTime.Format( L"[%02d:%02d:%02d]  ", pMsg->m_Time.GetHour(), pMsg->m_Time.GetMinute(), pMsg->m_Time.GetSecond() );

	CQuickLock pLock( m_pSection );

	for ( int pos = 0 ; ; )
	{
		CString strLine = pMsg->m_strLog.Tokenize( L"\r\n", pos );
		if ( strLine.IsEmpty() )
			break;
		if ( Settings.General.ShowTimestamp )
			AddLine( pMsg->m_nType, strTime + strLine );
		else
			AddLine( pMsg->m_nType, strLine );
	}
}

void CTextCtrl::AddLine(WORD nType, const CString& strLine)
{
	ASSERT( ( nType & MSG_SEVERITY_MASK ) < ( sizeof m_crText / sizeof m_crText[0] ) );
//	ASSERT( ( ( nType & MSG_FACILITY_MASK ) >> 8 ) < ( sizeof m_crBackground / sizeof m_crBackground[0] ) );

	CQuickLock pLock( m_pSection );

	if ( m_pLines.GetSize() >= LINE_BUFFER_LIMIT )
	{
		for ( int nCount = 0 ; nCount < LINE_BUFFER_BLOCK ; nCount++ )
		{
			delete m_pLines.GetAt( nCount );
		}
		m_pLines.RemoveAt( 0, LINE_BUFFER_BLOCK );

		m_bProcess = TRUE;

		if ( m_nLastClicked < LINE_BUFFER_BLOCK )
			m_nLastClicked = -1;
		else
			m_nLastClicked -= LINE_BUFFER_BLOCK;
	}

	m_pLines.Add( new CTextLine( nType, strLine ) );

	Invalidate();
}

void CTextCtrl::Clear(BOOL bInvalidate)
{
	CQuickLock pLock( m_pSection );

	for ( int nLine = 0 ; nLine < m_pLines.GetSize() ; nLine++ )
	{
		delete m_pLines.GetAt( nLine );
	}
	m_pLines.RemoveAll();

	m_nPosition = m_nTotal = 0;

	m_nLastClicked = -1;

	if ( bInvalidate )
	{
		UpdateScroll( TRUE );
		Invalidate();
	}
}

void CTextCtrl::UpdateScroll(BOOL bFull)
{
	SCROLLINFO si = {};
	si.cbSize	= sizeof( si );
	si.fMask	= SIF_POS;
	si.nPos		= m_nPosition;

	if ( bFull )
	{
		CRect rc;
		GetClientRect( &rc );

		si.fMask	= SIF_POS|SIF_PAGE|SIF_RANGE|SIF_DISABLENOSCROLL;
		si.nPage	= rc.Height() / m_nHeight;
		si.nMax		= m_nTotal + si.nPage - 1;
		si.nMin		= 0;
	}

	SetScrollInfo( SB_VERT, &si );
}

/////////////////////////////////////////////////////////////////////////////
// CTextCtrl message handlers

void CTextCtrl::OnVScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar* /*pScrollBar*/)
{
	CQuickLock pLock( m_pSection );

	SCROLLINFO si = {};
	si.cbSize	= sizeof( si );
	si.fMask	= SIF_ALL;

	GetScrollInfo( SB_VERT, &si );

	switch ( nSBCode )
	{
	case SB_TOP:
		m_nPosition = 1;
		break;
	case SB_BOTTOM:
		m_nPosition = m_nTotal;
		break;
	case SB_LINEUP:
		m_nPosition--;
		break;
	case SB_LINEDOWN:
		m_nPosition++;
		break;
	case SB_PAGEUP:
		m_nPosition -= ( si.nPage - 1 );
		break;
	case SB_PAGEDOWN:
		m_nPosition += ( si.nPage - 1 );
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nPosition = si.nTrackPos;
		break;
	}

	m_nPosition = max( 0, min( m_nTotal, m_nPosition ) );

	UpdateScroll();
	Invalidate();
}

BOOL CTextCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTextCtrl::OnPaint()
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CQuickLock pLock( m_pSection );
	CPaintDC dc( this );

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFont );

	if ( ! m_nHeight )
	{
		CSize size = dc.GetTextExtent( L"X" );
		m_nHeight = size.cy += LINE_GAP;
	}

	const int nWidth = rcClient.right - OFFSET;

	BOOL bBottom	= ( m_nPosition >= m_nTotal );
	BOOL bModified	= m_bProcess;

	if ( m_bProcess ) m_nTotal = 0;

	for ( int nLine = 0 ; nLine < m_pLines.GetSize() ; nLine++ )
	{
		CTextLine* pLine = m_pLines.GetAt( nLine );

		if ( m_bProcess || ! pLine->m_nLine )
		{
			m_nTotal += pLine->Process( &dc, nWidth );
			bModified = TRUE;
		}
	}

	if ( bBottom ) m_nPosition = m_nTotal;
	if ( bModified ) UpdateScroll( TRUE );
	m_bProcess = FALSE;

	CRect rcLine( rcClient );
	rcLine.bottom += ( m_nTotal - m_nPosition ) * m_nHeight;
	rcLine.top = rcLine.bottom - m_nHeight;		// Note: Consider multi-line wrap and gap elsewhere

	dc.SetBkMode( OPAQUE );

	for ( INT_PTR nLine = m_pLines.GetSize() - 1 ; nLine >= 0 && rcLine.bottom > 0 ; nLine-- )
	{
		CTextLine* pLine = m_pLines.GetAt( nLine );
		const WORD nType = pLine->m_nType & MSG_SEVERITY_MASK;
		if ( pLine->m_bSelected && Images.m_bmSelected.m_hObject )	// Skinned
		{
			CRect rcPaint( rcLine );
			rcPaint.bottom++;	// Set wider for highlight
			if ( pLine->m_nLine > 1 )
				rcPaint.top -= ( pLine->m_nLine - 1 ) * m_nHeight;
			dc.SetBkMode( TRANSPARENT );
			CoolInterface.DrawWatermark( &dc, &rcPaint, &Images.m_bmSelected, FALSE ); 	// No overdraw
			dc.SetTextColor( nType > 1 ? Colors.m_crHiText : m_crText[ nType ] );		// Visibility fix
			pLine->Paint( &dc, &rcLine, TRUE );
			dc.SetBkMode( OPAQUE );
		}
		else // Default
		{
			dc.SetTextColor( pLine->m_bSelected ? Colors.m_crHiText : m_crText[ nType ] );
			dc.SetBkColor( pLine->m_bSelected ? Colors.m_crHighlight : Colors.m_crWindow );	 // ToDo: Fix m_crBackground[ ( pLine->m_nType & MSG_FACILITY_MASK ) >> 8 ]
			pLine->Paint( &dc, &rcLine );
		}
	}

	if ( rcLine.bottom > 0 )
	{
		rcLine.top = 0;
		dc.FillSolidRect( &rcLine, Colors.m_crWindow ); 	// m_crBackground[ 0 ]
	}

	dc.SelectObject( pOldFont );
}

int CTextCtrl::HitTest(const CPoint& pt) const
{
	CQuickLock pLock( m_pSection );

	if ( m_nHeight != 0 )
	{
		CRect rcClient;
		GetClientRect( &rcClient );
		CRect rcLine( rcClient );
		rcLine.bottom += ( m_nTotal - m_nPosition ) * m_nHeight;
		for ( int nLine = m_pLines.GetCount() - 1 ; nLine >= 0 && rcLine.bottom > rcClient.top ; nLine-- )
		{
			CTextLine* pLine = m_pLines.GetAt( nLine );
			rcLine.top = rcLine.bottom - pLine->m_nLine * m_nHeight;
			if ( rcLine.PtInRect( pt ) )
				return nLine;
			rcLine.bottom -= pLine->m_nLine * m_nHeight;
		}
	}
	return -1;
}

void CTextCtrl::CopyText() const
{
	CString str;
	BOOL bGotIt = FALSE;

	{
		CQuickLock pLock( m_pSection );

		for ( int i = 0 ; i < m_pLines.GetCount() ; i++ )
		{
			CTextLine* pLineTemp = m_pLines.GetAt( i );
			if ( pLineTemp->m_bSelected )
			{
				str += pLineTemp->m_sText + L"\r\n";
				bGotIt = TRUE;
			}
		}

		if ( ! bGotIt && m_nLastClicked != -1 )
		{
			CTextLine* pLineTemp = m_pLines.GetAt( m_nLastClicked );
			str = pLineTemp->m_sText;
			bGotIt = TRUE;
		}
	}

	if ( bGotIt )
		theApp.SetClipboard( str );
}

void CTextCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );
	m_bProcess = TRUE;
}

void CTextCtrl::OnSkinChange()
{
	// Severity (Text)
	m_crText[0] = Colors.m_crLogError;			// red			- MSG_ERROR
	m_crText[1] = Colors.m_crLogWarning;		// orange		- MSG_WARNING
	m_crText[2] = Colors.m_crLogNotice;			// dark blue	- MSG_NOTICE
	m_crText[3] = Colors.m_crLogInfo;			// black		- MSG_INFO
	m_crText[4] = Colors.m_crLogDebug;			// gray			- MSG_DEBUG

	// Facility (Window) DISABLED	ToDo: Fix MSG_FACILITY_MASK
	//m_crBackground[0] = Colors.m_crWindow;	// whitespace	- MSG_FACILITY_DEFAULT
	//m_crBackground[1] = RGB( 255, 255, 224 );	// light yellow	- MSG_FACILITY_SEARCH
	//m_crBackground[2] = RGB( 224, 255, 224 );	// light green	- MSG_FACILITY_INCOMING
	//m_crBackground[3] = RGB( 224, 240, 255 );	// light blue	- MSG_FACILITY_OUTGOING

	OnSize( 0, 0, 0 );
}

void CTextCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CQuickLock pLock( m_pSection );

	const int nLine = HitTest( point );
	if ( nLine != -1 )
	{
		CTextLine* pLine = m_pLines.GetAt( nLine );

		if ( ( nFlags & MK_CONTROL ) == MK_CONTROL )
		{
			// Invert
			pLine->m_bSelected = ! pLine->m_bSelected;
			m_nLastClicked = nLine;
		}
		else if ( ( nFlags & MK_SHIFT ) == MK_SHIFT )
		{
			// Select from m_nLastClicked to nLine
			if ( m_nLastClicked == -1 )
				m_nLastClicked = nLine;

			for ( int i = 0 ; i < m_pLines.GetCount() ; i++ )
			{
				CTextLine* pLineTemp = m_pLines.GetAt( i );
				pLineTemp->m_bSelected = ( m_nLastClicked < nLine ) ?
					( i >= m_nLastClicked && i <= nLine ) :
					( i <= m_nLastClicked && i >= nLine );
			}
		}
		else
		{
			// Select one, unselect others
			for ( int i = 0 ; i < m_pLines.GetCount() ; i++ )
			{
				CTextLine* pLineTemp = m_pLines.GetAt( i );
				pLineTemp->m_bSelected = ( pLineTemp == pLine );
			}
			m_nLastClicked = nLine;
		}
	}

	InvalidateRect( NULL );
}

void CTextCtrl::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	CQuickLock pLock( m_pSection );

	if ( m_nLastClicked == -1 )
		m_nLastClicked = HitTest( point );
}

void CTextCtrl::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if ( GetKeyState( VK_CONTROL ) < 0 )
	{
		switch ( nChar )
		{
		// Ctrl+C, Ctrl+X or Ctrl+Insert
		case 'C':
		case 'X':
		case VK_INSERT:
			CopyText();
			break;

		// Ctrl+A = Select all
		case 'A':
			{
				CQuickLock pLock( m_pSection );
				for ( int i = 0 ; i < m_pLines.GetCount() ; i++ )
				{
					CTextLine* pLineTemp = m_pLines.GetAt( i );
					pLineTemp->m_bSelected = TRUE;
				}
			}
			InvalidateRect( NULL );
			break;
		}
	}

	if ( nChar == VK_ESCAPE )
	{
		// Esc = Unselect all
		CQuickLock pLock( m_pSection );
		for ( int i = 0, nCount = m_pLines.GetCount() ; i < nCount ; i++ )
		{
			CTextLine* pLineTemp = m_pLines.GetAt( i );
			pLineTemp->m_bSelected = FALSE;
		}
		InvalidateRect( NULL );
	}
}

BOOL CTextCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int nScroll = zDelta / WHEEL_DELTA * theApp.m_nMouseWheel;

	if ( theApp.m_nMouseWheel == 20 )	// 20 lines set for rare WHEEL_PAGESCROLL (UINT_MAX)
	{
		// Scroll by page is activated
		SCROLLINFO si = {};
		si.cbSize	= sizeof( si );
		si.fMask	= SIF_ALL;
		GetScrollInfo( SB_VERT, &si );

		nScroll = zDelta / WHEEL_DELTA * ( si.nPage - 1 );
	}

	{
		CQuickLock pLock( m_pSection );

		m_nPosition -= nScroll;
		m_nPosition = max( 0, min( m_nTotal, m_nPosition ) );
	}

	UpdateScroll();
	Invalidate();

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


/////////////////////////////////////////////////////////////////////////////
// CTextLine construction

CTextLine::CTextLine(WORD nType, const CString& strText)
	: m_sText	( strText )
	, m_pLine	( NULL )
	, m_nLine	( 0 )
	, m_nType	( nType )
	, m_bSelected ( FALSE )
{
}

CTextLine::~CTextLine()
{
	delete [] m_pLine;
}

/////////////////////////////////////////////////////////////////////////////
// CTextLine process

int CTextLine::Process(CDC* pDC, int nWidth)
{
	delete [] m_pLine;
	m_pLine = NULL;
	m_nLine = 0;

	// Single-line
	if ( m_sText.GetLength() < 100 ||
		 pDC->GetTextExtent( m_sText ).cx < nWidth )
	{
		AddLine( m_sText.GetLength() );
		return m_nLine;
	}

	// Multi-line wrap
	static const CSize size = pDC->GetTextExtent( L"X" );
	const int nMax = nWidth / size.cx + 4;

	int nLength = 0;
	int nLast = 0;

	for ( LPCTSTR pszText = m_sText ; ; pszText++, nLength++ )
	{
		if ( *pszText == 32 || *pszText == 0 )
		{
			if ( nLength <= nMax )
			{
				nLast = nLength;
			}
			else if ( nLast )
			{
				AddLine( nLast );
				nLength = nLast = nLength - nLast;
			}
			else
			{
				break;
			}
		}

		if ( *pszText == 0 ) break;
	}

	if ( nLength || ! m_nLine )
		AddLine( nLength );

	return m_nLine;
}

void CTextLine::AddLine(int nLength)
{
	int* pLine = new int[ m_nLine + 1 ];
	if ( m_pLine ) CopyMemory( pLine, m_pLine, m_nLine * sizeof( int ) );
	delete [] m_pLine;
	m_pLine = pLine;
	m_pLine[ m_nLine++ ] = nLength;
}

/////////////////////////////////////////////////////////////////////////////
// CTextLine paint

void CTextLine::Paint(CDC* pDC, CRect* pRect, BOOL bSkinned)
{
	const int nHeight = pRect->bottom - pRect->top;		// m_nHeight
	LPCTSTR pszLine = m_sText;

	// Single-line increment:

	if ( m_nLine == 1 )
	{
		if ( m_pLine && pDC->RectVisible( pRect ) )
		{
			pDC->ExtTextOut( pRect->left + OFFSET, pRect->top,
				ETO_CLIPPED|( bSkinned ? 0 : ETO_OPAQUE ),
				pRect, pszLine, m_pLine[ 0 ], NULL );
		}

		pRect->top -= nHeight;
		pRect->bottom -= nHeight;

		return;
	}

	// Multi-line wrap:

	pRect->top		-= ( m_nLine - 1 ) * nHeight;
	pRect->bottom	-= ( m_nLine - 1 ) * nHeight;

	for ( int nLine = 0 ; nLine < m_nLine ; nLine++ )
	{
		if ( m_pLine )
		{
			if ( pDC->RectVisible( pRect ) )
			{
				pDC->ExtTextOut( pRect->left + OFFSET, pRect->top,
					ETO_CLIPPED|( bSkinned ? 0 : ETO_OPAQUE ),
					pRect, pszLine, m_pLine[ nLine ], NULL );
			}
			pszLine += m_pLine[ nLine ];
		}

		pRect->top += nHeight;
		pRect->bottom += nHeight;
	}

	pRect->top		-= ( m_nLine + 1 ) * nHeight;
	pRect->bottom	-= ( m_nLine + 1 ) * nHeight;
}
