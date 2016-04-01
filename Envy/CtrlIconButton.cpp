//
// CtrlIconButton.cpp
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
#include "Envy.h"
#include "CtrlIconButton.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CIconButtonCtrl, CWnd)

BEGIN_MESSAGE_MAP(CIconButtonCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ENABLE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIconButtonCtrl construction

CIconButtonCtrl::CIconButtonCtrl()
	: m_bCursor 	( FALSE )
	, m_bCapture	( FALSE )
	, m_bDown		( FALSE )
{
	m_pImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CIconButtonCtrl operations

BOOL CIconButtonCtrl::Create(const RECT& rect, CWnd* pParentWnd, UINT nControlID, DWORD dwStyle)
{
	if ( CWnd::CreateEx( 0, NULL, L"", dwStyle | WS_CHILD | WS_VISIBLE,
		rect, pParentWnd, nControlID ) )
	{
		// Fix
		GetParent()->IsDlgButtonChecked( nControlID );
		return TRUE;
	}
	return FALSE;
}

void CIconButtonCtrl::SetText(LPCTSTR pszText)
{
	CString strText;
	GetWindowText( strText );
	if ( strText == pszText ) return;
	SetWindowText( pszText );
	Invalidate();
}

void CIconButtonCtrl::SetIcon(HICON hIcon, BOOL bMirrored)
{
	if ( hIcon )
	{
		if ( bMirrored )
		{
			hIcon = CreateMirroredIcon( hIcon );
			ASSERT( hIcon != NULL );
		}
		if ( hIcon )
		{
			if ( m_pImageList.GetImageCount() )
			{
				ASSERT( m_pImageList.GetImageCount() == 1 );
				VERIFY( m_pImageList.Remove( 0 ) );
			}
			VERIFY( m_pImageList.Add( hIcon ) != -1 );
			VERIFY( DestroyIcon( hIcon ) );

			RemoveStyle();
		}
	}
}

void CIconButtonCtrl::SetIcon(UINT nIconID, BOOL bMirrored)
{
	SetIcon( AfxGetApp()->LoadIcon( nIconID ), bMirrored );
}

void CIconButtonCtrl::SetCoolIcon(UINT nIconID, BOOL bMirrored)
{
	SetIcon( CoolInterface.ExtractIcon( nIconID, bMirrored ), FALSE );
}

void CIconButtonCtrl::SetHandCursor(BOOL bCursor)
{
	m_bCursor = bCursor;
}

BOOL CIconButtonCtrl::RemoveStyle()
{
	return CoolInterface.EnableTheme( this, FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CIconButtonCtrl mouse message handlers

void CIconButtonCtrl::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if ( ! IsWindowEnabled() ) return;

	CRect rc;
	GetClientRect( &rc );

	if ( m_bDown )
	{
		BOOL bInside = rc.PtInRect( point );
		if ( bInside == m_bCapture ) return;

		m_bCapture = bInside;
	}
	else if ( m_bCapture )
	{
		if ( rc.PtInRect( point ) ) return;

		ReleaseCapture();
		m_bCapture = FALSE;
	}
	else
	{
		SetCapture();
		if ( m_bCursor ) ::SetCursor( theApp.LoadCursor( IDC_HAND ) );
		m_bCapture = TRUE;
	}

	Invalidate();
}

void CIconButtonCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if ( ! IsWindowEnabled() ) return;

	SetFocus();
	SetCapture();
	m_bDown = TRUE;

	Invalidate();
}

void CIconButtonCtrl::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if ( m_bDown )
	{
		ReleaseCapture();
		m_bDown = FALSE;

		if ( m_bCapture )
		{
			RedrawWindow();

			GetParent()->SendMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), 0 ),
				(LPARAM)GetSafeHwnd() );
		}

		m_bCapture = FALSE;

		Invalidate();
	}
}

void CIconButtonCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );
}

/////////////////////////////////////////////////////////////////////////////
// CIconButtonCtrl paint message handlers

