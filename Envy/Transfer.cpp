//
// Transfer.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2010
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
#include "Transfers.h"
#include "Transfer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CTransfer construction

CTransfer::CTransfer(PROTOCOLID nProtocol)
	: CConnection		( nProtocol )
	, m_nRunCookie		( 0 )
	, m_nState			( 0 )
	, m_nLength			( SIZE_UNKNOWN )
	, m_nOffset			( SIZE_UNKNOWN )
	, m_nPosition		( 0 )
	, m_nBandwidth		( 0ul )
	, m_tRequest		( 0 )
{
}

CTransfer::~CTransfer()
{
	ASSERT( ! IsValid() );
	if ( IsValid() ) Close();
}

//////////////////////////////////////////////////////////////////////
// CTransfer operations

BOOL CTransfer::ConnectTo(const IN_ADDR* pAddress, WORD nPort)
{
	m_nState = 0;

	if ( CConnection::ConnectTo( pAddress, nPort ) )
	{
		Transfers.Add( this );
		return TRUE;
	}

	return FALSE;
}

void CTransfer::AttachTo(CConnection* pConnection)
{
	CConnection::AttachTo( pConnection );
	Transfers.Add( this );
}

void CTransfer::Close(UINT nError)
{
	Transfers.Remove( this );
	CConnection::Close( nError );
}

//////////////////////////////////////////////////////////////////////
// CTransfer HTTP headers

void CTransfer::ClearHeaders()
{
	m_pHeaderName.RemoveAll();
	m_pHeaderValue.RemoveAll();
}

BOOL CTransfer::OnHeaderLine(CString& strHeader, CString& strValue)
{
	m_pHeaderName.Add( strHeader );
	m_pHeaderValue.Add( strValue );

	return CConnection::OnHeaderLine( strHeader, strValue );
}
