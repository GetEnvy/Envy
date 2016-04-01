/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Memory leak detection macros.
 * Author: Maksim Pyatkovskiy.
 */

#pragma once

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#ifdef _DEBUG
 #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
 #define DEBUG_NEW new
#endif
