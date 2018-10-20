//
// HashLib.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2008 and PeerProject 2008
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#ifdef HASHLIB_EXPORTS
#define HASHLIB_API __declspec(dllexport)
#else
#define HASHLIB_API __declspec(dllimport)
#endif

#include "Utility.hpp"

#include "SHA.h"
#include "MD4.h"
#include "MD5.h"
#include "ED2K.h"
#include "TigerTree.h"

