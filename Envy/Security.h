//
// Security.h
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

#include "ThreadImpl.h"		// CListLoader

class CEnvyFile;

class CLiveList;
class CSecureRule;
class CQuerySearch;
class CXMLElement;


// Set Column Order
enum {
	COL_SECURITY_CONTENT,
	COL_SECURITY_HITS,
	COL_SECURITY_NUM,
	COL_SECURITY_ACTION,
	COL_SECURITY_EXPIRES,
	COL_SECURITY_TYPE,
	COL_SECURITY_COMMENT,
	COL_SECURITY_LAST	// Column Count
};

enum {
	banSession,
	ban5Mins,
	ban30Mins,
	ban2Hours,
	banWeek,
	banCustom,
	banForever
};

enum {
	urnSHA,
	urnTiger,
	urnBTH,
	urnED2K,
	urnMD5,
	urnLast
};


class CSecurity
{
public:
	CSecurity();
	~CSecurity();

public:
	mutable CCriticalSection	m_pSection;
	BOOL						m_bDenyPolicy;

	static LPCTSTR				xmlns;

protected:
	typedef struct
	{
		DWORD	m_nExpire;
		BYTE	m_nScore;
	} CComplain;

	typedef CMap< DWORD, DWORD, CComplain*, CComplain* > CComplainMap;
	typedef std::map< DWORD, BYTE > AddressMap;
	typedef std::map< CString, BYTE > HashMap;
	typedef std::map< BYTE, CSecureRule* > RuleIndexMap;

	CComplainMap				m_Complains;
	AddressMap					m_AddressMap;		// Consolidated single-IP filters (in reverse byte order)
	HashMap						m_HashMap[urnLast];	// Consolidated blacklist filters (raw hashstrings, by enum)
	RuleIndexMap				m_pRuleIndexMap;	// Applicable rule to index byte (memory efficiency)
//	std::vector< CSecureRule* >	m_pRuleIndex;		// Alt applicable rule to map index byte (memory efficiency)
	std::set< DWORD >			m_Cache;			// Known good addresses
	CList< CSecureRule* >		m_pRules;

public:
	INT_PTR			GetCount() const;
	POSITION		GetIterator() const;
	CSecureRule*	GetNext(POSITION& pos) const;
	BOOL			Check(CSecureRule* pRule) const;
	void			Add(CSecureRule* pRule);
	void			Remove(CSecureRule* pRule);
	void			MoveUp(CSecureRule* pRule);
	void			MoveDown(CSecureRule* pRule);

	void			Ban(const CEnvyFile* pFile, int nBanLength, BOOL bMessage = TRUE /*, LPCTSTR szComment = NULL*/);
	void			Ban(const IN_ADDR* pAddress, int nBanLength, BOOL bMessage = TRUE, LPCTSTR szComment = NULL);

	bool			Complain(const IN_ADDR* pAddress, int nBanLength = ban5Mins, int nExpire = 15, int nCount = 3);
	BOOL			IsDenied(const IN_ADDR* pAddress);
	BOOL			IsDenied(LPCTSTR pszContent);
	BOOL			IsDenied(const CEnvyFile* pFile);
	BOOL			IsDenied(const CQuerySearch* pQuery, const CString& strContent);
	BOOL			IsFlood(const IN_ADDR* pAddress, const LPCTSTR pszVendor = NULL, PROTOCOLID nProtocol = PROTOCOL_NULL);
	BOOL			Import(LPCTSTR pszFile);
	BOOL			Load();
	BOOL			Save();
	void			Clear();
	void			Expire();
	CLiveList*		GetList() const;	// Creates new CLiveList object filled by all security rules

	BYTE			SetRuleIndex(CSecureRule* pRule);
	CSecureRule*	GetRuleByIndex(BYTE nIndex);
	void			SetHashMap(CString sURN, BYTE nIndex);
	BYTE			GetHashMap(CString sURN);
	inline void		SetAddressMap(DWORD nIP, BYTE nIndex)
	{
		m_AddressMap[ nIP ] = nIndex;
	}
	inline BYTE		GetAddressMap(DWORD nIP)
	{
		return m_AddressMap.count( nIP ) ? m_AddressMap[ nIP ] : 0;
	}

	// Don't ban GPL breakers, but don't offer leaf slots to them. Ban others.
	BOOL			IsClientBad(const CString& sUserAgent) const;
	BOOL			IsClientBanned(const CString& sUserAgent);
	BOOL			IsAgentBlocked(const CString& sUserAgent) const;	// User-defined names
	BOOL			IsVendorBlocked(const CString& sVendor) const;		// G1/G2 code

protected:
	CSecureRule*	NewBanRule(int nBanLength = 0, CString sComment = L"") const;
	CSecureRule*	GetGUID(const GUID& oGUID) const;
	CXMLElement*	ToXML(BOOL bRules = TRUE);
	BOOL			FromXML(const CXMLElement* pXML);
	void			Serialize(CArchive& ar);

	friend class CListLoader;
};


// An adult filter class, used in searches, chat, etc
class CAdultFilter
{
public:
	CAdultFilter();
	~CAdultFilter();

private:
	LPTSTR		m_pszBlockedWords;					// Definitely adult content
	LPTSTR		m_pszDubiousWords;					// Possibly adult content
	LPTSTR		m_pszChildWords;					// Words related to child ponography

public:
	void		Load();
	BOOL		Censor(CString& sText) const;		// Censor (hide) bad words from a string
	BOOL		IsHitAdult(LPCTSTR) const;			// Does this search result have adult content?
	BOOL		IsChildPornography(LPCTSTR) const;	// Word combination indicates underage
	BOOL		IsSearchFiltered(LPCTSTR) const;	// Check if search is filtered
	BOOL		IsChatFiltered(LPCTSTR) const;		// Check filter for chat
private:
	BOOL		IsFiltered(LPCTSTR) const;
};


// A message filter class for chat messages. (Spam protection)
class CMessageFilter
{
public:
	CMessageFilter();
	~CMessageFilter();

private:
	LPTSTR		m_pszED2KSpam;				// Known ED2K spam phrases
	LPTSTR		m_pszFilteredPhrases;		// Known spam phrases

public:
	void		Load();
	BOOL		IsED2KSpam( LPCTSTR );		// ED2K message spam filter (ED2K only, always on)
	BOOL		IsFiltered( LPCTSTR );		// Chat message spam filter
};


// Async external blocklist handling (may take minutes)
class CListLoader : public CThreadImpl
{
public:
	CListLoader();
	virtual ~CListLoader();

protected:
//	CCriticalSection		m_pSection;
	CList< CSecureRule* >	m_pQueue;

public:
	void		AddList(CSecureRule* pRule);

protected:
	void		OnRun();
};

extern CSecurity Security;
extern CAdultFilter AdultFilter;
extern CMessageFilter MessageFilter;
extern CListLoader ListLoader;
