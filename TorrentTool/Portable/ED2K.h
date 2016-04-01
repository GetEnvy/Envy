//
// ED2K.h
//
// This file is part of Envy Torrent Tool (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2007
//
// Envy Torrent Tool is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Torrent Tool is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once


#include "MD4.h"

class CED2K : public CHashMD4
{
// Construction
public:
	inline	CED2K();
	inline	CED2K(const CHashMD4 &oHash);
	inline	~CED2K();
// Attributes
protected:
	CMD4		m_oComposite;
	CMD4		m_oBlock;
	DWORD		m_nBlockCount;
	DWORD		m_nBlockBytes;
// Operations
public:
	inline	void		operator =(const CHashMD4 &oHash);
	inline	void		Reset();
	inline	void		Add(LPCVOID pData, DWORD nLength);
	inline	void		Finish();

};

#define ED2K_BLOCK_SIZE		9500 * 1024

//////////////////////////////////////////////////////////////////////
// CED2K construction

inline CED2K::CED2K()
{
	Reset();
}

inline CED2K::CED2K(const CHashMD4 &oHash)
{
	memcpy( &m_b, &oHash.m_b, sizeof m_b );
}

inline CED2K::~CED2K()
{
}

inline void CED2K::operator =(const CHashMD4 &oHash)
{
	memcpy( &m_b, &oHash.m_b, sizeof m_b );
}

//////////////////////////////////////////////////////////////////////
// CED2K reset state

inline void CED2K::Reset()
{
	m_oComposite.Reset();
	m_oBlock.Reset();
	m_nBlockCount = 0;
	m_nBlockBytes = 0;
}

//////////////////////////////////////////////////////////////////////
// CED2K add data

inline void CED2K::Add(LPCVOID pData, DWORD nLength)
{
	LPBYTE pInput = (LPBYTE)pData;
	while ( nLength > 0 )
	{
		DWORD nProcess = min( nLength, ED2K_BLOCK_SIZE - m_nBlockBytes );
		m_oBlock.Add( pInput, nProcess );
		pInput += nProcess;
		nLength -= nProcess;
		m_nBlockBytes += nProcess;
		if ( m_nBlockBytes >= ED2K_BLOCK_SIZE )
		{
			ASSERT( m_nBlockBytes == ED2K_BLOCK_SIZE );
			m_oBlock.Finish();
			operator =( m_oBlock );
			m_oComposite.Add( &m_b, sizeof m_b );
			m_oBlock.Reset();
			++m_nBlockCount;
			m_nBlockBytes = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CED2K finish hash operation

inline void CED2K::Finish()
{
	if ( m_nBlockCount == 0 )
	{
		m_oBlock.Finish();
		operator =( m_oBlock );
	}
	else if ( m_nBlockBytes == 0 )
	{
		m_oBlock.Reset();
		m_oBlock.Finish();
		m_oComposite.Add( &m_oBlock.m_b, sizeof( m_oBlock.m_b ) );
		m_oComposite.Finish();
		operator =( m_oComposite );
	}
	else if ( m_nBlockCount > 1 || m_nBlockBytes )
	{
		if ( m_nBlockBytes > 0 )
		{
			m_oBlock.Finish();
			m_oComposite.Add( &m_oBlock.m_b, sizeof( m_oBlock.m_b ) );
			m_oBlock.Reset();
			++m_nBlockCount;
			m_nBlockBytes = 0;
		}
		m_oComposite.Finish();
		operator =( m_oComposite );
	}
}
