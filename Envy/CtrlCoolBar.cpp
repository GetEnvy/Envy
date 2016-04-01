//
// CtrlCoolBar.cpp
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
#include "CtrlCoolBar.h"

#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CCoolBarCtrl, CControlBar)

BEGIN_MESSAGE_MAP(CCoolBarCtrl, CControlBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

//Skin  TOOLBAR_HEIGHT		28	// Settings.Skin.ToolbarHeight
#define DEFAULT_HEIGHT		30
#define GRIPPER_WIDTH		4
#define SEPARATOR_WIDTH		7
#define MARGIN_WIDTH		5

#define BUTTON_WIDTH		9
#define IMAGE_WIDTH			16
#define IMAGEBUTTON_WIDTH	25	// ( BUTTON_WIDTH + IMAGE_WIDTH )
#define TEXT_GAP			4
#define CONTROL_HEIGHT		19


/////////////////////////////////////////////////////////////////////////////
// CCoolBar construction

CCoolBarCtrl::CCoolBarCtrl()
	: m_nHeight 	( DEFAULT_HEIGHT )
	, m_bStretch	( FALSE )
	, m_bGripper	( FALSE )
	, m_bBold		( FALSE )
	, m_bDragForward( FALSE )
	, m_pSyncObject	( NULL )
	, m_tLastUpdate ( 0 )		// No static
	, m_tHoverTime	( 0 )
	, m_bBuffered	( FALSE )
	, m_bMenuGray	( FALSE )
	, m_pDown		( NULL )
	, m_pHot		( NULL )
	, m_bTimer		( FALSE )
	, m_crBack		( 0 )
	, m_bRecalc 	( FALSE )
	, m_bDropEnabled( FALSE )
{
}

CCoolBarCtrl::~CCoolBarCtrl()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBar system operations

BOOL CCoolBarCtrl::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc( 0, 0, 0, 0 );
	return CWnd::CreateEx( WS_EX_CONTROLPARENT, NULL, L"CCoolBarCtrl",
		dwStyle | WS_CHILD, rc, pParentWnd, nID, NULL );
}

void CCoolBarCtrl::SetSize(int nHeight, BOOL bStretch)
{
	m_bStretch = bStretch;
	m_nHeight  = nHeight < 1 ? DEFAULT_HEIGHT : nHeight;

	SetWindowPos( NULL, 0, 0, 0, 0,
		SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER );
}

void CCoolBarCtrl::SetGripper(BOOL bGripper)
{
	m_bGripper = bGripper;
}

void CCoolBarCtrl::SetBold(BOOL bBold)
{
	m_bBold = bBold;
	OnUpdated();
}

void CCoolBarCtrl::SetDragForward(BOOL bForward)
{
	m_bDragForward = bForward;
}

void CCoolBarCtrl::SetSyncObject(CSyncObject* pSyncObject)
{
	m_pSyncObject = pSyncObject;
}

void CCoolBarCtrl::SetWatermark(HBITMAP hBitmap, BOOL bDetach)
{
	if ( m_bmImage.m_hObject )
	{
		if ( bDetach )
			m_bmImage.Detach();
		else
			m_bmImage.DeleteObject();
	}

	if ( hBitmap ) m_bmImage.Attach( hBitmap );
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBar item operations

CCoolBarItem* CCoolBarCtrl::Add(UINT nID, LPCTSTR pszText, int nPosition)
{
	CCoolBarItem* pItem = new CCoolBarItem( this, nID );

	if ( nPosition == -1 )
	{
		m_pItems.AddTail( pItem );
	}
	else
	{
		if ( POSITION pos = m_pItems.FindIndex( nPosition ) )
			m_pItems.InsertBefore( pos, pItem );
		else
			m_pItems.AddTail( pItem );
	}

	pItem->m_nImage = CoolInterface.ImageForID( nID );

	if ( pszText ) pItem->SetText( pszText );

	return pItem;
}

CCoolBarItem* CCoolBarCtrl::Add(UINT nCtrlID, int nWidth, int nHeight)
{
	CCoolBarItem* pItem = new CCoolBarItem( this, nCtrlID );
	m_pItems.AddTail( pItem );

	pItem->m_nCtrlID		= nCtrlID;
	pItem->m_nWidth			= nWidth;
	pItem->m_nCtrlHeight	= nHeight ? nHeight : CONTROL_HEIGHT;

	return pItem;
}

CCoolBarItem* CCoolBarCtrl::GetIndex(int nIndex) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = m_pItems.GetNext( pos );
		if ( ! nIndex-- ) return pItem;
	}

	return NULL;
}

CCoolBarItem* CCoolBarCtrl::GetID(UINT nID) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = m_pItems.GetNext( pos );
		if ( pItem->m_nID == nID ) return pItem;
	}

	return NULL;
}

int CCoolBarCtrl::GetIndexForID(UINT nID) const
{
	int nIndex = 0;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; nIndex++ )
	{
		CCoolBarItem* pItem = m_pItems.GetNext( pos );
		if ( pItem->m_nID == nID ) return nIndex;
	}

	return -1;
}

BOOL CCoolBarCtrl::LoadToolBar(UINT nIDToolBar)
{
	CToolBar pToolBar;

	if ( ! pToolBar.Create( this ) || ! pToolBar.LoadToolBar( nIDToolBar ) ) return FALSE;

	for ( int i = 0 ; i < pToolBar.GetCount() ; i++ )
	{
		UINT nID, nStyle;
		int nImage;

		pToolBar.GetButtonInfo( i, nID, nStyle, nImage );

		Add( nID );
	}

	return TRUE;
}

