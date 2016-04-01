//
// SecureRule.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2012 and Shareaza 2002-2008
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


class CLiveList;
class CQuerySearch;
class CXMLElement;
class CEnvyFile;


class CSecureRule
{
public:
	CSecureRule(BOOL bCreate = TRUE);
	CSecureRule(const CSecureRule& pRule);
	CSecureRule& operator=(const CSecureRule& pRule);
	~CSecureRule();

public:
	typedef enum { srAddress, srContentAny, srContentAll, srContentRegExp, srContentHash, srSizeType, srExternal } RuleType;
	enum { srNull, srAccept, srDeny };
	enum { srIndefinite, srSession, srTimed };

	RuleType	m_nType;
	BYTE		m_nAction;
	CString		m_sComment;
	GUID		m_pGUID;
	DWORD		m_nExpire;
	DWORD		m_nToday;
	DWORD		m_nEver;
	BYTE		m_nIP[4];
	BYTE		m_nMask[4];
	TCHAR*		m_pContent;
	DWORD		m_nContentLength;

	void		Remove();
	void		Reset();
	void		MaskFix();
	BOOL		IsExpired(DWORD nNow, BOOL bSession = FALSE) const;
	BOOL		Match(const IN_ADDR* pAddress) const;
	BOOL		Match(LPCTSTR pszContent) const;
	BOOL		Match(const CEnvyFile* pFile) const;
	BOOL		Match(const CQuerySearch* pQuery, const CString& strContent) const;
	CString 	GetContentWords() const;
	void		SetContentWords(const CString& strContent);
	void		Serialize(CArchive& ar, int nVersion);
	void		ToList(CLiveList* pLiveList, int nCount, DWORD tNow) const;		// Adds new item to CLiveList object

	CXMLElement* ToXML();
	BOOL		FromXML(CXMLElement* pXML);
	BOOL		FromGnucleusString(CString& str);
	CString		ToGnucleusString() const;
};
