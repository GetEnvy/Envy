//
// DlgLanguage.cpp
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
#include "DlgLanguage.h"

#include "SkinWindow.h"
#include "Colors.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CLanguageDlg, CSkinDialog)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_VSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

#define ITEM_HEIGHT		38
#define ITEM_WIDTH		200
#define ITEM_ROWS		11
#define TEXT_MARGIN		0


/////////////////////////////////////////////////////////////////////////////
// CLanguageDlg dialog

CLanguageDlg::CLanguageDlg(CWnd* pParent)
	: CSkinDialog	(CLanguageDlg::IDD, pParent)
	, m_sLanguage	( L"en" )
	, m_bLanguageRTL( false )
{
}

void CLanguageDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CLanguageDlg message handlers

BOOL CLanguageDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	CWaitCursor pCursor;

	SkinMe( L"CLanguageDlg", ID_TOOLS_LANGUAGE );

	m_hArrow = theApp.LoadStandardCursor( IDC_ARROW );
	m_hHand  = theApp.LoadCursor( IDC_HAND );

	m_fntNormal.CreateFont( -(int)(Settings.Fonts.DefaultSize + 1), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_fntBold.CreateFont( -(int)(Settings.Fonts.DefaultSize + 3), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_fntSmall.CreateFont( -(int)(Settings.Fonts.DefaultSize - 1), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, Settings.Fonts.Quality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_pImages.Create( 32, 32, ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	m_pImages.Create( 32, 32, ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	m_pImages.Create( 32, 32, ILC_COLOR16|ILC_MASK, 1, 1 );

	AddEnglishDefault();	// Always include English as a choice

	Enumerate();			// Add any other available languages

	m_nHover	= 0;
	m_nDown		= 0;
	m_bKeyMode	= FALSE;

	CRect rc( 0, 0, ITEM_WIDTH * 3 /*+ GetSystemMetrics( SM_CXVSCROLL )*/,
		ITEM_HEIGHT * ITEM_ROWS + Skin.m_nBanner );

	SCROLLINFO pScroll = {};
	pScroll.cbSize	= sizeof( pScroll );
	pScroll.fMask	= SIF_RANGE|SIF_PAGE|SIF_DISABLENOSCROLL;
	pScroll.nMin	= 0;
	pScroll.nMax	= ( m_pPaths.GetSize() / 3 ) + ( ( m_pPaths.GetSize() % 3 ) ? 1 : 0 );
	pScroll.nPage	= ITEM_ROWS + 1;
	SetScrollInfo( SB_VERT, &pScroll, TRUE );

	CalcWindowRect( &rc );

	rc.OffsetRect(	GetSystemMetrics( SM_CXSCREEN ) / 2 -  rc.Width() / 2 - rc.left,
					GetSystemMetrics( SM_CYSCREEN ) / 2 - rc.Height() / 2 - rc.top );

	SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height(), 0 );

	SetTimer( 1, 100, NULL );

	return TRUE;
}

void CLanguageDlg::OnDestroy()
{
	KillTimer( 1 );
	CSkinDialog::OnDestroy();
}

BOOL CLanguageDlg::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CLanguageDlg::OnPaint()
{
	CPaintDC dc( this );
	CRect rc;
	CRect rcDlg;
	int nScroll = GetScrollPos( SB_VERT );

	GetClientRect( &rc );
	GetClientRect( &rcDlg );

	// Obsolete:
	//CDC mdc;
	//mdc.CreateCompatibleDC( &dc );
	//m_bmHeader.LoadBitmap( IDB_BANNER );
	//CBitmap* pOldBmp = (CBitmap*)mdc.SelectObject( &m_bmHeader );
	//dc.BitBlt( 0, 0, rc.Width(), GetBannerHeight(), &mdc, 0, 0, SRCCOPY );
	//mdc.SelectObject( pOldBmp );
	//mdc.DeleteDC();

	rc.top += Skin.m_nBanner;
	rc.right = rc.left + ITEM_WIDTH;
	CFont* pOldFont = (CFont*)dc.SelectObject( &m_fntNormal );

	for ( int nCount = 0 ; nCount < m_pPaths.GetSize() ; nCount += 3 )
	{
		if ( nScroll > 0 )
		{
			nScroll --;
		}
		else
		{
			PaintItem( nCount, &dc, &rc );
			rc.OffsetRect( ITEM_WIDTH, 0 );
			if ( nCount + 1 < m_pPaths.GetSize() )
				PaintItem( nCount + 1, &dc, &rc );
			else
				dc.FillSolidRect( &rc, Colors.m_crBackNormal );

			rc.OffsetRect( ITEM_WIDTH, 0 );
			if ( nCount + 2 < m_pPaths.GetSize() )
				PaintItem( nCount + 2, &dc, &rc );
			else
				dc.FillSolidRect( &rc, Colors.m_crBackNormal );

			rc.OffsetRect( -ITEM_WIDTH * 2, 0 );
			rc.OffsetRect( 0, rc.Height() );
		}
	}

	rcDlg.top = rc.top;
	dc.FillSolidRect( &rcDlg, Colors.m_crBackNormal );
	dc.SelectObject( pOldFont );
}

void CLanguageDlg::PaintItem(int nItem, CDC* pDC, CRect* pRect)
{
	if ( Settings.General.LanguageRTL )
	{
		UINT nFlags = pDC->GetTextAlign();
		pDC->SetTextAlign( nFlags | TA_RTLREADING );
	}

	pRect->bottom = pRect->top + ITEM_HEIGHT;

	BOOL bHover = m_nHover == ( nItem + 1 );
	BOOL bDown  = m_nDown  == ( nItem + 1 );

	CRect rc( pRect );

	pDC->Draw3dRect( &rc, Colors.m_crBackNormal, Colors.m_crBackNormal );
	rc.DeflateRect( 1, 1 );

	COLORREF crBack;

	if ( bHover || bDown )
	{
		pDC->Draw3dRect( &rc, Colors.m_crBorder, Colors.m_crBorder );
		pDC->SetBkColor( crBack = ( bDown && bHover ? Colors.m_crBackCheckSel : Colors.m_crBackSel ) );
	}
	else if ( m_pLangCodes.GetAt( nItem ).CompareNoCase( Settings.General.Language ) == 0 )
	{
		pDC->Draw3dRect( &rc, Colors.m_crBorder, Colors.m_crBorder );
		pDC->SetBkColor( crBack = Colors.m_crBackCheck );
	}
	else
	{
		pDC->Draw3dRect( &rc, Colors.m_crBackNormal, Colors.m_crBackNormal );
		pDC->SetBkColor( crBack = Colors.m_crBackNormal );
	}

	rc.DeflateRect( 1, 1 );

	CPoint ptIcon( rc.left + 4, ( rc.top + rc.bottom ) / 2 - 16 );

	if ( bHover != bDown )
	{
		pDC->FillSolidRect( ptIcon.x - 1, ptIcon.y - 1, 34, 2, crBack );
		pDC->FillSolidRect( ptIcon.x - 1, ptIcon.y + 1, 2, 32, crBack );

		ptIcon.Offset( 1, 1 );

		pDC->SetTextColor( Colors.m_crShadow );
		ImageList_DrawEx( m_pImages.GetSafeHandle(), nItem,
			pDC->GetSafeHdc(), ptIcon.x, ptIcon.y, 32, 32, crBack, CLR_NONE, ILD_MASK );

		ptIcon.Offset( -2, -2 );

		ImageList_DrawEx( m_pImages.GetSafeHandle(), nItem,
			pDC->GetSafeHdc(), ptIcon.x, ptIcon.y, 32, 32, CLR_NONE, CLR_NONE, ILD_NORMAL );

		pDC->ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 34, ptIcon.y + 34 );
	}
	else
	{
		ImageList_DrawEx( m_pImages.GetSafeHandle(), nItem,
			pDC->GetSafeHdc(), ptIcon.x, ptIcon.y, 32, 32, crBack, CLR_NONE, ILD_NORMAL );

		pDC->ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 32, ptIcon.y + 32 );
	}

	CRect rcText( rc.left + 46, rc.top + TEXT_MARGIN,
				  rc.right - TEXT_MARGIN, rc.top + 20 + TEXT_MARGIN );

	rcText.OffsetRect( 0, 6 );
	pDC->SetTextColor( bHover || bDown ? Colors.m_crCmdTextSel : Colors.m_crCmdText );
	pDC->SelectObject( &m_fntBold );
	pDC->ExtTextOut( rcText.left + 1, rcText.top + 1, ETO_CLIPPED|ETO_OPAQUE, &rcText,
		m_pTitles.GetAt( nItem ), NULL );
	pDC->ExcludeClipRect( &rcText );
	pDC->FillSolidRect( &rc, crBack );
}

void CLanguageDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	SCROLLINFO pInfo = {};

	pInfo.cbSize = sizeof( pInfo );
	pInfo.fMask  = SIF_ALL & ~SIF_TRACKPOS;

	GetScrollInfo( SB_VERT, &pInfo );
	int nDelta = pInfo.nPos;

	switch ( nSBCode )
	{
	case SB_BOTTOM:
		pInfo.nPos = pInfo.nMax - pInfo.nPage;
		break;
	case SB_LINEDOWN:
		pInfo.nPos ++;
		break;
	case SB_LINEUP:
		pInfo.nPos --;
		break;
	case SB_PAGEDOWN:
		pInfo.nPos += pInfo.nPage;
		break;
	case SB_PAGEUP:
		pInfo.nPos -= pInfo.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pInfo.nPos = nPos;
		break;
	case SB_TOP:
		pInfo.nPos = 0;
		break;
	}

	pInfo.nPos = max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	if ( pInfo.nPos == nDelta ) return;

	SetScrollInfo( SB_VERT, &pInfo, TRUE );

	m_nHover = 0;
	m_nDown  = 0;

	Invalidate();
}

