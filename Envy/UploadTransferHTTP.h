//
// UploadTransferHTTP.h
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

#include "UploadTransfer.h"

class CLibraryFile;
class CDownload;
class CTigerTree;
class CXMLElement;
class CED2K;


class CUploadTransferHTTP : public CUploadTransfer
{
public:
	CUploadTransferHTTP();
	virtual ~CUploadTransferHTTP();

protected:
	CString		m_sRequest;
	BOOL		m_bHead;
	BOOL		m_bConnectHdr;
	BOOL		m_bKeepAlive;
	BOOL		m_bDeflate;
	BOOL		m_bBackwards;
	BOOL		m_bRange;
	BOOL		m_bQueueMe;
	BOOL		m_bNotEnvy;
	// "Accept" header:
	// 0 - unknown
	// 1 - Gnutella 1 (application/x-gnutella-packets)
	// 2 - Gnutella 2 (application/x-gnutella2 or application/x-envy)
	int			m_nAccept;
	// Gnutella functionality:
	// 0 - Pure HTTP
	// 1 - Pure G1
	// 2 - Pure G2
	// 3 - Mixed G1/G2
	int			m_nGnutella;
	int			m_nReaskMultiplier; // Last re-ask time multiplier used
	BOOL		m_bTigerTree;		// Is TigerTree hashset present?
	BOOL		m_bHashset;			// Is eDonkey2000 hashset present?
	BOOL		m_bMetadata;
	CString		m_sLocations;
	CString		m_sRanges;
	CString		m_sPrivateKey;		// Envy-proposed extension: Browse Private Key

public:
	virtual void	AttachTo(CConnection* pConnection);
	inline BOOL 	IsBackwards() const { return m_bBackwards; }

protected:
	BOOL	ReadRequest();
	BOOL	RequestSharedFile(CLibraryFile* pFile, CSingleLock& oLibraryLock);
	BOOL	RequestPartialFile(CDownload* pFile);
	BOOL	RequestTigerTreeRaw(const CTigerTree* pTigerTree, BOOL bDelete);
	BOOL	RequestTigerTreeDIME(const CTigerTree* pTigerTree, int nDepth, const CED2K* pHashset, BOOL bDelete);
	BOOL	RequestMetadata(CXMLElement* pMetadata);
	BOOL	RequestPreview(CLibraryFile* pFile, CSingleLock& oLibraryLock);
	BOOL	RequestHostBrowse();

	BOOL	IsNetworkDisabled();
	BOOL	QueueRequest();
	BOOL	OpenFileSendHeaders();
	void	SendDefaultHeaders();
	void	SendFileHeaders();
	void	OnCompleted();
	void	SendResponse(UINT nResourceID, BOOL bFileHeaders = FALSE);
	void	GetNeighbourList(CString& strOutput);

	virtual BOOL	OnRun();
	virtual void	OnDropped();
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();
};
