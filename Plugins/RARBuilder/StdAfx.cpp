//
// stdafx.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008
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

// stdafx.cpp source file includes just the standard Includes.
// RARBuilder.pch will be the pre-compiled header.
// stdafx.obj will contain the pre-compiled type information.

#include "stdafx.h"

tRAROpenArchiveEx fnRAROpenArchiveEx = NULL;
tRARCloseArchive fnRARCloseArchive = NULL;
tRARReadHeaderEx fnRARReadHeaderEx = NULL;
tRARProcessFileW fnRARProcessFileW = NULL;
