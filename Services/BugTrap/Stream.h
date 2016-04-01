/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Input & output stream.
 * Author: Maksim Pyatkovskiy.
 */

#pragma once

#include "InputStream.h"
#include "OutputStream.h"

class CStream : public CInputStream, public COutputStream
{
};
