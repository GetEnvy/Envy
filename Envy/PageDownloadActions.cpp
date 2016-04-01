//
// PageDownloadActions.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2008
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
#include "DlgDownloadSheet.h"
#include "PageDownloadActions.h"

#include "Download.h"
#include "Downloads.h"
#include "DownloadTask.h"
#include "FragmentedFile.h"
#include "Transfers.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CDownloadActionsPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CDownloadActionsPage, CPropertyPageAdv)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_ERASE, &CDownloadActionsPage::OnErase)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CDownloadActionsPage construction

CDownloadActionsPage::CDownloadActionsPage() :
	CPropertyPageAdv( CDownloadActionsPage::IDD )
{
}

CDownloadActionsPage::~CDownloadActionsPage()
{
}

void CDownloadActionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CANCEL_DOWNLOAD, m_wndCancelDownload);
	DDX_Control(pDX, IDC_MERGE_AND_VERIFY, m_wndMergeVerify);
	DDX_Control(pDX, IDC_COMPLETE_AND_VERIFY, m_wndCompleteVerify);
	DDX_Control(pDX, IDC_FORGET_VERIFY, m_wndForgetVerify);
	DDX_Control(pDX, IDC_FORGET_SOURCES, m_wndForgetSources);
	DDX_Text(pDX, IDC_ERASE_FROM, m_sEraseFrom);
	DDX_Text(pDX, IDC_ERASE_TO, m_sEraseTo);
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadActionsPage message handlers

BOOL CDownloadActionsPage::OnInitDialog()
{
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;

	ASSUME_LOCK( Transfers.m_pSection );

	return TRUE;
}

HBRUSH CDownloadActionsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPageAdv::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndCancelDownload ||
		 pWnd == &m_wndMergeVerify ||
		 pWnd == &m_wndCompleteVerify ||
		 pWnd == &m_wndForgetVerify ||
		 pWnd == &m_wndForgetSources )
	{
		pDC->SelectObject( &theApp.m_gdiFontLine );
		pDC->SetTextColor( Colors.m_crTextLink );
	}

	return hbr;
}

BOOL CDownloadActionsPage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CRect rcCtrl1, rcCtrl2, rcCtrl3, rcCtrl4, rcCtrl5;
	CPoint point;

	GetCursorPos( &point );
	m_wndCancelDownload.GetWindowRect( &rcCtrl1 );
	m_wndMergeVerify.GetWindowRect( &rcCtrl2 );
	m_wndCompleteVerify.GetWindowRect( &rcCtrl3 );
	m_wndForgetVerify.GetWindowRect( &rcCtrl4 );
	m_wndForgetSources.GetWindowRect( &rcCtrl5 );

	if ( rcCtrl1.PtInRect( point ) ||
		 rcCtrl2.PtInRect( point ) ||
		 rcCtrl3.PtInRect( point ) ||
		 rcCtrl4.PtInRect( point ) ||
		 rcCtrl5.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	return CPropertyPageAdv::OnSetCursor( pWnd, nHitTest, message );
}

void CDownloadActionsPage::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPropertyPageAdv::OnLButtonUp(nFlags, point);

	CRect rcCtrl1, rcCtrl2, rcCtrl3, rcCtrl4, rcCtrl5;

	m_wndCancelDownload.GetWindowRect( &rcCtrl1 );
	ScreenToClient( &rcCtrl1 );
	m_wndMergeVerify.GetWindowRect( &rcCtrl2 );
	ScreenToClient( &rcCtrl2 );
	m_wndCompleteVerify.GetWindowRect( &rcCtrl3 );
	ScreenToClient( &rcCtrl3 );
	m_wndForgetVerify.GetWindowRect( &rcCtrl4 );
	ScreenToClient( &rcCtrl4 );
	m_wndForgetSources.GetWindowRect( &rcCtrl5 );
	ScreenToClient( &rcCtrl5 );

	if ( rcCtrl1.PtInRect( point ) )
		OnCancelDownload();
	else if ( rcCtrl2.PtInRect( point ) )
		OnMergeAndVerify();
	else if ( rcCtrl3.PtInRect( point ) )
		OnCompleteVerify();
	else if ( rcCtrl4.PtInRect( point ) )
		OnForgetVerify();
	else if ( rcCtrl5.PtInRect( point ) )
		OnForgetSources();
}

