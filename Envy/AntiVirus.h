//
// AntiVirus.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2014 and PeerProject 2014
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

class CAntiVirus
{
public:
	// Enum available anti-viruses and output results to ComboBox
	static void Enum(CComboBox& wndAntiVirus);

	// Free ComboBox memory allocated by Enum() method
	static void Free(CComboBox& wndAntiVirus);

	// Get user selected anti-virus from ComboBox filled by Enum() method
	static void UpdateData(CComboBox& wndAntiVirus);

	// Scan file for viruses (TRUE - ok, FALSE - infected)
	static bool Scan(LPCTSTR szPath);
};

extern CAntiVirus AntiVirus;
