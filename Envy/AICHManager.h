//
 // AICHManager.h
//
// This file is part of Envy (getenvy.com) � 2016-2020
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2016
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

class CAICHHash;

class CAICHManager
{
public:
	CAICHManager();
	virtual ~CAICHManager();

	// Get AICH data for a peer
	bool GetAICHDataForPeer(const Hashes::Guid& oGUID, CBuffer& oBuffer);

	// Build AICH hash tree from file
	bool BuildAICHHashTree(LPCTSTR szFilename, CAICHHash& oMasterHash);

	// Verify file integrity using AICH
	bool VerifyFile(LPCTSTR szFilename, const CAICHHash& oMasterHash);

	// Store AICH data for a peer
	bool StoreAICHData(const Hashes::Guid& oGUID, const CBuffer& oBuffer);

	// Check if we have AICH data for a peer
	bool HasAICHData(const Hashes::Guid& oGUID);

private:
	void Cleanup();
	bool BuildHashTree(std::vector<CAICHHash>& blockHashes, CAICHHash& oMasterHash);

	CMap<Hashes::Guid, Hashes::Guid&, CBuffer*, CBuffer*&> m_AICHData;
	CCriticalSection m_pSection;
};

// AICH hash implementation
class CAICHHash
{
public:
	CAICHHash();
	~CAICHHash();

	// Initialize with data
	bool InitFromData(const BYTE* pData, size_t nDataLen);

	// Initialize from hash
	bool InitFromHash(const BYTE* pHash);

	// Get hash value
	const BYTE* GetHash() const;
	size_t GetHashSize() const;

	// Compare hashes
	bool operator==(const CAICHHash& other) const;
	bool operator!=(const CAICHHash& other) const;

private:
	BYTE m_pHash[20]; // SHA-1 hash
	bool m_bValid;
};
