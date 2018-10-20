//
// DownloadSource.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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

#include "FileFragments.hpp"
#include "DownloadTransfer.h"
#include "Download.h"
#include "Transfers.h"

class CQueryHit;
class CEDClient;


class CDownloadSource
{
public:
	CDownloadSource(const CDownload* pDownload);
	CDownloadSource(const CDownload* pDownload, const CQueryHit* pHit);
	CDownloadSource(const CDownload* pDownload, DWORD nClientID, WORD nClientPort, DWORD nServerIP, WORD nServerPort, const Hashes::Guid& oGUID);
	CDownloadSource(const CDownload* pDownload, const Hashes::BtGuid& oGUID, const IN_ADDR* pAddress, WORD nPort);
	CDownloadSource(const CDownload* pDownload, LPCTSTR pszURL, BOOL bHashAuth = FALSE, FILETIME* pLastSeen = NULL, int nRedirectionCount = 0);
	~CDownloadSource();

private:
	CDownloadTransfer*	m_pTransfer;

	inline void Construct(const CDownload* pDownload);

public:
	CDownload*			m_pDownload;
	BOOL				m_bSelected;
	CString				m_sURL;
	PROTOCOLID			m_nProtocol;
	Hashes::Guid		m_oGUID;
	IN_ADDR				m_pAddress;
	WORD				m_nPort;
	WORD				m_nServerPort;
	IN_ADDR				m_pServerAddress;
	CString				m_sCountry;
	CString				m_sCountryName;
	CString				m_sName;
	DWORD				m_nIndex;
	BOOL				m_bHashAuth;
	BOOL				m_bSHA1;
	BOOL				m_bTiger;
	BOOL				m_bED2K;
	BOOL				m_bBTH;
	BOOL				m_bMD5;
	CString				m_sServer;
	CString				m_sNick;
	DWORD				m_nSpeed;
	FILETIME			m_tLastSeen;
	BOOL				m_bPushOnly;
	BOOL				m_bCloseConn;
	BOOL				m_bReadContent;
	BOOL				m_bClientExtended;		// Does the user support extended G2 functions? (We can use G2 chat, browse, etc...)
	int					m_nGnutella;			// Gnutella functionality:
												// 0: Pure HTTP, 1: Pure G1, 2: Pure G2, 3: Mixed G1/G2

public:
	int					m_nColor;
	DWORD				m_nSortOrder;			// How should this source be sorted in the list?
	DWORD				m_tAttempt;
	BOOL				m_bKeep;				// Source keeped by NeverDrop == TRUE flag
	DWORD				m_nFailures;			// Failure count
	DWORD				m_nBusyCount;			// Busy count (for incrementing RetryDelay)
	DWORD				m_nRedirectionCount;
	Fragments::List		m_oAvailable;
	Fragments::List		m_oPastFragments;

	CString				m_sPreview;				// If empty it has the default /gnutella/preview/v1?urn:xyz format
	BOOL				m_bPreview;				// Does the user allow previews?
	BOOL				m_bPreviewRequestSent;
	BOOL				m_bMetaIgnore;			// Ignore metadata from this source (for example already got)

public:
	BOOL		ResolveURL();
	void		Serialize(CArchive& ar, int nVersion);	// DOWNLOAD_SER_VERSION
	BOOL		CanInitiate(BOOL bNetwork, BOOL bEstablished);
	// Remove source from download, add it to failed sources if bBan == TRUE, and destroy source itself
	void		Remove(BOOL bCloseTransfer = TRUE, BOOL bBan = FALSE, DWORD nRetryAfter = 0);
	void		OnFailure(BOOL bNondestructive, DWORD nRetryAfter = 0);
	DWORD		CalcFailureDelay(DWORD nRetryAfter = 0) const;
	void		OnResume();
	void		OnResumeClosed();

	void		SetValid();
	void		SetLastSeen();
	void		SetGnutella(int nGnutella);
	BOOL		CheckHash(const Hashes::Sha1Hash& oSHA1);
	BOOL		CheckHash(const Hashes::TigerHash& oTiger);
	BOOL		CheckHash(const Hashes::Ed2kHash& oED2K);
	BOOL		CheckHash(const Hashes::BtHash& oBTH);
	BOOL		CheckHash(const Hashes::Md5Hash& oMD5);

