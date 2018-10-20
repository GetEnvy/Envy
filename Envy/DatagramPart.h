//
// DatagramPart.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2006 and PeerProject 2008-2014
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

#pragma once

class CBuffer;
class CG2Packet;


class CDatagramOut
{
public:
	CDatagramOut();
	~CDatagramOut();

public:
	CDatagramOut*	m_pNextHash;
	CDatagramOut**	m_pPrevHash;
	CDatagramOut*	m_pNextTime;
	CDatagramOut*	m_pPrevTime;

	SOCKADDR_IN		m_pHost;
	WORD			m_nSequence;
	CBuffer*		m_pBuffer;
	LPVOID			m_pToken;
	DWORD			m_tSent;
	BOOL			m_bAck;

protected:
	BOOL			m_bCompressed;
	DWORD			m_nPacket;
	BYTE			m_nCount;
	BYTE			m_nAcked;
	BYTE			m_nLocked;
	DWORD*			m_pLocked;

public:
	void	Create(const SOCKADDR_IN* pHost, CG2Packet* pPacket, WORD nSequence, CBuffer* pBuffer, BOOL bAck);
	BOOL	GetPacket(DWORD tNow, BYTE** ppPacket, DWORD* pnPacket, BOOL bResend);
	BOOL	Acknowledge(BYTE nPart);
};
