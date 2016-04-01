//
// ThumbCache.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2008
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

class CImageFile;

class CThumbCache
{
public:
	static void InitDatabase();
	static BOOL	Load(LPCTSTR pszPath, CImageFile* pImage);
	static void Delete(LPCTSTR pszPath);
	static BOOL	Store(LPCTSTR pszPath, CImageFile* pImage);
	static BOOL Cache(LPCTSTR pszPath, CImageFile* pImage = NULL, BOOL bLoadFromFile = TRUE);
};
