//
// ThreadImpl.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2008
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

#include "Envy.h"
#include "EnvyThread.h"


class CThreadImpl
{
public:
	CThreadImpl()
		: m_pCancel		( FALSE, TRUE )
		, m_nThreadID	( 0 )
		, m_bCancelled	( FALSE )
	//	, m_bCompleted	( false )
	{
	}

	virtual ~CThreadImpl()
	{
		CEnvyThread::DetachThread( m_nThreadID );
	}

private:
	DWORD			m_nThreadID;	// Thread ID
	CEvent			m_pWakeup;		// Thread wakeup event (optional)
	CEvent			m_pCancel;		// Thread cancel event (signaled if abort requested)
	volatile LONG	m_bCancelled;	// Thread is canceling
//	volatile bool	m_bCompleted;	// TRUE - thread runs at least once

	static UINT ThreadStart(LPVOID pParam)
	{
		CThreadImpl* pThis = reinterpret_cast< CThreadImpl* >( pParam );
		pThis->OnRun();
	//	pThis->m_bCompleted = true;	// Set complete status
		return 0;
	}

	CThreadImpl(const CThreadImpl&);
	CThreadImpl& operator=(const CThreadImpl&);

protected:
	virtual void OnRun() = 0;

public:
	inline bool BeginThread(LPCSTR szName = NULL, int nPriority = THREAD_PRIORITY_NORMAL) throw()
	{
		if ( IsThreadAlive() )
			return true;

	//	m_bCompleted = false;		// Reset complete status
		m_pCancel.ResetEvent();		// Enable thread run
		return ( CEnvyThread::BeginThread( szName, ThreadStart, this, nPriority, 0, 0, NULL, &m_nThreadID ) != NULL );
	}

	inline void CloseThread(DWORD dwTimeout = ALMOST_INFINITE) throw()
	{
		m_pCancel.SetEvent();	// Ask thread for exit
		m_pWakeup.SetEvent();	// Wakeup thread if any
		if ( ! InterlockedCompareExchange( &m_bCancelled, TRUE, FALSE ) )
		{
			if ( m_nThreadID != GetCurrentThreadId() )
			{
				CEnvyThread::CloseThread( m_nThreadID, dwTimeout );
				m_nThreadID = 0;
			}
			InterlockedExchange( &m_bCancelled, FALSE );
		}
	}

	inline void Wait() throw()
	{
		if ( ! InterlockedCompareExchange( &m_bCancelled, TRUE, FALSE ) )
		{
			if ( m_nThreadID != GetCurrentThreadId() )
			{
				CEnvyThread::CloseThread( m_nThreadID, INFINITE );
				m_nThreadID = 0;
			}
			InterlockedExchange( &m_bCancelled, FALSE );
		}
	}

	inline bool Wakeup() throw()
	{
		return ( m_pWakeup.SetEvent() != FALSE );
	}

	inline void Doze(DWORD dwTimeout) throw()	// = INFINITE
	{
		SwitchToThread();
		do
		{
			SafeMessageLoop();
		}
		while ( MsgWaitForMultipleObjects( 1, &m_pWakeup.m_hObject, FALSE, dwTimeout, QS_ALLINPUT | QS_ALLPOSTMESSAGE ) == WAIT_OBJECT_0 + 1 );
	}

	inline HANDLE GetWakeupEvent() const throw()
	{
		return m_pWakeup;
	}

	inline bool IsThreadEnabled(DWORD dwTimeout = 0) const throw()
	{
		return ( WaitForSingleObject( m_pCancel, dwTimeout ) == WAIT_TIMEOUT );
	}

	inline bool IsThreadAlive() const throw()
	{
		return CEnvyThread::IsThreadAlive( m_nThreadID );
	}

	inline void Exit() throw()
	{
		m_pCancel.SetEvent();
	}

	inline bool SetThreadPriority(int nPriority) throw()
	{
		return CEnvyThread::SetThreadPriority( m_nThreadID, nPriority );
	}
};