BOOL CIconButtonCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CIconButtonCtrl::OnPaint()
{
	CPaintDC dc( this );
	BOOL bSkinned( FALSE );
	COLORREF crBack( CLR_NONE );
	CString strText;
	CPoint ptIcon;
	CRect rc;

	GetClientRect( &rc );
	GetWindowText( strText );

	BOOL bTextButton = ( strText.IsEmpty() == false );

	ptIcon.x = bTextButton ? ( rc.left + 5 ) : ( ( rc.left + rc.right ) / 2 - 8 );		// Rich Button (left) or Icon Button (centered)
	ptIcon.y = ( rc.top + rc.bottom ) / 2 - 8;

	if ( rc.Width() < 20 || rc.Height() < 20 )		// Don't skin special case small buttons
	{
		dc.FillSolidRect( ptIcon.x - 1, ptIcon.y - 1, rc.right + 1, rc.bottom + 1, Colors.m_crTaskBoxClient );

		if ( m_bDown != m_bCapture ) ptIcon.Offset( -1, -1 );

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, CLR_NONE, CLR_NONE,
			( ! IsWindowEnabled() || ( m_bDown && m_bCapture ) ) ? ILD_BLEND50 : ILD_NORMAL );

		return;
	}

	if ( m_bDown && m_bCapture )		// Pressed
	{
		if ( ! bTextButton && Images.DrawButtonState( &dc, &rc, ICONBUTTON_PRESS ) ) 	// IconButton.Press
		{
			bSkinned = TRUE;
		}
		else if ( Images.DrawButtonState( &dc, &rc, RICHBUTTON_PRESS ) ) 				// RichButton.Press
		{
			dc.SetBkMode( TRANSPARENT );
			bSkinned = TRUE;
			rc.left++;
		}
		else
		{
			dc.Draw3dRect( &rc, Colors.m_crBorder, Colors.m_crBorder );
			crBack = Colors.m_crBackCheckSel;
			rc.DeflateRect( 1, 1 );
		}

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, CLR_NONE, ILD_BLEND50 );
	}
	else if ( m_bDown != m_bCapture )	// Hover  (or Unpressed)
	{
		if ( ! bTextButton && Images.DrawButtonState( &dc, &rc, ICONBUTTON_HOVER ) ) 	// IconButton.Hover
		{
			bSkinned = TRUE;
		}
		else if ( Images.DrawButtonState( &dc, &rc, RICHBUTTON_HOVER ) )					// RichButton.Hover
		{
			dc.SetBkMode( TRANSPARENT );
			bSkinned = TRUE;
			rc.left++;
		}
		else
		{
			dc.Draw3dRect( &rc, Colors.m_crBorder, Colors.m_crBorder );
			crBack = Colors.m_crBackSel;
			rc.DeflateRect( 1, 1 );

			ptIcon.Offset( -1, -1 );
			dc.FillSolidRect( ptIcon.x, ptIcon.y, 18, 2, crBack );
			dc.FillSolidRect( ptIcon.x, ptIcon.y + 2, 2, 16, crBack );

			ptIcon.Offset( 2, 2 );
			dc.SetTextColor( Colors.m_crShadow );
			ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
				ptIcon.x, ptIcon.y, 0, 0, crBack, CLR_NONE, ILD_MASK );

			ptIcon.Offset( -1, -1 );
		}

		ptIcon.Offset( -1, -1 );
		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL );

		dc.ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 16, ptIcon.y + 16 );
		ptIcon.Offset( 1, 1 );
	}
	else if ( IsWindowEnabled() && GetFocus() == this ) 	// Button w/ Cursor Focus
	{
		if ( ! bTextButton && Images.DrawButtonState( &dc, &rc, ICONBUTTON_ACTIVE ) )	// IconButton.Active
		{
			bSkinned = TRUE;
		}
		else if ( Images.DrawButtonState( &dc, &rc, RICHBUTTON_ACTIVE ) )				// RichButton.Active
		{
			dc.SetBkMode( TRANSPARENT );
			bSkinned = TRUE;
			rc.left++;
		}
		else
		{
			dc.Draw3dRect( &rc, Colors.m_crBorder, Colors.m_crBorder );
			crBack = Colors.m_crBackNormal;
			rc.DeflateRect( 1, 1 );
		}

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, CLR_NONE, ILD_NORMAL );
	}
	else if ( IsWindowEnabled() )	// Button Default w/o Focus
	{
		if ( ! bTextButton && Images.DrawButtonState( &dc, &rc, ICONBUTTON_DEFAULT ) )	// IconButton
		{
			bSkinned = TRUE;
		}
		else if ( Images.DrawButtonState( &dc, &rc, RICHBUTTON_DEFAULT ) )				// RichButton
		{
			dc.SetBkMode( TRANSPARENT );
			bSkinned = TRUE;
			rc.left++;
		}
		else
		{
			dc.Draw3dRect( &rc, Colors.m_crShadow, Colors.m_crShadow );
			crBack = Colors.m_crBackNormal;
			rc.DeflateRect( 1, 1 );
		}

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, CLR_NONE, ILD_NORMAL );
	}
	else	// Disabled
	{
		if ( ! bTextButton && Images.DrawButtonState( &dc, &rc, ICONBUTTON_DISABLED ) )	// IconButton.Disabled
		{
			bSkinned = TRUE;
		}
		else if ( Images.DrawButtonState( &dc, &rc, RICHBUTTON_DISABLED ) )				// RichButton.Disabled
		{
			dc.SetBkMode( TRANSPARENT );
			bSkinned = TRUE;
			rc.left++;
		}
		else
		{
			dc.Draw3dRect( &rc, Colors.m_crShadow, Colors.m_crShadow );
			crBack = Colors.m_crMidtone;
			rc.DeflateRect( 1, 1 );
		}

		dc.SetTextColor( Colors.m_crDisabled );
		dc.SetBkColor( crBack );

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, Colors.m_crDisabled, ILD_BLEND50 );	// CLR_NONE, ILD_MASK ?
	}

	if ( ! bSkinned )
		dc.ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 16, ptIcon.y + 16 );

	if ( bTextButton )	// strText
	{
		rc.left += ptIcon.x + 16 + 2;	// Text Offset

		CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );

		if ( ! bSkinned )
			dc.SetBkColor( crBack );
		dc.SetTextColor( IsWindowEnabled() ? Colors.m_crCmdText : Colors.m_crDisabled );
		dc.ExtTextOut( rc.left, ptIcon.y + 1, ETO_CLIPPED|( ! bSkinned ? ETO_OPAQUE : 0 ), &rc, strText, NULL );
		dc.SelectObject( pOldFont );

		if ( ! bSkinned )	// Fill icon area not covered by opaque text
			dc.FillSolidRect( rc.left - ( ptIcon.x + 16 + 2 ), rc.top, rc.left, rc.bottom - 1, crBack );
	}
	else if ( ! bSkinned )
		dc.FillSolidRect( &rc, crBack );
}

void CIconButtonCtrl::OnEnable(BOOL bEnable)
{
	CWnd::OnEnable( bEnable );
	Invalidate();
}

void CIconButtonCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus( pOldWnd );
	Invalidate();
}

void CIconButtonCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus( pNewWnd );
	Invalidate();
}

UINT CIconButtonCtrl::OnGetDlgCode()
{
	return ( GetStyle() & BS_DEFPUSHBUTTON ) ? DLGC_DEFPUSHBUTTON : DLGC_UNDEFPUSHBUTTON;
}

BOOL CIconButtonCtrl::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_SPACE )
	{
		GetParent()->PostMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), BN_CLICKED ), (LPARAM)GetSafeHwnd() );
		return TRUE;
	}
	return CWnd::PreTranslateMessage( pMsg );
}
