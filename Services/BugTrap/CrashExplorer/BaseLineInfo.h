/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Base line information.
 * Author: Maksim Pyatkovskiy.
 */

#pragma once

/// Base line information.
class CBaseLineInfo
{
public:
	/// Destroy the object.
	virtual ~CBaseLineInfo() { }
	/// Get line offset.
	virtual PVOID GetAddress() const = 0;
	/// Get line number.
	virtual UINT GetNumber() const = 0;
};
