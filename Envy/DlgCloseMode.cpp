//
// DlgCloseMode.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2016
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
#include "DlgCloseMode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Settings.General.CloseMode Options:
// 0 Unknown
// 1 Minimize
// 2 Tray
// 3 Close after
// 4 Close now

//BEGIN_MESSAGE_MAP(CCloseModeDlg, CSkinDialog)
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCloseModeDlg dialog

CCloseModeDlg::CCloseModeDlg(CWnd* pParent) : CSkinDialog( CCloseModeDlg::IDD, pParent )
	, m_nMode ( -1 )
{
}

void CCloseModeDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_CLOSE, m_nMode);
}

/////////////////////////////////////////////////////////////////////////////
// CCloseModeDlg message handlers

BOOL CCloseModeDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CCloseModeDlg", IDR_MAINFRAME );

	if ( Settings.General.CloseMode )
		m_nMode = Settings.General.CloseMode - 1;
	else
		m_nMode = 1;

	UpdateData( FALSE );

	return TRUE;
}

void CCloseModeDlg::OnOK()
{
	UpdateData();

	Settings.General.CloseMode = m_nMode + 1;

	if ( Settings.General.CloseMode == 4 )
		CSkinDialog::OnCancel();
	else
		CSkinDialog::OnOK();
}
