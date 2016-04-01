//
// CtrlDragList.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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

class CDragListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CDragListCtrl)

public:
	CDragListCtrl();
	virtual ~CDragListCtrl();

protected:
	CImageList*	m_pDragImage;
	int			m_nDragDrop;
	BOOL		m_bCreateDragImage;

public:
	virtual void	OnDragDrop(int nDrop);

protected:
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};

#define LVN_DRAGDROP	(LVN_FIRST+1)
