//
// SaveFilterAsDlg.cpp
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
// Original Author: roo_koo_too@yahoo.com
//

#include "StdAfx.h"
#include "Envy.h"
#include "DlgSaveFilterAs.h"
#include "DlgFilterSearch.h"
#include "ResultFilters.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// CSaveFilterAsDlg dialog

CSaveFilterAsDlg::CSaveFilterAsDlg( CWnd* pParent /*=NULL*/ )
	: CSkinDialog( CSaveFilterAsDlg::IDD, pParent )
{
}

CSaveFilterAsDlg::~CSaveFilterAsDlg()
{
}

void CSaveFilterAsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text( pDX, IDC_NAME, m_sName );
}

BEGIN_MESSAGE_MAP(CSaveFilterAsDlg, CSkinDialog)
	ON_EN_CHANGE(IDC_NAME, OnEnChangeName)
END_MESSAGE_MAP()


// CSaveFilterAsDlg message handlers

void CSaveFilterAsDlg::OnOK()
{
	if ( m_sName.IsEmpty() )
	{
		MsgBox( IDS_FILTER_NO_NAME, MB_OK );
		return;
	}

	if ( ( (CFilterSearchDlg*)m_pParentWnd)->m_pResultFilters->Search( m_sName ) >= 0 )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_FILTER_REPLACE ), (LPCTSTR)m_sName );
		if ( MsgBox( strMessage, MB_ICONQUESTION | MB_YESNO ) == IDNO )
			return;
	}

	CSkinDialog::OnOK();
}


BOOL CSaveFilterAsDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CSaveFilterAsDlg", IDR_SEARCHFRAME );

	return TRUE;
}

void CSaveFilterAsDlg::OnEnChangeName()
{
	UpdateData(TRUE);

	GetDlgItem( IDOK )->EnableWindow( ! m_sName.IsEmpty() );
}