void CCoolBarCtrl::Clear()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete m_pItems.GetNext( pos );
	}

	m_pItems.RemoveAll();
}

void CCoolBarCtrl::Copy(CCoolBarCtrl* pOther)
{
	Clear();

	for ( POSITION pos = pOther->m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = pOther->m_pItems.GetNext( pos );
		m_pItems.AddTail( new CCoolBarItem( this, pItem ) );
	}

	Invalidate();
}

UINT CCoolBarCtrl::ThrowMenu(UINT nID, CMenu* pMenu, CWnd* pParent, BOOL bCommand, BOOL bRight)
{
	if ( pMenu == NULL ) return 0;
	if ( pParent == NULL ) pParent = AfxGetMainWnd();
	if ( pParent == NULL ) pParent = this;

	m_pDown = GetID( nID );
	if ( m_pDown == NULL ) return 0;

	m_bMenuGray = TRUE;
	Invalidate();
	UpdateWindow();

	CRect rcButton;
	GetItemRect( m_pDown, &rcButton );
	ClientToScreen( &rcButton );
	rcButton.DeflateRect( 1, 2 );

	TPMPARAMS tpm;
	tpm.cbSize = sizeof( tpm );
	tpm.rcExclude = rcButton;

	DWORD nFlags = TPM_LEFTBUTTON|TPM_VERTICAL;

	if ( bCommand ) nFlags |= TPM_RETURNCMD;

//#if 1		// Why?
	CoolMenu.RegisterEdge( Settings.General.LanguageRTL ? rcButton.right : rcButton.left,
		rcButton.bottom, rcButton.Width() );
	bRight = FALSE;
//#endif

	nFlags |= ( bRight ? TPM_RIGHTALIGN : TPM_LEFTALIGN );

	UINT nCmd = TrackPopupMenuEx( pMenu->GetSafeHmenu(), nFlags,
		bRight || Settings.General.LanguageRTL ? rcButton.right : rcButton.left, rcButton.bottom,
		pParent->GetSafeHwnd(), &tpm );

	m_bMenuGray = FALSE;
	m_pDown = NULL;
	Invalidate();

	return nCmd;
}

void CCoolBarCtrl::OnUpdated()
{
	if ( ! m_bStretch )
	{
		CSize czLast = m_czLast;

		if ( CalcFixedLayout( FALSE, TRUE ) != czLast && m_hWnd )
		{
			CMDIFrameWnd* pOwner = (CMDIFrameWnd*)GetOwner();

			if ( pOwner && pOwner->IsKindOf( RUNTIME_CLASS(CMDIFrameWnd) ) )
			{
				if ( pOwner->IsIconic() )
					m_bRecalc = TRUE;
				else
					pOwner->RecalcLayout();
			}
		}
	}

	if ( m_hWnd )
		Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBar message handlers

int CCoolBarCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 )
		return -1;

//	if ( Settings.Skin.MenuBorders )
	m_dwStyle |= CBRS_BORDER_3D;

	if ( m_bDropEnabled ) ENABLE_DROP()

	return 0;
}

void CCoolBarCtrl::OnDestroy()
{
	if ( m_bDropEnabled )
		DISABLE_DROP()

	KillTimer( 1 );

	CControlBar::OnDestroy();
}

CSize CCoolBarCtrl::CalcFixedLayout(BOOL bStretch, BOOL /*bHorz*/)
{
	if ( m_bStretch || bStretch )
	{
		CSize size( 32000, m_nHeight );

		if ( CWnd* pParent = AfxGetMainWnd() )
		{
			CRect rc;
			pParent->GetWindowRect( &rc );
			if ( rc.Width() > 32 )
				size.cx = rc.Width() + 2;
		}

		m_czLast = size;

		return size;
	}
	else
	{
		CSize size( MARGIN_WIDTH * 2 + 5, m_nHeight );
		if ( m_bGripper ) size.cx += GRIPPER_WIDTH;

		for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			CCoolBarItem* pItem = m_pItems.GetNext( pos );
			if ( pItem->m_bVisible )
				size.cx += pItem->m_nWidth;
		}

		m_czLast = size;

		return size;
	}
}

void CCoolBarCtrl::PrepareRect(CRect* pRect) const
{
	CRect rcClient;
	GetClientRect( &rcClient );
	CalcInsideRect( rcClient, FALSE );

	rcClient.left	-= m_cyTopBorder;
	rcClient.top	-= m_cxLeftBorder;
	rcClient.right	+= m_cyBottomBorder;
	rcClient.bottom	+= m_cxRightBorder;

	pRect->SetRect( rcClient.left + MARGIN_WIDTH, rcClient.top + 1, rcClient.right - MARGIN_WIDTH, rcClient.bottom - 1 );
	if ( m_bGripper ) pRect->left += GRIPPER_WIDTH;
}