BOOL CLanguageDlg::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	OnVScroll( SB_THUMBPOSITION, GetScrollPos( SB_VERT ) - zDelta / WHEEL_DELTA );
	return TRUE;
}

void CLanguageDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rc;
	GetClientRect( &rc );
	rc.top += Skin.m_nBanner;

	if ( rc.PtInRect( point ) )
	{
		int nScroll = GetScrollPos( SB_VERT );
		int nHover = ( ( point.y - rc.top ) / ITEM_HEIGHT + 1 + nScroll ) * 3 - 2 + ( point.x - rc.left ) / ITEM_WIDTH;

		if ( nHover != m_nHover )
		{
			m_nHover = nHover;
			Invalidate();
		}
	}
	else if ( m_nHover )
	{
		m_nHover = 0;
		Invalidate();
	}

	m_bKeyMode = FALSE;

	CSkinDialog::OnMouseMove( nFlags, point );
}

BOOL CLanguageDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( nHitTest == HTCLIENT )
	{
		CPoint pt;
		GetCursorPos( &pt );
		ScreenToClient( &pt );

		SetCursor( pt.y > Skin.m_nBanner && ( m_nHover > 0 && m_nHover - 1 < m_pPaths.GetSize() ) ? m_hHand : m_hArrow );
		return TRUE;
	}

	return CSkinDialog::OnSetCursor( pWnd, nHitTest, message );
}

