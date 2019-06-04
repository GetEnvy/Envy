//
// MetaList.h
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

#include "Schema.h"

class CMetaList;
class CMetaItem;
class CXMLElement;
class CAlbumFolder;


class CMetaList
{
public:
	CMetaList();
	virtual ~CMetaList();

protected:
	CList< CMetaItem* >	m_pItems;
	BOOL		m_bMusicBrainz;
	CBitmap		m_bmMusicBrainz;
	int			m_nHeight;

public:
	CMetaItem*	Add(LPCTSTR pszKey, LPCTSTR pszValue);
	CMetaItem*	Find(LPCTSTR pszKey) const;
	void		Remove(LPCTSTR pszKey);
	void		Clear();
	void		Vote();
	void		Shuffle();
	void		Setup(CSchemaPtr pSchema, BOOL bClear = TRUE);
	void		Setup(const CMetaList* pMetaList);				// For copying data from the external list
	void		Combine(const CXMLElement* pXML);
	void		CreateLinks();
	void		Clean(int nMaxLength = 128);
	void		ComputeWidth(CDC* pDC, int& nKeyWidth, int& nValueWidth);
	CMetaItem*	HitTest(const CPoint& point, BOOL bLinksOnly = FALSE);
	BOOL		IsMusicBrainz() const;
	BOOL		OnSetCursor(CWnd* pWnd);
	BOOL		OnClick(const CPoint& point);
	void		Paint(CDC* pDC, const CRect* prcArea);
	int			Layout(CDC* pDC, int nWidth);

// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pItems.GetHeadPosition();
	}

	inline CMetaItem* GetNext(POSITION& pos) const
	{
		return pos ? m_pItems.GetNext( pos ) : NULL;
	}

	inline INT_PTR GetCount() const
	{
		return m_pItems.GetCount();
	}

	inline CMetaItem* GetFirst() const
	{
		return m_pItems.IsEmpty() ? NULL : m_pItems.GetHead();
	}

	inline int GetHeight() const
	{
		return m_nHeight;
	}

	INT_PTR	GetCount(BOOL bVisibleOnly) const;
};


class CMetaItem : public CRect
{
public:
	CMetaItem(CSchemaMemberPtr pMember = NULL);

public:
	CSchemaMemberPtr m_pMember;
	CMap< CString, const CString&, int, int > m_pVote;
	CString			m_sKey;
	CString			m_sValue;
	BOOL			m_bValueDefined;
	BOOL			m_bLink;
	CString			m_sLink;
	CString			m_sLinkName;
	BOOL			m_bFullWidth;
	int				m_nHeight;

public:
	BOOL			Combine(const CXMLElement* pXML);
	void			Vote();
	BOOL			Limit(int nMaxLength);
	BOOL			CreateLink();
	CAlbumFolder*	GetLinkTarget(BOOL bHTTP = TRUE) const;
	CString			GetMusicBrainzLink() const;

	inline CString GetDisplayValue() const
	{
		if ( m_bLink && ! m_sLinkName.IsEmpty() )
			return m_sLinkName;
		return m_sValue;
	}
};
