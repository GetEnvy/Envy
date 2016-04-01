//
// DlgProgressBar.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2009-2010 and Shareaza 2009
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
#include "DlgProgressBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CProgressBarDlg, CSkinDialog)

//BEGIN_MESSAGE_MAP(CProgressBarDlg, CSkinDialog)
//END_MESSAGE_MAP()

CProgressBarDlg::CProgressBarDlg(CWnd* pParent) :
	CSkinDialog	( CProgressBarDlg::IDD, pParent )
{
	Create( CProgressBarDlg::IDD, pParent );
}

CProgressBarDlg::~CProgressBarDlg()
{
}

void CProgressBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LABEL_ACTION, m_sAction);
	DDX_Text(pDX, IDC_LABEL_EVENT, m_sEvent);
	DDX_Control(pDX, IDC_PROGRESS_EVENT, m_oEventProgress);
	DDX_Text(pDX, IDC_LABEL_SUB_ACTION, m_sSubAction);
	DDX_Text(pDX, IDC_SUB_EVENT, m_sSubEvent);
	DDX_Control(pDX, IDC_PROGRESS_SUB_EVENT, m_oSubEventProgress);
}

void CProgressBarDlg::SetActionText(const CString& strText)
{
	m_sAction = strText;
}

void CProgressBarDlg::SetEventText(const CString& strText)
{
	m_sEvent = strText;
}

void CProgressBarDlg::SetEventRange(const int nLower, const int nUpper)
{
	m_oEventProgress.SetRange32( nLower, nUpper );
	m_oEventProgress.SetPos( nLower );
}

void CProgressBarDlg::SetEventPos(const int nPos)
{
	m_oEventProgress.SetPos( nPos );
}

void CProgressBarDlg::SetEventStep(const int nStep)
{
	m_oEventProgress.SetStep( nStep );
}

void CProgressBarDlg::StepEvent()
{
	m_oEventProgress.StepIt();
}

void CProgressBarDlg::StepEvent( const int nPos )
{
	m_oEventProgress.OffsetPos( nPos );
}

void CProgressBarDlg::SetSubActionText(const CString& strText)
{
	m_sSubAction = strText;
}

void CProgressBarDlg::SetSubEventText(const CString& strText)
{
	m_sSubEvent = strText;
}

void CProgressBarDlg::SetSubEventRange(const int nLower, const int nUpper)
{
	m_oSubEventProgress.SetRange32( nLower, nUpper );
	m_oSubEventProgress.SetPos( nLower );
}

void CProgressBarDlg::SetSubEventPos(const int nPos)
{
	m_oSubEventProgress.SetPos( nPos );
}

void CProgressBarDlg::SetSubEventStep(const int nStep)
{
	m_oSubEventProgress.SetStep( nStep );
}

void CProgressBarDlg::StepSubEvent()
{
	m_oSubEventProgress.StepIt();
}

void CProgressBarDlg::StepSubEvent( const int nPos )
{
	m_oSubEventProgress.OffsetPos( nPos );
}

BOOL CProgressBarDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CProgressBarDlg", IDR_MAINFRAME );

	if ( Settings.General.LanguageRTL )
	{
		m_oEventProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_oSubEventProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	}

	return TRUE;
}

void CProgressBarDlg::OnOK() {}
void CProgressBarDlg::OnCancel() {}