void CLanguageDlg::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	m_nDown = m_nHover;
	SetCapture();
	Invalidate();
}

void CLanguageDlg::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	int nSelected = ( m_nDown && m_nDown == m_nHover ) ? m_nDown : 0;

	m_nDown = m_nHover = 0;

	ReleaseCapture();
	Invalidate();
	UpdateWindow();

	if ( nSelected ) Execute( nSelected );
}

void CLanguageDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	if ( ! m_nHover || m_bKeyMode ) return;

	CPoint pt;
	GetCursorPos( &pt );
	ScreenToClient( &pt );

	CRect rc;
	GetClientRect( &rc );

	if ( ! rc.PtInRect( pt ) )
	{
		m_nHover = 0;
		Invalidate();
	}
}

BOOL CLanguageDlg::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		OnKeyDown( static_cast< UINT >( pMsg->wParam ), LOWORD( pMsg->lParam ), HIWORD( pMsg->lParam ) );
		return TRUE;
	}

	return CSkinDialog::PreTranslateMessage( pMsg );
}

void CLanguageDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch ( nChar )
	{
	case VK_ESCAPE:
		PostMessage( WM_CLOSE );
		return;
	case VK_LEFT:
	case VK_UP:
		if ( ! m_nDown )
		{
			m_nHover--;
			m_bKeyMode = TRUE;
			SCROLLINFO pInfo = {};
			pInfo.cbSize = sizeof( pInfo );
			pInfo.fMask  = SIF_ALL & ~SIF_TRACKPOS;

			GetScrollInfo( SB_VERT, &pInfo );
			if ( m_nHover <= 0 )
			{
				m_nHover = static_cast< int >( m_pPaths.GetSize() );
				pInfo.nPos = pInfo.nMax;
				SetScrollInfo( SB_VERT, &pInfo, TRUE );
			}
			else if ( m_nHover % 3 == 0 && m_nHover / 3 <= pInfo.nPos )
			{
				pInfo.nPos--;
				SetScrollInfo( SB_VERT, &pInfo, TRUE );
			}

			Invalidate();
		}
		return;
	case VK_TAB:
	case VK_RIGHT:
	case VK_DOWN:
		if ( ! m_nDown )
		{
			m_nHover++;
			m_bKeyMode = TRUE;
			SCROLLINFO pInfo = {};
			pInfo.cbSize = sizeof( pInfo );
			pInfo.fMask  = SIF_ALL & ~SIF_TRACKPOS;
			GetScrollInfo( SB_VERT, &pInfo );

			if ( m_nHover > m_pPaths.GetSize() )
			{
				m_nHover = 1;
				pInfo.nPos = 0;
				SetScrollInfo( SB_VERT, &pInfo, TRUE );
			}
			else if ( ( m_nHover - 1 ) / 3 > pInfo.nPos + ITEM_ROWS - 1 )
			{
				pInfo.nPos++;
				SetScrollInfo( SB_VERT, &pInfo, TRUE );
			}

			Invalidate();
		}
		return;
	case VK_RETURN:
		if ( m_nHover && ! m_nDown )
			Execute( m_nHover );
		return;
	}

	CSkinDialog::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CLanguageDlg::AddEnglishDefault()
{
	m_pPaths.Add( L"" );
	m_pTitles.Add( L"English (Default)" );
	m_pLangCodes.Add( L"en" );
	m_pGUIDirs.Add( L"ltr" );
	m_pPriorities.Add( 0 );
	m_pImages.Add( theApp.LoadIcon( IDI_FLAG_ENGLISH ) );
}

