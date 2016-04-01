//
// PageTorrentFiles.h
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

#include "PagePropertyAdv.h"
//#include "ComboListCtrl.h"


class CTorrentFilesPage : public CPropertyPageAdv
{
	DECLARE_DYNCREATE(CTorrentFilesPage)

public:
	CTorrentFilesPage();
	virtual ~CTorrentFilesPage();

	enum { IDD = IDD_TORRENT_FILES };

protected:
//	CComboListCtrl	m_wndFiles;		// Custom feature currently unused
	CListCtrl	m_wndFiles;
	CString 	m_sFilecount;
	BOOL		m_bLoaded;	// Filelist complete

	void GetFiles();		// Populate interface
	void Update();			// Refresh interface
	void UpdateCount();		// Refresh file counter text

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCheckbox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSortColumn(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnNMDblclkTorrentFiles(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
