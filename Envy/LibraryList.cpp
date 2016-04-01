//
// LibraryList.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2006
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

#include "StdAfx.h"
#include "Envy.h"
#include "LiveList.h"
#include "LibraryList.h"
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "Application.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CLibraryListItem

CLibraryListItem::CLibraryListItem()
	: Type( CLibraryListItem::Empty )
	, pAlbumFolder( NULL )
{
}

CLibraryListItem::CLibraryListItem(DWORD val)
	: Type( CLibraryListItem::LibraryFile )
	, dwLibraryFile( val )
{
	ASSERT( val > 0 && val < 0x00100000 );
}

CLibraryListItem::CLibraryListItem(CAlbumFolder* val)
	: Type( CLibraryListItem::AlbumFolder )
	, pAlbumFolder( val )
{
}

CLibraryListItem::CLibraryListItem(CLibraryFolder* val)
	: Type( CLibraryListItem::LibraryFolder )
	, pLibraryFolder( val )
{
}

CLibraryListItem::CLibraryListItem(const CLibraryFile* val)
	: Type( CLibraryListItem::LibraryFile )
	, dwLibraryFile( val->m_nIndex )
{
	ASSERT_VALID( val );
}

CLibraryListItem::CLibraryListItem(const CLibraryListItem& val)
	: Type( val.Type )
	, pAlbumFolder( val.pAlbumFolder )
{
}

CLibraryListItem::operator DWORD () const
{
	ASSERT( Type == CLibraryListItem::LibraryFile );
	if ( Type == CLibraryListItem::LibraryFile )
		return dwLibraryFile;
	return 0;		// Error
}

CLibraryListItem::operator CLibraryFile* () const
{
	ASSERT( Type == CLibraryListItem::LibraryFile );
	if ( Type == CLibraryListItem::LibraryFile )
		return Library.LookupFile( dwLibraryFile );
	return 0;		// File was removed or error happened
}

CLibraryListItem::operator CAlbumFolder* () const
{
	ASSERT( Type == CLibraryListItem::AlbumFolder );
	if ( Type == CLibraryListItem::AlbumFolder )
		return pAlbumFolder;
	return NULL;	// Error
}

CLibraryListItem::operator CLibraryFolder* () const
{
	ASSERT( Type == CLibraryListItem::LibraryFolder );
	if ( Type == CLibraryListItem::LibraryFolder )
		return pLibraryFolder;
	return NULL;	// Error
}

bool CLibraryListItem::operator == (const CLibraryListItem& val) const
{
	if ( Type == val.Type )
	{
		switch ( Type )
		{
		case CLibraryListItem::Empty:
			return TRUE;	// Both Empty

		case CLibraryListItem::LibraryFile:
			// Same file library number
			return dwLibraryFile == val.dwLibraryFile;

		case CLibraryListItem::AlbumFolder:
			// Same object, or same parent album and same album name
			return ( pAlbumFolder == val.pAlbumFolder ) ||
				( pAlbumFolder->GetParent() == val.pAlbumFolder->GetParent() &&
				  pAlbumFolder->m_sName == val.pAlbumFolder->m_sName );

		case CLibraryListItem::LibraryFolder:
			// Same object, or same path
			return ( pLibraryFolder == val.pLibraryFolder ) ||
				( pLibraryFolder->m_sPath == val.pLibraryFolder->m_sPath );
		}
	}
	return false;
}

bool CLibraryListItem::operator != (const CLibraryListItem& val) const
{
	return ! ( *this == val );
}

CLibraryListItem& CLibraryListItem::operator = (const CLibraryListItem& val)
{
	Type = val.Type;
	pAlbumFolder = val.pAlbumFolder;
	return *this;
}


//////////////////////////////////////////////////////////////////////
// CLibraryList

IMPLEMENT_DYNAMIC(CLibraryList, CComObject)

