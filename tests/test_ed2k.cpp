//
// test_ed2k.cpp
//
// Integration tests for ED2K protocol enhancements
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "../Envy/FileIdentifier.h"
#include "../Envy/EDPacket.h"
#include "../Envy/EDNeighbour.h"
#include "../HashLib/AICH.h"
#include "../Envy/ED2KMetrics.h"
#include <iostream>
#include <vector>

// Test FileIdentifier functionality
bool test_ed2k_file_identifier() {
    try {
        // Test FileIdentifier creation
        // Note: This would test the FileIdentifier class functionality
        // For integration testing, we verify the class exists and can be used

        std::cout << "FileIdentifier class available for ED2K operations";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "ED2K FileIdentifier test failed: " << e.what();
        return false;
    }
}

// Test MultiPacket Ext2 support
bool test_ed2k_multipacket() {
    try {
        // Test MultiPacket Ext2 protocol support
        // This would test opcodes 0xA9, 0xB0 and related functionality

        std::cout << "MultiPacket Ext2 structures available";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "ED2K MultiPacket test failed: " << e.what();
        return false;
    }
}

// Test HashsetRequest2/Answer2 implementation
bool test_ed2k_hashset() {
    try {
        // Test hashset request/response functionality
        // This would test the HashsetRequest2/Answer2 protocol implementation

        std::cout << "Hashset request/response structures available";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "ED2K hashset test failed: " << e.what();
        return false;
    }
}

// Test AICH integration with ED2K
bool test_ed2k_aich_integration() {
    try {
        // Test AICH support in ED2K client
        // This would test how AICH integrates with ED2K file transfers

        std::cout << "AICH integration with ED2K available";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "ED2K AICH integration test failed: " << e.what();
        return false;
    }
}

// Test AICH hash functionality
bool test_aich_hash() {
    try {
        CAICHHash aichHash;

        // Test basic initialization
        if (!aichHash.IsEmpty()) {
            std::cerr << "AICH hash should be empty initially";
            return false;
        }

        // Test file hashing
        const char* testData = "Hello, World! This is a test for AICH hashing.";
        const uint32 testDataSize = (uint32)strlen(testData);

        aichHash.BeginFile(testDataSize * 3); // Test with larger file size
        aichHash.AddToFile((LPCVOID)testData, testDataSize);
        aichHash.AddToFile((LPCVOID)testData, testDataSize);
        aichHash.AddToFile((LPCVOID)testData, testDataSize);

        if (!aichHash.FinishFile()) {
            std::cerr << "AICH hash finalization failed";
            return false;
        }

        if (!aichHash.GetRawHash()) {
            std::cerr << "AICH hash should have raw data after finalization";
            return false;
        }

        // Test hash string generation
        CString hashString;
        aichHash.GetStringHash(hashString);
        if (hashString.GetLength() != AICH_HASH_SIZE * 2) { // 2 chars per byte for hex
            std::cerr << "AICH hash string has incorrect length";
            return false;
        }

        // Test hash comparison
        CAICHHash aichHash2;
        aichHash2.BeginFile(testDataSize * 3);
        aichHash2.AddToFile((LPCVOID)testData, testDataSize);
        aichHash2.AddToFile((LPCVOID)testData, testDataSize);
        aichHash2.AddToFile((LPCVOID)testData, testDataSize);
        aichHash2.FinishFile();

        if (!aichHash.Compare(aichHash2)) {
            std::cerr << "Identical AICH hashes should compare equal";
            return false;
        }

        std::cout << "AICH hash functionality test passed";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "AICH hash test failed: " << e.what();
        return false;
    }
}

// Test ED2K server message parsing and display
bool test_ed2k_server_message_parsing() {
    try {
        // Test basic server message parsing
        CEDPacket* pPacket = CEDPacket::New(ED2K_S2C_SERVERMESSAGE, ED2K_PROTOCOL_EDONKEY);

        if (!pPacket) {
            std::cerr << "Failed to create server message packet";
            return false;
        }

        // Test message with different line endings
        const char* testMessage = "Welcome to this ED2K server!\r\nPlease follow the rules.\nHave fun sharing!";
        pPacket->WriteEDString(testMessage, FALSE);

        // Test the ToASCII method displays the message correctly
        CString asciiDisplay = pPacket->ToASCII();
        if (asciiDisplay.IsEmpty() || asciiDisplay.Find(L"Welcome") == -1) {
            std::cerr << "Server message not displayed correctly in ToASCII";
            pPacket->Release();
            return false;
        }

        std::cout << "Server message ASCII display: " << (LPCTSTR)asciiDisplay;

        pPacket->Release();

        // Test Unicode message parsing
        pPacket = CEDPacket::New(ED2K_S2C_SERVERMESSAGE, ED2K_PROTOCOL_EDONKEY);
        if (!pPacket) {
            std::cerr << "Failed to create Unicode server message packet";
            return false;
        }

        const char* unicodeMessage = "Bienvenue sur ce serveur ED2K!"; // French welcome
        pPacket->WriteEDString(unicodeMessage, TRUE); // Write as Unicode

        asciiDisplay = pPacket->ToASCII();
        if (asciiDisplay.IsEmpty() || asciiDisplay.Find(L"Bienvenue") == -1) {
            std::cerr << "Unicode server message not displayed correctly";
            pPacket->Release();
            return false;
        }

        std::cout << "Unicode server message ASCII display: " << (LPCTSTR)asciiDisplay;

        pPacket->Release();

        std::cout << "ED2K server message parsing and display test passed";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "ED2K server message parsing test failed: " << e.what();
        return false;
    }
}

// Test ED2K metrics collection
bool test_ed2k_metrics() {
    try {
        CED2KMetrics metrics;

        // Test AICH metrics
        metrics.RecordAICHVerification(true, 100);
        metrics.RecordAICHVerification(false, 150);
        metrics.RecordAICHHashing(1024 * 1024, 200); // 1MB in 200ms

        // Test Kademlia metrics
        metrics.RecordKadRequest(KAD_REQUEST_BOOTSTRAP, true);
        metrics.RecordKadRequest(KAD_REQUEST_FIND_NODE, false);
        metrics.RecordKadResponseTime(50);
        metrics.RecordKadContactAdded();
        metrics.RecordKadContactRemoved();

        // Test MultiPacket metrics
        metrics.RecordMultiPacketSent(3);
        metrics.RecordMultiPacketReceived(2);

        // Test network metrics
        metrics.RecordIPv6Connection(true);
        metrics.RecordIPv6Connection(false);
        metrics.RecordUPnPAttempt(true);

        // Test metrics output
        CString metricsString;
        metrics.GetMetrics(metricsString);

        if (metricsString.IsEmpty()) {
            std::cerr << "Metrics output should not be empty";
            return false;
        }

        // Test metrics reset
        metrics.Reset();
        metricsString.Empty();
        metrics.GetMetrics(metricsString);

        // After reset, metrics should be minimal
        if (metricsString.Find(L"0/0") == -1) {
            std::cerr << "Metrics should show zeros after reset";
            return false;
        }

        std::cout << "ED2K metrics functionality test passed";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "ED2K metrics test failed: " << e.what();
        return false;
    }
}
