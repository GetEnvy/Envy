//
// Uploads.h
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

class CBuffer;
class CConnection;
class CUploadTransfer;
class CEnvyFile;


class CUploads
{
public:
	CUploads();
	~CUploads();

public:
	DWORD		m_nCount;			// Active count
	DWORD		m_nBandwidth;		// Total speed
	DWORD		m_nBestSpeed;		// Best speed
	DWORD		m_nTorrentSpeed;	// BitTorrent clamp
	BOOL		m_bStable;			// Stable flag

protected:
	CList< CUploadTransfer* >	m_pList;

public:
	void		Clear(BOOL bMessage = TRUE);
	DWORD		GetCount(CUploadTransfer* pExcept, int nState = -1) const;
	DWORD		GetTorrentCount(int nState) const;

	BOOL		AllowMoreTo(const IN_ADDR* pAddress) const;
	BOOL		CanUploadFileTo(const IN_ADDR* pAddress, const CEnvyFile* pFile) const;
	BOOL		EnforcePerHostLimit(CUploadTransfer* pUpload, BOOL bRequest = FALSE);

	void		OnRun();
	DWORD		GetBandwidth() const;
	DWORD		GetBandwidthLimit() const;			// Calculate upload speed (Bytes/s)
	void		SetStable(DWORD nSpeed);
	BOOL		OnAccept(CConnection* pConnection);
	void		OnRename(LPCTSTR pszSource, LPCTSTR pszTarget);		// pszTarget: 0 = delete file, 1 = release file

	void		Add(CUploadTransfer* pUpload);
	void		Remove(CUploadTransfer* pUpload);

// List Access
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CUploadTransfer* GetNext(POSITION& pos) const
	{
		return m_pList.GetNext( pos );
	}

	inline BOOL Check(CUploadTransfer* pUpload) const
	{
		return m_pList.Find( pUpload ) != NULL;
	}

	inline INT_PTR GetTransferCount() const
	{
		return GetCount( NULL, -2 );
	}

	inline DWORD GetTorrentTransferCount() const
	{
		return GetTorrentCount( -2 );
	}

	inline DWORD GetTorrentUploadCount() const
	{
		return GetTorrentCount( -3 );
	}
};

extern CUploads Uploads;
