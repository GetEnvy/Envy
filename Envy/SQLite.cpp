//
// SQLite.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2008 and PeerProject 2008-2012
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
#include "SQLite.h"
#include <SQLite/sqlite3.h>	// Services

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////////////
// CDatabase

CDatabase::CDatabase(LPCTSTR szDatabase)
	: m_db			( NULL )
	, m_st			( NULL )
	, m_bBusy		( false )
#ifdef _DEBUG
	, m_nThread		( GetCurrentThreadId() )
#endif
{
	sqlite3_enable_shared_cache( 1 );

	if ( sqlite3_open16( szDatabase, &m_db ) == SQLITE_OK )
		sqlite3_busy_timeout( m_db, 200 ); // 0.2 s
}

CDatabase::~CDatabase()
{
	ASSERT( m_nThread == GetCurrentThreadId() );	// Don't pass database across thread boundaries

	Finalize();

	if ( m_db )
	{
		sqlite3_close( m_db );
		m_db = NULL;
	}
}

CDatabase::operator bool() const throw()
{
	return ( m_db != NULL );
}

CString CDatabase::GetLastErrorMessage() const
{
	return ( m_db ? CString( (LPCWSTR)sqlite3_errmsg16( m_db ) ) : CString() );
}

bool CDatabase::IsBusy() const throw()
{
	return m_bBusy;
}

int CDatabase::GetCount() const throw()
{
	return (int)m_raw.GetCount();	// IN_PTR
}

bool CDatabase::Exec(LPCTSTR szQuery)
{
	ASSERT( szQuery && *szQuery );

	m_sQuery = szQuery;

	while ( PrepareHelper() )
	{
		bool ret = Step();

		Finalize();

		if ( m_sQuery.IsEmpty() )
			return ret;		// Done
	}
	return false;
}

bool CDatabase::Prepare(LPCTSTR szQuery)
{
	ASSERT( szQuery && *szQuery );

	m_sQuery = szQuery;

	return PrepareHelper();
}

bool CDatabase::PrepareHelper()
{
	ASSERT( m_nThread == GetCurrentThreadId() );	// Don't pass database across thread boundaries
	ASSERT( m_db );

	if ( ! m_db )
		return false;	// Database open error

	Finalize();

	for ( ;; )
	{
		LPCWSTR pszTail = NULL;
		CT2CW szQueryW( m_sQuery );
		switch ( sqlite3_prepare16_v2( m_db, (LPCVOID)(LPCWSTR)szQueryW, -1, &m_st, (LPCVOID*)&pszTail ) )
		{
		case SQLITE_OK:
			if ( pszTail && *pszTail )
				m_sQuery = pszTail;
			else
				m_sQuery.Empty();

			if ( m_st )
				return true;

			// This happens for a comment or white-space
			break;

		case SQLITE_BUSY:
			m_bBusy = true;
			return false;

		default:
			return false;
		}
	}
}

void CDatabase::Finalize()
{
	ASSERT( m_nThread == GetCurrentThreadId() );	// Don't pass database across thread boundaries

	if ( m_st )
	{
		sqlite3_finalize( m_st );
		m_st = NULL;
	}

	m_bBusy = false;
	m_raw.RemoveAll();
}

bool CDatabase::Step()
{
	ASSERT( m_nThread == GetCurrentThreadId() );	// Don't pass database across thread boundaries
	ASSERT( m_st );

	if ( ! m_st )
		return false;

	m_bBusy = false;
	m_raw.RemoveAll();

	switch ( sqlite3_step( m_st ) )
	{
	case SQLITE_ROW:
		break;

	case SQLITE_BUSY:
		m_bBusy = true;
		return false;

	case SQLITE_DONE:
		Finalize();
		return true;

	//case SQLITE_ERROR:
	default:
		// To get good error message
		sqlite3_reset( m_st );
		return false;
	}

	// Save column names
	int count = sqlite3_data_count( m_st );
	for ( int i = 0; i < count; i++ )
	{
		m_raw.SetAt( (LPCTSTR)CW2CT( (LPCWSTR)sqlite3_column_name16( m_st, i ) ), i );
	}
	return true;
}

int CDatabase::GetColumn(LPCTSTR szName) const
{
	int column;
	return ( m_st &&
		m_raw.Lookup( szName, column ) &&
		column >= 0 &&
		column < sqlite3_data_count( m_st ) ) ? column : -1;
}

int CDatabase::GetType(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
		return sqlite3_column_type( m_st, column );

	return 0;
}

__int32 CDatabase::GetInt32(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
		return sqlite3_column_int( m_st, column );

	return 0;
}

__int64 CDatabase::GetInt64(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
		return sqlite3_column_int64( m_st, column );

	return 0;
}

double CDatabase::GetDouble(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
		return sqlite3_column_double( m_st, column );

	return 0;
}

CString CDatabase::GetString(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
		return (LPCWSTR)sqlite3_column_text16( m_st, column );

	return CString();
}

LPCVOID CDatabase::GetBlob(LPCTSTR pszName, int* pnLength) const
{
	if ( pnLength )
		*pnLength = 0;
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		if ( pnLength )
			*pnLength = sqlite3_column_bytes( m_st, column );
		return sqlite3_column_blob( m_st, column );
	}
	return NULL;
}

bool CDatabase::Bind(int nIndex, __int32 nData)
{
	return m_st && sqlite3_bind_int( m_st, nIndex, nData ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, __int64 nData)
{
	return m_st && sqlite3_bind_int64( m_st, nIndex, nData ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, double dData)
{
	return m_st && sqlite3_bind_double( m_st, nIndex, dData ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, LPCTSTR szData)
{
	CT2CW szDataW( szData );
	return m_st && sqlite3_bind_text16( m_st, nIndex, (LPVOID)(LPCWSTR)szDataW,
		lstrlenW( (LPCWSTR)szDataW ) * sizeof( WCHAR ), SQLITE_STATIC ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, LPCVOID pData, int nLength)
{
	return m_st && sqlite3_bind_blob( m_st, nIndex, pData, nLength, SQLITE_STATIC ) == SQLITE_OK;
}
