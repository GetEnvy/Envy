//
// LibraryList.h
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

#pragma once

class CLibraryList;
class CLibraryFile;
class CLibraryFolder;
class CAlbumFolder;


class CLibraryListItem
{
public:
	CLibraryListItem();
	CLibraryListItem(DWORD val);
	CLibraryListItem(CAlbumFolder* val);
	CLibraryListItem(CLibraryFolder* val);
	CLibraryListItem(const CLibraryFile* val);
	CLibraryListItem(const CLibraryListItem& val);
	bool operator == (const CLibraryListItem& val) const;
	bool operator != (const CLibraryListItem& val) const;
	operator DWORD () const;
	operator CLibraryFile* () const;
	operator CAlbumFolder* () const;
	operator CLibraryFolder* () const;
	CLibraryListItem& operator = (const CLibraryListItem& val);

public:
	enum {
		Empty = 0, LibraryFile, AlbumFolder, LibraryFolder
	}	Type;

protected:
	union {
		DWORD			dwLibraryFile;
		CAlbumFolder*	pAlbumFolder;
		CLibraryFolder*	pLibraryFolder;
	};
};


typedef CComObjectPtr< CLibraryList > CLibraryListPtr;


class CLibraryList : public CComObject
{
	DECLARE_DYNAMIC(CLibraryList)

public:
	CLibraryList();
protected:
	virtual ~CLibraryList();

protected:
	CList < CLibraryListItem, CLibraryListItem > m_List;

public:
	inline INT_PTR GetCount() const
	{
		return m_List.GetCount();
	}

	inline BOOL IsEmpty() const
	{
		return m_List.IsEmpty();
	}

	inline const CLibraryListItem& GetHead()const
	{
		return m_List.GetHead();
	}

	inline CLibraryListItem& GetTail()
	{
		return m_List.GetTail();
	}

	inline POSITION GetHeadPosition() const
	{
		return m_List.GetHeadPosition();
	}

	inline POSITION GetTailPosition() const
	{
		return m_List.GetTailPosition();
	}

	inline CLibraryListItem GetPrev(POSITION& pos) const
	{
		return m_List.GetPrev( pos );
	}

	inline CLibraryListItem GetNext(POSITION& pos) const
	{
		return m_List.GetNext( pos );
	}

	inline POSITION AddHead(CLibraryListItem nItem)
	{
		return m_List.AddHead( nItem );
	}

	inline POSITION AddTail(CLibraryListItem nItem)
	{
		return m_List.AddTail( nItem );
	}

	inline POSITION CheckAndAdd(CLibraryListItem nItem)
	{
		return ( Find( nItem ) == NULL ) ? AddTail( nItem ) : NULL;
	}

	inline CLibraryListItem RemoveHead()
	{
		return m_List.RemoveHead();
	}

	inline CLibraryListItem RemoveTail()
	{
		return m_List.RemoveTail();
	}

	inline void RemoveAt(POSITION pos)
	{
		m_List.RemoveAt( pos );
	}

	inline void RemoveAll()
	{
		m_List.RemoveAll();
	}

	inline POSITION Find(CLibraryListItem nItem) const
	{
		return m_List.Find( nItem );
	}

	CLibraryFile*	GetNextFile(POSITION& pos);
	INT_PTR			Merge(const CLibraryList* pList);

// Automation
public:
	DECLARE_INTERFACE_MAP()

	BEGIN_INTERFACE_PART(GenericView, IGenericView)
		DECLARE_DISPATCH()
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(get_Unknown)(IUnknown FAR* FAR* ppUnknown);
		STDMETHOD(get_Param)(LONG FAR* pnParam);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(get_Item)(VARIANT vIndex, VARIANT FAR* pvItem);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
	END_INTERFACE_PART(GenericView)

	BEGIN_INTERFACE_PART(EnumVARIANT, IEnumVARIANT)
		XEnumVARIANT();
		STDMETHOD(Next)(DWORD celt, VARIANT* rgvar, DWORD* pceltFetched);
		STDMETHOD(Skip)(DWORD celt);
		STDMETHOD(Reset)();
		STDMETHOD(Clone)(IEnumVARIANT** ppenum);
		POSITION m_pos;
	END_INTERFACE_PART(EnumVARIANT)

private:
	CLibraryList(const CLibraryList&);
	CLibraryList& operator=(const CLibraryList&);
};