void CDownloadActionsPage::OnErase()
{
	QWORD nFrom = 0, nTo = 0;

	if ( ! UpdateData() ||
		 _stscanf( m_sEraseFrom, L"%I64i", &nFrom ) != 1 ||
		 _stscanf( m_sEraseTo, L"%I64i", &nTo ) != 1 ||
		 nTo < nFrom )
	{
		MsgBox( IDS_DOWNLOAD_EDIT_BAD_RANGE, MB_ICONEXCLAMATION );
		return;
	}

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return;

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
		return;
	}

	pDownload->CloseTransfers();
	QWORD nErased = pDownload->EraseRange( nFrom, nTo + 1 - nFrom );

	if ( nErased > 0 )
	{
		pDownload->ClearVerification();
		pLock.Unlock();
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_ERASED ), nErased );
		MsgBox( strMessage, MB_ICONINFORMATION );
	}
	else
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_CANT_ERASE, MB_ICONEXCLAMATION );
	}
}

void CDownloadActionsPage::OnForgetVerify()
{
	if ( MsgBox( IDS_DOWNLOAD_EDIT_FORGET_VERIFY, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return;

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
		return;
	}

	pDownload->ClearVerification();
}

void CDownloadActionsPage::OnForgetSources()
{
	if ( MsgBox( IDS_DOWNLOAD_EDIT_FORGET_SOURCES, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return;

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
		return;
	}

	pDownload->CloseTransfers();
	pDownload->ClearSources();
	pDownload->SetModified();
}

void CDownloadActionsPage::OnCompleteVerify()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return;

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
		return;
	}

	if (  pDownload->NeedTigerTree() &&
		  pDownload->NeedHashset() &&
		! pDownload->IsTorrent() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_COMPLETE_NOHASH, MB_ICONEXCLAMATION );
		return;
	}

	pLock.Unlock();
	if ( MsgBox( IDS_DOWNLOAD_EDIT_COMPLETE_VERIFY, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
	pLock.Lock();

	pDownload->MakeComplete();
	pDownload->ResetVerification();
	pDownload->SetModified();
}

void CDownloadActionsPage::OnMergeAndVerify()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return;

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
		return;
	}

	if ( pDownload->IsCompleted() || ! pDownload->PrepareFile() )
		return;		// Download almost completed

	const Fragments::List oList( pDownload->GetEmptyFragmentList() );
	if ( ! oList.size() )
		return;		// No available fragments

	CString strName = pDownload->m_sName;

	pLock.Unlock();

	// Select file
	CString strExt( PathFindExtension( strName ) );
	if ( ! strExt.IsEmpty() ) strExt = strExt.Mid( 1 );

	CFileDialog dlgSelectFile( TRUE, strExt, strName,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT, NULL, this );

	CAutoVectorPtr< TCHAR >szFiles( new TCHAR[ 2048 ] );
	if ( ! szFiles )
		return;	// Out of memory

	*szFiles = 0;
	dlgSelectFile.GetOFN().lpstrFile = szFiles;
	dlgSelectFile.GetOFN().nMaxFile = 2048;

	if ( dlgSelectFile.DoModal() != IDOK )
		return;

	pLock.Lock();
	pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return;

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
		return;
	}

	CList< CString > oFiles;
	CString strFolder = (LPCTSTR)szFiles;
	for ( LPCTSTR szFile = szFiles ; *szFile ; )
	{
		szFile += _tcslen( szFile ) + 1;
		if ( *szFile )	// Folder + files
			oFiles.AddTail( strFolder + L"\\" + szFile );
		else	// Single file
			oFiles.AddTail( strFolder );
	}

	if ( oFiles.GetCount() )
		pDownload->MergeFile( &oFiles );
}

void CDownloadActionsPage::OnCancelDownload()
{
	if ( MsgBox( IDS_DOWNLOAD_EDIT_CANCEL_DOWNLOAD, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload )
		return;

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		MsgBox( IDS_DOWNLOAD_EDIT_ACTIVE_TASK, MB_ICONEXCLAMATION );
		return;
	}

	if ( pDownload->IsCompleted() )
		return;		// Download almost completed

	pDownload->ForceComplete();
}
