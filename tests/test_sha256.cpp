//
// test_sha256.cpp
//
// Unit tests for SHA-256 implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "../HashLib/SHA256.h"
#include <iostream>
#include <iomanip>
#include <sstream>

// Test SHA-256 with known test vectors
bool test_sha256_known_vectors() {
    try {
        // Test vector 1: empty string
        {
            CSHA256 sha256;
            sha256.Reset();
            sha256.Finish();

            uint8 hash[32];
            sha256.GetHash(hash);

            // Expected: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
            uint8 expected[32] = {
                0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
                0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
                0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
                0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
            };

            if (memcmp(hash, expected, 32) != 0) {
                std::cout << "SHA-256 empty string test failed" << std::endl;
                return false;
            }
        }

        // Test vector 2: "abc"
        {
            CSHA256 sha256;
            sha256.Reset();
            sha256.Add("abc", 3);
            sha256.Finish();

            uint8 hash[32];
            sha256.GetHash(hash);

            // Expected: ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
            uint8 expected[32] = {
                0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
                0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
                0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
                0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
            };

            if (memcmp(hash, expected, 32) != 0) {
                std::cout << "SHA-256 'abc' test failed" << std::endl;
                return false;
            }
        }

        // Test vector 3: longer message
        {
            CSHA256 sha256;
            sha256.Reset();
            const char* message = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
            sha256.Add(message, strlen(message));
            sha256.Finish();

            uint8 hash[32];
            sha256.GetHash(hash);

            // Expected: 248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1
            uint8 expected[32] = {
                0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
                0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
                0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
            };

            if (memcmp(hash, expected, 32) != 0) {
                std::cout << "SHA-256 longer message test failed" << std::endl;
                return false;
            }
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "SHA-256 known vectors test failed: " << e.what();
        return false;
    }
}

// Test SHA-256 incremental hashing
bool test_sha256_incremental() {
    try {
        // Test incremental hashing
        CSHA256 sha256_1;
        sha256_1.Reset();
        sha256_1.Add("Hello", 5);
        sha256_1.Add(" ", 1);
        sha256_1.Add("World", 5);
        sha256_1.Finish();

        CSHA256 sha256_2;
        sha256_2.Reset();
        sha256_2.Add("Hello World", 11);
        sha256_2.Finish();

        uint8 hash1[32], hash2[32];
        sha256_1.GetHash(hash1);
        sha256_2.GetHash(hash2);

        if (memcmp(hash1, hash2, 32) != 0) {
            std::cout << "SHA-256 incremental hashing test failed" << std::endl;
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "SHA-256 incremental test failed: " << e.what();
        return false;
    }
}

// Test SHA-256 state management
bool test_sha256_state() {
    try {
        CSHA256 sha1, sha2;

        // Test copy constructor
        sha1.Reset();
        sha1.Add("test", 4);
        CSHA256 sha_copy = sha1;

        if (sha1 != sha_copy) {
            std::cout << "SHA-256 copy constructor test failed" << std::endl;
            return false;
        }

        // Test assignment operator
        sha2.Reset();
        sha2 = sha1;

        if (sha1 != sha2) {
            std::cout << "SHA-256 assignment operator test failed" << std::endl;
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "SHA-256 state test failed: " << e.what();
        return false;
    }
}
