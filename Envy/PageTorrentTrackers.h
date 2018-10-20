//
// PageTorrentTrackers.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2006 and PeerProject 2008-2012
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

#include "PagePropertyAdv.h"
#include "BTTrackerRequest.h"

// Obsolete Scrape Thread
//#include "ThreadImpl.h"
//
//class CDownload;
//class CBENode;


class CTorrentTrackersPage : public CPropertyPageAdv, public CTrackerEvent	/*public CThreadImpl (Obsolete)*/
{
	DECLARE_DYNCREATE(CTorrentTrackersPage)

public:
	CTorrentTrackersPage();
	virtual ~CTorrentTrackersPage();

	enum { IDD = IDD_TORRENT_TRACKERS };

protected:
	CStringList		m_sOriginalTrackers;
	CString			m_sOriginalTracker;
	int				m_nOriginalMode;

	CEdit			m_wndTracker;
	CEdit			m_wndComplete;
	CEdit			m_wndIncomplete;
	CComboBox		m_wndTrackerMode;
	CButton			m_wndRefresh;
	CButton			m_wndAdd;
	CButton			m_wndDel;
//	CButton			m_wndRename;
	CListCtrl		m_wndTrackers;

	DWORD			m_nRequest;						// Tracker request transaction ID
	DWORD			m_nComplete;					// Seeder scrape request
	DWORD			m_nIncomplete;					// Leecher scrape request
//	DWORD			m_nDownloaded;					// Domnload count scrape request (Unused)

// Obsolete Scrape Thread
//	void			OnRun();
//	BOOL			OnTree(CDownload* pDownload, CBENode* pNode);

	BOOL			ApplyTracker();							// Apply settings to download
	void			InsertTracker();						// Insert new tracker
	void			EditTracker(int nItem, LPCTSTR szText);	// Set tracker new text
	void			SelectTracker(int nItem);				// Select this tracker as current one in single mode
	void			UpdateInterface();						// Updated interface

	virtual void	DoDataExchange(CDataExchange* pDX);
	virtual BOOL	OnInitDialog();
	virtual BOOL	OnApply();
	virtual void	OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip, CBTTrackerRequest* pEvent);

	afx_msg void	OnDestroy();
	afx_msg void	OnTorrentRefresh();
	afx_msg void	OnTimer(UINT_PTR nIDEvent);
	afx_msg void	OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void	OnNMClickTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnNMDblclkTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnLvnEndlabeleditTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnLvnKeydownTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnCbnSelchangeTorrentTrackermode();
	afx_msg void	OnBnClickedTorrentTrackersAdd();
	afx_msg void	OnBnClickedTorrentTrackersDel();
//	afx_msg void	OnBnClickedTorrentTrackersRename();

	DECLARE_MESSAGE_MAP()
};
