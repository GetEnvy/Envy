//
// CtrlPrivateChatFrame.h"
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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

#include "CtrlChatFrame.h"
#include "GProfile.h"

class CChatSession;


class CPrivateChatFrame : public CChatFrame
{
	DECLARE_DYNAMIC(CPrivateChatFrame)

public:
	CPrivateChatFrame();
	virtual ~CPrivateChatFrame();

public:
	CString	m_sNick;

	void	Initiate(const Hashes::Guid& oGUID, SOCKADDR_IN* pHost, BOOL bMustPush);
	BOOL	Accept(CChatSession* pSession);

public:
	virtual void OnLocalMessage(bool bAction, LPCTSTR pszText);
	virtual void OnLocalCommand(LPCTSTR pszCommand, LPCTSTR pszArgs);
	virtual void OnProfileReceived();
	virtual void OnRemoteMessage(BOOL bAction, LPCTSTR pszText);

public:
	virtual void OnSkinChange();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateChatBrowse(CCmdUI* pCmdUI);
	afx_msg void OnChatBrowse();
	afx_msg void OnUpdateChatPriority(CCmdUI* pCmdUI);
	afx_msg void OnChatPriority();

	DECLARE_MESSAGE_MAP()
};
