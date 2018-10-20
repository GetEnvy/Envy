//
// DlgURLCopy.cpp
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
#include "Envy.h"
#include "DlgURLCopy.h"
#include "Transfer.h"
#include "Transfers.h"	// Lock
#include "Network.h"
#include "Colors.h"

#include "Library.h"
#include "Download.h"	// &tr=
#include "Downloads.h"
//#include "EnvyFile.h"	// In Header


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CURLCopyDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CURLCopyDlg, CSkinDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_INCLUDE_SELF, &CURLCopyDlg::OnIncludeSelf)
	ON_STN_CLICKED(IDC_URL_MAGNET, &CURLCopyDlg::OnStnClickedUrlMagnet)
	ON_STN_CLICKED(IDC_URL_ED2K, &CURLCopyDlg::OnStnClickedUrlEd2k)
	ON_STN_CLICKED(IDC_URL_HOST, &CURLCopyDlg::OnStnClickedUrlHost)
	ON_STN_CLICKED(IDC_URL_HASH, &CURLCopyDlg::OnStnClickedUrlHash)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CURLCopyDlg dialog

CURLCopyDlg::CURLCopyDlg(CWnd* pParent) : CSkinDialog(CURLCopyDlg::IDD, pParent)
{
}

void CURLCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INCLUDE_SELF, m_wndIncludeSelf);
	DDX_Control(pDX, IDC_MESSAGE, m_wndMessage);
	DDX_Text(pDX, IDC_URL_MAGNET, m_sMagnet);
	DDX_Text(pDX, IDC_URL_ED2K, m_sED2K);
	DDX_Text(pDX, IDC_URL_HOST, m_sHost);
	DDX_Text(pDX, IDC_URL_HASH, m_sHash);
}

void CURLCopyDlg::Add(const CEnvyFile* pFile)
{
	ASSERT( pFile != NULL );
	m_pFile = *pFile;
}

/////////////////////////////////////////////////////////////////////////////
// CURLCopyDlg message handlers

BOOL CURLCopyDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CURLCopyDlg", IDI_WEB_URL );

	m_wndIncludeSelf.ShowWindow( ( Network.IsListening() && m_pFile.m_sURL.IsEmpty() ) ? SW_SHOW : SW_HIDE );

	OnIncludeSelf();

	return TRUE;
}

void CURLCopyDlg::Resolve(CEnvyFile& pFile, CString& sTracker)
{
	// Use contents of .torrent-file instead of file itself
	if ( ! pFile.m_sPath.IsEmpty() && EndsWith( pFile.m_sName, _P( L".torrent" ) ) )		// _tcsicmp( PathFindExtension( pFile.m_sName ), L".torrent" ) == 0 )
	{
		CBTInfo pTorrent;
		if ( pTorrent.LoadTorrentFile( pFile.m_sPath + L"\\" + pFile.m_sName ) )
		{
			pFile = pTorrent;

			// Get trackers
			const int nCount = pTorrent.GetTrackerCount();
			for ( int i = 0; i < nCount; ++i )
			{
				if ( sTracker.GetLength() ) sTracker += L"&";
				sTracker += L"tr=" + URLEncode( pTorrent.GetTrackerAddress( i ) );
			}
		}
	}

	if ( ! ( pFile.m_oBTH ) )
		return;

	CSingleLock pTransfersLock( &Transfers.m_pSection );
	if ( SafeLock( pTransfersLock ) )
	{
		if ( const CDownload* pDownload = Downloads.FindByBTH( pFile.m_oBTH ) )
		{
			// Refill missed hashes
			if ( ! pFile.m_oSHA1 && pDownload->m_oSHA1 )
				pFile.m_oSHA1 = pDownload->m_oSHA1;
			if ( ! pFile.m_oTiger && pDownload->m_oTiger )
				pFile.m_oTiger = pDownload->m_oTiger;
			if ( ! pFile.m_oED2K && pDownload->m_oED2K )
				pFile.m_oED2K = pDownload->m_oED2K;
			if ( ! pFile.m_oMD5 && pDownload->m_oMD5 )
				pFile.m_oMD5 = pDownload->m_oMD5;

			// Get trackers
			if ( sTracker.IsEmpty() && pDownload->IsTorrent() )
			{
				const int nCount = pDownload->m_pTorrent.GetTrackerCount();
				for ( int i = 0; i < nCount; ++i )
				{
					// ToDo: Verify live tracker?
					if ( sTracker.GetLength() ) sTracker += L"&";
					sTracker += L"tr=" + URLEncode( pDownload->m_pTorrent.GetTrackerAddress( i ) );
				}
			}
		}
		pTransfersLock.Unlock();
	}

	CSingleLock pLibraryLock( &Library.m_pSection );
	if ( SafeLock( pLibraryLock ) )
	{
		if ( const CLibraryFile* pLibraryFile = LibraryMaps.LookupFileByHash( &pFile ) )
		{
			// Refill missed hashes
			if ( ! pFile.m_oSHA1 && pLibraryFile->m_oSHA1 )
				pFile.m_oSHA1 = pLibraryFile->m_oSHA1;
			if ( ! pFile.m_oTiger && pLibraryFile->m_oTiger )
				pFile.m_oTiger = pLibraryFile->m_oTiger;
			if ( ! pFile.m_oED2K && pLibraryFile->m_oED2K )
				pFile.m_oED2K = pLibraryFile->m_oED2K;
			if ( ! pFile.m_oMD5 && pLibraryFile->m_oMD5 )
				pFile.m_oMD5 = pLibraryFile->m_oMD5;
		}
		pLibraryLock.Unlock();
	}
}