CCoolBarItem* CCoolBarCtrl::HitTest(const CPoint& point, CRect* pItemRect, BOOL bSeparators) const
{
	if ( m_pItems.IsEmpty() ) return NULL;

	BOOL bRight = FALSE;
	CRect rcClient, rcItem;

	PrepareRect( &rcClient );
	rcItem.CopyRect( &rcClient );

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = m_pItems.GetNext( pos );
		if ( ! pItem->m_bVisible ) continue;

		if ( pItem->m_nID == ID_RIGHTALIGN && ! bRight )
		{
			int nRight = 0;
			bRight = TRUE;

			for ( POSITION pos2 = pos ; pos2 ; )
			{
				CCoolBarItem* pRight = m_pItems.GetNext( pos2 );
				if ( pRight->m_bVisible )
					nRight += pRight->m_nWidth;
			}

			if ( rcClient.right - rcItem.left >= nRight )
				rcItem.left = rcClient.right - nRight;
		}
		else
		{
			rcItem.right = rcItem.left + pItem->m_nWidth;

			if ( rcItem.PtInRect( point ) )
			{
				if ( pItemRect ) *pItemRect = rcItem;
				return ( pItem->m_nID != ID_SEPARATOR || bSeparators ) ? pItem : NULL;
			}

			rcItem.OffsetRect( rcItem.Width(), 0 );
		}
	}

	return NULL;
}

BOOL CCoolBarCtrl::GetItemRect(CCoolBarItem* pFind, CRect* pRect) const
{
	if ( m_pItems.IsEmpty() ) return FALSE;

	BOOL bRight = FALSE;
	CRect rcClient, rcItem;

	PrepareRect( &rcClient );
	rcItem.CopyRect( &rcClient );

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = m_pItems.GetNext( pos );
		if ( ! pItem->m_bVisible ) continue;

		if ( pItem->m_nID == ID_RIGHTALIGN && ! bRight )
		{
			int nRight = 0;
			bRight = TRUE;

			for ( POSITION pos2 = pos ; pos2 ; )
			{
				CCoolBarItem* pRight = m_pItems.GetNext( pos2 );
				if ( pRight->m_bVisible )
					nRight += pRight->m_nWidth;
			}

			if ( rcClient.right - rcItem.left >= nRight )
				rcItem.left = rcClient.right - nRight;
		}
		else
		{
			rcItem.right = rcItem.left + pItem->m_nWidth;

			if ( pItem == pFind )
			{
				*pRect = rcItem;
				return TRUE;
			}

			rcItem.OffsetRect( rcItem.Width(), 0 );
		}
	}

	return FALSE;
}

INT_PTR CCoolBarCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rcItem;
	CCoolBarItem* pItem = HitTest( point, &rcItem );

	if ( pItem == NULL ) return -1;
	if ( ! pTI ) return 1;

	pTI->uFlags		= 0;
	pTI->hwnd		= GetSafeHwnd();
	pTI->uId		= (UINT)pItem->m_nID;
	pTI->rect		= rcItem;
	pTI->lpszText	= LPSTR_TEXTCALLBACK;

	if ( ! pItem->m_sTip.IsEmpty() )
	{
		pTI->lpszText = _tcsdup( pItem->m_sTip );
	}
	else
	{
		CString strTip;
		if ( LoadString( strTip, static_cast< UINT >( pTI->uId ) ) )
		{
			if ( LPCTSTR pszBreak = _tcschr( strTip, '\n' ) )
			{
				pTI->lpszText = _tcsdup( pszBreak + 1 );
			}
			else
			{
				BOOL bRTL = FALSE;
				if ( Settings.General.LanguageRTL )
				{
					Skin.GetTextFlowChange( strTip, &bRTL );
					if ( bRTL ) strTip.MakeReverse();
				}
				strTip = strTip.SpanExcluding( L".\x2026" );
				if ( bRTL ) strTip.MakeReverse();
				pTI->lpszText = _tcsdup( strTip );
			}
		}
	}

	return pTI->uId;
}

void CCoolBarCtrl::DoPaint(CDC* pDC)
{
	ASSERT_VALID( this );
	ASSERT_VALID( pDC );

	CRect rc;
	GetClientRect( &rc );

	if ( m_bBuffered || m_bmImage.m_hObject != NULL )
	{
		CSize size = rc.Size();
		CDC* pBuffer = CoolInterface.GetBuffer( *pDC, size );

		if ( CoolInterface.DrawWatermark( pBuffer, &rc, &m_bmImage ) )
		{
			CalcInsideRect( rc, FALSE );
			rc.left -= m_cyTopBorder;
			rc.top -= m_cxLeftBorder;
			rc.right += m_cyBottomBorder;
			rc.bottom += m_cxRightBorder;

			if ( rc.top < 2 )	// Library top bars workaround fix
			{
				rc.top += 2;
				rc.bottom += 2;
			}

			DoPaint( pBuffer, rc, TRUE );

			GetClientRect( &rc );
			pDC->BitBlt( 0, 0, rc.Width(), rc.Height(), pBuffer, 0, 0, SRCCOPY );
			pBuffer->SelectClipRgn( NULL );

			return;
		}
	}

	// Unskinned
	if ( Settings.Skin.MenuBorders )
		DrawBorders( pDC, rc );
	pDC->FillSolidRect( &rc, Colors.m_crMidtone );
	DoPaint( pDC, rc, FALSE );
}