	BOOL		PushRequest();
	BOOL		CheckPush(const Hashes::Guid& oClientID);
	BOOL		CheckDonkey(const CEDClient* pClient);

	void		AddFragment(QWORD nOffset, QWORD nLength, BOOL bMerge = FALSE);
	void		SetAvailableRanges(LPCTSTR pszRanges);
	BOOL		HasUsefulRanges() const;
	BOOL		TouchedRange(QWORD nOffset, QWORD nLength) const;
	int			GetColor();
	// Draw complete source or draw transfer bar only
	void		Draw(CDC* pDC, CRect* prcBar, COLORREF crNatural);
	void		Draw(CDC* pDC, CRect* prcBar);
	void		Close(DWORD nRetryAfter = 0);	// Close source transfer

	CDownloadTransfer*	CreateTransfer(LPVOID pParam = NULL);

	inline bool Equals(const CDownloadSource* pSource) const	// Was CDownloadSource::Equals
	{
		if ( m_oGUID.isValid() && pSource->m_oGUID.isValid() )
			return m_oGUID == pSource->m_oGUID;

		if ( m_nServerPort != pSource->m_nServerPort )
		{
			return FALSE;
		}
		else if ( m_nServerPort > 0 )	// Push
		{
			if ( m_pServerAddress.S_un.S_addr != pSource->m_pServerAddress.S_un.S_addr ) return FALSE;
			if ( m_pAddress.S_un.S_addr != pSource->m_pAddress.S_un.S_addr ) return FALSE;
		}
		else	// Direct
		{
			if ( m_pAddress.S_un.S_addr != pSource->m_pAddress.S_un.S_addr ) return FALSE;
			if ( m_nPort != pSource->m_nPort ) return FALSE;
		}

		return TRUE;
	}

	inline bool IsOnline() const
	{
		return m_nBusyCount || IsConnected();
	}

	inline bool IsHTTPSource() const
	{
		return ( m_nProtocol == PROTOCOL_HTTP || m_nProtocol == PROTOCOL_G2 );
	}

	// Source has live connection
	inline bool IsConnected() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer && m_pTransfer->m_nState > dtsConnecting );
	}

	// Source has no transfer
	inline bool IsIdle() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer == NULL );
	}

	// Source transfer state
	inline int GetState() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer ? m_pTransfer->m_nState : dtsNull );
	}

	// Source transfer protocol
	inline PROTOCOLID GetTransferProtocol() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer ? m_pTransfer->m_nProtocol : PROTOCOL_NULL );
	};

	// Source transfer speed
	inline DWORD GetMeasuredSpeed() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer ? m_pTransfer->GetMeasuredSpeed() : 0 );
	}

	// Source transfer host address
	inline CString GetAddress() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer ? m_pTransfer->m_sAddress : CString() );
	}

	// Source transfer host port (in network byte order)
	inline WORD GetPort() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer ? m_pTransfer->m_pHost.sin_port : 0 );
	}

	// Source transfer downloaded bytes
	inline QWORD GetDownloaded() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer ? m_pTransfer->m_nDownloaded : 0 );
	}

	// Source transfer state
	inline CString GetState(BOOL bLong) const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( m_pTransfer ? m_pTransfer->GetStateText( bLong ) :
			LoadString( IDS_STATUS_UNKNOWN ) );
	}

	// Source transfer current limit (zero - no limit)
	inline DWORD GetLimit() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return ( ( m_pTransfer && m_pTransfer->m_mInput.pLimit ) ?
			*m_pTransfer->m_mInput.pLimit : 0 );
	}

	// Direct access to transfer
	inline const CDownloadTransfer* GetTransfer() const
	{
		ASSUME_LOCK( Transfers.m_pSection );
		return m_pTransfer;
	}

	// Source can be previewed
	bool IsPreviewCapable() const;
};

template<>
struct std::less< CDownloadSource* > : public std::binary_function < CDownloadSource*, CDownloadSource*, bool >
{
	inline bool operator()( const CDownloadSource* _Left, const CDownloadSource* _Right ) const throw( )
	{
		return ( CompareFileTime( &_Left->m_tLastSeen, &_Right->m_tLastSeen ) < 0 );
	}
};

typedef std::set< CDownloadSource* > CSortedSources;
