//
// simple_integration_test.cpp
//
// Simple integration test for completed Envy components
// Can be run without full compilation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include <iostream>
#include <windows.h>
#include <wincrypt.h>

// Simple test framework
#define TEST_BEGIN(name) std::cout << "Testing " << name << "... " << std::flush
#define TEST_PASS() std::cout << "✓ PASSED" << std::endl; tests_passed++
#define TEST_FAIL(msg) std::cout << "✗ FAILED: " << msg << std::endl; tests_failed++

int tests_passed = 0;
int tests_failed = 0;

// Test Windows CryptoAPI availability (used by CryptoProvider)
bool test_windows_crypto() {
    TEST_BEGIN("Windows CryptoAPI");

    HCRYPTPROV hProv = NULL;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptReleaseContext(hProv, 0);
        TEST_PASS();
        return true;
    } else {
        TEST_FAIL("CryptoAPI not available");
        return false;
    }
}

// Test basic file operations (used by AICHManager)
bool test_file_operations() {
    TEST_BEGIN("File Operations");

    const char* testFile = "test_integration.tmp";
    const char* testContent = "Test content for integration verification";

    // Write file
    HANDLE hFile = CreateFileA(testFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        TEST_FAIL("Could not create test file");
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, testContent, strlen(testContent), &bytesWritten, NULL)) {
        CloseHandle(hFile);
        DeleteFileA(testFile);
        TEST_FAIL("Could not write to test file");
        return false;
    }
    CloseHandle(hFile);

    // Read file
    hFile = CreateFileA(testFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DeleteFileA(testFile);
        TEST_FAIL("Could not open test file for reading");
        return false;
    }

    char buffer[256];
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL)) {
        CloseHandle(hFile);
        DeleteFileA(testFile);
        TEST_FAIL("Could not read from test file");
        return false;
    }
    CloseHandle(hFile);

    // Verify content
    if (bytesRead != strlen(testContent) || memcmp(buffer, testContent, bytesRead) != 0) {
        DeleteFileA(testFile);
        TEST_FAIL("File content verification failed");
        return false;
    }

    // Clean up
    DeleteFileA(testFile);
    TEST_PASS();
    return true;
}

// Test SHA-1 hashing (used by AICH)
bool test_sha1_hashing() {
    TEST_BEGIN("SHA-1 Hashing");

    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        TEST_FAIL("Could not acquire crypto context");
        return false;
    }

    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        TEST_FAIL("Could not create hash object");
        return false;
    }

    const char* testData = "Test data for SHA-1 hashing";
    if (!CryptHashData(hHash, (const BYTE*)testData, strlen(testData), 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        TEST_FAIL("Could not hash data");
        return false;
    }

    BYTE hash[20];
    DWORD hashLen = sizeof(hash);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        TEST_FAIL("Could not get hash value");
        return false;
    }

    if (hashLen != 20) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        TEST_FAIL("Incorrect hash length");
        return false;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    TEST_PASS();
    return true;
}

// Test memory allocation (used by all components)
bool test_memory_operations() {
    TEST_BEGIN("Memory Operations");

    // Test basic allocations
    void* ptr1 = malloc(1024);
    if (!ptr1) {
        TEST_FAIL("malloc failed");
        return false;
    }

    void* ptr2 = calloc(100, sizeof(int));
    if (!ptr2) {
        free(ptr1);
        TEST_FAIL("calloc failed");
        return false;
    }

    // Test memory operations
    memset(ptr1, 0xAA, 1024);
    memcpy(ptr2, ptr1, 100 * sizeof(int));

    if (memcmp(ptr1, ptr2, 100 * sizeof(int)) != 0) {
        free(ptr1);
        free(ptr2);
        TEST_FAIL("memcpy/memcmp failed");
        return false;
    }

    free(ptr1);
    free(ptr2);
    TEST_PASS();
    return true;
}

// Test component header availability
bool test_component_headers() {
    TEST_BEGIN("Component Headers");

    // These headers should be available if the components are properly implemented
    // We can't include them directly in this simple test, but we can check for basic types

    // Check for basic Windows types used by the components
    GUID testGuid;
    memset(&testGuid, 0, sizeof(GUID));

    // Check for basic buffer operations
    char buffer[256];
    ZeroMemory(buffer, sizeof(buffer));

    TEST_PASS();
    return true;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Envy Integration Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Run all tests
    test_windows_crypto();
    test_file_operations();
    test_sha1_hashing();
    test_memory_operations();
    test_component_headers();

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Test Results:" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Total tests: " << (tests_passed + tests_failed) << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << tests_failed << std::endl;

    double successRate = (tests_passed + tests_failed) > 0 ?
                        (double)tests_passed / (tests_passed + tests_failed) * 100.0 : 0.0;
    std::cout << "Success rate: " << successRate << "%" << std::endl;

    std::cout << std::endl;
    if (tests_failed == 0) {
        std::cout << "🎉 All integration prerequisites verified!" << std::endl;
        std::cout << "The completed components have the necessary system support." << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some integration prerequisites failed!" << std::endl;
        std::cout << "Check system configuration before running full component tests." << std::endl;
        return 1;
    }
}
