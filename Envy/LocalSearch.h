//
// LocalSearch.h
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

#include "QuerySearch.h"

class CNeighbour;
class CLibraryFile;
class CAlbumFolder;
class CLibraryFolder;
class CXMLElement;
class CBuffer;
class CDownload;
class CPacket;
class CG1Packet;
class CG2Packet;
class CDCPacket;


class CLocalSearch
{
public:
	CLocalSearch(CQuerySearch* pSearch, const CNeighbour* pNeighbour);
	CLocalSearch(CQuerySearch* pSearch, PROTOCOLID nProtocol);
	CLocalSearch(CBuffer* pBuffer, PROTOCOLID nProtocol);

	SOCKADDR_IN		m_pEndpoint;	// Endpoint or neighbour address
	BOOL			m_bUDP;			// Send packets via UDP or TCP

protected:
	CQuerySearchPtr	m_pSearch;
	CBuffer*		m_pBuffer;		// Save packets to this buffer instead
	Hashes::Guid	m_oGUID;
	PROTOCOLID		m_nProtocol;

	typedef CMap< CSchemaPtr, CSchemaPtr, CXMLElement*, CXMLElement* > CSchemaMap;

public:
	const CQuerySearch*		GetSearch() const{ return m_pSearch; }
	// Search library files and active downloads (-1 = use default limit, 0 = no limit)
	bool		Execute(INT_PTR nMaximum = -1, bool bPartial = true, bool bShared = true);

protected:
	bool		ExecuteSharedFiles(INT_PTR nMaximum, INT_PTR& nHits);
	bool		ExecutePartialFiles(INT_PTR nMaximum, INT_PTR& nHits);
	template< typename T > void SendHits(const CList< T * >& oFiles);
	template< typename T > void AddHit(CPacket* pPacket, CSchemaMap& pSchemas, T * pHit, int nIndex);
	void		AddHitG1(CG1Packet* pPacket, CSchemaMap& pSchemas, CLibraryFile* pFile, int nIndex);
	void		AddHitG2(CG2Packet* pPacket, CSchemaMap& pSchemas, CLibraryFile* pFile, int nIndex);
	void		AddHitDC(CDCPacket* pPacket, CSchemaMap& pSchemas, CLibraryFile* pFile, int nIndex);
	void		AddHitG1(CG1Packet* pPacket, CSchemaMap& pSchemas, CDownload* pDownload, int nIndex);
	void		AddHitG2(CG2Packet* pPacket, CSchemaMap& pSchemas, CDownload* pDownload, int nIndex);
	void		AddHitDC(CDCPacket* pPacket, CSchemaMap& pSchemas, CDownload* pDownload, int nIndex);
	template< typename T > bool IsValidForHit(const T * pHit) const;

	CPacket*	CreatePacket();
	CG1Packet*	CreatePacketG1();
	CG2Packet*	CreatePacketG2();
	CDCPacket*	CreatePacketDC();
	void		WriteTrailer(CPacket* pPacket, CSchemaMap& pSchemas, BYTE nHits);
	void		WriteTrailerG1(CG1Packet* pPacket, CSchemaMap& pSchemas, BYTE nHits);
	void		WriteTrailerG2(CG2Packet* pPacket, CSchemaMap& pSchemas, BYTE nHits);
	void		WriteTrailerDC(CDCPacket* pPacket, CSchemaMap& pSchemas, BYTE nHits);
	void		DispatchPacket(CPacket* pPacket);
	CG2Packet*	AlbumToPacket(CAlbumFolder* pFolder);
	CG2Packet*	FolderToPacket(CLibraryFolder* pFolder);
	CG2Packet*	FoldersToPacket();

private:
	// Limit query answer packet size since Gnutella 1/2 drops packets larger than Settings.Gnutella.MaximumPacket
	static const DWORD	MAX_QUERY_PACKET_SIZE = 16384;	// (bytes)
};
