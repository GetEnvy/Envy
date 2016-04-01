//
// EnvyThread.h
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


#ifdef _DEBUG
	#define MS_VC_EXCEPTION	0x406D1388
	#define ALMOST_INFINITE	INFINITE
#else // Release
	#define ALMOST_INFINITE	20000	// 20s Thread Timeout
#endif


class CEnvyThread : public CWinThread
{
	DECLARE_DYNAMIC(CEnvyThread)

public:
	CEnvyThread(AFX_THREADPROC pfnThreadProc = NULL, LPVOID pParam = NULL);
	virtual ~CEnvyThread();

public:
	virtual HANDLE CreateThread(LPCSTR pszName, int nPriority, DWORD dwCreateFlags, UINT nStackSize, LPSECURITY_ATTRIBUTES lpSecurityAttrs, DWORD* pnThreadID);
	virtual BOOL InitInstance();
	virtual int Run();

	static void Add(CEnvyThread* pThread, LPCSTR pszName);
	static void Remove(DWORD nThreadID);
	static HANDLE GetHandle(DWORD nThreadID);
	static bool IsThreadAlive(DWORD nThreadID);
	static bool SetThreadPriority(DWORD nThreadID, int nPriority);
	static void DeleteThread(DWORD nThreadID);
	static void DetachThread(DWORD nThreadID);
	static HANDLE BeginThread(LPCSTR pszName, AFX_THREADPROC pfnThreadProc,
							  LPVOID pParam, int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
							  DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL, DWORD* pnThreadID = NULL);
	static void CloseThread(DWORD nThreadID, DWORD dwTimeout = ALMOST_INFINITE);

protected:
	typedef struct
	{
		CEnvyThread*	pThread;	// Thread object
		LPCSTR			pszName;	// Thread name
	} CThreadTag;

	typedef CMap< DWORD, DWORD, CThreadTag, const CThreadTag& > CThreadMap;

	static CCriticalSection	m_ThreadMapSection;	// Guarding of m_ThreadMap
	static CThreadMap		m_ThreadMap;		// Map of running threads
	AFX_THREADPROC			m_pfnThreadProcExt;
	LPDWORD					m_pnOwnerThreadID;

private:
	CEnvyThread(const CEnvyThread&);
	CEnvyThread& operator=(const CEnvyThread&);
};
