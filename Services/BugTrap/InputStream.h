/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Input stream.
 * Author: Maksim Pyatkovskiy.
 */

#pragma once

#include "BaseStream.h"

/// Input stream.
class CInputStream : public virtual CBaseStream
{
public:
	/// Return true if end of stream has been reached. This function is required.
	virtual bool IsEndOfStream(void) const = 0;
	/// Read one byte from the stream. This function is required.
	virtual bool ReadByte(unsigned char& bValue) = 0;
	/// Read array of bytes from the stream. This function is optional.
	virtual size_t ReadBytes(unsigned char* arrBytes, size_t nCount);
};
