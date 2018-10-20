//
// ShellIcons.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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


class CShellIcons
{
public:
	CShellIcons();
//	virtual ~CShellIcons();

public:
	void	Clear();
	int		Add(HICON hIcon, int nSize = 16);
	int		Get(LPCTSTR pszFile, int nSize = 16);
	HICON	ExtractIcon(int nIndex, int nSize = 16);
	CString	GetTypeString(LPCTSTR pszFile);
	CString	GetMIME(LPCTSTR pszType);
	CString	GetName(LPCTSTR pszType);
	void	AttachTo(CTreeCtrl* const pTree) const;						// pTree->SetImageList()
	void	AttachTo(CListCtrl* const pList, int nSize = 16) const;		// pList->SetImageList()
	BOOL	Draw(CDC* pDC, int nIcon, int nSize, int nX, int nY, COLORREF crBack = CLR_NONE, BOOL bSelected = FALSE) const;		// ImageList_DrawEx() default
	BOOL	Draw(CDC* pDC, int nIcon, int nSize, int nX, int nY, COLORREF crBack, COLORREF crBlend, UINT nStyle) const;			// ImageList_DrawEx() specific

// Obsolete: For reference & deletion
//	inline CImageList* GetObject(int nSize) const
//	{
//		switch ( nSize )
//		{
//		case 16:
//			return (CImageList*)&m_i16;
//		case 24:
//			return (CImageList*)&m_i24;
//		case 32:
//			return (CImageList*)&m_i32;
//		case 48:
//			return (CImageList*)&m_i48;
//		default:
//			return NULL;
//		}
//	}
//	inline HIMAGELIST GetHandle(int nSize) const
//	{
//		switch ( nSize )
//		{
//		case 16:
//			return m_i16.GetSafeHandle();
//		case 24:
//			return m_i24.GetSafeHandle();
//		case 32:
//			return m_i32.GetSafeHandle();
//		case 48:
//			return m_i48.GetSafeHandle();
//		default:
//			return NULL;
//		}
//	}

private:
	typedef CMap< CString, const CString&, int, int > CIconMap;
	typedef CMap< CString, const CString&, CString, const CString& > CStringMap;

	CCriticalSection	m_pSection;
	CImageList	m_i16;
//	CImageList	m_i24;
	CImageList	m_i32;
	CImageList	m_i48;
	CIconMap	m_m16;
//	CIconMap	m_m24;
	CIconMap	m_m32;
	CIconMap	m_m48;
	CStringMap	m_MIME;
	CStringMap	m_Name;

	BOOL		Lookup(LPCTSTR pszType, CString* psName, CString* psMIME, HICON* phSmallIcon, HICON* phLargeIcon = NULL, HICON* phHugeIcon = NULL);

	CShellIcons(const CShellIcons&);
	CShellIcons& operator=(const CShellIcons&);
};

extern CShellIcons ShellIcons;

// Predefined icons
enum
{
	SHI_FILE		= 0,
	SHI_EXECUTABLE	= 1,
	SHI_COMPUTER	= 2,
	SHI_FOLDER_CLOSED = 3,
	SHI_FOLDER_OPEN	= 4,
	SHI_LOCKED		= 5
};

enum
{
//	SHI_OVERLAY_NONE	= 0,
	SHI_OVERLAY_LOCKED	= 1		// Was SHI_O_LOCKED
};
