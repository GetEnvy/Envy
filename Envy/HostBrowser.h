//
// HostBrowser.h
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

#include "Transfer.h"
//#include "zlib.h"

class CG1Packet;
class CG2Packet;
class CEDPacket;
class CQueryHit;
class CGProfile;
class CLibraryFile;
class CBuffer;
class CVendor;
class CBrowseHostWnd;
class CXMLElement;


class CHostBrowser : public CTransfer
{
public:
	CHostBrowser(CBrowseHostWnd* pNotify = NULL, PROTOCOLID nProtocol = PROTOCOL_ANY, IN_ADDR* pAddress = NULL, WORD nPort = 0, BOOL bMustPush = FALSE, const Hashes::Guid& pClientID = Hashes::Guid(), const CString& sNick = CString());
	virtual ~CHostBrowser();

public:
	enum { hbsNull, hbsConnecting, hbsRequesting, hbsHeaders, hbsContent };

	CGProfile*		m_pProfile;
	IN_ADDR			m_pAddress;
	WORD			m_nPort;
	Hashes::Guid	m_oClientID;
	Hashes::Guid	m_oPushID;
	BOOL			m_bMustPush;
	DWORD			m_tPushed;
	BOOL			m_bConnect;
	int				m_nHits;
	BOOL			m_bCanChat;
	CString			m_sServer;
//	CString			m_sKey;		// ToDo: Proposed Private Key

protected:
	CVendor*		m_pVendor;
	CBuffer*		m_pBuffer;
	CString			m_sNick;
	DWORD			m_nReceived;
	BOOL			m_bNewBrowse;
	BOOL			m_bCanPush;
	BOOL			m_bDeflate;
	z_streamp		m_pInflate;

public:
	void			Serialize(CArchive& ar, int nVersion = 0);	// BROWSER_SER_VERSION
	BOOL			Browse();
	void			Stop(BOOL bCompleted = FALSE);
	BOOL			IsBrowsing() const;
	float			GetProgress() const;
	void			OnQueryHits(CQueryHit* pHits);
	BOOL			OnPush(const Hashes::Guid& oClientID, CConnection* pConnection);
	BOOL			OnNewFile(const CLibraryFile* pFile);

	virtual void	OnDropped();
	virtual BOOL	OnConnected();
	virtual BOOL	OnHeadersComplete();

protected:
	CBrowseHostWnd*	m_pNotify;

	BOOL			SendPush(BOOL bMessage);
	void			SendRequest();
	BOOL			ReadResponseLine();
	BOOL			ReadContent();
	BOOL			StreamContent();
	BOOL			StreamPacketsG1();
	BOOL			StreamPacketsG2();
	BOOL			StreamHTML();
	BOOL			OnPacket(CG1Packet* pPacket);
	BOOL			OnPacket(CG2Packet* pPacket);
	void			OnProfilePacket(CG2Packet* pPacket);
	BOOL			LoadDC(LPCTSTR pszFile, CQueryHit*& pHits);
	BOOL			LoadDCDirectory(CXMLElement* pRoot, CQueryHit*& pHits);

	virtual BOOL	OnRun();
	virtual BOOL	OnRead();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
};
