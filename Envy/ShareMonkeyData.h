//
// ShareMonkeyData.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2012
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

// Note: For reference & reuse only, ShareMonkey*com does not exist
// This file is unused.

#pragma once

#include "ThreadImpl.h"
#include "MetaPanel.h"

class CXMLElement;
class CLibraryFileView;

class CShareMonkeyData :
	public CMetaPanel,
	public CThreadImpl
{
public:
	CShareMonkeyData(INT_PTR nOffset, int nRequestType = CShareMonkeyData::stProductMatch);
	virtual ~CShareMonkeyData();

protected:
	CCriticalSection	m_pSection;

	DWORD				m_nFileIndex;
	HINTERNET			m_hInternet;
	HINTERNET			m_hSession;
	HINTERNET			m_hRequest;
	DWORD				m_nDelay;
	DWORD				m_nFailures;
	CSchemaPtr			m_pSchema;
	CXMLElement*		m_pXML;
	CXMLElement*		m_pEnvyXML;
	CLibraryFileView*	m_pFileView;
	CString				m_sResponse;
	int					m_nRequestType;
	INT_PTR				m_nOffset;

public:
	CString				m_sURL;
	CString				m_sComparisonURL;
	CString				m_sBuyURL;
	CString				m_sSessionID;
	CString				m_sProductID;
	CString				m_sCategoryID;
	CString				m_sCountry;
	CString				m_sProductName;
	CString				m_sDescription;
	CString				m_sStatus;

// Operations
public:
	BOOL		Start(CLibraryFileView* pView, DWORD nFileIndex);
	void		Stop();
	CSchemaPtr	GetSchema() { return m_pSchema; }

	enum WebRequestType
	{
		stProductMatch,
		stStoreMatch,
		stComparison
	};

protected:
	void		OnRun();
	void		Clear();
	BOOL		BuildRequest();
	BOOL		ExecuteRequest();
	BOOL		DecodeResponse(CString& strMessage);
	BOOL		ImportData(CXMLElement* pRoot);

	inline bool NotifyWindow(LPCTSTR pszMessage = NULL) const;
};
