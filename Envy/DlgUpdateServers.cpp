//
// DlgUpdateServers.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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

// Fetch .met and other hostcache lists from the web
// Was DlgDonkeyServers.cpp, CDonkeyServersDlg

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "DlgUpdateServers.h"
#include "HostCache.h"
#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CUpdateServersDlg, CSkinDialog)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_URL, OnChangeURL)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUpdateServersDlg dialog

CUpdateServersDlg::CUpdateServersDlg(CWnd* pParent)
	: CSkinDialog(CUpdateServersDlg::IDD, pParent)
{
}

void CUpdateServersDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URL, m_wndURL);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Text(pDX, IDC_URL, m_sURL);
}

/////////////////////////////////////////////////////////////////////////////
// CUpdateServersDlg message handlers

BOOL CUpdateServersDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CUpdateServersDlg", IDR_MAINFRAME );

	// Define dlg.m_URL = Settings.DC.HubListURL etc. before dlg.DoModal()
	if ( m_sURL.GetLength() < 12 )
		m_sURL = Settings.eDonkey.ServerListURL;

	m_wndOK.EnableWindow( IsValidURL() );
	m_wndProgress.SetRange( 0, 100 );
	m_wndProgress.SetPos( 0 );

	UpdateData( FALSE );

	return TRUE;
}

BOOL CUpdateServersDlg::IsValidURL()
{
	return
		m_sURL.GetLength() > 12 &&
		StartsWith( m_sURL, L"http://", 7 ) &&
	//	m_sURL.Find( L'/', 7 ) > 12 &&
		m_sURL.Find( L'.', 7 ) > 8;
}

void CUpdateServersDlg::OnChangeURL()
{
	UpdateData();

	m_wndOK.EnableWindow( IsValidURL() );
}

void CUpdateServersDlg::OnOK()
{
	UpdateData();

	if ( ! IsValidURL() ) return;
	if ( ! m_pRequest.SetURL( m_sURL ) ) return;
	if ( ! m_pRequest.Execute( true ) ) return;

	m_wndOK.EnableWindow( FALSE );
	m_wndURL.EnableWindow( FALSE );

	SetTimer( 1, 100, NULL );
}

void CUpdateServersDlg::OnCancel()
{
	KillTimer( 1 );

	m_pRequest.Cancel();

	CSkinDialog::OnCancel();
}

void CUpdateServersDlg::OnTimer(UINT_PTR nIDEvent)
{
	CSkinDialog::OnTimer( nIDEvent );

	if ( m_pRequest.IsPending() )
	{
		int n = m_wndProgress.GetPos();
		if ( n < 5 )
			n = 5;
		else if ( n < 100 )
			n++;
		m_wndProgress.SetPos( n );
	}
	else
	{
		KillTimer( 1 );

		if ( m_pRequest.GetStatusSuccess() )
		{
			const CString strExt = CString( PathFindExtension( m_sURL ) ).MakeLower();
			if ( strExt == L".met" || m_sURL.Find( L"//server", 8 ) > 8 )		// || strExt == L".php"
				Settings.eDonkey.ServerListURL = m_sURL;
			else if ( strExt == L".bz2" || m_sURL.Find( L"hublist", 8 ) > 8 )
				Settings.DC.HubListURL = m_sURL;
		//	else if ( strExt == L".xml" )
		//		Settings.Gnutella.CacheURL = m_sURL;
		//	else if ( strExt == L".dat" )
		//		Settings.KAD.NodesListURL = m_sURL;

			const CBuffer* pBuffer = m_pRequest.GetResponseBuffer();

			CMemFile pFile;
			pFile.Write( pBuffer->m_pBuffer, pBuffer->m_nLength );
			pFile.Seek( 0, CFile::begin );

			if ( ( strExt == L".bz2" && HostCache.ImportHubList( &pFile ) ) ||
				 HostCache.ImportMET( &pFile ) )
			//	 HostCache.ImportCache( &pFile ) || 	// ToDo: G2/Gnutella loading
			//	 HostCache.ImportNodes( &pFile ) )		// ToDo: KAD
			{
				HostCache.Save();

				m_sURL.Empty();
				EndDialog( IDOK );
				return;
			}
		}

		CString strError;
		strError.Format( LoadString( IDS_DOWNLOAD_DROPPED ), (LPCTSTR)m_sURL );
		MsgBox( strError, MB_OK | MB_ICONEXCLAMATION );

		m_sURL.Empty();
		EndDialog( IDCANCEL );
	}

	UpdateWindow();
}
