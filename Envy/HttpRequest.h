//
// HttpRequest.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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

class CBuffer;


class CHttpRequest : public CThreadImpl
{
public:
	CHttpRequest();
	virtual ~CHttpRequest();

public:
	void		Clear();
	CString		GetURL() const;
	BOOL		SetURL(LPCTSTR pszURL);
	void		AddHeader(LPCTSTR pszKey, LPCTSTR pszValue);
	void		SetPostData(LPCVOID pBody, DWORD nBody);
	void		LimitContentLength(DWORD nLimit);
	void		SetNotify(HWND hWnd, UINT nMsg, WPARAM wParam = 0);
	int			GetStatusCode() const;
	bool		GetStatusSuccess() const;
	CString		GetStatusString() const;
	CString		GetHeader(LPCTSTR pszName) const;
	CString		GetResponseString(UINT nCodePage = CP_UTF8) const;
	CBuffer*	GetResponseBuffer() const;
	BOOL		InflateResponse();
	bool		Execute(bool bBackground);
	BOOL		IsPending() const;
	BOOL		IsFinished() const;
	void		EnableCookie(bool bEnable);
	void		Cancel();

// Data
public:
	CString		m_sURL;
protected:
	HINTERNET	m_hInternet;
	CString		m_sRequestHeaders;
	bool		m_bUseCookie;
	DWORD		m_nLimit;
	int			m_nStatusCode;
	CString		m_sStatusString;
//	CBuffer*	m_pPost;
	CBuffer*	m_pResponse;
	CStringIMap	m_pResponseHeaders;
	HWND		m_hNotifyWnd;
	UINT		m_nNotifyMsg;
	WPARAM		m_nNotifyParam;

	void		OnRun();

private:
	CHttpRequest(const CHttpRequest&);
	CHttpRequest& operator=(const CHttpRequest&);
};
