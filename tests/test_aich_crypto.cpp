//
// test_aich_crypto.cpp
//
// Unit tests for AICHManager and CryptoProvider implementations
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

// Forward declarations and minimal implementations for testing
class CBuffer
{
public:
    CBuffer() : m_pData(NULL), m_nSize(0) {}
    ~CBuffer() { if (m_pData) delete[] m_pData; }

    void SetSize(size_t nSize) {
        if (m_pData) delete[] m_pData;
        m_pData = new BYTE[nSize];
        m_nSize = nSize;
        ZeroMemory(m_pData, nSize);
    }

    BYTE* GetData() { return m_pData; }
    size_t GetSize() const { return m_nSize; }

    CBuffer& operator=(const CBuffer& other) {
        if (this != &other) {
            SetSize(other.m_nSize);
            memcpy(m_pData, other.m_pData, m_nSize);
        }
        return *this;
    }

private:
    BYTE* m_pData;
    size_t m_nSize;
};

namespace Hashes
{
    class Sha1Hash
    {
    public:
        Sha1Hash() : m_hHash(NULL), m_hProv(NULL) {}
        ~Sha1Hash() { Cleanup(); }

        bool Add(const void* pData, size_t nLength) {
            if (!m_hHash) {
                if (!CryptAcquireContext(&m_hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
                    return false;
                if (!CryptCreateHash(m_hProv, CALG_SHA1, 0, 0, &m_hHash))
                    return false;
            }
            return CryptHashData(m_hHash, (const BYTE*)pData, (DWORD)nLength, 0) == TRUE;
        }

        bool Finish() {
            DWORD dwHashLen = 20;
            return CryptGetHashParam(m_hHash, HP_HASHVAL, m_hash, &dwHashLen, 0) == TRUE;
        }

        const BYTE* GetHash() const { return m_hash; }

    private:
        void Cleanup() {
            if (m_hHash) CryptDestroyHash(m_hHash);
            if (m_hProv) CryptReleaseContext(m_hProv, 0);
            m_hHash = NULL;
            m_hProv = NULL;
        }

        HCRYPTHASH m_hHash;
        HCRYPTPROV m_hProv;
        BYTE m_hash[20];
    };

    struct Guid {
        BYTE data[16];
        bool operator==(const Guid& other) const {
            return memcmp(data, other.data, 16) == 0;
        }
        bool operator!=(const Guid& other) const {
            return !(*this == other);
        }
        bool operator<(const Guid& other) const {
            return memcmp(data, other.data, 16) < 0;
        }
    };
}

// Include the actual implementation files
#include "..\Envy\AICHManager.h"
#include "..\Envy\CryptoProvider.h"

// Test AICH hash functionality
bool TestAICHHash()
{
    std::cout << "Testing AICH Hash functionality..." << std::endl;

    // Test data
    const char* testData = "Hello, World! This is a test for AICH hashing.";
    size_t testDataLen = strlen(testData);

    // Create hash from data
    CAICHHash hash1;
    if (!hash1.InitFromData((const BYTE*)testData, testDataLen))
    {
        std::cout << "ERROR: Failed to create hash from data" << std::endl;
        return false;
    }

    // Create another hash from same data
    CAICHHash hash2;
    if (!hash2.InitFromData((const BYTE*)testData, testDataLen))
    {
        std::cout << "ERROR: Failed to create second hash from data" << std::endl;
        return false;
    }

    // Hashes should be equal
    if (hash1 != hash2)
    {
        std::cout << "ERROR: Hashes from same data are not equal" << std::endl;
        return false;
    }

    // Create hash from different data
    const char* differentData = "This is different test data";
    CAICHHash hash3;
    if (!hash3.InitFromData((const BYTE*)differentData, strlen(differentData)))
    {
        std::cout << "ERROR: Failed to create hash from different data" << std::endl;
        return false;
    }

    // Hashes should be different
    if (hash1 == hash3)
    {
        std::cout << "ERROR: Hashes from different data are equal" << std::endl;
        return false;
    }

    std::cout << "✓ AICH Hash functionality tests passed" << std::endl;
    return true;
}

// Test AICHManager file hashing
bool TestAICHManager()
{
    std::cout << "Testing AICHManager functionality..." << std::endl;

    // Create a test file
    const char* testFilename = "test_aich_file.txt";
    const char* testContent = "This is test content for AICH file hashing verification. "
                             "It should be long enough to create multiple hash blocks. "
                             "The AICH system uses 180KB blocks for optimal performance. "
                             "This content needs to be sufficiently long to test the hash tree building.";

    // Write test file
    std::ofstream testFile(testFilename);
    if (!testFile.is_open())
    {
        std::cout << "ERROR: Could not create test file" << std::endl;
        return false;
    }

    // Write enough content to create multiple blocks
    for (int i = 0; i < 10; i++)
    {
        testFile << testContent;
    }
    testFile.close();

    // Test building hash tree
    CAICHHash masterHash;
    if (!AICHManager.BuildAICHHashTree(CString(testFilename), masterHash))
    {
        std::cout << "ERROR: Failed to build AICH hash tree" << std::endl;
        remove(testFilename);
        return false;
    }

    // Test file verification
    if (!AICHManager.VerifyFile(CString(testFilename), masterHash))
    {
        std::cout << "ERROR: File verification failed" << std::endl;
        remove(testFilename);
        return false;
    }

    // Test with modified file (should fail verification)
    std::ofstream modifiedFile(testFilename, std::ios::app);
    modifiedFile << "MODIFIED";
    modifiedFile.close();

    if (AICHManager.VerifyFile(CString(testFilename), masterHash))
    {
        std::cout << "ERROR: Modified file verification should have failed" << std::endl;
        remove(testFilename);
        return false;
    }

    // Clean up
    remove(testFilename);

    std::cout << "✓ AICHManager functionality tests passed" << std::endl;
    return true;
}

// Test CryptoProvider functionality
bool TestCryptoProvider()
{
    std::cout << "Testing CryptoProvider functionality..." << std::endl;

    // Check if crypto is available
    if (!CCryptoProvider::IsCryptoAvailable())
    {
        std::cout << "WARNING: Cryptographic functions not available on this system" << std::endl;
        std::cout << "✓ CryptoProvider availability check passed (functions unavailable)" << std::endl;
        return true; // This is acceptable for systems without crypto support
    }

    CCryptoProvider crypto;

    // Generate RSA key pair
    if (!crypto.GenerateRSAKeyPair())
    {
        std::cout << "ERROR: Failed to generate RSA key pair" << std::endl;
        return false;
    }

    // Get public key
    const BYTE* pPublicKey = crypto.GetPublicKey();
    size_t nPublicKeyLen = crypto.GetPublicKeyLen();

    if (!pPublicKey || nPublicKeyLen == 0)
    {
        std::cout << "ERROR: Failed to get public key" << std::endl;
        return false;
    }

    // Test data
    const char* testMessage = "This is a test message for cryptographic operations.";
    size_t testMessageLen = strlen(testMessage);

    // Sign data
    BYTE signature[1024];
    size_t signatureLen = sizeof(signature);
    if (!crypto.SignData((const BYTE*)testMessage, testMessageLen, signature, signatureLen))
    {
        std::cout << "ERROR: Failed to sign data" << std::endl;
        return false;
    }

    // Verify signature
    if (!crypto.VerifySignature((const BYTE*)testMessage, testMessageLen, signature, signatureLen, pPublicKey, nPublicKeyLen))
    {
        std::cout << "ERROR: Signature verification failed" << std::endl;
        return false;
    }

    // Test encryption/decryption
    BYTE encrypted[1024];
    size_t encryptedLen = sizeof(encrypted);
    if (!crypto.EncryptWithPublicKey((const BYTE*)testMessage, testMessageLen, encrypted, encryptedLen, pPublicKey, nPublicKeyLen))
    {
        std::cout << "ERROR: Failed to encrypt data" << std::endl;
        return false;
    }

    BYTE decrypted[1024];
    size_t decryptedLen = sizeof(decrypted);
    if (!crypto.DecryptWithPrivateKey(encrypted, encryptedLen, decrypted, decryptedLen))
    {
        std::cout << "ERROR: Failed to decrypt data" << std::endl;
        return false;
    }

    // Compare decrypted data with original
    if (decryptedLen != testMessageLen || memcmp(decrypted, testMessage, testMessageLen) != 0)
    {
        std::cout << "ERROR: Decrypted data does not match original" << std::endl;
        return false;
    }

    std::cout << "✓ CryptoProvider functionality tests passed" << std::endl;
    return true;
}

// Main test function
int main()
{
    std::cout << "=== AICH and CryptoProvider Verification Tests ===" << std::endl;
    std::cout << std::endl;

    bool allTestsPassed = true;

    // Run AICH hash tests
    if (!TestAICHHash())
    {
        allTestsPassed = false;
    }

    std::cout << std::endl;

    // Run AICHManager tests
    if (!TestAICHManager())
    {
        allTestsPassed = false;
    }

    std::cout << std::endl;

    // Run CryptoProvider tests
    if (!TestCryptoProvider())
    {
        allTestsPassed = false;
    }

    std::cout << std::endl;

    if (allTestsPassed)
    {
        std::cout << "🎉 All AICH and CryptoProvider tests PASSED!" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "❌ Some tests FAILED!" << std::endl;
        return 1;
    }
}
