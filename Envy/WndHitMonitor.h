//
// WndHitMonitor.h
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

#include "WndBaseMatch.h"


class CHitMonitorWnd : public CBaseMatchWnd
{
	DECLARE_SERIAL(CHitMonitorWnd)

public:
	CHitMonitorWnd();
	virtual ~CHitMonitorWnd();

public:
	virtual BOOL	OnQueryHits(const CQueryHit* pHits);
	virtual void	OnSkinChange();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnHitMonitorClear();
	afx_msg void OnUpdateHitMonitorPause(CCmdUI* pCmdUI);
	afx_msg void OnHitMonitorPause();

	DECLARE_MESSAGE_MAP()
};
