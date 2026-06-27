# Testing Guide

## Current Test Surface
- Primary test project: `tests/EnvyTests.vcxproj`
- Additional standalone test executables exist in `tests/` (`test_runner.cpp`, simple integration binaries)
- Current coverage is strongest in hashing/crypto-related paths

## Running Tests (Visual Studio)
1. Build `EnvyTests` for chosen config/platform.
2. Run `tests/<Config> <Platform>/EnvyTests.exe`.

## Running Tests (CMake path)
```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

## Writing New Tests
- Place new test source files under `tests/`.
- Register test entry in the existing test framework/main.
- Prefer deterministic tests that avoid network flakiness.
- For protocol logic, isolate parser/state transitions from live network where possible.

## Coverage Targets (Proposed)
- Hashing/core utilities: maintain high coverage.
- Protocol handlers: add targeted unit tests for packet parse/serialize logic.
- Persistence/config migration: add regression tests for settings and schema changes.
