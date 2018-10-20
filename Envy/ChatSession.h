//
// ChatSession.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2012
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

#include "Connection.h"
#include "WndChat.h"

class CDCPacket;
class CDCClient;
class CEDPacket;
class CEDClient;
class CG2Packet;
class CGProfile;
class CChatWnd;
class CPrivateChatWnd;


class CChatSession : public CConnection
{
public:
	CChatSession(PROTOCOLID nProtocol = PROTOCOL_ANY, CPrivateChatWnd* pFrame = NULL);
	virtual ~CChatSession();

public:
	enum
	{
		cssNull, cssConnecting, cssRequest1, cssHeaders1, cssRequest2, cssHeaders2,
		cssRequest3, cssHeaders3, cssHandshake, cssActive, cssAway
	};

	Hashes::Guid	m_oGUID;

	CString			m_sNick;
	BOOL			m_bMustPush;
	BOOL			m_bUnicode;		// ED2K Client in UTF-8 format
	DWORD			m_nClientID;	// ED2K Client ID (if appropriate)
	SOCKADDR_IN		m_pServer;		// ED2K server (if appropriate)

public:
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = 0);
	virtual void	AddUser(CChatUser* pUser);
	virtual void	DeleteUser(CString* pUser);
	virtual void	OnDropped();

	BOOL			Connect();
	TRISTATE		GetConnectedState() const;
	void			MakeActive(BOOL bAddUsers = TRUE);
	BOOL			SendPush(BOOL bAutomatic);
	BOOL			OnPush(const Hashes::Guid& oGUID, CConnection* pConnection);
	void			OnMessage(CPacket* pPacket);

	BOOL			SendPrivateMessage(bool bAction, const CString& strText);
	void			OnOpenWindow();
	void			OnCloseWindow();
	inline bool		IsOnline() const
	{
		return ( m_nState > cssConnecting );
	}

protected:
	TRISTATE		m_bOld; 		// Chat version: TRI_TRUE = CHAT/0.1
	int				m_nState;
	DWORD			m_tPushed;
	CGProfile*		m_pProfile;
	CPrivateChatWnd* m_pWndPrivate;
	CList< MSG >	m_pMessages;	// Undelivered messages queue

	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();

	void	Command(UINT nCommand);
	void	StatusMessage(MessageType bType, UINT nID, ...);
	void	NotifyMessage(MessageType bType, const CString& sFrom, const CString& sMessage = CString(), HBITMAP hBitmap = NULL);
//	BOOL	SendAwayMessage(const CString& strText);
	void	ProcessMessages();
	void	ClearMessages();

	// G1/G2
	BOOL	ReadHandshake();
	BOOL	ReadG2();
	void	Send(CG2Packet* pPacket);
	BOOL	OnEstablished();
	BOOL	OnProfileChallenge(CG2Packet* pPacket);
	BOOL	OnProfileDelivery(CG2Packet* pPacket);
	BOOL	OnChatRequest(CG2Packet* pPacket);
	BOOL	OnChatAnswer(CG2Packet* pPacket);
	BOOL	OnChatMessage(CG2Packet* pPacket);

	BOOL	ReadG1();
	void	OnChatMessage(const CString& sFrom, const CString& sMessage);

	// DC++
	BOOL	ReadDC();
	BOOL	SendDC();
	BOOL	Send(CDCPacket* pPacket);
	BOOL	OnChatMessage(CDCPacket* pPacket);

	// ED2K
	BOOL	ReadED2K();
	BOOL	SendED2K();
	BOOL	Send(CEDPacket* pPacket);
	BOOL	OnChatMessage(CEDPacket* pPacket);
	BOOL	OnCaptchaRequest(CEDPacket* pPacket);
	BOOL	OnCaptchaResult(CEDPacket* pPacket);
};
