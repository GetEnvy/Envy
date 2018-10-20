//
// Buffer.h
//
// This file is part of Torrent Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2007 and PeerProject 2008
//
// Envy is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once


class CBuffer
{
// Construction
public:
	CBuffer(DWORD* pLimit = NULL);
	virtual ~CBuffer();

// Attributes
public:
	CBuffer*	m_pNext;
	BYTE*		m_pBuffer;
	DWORD		m_nLength;
	DWORD		m_nBuffer;

// Operations
public:
	LPVOID	Allocate(DWORD nLength);
	void	Add(const void* pData, DWORD nLength);
	void	Insert(DWORD nOffset, const void* pData, DWORD nLength);
	void	Remove(DWORD nLength);
	void	Clear();
	void	Print(LPCSTR pszText);
	DWORD	Append(CBuffer* pBuffer, DWORD nLength = 0xFFFFFFFF);
	BOOL	ReadLine(CString& strLine, BOOL bPeek = FALSE);

// Extras
public:
#ifdef _WINSOCKAPI_
	DWORD	Receive(SOCKET hSocket);
	DWORD	Send(SOCKET hSocket);
#endif
#ifdef _DEFLATE_
	BOOL	Deflate(BOOL bIfSmaller = FALSE);
	BOOL	Inflate(DWORD nSuggest = 0);
#endif

};

