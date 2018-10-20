//
// SchemaMember.h
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

class CSchema;
class CXMLElement;


class CSchemaMember
{
public:
	CSchemaMember(CSchema* pSchema);
	//virtual ~CSchemaMember();

public:
	enum Format
	{
		smfNone, smfTimeMMSS, smfBitrate, smfFrequency, smfTimeHHMMSSdec
	};

	CSchema*	m_pSchema;
	CString		m_sName;
	CString		m_sType;
	CString		m_sTitle;
	BOOL		m_bElement;
	BOOL		m_bNumeric;
	BOOL		m_bGUID;
	BOOL		m_bYear;
	BOOL		m_bIndexed;
	BOOL		m_bSearched;
	BOOL		m_bHidden;
	BOOL		m_bBoolean;

	int			m_nMinOccurs;
	int			m_nMaxOccurs;
	int			m_nMaxLength;

	BOOL		m_bPrompt;
	Format		m_nFormat;
	int			m_nColumnWidth;
	int			m_nColumnAlign;

	CString		m_sLinkURI;
	CString		m_sLinkName;

	CList< CString > m_pItems;

public:
	POSITION	GetItemIterator() const;
	CString		GetNextItem(POSITION& pos) const;
	INT_PTR		GetItemCount() const { return m_pItems.GetCount(); }
	CString		GetValueFrom(const CXMLElement* pElement, LPCTSTR pszDefault = NULL, BOOL bFormat = FALSE, BOOL bNoValidation = FALSE) const;
	void		SetValueTo(CXMLElement* pBase, LPCTSTR pszValue);

protected:
	BOOL		LoadSchema(const CXMLElement* pRoot, const CXMLElement* pElement);
	BOOL		LoadType(const CXMLElement* pType);
	BOOL		LoadDescriptor(const CXMLElement* pXML);
	BOOL		LoadDisplay(const CXMLElement* pXML);

	friend class CSchema;

private:
	CSchemaMember(const CSchemaMember&);
	CSchemaMember& operator=(const CSchemaMember&);
};


//#ifdef WIN64
//
//template<>
//AFX_INLINE UINT AFXAPI HashKey( CSchemaMemberPtr key )
//{
//	return HashKey< __int64 >( (__int64)key );
//}
//
//#endif // WIN64
