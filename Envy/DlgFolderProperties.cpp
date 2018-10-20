//
// DlgFolderProperties.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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
#include "DlgFolderProperties.h"

#include "Library.h"
#include "LibraryFolders.h"
#include "AlbumFolder.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "SchemaChild.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CFolderPropertiesDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CFolderPropertiesDlg, CSkinDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_GETMINMAXINFO()
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, OnCloseUpSchemas)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_EN_CHANGE(IDC_TITLE, OnChangeTitle)
	ON_EN_CHANGE(IDC_METADATA, OnChangeData)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFolderPropertiesDlg dialog

CFolderPropertiesDlg::CFolderPropertiesDlg(CWnd* pParent, CAlbumFolder* pFolder)
	: CSkinDialog( CFolderPropertiesDlg::IDD, pParent )
	, m_pFolder	( pFolder )
	, m_nWidth	( 0 )
	, m_bUpdating ( FALSE )
{
}

void CFolderPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APPLY_METADATA, m_wndApply);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_TITLE, m_wndTitle);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
}

/////////////////////////////////////////////////////////////////////////////
// CFolderPropertiesDlg message handlers

BOOL CFolderPropertiesDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CFolderPropertiesDlg", IDR_LIBRARYFRAME );

	CRect rc;
	GetWindowRect( &rc );
	m_nWidth = rc.Width();

	m_wndData.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP, rc, this, IDC_METADATA );

	if ( ! Settings.LoadWindow( L"CFolderPropertiesDlg", this ) )
	{
		GetWindowRect( &rc );
		rc.bottom++;
		MoveWindow( &rc );
	}

	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( LibraryFolders.CheckAlbum( m_pFolder ) )
	{
		m_wndTitle.SetWindowText( m_pFolder->m_sName );
		m_wndSchemas.Load( m_pFolder->m_sSchemaURI, CSchema::stFolder );
		if ( m_wndSchemas.GetCurSel() < 0 ) m_wndSchemas.SetCurSel( 0 );

		OnSelChangeSchemas();

		m_bUpdating = TRUE;
		if ( m_pFolder->m_pXML )
			m_wndData.UpdateData( m_pFolder->m_pXML, FALSE );
		m_bUpdating = FALSE;
	}
	else
	{
		PostMessage( WM_CLOSE );
		return TRUE;
	}

	pLock.Unlock();

	CString strSchemaURI = m_wndData.GetSchemaURI();
	if ( CSchemaPtr pSchema = SchemaCache.Get( strSchemaURI ) )
	{
		CString strChildURI = pSchema->GetContainedURI( CSchema::stFile );
		CSchemaChild* pContained = pSchema->GetContained( strChildURI );
		if ( pContained == NULL || pContained->m_pMap.GetCount() == 0 )
			m_wndApply.ShowWindow( SW_HIDE );
	}

	return TRUE;
}

void CFolderPropertiesDlg::OnDestroy()
{
	Settings.SaveWindow( L"CFolderPropertiesDlg", this );
	CSkinDialog::OnDestroy();
}

void CFolderPropertiesDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CSkinDialog::OnGetMinMaxInfo( lpMMI );

	if ( m_nWidth )
	{
		lpMMI->ptMinTrackSize.x = m_nWidth;
		lpMMI->ptMinTrackSize.y = 256;
		lpMMI->ptMaxTrackSize.x = m_nWidth;
	}
}

void CFolderPropertiesDlg::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 ) CSkinDialog::OnSize( nType, cx, cy );

	if ( ! IsWindow( m_wndData.m_hWnd ) ) return;

	CRect rc, rcClient;
	GetClientRect( &rcClient );

	m_wndTitle.GetWindowRect( &rc );
	ScreenToClient( &rc );
	int nRight = rc.right;

	m_wndSchemas.GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.right = nRight;
	rc.left  = rcClient.right - rc.right;

	HDWP hDWP = BeginDeferWindowPos( 4 );

	DeferWindowPos( hDWP, m_wndData, NULL, rc.left, rc.bottom + 18,
		rc.Width(), cy - 24 - 16 - 16 - ( rc.bottom + 18 ),
		SWP_NOACTIVATE | SWP_NOZORDER );

	m_wndApply.GetWindowRect( &rc );
	ScreenToClient( &rc );
	DeferWindowPos( hDWP, m_wndApply, NULL, rc.left, cy - 32 - 19, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );

	m_wndOK.GetWindowRect( &rc );
	ScreenToClient( &rc );

	DeferWindowPos( hDWP, m_wndOK, NULL, rc.left, cy - 32, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndCancel, NULL, rc.right + 8, cy - 32, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );

	EndDeferWindowPos( hDWP );
}

