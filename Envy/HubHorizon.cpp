//
// HubHorizon.cpp
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "HubHorizon.h"
#include "G2Packet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CHubHorizonPool HubHorizonPool;


//////////////////////////////////////////////////////////////////////
// CHubHorizonPool construction

CHubHorizonPool::CHubHorizonPool()
	: m_pBuffer	( NULL )
	, m_nBuffer	( 0 )
	, m_pFree	( NULL )
	, m_pActive	( NULL )
	, m_nActive	( 0 )
{
}

CHubHorizonPool::~CHubHorizonPool()
{
	if ( m_pBuffer != NULL ) delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonPool setup

void CHubHorizonPool::Setup()
{
	if ( m_pBuffer != NULL ) delete [] m_pBuffer;

	m_nBuffer	= Settings.Gnutella2.HubHorizonSize;
	m_pBuffer	= new CHubHorizonHub[ m_nBuffer ];
	m_pActive	= NULL;
	m_nActive	= 0;
	m_pFree		= m_pBuffer;

	for ( DWORD nItem = 0 ; nItem < m_nBuffer ; nItem++ )
	{
		m_pBuffer[ nItem ].m_pNext	= ( nItem < m_nBuffer - 1 )
									? &m_pBuffer[ nItem + 1 ] : NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonPool clear

void CHubHorizonPool::Clear()
{
	m_pActive	= NULL;
	m_nActive	= 0;
	m_pFree		= m_pBuffer;

	for ( DWORD nItem = 0 ; nItem < m_nBuffer ; nItem++ )
	{
		m_pBuffer[ nItem ].m_pNext	= ( nItem < m_nBuffer - 1 )
									? &m_pBuffer[ nItem + 1 ] : NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonPool add

CHubHorizonHub* CHubHorizonPool::Add(IN_ADDR* pAddress, WORD nPort)
{
	CHubHorizonHub* pHub = m_pActive;
	for ( ; pHub ; pHub = pHub->m_pNext )
	{
		if ( pHub->m_pAddress.S_un.S_addr == pAddress->S_un.S_addr )
		{
			pHub->m_nPort = nPort;
			pHub->m_nReference ++;
			return pHub;
		}
	}

	if ( m_nActive == m_nBuffer || m_pFree == NULL ) return FALSE;

	pHub = m_pFree;
	m_pFree = m_pFree->m_pNext;

	pHub->m_pNext = m_pActive;
	m_pActive = pHub;
	m_nActive ++;

	pHub->m_pAddress	= *pAddress;
	pHub->m_nPort		= nPort;
	pHub->m_nReference	= 1;

	return pHub;
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonPool remove

void CHubHorizonPool::Remove(CHubHorizonHub* pHub)
{
	CHubHorizonHub** ppPrev = &m_pActive;

	for ( CHubHorizonHub* pSeek = *ppPrev ; pSeek ; pSeek = pSeek->m_pNext )
	{
		if ( pHub == pSeek )
		{
			*ppPrev = pHub->m_pNext;
			pHub->m_pNext = m_pFree;
			m_pFree = pHub;
			m_nActive --;
			break;
		}

		ppPrev = &pSeek->m_pNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonPool find

CHubHorizonHub* CHubHorizonPool::Find(IN_ADDR* pAddress)
{
	for ( CHubHorizonHub* pHub = m_pActive ; pHub ; pHub = pHub->m_pNext )
	{
		if ( pHub->m_pAddress.S_un.S_addr == pAddress->S_un.S_addr )
			return pHub;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonPool add hubs to packet

int CHubHorizonPool::AddHorizonHubs(CG2Packet* pPacket)
{
	int nCount = 0;

	for ( CHubHorizonHub* pHub = m_pActive ; pHub ; pHub = pHub->m_pNext )
	{
		pPacket->WritePacket( G2_PACKET_HORIZON, 6 );
		pPacket->WriteLongLE( pHub->m_pAddress.S_un.S_addr );
		pPacket->WriteShortBE( pHub->m_nPort );

		theApp.Message( MSG_DEBUG, L"  Try horizon %s",
			(LPCTSTR)CString( inet_ntoa( pHub->m_pAddress ) ) );

		nCount++;
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonGroup construction

CHubHorizonGroup::CHubHorizonGroup()
{
	m_pList		= NULL;
	m_nCount	= 0;
	m_nBuffer	= 0;
}

CHubHorizonGroup::~CHubHorizonGroup()
{
	Clear();
	if ( m_pList != NULL ) delete [] m_pList;
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonGroup add

void CHubHorizonGroup::Add(IN_ADDR* pAddress, WORD nPort)
{
	CHubHorizonHub** ppHub = m_pList;

	for ( DWORD nCount = m_nCount ; nCount ; nCount--, ppHub++ )
	{
		if ( (*ppHub)->m_pAddress.S_un.S_addr == pAddress->S_un.S_addr )
		{
			(*ppHub)->m_nPort = nPort;
			return;
		}
	}

	CHubHorizonHub* pHub = HubHorizonPool.Add( pAddress, nPort );
	if ( pHub == NULL ) return;

	if ( m_nCount == m_nBuffer )
	{
		m_nBuffer += 8;
		CHubHorizonHub** pList = new CHubHorizonHub*[ m_nBuffer ];
		if ( m_pList )
		{
			if ( m_nCount ) CopyMemory( pList, m_pList, sizeof(CHubHorizonHub*) * m_nCount );
			delete [] m_pList;
		}
		m_pList = pList;
	}

	m_pList[ m_nCount++ ] = pHub;
}

//////////////////////////////////////////////////////////////////////
// CHubHorizonGroup clear

void CHubHorizonGroup::Clear()
{
	CHubHorizonHub** ppHub = m_pList;

	for ( DWORD nCount = m_nCount ; nCount ; nCount--, ppHub++ )
	{
		if ( -- ( (*ppHub)->m_nReference ) == 0 )
			HubHorizonPool.Remove( *ppHub );
	}

	m_nCount = 0;
}
