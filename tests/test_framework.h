//
// test_framework.h
//
// Lightweight test framework for Envy unit tests.
// Provides TestSuite class used by test_main.cpp and test modules.
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
// License: GNU Affero General Public License v3.0 (AGPLv3)
//

#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

class TestSuite {
public:
	struct TestResult {
		std::string name;
		bool passed;
		std::string message;
		double duration_ms;
	};

	void add_test(const std::string& name, std::function<bool()> test_func) {
		m_tests.push_back({name, test_func});
	}

	int run_all_tests() {
		std::cout << "=========================================\n";
		std::cout << "   Envy Unit Tests\n";
		std::cout << "=========================================\n\n";

		int passed = 0;
		int failed = 0;
		std::vector<TestResult> results;

		for (const auto& test : m_tests) {
			std::cout << "  " << test.name << " ... " << std::flush;

			auto t0 = std::chrono::high_resolution_clock::now();
			bool ok = false;
			std::string err;

			try {
				ok = test.func();
			}
			catch (const std::exception& e) {
				err = std::string("Exception: ") + e.what();
			}
			catch (...) {
				err = "Unknown exception";
			}

			auto t1 = std::chrono::high_resolution_clock::now();
			double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

			results.push_back({test.name, ok, err, ms});

			if (ok) {
				std::cout << "PASSED (" << ms << " ms)\n";
				++passed;
			} else {
				std::cout << "FAILED (" << ms << " ms)\n";
				if (!err.empty())
					std::cout << "    -> " << err << "\n";
				++failed;
			}
		}

		std::cout << "\n=========================================\n";
		std::cout << "  Total: " << m_tests.size()
		          << "  Passed: " << passed
		          << "  Failed: " << failed << "\n";
		std::cout << "=========================================\n";

		if (failed > 0) {
			std::cout << "\nFailed tests:\n";
			for (const auto& r : results) {
				if (!r.passed) {
					std::cout << "  - " << r.name;
					if (!r.message.empty())
						std::cout << ": " << r.message;
					std::cout << "\n";
				}
			}
		}

		return failed;
	}

private:
	struct Test {
		std::string name;
		std::function<bool()> func;
	};
	std::vector<Test> m_tests;
};
