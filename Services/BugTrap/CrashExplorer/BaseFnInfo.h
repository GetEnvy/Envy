/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Base function information.
 * Author: Maksim Pyatkovskiy.
 */

#pragma once

/// Base function information.
class CBaseFnInfo
{
public:
	/// Destroy the object.
	virtual ~CBaseFnInfo() { }
	/// Get function name.
	virtual std::string GetName() const = 0;
	/// Get function address.
	virtual PVOID GetAddress() const = 0;
};
