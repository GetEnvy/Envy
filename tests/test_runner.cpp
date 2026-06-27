//
// test_runner.cpp
//
// Integration test runner for Envy P2P client components
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

// Test framework
class TestSuite {
public:
    struct TestResult {
        std::string name;
        bool passed;
        std::string message;
        double duration_ms;
    };

    void add_test(const std::string& name, std::function<bool()> test_func) {
        tests_.push_back({name, test_func});
    }

    bool run_all_tests() {
        std::cout << "=========================================" << std::endl;
        std::cout << "   Envy P2P Client - Integration Tests" << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << std::endl;

        int passed = 0;
        int failed = 0;
        std::vector<TestResult> results;

        for (const auto& test : tests_) {
            std::cout << "Running test: " << test.name << "... " << std::flush;

            auto start_time = std::chrono::high_resolution_clock::now();
            bool result = false;
            std::string error_msg;

            try {
                result = test.func();
            }
            catch (const std::exception& e) {
                error_msg = std::string("Exception: ") + e.what();
                result = false;
            }
            catch (...) {
                error_msg = "Unknown exception";
                result = false;
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            TestResult test_result{test.name, result, error_msg, duration};
            results.push_back(test_result);

            if (result) {
                std::cout << "✓ PASSED (" << duration << "ms)" << std::endl;
                passed++;
            } else {
                std::cout << "✗ FAILED (" << duration << "ms)" << std::endl;
                if (!error_msg.empty()) {
                    std::cout << "    Error: " << error_msg << std::endl;
                }
                failed++;
            }
        }

        std::cout << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "Test Results Summary:" << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "Total tests: " << tests_.size() << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Success rate: " << (tests_.size() > 0 ? (passed * 100.0 / tests_.size()) : 0) << "%" << std::endl;

        if (failed > 0) {
            std::cout << std::endl << "Failed tests:" << std::endl;
            for (const auto& result : results) {
                if (!result.passed) {
                    std::cout << "  - " << result.name;
                    if (!result.message.empty()) {
                        std::cout << ": " << result.message;
                    }
                    std::cout << std::endl;
                }
            }
        }

        return failed == 0;
    }

private:
    struct Test {
        std::string name;
        std::function<bool()> func;
    };

    std::vector<Test> tests_;
};

// Include test modules
#include "test_kademlia.cpp"
#include "test_aich.cpp"
#include "test_crypto.cpp"
#include "test_ed2k.cpp"
#include "test_integration.cpp"
#include "test_sha256.cpp"
#include "test_merkle_tree.cpp"

// Forward declarations for test functions
bool test_kademlia_basic_impl();
bool test_aich_basic_impl();
bool test_crypto_basic_impl();
bool test_ed2k_basic_impl();
bool test_integration_kademlia_ed2k_impl();
bool test_integration_crypto_aich_impl();
int test_merkle_tree();

// Test function implementations that call the actual test functions
bool test_kademlia_basic() { return test_kademlia_basic_impl(); }
bool test_aich_basic() { return test_aich_hash_operations() && test_aich_file_operations() && test_aich_peer_management(); }
bool test_crypto_basic() { return test_crypto_availability() && test_crypto_key_generation() && test_crypto_encryption() && test_crypto_signatures(); }
bool test_ed2k_basic() { return test_ed2k_file_identifier() && test_ed2k_multipacket() && test_ed2k_hashset() && test_ed2k_aich_integration() && test_ed2k_server_message_parsing(); }

bool test_ed2k_advanced() { return test_aich_hash() && test_ed2k_metrics(); }
bool test_integration_kademlia_ed2k() { return test_integration_kademlia_ed2k_impl(); }
bool test_integration_crypto_aich() { return test_integration_crypto_aich_impl(); }
bool test_sha256_basic() { return test_sha256_known_vectors() && test_sha256_incremental() && test_sha256_state(); }
bool test_merkle_tree_basic() { return test_merkle_tree() == 0; }

// Test suite instance
TestSuite g_test_suite;

int main() {
    // Register all tests
    g_test_suite.add_test("Kademlia DHT Basic Functionality", test_kademlia_basic);
    g_test_suite.add_test("AICHManager Basic Functionality", test_aich_basic);
    g_test_suite.add_test("CryptoProvider Basic Functionality", test_crypto_basic);
    g_test_suite.add_test("ED2K Protocol Basic Functionality", test_ed2k_basic);
    g_test_suite.add_test("ED2K Advanced Features (AICH/Metrics)", test_ed2k_advanced);
    g_test_suite.add_test("SHA-256 Hash Implementation", test_sha256_basic);
    g_test_suite.add_test("MerkleTree Basic Functionality", test_merkle_tree_basic);
    g_test_suite.add_test("Kademlia-ED2K Integration", test_integration_kademlia_ed2k);
    g_test_suite.add_test("Crypto-AICH Integration", test_integration_crypto_aich);

    // Run all tests
    bool success = g_test_suite.run_all_tests();

    std::cout << std::endl;
    if (success) {
        std::cout << "🎉 All integration tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some integration tests FAILED!" << std::endl;
        return 1;
    }
}