BEGIN_INTERFACE_MAP(CLibraryList, CComObject)
	INTERFACE_PART(CLibraryList, IID_IGenericView, GenericView)
	INTERFACE_PART(CLibraryList, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()

CLibraryList::CLibraryList()
{
	m_dwRef = 0;
}

CLibraryList::~CLibraryList()
{
}

//////////////////////////////////////////////////////////////////////
// CLibraryList file access

CLibraryFile* CLibraryList::GetNextFile(POSITION& pos)
{
	ASSERT( pos != NULL );
	return GetNext( pos );
}

//////////////////////////////////////////////////////////////////////
// CLibraryList list merging

INT_PTR CLibraryList::Merge(const CLibraryList* pList)
{
	ASSERT( pList != NULL );
	if ( pList == NULL )
		return 0;

	POSITION pos = pList->GetHeadPosition();
	while ( pos )
	{
		CheckAndAdd( pList->GetNext( pos ) );
	}

	return GetCount();
}

//////////////////////////////////////////////////////////////////////
// CLibraryList IGenericView

IMPLEMENT_DISPATCH(CLibraryList, GenericView)

STDMETHODIMP CLibraryList::XGenericView::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )
	*psName = CComBSTR( L"CLibraryFileView" ).Detach();
	return S_OK;
}

STDMETHODIMP CLibraryList::XGenericView::get_Unknown(IUnknown FAR* FAR* /*ppUnknown*/)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryList::XGenericView::get_Param(LONG FAR* /*pnParam*/)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryList::XGenericView::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )

	*ppEnum = &pThis->m_xEnumVARIANT;
	pThis->m_xEnumVARIANT.m_pos = pThis->GetHeadPosition();
	AddRef();

	return S_OK;
}

STDMETHODIMP CLibraryList::XGenericView::get_Item(VARIANT vIndex, VARIANT FAR* pvItem)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )

	VARIANT va;
	VariantInit( &va );
	VariantClear( pvItem );

	if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
		return E_INVALIDARG;

	if ( va.lVal < 0 || va.lVal >= pThis->GetCount() ) return S_OK;

	for ( POSITION pos = pThis->GetHeadPosition() ; pos ; )
	{
		CLibraryListItem Item = pThis->GetNext( pos );
		if ( Item.Type == CLibraryListItem::LibraryFile )
		{
			if ( va.lVal-- == 0 )
			{
				pvItem->vt		= VT_I4;
				pvItem->lVal	= (DWORD) Item;
				break;
			}
		}
	}

	return S_OK;
}

STDMETHODIMP CLibraryList::XGenericView::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )

	*pnCount = 0;
	for ( POSITION pos = pThis->GetHeadPosition() ; pos ; )
	{
		CLibraryListItem Item = pThis->GetNext( pos );
		if ( Item.Type == CLibraryListItem::LibraryFile )
			(*pnCount) ++;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryList IEnumVARIANT enumerator

IMPLEMENT_UNKNOWN(CLibraryList, EnumVARIANT)

CLibraryList::XEnumVARIANT::XEnumVARIANT()
{
	METHOD_PROLOGUE( CLibraryList, EnumVARIANT )

	m_pos = pThis->GetHeadPosition();
}

STDMETHODIMP CLibraryList::XEnumVARIANT::Next(ULONG celt, VARIANT* rgvar, ULONG* pceltFetched)
{
	METHOD_PROLOGUE( CLibraryList, EnumVARIANT )

	if ( rgvar == NULL )
		return E_POINTER;

	if ( pceltFetched )
		*pceltFetched = 0;

	while ( celt && m_pos )
	{
		CLibraryListItem Item = pThis->GetNext( m_pos );
		if ( Item.Type == CLibraryListItem::LibraryFile )
		{
			VariantInit( rgvar );
			rgvar->vt	= VT_I4;
			rgvar->lVal	= (DWORD) Item;
			rgvar++;
			celt--;
			if ( pceltFetched )
				(*pceltFetched)++;
		}
	}

	return ( celt == 0 ? S_OK : S_FALSE );
}

STDMETHODIMP CLibraryList::XEnumVARIANT::Skip(ULONG celt)
{
	METHOD_PROLOGUE( CLibraryList, EnumVARIANT )

	while ( celt && m_pos )
	{
		CLibraryListItem Item = pThis->GetNext( m_pos );
		if ( Item.Type == CLibraryListItem::LibraryFile )
			celt--;
	}

	return ( celt == 0 ? S_OK : S_FALSE );
}

STDMETHODIMP CLibraryList::XEnumVARIANT::Reset()
{
	METHOD_PROLOGUE( CLibraryList, EnumVARIANT )

	m_pos = pThis->GetHeadPosition();

	return S_OK;
}

STDMETHODIMP CLibraryList::XEnumVARIANT::Clone(IEnumVARIANT** /*ppenum*/)
{
	return E_NOTIMPL;
}