CString CURLCopyDlg::CreateMagnet(CEnvyFile& pFile)
{
	CString strURN, strTracker;

	Resolve( pFile, strTracker );

	if ( pFile.m_oTiger && pFile.m_oSHA1 )
		strURN = L"xt=urn:bitprint:" + pFile.m_oSHA1.toString() + L'.' + pFile.m_oTiger.toString();
	else if ( pFile.m_oSHA1 )
		strURN = L"xt=" + pFile.m_oSHA1.toUrn();
	else if ( pFile.m_oTiger )
		strURN = L"xt=" + pFile.m_oTiger.toUrn();

	if ( pFile.m_oED2K )
	{
		if ( ! strURN.IsEmpty() ) strURN += L"&";
		strURN += L"xt=" + pFile.m_oED2K.toUrn();
	}

	if ( pFile.m_oMD5 && ! pFile.m_oTiger && ! pFile.m_oSHA1 && ! pFile.m_oED2K )
	{
		if ( ! strURN.IsEmpty() ) strURN += L"&";
		strURN += L"xt=" + pFile.m_oMD5.toUrn();
	}

	if ( pFile.m_oBTH )
	{
		if ( ! strURN.IsEmpty() ) strURN += L"&";
		strURN += L"xt=" + pFile.m_oBTH.toUrn();
	}

	CString strMagnet = strURN;

	if ( pFile.m_nSize != 0 && pFile.m_nSize != SIZE_UNKNOWN )
	{
		if ( ! strMagnet.IsEmpty() ) strMagnet += L"&";
		strMagnet.AppendFormat( L"xl=%I64u", pFile.m_nSize );
	}

	if ( ! pFile.m_sName.IsEmpty() )
	{
		if ( ! strMagnet.IsEmpty() ) strMagnet += L"&";
		if ( ! strURN.IsEmpty() )
			strMagnet += L"dn=";
		else
			strMagnet += L"kt=";
		strMagnet += URLEncode( pFile.m_sName );
	}

	if ( ! strTracker.IsEmpty() )
	{
		if ( strMagnet.GetLength() ) strMagnet += L"&";
		strMagnet += strTracker;
	}

	strMagnet = L"magnet:?" + strMagnet;

	return strMagnet;
}

