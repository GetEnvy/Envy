//
// ZIPFile.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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

class CBuffer;


class CZIPFile
{
public:
	CZIPFile(HANDLE hAttach = INVALID_HANDLE_VALUE);
	~CZIPFile();

// File Class
public:
	class File
	{
	private:
		friend class CZIPFile;
		inline File() {};
		CZIPFile*	m_pZIP;
	public:
		CBuffer*	Decompress();
		BOOL		Extract(LPCTSTR pszFile);
	public:
		CString		m_sName;
		QWORD		m_nSize;
	protected:
		QWORD		m_nLocalOffset;
		QWORD		m_nCompressedSize;
		int			m_nCompression;
		BOOL		PrepareToDecompress(LPVOID pStream);
	};

protected:
	BOOL	m_bAttach;
	HANDLE	m_hFile;
	File*	m_pFile;
	int		m_nFile;

public:
	BOOL	Open(LPCTSTR pszFile);
	BOOL	Attach(HANDLE hFile);
	BOOL	IsOpen() const;
	void	Close();

	int		GetCount() const;
	File*	GetFile(int nFile) const;
	File*	GetFile(LPCTSTR pszFile, BOOL bPartial = FALSE) const;
protected:
	BOOL	LocateCentralDirectory();
	BOOL	ParseCentralDirectory(BYTE* pDirectory, DWORD nDirectory);
	BOOL	SeekToFile(File* pFile);
};
