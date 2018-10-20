//
// CrawlSession.cpp
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
#include "Network.h"
#include "Datagrams.h"
#include "G2Packet.h"
#include "CrawlSession.h"

#include "Neighbours.h"
#include "Neighbour.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CCrawlSession CrawlSession;


//////////////////////////////////////////////////////////////////////
// CCrawlSession construction

CCrawlSession::CCrawlSession()
	: m_bActive ( FALSE )
{
}

CCrawlSession::~CCrawlSession()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CCrawlSession clear

void CCrawlSession::Clear()
{
	for ( POSITION pos = m_pNodes.GetHeadPosition(); pos; )
	{
		delete m_pNodes.GetNext( pos );
	}

	m_pNodes.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CCrawlSession bootstrap

void CCrawlSession::Bootstrap()
{
	CQuickLock oLock( Network.m_pSection );

	for ( POSITION pos = Neighbours.GetIterator(); pos; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		if ( pNeighbour->m_nNodeType != ntLeaf && pNeighbour->m_nProtocol == PROTOCOL_G2 )
			SendCrawl( &pNeighbour->m_pHost );
	}
}

//////////////////////////////////////////////////////////////////////
// CCrawlSession send a crawl request

void CCrawlSession::SendCrawl(SOCKADDR_IN* pHost)
{
	theApp.Message( MSG_DEBUG, L"CRAWL: Crawling host %s", (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_CRAWL_REQ, TRUE );
	pPacket->WritePacket( G2_PACKET_CRAWL_RLEAF, 0 );
	pPacket->WritePacket( G2_PACKET_CRAWL_RNAME, 0 );
	pPacket->WritePacket( G2_PACKET_CRAWL_RGPS, 0 );
	Datagrams.Send( pHost, pPacket );
}

//////////////////////////////////////////////////////////////////////
// CCrawlSession counts

int CCrawlSession::GetHubCount()
{
	int nCount = 0;

	for ( POSITION pos = m_pNodes.GetHeadPosition(); pos; )
	{
		CCrawlNode* pNode = m_pNodes.GetNext( pos );
		if ( pNode->m_nType == CCrawlNode::ntHub ) nCount ++;
	}

	return nCount;
}

int CCrawlSession::GetLeafCount()
{
	int nCount = 0;

	for ( POSITION pos = m_pNodes.GetHeadPosition(); pos; )
	{
		CCrawlNode* pNode = m_pNodes.GetNext( pos );
		if ( pNode->m_nType == CCrawlNode::ntLeaf ) nCount ++;
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CCrawlSession run

void CCrawlSession::OnRun()
{
	if ( ! m_bActive ) return;

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = m_pNodes.GetTailPosition(); pos; )
	{
		CCrawlNode* pNode = m_pNodes.GetPrev( pos );

		if ( pNode->m_nType == CCrawlNode::ntHub &&
			 pNode->m_tResponse == 0 &&
			 tNow - pNode->m_tCrawled >= 30 )
		{
			pNode->m_tCrawled = tNow;
			SendCrawl( &pNode->m_pHost );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CCrawlSession process a crawl reply

void CCrawlSession::OnCrawl(const SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	if ( ! m_bActive ) return;

	theApp.Message( MSG_DEBUG, L"CRAWL: Response from %s", (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );

	CCrawlNode* pNode = Find( &pHost->sin_addr, TRUE );

	pNode->OnCrawl( this, pPacket );

	// pNode->Dump();
}

//////////////////////////////////////////////////////////////////////
// CCrawlSession find a crawled node

CCrawlNode* CCrawlSession::Find(const IN_ADDR* pAddress, BOOL bCreate)
{
	for ( POSITION pos = m_pNodes.GetTailPosition(); pos; )
	{
		CCrawlNode* pNode = m_pNodes.GetPrev( pos );

		if ( pNode->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
			return pNode;
	}

	if ( ! bCreate ) return NULL;

	CCrawlNode* pNode = new CCrawlNode();
	pNode->m_nUnique = m_pNodes.AddTail( pNode );

	return pNode;
}


//////////////////////////////////////////////////////////////////////
// CCrawlNode construction

CCrawlNode::CCrawlNode()
{
	ZeroMemory( &m_pHost, sizeof( m_pHost ) );

	m_nType			= ntUnknown;
	m_nLeaves		= 0;
	m_nLatitude		= 0;
	m_nLongitude	= 0;

	m_tDiscovered	= static_cast< DWORD >( time( NULL ) );
	m_tCrawled		= 0;
	m_tResponse		= 0;
	m_nUnique		= 0;
}

CCrawlNode::~CCrawlNode()
{
}

//////////////////////////////////////////////////////////////////////
// CCrawlNode process a crawl reply

void CCrawlNode::OnCrawl(CCrawlSession* pSession, CG2Packet* pPacket)
{
	G2_PACKET nType;
	BOOL bCompound;
	DWORD nLength = 0;

	m_tResponse = static_cast< DWORD >( time( NULL ) );
	if ( m_tCrawled == 0 )
		m_tCrawled = m_tResponse;

	while ( pPacket->ReadPacket( nType, nLength, &bCompound ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		switch ( nType )
		{
		case G2_PACKET_SELF:
			OnNode( pSession, pPacket, nLength, parseSelf );
			break;
		case G2_PACKET_NEIGHBOUR_HUB:
			OnNode( pSession, pPacket, nLength, parseHub );
			break;
		case G2_PACKET_NEIGHBOUR_LEAF:
			OnNode( pSession, pPacket, nLength, parseLeaf );
			break;
		}

		pPacket->m_nPosition = nNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CCrawlNode process a crawl reply node

void CCrawlNode::OnNode(CCrawlSession* pSession, CG2Packet* pPacket, DWORD /*nPacket*/, int nType)
{
	SOCKADDR_IN pHost = { 0 };
	pHost.sin_family = PF_INET + 1;

	BOOL bHub = FALSE;
	int nLeafs = 0;

	CString strNick;
	float fLatitude = 0;
	float fLongitude = 0;

	G2_PACKET nInnerType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nInnerType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( nInnerType == G2_PACKET_NODE_ADDRESS && nLength >= 6 )
		{
			pHost.sin_family = PF_INET;
			pHost.sin_addr.S_un.S_addr = pPacket->ReadLongLE();
			pHost.sin_port = htons( pPacket->ReadShortBE() );
		}
		else if ( nInnerType == G2_PACKET_HUB_STATUS && nLength >= 2 )
		{
			bHub = TRUE;
			nLeafs = (int)pPacket->ReadShortBE();
		}
		else if ( nInnerType == G2_PACKET_NAME )
		{
			strNick = pPacket->ReadString( nLength );
		}
		else if ( nInnerType == G2_PACKET_GPS && nLength >= 4 )
		{
			DWORD nGPS = pPacket->ReadLongBE();
			fLatitude  = (float)HIWORD( nGPS ) / 65535.0f * 180.0f - 90.0f;
			fLongitude = (float)LOWORD( nGPS ) / 65535.0f * 360.0f - 180.0f;
		}

		pPacket->m_nPosition = nNext;
	}

	if ( pHost.sin_family != PF_INET )
		return;

	if ( nType == parseSelf )
	{
		m_pHost			= pHost;
		m_nType			= bHub ? ntHub : ntLeaf;
		m_nLeaves		= nLeafs;
		m_sNick			= strNick;
		m_nLatitude		= fLatitude;
		m_nLongitude	= fLongitude;

		theApp.Message( MSG_DEBUG, L"CRAWL: Found %s, %s(%i), \"%s\", lat: %.3f, lon: %.3f :",
			(LPCTSTR)CString( inet_ntoa( pHost.sin_addr ) ), bHub ? L"hub" : L"leaf",
			nLeafs, (LPCTSTR)strNick, double( fLatitude ), double( fLongitude ) );
	}
	else
	{
		CCrawlNode* pNode = pSession->Find( &pHost.sin_addr, TRUE );

		if ( pNode->m_tResponse == 0 )
		{
			pNode->m_pHost		= pHost;
			pNode->m_nType		= nType;
			pNode->m_nLeaves	= nLeafs;
			pNode->m_sNick		= strNick;
			pNode->m_nLatitude	= fLatitude;
			pNode->m_nLongitude	= fLongitude;
		}

		if ( m_pNeighbours.Find( pNode ) == NULL )
			m_pNeighbours.AddTail( pNode );

		theApp.Message( MSG_DEBUG, L"CRAWL:    %s, %s(%i), \"%s\", lat: %.3f, lon: %.3f",
			(LPCTSTR)CString( inet_ntoa( pHost.sin_addr ) ), bHub ? L"hub" : L"leaf",
			nLeafs, (LPCTSTR)strNick, double( fLatitude ), double( fLongitude ) );
	}
}
