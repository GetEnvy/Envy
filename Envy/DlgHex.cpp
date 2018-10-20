//
// DlgHex.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2014 and PeerProject 2014
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
// Debug-only

#include "StdAfx.h"
#include "Envy.h"
#include "DlgHex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNAMIC(CHexDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CHexDlg, CSkinDialog)
END_MESSAGE_MAP()

CHexDlg::CHexDlg(CWnd* pParent /*=NULL*/)
	: CSkinDialog	( CHexDlg::IDD, pParent )
{
}

CHexDlg::~CHexDlg()
{
}

void CHexDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_HEX, m_sHex );
}

BOOL CHexDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( L"CHexDlg", IDR_MAINFRAME );

	return TRUE;
}

void CHexDlg::OnOK()
{
	UpdateData();

	m_pBuffer.Clear();

	m_sHex.Remove( L' ' );
	m_sHex.Remove( L'\t' );
	m_sHex.Remove( L'\r' );
	m_sHex.Remove( L'\n' );

	const int length = m_sHex.GetLength();

	if ( ! length )
	{
		MsgBox( L"Empty string!" );
		return;
	}

	if ( ( length & 1 ) != 0 )
	{
		MsgBox( L"String has wrong even length." );
		return;
	}

	m_pBuffer.EnsureBuffer( length / 2 );

	for ( int i = 0; i < length; ++i )
	{
		BYTE b = 0;
		TCHAR c = m_sHex.GetAt( i );
		if ( c >= '0' && c <= '9' )
			b = BYTE( ( c - '0' ) << 4 );
		else if ( c >= 'A' && c <= 'F' )
			b = BYTE( ( c - 'A' + 10 ) << 4 );
		else if ( c >= 'a' && c <= 'f' )
			b = BYTE( ( c - 'a' + 10 ) << 4 );
		else
		{
			CString strMsg;
			strMsg.Format( L"Found wrong symbol at position: %d", i );
			MsgBox( strMsg );
			return;
		}
		++i;
		c = m_sHex.GetAt( i );
		if ( c >= '0' && c <= '9' )
			b |= BYTE( c - '0' );
		else if ( c >= 'A' && c <= 'F' )
			b |= BYTE( c - 'A' + 10 );
		else if ( c >= 'a' && c <= 'f' )
			b |= BYTE( c - 'a' + 10 );
		else
		{
			CString strMsg;
			strMsg.Format( L"Found wrong symbol at position: %d", i );
			MsgBox( strMsg );
			return;
		}

		m_pBuffer.Add( &b, 1 );
	}

	CSkinDialog::OnOK();
}
