//
// ZLib.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
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

// CZLib makes it easier to use the zlib compression library
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CZLib
// http://getenvy.com/archives/shareazawiki/Developers.Code.CZLib.html

#pragma once

// Wraps the compress/decompress data functions of the ZLib compression library
class CZLib
{
public:
	// Compress/Decompress nInput bytes at pInput to a new returned buffer of size pnOutput:

	// After use free memory by delete[] function:
	static auto_array< BYTE > Compress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest = 0);
	static auto_array< BYTE > Decompress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput);

	// Or, after use free memory by free() function:
	static BYTE* Compress2(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest = 0);
	static BYTE* Decompress2(LPCVOID pInput, DWORD nInput, DWORD* pnOutput);
};
