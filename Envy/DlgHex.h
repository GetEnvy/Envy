//
// DlgHex.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2014 and PeerProject 2014
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
// Debug-only

#pragma once

#include "DlgSkinDialog.h"
#include "Buffer.h"


class CHexDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CHexDlg)

public:
	CHexDlg(CWnd* pParent = NULL);
	virtual ~CHexDlg();

	enum { IDD = IDD_DEBUG_HEX };

	CBuffer* GetData() { return &m_pBuffer; }

protected:
	CString m_sHex;
	CBuffer m_pBuffer;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
