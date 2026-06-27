# CMakeLists.txt for HashLib
# Hash algorithm library (MD4, MD5, SHA, ED2K, Tiger Tree)

cmake_minimum_required(VERSION 3.20)

project(HashLib VERSION 1.0.0 LANGUAGES CXX ASM_MASM)

# HashLib source files
set(HASHLIB_SOURCES
    ED2K.cpp
    HashLib.cpp
    MD4.cpp
    MD5.cpp
    SHA.cpp
    TigerTree.cpp
    stdafx.cpp
)

# HashLib header files
set(HASHLIB_HEADERS
    ED2K.h
    HashLib.h
    MD4.h
    MD5.h
    Resource.h
    SHA.h
    stdafx.h
    TigerTree.h
    Utility.hpp
)

# ASM files (Win32 only)
set(HASHLIB_ASM_WIN32
    MD4.32.asm
    MD5.32.asm
    SHA.32.asm
    Tiger.32.asm
)

# Resource file
set(HASHLIB_RESOURCES
    HashLib.rc
)

# Create HashLib library
add_library(HashLib SHARED
    ${HASHLIB_SOURCES}
    ${HASHLIB_HEADERS}
    ${HASHLIB_RESOURCES}
)

# ASM files conditionally (Win32 only)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86" OR CMAKE_SIZEOF_VOID_P EQUAL 4)
    enable_language(ASM_MASM)
    target_sources(HashLib PRIVATE ${HASHLIB_ASM_WIN32})
    target_compile_definitions(HashLib PRIVATE HASHLIB_USE_ASM)
endif()

# Include directories
target_include_directories(HashLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Services>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Preprocessor definitions
target_compile_definitions(HashLib
    PRIVATE
        HASHLIB_EXPORTS
        _SCL_SECURE_NO_WARNINGS
    PUBLIC
        _HASHLIB
)

# Compiler options
target_compile_options(HashLib PRIVATE
    /W4              # Warning level 4
    /WX-             # Don't treat warnings as errors (for now)
    /MP              # Multi-processor compilation
)

# Link options
target_link_options(HashLib PRIVATE
    /LARGEADDRESSAWARE
    /BASE:0x29100000
)

# Windows-specific settings
if(WIN32)
    target_compile_definitions(HashLib PRIVATE
        WIN32
        _WINDOWS
        _USRDLL
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_compile_definitions(HashLib PRIVATE _ATL_XP_TARGETING)
        target_link_options(HashLib PRIVATE /SAFESEH:NO)
    endif()
endif()

# Precompiled headers
target_precompile_headers(HashLib
    PRIVATE
        <stdafx.h>
)

# Output directory
set_target_properties(HashLib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Install rules (optional)
install(TARGETS HashLib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HASHLIB_HEADERS}
    DESTINATION include/HashLib
)
