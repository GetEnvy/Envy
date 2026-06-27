//
// test_aich.cpp
//
// Integration tests for AICHManager (Advanced Intelligent Corruption Handling)
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "../Envy/AICHManager.h"
#include <fstream>
#include <iostream>

// Test AICH hash functionality
bool test_aich_hash_operations() {
    try {
        // Test hash creation from data
        CAICHHash hash1;
        const char* testData = "Test data for AICH hashing verification";
        size_t dataLen = strlen(testData);

        if (!hash1.InitFromData((const BYTE*)testData, dataLen)) {
            throw std::runtime_error("Failed to create hash from data");
        }

        // Test hash comparison
        CAICHHash hash2;
        if (!hash2.InitFromData((const BYTE*)testData, dataLen)) {
            throw std::runtime_error("Failed to create second hash from same data");
        }

        if (hash1 != hash2) {
            throw std::runtime_error("Hashes from same data should be equal");
        }

        // Test hash from different data
        CAICHHash hash3;
        const char* differentData = "Different test data";
        if (!hash3.InitFromData((const BYTE*)differentData, strlen(differentData))) {
            throw std::runtime_error("Failed to create hash from different data");
        }

        if (hash1 == hash3) {
            throw std::runtime_error("Hashes from different data should not be equal");
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "AICH hash operations test failed: " << e.what();
        return false;
    }
}

// Test AICHManager file operations
bool test_aich_file_operations() {
    try {
        // Create a test file
        const char* testFilename = "test_aich_integration.txt";
        const char* testContent = "This is test content for AICH file integrity verification. "
                                "The AICH system should be able to create hash trees and verify file integrity.";

        // Write test file
        std::ofstream testFile(testFilename, std::ios::binary);
        if (!testFile.is_open()) {
            throw std::runtime_error("Could not create test file");
        }
        testFile.write(testContent, strlen(testContent));
        testFile.close();

        // Test hash tree building
        CAICHHash masterHash;
        if (!AICHManager.BuildAICHHashTree(CString(testFilename), masterHash)) {
            remove(testFilename);
            throw std::runtime_error("Failed to build AICH hash tree");
        }

        // Test file verification
        if (!AICHManager.VerifyFile(CString(testFilename), masterHash)) {
            remove(testFilename);
            throw std::runtime_error("File verification failed");
        }

        // Clean up
        remove(testFilename);
        return true;

    } catch (const std::exception& e) {
        std::cerr << "AICH file operations test failed: " << e.what();
        return false;
    }
}

// Test AICH peer data management
bool test_aich_peer_management() {
    try {
        // Test storing and retrieving AICH data for peers
        Hashes::Guid testGuid;
        memset(&testGuid, 0xAA, sizeof(testGuid));

        CBuffer testBuffer;
        const char* testData = "Test AICH data for peer";
        testBuffer.SetSize(strlen(testData));
        memcpy(testBuffer.GetData(), testData, strlen(testData));

        // Store AICH data
        if (!AICHManager.StoreAICHData(testGuid, testBuffer)) {
            throw std::runtime_error("Failed to store AICH data");
        }

        // Check if we have data for the peer
        if (!AICHManager.HasAICHData(testGuid)) {
            throw std::runtime_error("AICH data should be available for peer");
        }

        // Retrieve AICH data
        CBuffer retrievedBuffer;
        if (!AICHManager.GetAICHDataForPeer(testGuid, retrievedBuffer)) {
            throw std::runtime_error("Failed to retrieve AICH data");
        }

        // Verify retrieved data
        if (retrievedBuffer.GetSize() != testBuffer.GetSize() ||
            memcmp(retrievedBuffer.GetData(), testBuffer.GetData(), testBuffer.GetSize()) != 0) {
            throw std::runtime_error("Retrieved AICH data does not match stored data");
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "AICH peer management test failed: " << e.what();
        return false;
    }
}
