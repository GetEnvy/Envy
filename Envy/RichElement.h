//
// RichElement.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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

class CRichDocument;
class CRichFragment;
class CRichViewCtrl;


class CRichElement
{
public:
	CRichElement(int nType = 0, LPCTSTR pszText = NULL, LPCTSTR pszLink = NULL, DWORD nFlags = 0, int nGroup = 0);
	CRichElement(HBITMAP hBitmap, LPCTSTR pszLink = NULL, DWORD nFlags = 0, int nGroup = 0);
	CRichElement(HICON hIcon, LPCTSTR pszLink = NULL, DWORD nFlags = 0, int nGroup = 0);
	virtual ~CRichElement();

public:
	CRichDocument*	m_pDocument;
	int				m_nType;
	int				m_nGroup;
	DWORD			m_nFlags;
	CString			m_sText;
	CString			m_sLink;
	HANDLE			m_hImage;
	int				m_nImageIndex;
	COLORREF		m_cColor;

public:
	void	Show(BOOL bShow = TRUE);
	void	SetText(LPCTSTR pszText);
	void	SetFlags(DWORD nFlags, DWORD nMask = 0xFFFFFFFF);
	void	Delete();

protected:
	void	PrePaint(CDC* pDC, BOOL bHover);
	void	PrePaintBitmap(CDC* pDC);
	void	PrePaintIcon(CDC* pDC);
	CSize	GetSize() const;

	friend class CRichFragment;
	friend class CRichViewCtrl;
};

enum
{
	retNull, retNewline, retGap, retAlign,
	retBitmap, retIcon, retAnchor, retCmdIcon, retEmoticon,
	retText, retLink, retHeading
};

enum
{
	retfNull		= 0x00,
	retfBold		= 0x01,
	retfItalic		= 0x02,
	retfUnderline	= 0x04,
	retfHeading		= 0x08,
	retfMiddle		= 0x10,
	retfColor		= 0x20,
	retfHidden		= 0x80
};

enum
{
	reaLeft, reaCenter, reaRight
};
