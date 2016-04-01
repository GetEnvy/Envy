//
// EnvyFile.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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

#include "EnvyOM.h"

typedef CMap< CString, const CString&, FILETIME, FILETIME& > CMapStringToFILETIME;

class CEnvyFile : public CComObject
{
	DECLARE_DYNAMIC(CEnvyFile)

public:
	CEnvyFile();
	CEnvyFile(const CEnvyFile& pFile);
	CEnvyFile& operator=(const CEnvyFile& pFile);

public:
	CString				m_sName;	// Filename only
	QWORD				m_nSize;	// Size if any: (no size if equal to 0 or SIZE_UNKNOWN)
	Hashes::Sha1Hash	m_oSHA1;	// SHA1 (Base32)
	Hashes::TigerHash	m_oTiger;	// TigerTree Root Hash (Base32)
	Hashes::Ed2kHash	m_oED2K;	// ED2K (MD4, Base16)
	Hashes::Md5Hash		m_oMD5;		// MD5 (Base16)
	Hashes::BtHash		m_oBTH;		// BitTorrent Info Hash (Base32)
	CTime				m_tDate;	// Initial Time (Age)
	CString				m_sURL;		// Host if any
	CString				m_sPath;	// Usage:
									// CEnvyURL : Path part of URL
									// CLibraryFile : Local path without filename
									// CBTFile      : Relative path inside .torrent
									// CDownload    : Path of .pd file (.sd)
									// CUploadFile  : Path of requested file	// ToDo: Fix This?!

	// Returns "urn:bitprint:SHA1.TIGER" or "urn:sha1:SHA1" or "urn:tree:tiger/:TIGER" or empty string
	CString GetBitprint() const;

	// Returns any available URNs
	CString GetURN() const;
	CString GetShortURN() const;

	// Returns "sha1_SHA1", "ttr_TIGER" etc.
	CString GetFilename() const;

	// Returns "http://nAddress:nPort/uri-res/N2R?{SHA1|TIGER|ED2K|MD5|BTH}"
	CString GetURL(const IN_ADDR& nAddress, WORD nPort) const;

	// Split string of URLs delimited by commas to URL list
	bool SplitStringToURLs(LPCTSTR pszURLs, CMapStringToFILETIME& oUrls) const;

	// Are files sufficiently equal?
	bool operator==(const CEnvyFile& pFile) const;

	// Are files sufficiently unequal? (different sizes or hashes)
	bool operator!=(const CEnvyFile& pFile) const;

	// Are any hashes available?
	inline bool HasHash() const
	{
		return m_oSHA1 || m_oTiger || m_oED2K || m_oBTH || m_oMD5;
	}

	// Printable file size
	inline QWORD GetSize() const throw()
	{
		return ( ( m_nSize == SIZE_UNKNOWN ) ? 0 : m_nSize );
	}

	// Get file name suitable for searching
	virtual CString GetSearchName() const
	{
		CString sName = m_sName;
		ToLower( sName );
		return sName;
	}

// Automation
protected:
	BEGIN_INTERFACE_PART(EnvyFile, IEnvyFile)
		DECLARE_DISPATCH()
		STDMETHOD(get_Path)(BSTR FAR* psPath);
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(get_Size)(ULONGLONG FAR* pnSize);
		STDMETHOD(get_URN)(BSTR sURN, BSTR FAR* psURN);
		STDMETHOD(get_Hash)(URN_TYPE nType, ENCODING nBase, BSTR FAR* psURN);
		STDMETHOD(get_URL)(BSTR FAR* psURL);
		STDMETHOD(get_Magnet)(BSTR FAR* psMagnet);
	END_INTERFACE_PART(EnvyFile)

	DECLARE_INTERFACE_MAP()
};
