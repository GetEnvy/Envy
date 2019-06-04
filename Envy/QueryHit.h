//
// QueryHit.h
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

#include "EnvyFile.h"
#include "VendorCache.h"
#include "Schema.h"

class CG1Packet;
class CG2Packet;
class CEDPacket;
class CDCPacket;
class CMatchFile;
class CQuerySearch;
class CXMLElement;

class CQueryHit : public CEnvyFile
{
public:
	CQueryHit(PROTOCOLID nProtocol, const Hashes::Guid& oSearchID = Hashes::Guid());
	CQueryHit(const CQueryHit& pHit);
	virtual ~CQueryHit();

public:
	CQueryHit*		m_pNext;
	Hashes::Guid	m_oSearchID;

	PROTOCOLID		m_nProtocol;
	Hashes::Guid	m_oClientID;
	IN_ADDR			m_pAddress;
	CString			m_sCountry;
	WORD			m_nPort;
	DWORD			m_nSpeed;
	CString			m_sSpeed;
	CVendorPtr		m_pVendor;
	TRISTATE		m_bPush;
	TRISTATE		m_bBusy;
	TRISTATE		m_bStable;
	TRISTATE		m_bMeasured;
	BOOL			m_bChat;
	BOOL			m_bBrowseHost;
	CString			m_sNick;

	int				m_nGroup;
	DWORD			m_nIndex;
	BOOL			m_bSize;
	DWORD			m_nPartial;
	BOOL			m_bPreview;
	CString			m_sPreview;
	int				m_nUpSlots;
	int				m_nUpQueue;
	BOOL			m_bCollection;

	CSchemaPtr		m_pSchema;
	CXMLElement*	m_pXML;
	CString			m_sComments;
	int				m_nRating;

	BOOL			m_bBogus;
	BOOL			m_bMatched;
	BOOL			m_bExactMatch;
	BOOL			m_bFiltered;
	BOOL			m_bDownload;
	BOOL			m_bSelected;
	BOOL			m_bNew;
protected:
	BOOL			m_bResolveURL;
	DWORD			m_nHitSources;

// Static Decode Operations
public:
	static CQueryHit*	FromG1Packet(CG1Packet* pPacket, int* pnHops = NULL);
	static CQueryHit*	FromG2Packet(CG2Packet* pPacket, int* pnHops = NULL);
	static CQueryHit*	FromEDPacket(CEDPacket* pPacket, const SOCKADDR_IN* pServer, BOOL bUnicode, const Hashes::Guid& pSearchID = Hashes::Guid());
	static CQueryHit*	FromDCPacket(CDCPacket* pPacket);
protected:
	static CXMLElement*	ReadXML(CG1Packet* pPacket, int nSize);

public:
	CQueryHit&	operator=(const CQueryHit& pOther);
	void		Delete();
	int			GetRating();
	void		Serialize(CArchive& ar, int nVersion);	// MATCHLIST_SER_VERSION
	void		Ban(int nBanLength);	// Ban by host IP only
	void		Resolve();
	void		ReadEDPacket(CEDPacket* pPacket, const SOCKADDR_IN* pServer, BOOL bUnicode);
protected:
	void		ParseAttributes(const Hashes::Guid& pClientID, CVendorPtr pVendor, BYTE* nFlags, BOOL bChat, BOOL bBrowseHost);
	void		ReadG1Packet(CG1Packet* pPacket);
	void		ReadGGEP(CG1Packet* pPacket);
	BOOL		CheckValid() const;
	void		ReadG2Packet(CG2Packet* pPacket, DWORD nLength);
	void		ReadEDAddress(CEDPacket* pPacket, const SOCKADDR_IN* pServer);
	BOOL		ParseXML(CXMLElement* pXML, DWORD nRealIndex);

// Inlines
public:
	inline DWORD GetSources() const
	{
		return m_nHitSources;
	}
	inline BOOL IsRated() const
	{
		return ( m_nRating || ! m_sComments.IsEmpty() );
	}
};

#define HITEQUALS_NOT		0
#define HITEQUALS_SIMILAR	1
#define HITEQUALS_IDENTICAL	2