// Recursively scans the Skins directory looking for language files.
void CLanguageDlg::Enumerate(LPCTSTR pszPath)
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;

	strPath.Format( L"%s\\Skins\\%s*.*",
		(LPCTSTR)Settings.General.Path, pszPath ? pszPath : L"" );

	hSearch = FindFirstFile( strPath, &pFind );
	if ( hSearch == INVALID_HANDLE_VALUE )
		return;

	do
	{
		if ( pFind.cFileName[0] == '.' ) continue;

		if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			strPath.Format( L"%s%s\\",
				pszPath ? pszPath : L"", pFind.cFileName );

			Enumerate( strPath );
		}
		else if (	_tcsistr( pFind.cFileName, L".xml" ) != NULL &&
					_tcsicmp( pFind.cFileName, L"Definitions.xml" ) &&
					_tcsicmp( pFind.cFileName, L"Default-en.xml" ) )
		{
			AddSkin( pszPath, pFind.cFileName );
		}
	}
	while ( FindNextFile( hSearch, &pFind ) );

	FindClose( hSearch );
}

BOOL CLanguageDlg::AddSkin(LPCTSTR pszPath, LPCTSTR pszName)
{
	CString strRoot = Settings.General.Path + L"\\Skins\\";
	if ( pszPath != NULL ) strRoot += pszPath;
	CString strXML = strRoot + pszName;

	CFile pFile;
	if ( ! pFile.Open( strXML, CFile::modeRead ) ) return FALSE;

	DWORD nSource = (DWORD)pFile.GetLength();
	if ( nSource > 4096*1024 ) return FALSE;

	CHAR* pSource = new CHAR[ nSource ];
	pFile.Read( pSource, nSource );
	pFile.Close();

	BYTE* pByte = (BYTE*)pSource;
	DWORD nByte = nSource;

	if ( nByte >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
	{
		nByte = nByte / 2 - 1;

		if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
		{
			pByte += 2;

			for ( DWORD nSwap = 0 ; nSwap < nByte ; nSwap ++ )
			{
				register CHAR nTemp = pByte[ ( nSwap << 1 ) + 0 ];
				pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
				pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
			}
		}
		else
		{
			pByte += 2;
		}

		CopyMemory( strXML.GetBuffer( nByte ), pByte, nByte * sizeof( TCHAR ) );
		strXML.ReleaseBuffer( nByte );
	}
	else
	{
		if ( nByte >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF )
		{
			pByte += 3;
			nByte -= 3;
		}

		strXML = UTF8Decode( (LPCSTR)pByte, nByte );
	}

	delete [] pSource;

	CXMLElement* pXML = NULL;

	int nManifest = strXML.Find( L"<manifest" );

	if ( nManifest > 0 )
	{
		CString strManifest = strXML.Mid( nManifest ).SpanExcluding( L">" ) + '>';

		if ( CXMLElement* pManifest = CXMLElement::FromString( strManifest ) )
		{
			pXML = new CXMLElement( NULL, L"skin" );
			pXML->AddElement( pManifest );
		}
	}

	if ( pXML == NULL )
	{
		pXML = CXMLElement::FromString( strXML, TRUE );
		if ( pXML == NULL ) return FALSE;
	}

	strXML.Empty();

	CXMLElement* pManifest = pXML->GetElementByName( L"manifest" );

	if ( ! pXML->IsNamed( L"skin" ) || pManifest == NULL ||
		 pManifest->GetAttributeValue( L"type" ).CompareNoCase( L"language" ) != 0 )
	{
		delete pXML;
		return FALSE;
	}

	CString strName		= pManifest->GetAttributeValue( L"name", pszName );
	CString strIcon		= pManifest->GetAttributeValue( L"icon" );
	CString strLangCode = pManifest->GetAttributeValue( L"language" );
	CString strGUIDir	= pManifest->GetAttributeValue( L"dir", L"ltr" );
	CString strPriority	= pManifest->GetAttributeValue( L"priority" );
//	CString strPrompt	= pManifest->GetAttributeValue( L"prompt" );		// Tip unused

	delete pXML;

	if ( pszPath ) strXML += pszPath;
	strXML += pszName;

	if ( ! strIcon.IsEmpty() )
	{
		strIcon = strRoot + strIcon;
	}
	else
	{
		strIcon = strRoot + pszName;
		strIcon = strIcon.Left( strIcon.GetLength() - 3 ) + L"ico";
	}

	HICON hIcon;
	if ( ExtractIconEx( strIcon, 0, &hIcon, NULL, 1 ) == NULL || ! hIcon )
		hIcon = theApp.LoadIcon( IDR_MAINFRAME );

	UINT nPriority = 100;
	if ( ! strPriority.IsEmpty() && _stscanf( strPriority, L"%u", &nPriority ) == 1 && nPriority )
	{
		for ( int nIndex = 1 ; nIndex < m_pPaths.GetSize() ; nIndex++ )
		{
			if ( m_pPriorities.GetAt( nIndex ) <= nPriority )
				continue;

			m_pPaths.InsertAt( nIndex, strXML );
			m_pTitles.InsertAt( nIndex, strName );
			m_pGUIDirs.InsertAt( nIndex, strGUIDir );
			m_pLangCodes.InsertAt( nIndex, strLangCode );
			m_pPriorities.InsertAt( nIndex, nPriority );

			m_pImages.Add( hIcon );
			for ( int nCount = m_pImages.GetImageCount() - 1 ; nCount > nIndex ; nCount-- )
			{
				m_pImages.Copy( nCount - 1, nCount, ILCF_SWAP );
			}

			return TRUE;
		}
	}

	m_pPaths.Add( strXML );
	m_pTitles.Add( strName );
	m_pGUIDirs.Add( strGUIDir );
	m_pLangCodes.Add( strLangCode );
	m_pPriorities.Add( nPriority );
	m_pImages.Add( hIcon );

	return TRUE;
}

void CLanguageDlg::Execute(int nSelected)
{
	// Don't try to process selections past the end of the list
	if ( nSelected - 1 >= m_pPaths.GetSize() ) return;

	for ( int nItem = 0 ; nItem < m_pPaths.GetSize() ; nItem++ )
	{
		theApp.WriteProfileInt( L"Skins", m_pPaths.GetAt( nItem ),
			( nSelected - 1 ) == nItem );
	}

	m_bLanguageRTL = ( nSelected > 1 ) &&
		( m_pGUIDirs.GetAt( nSelected - 1 ) == L"rtl" );

	if ( nSelected > 1 )
		m_sLanguage = m_pLangCodes.GetAt( nSelected - 1 );

	//CString strLangCode = L"en";
	//if ( nSelected > 1 ) strLangCode = m_pLangCodes.GetAt( nSelected - 2 );
	//
	//// Required to have schemas reloaded after restart
	//Settings.General.Language = strLangCode;
	//
	//if ( Settings.General.LanguageRTL != ( bRTL != FALSE ) )
	//{
	//	Settings.General.LanguageRTL = bRTL != FALSE;
	//	if ( MsgBox( IDS_WARNING_RTL, MB_ICONQUESTION|MB_YESNO ) == IDYES )
	//	{
	//		GetParent()->PostMessage( WM_CLOSE, NULL, NULL );
	//		EndDialog( IDCANCEL );
	//	}
	//}

	EndDialog( IDOK );
}

void CLanguageDlg::OnClose()
{
	EndDialog( IDCANCEL );
}
