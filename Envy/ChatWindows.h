//
// ChatWindows.h
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

#pragma once

class CChatWnd;
class CPrivateChatWnd;


class CChatWindows
{
public:
	CChatWindows();
	virtual ~CChatWindows();

public:
	void				Add(CChatWnd* pFrame);
	void				Remove(CChatWnd* pFrame);
//	INT_PTR				GetCount() const { return m_pList.GetCount(); }
//	void				Close();	// Empty m_pList

	// Start new or reopen existing chat session
	CPrivateChatWnd*	OpenPrivate(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush = FALSE, PROTOCOLID nProtocol = PROTOCOL_NULL, SOCKADDR_IN* pServer = NULL);
	CPrivateChatWnd*	OpenPrivate(const Hashes::Guid& oGUID, const IN_ADDR* pAddress, WORD nPort = protocolPorts[ PROTOCOL_G2 ], BOOL bMustPush = FALSE, PROTOCOLID nProtocol = PROTOCOL_NULL, IN_ADDR* pServerAddress = NULL, WORD nServerPort = 0);		// (nPort and nServerPort must be in host byte order)
	CPrivateChatWnd*	FindPrivate(const Hashes::Guid& oGUID, bool bLive) const;
	CPrivateChatWnd*	FindPrivate(const SOCKADDR_IN* pAddress) const;
	CPrivateChatWnd*	FindED2KFrame(const SOCKADDR_IN* pAddress) const;
	CPrivateChatWnd*	FindED2KFrame(DWORD nClientID, const SOCKADDR_IN* pServerAddress) const;

private:
	CList< CChatWnd* >	m_pList;

	POSITION			GetIterator() const;
	CChatWnd*			GetNext(POSITION& pos) const;

	CPrivateChatWnd*	OpenPrivateGnutella(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, PROTOCOLID nProtocol);
	CPrivateChatWnd*	OpenPrivateED2K(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, SOCKADDR_IN* pServer);
};

extern CChatWindows ChatWindows;
