//
// UPnP.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2014 and Shareaza 2014
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

class CUPnP
{
public:
	CUPnP() {}
	virtual ~CUPnP() {}

public:
	virtual void DeletePorts() = 0;
	virtual void StartDiscovery() = 0;
	virtual void StopAsyncFind() = 0;
	virtual bool IsAsyncFindRunning() = 0;
};
