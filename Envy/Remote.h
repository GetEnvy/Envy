//
// Remote.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2016
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

#include "Buffer.h"
#include "Transfer.h"

class CMatchFile;


class CRemote : public CTransfer
{
public:
	CRemote(CConnection* pConnection);
	virtual ~CRemote();

protected:
	CString			m_sHandshake;
	CString			m_sRedirect;
	CString			m_sHeader;
	CString			m_sResponse;
	CBuffer			m_pResponse;
	CStringIMap		m_pKeys;
	static CList<int> m_pCookies;

	enum ActiveTab { tabNone, tabHome, tabDownloads, tabUploads, tabNetwork, tabSearch };
	ActiveTab		m_nTab;


public:
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();
	virtual void	OnDropped();
	virtual BOOL	OnHeadersComplete();

protected:
	CString			GetKey(LPCTSTR pszName);
	BOOL			CheckCookie();
	BOOL			RemoveCookie();
	void			Prepare(LPCTSTR pszPrefix = NULL);
	void			Add(LPCTSTR pszKey, LPCTSTR pszValue);
	void			AddText(LPCTSTR pszKey, LPCTSTR pszDefault = NULL);
	void			Output(LPCTSTR pszName);

protected:
	void			PageSwitch(const CString& strPath);
	void			PageLogin();
	void			PageLogout();
	void			PageHome();
	void			PageSearch();
	void			PageNewSearch();
	void			PageDownloads();
	void			PageNewDownload();
	void			PageUploads();
	void			PageNetwork();
	void			PageBanner(const CString& strPath);
	void			PageImage(const CString& strPath);

	void			PageSearchHeaderColumn(int nColumnID, LPCTSTR pszCaption, LPCTSTR pszAlign);
	void			PageSearchRowColumn(int nColumnID, CMatchFile* pFile, LPCTSTR pszValue, LPCTSTR pszAlign = L"center");
	void			PageNetworkNetwork(int nID, bool* pbConnect, LPCTSTR pszName);
};
