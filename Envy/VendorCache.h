//
// VendorCache.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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

class CVendor;
class CXMLElement;

typedef const CVendor* CVendorPtr;


class CVendorCache
{
public:
	CVendorCache();
	~CVendorCache();

public:
	CVendorPtr	m_pNull;

	// Lookup 4-bytes vendor code (ASCII without terminating null)
	inline CVendorPtr Lookup(LPCSTR pszCode) const
	{
		ASSERT( pszCode );
		if ( pszCode && pszCode[ 0 ] && pszCode[ 3 ] )
		{
			const WCHAR szCode[5] = { (WCHAR)pszCode[0], (WCHAR)pszCode[1], (WCHAR)pszCode[2], (WCHAR)pszCode[3], 0 };
			CVendorPtr pVendor;
			if ( m_pCodeMap.Lookup( szCode, pVendor ) )
				return pVendor;
		//	theApp.Message( MSG_INFO, L"Unknown Vendor Code: %s", pszCode );
		}
		return NULL;
	}

	// Lookup 4-chars vendor code (with terminating null)
	inline CVendorPtr Lookup(LPCWSTR pszCode) const
	{
		ASSERT( pszCode );
		if ( pszCode && pszCode[0] && pszCode[1] && pszCode[2] && pszCode[3] && ! pszCode[4] )
		{
			CVendorPtr pVendor;
			if ( m_pCodeMap.Lookup( pszCode, pVendor ) )
				return pVendor;

			theApp.Message( MSG_INFO, L"Unknown Vendor Code: %s", pszCode );
		}
		return NULL;
	}

	BOOL		Load(); 								// Load data from Vendors.xml
	CVendorPtr	LookupByName(LPCTSTR pszName) const;	// Lookup by code or by name
	bool		IsExtended(LPCTSTR pszCode) const;		// Is specified vendor Envy/Shareaza-powered?

protected:
	typedef CAtlMap< CString, CVendorPtr, CStringElementTraitsI< CString > > CVendorMap;

	CVendorMap	m_pCodeMap;		// Vendor code map
	CVendorMap	m_pNameMap;		// Name map

	void		Clear();
	BOOL		LoadFrom(const CXMLElement* pXML);
};


class CVendor
{
public:
	CVendor();
	CVendor(LPCTSTR pszCode);

public:
	CString		m_sCode;
	CString		m_sName;
	CString		m_sLink;
	bool		m_bChatFlag;
	bool		m_bBrowseFlag;
	bool		m_bExtended;	// Envy/Shareaza-powered

protected:
	BOOL		LoadFrom(const CXMLElement* pXML);

	friend class CVendorCache;
};

extern CVendorCache VendorCache;
