//
// CtrlHomeView.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2010
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

#include "RichViewCtrl.h"
#include "RichDocument.h"
#include "CtrlHomeSearch.h"

#define IDC_HOME_VIEW		150
#define IDC_HOME_SEARCH		151


class CHomeViewCtrl : public CRichViewCtrl
{
	DECLARE_DYNCREATE(CHomeViewCtrl)

public:
	CHomeViewCtrl();

	CRichDocument	m_pDocument;
	CRichElement*	m_peHeader;
	CRichElement*	m_peSearch;
	CRichElement*	m_peUpgrade;
	CRichElement*	m_peRemote1;
	CRichElement*	m_peRemote2;
	CRichElement*	m_peRemoteBrowse;
	CHomeSearchCtrl	m_wndSearch;
	CBitmap			m_bmHeader1;
	CBitmap			m_bmHeader2;

	virtual BOOL	Create(const RECT& rect, CWnd* pParentWnd);
	void			Activate();
	void			Update();
	void			OnSkinChange();

protected:
	virtual void	OnLayoutComplete();
	virtual void	OnPaintBegin(CDC* pDC);
	virtual void	OnVScrolled();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};
