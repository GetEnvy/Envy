//
// FileExecutor.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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


class CFileExecutor
{
public:
	// Is file extension safe to execute?  TRI_TRUE -safe, TRI_FALSE -dangerous, TRI_UNKNOWN -dangerous and cancel
	static TRISTATE IsSafeExecute(LPCTSTR szExt, LPCTSTR szFile = NULL);
	static TRISTATE	IsVerified(LPCTSTR szFile);
	static BOOL		Execute(LPCTSTR pszFile, LPCTSTR pszExt = NULL);
	static BOOL		Execute(const CStringList& pList);
	static BOOL		Enqueue(LPCTSTR pszFiles, LPCTSTR pszExt = NULL);
	static BOOL		Enqueue(const CStringList& pList);
//	static BOOL		ShowBitprintsTicket(DWORD nFile);	// Moved to WebServices
//	static BOOL		DisplayURL(LPCTSTR pszURL);

protected:
	// Is video, audio, or image file?
	static void DetectFileType(LPCTSTR pszFile, LPCTSTR szType, bool& bVideo, bool& bAudio, bool& bImage);

	// Extract custom mediaplayers from settings
	static CString GetCustomPlayer();
};
