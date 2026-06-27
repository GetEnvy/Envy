//
// test_main.cpp
//
// Test runner for Envy unit tests
// Structured output for CI integration
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
// License: GNU Affero General Public License v3.0 (AGPLv3)
//

#include "test_framework.h"

// Test modules register their tests via these functions
void register_hashlib_tests(TestSuite& suite);
void register_protocol_parser_smoke_tests(TestSuite& suite);

int main() {
	TestSuite suite;

	register_hashlib_tests(suite);
	register_protocol_parser_smoke_tests(suite);

	int failures = suite.run_all_tests();

	return (failures == 0) ? 0 : 1;
}
