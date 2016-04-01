//
// CtrlBrowseFrame.h
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

#include "CtrlCoolBar.h"
#include "CtrlBrowseTree.h"
#include "CtrlSearchDetailPanel.h"

class CMatchCtrl;
class CG2Packet;
class CQueryHit;
class CBrowseTreeItem;


class CBrowseFrameCtrl : public CWnd
{
	DECLARE_DYNAMIC(CBrowseFrameCtrl)

public:
	CBrowseFrameCtrl();
	virtual ~CBrowseFrameCtrl();

protected:
	CCoolBarCtrl		m_wndTreeTop;
	CBrowseTreeCtrl		m_wndTree;
	CSearchDetailPanel	m_wndDetails;
	CMatchCtrl*			m_wndList;
	BOOL				m_bTreeVisible;
	int					m_nTreeSize;
	BOOL				m_bPanelEnable;
	BOOL				m_bPanelVisible;
	int					m_nPanelSize;
	int					m_nTree;
	CG2Packet*			m_pTree[2];

public:
	void			Serialize(CArchive& ar, int /*nVersion*/);	// BROWSER_SER_VERSION
	virtual BOOL	Create(CWnd* pParentWnd, CMatchCtrl* pMatch);
	void			OnSkinChange();
	void			OnPhysicalTree(CG2Packet* pPacket);
	void			OnVirtualTree(CG2Packet* pPacket);
	void			OnSelChangeMatches();
protected:
	BOOL			DoSizeTree();
	BOOL			DoSizePanel();
	void			SelectTree(CBrowseTreeItem* pItem, CQueryHit* pHit);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnUpdateSearchDetails(CCmdUI* pCmdUI);
	afx_msg void OnSearchDetails();
	afx_msg void OnUpdateLibraryTreePhysical(CCmdUI *pCmdUI);
	afx_msg void OnLibraryTreePhysical();
	afx_msg void OnUpdateLibraryTreeVirtual(CCmdUI *pCmdUI);
	afx_msg void OnLibraryTreeVirtual();
	afx_msg void OnTreeSelection(NMHDR* pNotify, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

#define IDC_BROWSE_FRAME	110
