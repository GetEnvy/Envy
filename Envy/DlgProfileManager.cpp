//
// DlgProfileManager.cpp
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
#include "DlgProfileManager.h"
#include "GProfile.h"
#include "Skin.h"

#include "PageProfileIdentity.h"
#include "PageProfileContact.h"
#include "PageProfileProfile.h"
#include "PageProfileBio.h"
#include "PageProfileAvatar.h"
#include "PageProfileFavorites.h"
#include "PageProfileFiles.h"
#include "PageProfileCertificate.h"
#include "PageSettingsRich.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CProfileManagerDlg, CSettingsSheet)

BEGIN_MESSAGE_MAP(CProfileManagerDlg, CSettingsSheet)
	ON_COMMAND(IDRETRY, OnApply)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProfileManagerDlg construction

CProfileManagerDlg::CProfileManagerDlg(CWnd* pParent)
	: CSettingsSheet( pParent, IDS_USER_PROFILE )
{
}

//CProfileManagerDlg::~CProfileManagerDlg()
//{
//}

/////////////////////////////////////////////////////////////////////////////
// CProfileManagerDlg operations

BOOL CProfileManagerDlg::Run(LPCTSTR pszWindow)
{
	CProfileManagerDlg pSheet;
	BOOL bResult = ( pSheet.DoModal( pszWindow ) == IDOK );
	return bResult;
}

INT_PTR CProfileManagerDlg::DoModal(LPCTSTR pszWindow)
{
	CIdentityProfilePage		pIdentity;
	CAvatarProfilePage			pAvatar;
	CContactProfilePage			pContact;
	CProfileProfilePage			pProfile;
	CBioProfilePage				pBio;
	CFilesProfilePage			pFiles;
	CFavoritesProfilePage		pFavorites;
	CCertificateProfilePage 	pCertificate;

	AddGroup( &pIdentity );		// IDD_PROFILE_IDENTITY
	AddPage( &pAvatar );		// IDD_PROFILE_AVATAR
	AddPage( &pProfile );		// IDD_PROFILE_PROFILE
	AddPage( &pContact );		// IDD_PROFILE_CONTACT
	AddPage( &pBio );			// IDD_PROFILE_BIO
	AddGroup( &pFiles );		// IDD_PROFILE_FILES
	AddPage( &pFavorites );		// IDD_PROFILE_FAVORITES
	AddGroup( &pCertificate );	// IDD_PROFILE_CERTIFICATE

	if ( pszWindow ) SetActivePage( GetPage( pszWindow ) );

	INT_PTR nReturn = CSettingsSheet::DoModal();

	return nReturn;
}

void CProfileManagerDlg::AddPage(CSettingsPage* pPage)
{
	CString strCaption = Skin.GetDialogCaption( CString( pPage->GetRuntimeClass()->m_lpszClassName ) );
	CSettingsSheet::AddPage( pPage, strCaption.GetLength() ? (LPCTSTR)strCaption : NULL );
}

void CProfileManagerDlg::AddGroup(CSettingsPage* pPage)
{
	if ( pPage->IsKindOf( RUNTIME_CLASS(CRichSettingsPage) ) )
	{
		CString strCaption = ((CRichSettingsPage*)pPage)->m_sCaption;
		CSettingsSheet::AddGroup( pPage, strCaption );
	}
	else
	{
		CString strName( pPage->GetRuntimeClass()->m_lpszClassName );
		CString strCaption = Skin.GetDialogCaption( strName );
		CSettingsSheet::AddGroup( pPage, strCaption.GetLength() ? (LPCTSTR)strCaption : NULL );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CProfileManagerDlg message handlers

BOOL CProfileManagerDlg::OnInitDialog()
{
	CSettingsSheet::OnInitDialog();

//	m_bmHeader.LoadBitmap( IDB_BANNER );

	SkinMe( L"CProfileManagerDlg", IDR_MAINFRAME );

	return TRUE;
}

void CProfileManagerDlg::OnOK()
{
	CSettingsSheet::OnOK();
	MyProfile.Save();
}

void CProfileManagerDlg::OnApply()
{
	CSettingsSheet::OnApply();
	MyProfile.Save();
}

// Obsolete: Banner handled by WndSettingsPage
//void CProfileManagerDlg::DoPaint(CDC& dc)
//{
//	CRect rc;
//	GetClientRect( &rc );
//	BITMAP pInfo;
//	m_bmHeader.GetBitmap( &pInfo );
//
//	CDC mdc;
//	mdc.CreateCompatibleDC( &dc );
//	CBitmap* pOldBitmap = (CBitmap*)mdc.SelectObject( &m_bmHeader );
//	dc.BitBlt( 0, 0, pInfo.bmWidth, pInfo.bmHeight, &mdc, 0, 0, SRCCOPY );
//	mdc.SelectObject( pOldBitmap );
//	mdc.DeleteDC();
//
//	CSettingsSheet::DoPaint( dc );
//}