void CFolderPropertiesDlg::OnPaint()
{
	CPaintDC dc( this );
	CRect rc( 8, 6 + Skin.m_nBanner, 8 + 98, 6 + 98 + Skin.m_nBanner );

	const COLORREF crBack = CColors::CalculateColor( Colors.m_crDialog, RGB( 255, 255, 255 ), 40 );

	dc.Draw3dRect( &rc, Colors.m_crSysActiveCaption, Colors.m_crSysActiveCaption );
	rc.DeflateRect( 1, 1 );

	CPoint pt = rc.CenterPoint();

	CSchemaPtr pSchema = m_wndSchemas.GetSelected();

	if ( pSchema && pSchema->m_nIcon48 >= 0 )
	{
		// ImageList_DrawEx()
		pt.x -= 24;
		pt.y -= 24;
		ShellIcons.Draw( &dc, pSchema->m_nIcon48, 48, pt.x, pt.y, crBack );
		dc.ExcludeClipRect( pt.x, pt.y, pt.x + 48, pt.y + 48 );
	}
	else if ( pSchema && pSchema->m_nIcon32 >= 0 )
	{
		pt.x -= 16;
		pt.y -= 16;
		ShellIcons.Draw( &dc, pSchema->m_nIcon32, 32, pt.x, pt.y, crBack );
		dc.ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );
	}
	else
	{
		pt.x -= 24;
		pt.y -= 24;
		CoolInterface.Draw( &dc, IDI_FOLDER_OPEN, 48, pt.x, pt.y, crBack );
		dc.ExcludeClipRect( pt.x, pt.y, pt.x + 48, pt.y + 48 );
	}

	dc.FillSolidRect( &rc, crBack );
}

HBRUSH CFolderPropertiesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndApply )
	{
		pDC->SetTextColor( Colors.m_crTextLink );
		pDC->SelectObject( &theApp.m_gdiFontLine );
	}

	return hbr;
}

BOOL CFolderPropertiesDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	CRect rc;

	GetCursorPos( &point );
	m_wndApply.GetWindowRect( &rc );

	if ( rc.PtInRect( point ) )
	{
		SetCursor( theApp.LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	return CSkinDialog::OnSetCursor( pWnd, nHitTest, message );
}

void CFolderPropertiesDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CSkinDialog::OnLButtonUp( nFlags, point );

	CRect rc;
	m_wndApply.GetWindowRect( &rc );
	ScreenToClient( &rc );

	if ( rc.PtInRect( point ) )
		DoApply( TRUE );
}

void CFolderPropertiesDlg::OnSelChangeSchemas()
{
	m_wndData.SetSchema( m_wndSchemas.GetSelected() );

	CString strSchemaURI = m_wndData.GetSchemaURI();
	if ( CSchemaPtr pSchema = SchemaCache.Get( strSchemaURI ) )
	{
		CString strChildURI = pSchema->GetContainedURI( CSchema::stFile );
		CSchemaChild* pContained = pSchema->GetContained( strChildURI );
		if ( pContained == NULL || pContained->m_pMap.GetCount() == 0 )
			m_wndApply.ShowWindow( SW_HIDE );
		else
			m_wndApply.ShowWindow( SW_SHOW );
	}

	OnChangeTitle();
	Invalidate();
}

void CFolderPropertiesDlg::OnCloseUpSchemas()
{
	if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
		PostMessage( WM_KEYDOWN, VK_TAB );
}

void CFolderPropertiesDlg::OnChangeTitle()
{
	if ( m_bUpdating ) return;
	m_bUpdating = TRUE;

	if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
	{
		CString strTitle;
		m_wndTitle.GetWindowText( strTitle );

		CXMLElement* pXML = new CXMLElement( NULL, pSchema->m_sSingular );
		m_wndData.UpdateData( pXML, TRUE );
		pXML->AddAttribute( pSchema->GetFirstMemberName(), strTitle );
		m_wndData.UpdateData( pXML, FALSE );
		delete pXML;
	}

	m_bUpdating = FALSE;
}

void CFolderPropertiesDlg::OnChangeData()
{
	if ( m_bUpdating ) return;
	m_bUpdating = TRUE;

	if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
	{
		CXMLElement* pXML = new CXMLElement( NULL, pSchema->m_sSingular );
		m_wndData.UpdateData( pXML, TRUE );
		m_wndTitle.SetWindowText( pXML->GetAttributeValue( pSchema->GetFirstMemberName() ) );
		delete pXML;
	}

	m_bUpdating = FALSE;
}

void CFolderPropertiesDlg::OnOK()
{
	DoApply( FALSE );
	CSkinDialog::OnOK();
}

void CFolderPropertiesDlg::DoApply(BOOL bMetaToFiles)
{
	CWaitCursor pCursor;

	CString str;
	m_wndTitle.GetWindowText( str );
	if ( str.IsEmpty() ) return;

	CQuickLock oLock( Library.m_pSection );

	if ( LibraryFolders.CheckAlbum( m_pFolder ) )
	{
		m_wndTitle.GetWindowText( m_pFolder->m_sName );

		if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
		{
			CXMLElement* pXML		= pSchema->Instantiate( TRUE );
			CXMLElement* pSingular	= pXML->AddElement( pSchema->m_sSingular );

			m_wndData.UpdateData( pSingular, TRUE );
			m_pFolder->SetMetadata( pXML );
			delete pXML;
		}
		else
		{
			m_pFolder->SetMetadata( NULL );
		}

		m_pFolder->m_nUpdateCookie++;

		m_pFolder->MetaToFiles( bMetaToFiles );

		Library.Update();
	}
}

void CFolderPropertiesDlg::OnCancel()
{
	CQuickLock oLock( Library.m_pSection );

	if ( LibraryFolders.CheckAlbum( m_pFolder ) )
	{
		if ( m_pFolder->m_sSchemaURI && m_pFolder->m_sSchemaURI.IsEmpty() )
			m_pFolder->Delete();	// "New Folder" created but Cancelled, Only OK button assigns schema
	}

	return CSkinDialog::OnCancel();
}
