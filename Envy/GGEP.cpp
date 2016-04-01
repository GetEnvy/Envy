//
// GGEP.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2007
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
#include "Buffer.h"
#include "GGEP.h"
#include "G1Packet.h"
#include "ZLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CGGEPBlock construction

CGGEPBlock::CGGEPBlock()
	: m_pFirst	( NULL )
	, m_pLast	( NULL )
	, m_pInput	( NULL )
	, m_nInput	( 0 )
	, m_nItemCount ( 0 )
{
}

CGGEPBlock::~CGGEPBlock()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock clear

void CGGEPBlock::Clear()
{
	while ( m_pFirst )
	{
		CGGEPItem* pNext = m_pFirst->m_pNext;
		m_nItemCount--;
		delete m_pFirst;
		m_pFirst = pNext;
	}

	m_pLast = NULL;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock add

CGGEPItem* CGGEPBlock::Add(LPCTSTR pszID)
{
	if ( ! pszID || ! *pszID ) return NULL;

	CGGEPItem* pItem = new CGGEPItem( pszID );

	if ( pItem == NULL )
		return NULL;

	if ( ! m_pFirst ) m_pFirst = pItem;
	if ( m_pLast ) m_pLast->m_pNext = pItem;
	m_pLast = pItem;
	m_nItemCount++;
	return pItem;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock find

CGGEPItem* CGGEPBlock::Find(LPCTSTR pszID, DWORD nMinLength) const
{
	if ( ! pszID || ! *pszID ) return NULL;

	for ( CGGEPItem* pItem = m_pFirst ; pItem ; pItem = pItem->m_pNext )
	{
		if ( pItem->m_sID.Compare( pszID ) == 0 && pItem->m_nLength >= nMinLength )			// GGEP is Case sensitive
			return pItem;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock read helpers

BOOL CGGEPBlock::ReadFromPacket(CPacket* pPacket)
{
	m_pInput = pPacket->GetCurrent();
	m_nInput = pPacket->GetRemaining();

	BOOL bSuccess = ReadInternal();

	pPacket->Seek( m_nInput, CPacket::seekEnd );

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock read internal

BOOL CGGEPBlock::ReadInternal()
{
	// Skip packet data up to GGEP Magic byte
	while ( m_nInput && ReadByte() != GGEP_MAGIC );
	if ( ! m_nInput )
		return FALSE;	// Error: GGEP Magic byte was not found

	while ( m_nInput >= 3 )
	{
		BYTE nFlags = ReadByte();
		if ( ! ( nFlags & GGEP_HDR_IDLEN ) || ( nFlags & GGEP_HDR_RESERVE ) )
			return FALSE;	// Error: Invalid format of GGEP header

		CGGEPItem* pItem = ReadItem( nFlags );
		if ( ! pItem )
			return FALSE;

		if ( ! m_pFirst ) m_pFirst = pItem;
		if ( m_pLast ) m_pLast->m_pNext = pItem;
		m_pLast = pItem;
		m_nItemCount++;

		if ( nFlags & GGEP_HDR_LAST )
			return TRUE;	// OK. It was last item
	}

	// Error: Packet too short
	return FALSE;
}

BYTE CGGEPBlock::ReadByte()
{
	ASSERT( m_nInput );
	m_nInput--;
	return *m_pInput ++;
}

CGGEPItem* CGGEPBlock::ReadItem(BYTE nFlags)
{
	BYTE nIDLen = ( nFlags & GGEP_HDR_IDLEN );
	if ( m_nInput < nIDLen )
		return NULL;	// Error: Too short packet

	// Get GGEP ID
	TCHAR szID[ 16 ];
	for ( BYTE i = 0 ; i < nIDLen && m_nInput ; i++ )
	{
		szID[ i ] = (TCHAR)ReadByte();
		if ( szID[ i ] == 0 )
			return NULL;	// Error: Embedded zero byte
	}
	szID[ nIDLen ] = 0;

	auto_ptr< CGGEPItem > pItem( new CGGEPItem( szID ) );
	if ( ! pItem.get() )
		return NULL;	// Error: Out of memory

	// Decode GGEP data length
	for ( BYTE i = 0 ; ; i++ )
	{
		if ( ! m_nInput )
			return NULL;	// Error: Packet too short

		BYTE nDataLen = ReadByte();
		if ( nDataLen == 0 )
			return NULL;	// Error: Embedded zero byte

		pItem->m_nLength = ( pItem->m_nLength << 6 ) | ( nDataLen & GGEP_LEN_MASK );

		if ( nDataLen & GGEP_LEN_LAST )
		{
			if ( nDataLen & GGEP_LEN_MORE )
				return NULL;	// Error: Invalid format

			break;	// Last length byte
		}
		if ( ! ( nDataLen & GGEP_LEN_MORE ) )
			return NULL;	// Error: Invalid format
		if ( i == 2 )
			return NULL;	// Error: Too many data length bytes
	}

	if ( pItem->m_nLength == 0 )
		return pItem.release();	// OK. Its zero length item

	if ( m_nInput < pItem->m_nLength )
		return NULL;	// Error: Packet too short

	pItem->m_pBuffer = new BYTE[ pItem->m_nLength + 1 ];
	if ( pItem->m_pBuffer == NULL )
		return NULL;	// Error: Out of memory

	CopyMemory( pItem->m_pBuffer, m_pInput, pItem->m_nLength );
	pItem->m_pBuffer[ pItem->m_nLength ] = 0;
	m_pInput += pItem->m_nLength;
	m_nInput -= pItem->m_nLength;

	if ( ( nFlags & GGEP_HDR_COBS ) && ! pItem->Decode() )
	{
		delete [] pItem->m_pBuffer;
		pItem->m_pBuffer = NULL;

		return NULL;	// Error: COBS decode error
	}

	if ( ( nFlags & GGEP_HDR_DEFLATE ) && ! pItem->Inflate() )
	{
		delete [] pItem->m_pBuffer;
		pItem->m_pBuffer = NULL;

		return NULL;	// Error: Decompress error
	}

	return pItem.release();
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock write helpers

void CGGEPBlock::Write(CPacket* pPacket)
{
	if ( ! m_pFirst ) return;

	pPacket->WriteByte( GGEP_MAGIC );

	for ( CGGEPItem* pItem = m_pFirst ; pItem ; pItem = pItem->m_pNext )
	{
		pItem->WriteTo( pPacket );
	}

	Clear();
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem construction

CGGEPItem::CGGEPItem(LPCTSTR pszID)
	: m_pNext		( NULL )
	, m_pBuffer 	( NULL )
	, m_sID			( pszID ? pszID : L"" )
	, m_nLength 	( 0 )
	, m_nPosition	( 0 )
{
}

CGGEPItem::~CGGEPItem()
{
	delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem reading and writing

void CGGEPItem::Read(LPVOID pData, int nLength)
{
	if ( m_nPosition + (DWORD)nLength > m_nLength )
		nLength = m_nLength;
	CopyMemory( pData, m_pBuffer + m_nPosition, nLength );
	m_nPosition += nLength;
}

BYTE CGGEPItem::ReadByte()
{
	BYTE nByte;
	Read( &nByte, 1 );
	return nByte;
}

void CGGEPItem::Write(LPCVOID pData, int nLength)
{
	BYTE* pNew = new BYTE[ m_nLength + (DWORD)nLength ];

	if ( pNew == NULL )
		return;

	if ( m_pBuffer )
	{
		CopyMemory( pNew, m_pBuffer, m_nLength );
		delete [] m_pBuffer;
	}

	m_pBuffer = pNew;
	CopyMemory( m_pBuffer + m_nLength, pData, nLength );
	m_nLength += nLength;
}

void CGGEPItem::WriteByte(BYTE nValue)
{
	Write( &nValue, 1 );
}

void CGGEPItem::WriteShort(WORD nValue)
{
	Write( &nValue, 2 );
}

void CGGEPItem::WriteLong(DWORD nValue)
{
	Write( &nValue, 4 );
}

void CGGEPItem::WriteInt64(QWORD nValue)
{
	Write( &nValue, 8 );
}

void CGGEPItem::WriteUTF8(const CString& strText)
{
	CBuffer pBuffer;
	pBuffer.Print( strText, CP_UTF8 );
	Write( pBuffer.m_pBuffer, pBuffer.m_nLength );
}

void CGGEPItem::WriteVary(QWORD nValue)
{
	int nLength;
	if (      nValue & 0xff00000000000000ui64 )
		nLength = 8;
	else if ( nValue & 0x00ff000000000000ui64 )
		nLength = 7;
	else if ( nValue & 0x0000ff0000000000ui64 )
		nLength = 6;
	else if ( nValue & 0x000000ff00000000ui64 )
		nLength = 5;
	else if ( nValue & 0x00000000ff000000ui64 )
		nLength = 4;
	else if ( nValue & 0x0000000000ff0000ui64 )
		nLength = 3;
	else if ( nValue & 0x000000000000ff00ui64 )
		nLength = 2;
	else
		nLength = 1;
	Write( &nValue, nLength );
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem string conversion

CString CGGEPItem::ToString() const
{
	CString strValue;

	LPTSTR pszOut = strValue.GetBuffer( m_nLength );
	LPCSTR pszIn  = (LPCSTR)m_pBuffer;

	for ( DWORD nChar = 0 ; nChar < m_nLength ; nChar++ )
	{
		*pszOut++ = (TCHAR)*pszIn++;
	}

	strValue.ReleaseBuffer( m_nLength );
	return strValue;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem write to packet

void CGGEPItem::WriteTo(CPacket* pPacket)
{
	ASSERT( ! ( m_sID.IsEmpty() || m_sID.GetLength() > 15 ) );
	if ( m_sID.IsEmpty() || m_sID.GetLength() > 15 )
		return;

	// Create GGEP Extension Header
	BYTE nFlags = (BYTE)m_sID.GetLength();

	if ( Deflate() )
		nFlags |= GGEP_HDR_DEFLATE;

	if ( Encode() )
		nFlags |= GGEP_HDR_COBS;

	if ( m_pNext == NULL )
		nFlags |= GGEP_HDR_LAST;	// Last extension in the block

	// Flags -- 1 byte
	pPacket->WriteByte( nFlags );

	// ID -- 1-15 byte
	for ( BYTE i = 0 ; i < m_sID.GetLength() ; i++ )
		pPacket->WriteByte( (BYTE)m_sID.GetAt( i ) );

	// Length of the raw extension data -- 1-3 bytes
	if ( m_nLength & 0x3F000 )
		pPacket->WriteByte( (BYTE)( ( ( m_nLength >> 12 ) & GGEP_LEN_MASK ) | GGEP_LEN_MORE ) );

	if ( m_nLength & 0xFC0 )
		pPacket->WriteByte( (BYTE)( ( ( m_nLength >> 6 ) & GGEP_LEN_MASK ) | GGEP_LEN_MORE ) );

	// Shut off everything except the last 6 bits
	pPacket->WriteByte( (BYTE)( ( m_nLength & GGEP_LEN_MASK ) | GGEP_LEN_LAST ) );

	if ( m_pBuffer && m_nLength )
		pPacket->Write( m_pBuffer, m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem COBS encoding

BOOL CGGEPItem::Encode()
{
	if ( ! m_pBuffer || ! m_nLength )
		return FALSE;

	DWORD nLength = m_nLength;
	for ( BYTE* pIn = m_pBuffer ; nLength > 0 ; nLength--, pIn++ )
	{
		if ( *pIn == 0 )
			break;
	}
	if ( ! nLength )
		return FALSE;	// No need

	auto_array< BYTE > pOutput( new BYTE[ m_nLength * 2 ] );
	if ( ! pOutput.get() )
		return FALSE;	// Out of memory

	BYTE* pOut = pOutput.get();
	BYTE* pRange = NULL;
	DWORD nRange = 0;
	nLength = m_nLength;
	for ( BYTE* pIn = m_pBuffer ; nLength > 0 ; nLength--, pIn++ )
	{
		if ( *pIn == 0 )
		{
			if ( pRange )
			{
				ASSERT( nRange > 0 && nRange < 254 );
				*pOut++ = (BYTE)( nRange + 1 );
				CopyMemory( pOut, pRange, nRange );
				pOut += nRange;
				pRange = NULL;
				nRange = 0;
			}
			else
			{
				*pOut++ = 1;
			}
		}
		else if ( pRange )
		{
			if ( ++nRange == 254 )
			{
				*pOut++ = 255;
				CopyMemory( pOut, pRange, 254 );
				pOut += 254;
				pRange = NULL;
				nRange = 0;
			}
		}
		else
		{
			pRange = pIn;
			nRange = 1;
		}
	}

	if ( pRange )
	{
		ASSERT( nRange > 0 && nRange < 254 );
		*pOut++ = (BYTE)( nRange + 1 );
		CopyMemory( pOut, pRange, nRange );
		pOut += nRange;
		pRange = NULL;
		nRange = 0;
	}

	delete [] m_pBuffer;
	m_nLength = static_cast< DWORD >( pOut - pOutput.get() );
	m_pBuffer = pOutput.release();

	return TRUE;
}

BOOL CGGEPItem::Decode()
{
	ASSERT( m_nLength );

	// Calculate decoded data size
	const BYTE* pIn = m_pBuffer;
	DWORD nDecodedLength = 0;
	for ( DWORD nLength = m_nLength ; nLength ; )
	{
		BYTE nCode = *pIn++;
		if ( nCode == 0 )
			return FALSE;		// Invalid code
		nLength--;

		BYTE nLen = nCode - 1;
		if ( nLength < nLen )
			return FALSE;		// Too short packet

		pIn += nLen;
		nDecodedLength += nLen;
		nLength -= nLen;

		if ( nCode != 0xff )
			nDecodedLength++;	// + zero byte
	}

	auto_array< BYTE > pOutput( new BYTE[ nDecodedLength ] );
	if ( ! pOutput.get() )
		return FALSE;			// Out of memory

	// Decode
	pIn = m_pBuffer;
	BYTE* pOut = pOutput.get();
	for ( DWORD nLength = m_nLength ; nLength ; )
	{
		BYTE nCode = *pIn++;
		nLength--;

		BYTE nLen = nCode - 1;

		nLength -= nLen;
		while ( nLen-- )
			*pOut++ = *pIn++;

		if ( nCode != 0xff )
			*pOut++ = 0;
	}

	delete [] m_pBuffer;
	m_pBuffer = pOutput.release();
	m_nLength = nDecodedLength;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem compression

BOOL CGGEPItem::Deflate()
{
	if ( ! m_pBuffer || m_nLength < 45 )
		return FALSE;

	DWORD nCompressed = 0;
	auto_array< BYTE > pCompressed( CZLib::Compress( m_pBuffer, m_nLength, &nCompressed ) );

	if ( ! pCompressed.get() )
		return FALSE;

	if ( nCompressed >= m_nLength )
		return FALSE;

	delete [] m_pBuffer;
	m_pBuffer = pCompressed.release();
	m_nLength = nCompressed;

	return TRUE;
}

BOOL CGGEPItem::Inflate()
{
	ASSERT( m_pBuffer );
	ASSERT( m_nLength );

	DWORD nCompressed = 0;
	auto_array< BYTE > pCompressed( CZLib::Decompress( m_pBuffer, m_nLength, &nCompressed ) );

	if ( ! pCompressed.get() )
		return FALSE;

	delete [] m_pBuffer;
	m_pBuffer = pCompressed.release();
	m_nLength = nCompressed;

	return TRUE;
}
