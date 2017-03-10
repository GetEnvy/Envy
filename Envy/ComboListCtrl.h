//
// ComboListCtrl.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2008
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

// Note this file is not used.

//#pragma once
//
//#include "CtrlLibraryTip.h"
//
//
//class CComboListCtrl : public CListCtrl
//{
//	DECLARE_DYNAMIC(CComboListCtrl)
//
//public:
//	CComboListCtrl();
//	virtual ~CComboListCtrl();
//
//public:
//	typedef std::map< int, CString > CIntStringMap;
//	typedef std::map< int, int > CIntIntMap;
//	typedef std::vector< CIntIntMap > CIntIntMapVector;
//	typedef std::map< int, CIntStringMap > CIntIntStringMapMap;
//
//	int  GetColumnData(int iItem, int iColumn) const;
//	void SetColumnData(int iItem, int iColumn, int iData);
//	void SetColumnValues(int iColumn, const CIntStringMap& oValues);
//	void EnableTips(std::unique_ptr< CLibraryTipCtrl > pTip);
//
//protected:
//	std::unique_ptr< CLibraryTipCtrl > m_pTip;
//	CComboBox*			m_pCombo;
//	int					m_iSelectedItem;
//	int					m_iSelectedSubItem;
//	CIntIntMapVector	m_oData;
//	CIntIntStringMapMap	m_oColumns;
//
//	int HitTest(const CPoint& ptAction);
//	void Show(int iItem, int iSubItem);
//	void Hide();
//
//	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
//
//	DECLARE_MESSAGE_MAP()
//};
//
//#define BEGIN_COLUMN_MAP() \
//	{ CComboListCtrl::CIntStringMap x;
//
//#define COLUMN_MAP(nValue, nString) \
//	x.insert( CComboListCtrl::CIntStringMap::value_type( (nValue), (nString) ) );
//
//#define END_COLUMN_MAP(oWnd, nColumn) \
//	(oWnd).SetColumnValues( (nColumn), x ); }
