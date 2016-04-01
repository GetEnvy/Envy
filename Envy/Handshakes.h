//
// Handshakes.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2007
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

#pragma once

#include "ThreadImpl.h"

class CHandshake;


class CHandshakes :
	public CThreadImpl
{
public:
	CHandshakes();
	virtual ~CHandshakes();

public:
	BOOL Listen();							// Listen on the socket
	void Disconnect();						// Stop listening

	BOOL PushTo(IN_ADDR* pAddress, WORD nPort, DWORD nIndex = 0);	// Connect to the given IP
	BOOL IsConnectedTo(const IN_ADDR* pAddress) const;				// Looks for the IP in the handshake objects list

protected:
	DWORD m_nStableCount;					// The number of connections our listening socket has received
	DWORD m_tStableTime;					// The time at least one has been connected (do)
	SOCKET m_hSocket;						// Our one listening socket
	CList< CHandshake* > m_pList;			// The list of pointers to CHandshake objects
	mutable CMutex m_pSection;				// Use to make sure only one thread accesses the list at a time

	void Add(CHandshake* pHandshake);		// Add a CHandshake object to the list
	void Remove(CHandshake* pHandshake);	// Remove a CHandshake object from the list

	void OnRun();							// Accept incoming connections from remote computers
	void RunHandshakes();					// Send and receive data with each remote computer in the list
	void RunStableUpdate();					// Update the discovery services (do)
	BOOL AcceptConnection();				// Accept a connection, making a new CHandshake object in the list for it
//	void CreateHandshake(SOCKET hSocket, SOCKADDR_IN* pHost); // Make the new CHandshake object for the new connection

	// Tell WSAAccept if we want to accept a connection from a computer that just called us
	static int CALLBACK AcceptCheck(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN OUT LPQOS lpSQOS, IN OUT LPQOS lpGQOS, IN LPWSABUF lpCalleeId, IN LPWSABUF lpCalleeData, OUT GROUP FAR * g, IN DWORD_PTR dwCallbackData);

public:
	// True if the socket is valid, false if its closed
	inline BOOL IsValid() const throw()
	{
		return ( m_hSocket != INVALID_SOCKET );
	}

	// The time at least one has been connected (seconds)
	inline DWORD GetStableTime() const
	{
		if ( ! m_tStableTime ) return 0;
		const DWORD tNow = static_cast< DWORD >( time( NULL ) );
		if ( tNow < m_tStableTime ) return 0;
		return tNow - m_tStableTime;
	}
};

extern CHandshakes Handshakes;
