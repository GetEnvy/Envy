//
// Hresults.cs
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008
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

using System;

namespace Envy
{
	internal sealed class Hresults
	{
		public const int E_FAIL = unchecked((int)0x80004005);
        public const int E_INVALIDARG = unchecked((int)0x80070057);
		public const int E_UNEXPECTED = unchecked((int)0x8000FFFF);
		public const int E_NOINTERFACE = unchecked((int)0x80004002);
		public const int E_OUTOFMEMORY = unchecked((int)0x8007000E);
		public const int E_NOTIMPL = unchecked((int)0x80004001);
		public const int S_OK = 0;
		public const int S_FALSE = 1;
	}
}
