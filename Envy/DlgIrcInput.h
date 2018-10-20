//
// DlgIrcInput.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2005 and PeerProject 2008-2014
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

class CIrcInputDlg : public CSkinDialog
{
public:
	CIrcInputDlg(CWnd* pParent = NULL, int m_nCaptionIndex = 0, BOOL m_bKickOnly = FALSE);
	virtual ~CIrcInputDlg();

	enum { IDD = IDD_IRC_INPUTBOX };

public:
	int			m_nCaptionIndex;
	BOOL		m_bKickOnly;
	CButton		m_wndPrompt;
	CEdit		m_wndAnswer;
	CString 	m_sAnswer;

	void OnOK();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
};
