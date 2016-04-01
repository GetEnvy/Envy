//
// stdafx.h
//

#pragma once

// For /Wall
#pragma warning(disable:4668)
#pragma warning(disable:4820)
#pragma warning(disable:4548)
#pragma warning(disable:4710)	// 'inline' function not inlined
#pragma warning(disable:4986)	// operator new[]/delete[] previous declaration

#define NTDDI_VERSION	0x05010200	// NTDDI_WINXPSP2
#define _WIN32_WINNT	0x0501
#include <sdkddkver.h>

#define STRICT
#define WIN32_LEAN_AND_MEAN
//#define BOOST_USE_WINDOWS_H

#include <windows.h>
#include <algorithm>

// Define HASHLIB_USE_ASM for win32 assembler use (several times faster)
