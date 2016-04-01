//
// DlgTorrentSeed.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
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

#include "DlgSkinDialog.h"
#include "ThreadImpl.h"
#include "BTInfo.h"


class CTorrentSeedDlg :
	public CSkinDialog,
	public CThreadImpl
{
	DECLARE_DYNAMIC(CTorrentSeedDlg)

public:
	CTorrentSeedDlg(LPCTSTR pszTorrent, BOOL bForceSeed = FALSE, CWnd* pParent = NULL);
	BOOL			LoadTorrent(CString strPath);

	enum { IDD = IDD_TORRENT_SEED };

protected:
	CProgressCtrl	m_wndProgress;
	CButton			m_wndDownload;
	CButton			m_wndSeed;
	BOOL			m_bCancel;
	CString			m_sTorrent;
	BOOL			m_bForceSeed;
	CBTInfo			m_pInfo;
	CString			m_sMessage;
	int				m_nScaled;
	int				m_nOldScaled;

	void		OnRun();
	BOOL		CreateDownload();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	afx_msg void OnDownload();
	afx_msg void OnSeed();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
