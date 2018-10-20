//
// XML.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2015
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

class CXMLNode;
class CXMLElement;
class CXMLAttribute;


class CXMLNode
{
public:
	CXMLNode(CXMLElement* pParent = NULL, LPCTSTR pszName = NULL);
	virtual ~CXMLNode();

protected:
	enum {
		xmlNode,
		xmlElement,
		xmlAttribute
	};

	int				m_nNode;
	CXMLElement*	m_pParent;
	CString			m_sName;
	CString			m_sValue;

	static BOOL		ParseMatch(LPCTSTR& pszXML, LPCTSTR pszToken);
	static BOOL		ParseIdentifier(LPCTSTR& pszXML, CString& strIdentifier);
	void			Serialize(CArchive& ar);

public:
	void			Delete();
	int				GetType() const;
	CXMLNode*		AsNode() const;
	CXMLElement*	AsElement() const;
	CXMLAttribute*	AsAttribute() const;
	CXMLElement*	GetParent() const;
	CString			GetName() const;
	void			SetName(LPCTSTR pszValue);
	BOOL			IsNamed(LPCTSTR pszName) const;
	CString			GetValue() const;
	void			SetValue(const CString& strValue);
	static void		UniformString(CString& str);

	friend class	CXMLElement;
	friend class	CQuerySearch;
	friend class	CXMLCOM;
};


class CXMLElement : public CXMLNode
{
public:
	CXMLElement(CXMLElement* pParent = NULL, LPCTSTR pszName = NULL);
	virtual ~CXMLElement();

public:
	BOOL			m_bOrdered;						// Track attribute insertion order? (Workaround. ToDo: Fix properly)

protected:
	CList< CXMLElement* > m_pElements;
	CList< CString > m_pAttributesInsertion;		// Track insertion order (parallel workaround for deterministic file output)
	CMap< CString, const CString&, CXMLAttribute*, CXMLAttribute* > m_pAttributes;

	void			AddRecursiveWords(CString& strWords) const;
	void			ToString(CString& strXML, BOOL bNewline) const;

public:
	CXMLElement*	Detach();
	CXMLElement*	Clone(CXMLElement* pParent = NULL) const;
	CXMLElement*	Prefix(const CString& sPrefix, CXMLElement* pParent = NULL) const;		// Clone element then rename all elements and attributes by using specified prefix
	CXMLElement*	AddElement(LPCTSTR pszName);
	CXMLElement*	AddElement(CXMLElement* pElement);
	INT_PTR			GetElementCount() const;
	CXMLElement*	GetFirstElement() const;
	POSITION		GetElementIterator() const;
	CXMLElement*	GetNextElement(POSITION& pos) const;
	CXMLElement*	GetElementByName(LPCTSTR pszName) const;
	CXMLElement*	GetElementByName(LPCTSTR pszName, BOOL bCreate);
	void			RemoveElement(CXMLElement* pElement);
	void			DeleteAllElements();
	CXMLAttribute*	AddAttribute(LPCTSTR pszName, LPCTSTR pszValue = NULL);
	CXMLAttribute*	AddAttribute(LPCTSTR pszName, __int64 nValue);
	CXMLAttribute*	AddAttribute(CXMLAttribute* pAttribute);
	int				GetAttributeCount() const;
	POSITION		GetAttributeIterator() const;
	CXMLAttribute*	GetNextAttribute(POSITION& pos) const;
	CXMLAttribute*	GetAttribute(LPCTSTR pszName) const;
	CString			GetAttributeValue(LPCTSTR pszName, LPCTSTR pszDefault = NULL) const;
	void			RemoveAttribute(CXMLAttribute* pAttribute);
	void			DeleteAttribute(LPCTSTR pszName);
	void			DeleteAllAttributes();
	CString			ToString(BOOL bHeader = FALSE, BOOL bNewline = FALSE, BOOL bEncoding = FALSE, TRISTATE bStandalone = TRI_UNKNOWN) const;
	BOOL			ParseString(LPCTSTR& strXML);
	BOOL			Equals(CXMLElement* pXML) const;
	// Add missing elements and attributes from pInput, preserve or overwrite existing
	BOOL			Merge(const CXMLElement* pInput, BOOL bOverwrite = FALSE);
	CString			GetRecursiveWords() const;
	void			Serialize(CArchive& ar);

	static CXMLElement*	FromString(LPCTSTR pszXML, BOOL bHeader = FALSE, CString* pEncoding = NULL);
	static CXMLElement* FromBytes(BYTE* pByte, DWORD nByte, BOOL bHeader = FALSE);
	static CXMLElement* FromFile(LPCTSTR pszPath, BOOL bHeader = FALSE);
	static CXMLElement* FromFile(HANDLE hFile, BOOL bHeader = FALSE);
};


class CXMLAttribute : public CXMLNode
{
public:
	CXMLAttribute(CXMLElement* pParent, LPCTSTR pszName = NULL);
	virtual ~CXMLAttribute();

public:
	static LPCTSTR	xmlnsSchema;
	static LPCTSTR	xmlnsInstance;
	static LPCTSTR	schemaName;

	CXMLAttribute*	Clone(CXMLElement* pParent = NULL) const;
	void			ToString(CString& strXML) const;
	BOOL			ParseString(LPCTSTR& strXML);
	BOOL			Equals(CXMLAttribute* pXML) const;
	void			Serialize(CArchive& ar);
	virtual void	SetName(LPCTSTR pszValue);
};

#include "XML.inl"
