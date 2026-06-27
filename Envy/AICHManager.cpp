//
// AICHManager.cpp
//
// Advanced Intelligent Corruption Handler (AICH) implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//

#include "StdAfx.h"
#include "AICHManager.h"
#include "Hashes.h"
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// AICH hash block size (180KB for optimal performance)
#define AICH_BLOCK_SIZE 184320

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
	const BYTE* GetHash() const { return m_pHash; }
	size_t GetHashSize() const { return 20; } // SHA-1 hash size

	// Compare hashes
	bool operator==(const CAICHHash& other) const;
	bool operator!=(const CAICHHash& other) const;

private:
	BYTE m_pHash[20]; // SHA-1 hash
	bool m_bValid;
};

CAICHHash::CAICHHash()
	: m_bValid(false)
{
	ZeroMemory(m_pHash, sizeof(m_pHash));
}

CAICHHash::~CAICHHash()
{
	// Clear sensitive data
	ZeroMemory(m_pHash, sizeof(m_pHash));
}

bool CAICHHash::InitFromData(const BYTE* pData, size_t nDataLen)
{
	if (!pData || nDataLen == 0)
		return false;

	// Use SHA-1 for AICH hashing
	Hashes::Sha1Hash sha1;
	sha1.Add(pData, nDataLen);
	sha1.Finish();
	if (sha1.GetHash())
	{
		memcpy(m_pHash, sha1.GetHash(), 20);
		m_bValid = true;
	}
	else
	{
		m_bValid = false;
	}

	m_bValid = true;
	return true;
}

bool CAICHHash::InitFromHash(const BYTE* pHash)
{
	if (!pHash)
		return false;

	memcpy(m_pHash, pHash, 20);
	m_bValid = true;
	return true;
}

bool CAICHHash::operator==(const CAICHHash& other) const
{
	if (!m_bValid || !other.m_bValid)
		return false;

	return (memcmp(m_pHash, other.m_pHash, 20) == 0);
}

bool CAICHHash::operator!=(const CAICHHash& other) const
{
	return !(*this == other);
}

//////////////////////////////////////////////////////////////////////
// CAICHManager Implementation

CAICHManager::CAICHManager()
{
}

CAICHManager::~CAICHManager()
{
	// Clean up stored AICH data
	Cleanup();
}

void CAICHManager::Cleanup()
{
	CSingleLock pLock(&m_pSection, TRUE);

	for (POSITION pos = m_AICHData.GetStartPosition(); pos;)
	{
		Hashes::Guid oGUID;
		CBuffer* pBuffer = NULL;
		m_AICHData.GetNextAssoc(pos, oGUID, pBuffer);
		delete pBuffer;
	}

	m_AICHData.RemoveAll();
}

bool CAICHManager::GetAICHDataForPeer(const Hashes::Guid& oGUID, CBuffer& oBuffer)
{
	CSingleLock pLock(&m_pSection, TRUE);

	CBuffer* pStoredBuffer = NULL;
	if (m_AICHData.Lookup(oGUID, pStoredBuffer) && pStoredBuffer)
	{
		oBuffer = *pStoredBuffer; // Copy buffer
		return true;
	}

	return false;
}

bool CAICHManager::BuildAICHHashTree(LPCTSTR szFilename, CAICHHash& oMasterHash)
{
	if (!szFilename || !*szFilename)
		return false;

	// Open file for reading
	CFile file;
	if (!file.Open(szFilename, CFile::modeRead | CFile::shareDenyWrite))
		return false;

	// Get file size
	ULONGLONG nFileSize = file.GetLength();
	if (nFileSize == 0)
	{
		file.Close();
		return false;
	}

	// Calculate number of blocks
	size_t nBlockCount = (size_t)((nFileSize + AICH_BLOCK_SIZE - 1) / AICH_BLOCK_SIZE);
	if (nBlockCount == 0)
		nBlockCount = 1;

	// Read file in blocks and build hash tree
	std::vector<CAICHHash> blockHashes;
	CBuffer blockData;

	try
	{
		for (size_t i = 0; i < nBlockCount; i++)
		{
			// Calculate block size (last block may be smaller)
			size_t nBlockSize = AICH_BLOCK_SIZE;
			if (i == nBlockCount - 1)
			{
				nBlockSize = (size_t)(nFileSize % AICH_BLOCK_SIZE);
				if (nBlockSize == 0)
					nBlockSize = AICH_BLOCK_SIZE;
			}

			// Read block data
			blockData.SetSize(nBlockSize);
			UINT nRead = file.Read(blockData.GetData(), (UINT)nBlockSize);
			if (nRead != nBlockSize)
				throw false;

			// Create hash for this block
			CAICHHash blockHash;
			if (!blockHash.InitFromData(blockData.GetData(), nBlockSize))
				throw false;

			blockHashes.push_back(blockHash);
		}
	}
	catch (...)
	{
		file.Close();
		return false;
	}

	file.Close();

	// Build hash tree (Merkle tree)
	return BuildHashTree(blockHashes, oMasterHash);
}

bool CAICHManager::BuildHashTree(std::vector<CAICHHash>& blockHashes, CAICHHash& oMasterHash)
{
	if (blockHashes.empty())
		return false;

	// If only one block, it's the master hash
	if (blockHashes.size() == 1)
	{
		oMasterHash = blockHashes[0];
		return true;
	}

	// Build tree by repeatedly hashing pairs
	while (blockHashes.size() > 1)
	{
		std::vector<CAICHHash> nextLevel;

		for (size_t i = 0; i < blockHashes.size(); i += 2)
		{
			if (i + 1 < blockHashes.size())
			{
				// Hash pair of hashes together
				BYTE combinedHash[40]; // 20 + 20 bytes
				memcpy(combinedHash, blockHashes[i].GetHash(), 20);
				memcpy(combinedHash + 20, blockHashes[i + 1].GetHash(), 20);

				CAICHHash pairHash;
				if (!pairHash.InitFromData(combinedHash, 40))
					return false;

				nextLevel.push_back(pairHash);
			}
			else
			{
				// Odd number of hashes - promote last one
				nextLevel.push_back(blockHashes[i]);
			}
		}

		blockHashes = nextLevel;
	}

	if (!blockHashes.empty())
	{
		oMasterHash = blockHashes[0];
		return true;
	}

	return false;
}

bool CAICHManager::VerifyFile(LPCTSTR szFilename, const CAICHHash& oMasterHash)
{
	CAICHHash computedMasterHash;
	if (!BuildAICHHashTree(szFilename, computedMasterHash))
		return false;

	return (computedMasterHash == oMasterHash);
}

bool CAICHManager::StoreAICHData(const Hashes::Guid& oGUID, const CBuffer& oBuffer)
{
	CSingleLock pLock(&m_pSection, TRUE);

	CBuffer* pNewBuffer = new CBuffer(oBuffer); // Copy buffer

	// Remove existing entry if any
	CBuffer* pOldBuffer = NULL;
	if (m_AICHData.Lookup(oGUID, pOldBuffer))
	{
		delete pOldBuffer;
		m_AICHData.RemoveKey(oGUID);
	}

	// Store new data
	m_AICHData.SetAt(oGUID, pNewBuffer);
	return true;
}

bool CAICHManager::HasAICHData(const Hashes::Guid& oGUID)
{
	CSingleLock pLock(&m_pSection, TRUE);
	CBuffer* pBuffer = NULL;
	return m_AICHData.Lookup(oGUID, pBuffer) && pBuffer;
}

// Global AICH manager instance
CAICHManager AICHManager;
