@echo off
REM run_integration_tests.bat
REM
REM Batch script to run Envy integration tests
REM
REM This file is part of Envy (getenvy.com) © 2016-2026

echo =========================================
echo    Envy P2P Client - Integration Tests
echo =========================================
echo.

if not exist "..\Envy\Release x64\Envy.exe" (
    echo ERROR: Envy.exe not found. Please build the project first.
    echo Expected location: ..\Envy\Release x64\Envy.exe
    pause
    exit /b 1
)

echo Building test runner...

REM Try to compile the test runner if cl.exe is available
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Compiling test runner with MSVC...
    cl /EHsc /I"../Envy" /I"." test_runner.cpp /Fe:test_runner.exe /link /SUBSYSTEM:CONSOLE
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo Running integration tests...
        echo.
        test_runner.exe
        set TEST_RESULT=%ERRORLEVEL%
        del test_runner.exe >nul 2>nul
    ) else (
        echo ERROR: Failed to compile test runner
        set TEST_RESULT=1
    )
) else (
    echo WARNING: MSVC compiler not found.
    echo Please run tests manually or ensure MSVC is in PATH.
    echo.
    echo To run tests manually:
    echo 1. Open Visual Studio Developer Command Prompt
    echo 2. Navigate to the tests directory
    echo 3. Compile: cl /EHsc /I"../Envy" /I"." test_runner.cpp /Fe:test_runner.exe
    echo 4. Run: test_runner.exe
    set TEST_RESULT=1
)

echo.
if %TEST_RESULT% EQU 0 (
    echo =========================================
    echo Integration tests completed successfully!
    echo =========================================
) else (
    echo =========================================
    echo Integration tests failed!
    echo =========================================
)

echo.
echo Press any key to continue...
pause >nul