void CURLCopyDlg::OnIncludeSelf()
{
	UpdateData();

	const BOOL bIncludeSelf = ( m_wndIncludeSelf.GetCheck() == BST_CHECKED );

	m_sMagnet = CreateMagnet( m_pFile );

	CString strSelf;
	if ( bIncludeSelf )
		strSelf = m_pFile.GetURL( Network.m_pHost.sin_addr, htons( Network.m_pHost.sin_port ) );

	if ( ! strSelf.IsEmpty() )	// bIncludeSelf
		m_sMagnet += L"&xs=" + URLEncode( strSelf );
	else if ( ! m_pFile.m_sURL.IsEmpty() )
		m_sMagnet += L"&xs=" + URLEncode( m_pFile.m_sURL );

	//if ( m_pFile.m_oSHA1 )
	//{
	//	m_sGnutella.Format( L"gnutella://%s/", (LPCTSTR)m_pFile.m_oSHA1.toUrn() );
	//
	//	if ( ! m_pFile.m_sName.IsEmpty() )
	//		m_sGnutella += URLEncode( m_pFile.m_sName ) + L"/";
	//}

	if ( m_pFile.m_oSHA1 )
		m_sHash = m_pFile.m_oSHA1.toUrn();
	else if ( m_pFile.m_oED2K )
		m_sHash = m_pFile.m_oED2K.toUrn();
	else
		m_sHash.Empty();

	if ( ! m_sHash.IsEmpty() )
	{
		if ( ! m_pFile.m_sName.IsEmpty() )
			m_sHash.Format( L"%s	(%I64u)	%s", (LPCTSTR)m_sHash, m_pFile.m_nSize, (LPCTSTR)m_pFile.m_sName );
		else if ( m_pFile.m_nSize && m_pFile.m_nSize != SIZE_UNKNOWN )
			m_sHash.Format( L"%s	(%I64u)", (LPCTSTR)m_sHash, m_pFile.m_nSize );
	}

	if ( m_pFile.m_oED2K &&
		 m_pFile.m_nSize != 0 &&
		 m_pFile.m_nSize != SIZE_UNKNOWN &&
		! m_pFile.m_sName.IsEmpty() )
	{
		m_sED2K.Format( L"ed2k://|file|%s|%I64u|%s|/",
			(LPCTSTR)URLEncode( m_pFile.m_sName ),
			m_pFile.m_nSize,
			(LPCTSTR)m_pFile.m_oED2K.toString() );

		if ( bIncludeSelf )
			m_sED2K += L"|sources," + HostToString( &Network.m_pHost ) + L"|/";

			//CString strURL;	// Obsolete
			//strURL.Format( L"%s:%i", (LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ), htons( Network.m_pHost.sin_port ) );
			//m_sED2K += L"|sources," + strURL + L"|/";
	}
	else
	{
		m_sED2K.Empty();
	}

	if ( bIncludeSelf )
		m_sHost = strSelf;
	else if ( ! m_pFile.m_sURL.IsEmpty() )
		m_sHost = m_pFile.m_sURL;
	else
		m_sHost.Empty();

	UpdateData( FALSE );
}

HBRUSH CURLCopyDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( pWnd && pWnd != &m_wndMessage )
	{
		TCHAR szName[32];
		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( _tcsicmp( szName, L"Static" ) == 0 )
		{
			pDC->SetTextColor( Colors.m_crTextLink );
			pDC->SelectObject( &theApp.m_gdiFontLine );
		}
	}

	return hbr;
}

BOOL CURLCopyDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );

	for ( pWnd = GetWindow( GW_CHILD ); pWnd; pWnd = pWnd->GetNextWindow() )
	{
		TCHAR szName[32];
		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( _tcsicmp( szName, L"Static" ) == 0 && pWnd != &m_wndMessage )
		{
			CRect rc;
			pWnd->GetWindowRect( &rc );

			if ( rc.PtInRect( point ) )
			{
				CString strText;
				pWnd->GetWindowText( strText );

				if ( ! strText.IsEmpty() )
				{
					SetCursor( theApp.LoadCursor( IDC_HAND ) );
					return TRUE;
				}
			}
		}
	}

	return CSkinDialog::OnSetCursor( pWnd, nHitTest, message );
}

// Use theApp.SetClipboard( string )

void CURLCopyDlg::OnStnClickedUrlMagnet()
{
	UpdateData();

	if ( m_sMagnet.IsEmpty() )
		return;

	theApp.SetClipboard( m_sMagnet, TRUE );

	CSkinDialog::OnOK();
}

void CURLCopyDlg::OnStnClickedUrlEd2k()
{
	UpdateData();

	if ( m_sED2K.IsEmpty() )
		return;

	theApp.SetClipboard( m_sED2K, TRUE );

	CSkinDialog::OnOK();
}

void CURLCopyDlg::OnStnClickedUrlHost()
{
	UpdateData();

	if ( m_sHost.IsEmpty() )
		return;

	theApp.SetClipboard( m_sHost, TRUE );

	CSkinDialog::OnOK();
}

void CURLCopyDlg::OnStnClickedUrlHash()
{
	UpdateData();

	if ( m_sHash.IsEmpty() )
		return;

	theApp.SetClipboard( m_sHash, TRUE );

	CSkinDialog::OnOK();
}
