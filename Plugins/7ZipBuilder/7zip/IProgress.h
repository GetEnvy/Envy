//
// IProgress.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008 and 7Zip (7-zip.org)
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#ifndef __IPROGRESS_H
#define __IPROGRESS_H

#include "MyUnknown.h"
#include "Types.h"

#include "IDecl.h"

#define INTERFACE_IProgress(x) \
  STDMETHOD(SetTotal)(UInt64 total) x; \
  STDMETHOD(SetCompleted)(const UInt64 *completeValue) x; \

DECL_INTERFACE(IProgress, 0, 5)
{
  INTERFACE_IProgress(PURE)
};

/*
// {23170F69-40C1-278A-0000-000000050002}
DEFINE_GUID(IID_IProgress2,
0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02);
MIDL_INTERFACE("23170F69-40C1-278A-0000-000000050002")
IProgress2: public IUnknown
{
public:
  STDMETHOD(SetTotal)(const UInt64 *total) PURE;
  STDMETHOD(SetCompleted)(const UInt64 *completeValue) PURE;
};
*/

#endif