void CCoolBarCtrl::DoPaint(CDC* pDC, CRect& rcClient, BOOL bTransparent)
{
	CRect rcItem( rcClient.left + MARGIN_WIDTH, rcClient.top + 1, rcClient.right - MARGIN_WIDTH, rcClient.bottom - 1 );

	if ( m_bGripper && Settings.Skin.MenuGripper )
	{
		for ( int nY = rcClient.top + 4 ; nY < rcClient.bottom - 4 ; nY += 2 )
		{
			pDC->Draw3dRect( rcClient.left + 3, nY, GRIPPER_WIDTH, bTransparent ? 1 : 2,
				Colors.m_crDisabled, bTransparent ? Colors.m_crDisabled : Colors.m_crMidtone );
		}

		if ( ! bTransparent )
			pDC->ExcludeClipRect( rcClient.left + 3, rcClient.top + 4, rcClient.left + GRIPPER_WIDTH + 2, rcClient.bottom - 4 );

		rcItem.left += GRIPPER_WIDTH;
	}

	if ( m_pItems.IsEmpty() ) return;

	CFont* pOldFont = (CFont*)pDC->SelectObject( m_bBold ? &CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );
	BOOL bRight = FALSE;
	CRect rcCopy;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = m_pItems.GetNext( pos );

		if ( pItem->m_nID == ID_RIGHTALIGN && ! bRight )
		{
			int nRight = 0;
			bRight = TRUE;

			for ( POSITION pos2 = pos ; pos2 ; )
			{
				CCoolBarItem* pRight = m_pItems.GetNext( pos2 );
				if ( pRight->m_bVisible ) nRight += pRight->m_nWidth;
			}

			if ( rcClient.right - rcItem.left >= nRight + MARGIN_WIDTH )
				rcItem.left = rcClient.right - nRight - MARGIN_WIDTH;
		}
		else if ( pItem->m_bVisible )
		{
			rcItem.right = rcItem.left + pItem->m_nWidth;
			rcCopy.CopyRect( &rcItem );

			CWnd* pCtrl = ( pItem->m_nCtrlID ) ? GetDlgItem( pItem->m_nCtrlID ) : NULL;

			pItem->Paint( pDC, rcCopy, m_pDown == pItem,
				m_pHot == pItem || ( pCtrl && pCtrl == GetFocus() ),
				m_bMenuGray, bTransparent );

			if ( ! bTransparent ) pDC->ExcludeClipRect( &rcItem );
			if ( pCtrl ) SmartMove( pCtrl, &rcCopy );

			rcItem.OffsetRect( rcItem.Width(), 0 );
		}
		else if ( pItem->m_nCtrlID )
		{
			CWnd* pCtrl = GetDlgItem( pItem->m_nCtrlID );
			if ( pCtrl && pCtrl->IsWindowVisible() )
				pCtrl->ShowWindow( SW_HIDE );
		}
	}

	pDC->SelectObject( pOldFont );
}

void CCoolBarCtrl::SmartMove(CWnd* pCtrl, CRect* pRect)
{
	CRect rc;
	pCtrl->GetWindowRect( &rc );
	ScreenToClient( &rc );

	if ( rc != *pRect )
		pCtrl->SetWindowPos( NULL, pRect->left, pRect->top, pRect->Width(), pRect->Height(), SWP_NOZORDER|SWP_SHOWWINDOW );
	else if ( ! pCtrl->IsWindowVisible() )
		pCtrl->ShowWindow( SW_SHOW );
}

HBRUSH CCoolBarCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CControlBar::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( nCtlColor == CTLCOLOR_STATIC )
	{
		// Skin media sliders
		if ( pWnd->GetDlgCtrlID() == IDC_MEDIA_POSITION || pWnd->GetDlgCtrlID() == IDC_MEDIA_VOLUME || pWnd->GetDlgCtrlID() == IDC_MEDIA_SPEED )
			return (HBRUSH)Images.m_brMediaControl;		// m_brMediaSlider

		// ToDo: Is this Toolbar brush member needed?
		pDC->SetBkColor( Colors.m_crMidtone );
		if ( m_crBack != Colors.m_crMidtone )
		{
			if ( m_brBack.m_hObject ) m_brBack.DeleteObject();
			m_brBack.CreateSolidBrush( m_crBack = Colors.m_crMidtone );
		}
		hbr = m_brBack;
	}

	return hbr;
}

void CCoolBarCtrl::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
//	static DWORD tLastUpdate = 0;	// No static
	const DWORD tNow = GetTickCount();
	if ( tNow < m_tLastUpdate + Settings.Interface.RefreshRateUI ) return;		// Only refresh occasionally (~3/5x second?)
	m_tLastUpdate = tNow;

	UINT nIndex		= 0;
	BOOL bChanged	= FALSE;
	BOOL bLocked	= FALSE;

	if ( m_pSyncObject )
	{
		bLocked = m_pSyncObject->Lock( 200 );
		if ( ! bLocked ) return;
	}

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = m_pItems.GetNext( pos );

		if ( pItem->m_nID == ID_SEPARATOR ) continue;
		if ( pItem->m_nCtrlID ) continue;

		pItem->m_pOther		= this;
		pItem->m_nIndex		= nIndex++;
		pItem->m_nIndexMax	= static_cast< UINT >( m_pItems.GetCount() );
		pItem->m_bChanged	= FALSE;
		BOOL bEnabled		= pItem->m_bEnabled;

		if ( ! CWnd::OnCmdMsg( pItem->m_nID, CN_UPDATE_COMMAND_UI, pItem, NULL ) )
			pItem->DoUpdate( pTarget, bDisableIfNoHndler );

		pItem->m_bChanged |= ( pItem->m_bEnabled != bEnabled );
		bChanged |= pItem->m_bChanged;
	}

	if ( bLocked )
		m_pSyncObject->Unlock();

	if ( bChanged )
		OnUpdated();
}

void CCoolBarCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CCoolBarItem* pItem = HitTest( point );

	if ( m_pDown && m_pDown != pItem )
		pItem = NULL;

	if ( pItem != m_pHot )
	{
		m_tHoverTime = pItem ? GetTickCount() : 0;
		m_pHot = pItem;
		Invalidate();
	}

	if ( ! m_bTimer )
	{
		SetTimer( 1, 100, NULL );
		m_bTimer = TRUE;
	}

	CControlBar::OnMouseMove( nFlags, point );
}

void CCoolBarCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if ( m_bRecalc )
	{
		CMDIFrameWnd* pOwner = (CMDIFrameWnd*)GetOwner();

		if ( pOwner != NULL && pOwner->IsKindOf( RUNTIME_CLASS(CMDIFrameWnd) ) && ! pOwner->IsIconic() )
		{
			pOwner->RecalcLayout();
			m_bRecalc = FALSE;
		}
	}

	if ( nIDEvent == 1 && ( ! m_bMenuGray || m_pDown == NULL ) )
	{
		CPoint point;
		CRect rcWindow;

		GetCursorPos( &point );
		ScreenToClient( &point );
		GetClientRect( &rcWindow );

		CWnd* pParent;
		if ( rcWindow.PtInRect( point ) && ( pParent = GetTopLevelParent() ) != NULL && pParent->IsWindowEnabled() )
		{
			CCoolBarItem* pItem = HitTest( point );

			if ( m_pDown && m_pDown != pItem )
				pItem = NULL;

			if ( pItem != m_pHot )
			{
				m_pHot = pItem;
				Invalidate();
			}
		}
		else
		{
			KillTimer( nIDEvent );
			m_bTimer = FALSE;

			if ( m_pHot )
			{
				m_pHot = NULL;
				Invalidate();
			}
		}
	}

	CControlBar::OnTimer( nIDEvent );
}

void CCoolBarCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd* pFocus = GetFocus();

	if ( pFocus && pFocus->GetParent() == this )
		SetFocus();

	CCoolBarItem* pItem = HitTest( point );

	if ( pItem && pItem->m_bEnabled )
	{
		m_pDown = m_pHot = pItem;
		SetCapture();
		Invalidate();
		return;
	}

	if ( m_bDragForward )
		GetParent()->SendMessage( WM_LBUTTONDOWN, nFlags, MAKELONG( point.x, point.y ) );
	else if ( ! m_pDockContext || m_pDockContext->m_pDC == NULL )	// Fix rare drag-crash
		CControlBar::OnLButtonDown( nFlags, point );
}

void CCoolBarCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CCoolBarItem* pItem = HitTest( point );

	if ( m_pDown != NULL )
	{
		BOOL bOn = ( m_pDown == pItem );

		m_pDown = NULL;
		m_pHot  = pItem;

		ReleaseCapture();
		Invalidate();

		if ( bOn )
			GetOwner()->PostMessage( WM_COMMAND, pItem->m_nID );

		return;
	}

	if ( m_bDragForward )
		GetParent()->SendMessage( WM_LBUTTONUP, nFlags, MAKELONG( point.x, point.y ) );
	else
		CControlBar::OnRButtonUp( nFlags, point );
}

void CCoolBarCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CCoolBarCtrl::OnLButtonDown( nFlags, point );
}

void CCoolBarCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ( m_pDown != NULL || m_pHot != NULL )
	{
		m_pDown = m_pHot = NULL;

		ReleaseCapture();
		Invalidate();
	}

	CControlBar::OnRButtonDown( nFlags, point );
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBar message forwarding

BOOL CCoolBarCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if ( HIWORD( wParam ) == EN_SETFOCUS ||
		 HIWORD( wParam ) == EN_KILLFOCUS ) Invalidate();

	if ( GetParent()->SendMessage( WM_COMMAND, wParam, lParam ) )
		return TRUE;

	return CControlBar::OnCommand( wParam, lParam );
}

BOOL CCoolBarCtrl::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_RETURN )
			GetOwner()->PostMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), BN_CLICKED ), (LPARAM)GetSafeHwnd() );
		else if ( pMsg->wParam == VK_ESCAPE )
			GetOwner()->PostMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), BN_DBLCLK ), (LPARAM)GetSafeHwnd() );
	}

	return CControlBar::PreTranslateMessage( pMsg );
}

void CCoolBarCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetParent()->SendMessage( WM_HSCROLL, MAKELONG( nSBCode, nPos ), (LPARAM)pScrollBar->GetSafeHwnd() );
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBarCtrl IDropTarget implementation

IMPLEMENT_DROP(CCoolBarCtrl, CControlBar)

