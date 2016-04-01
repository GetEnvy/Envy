//
// XML.inl
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2008
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

//////////////////////////////////////////////////////////////////////
// CXMLNode node type and casting access

inline int CXMLNode::GetType() const
{
	return m_nNode;
}

inline CXMLNode* CXMLNode::AsNode() const
{
	return (CXMLNode*)this;
}

inline CXMLElement* CXMLNode::AsElement() const
{
	return ( m_nNode == xmlElement ) ? (CXMLElement*)this : NULL;
}

inline CXMLAttribute* CXMLNode::AsAttribute() const
{
	return ( m_nNode == xmlAttribute ) ? (CXMLAttribute*)this : NULL;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode parent access and delete

inline CXMLElement* CXMLNode::GetParent() const
{
	return m_pParent;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode name access

inline CString CXMLNode::GetName() const
{
	ASSERT( ! m_sName.IsEmpty() );
	return m_sName;
}

inline void CXMLNode::SetName(LPCTSTR pszName)
{
	ASSERT( pszName && *pszName );
	m_sName = pszName;
}

inline BOOL CXMLNode::IsNamed(LPCTSTR pszName) const
{
	ASSERT( pszName && *pszName );
	if ( this == NULL ) return FALSE;
	return m_sName.CompareNoCase( pszName ) == 0;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode value access

inline CString CXMLNode::GetValue() const
{
	return m_sValue;
}

inline void CXMLNode::SetValue(const CString& strValue)
{
	m_sValue = strValue;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement detach

inline CXMLElement* CXMLElement::Detach()
{
	if ( m_pParent ) m_pParent->RemoveElement( this );
	m_pParent = NULL;
	return this;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement element access

inline CXMLElement* CXMLElement::AddElement(CXMLElement* pElement)
{
	if ( pElement->m_pParent ) return NULL;
	m_pElements.AddTail( pElement );
	pElement->m_pParent = this;
	return pElement;
}

inline INT_PTR CXMLElement::GetElementCount() const
{
	return m_pElements.GetCount();
}

inline CXMLElement* CXMLElement::GetFirstElement() const
{
	if ( this == NULL ) return NULL;
	return m_pElements.GetCount() ? m_pElements.GetHead() : NULL;
}

inline POSITION CXMLElement::GetElementIterator() const
{
	return m_pElements.GetHeadPosition();
}

inline CXMLElement* CXMLElement::GetNextElement(POSITION& pos) const
{
	return m_pElements.GetNext( pos );
}

inline CXMLElement* CXMLElement::GetElementByName(LPCTSTR pszName) const
{
	ASSERT( pszName && *pszName );
	for ( POSITION pos = GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = GetNextElement( pos );
		if ( pElement->GetName().CompareNoCase( pszName ) == 0 ) return pElement;
	}
	return NULL;
}

inline CXMLElement* CXMLElement::GetElementByName(LPCTSTR pszName, BOOL bCreate)
{
	ASSERT( pszName && *pszName );
	for ( POSITION pos = GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = GetNextElement( pos );
		if ( pElement->GetName().CompareNoCase( pszName ) == 0 ) return pElement;
	}

	return bCreate ? AddElement( pszName ) : NULL;
}

inline void CXMLElement::RemoveElement(CXMLElement* pElement)
{
	POSITION pos = m_pElements.Find( pElement );
	if ( pos ) m_pElements.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CXMLElement attribute access

inline int CXMLElement::GetAttributeCount() const
{
	return (int)m_pAttributes.GetCount();
}

inline POSITION CXMLElement::GetAttributeIterator() const
{
	//ASSERT( m_pAttributes.GetCount() == m_pAttributesInsertion.GetCount() );

	return m_bOrdered ?
		m_pAttributesInsertion.GetHeadPosition() :		// Track output order workaround	(Broken Collection creation)
		m_pAttributes.GetStartPosition();				// Legacy unordered
}

inline CXMLAttribute* CXMLElement::GetNextAttribute(POSITION& pos) const
{
	CXMLAttribute* pAttribute;
	CString strName;

	m_bOrdered ?
		m_pAttributes.Lookup( m_pAttributesInsertion.GetNext( pos ), pAttribute ) :	// Track output order workaround	(Broken Collection creation)
		m_pAttributes.GetNextAssoc( pos, strName, pAttribute );						// Legacy unordered

	return pAttribute;
}

inline CXMLAttribute* CXMLElement::GetAttribute(LPCTSTR pszName) const
{
	ASSERT( pszName && *pszName );
	CXMLAttribute* pAttribute;
	CString strNameLower( pszName );
	strNameLower.MakeLower();

	return m_pAttributes.Lookup( strNameLower, pAttribute ) ? pAttribute : NULL;
}

inline CString CXMLElement::GetAttributeValue(LPCTSTR pszName, LPCTSTR pszDefault) const
{
	ASSERT( pszName && *pszName );
	if ( const CXMLAttribute* pAttribute = GetAttribute( pszName ) )
		return pAttribute->m_sValue;

	return pszDefault ? pszDefault : CString();
}

inline void CXMLElement::RemoveAttribute(CXMLAttribute* pAttribute)
{
	CString strNameLower( pAttribute->m_sName );
	strNameLower.MakeLower();

	m_pAttributes.RemoveKey( strNameLower );

	if ( m_pAttributesInsertion.GetCount() )		// Tracking output order
		m_pAttributesInsertion.RemoveAt( m_pAttributesInsertion.Find( strNameLower ) );
}

inline void CXMLElement::DeleteAttribute(LPCTSTR pszName)
{
	ASSERT( pszName && *pszName );
	CXMLAttribute* pAttribute = GetAttribute( pszName );
	if ( ! pAttribute ) return;

	CString strNameLower( pAttribute->m_sName );
	strNameLower.MakeLower();

	pAttribute->Delete();

	if ( m_pAttributesInsertion.GetCount() )	// Tracking output order workaround
		m_pAttributesInsertion.RemoveAt( m_pAttributesInsertion.Find( strNameLower ) );
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute name access

inline void CXMLAttribute::SetName(LPCTSTR pszName)
{
	ASSERT( pszName && *pszName );
	m_sName = pszName;

	if ( CXMLElement* pParent = m_pParent )
	{
		pParent->RemoveAttribute( this );
		m_pParent = NULL;
		pParent->AddAttribute( this );
	}
}
