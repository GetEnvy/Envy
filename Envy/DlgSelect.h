//
// DlgSelect.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2009 and PeerProject 2009-2014
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

#include "DlgSkinDialog.h"

// CSelectDialog dialog

class CSelectDialog : public CSkinDialog
{
	DECLARE_DYNAMIC(CSelectDialog)

public:
	CSelectDialog(CWnd* pParent = NULL);

	enum { IDD = IDD_SELECT };

public:
	class CItem
	{
	public:
		inline CItem(const CString& sItem = CString(), int nData = 0) :
			m_sItem( sItem ), m_nData( nData ) {}
		inline CItem(const CItem& it) :
			m_sItem( it.m_sItem ), m_nData( it.m_nData ) {}
		inline CItem& operator=(const CItem& it)
			{ m_sItem = it.m_sItem; m_nData = it.m_nData; return *this; }
		CString		m_sItem;
		DWORD_PTR	m_nData;
	};

	typedef CList< CItem > CItemList;

	// Add new item to selection list
	inline void Add(const CString& sItem, DWORD_PTR nData)
	{
		m_List.AddTail( CItem( sItem, nData ) );
	}

	// Get current selection data
	inline DWORD_PTR Get() const
	{
		return m_nData;
	}

protected:
	CComboBox	m_ListCtrl;
	CItemList	m_List;
	DWORD_PTR	m_nData;

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnCbnDropdownList();

	DECLARE_MESSAGE_MAP()
};
