//
// AICH.h
//
// Advanced Integrity Check Hash (AICH) implementation for eDonkey2000
// Provides additional file integrity verification beyond standard ED2K hashing
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#pragma once

#include <vector>
#include <memory>
#include "MD4.h"

const DWORD AICH_CHUNK_SIZE = 184320u; // 180KB chunks for AICH
const DWORD AICH_HASH_SIZE = 16;       // MD4 hash size

class HASHLIB_API CAICHHash
{
public:
    CAICHHash();
    ~CAICHHash();

    // Delete copy and move operations
    CAICHHash(const CAICHHash&) = delete;
    CAICHHash& operator=(const CAICHHash&) = delete;
    CAICHHash(CAICHHash&&) = delete;
    CAICHHash& operator=(CAICHHash&&) = delete;

    void Clear();
    bool IsEmpty() const { return m_hash.empty(); }

    // Hash calculation
    void BeginFile(uint64 nFileSize);
    void AddToFile(LPCVOID pInput, uint32 nLength);
    bool FinishFile();

    // Hash access
    const BYTE* GetRawHash() const;
    DWORD GetRawHashSize() const { return AICH_HASH_SIZE; }
    void GetStringHash(CString& strHash) const;

    // Comparison
    bool Compare(const CAICHHash& other) const;

private:
    void BuildHashTree();
    void CalculateHashForChunk(const BYTE* pChunk, DWORD nChunkSize, BYTE* pHash);

    std::vector<BYTE> m_hash;           // Final AICH hash
    std::vector<std::vector<BYTE>> m_hashTree; // Hash tree for verification
    uint64 m_nFileSize;                 // Total file size
    uint64 m_nBytesHashed;              // Bytes processed so far
    CMD4 m_md4;                         // MD4 hasher for individual chunks
    bool m_bFinalized;                  // Whether final hash has been calculated
};

class HASHLIB_API CAICHHashTree
{
public:
    CAICHHashTree();
    ~CAICHHashTree();

    // Tree operations
    void BuildTree(const CAICHHash& rootHash, uint64 nFileSize);
    bool VerifyChunk(uint64 nChunkOffset, const BYTE* pChunk, DWORD nChunkSize) const;

    // Tree access
    const CAICHHash& GetRootHash() const { return m_rootHash; }
    uint64 GetFileSize() const { return m_nFileSize; }

private:
    CAICHHash m_rootHash;
    uint64 m_nFileSize;
    std::vector<CAICHHash> m_hashNodes;
};
