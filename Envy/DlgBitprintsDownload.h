//
// DlgBitprintsDownload.h
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

#include "DlgSkinDialog.h"
#include "BitprintsDownloader.h"


class CBitprintsDownloadDlg : public CSkinDialog
{
public:
	CBitprintsDownloadDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_BITPRINTS_DOWNLOAD };

protected:
	CBitprintsDownloader	m_pDownloader;
	DWORD	m_nFailures;

public:
	void	AddFile(DWORD nIndex);
	void	OnNextFile(DWORD nIndex);
	void	OnRequesting(DWORD nIndex, LPCTSTR pszName);
	void	OnSuccess(DWORD nIndex);
	void	OnFailure(DWORD nIndex, LPCTSTR pszMessage);
	void	OnFinishedFile(DWORD nIndex);

public:
	CStatic		m_wndWeb;
	CListCtrl	m_wndFiles;
	CButton		m_wndCancel;
	CProgressCtrl	m_wndProgress;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};
