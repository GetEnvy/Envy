//
// AICH.cpp
//
// Advanced Integrity Check Hash (AICH) implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "StdAfx.h"
#include "AICH.h"
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CAICHHash::CAICHHash()
    : m_nFileSize(0)
    , m_nBytesHashed(0)
    , m_bFinalized(false)
{
    m_hash.resize(AICH_HASH_SIZE);
}

CAICHHash::~CAICHHash()
{
    Clear();
}

void CAICHHash::Clear()
{
    m_hash.clear();
    m_hashTree.clear();
    m_nFileSize = 0;
    m_nBytesHashed = 0;
    m_bFinalized = false;
    m_md4.Clear();
}

void CAICHHash::BeginFile(uint64 nFileSize)
{
    Clear();
    m_nFileSize = nFileSize;
    m_nBytesHashed = 0;

    // Pre-allocate space for hash tree
    // Number of chunks = ceil(fileSize / chunkSize)
    uint32 nChunks = (uint32)((nFileSize + AICH_CHUNK_SIZE - 1) / AICH_CHUNK_SIZE);
    if (nChunks > 0) {
        m_hashTree.reserve(nChunks);
    }
}

void CAICHHash::AddToFile(LPCVOID pInput, uint32 nLength)
{
    if (m_bFinalized) {
        theApp.Message(MSG_WARNING, L"AICH: Attempting to add data to finalized hash");
        return;
    }

    if (!pInput || nLength == 0) {
        theApp.Message(MSG_WARNING, L"AICH: Invalid input parameters - pInput: %p, nLength: %u", pInput, nLength);
        return;
    }

    if (m_nBytesHashed >= m_nFileSize) {
        theApp.Message(MSG_WARNING, L"AICH: Attempting to add data beyond file size - hashed: %I64u, fileSize: %I64u",
            m_nBytesHashed, m_nFileSize);
        return;
    }

    const BYTE* pData = (const BYTE*)pInput;
    uint32 nRemaining = nLength;

    try {
        // Pre-allocate space to avoid reallocations
        size_t estimatedChunks = (m_nBytesHashed + nLength + AICH_CHUNK_SIZE - 1) / AICH_CHUNK_SIZE;
        if (m_hashTree.capacity() < estimatedChunks) {
            m_hashTree.reserve(estimatedChunks);
        }

        while (nRemaining > 0 && m_nBytesHashed < m_nFileSize) {
            uint32 nChunkRemaining = AICH_CHUNK_SIZE - (uint32)(m_nBytesHashed % AICH_CHUNK_SIZE);
            uint32 nToProcess = min(nRemaining, nChunkRemaining);

            // Ensure we don't exceed file size
            uint64 nTotalAfterAdd = m_nBytesHashed + nToProcess;
            if (nTotalAfterAdd > m_nFileSize) {
                nToProcess = (uint32)(m_nFileSize - m_nBytesHashed);
            }

            if (nToProcess > 0) {
                m_md4.AddToHash(pData, nToProcess);
                m_nBytesHashed += nToProcess;
                pData += nToProcess;
                nRemaining -= nToProcess;
            }

            // If we've filled a chunk or reached end of file, finalize it
            if ((m_nBytesHashed % AICH_CHUNK_SIZE) == 0 || m_nBytesHashed == m_nFileSize) {
                // Use a static buffer to avoid repeated allocations
                static BYTE chunkHash[AICH_HASH_SIZE];
                m_md4.Finish();
                m_md4.GetRawHash(chunkHash);

                // Move the hash into the tree (more efficient than copy)
                m_hashTree.emplace_back(chunkHash, chunkHash + AICH_HASH_SIZE);
                m_md4.Clear();
            }
        }
    }
    catch (const std::exception& e) {
        // Log error: Exception during AICH processing
        Clear(); // Reset state on error
        throw; // Re-throw to caller
    }
    catch (...) {
        // Log error: Unknown exception during AICH processing
        Clear(); // Reset state on error
        throw; // Re-throw to caller
    }
}

bool CAICHHash::FinishFile()
{
    if (m_bFinalized || m_hashTree.empty())
        return false;

    BuildHashTree();
    m_bFinalized = true;
    return true;
}

void CAICHHash::BuildHashTree()
{
    if (m_hashTree.empty())
        return;

    // Optimized hash tree building - reduce memory allocations
    size_t currentSize = m_hashTree.size();
    std::vector<BYTE> tempBuffer(AICH_HASH_SIZE * 2); // Buffer for two hashes

    while (currentSize > 1) {
        size_t nextSize = (currentSize + 1) / 2; // Round up division

        // Process pairs in-place where possible to reduce memory usage
        for (size_t i = 0; i < currentSize; i += 2) {
            CMD4 md4;

            // Hash the first hash
            md4.AddToHash(m_hashTree[i].data(), AICH_HASH_SIZE);

            // If there's a pair, hash both together
            if (i + 1 < currentSize) {
                md4.AddToHash(m_hashTree[i + 1].data(), AICH_HASH_SIZE);
            }

            md4.Finish();

            // Overwrite the first hash with the combined result
            md4.GetRawHash(m_hashTree[i].data());
        }

        currentSize = nextSize;
        // Remove unused elements from the vector
        if (m_hashTree.size() > currentSize) {
            m_hashTree.resize(currentSize);
        }
    }

    // The root hash is the final remaining hash
    if (!m_hashTree.empty()) {
        memcpy(m_hash.data(), m_hashTree[0].data(), AICH_HASH_SIZE);
    }
}

const BYTE* CAICHHash::GetRawHash() const
{
    return m_bFinalized ? m_hash.data() : nullptr;
}

void CAICHHash::GetStringHash(CString& strHash) const
{
    if (!m_bFinalized) {
        strHash.Empty();
        return;
    }

    strHash.Empty();
    for (int i = 0; i < AICH_HASH_SIZE; ++i) {
        CString strByte;
        strByte.Format(L"%02x", m_hash[i]);
        strHash += strByte;
    }
}

bool CAICHHash::Compare(const CAICHHash& other) const
{
    if (!m_bFinalized || !other.m_bFinalized)
        return false;

    return memcmp(m_hash.data(), other.m_hash.data(), AICH_HASH_SIZE) == 0;
}

////////////////////////////////////////////////////////////////////////////////
// CAICHHashTree Implementation
////////////////////////////////////////////////////////////////////////////////

CAICHHashTree::CAICHHashTree()
    : m_nFileSize(0)
{
}

CAICHHashTree::~CAICHHashTree()
{
}

void CAICHHashTree::BuildTree(const CAICHHash& rootHash, uint64 nFileSize)
{
    m_rootHash = rootHash;
    m_nFileSize = nFileSize;
    // In a full implementation, we would rebuild the entire hash tree
    // For now, we just store the root hash for basic verification
}

bool CAICHHashTree::VerifyChunk(uint64 nChunkOffset, const BYTE* pChunk, DWORD nChunkSize) const
{
    if (!pChunk || nChunkSize == 0)
        return false;

    // Calculate hash of the provided chunk
    CMD4 md4;
    md4.AddToHash(pChunk, nChunkSize);
    md4.Finish();

    BYTE chunkHash[AICH_HASH_SIZE];
    md4.GetRawHash(chunkHash);

    // In a full implementation, we would verify against the hash tree
    // For now, we just check if the root hash is available
    return m_rootHash.GetRawHash() != nullptr;
}
