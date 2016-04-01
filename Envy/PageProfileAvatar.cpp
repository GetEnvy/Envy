//
// PageProfileAvatar.cpp
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
#include "PageProfileAvatar.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "GProfile.h"
#include "Colors.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CAvatarProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CAvatarProfilePage, CSettingsPage)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_AVATAR_BROWSE, OnAvatarBrowse)
	ON_BN_CLICKED(IDC_AVATAR_REMOVE, OnAvatarRemove)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAvatarProfilePage property page

CAvatarProfilePage::CAvatarProfilePage() : CSettingsPage( CAvatarProfilePage::IDD )
{
}

CAvatarProfilePage::~CAvatarProfilePage()
{
}

void CAvatarProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AVATAR_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_PREVIEW, m_wndPreview);
}

/////////////////////////////////////////////////////////////////////////////
// CAvatarProfilePage message handlers

BOOL CAvatarProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	if ( CXMLElement* pAvatar = MyProfile.GetXML( L"avatar" ) )
		m_sAvatar = pAvatar->GetAttributeValue( L"path" );
	else
		m_sAvatar = Settings.General.Path + L"\\Data\\DefaultAvatar.png";	// Settings.General.DataPath ?

	PrepareImage();

	return TRUE;
}

void CAvatarProfilePage::OnOK()
{
	if ( CXMLElement* pAvatar = MyProfile.GetXML( L"avatar", TRUE ) )
		pAvatar->AddAttribute( L"path", m_sAvatar );

	CSettingsPage::OnOK();
}

void CAvatarProfilePage::OnPaint()
{
	CPaintDC dc( this );

	CRect rc;
	m_wndPreview.GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.right = rc.left + 128;
	rc.bottom = rc.top + 128;

	if ( m_bmAvatar.m_hObject != NULL )
	{
		CDC dcMem;
		dcMem.CreateCompatibleDC( &dc );
		CBitmap* pOld = (CBitmap*)dcMem.SelectObject( &m_bmAvatar );
		dc.BitBlt( rc.left, rc.top, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY );
		dcMem.SelectObject( pOld );
	}
	else
	{
		dc.Draw3dRect( &rc, Colors.m_crWindow, Colors.m_crWindow );
		rc.InflateRect( 1, 1 );
		dc.Draw3dRect( &rc, Colors.m_crMidtone, Colors.m_crMidtone );
	}
}

void CAvatarProfilePage::OnAvatarBrowse()
{
	CFileDialog dlg( TRUE, L"png", m_sAvatar, OFN_HIDEREADONLY,
		L"Image Files|*.jpg;*.jpeg;*.png;*.bmp|" + LoadString( IDS_FILES_ALL ) + L"|*.*||", this );

	if ( dlg.DoModal() == IDOK )
	{
		m_sAvatar = dlg.GetPathName();
		PrepareImage();
		Invalidate();
	}
}

void CAvatarProfilePage::OnAvatarRemove()
{
	m_sAvatar.Empty();
	if ( m_bmAvatar.m_hObject != NULL ) m_bmAvatar.DeleteObject();
	Invalidate();
}

void CAvatarProfilePage::PrepareImage()
{
	if ( m_bmAvatar.m_hObject != NULL ) m_bmAvatar.DeleteObject();
	if ( m_sAvatar.IsEmpty() ) return;

	CImageFile pFile;

	CClientDC dc( this );
	SendMessage( WM_CTLCOLORSTATIC, (WPARAM)dc.GetSafeHdc(), (LPARAM)m_wndPreview.GetSafeHwnd() );

	if ( pFile.LoadFromFile( m_sAvatar ) && pFile.EnsureRGB( Colors.m_crDialog ) )	//dc.GetBkColor()
	{
		pFile.Resample( 128, 128 );
		m_bmAvatar.Attach( pFile.CreateBitmap() );
	}
}
