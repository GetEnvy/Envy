//
// ChatCore.h
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

#pragma once

#include "ThreadImpl.h"
#include "ChatSession.h"
//#include "EDClient.h"

class CEDClient;
class CPacket;


class CChatCore : public CThreadImpl
{
public:
	CChatCore();
	virtual ~CChatCore();

public:
	CMutexEx		m_pSection;

protected:
	CList< CChatSession* > m_pSessions;
	template< typename T > CChatSession* FindSession(const T* pClient, BOOL bCreate);

public:
	POSITION		GetIterator() const;
	CChatSession*	GetNext(POSITION& pos) const;
	INT_PTR			GetCount() const { return m_pSessions.GetCount(); }
	BOOL			Check(CChatSession* pSession) const;
	void			Close();
	BOOL			OnAccept(CConnection* pConnection);
	BOOL			OnPush(const Hashes::Guid& oGUID, CConnection* pConnection);

	template< typename T > void OnMessage(const T* pClient, CPacket* pPacket = NULL)
	{
		if ( ! Settings.Community.ChatEnable ||
			 ! Settings.Community.ChatAllNetworks )
			return;		// Chat disabled

		CSingleLock pLock( &m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CChatSession* pSession = FindSession< T >( pClient, TRUE ) )
			{
				pSession->OnMessage( pPacket );
			}
		}
	}

	template< typename T > void OnDropped(const T* pClient)
	{
		CSingleLock pLock( &m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CChatSession* pSession = FindSession< T >( pClient, FALSE ) )
			{
				pSession->OnDropped();
			}
		}
	}

	template< typename T > void OnAddUser(const T* pClient, CChatUser* pUser)
	{
		CSingleLock pLock( &m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CChatSession* pSession = FindSession< T >( pClient, FALSE ) )
			{
				pSession->AddUser( pUser );
				return;
			}
		}
		delete pUser;
	}

	template< typename T > void OnDeleteUser(const T* pClient, CString* pUser)
	{
		CSingleLock pLock( &m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CChatSession* pSession = FindSession< T >( pClient, FALSE ) )
			{
				pSession->DeleteUser( pUser );
				return;
			}
		}
		delete pUser;
	}

protected:
	void		Add(CChatSession* pSession);
	void		Remove(CChatSession* pSession);

	void		StartThread();
	void		OnRun();

	friend class CChatSession;
};

extern CChatCore ChatCore;
