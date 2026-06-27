# CMakeLists.txt for Envy P2P File Sharing Application
# Modern CMake build system for Envy project
#
# This is an initial CMake configuration. Additional work is needed to fully
# migrate from Visual Studio project files to CMake.

cmake_minimum_required(VERSION 3.20)
project(Envy VERSION 4.0.0 LANGUAGES CXX)

# Set C++ standard (targeting C++17 for modernization)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Windows-specific settings
if(WIN32)
    # Set Windows SDK version
    set(CMAKE_SYSTEM_VERSION 10.0)

    # Visual Studio 2026 toolset
    if(MSVC)
        set(CMAKE_GENERATOR_TOOLSET "v145" CACHE STRING "Platform Toolset" FORCE)
    endif()

    # Use static runtime
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Build configurations
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Services
    ${CMAKE_SOURCE_DIR}/Envy
    ${CMAKE_SOURCE_DIR}/HashLib
)

# Find required packages
find_package(ZLIB REQUIRED)

# Options
option(BUILD_TESTS "Build test programs" OFF)
option(ENVY_USE_ASM "Enable ASM optimizations" ON)

# Add tests subdirectory if enabled
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Preprocessor definitions
add_definitions(-D_ENVY)
add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)

if(WIN32)
    add_definitions(-DWIN32 -D_WINDOWS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

if(ENVY_USE_ASM)
    add_definitions(-DENVY_USE_ASM)
endif()

# Source groups for better IDE organization
source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})

# Add subdirectories
add_subdirectory(HashLib)

# TODO: Add remaining components
# - Services (SQLite, zlib, etc.)
# - Core Envy application
# - Plugins
# - MFC configuration
# - Plugin system

# Example structure (to be expanded):
# add_library(zlibwapi SHARED ...)     # Services/zlib
# add_library(sqlite3 SHARED ...)      # Services/SQLite
# add_executable(Envy ...)             # Main application
# target_link_libraries(Envy HashLib zlibwapi sqlite3 ${ZLIB_LIBRARIES})

# Print configuration summary
message(STATUS "")
message(STATUS "=== Envy Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(WIN32)
    message(STATUS "Windows SDK: ${CMAKE_SYSTEM_VERSION}")
    if(MSVC)
        message(STATUS "MSVC Toolset: ${CMAKE_GENERATOR_TOOLSET}")
    endif()
endif()
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "ASM optimizations: ${ENVY_USE_ASM}")
message(STATUS "================================")
message(STATUS "")
