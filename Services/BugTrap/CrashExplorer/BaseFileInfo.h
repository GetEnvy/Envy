/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Base file information.
 * Author: Maksim Pyatkovskiy.
 */

#pragma once

/// Base file information.
class CBaseFileInfo
{
public:
	/// Destroy the object.
	virtual ~CBaseFileInfo() { }
	/// Get file name.
	virtual const std::string& GetFileName() const = 0;
};