BOOL CCoolBarCtrl::OnDrop(IDataObject* pDataObj, DWORD /*grfKeyState*/, POINT ptScreen, DWORD* /*pdwEffect*/, BOOL /*bDrop*/)
{
	ASSERT( m_bDropEnabled == TRUE );

	// Mouse move imitation during dragging
	CPoint pt( ptScreen );
	ScreenToClient( &pt );
	OnMouseMove( 0, pt );

	if ( pDataObj )
	{
		// DragEnter or DragOver
		if ( m_pHot && m_tHoverTime )
		{
			if ( GetTickCount() - m_tHoverTime >= DRAG_HOVER_TIME )
			{
				m_tHoverTime = 0;
				switch ( m_pHot->m_nID )
				{
				// Ignore Some Buttons
				case ID_TAB_CONNECT:
				case ID_NETWORK_CONNECT:
				case ID_NETWORK_DISCONNECT:
				case ID_TAB_SEARCH:
					break;
				default:
					GetOwner()->PostMessage( WM_COMMAND, m_pHot->m_nID );
				}
			}
		}
	}
	else
	{
		// DragLeave
		m_tHoverTime = 0;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CCoolBarItem construction

CCoolBarItem::CCoolBarItem(CCoolBarCtrl* pBar, UINT nID, int nImage)
	: m_pBar		( pBar )
	, m_bCheckButton( FALSE )
	, m_nImage		( nImage )
	, m_bVisible	( TRUE )
	, m_bEnabled	( TRUE )
	, m_bChecked	( FALSE )
	, m_crText		( (UINT)-1 )
	, m_nCtrlHeight	( CONTROL_HEIGHT )
	, m_nCtrlID 	( 0 )
{
	m_nID = nID;

	switch ( nID )
	{
	case ID_SEPARATOR:
		m_nWidth = SEPARATOR_WIDTH;
		break;
	case ID_RIGHTALIGN:
		m_nWidth = 0;
		break;
	default:
		m_nWidth = IMAGEBUTTON_WIDTH;
		break;
	}
}

CCoolBarItem::CCoolBarItem(CCoolBarCtrl* pBar, CCoolBarItem* pCopy)
{
	m_pBar		= pBar;
	m_nID		= pCopy->m_nID;
	//m_nImage	= pCopy->m_nImage;
	m_sTip		= pCopy->m_sTip;
	m_bVisible	= pCopy->m_bVisible;
	m_bEnabled	= pCopy->m_bEnabled;
	m_bChecked	= pCopy->m_bChecked;
	m_crText	= pCopy->m_crText;
	m_nWidth	= pCopy->m_nWidth;

	m_nCtrlID		= pCopy->m_nCtrlID;
	m_nCtrlHeight	= pCopy->m_nCtrlHeight;
	m_bCheckButton	= pCopy->m_bCheckButton;

	//if ( m_nImage < 0 )
		m_nImage = CoolInterface.ImageForID( m_nID );
	SetText( pCopy->m_sText );
}

CCoolBarItem::~CCoolBarItem()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBarItem CCmdUI handlers

void CCoolBarItem::Show(BOOL bOn)
{
	if ( this == NULL ) return;
	if ( m_bVisible == bOn ) return;
	m_bVisible	= bOn;
	m_bChanged	= TRUE;
}

void CCoolBarItem::Enable(BOOL bOn)
{
	if ( this == NULL ) return;
	m_bEnableChanged = TRUE;
	m_bEnabled	= bOn;
}

void CCoolBarItem::SetCheck(int nCheck)
{
	if ( this == NULL ) return;
	if ( m_bChecked == ( nCheck == 1 ) ) return;
	m_bChecked	= ( nCheck == 1 );
	m_bChanged	= TRUE;
}

void CCoolBarItem::SetText(LPCTSTR lpszText)
{
	if ( this == NULL ) return;
	if ( m_sText == lpszText ) return;

	m_sText		= lpszText;
	m_bChanged	= TRUE;

	CDC dc;
	dc.Attach( ::GetDC( 0 ) );

	CFont* pOld = (CFont*)dc.SelectObject( m_pBar->m_bBold ? &CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );
	m_nWidth = BUTTON_WIDTH + dc.GetTextExtent( m_sText ).cx;
	if ( m_nImage >= 0 ) m_nWidth += IMAGE_WIDTH + TEXT_GAP;
	dc.SelectObject( pOld );

	::ReleaseDC( 0, dc.Detach() );
}

void CCoolBarItem::SetTip(LPCTSTR pszTip)
{
	if ( this == NULL ) return;
	if ( m_sTip == pszTip ) return;
	m_sTip		= pszTip;
	m_bChanged	= TRUE;
}

void CCoolBarItem::SetTextColor(COLORREF crText)
{
	if ( this == NULL ) return;
	if ( m_crText == crText ) return;
	m_crText	= crText;
	m_bChanged	= TRUE;
}

void CCoolBarItem::SetImage(UINT nCommandID)
{
	if ( this == NULL ) return;
	int nImage = CoolInterface.ImageForID( nCommandID );
	if ( nImage == m_nImage ) return;
	m_nImage	= nImage;
	m_bChanged	= TRUE;
}

CCoolBarItem* CCoolBarItem::FromCmdUI(CCmdUI* pUI)
{
	if ( pUI->m_pOther == NULL ) return NULL;
	if ( pUI->m_pOther->IsKindOf( RUNTIME_CLASS(CCoolBarCtrl) ) == FALSE ) return NULL;
	return (CCoolBarItem*)pUI;
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBarItem painting

void CCoolBarItem::Paint(CDC* pDC, CRect& rc, BOOL bDown, BOOL bHot, BOOL bMenuGray, BOOL bTransparent)
{
	COLORREF crBackground;
	BOOL bSkinned = FALSE;

	if ( m_nID == ID_SEPARATOR )
	{
		rc.InflateRect( 0, 2 );
		if ( Images.DrawButtonState( pDC, &rc, TOOLBAR_SEPARATOR ) )
			return;
		rc.DeflateRect( 0, 2 );

		if ( ! bTransparent )
			pDC->FillSolidRect( rc.left, rc.top, 3, rc.Height(), Colors.m_crMidtone );
		pDC->Draw3dRect( rc.left + 3, rc.top, 1, rc.Height(), Colors.m_crDisabled, Colors.m_crDisabled );
		if ( ! bTransparent )
			pDC->FillSolidRect( rc.left + 4, rc.top, 3, rc.Height(), Colors.m_crMidtone );

		return;
	}

	if ( m_nCtrlID )
	{
		for ( int nShrink = rc.Height() - m_nCtrlHeight ; nShrink > 0 ; nShrink -= 2 )
		{
			if ( ! bTransparent )
				pDC->Draw3dRect( &rc, Colors.m_crMidtone, Colors.m_crMidtone );
			rc.DeflateRect( 0, 1 );
		}
		rc.DeflateRect( 1, 0 );

		if ( m_bCheckButton )
		{
			DrawText( pDC, rc, bDown, bHot, bMenuGray, bTransparent );
			return;
		}
	}
	else	// Command Button
	{
		rc.InflateRect( 0, 2 );

		if ( ! m_pBar->m_bGripper )		// Detect Standard Toolbar Button
		{
			if ( ! m_bEnabled )
				bSkinned = Images.DrawButtonState( pDC, &rc, TOOLBARBUTTON_DISABLED );	// "CCoolbar.Disabled"
			else if ( bDown )
				bSkinned = Images.DrawButtonState( pDC, &rc, TOOLBARBUTTON_PRESS );		// "CCoolbar.Down"
			else if ( bHot )
				bSkinned = Images.DrawButtonState( pDC, &rc, TOOLBARBUTTON_HOVER );		// "CCoolbar.Hover"
			else if ( m_bChecked )
				bSkinned = Images.DrawButtonState( pDC, &rc, TOOLBARBUTTON_ACTIVE );	// "CCoolbar.Checked"
			else
				bSkinned = Images.DrawButtonState( pDC, &rc, TOOLBARBUTTON_DEFAULT );	// "CCoolbar.Up"
		}
		else if ( m_nImage > 0 ) 		// Windowed Mode Menu Icon
		{
			if ( bDown )
				bSkinned = Images.DrawButtonState( pDC, &rc, MENUBARBUTTON_PRESS );		// "CCoolMenuBar.Down"
			else if ( bHot )
				bSkinned = Images.DrawButtonState( pDC, &rc, MENUBARBUTTON_HOVER );		// "CCoolMenuBar.Hover"
			else if ( m_bChecked )
				bSkinned = Images.DrawButtonState( pDC, &rc, MENUBARBUTTON_ACTIVE );	// "CCoolMenuBar.Checked"
			else
				bSkinned = Images.DrawButtonState( pDC, &rc, MENUBARBUTTON_DEFAULT );	// "CCoolMenuBar.Up"
		}
		else	// Menu Text
		{
			if ( bDown )
				bSkinned = Images.DrawButtonState( pDC, &rc, MENUBARITEM_PRESS );		// "CCoolMenuItem.Down"
			else if ( bHot )
				bSkinned = Images.DrawButtonState( pDC, &rc, MENUBARITEM_HOVER );		// "CCoolMenuItem.Hover"
			else
				bSkinned = Images.DrawButtonState( pDC, &rc, MENUBARITEM_DEFAULT );		// "CCoolMenuItem.Up"
		}

		rc.DeflateRect( 0, 2 );

	//	if ( ! bTransparent && ! bSkinned )
	//		pDC->Draw3dRect( &rc, Colors.m_crMidtone, Colors.m_crMidtone ); 			// Potential hole with transparent button on non-skinned toolbar?

		rc.DeflateRect( 1, 1 );
	}

	if ( bSkinned ) 	// ! m_nCtrlID
	{
		// ToDo: Fix mess in function prototype. Why pass "bTransparent" and then ignore it? (Temp Watermark)
		pDC->SetBkMode( TRANSPARENT );
		crBackground = CLR_NONE;
		rc.OffsetRect( 1, 0 );
	}
	else if ( m_bEnabled && ( bHot || bDown || m_bChecked ) )
	{
		pDC->SetBkMode( OPAQUE );
		if ( bMenuGray && bDown )
			pDC->Draw3dRect( &rc, Colors.m_crDisabled, Colors.m_crDisabled );
		else	// Draw border around the control
			pDC->Draw3dRect( &rc, Colors.m_crBorder, Colors.m_crBorder );

		rc.DeflateRect( 1, 1 );

		if ( bMenuGray && bDown )
			crBackground = Colors.m_crBackNormal;
		else if ( m_bChecked )
			crBackground = bHot ? Colors.m_crBackCheckSel : Colors.m_crBackCheck;
		else
			crBackground = bDown && bHot ? Colors.m_crBackCheckSel : Colors.m_crBackSel;
	}
	else  // Unskinned item
	{
		if ( bTransparent )
		{
			crBackground = CLR_NONE;
		}
		else
		{
			crBackground = Colors.m_crMidtone;
			pDC->Draw3dRect( &rc, crBackground, crBackground );
		}

		rc.DeflateRect( 1, 1 );
	}

	if ( m_nCtrlID )
	{
		if ( m_nCtrlHeight == CONTROL_HEIGHT )
		{
			pDC->Draw3dRect( &rc, Colors.m_crWindow, Colors.m_crWindow );
			rc.DeflateRect( 1, 1 );
		}
		return;
	}

	if ( crBackground == CLR_NONE )
	{
		pDC->SetBkMode( TRANSPARENT );
	}
	else if ( ! bSkinned )
	{
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor( crBackground );
	}

	DrawText( pDC, rc, bDown, bHot, bMenuGray, ( crBackground == CLR_NONE || ( bSkinned && ! m_bCheckButton ) ) );

	if ( m_nImage >= 0 )
	{
		CPoint ptImage( rc.left + 3, ( rc.top + rc.bottom ) / 2 - 8 );

		if ( ! m_bEnabled )
		{
			CoolInterface.DrawEx( pDC, m_nImage, (POINT)ptImage, (CSize)0, crBackground, Colors.m_crShadow, ILD_BLEND50 );
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		}
		else if ( m_bChecked )
		{
			CoolInterface.DrawEx( pDC, m_nImage, (POINT)ptImage, (CSize)0, crBackground, CLR_NONE, ILD_NORMAL );
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		}
		else if ( ( bHot && ! bDown ) || ( bDown && ! bHot ) )
		{
			//ptImage.Offset( 1, 1 );

			if ( crBackground != CLR_NONE || ! bSkinned )
			{
				pDC->SetTextColor( crBackground );		// Colors.m_crShadow
				CoolInterface.DrawEx( pDC, m_nImage, (POINT)ptImage, (CSize)0, crBackground, CLR_NONE, ILD_MASK );
			}
			else // Skinned
			{
			//	CoolInterface.DrawEx( pDC, m_nImage,
			//		ptImage, (CSize)0, crBackground, Colors.m_crShadow, ILD_NORMAL );

			//	CoolInterface.m_pImages16.DrawIndirect( pDC, m_nImage, (POINT)ptImage, (SIZE)(CSize)( 16, 16 ), (POINT)(CPoint)0,
			//		ILD_BLEND50|ILD_BLEND25|ILD_ROP, MERGECOPY, CLR_NONE, Colors.m_crShadow, ILS_SATURATE|ILS_ALPHA, 160, Colors.m_crShadow );

			//	CoolInterface.DrawIndirect( pDC, m_nImage, (POINT)ptImage, (SIZE)(CSize)0,
			//		CLR_NONE, Colors.m_crShadow, ILD_BLEND50|ILD_BLEND25|ILD_ROP, ILS_SATURATE|ILS_ALPHA, 160 );

				CoolInterface.DrawEx( pDC, m_nImage, (POINT)ptImage, (CSize)0, CLR_NONE, Colors.m_crShadow, ILD_BLEND50|ILD_BLEND25 );
			}

			ptImage.Offset( -1, -1 );

			if ( crBackground != CLR_NONE || ! bSkinned )
			{
				pDC->FillSolidRect( ptImage.x, ptImage.y, 18, 2, crBackground );
				pDC->FillSolidRect( ptImage.x, ptImage.y + 2, 2, 16, crBackground );
				CoolInterface.DrawEx( pDC, m_nImage, (POINT)ptImage, (CSize)0, crBackground, CLR_NONE, ILD_NORMAL );
				pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
			}
			else // Skinned
			{
				CoolInterface.DrawEx( pDC, m_nImage, (POINT)ptImage, (CSize)0, CLR_NONE, CLR_NONE, ILD_NORMAL );
			}
		}
		else
		{
			CoolInterface.DrawEx( pDC, m_nImage, (POINT)ptImage, (CSize)0, crBackground, Colors.m_crBackNormal, bDown ? ILD_BLEND25 : ILD_NORMAL );
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		}
	}

	if ( crBackground != CLR_NONE && ! bSkinned )		// ! bTransparent
		pDC->FillSolidRect( &rc, crBackground );
}

void CCoolBarItem::DrawText(CDC* pDC, CRect& rc, BOOL bDown, BOOL bHot, BOOL bMenuGray, BOOL bTransparent)
{
	if ( m_sText.IsEmpty() )
		return;

	if ( m_crText != 0xFFFFFFFF )
		pDC->SetTextColor( m_crText );
	else if ( ! m_bEnabled )
		pDC->SetTextColor( Colors.m_crDisabled );
	else if ( ( bHot || bDown || m_bChecked ) && ( ! bMenuGray || ! bDown ) )
		pDC->SetTextColor( Colors.m_crCmdTextSel );
	else
		pDC->SetTextColor( Colors.m_crCmdText );

	rc.left += ( m_nImage >= 0 ) ? 20 : 1;
	const int nY = ( rc.top + rc.bottom ) / 2 - pDC->GetTextExtent( m_sText ).cy / 2 - 1;

	pDC->ExtTextOut( rc.left + 2, nY, ETO_CLIPPED|( bTransparent ? 0 : ETO_OPAQUE ), &rc, m_sText, NULL );

	rc.right = rc.left;
	rc.left -= ( m_nImage >= 0 ) ? 20 : 1;
}
