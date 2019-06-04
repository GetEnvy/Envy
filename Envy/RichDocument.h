//
// RichDocument.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2012
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

class CRichElement;
class CXMLElement;

typedef CAtlMap< CString, CRichElement*, CStringRefElementTraits< CString > > CElementMap;


class CRichDocument
{
public:
	CRichDocument();
	virtual ~CRichDocument();

public:
	CCriticalSection	m_pSection;
	DWORD			m_nCookie;
	CSize			m_szMargin;
	COLORREF		m_crBackground;
	COLORREF		m_crText;
	COLORREF		m_crLink;
	COLORREF		m_crHover;
	COLORREF		m_crHeading;
	CFont			m_fntNormal;
	CFont			m_fntBold;
	CFont			m_fntItalic;
	CFont			m_fntUnder;
	CFont			m_fntBoldUnder;
	CFont			m_fntHeading;

	POSITION		GetIterator() const;
	CRichElement*	GetNext(POSITION& pos) const;
	CRichElement*	GetPrev(POSITION& pos) const;
	INT_PTR			GetCount() const;
	POSITION		Find(CRichElement* pElement) const;
	CRichElement*	Add(CRichElement* pElement, POSITION posBefore = NULL);
	CRichElement*	Add(int nType, LPCTSTR pszText, LPCTSTR pszLink = NULL, DWORD nFlags = 0, int nGroup = 0, POSITION posBefore = NULL);
	CRichElement*	Add(HBITMAP hBitmap, LPCTSTR pszLink = NULL, DWORD nFlags = 0, int nGroup = 0, POSITION posBefore = NULL);
	CRichElement*	Add(HICON hIcon, LPCTSTR pszLink = NULL, DWORD nFlags = 0, int nGroup = 0, POSITION posBefore = NULL);
	void			Remove(CRichElement* pElement);
	void			ShowGroup(int nGroup, BOOL bShow = TRUE);
	void			ShowGroupRange(int nMin, int nMax, BOOL bShow = TRUE);
	void			SetModified();
	void			Clear();
	void			CreateFonts(const LOGFONT* lpLogFont = NULL, const LOGFONT* lpHeading = NULL);
	BOOL			LoadXML(CXMLElement* pBase, CElementMap* pMap = NULL, int nGroup = 0);

protected:
	CList< CRichElement* >	m_pElements;

	BOOL			LoadXMLStyles(CXMLElement* pParent);
};
