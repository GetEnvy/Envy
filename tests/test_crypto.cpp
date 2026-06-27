//
// test_crypto.cpp
//
// Integration tests for CryptoProvider (RSA cryptography)
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "../Envy/CryptoProvider.h"
#include <iostream>

// Test basic cryptographic availability
bool test_crypto_availability() {
    try {
        if (!CCryptoProvider::IsCryptoAvailable()) {
            throw std::runtime_error("Cryptographic functions not available on this system");
        }
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Crypto availability test failed: " << e.what();
        return false;
    }
}

// Test RSA key pair generation
bool test_crypto_key_generation() {
    try {
        if (!CCryptoProvider::IsCryptoAvailable()) {
            std::cout << "Skipping key generation test (crypto not available)";
            return true;
        }

        CCryptoProvider crypto;

        if (!crypto.GenerateRSAKeyPair()) {
            throw std::runtime_error("Failed to generate RSA key pair");
        }

        // Test public key retrieval
        const BYTE* publicKey = crypto.GetPublicKey();
        size_t keyLen = crypto.GetPublicKeyLen();

        if (!publicKey || keyLen == 0) {
            throw std::runtime_error("Failed to retrieve public key");
        }

        if (keyLen < 100) { // RSA keys should be reasonably large
            throw std::runtime_error("Public key too small");
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Crypto key generation test failed: " << e.what();
        return false;
    }
}

// Test encryption and decryption
bool test_crypto_encryption() {
    try {
        if (!CCryptoProvider::IsCryptoAvailable()) {
            std::cout << "Skipping encryption test (crypto not available)";
            return true;
        }

        CCryptoProvider crypto;

        if (!crypto.GenerateRSAKeyPair()) {
            throw std::runtime_error("Failed to generate RSA key pair for encryption test");
        }

        // Get public key for encryption
        const BYTE* publicKey = crypto.GetPublicKey();
        size_t publicKeyLen = crypto.GetPublicKeyLen();

        if (!publicKey || publicKeyLen == 0) {
            throw std::runtime_error("No public key available for encryption");
        }

        // Test data
        const char* testMessage = "This is a test message for RSA encryption/decryption.";
        size_t messageLen = strlen(testMessage);

        // Encrypt data
        BYTE encrypted[1024];
        size_t encryptedLen = sizeof(encrypted);

        if (!crypto.EncryptWithPublicKey((const BYTE*)testMessage, messageLen,
                                       encrypted, encryptedLen, publicKey, publicKeyLen)) {
            throw std::runtime_error("Failed to encrypt data");
        }

        if (encryptedLen == messageLen) {
            throw std::runtime_error("Encrypted data should be different length than original");
        }

        // Decrypt data
        BYTE decrypted[1024];
        size_t decryptedLen = sizeof(decrypted);

        if (!crypto.DecryptWithPrivateKey(encrypted, encryptedLen, decrypted, decryptedLen)) {
            throw std::runtime_error("Failed to decrypt data");
        }

        // Verify decrypted data matches original
        if (decryptedLen != messageLen ||
            memcmp(decrypted, testMessage, messageLen) != 0) {
            throw std::runtime_error("Decrypted data does not match original");
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Crypto encryption test failed: " << e.what();
        return false;
    }
}

// Test digital signatures
bool test_crypto_signatures() {
    try {
        if (!CCryptoProvider::IsCryptoAvailable()) {
            std::cout << "Skipping signature test (crypto not available)";
            return true;
        }

        CCryptoProvider crypto;

        if (!crypto.GenerateRSAKeyPair()) {
            throw std::runtime_error("Failed to generate RSA key pair for signature test");
        }

        // Get public key for verification
        const BYTE* publicKey = crypto.GetPublicKey();
        size_t publicKeyLen = crypto.GetPublicKeyLen();

        if (!publicKey || publicKeyLen == 0) {
            throw std::runtime_error("No public key available for signature verification");
        }

        // Test data
        const char* testData = "This is test data to be signed.";
        size_t dataLen = strlen(testData);

        // Sign data
        BYTE signature[1024];
        size_t signatureLen = sizeof(signature);

        if (!crypto.SignData((const BYTE*)testData, dataLen, signature, signatureLen)) {
            throw std::runtime_error("Failed to sign data");
        }

        // Verify signature
        if (!crypto.VerifySignature((const BYTE*)testData, dataLen,
                                  signature, signatureLen, publicKey, publicKeyLen)) {
            throw std::runtime_error("Signature verification failed");
        }

        // Test with modified data (should fail verification)
        const char* modifiedData = "This is modified test data.";
        if (crypto.VerifySignature((const BYTE*)modifiedData, strlen(modifiedData),
                                 signature, signatureLen, publicKey, publicKeyLen)) {
            throw std::runtime_error("Signature verification should have failed for modified data");
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Crypto signatures test failed: " << e.what();
        return false;
    }
}
