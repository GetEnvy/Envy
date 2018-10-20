//
// PageSettingsUploads.h
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

#include "WndSettingsPage.h"
#include "CtrlDragList.h"


class CUploadsSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CUploadsSettingsPage)

public:
	CUploadsSettingsPage();
	virtual ~CUploadsSettingsPage();

	enum { IDD = IDD_SETTINGS_UPLOADS };

public:
	CSpinButtonCtrl	m_wndMaxPerHost;
	CComboBox		m_wndAgentList;
	CButton			m_wndAgentAdd;
	CButton			m_wndAgentRemove;
	CComboBox		m_wndBandwidthLimit;
	CDragListCtrl	m_wndQueues;
	CButton			m_wndQueueDelete;
	CButton			m_wndQueueEdit;

	BOOL			m_bHubUnshare;
	BOOL			m_bSharePartials;
	BOOL			m_bSharePreviews;
	BOOL			m_bThrottleMode;
	BOOL			m_bFairUseMode;
	DWORD			m_nMaxPerHost;
	CString			m_sBandwidthLimit;

public:
	BOOL			m_bQueuesChanged;		// Have the queues been changed? (Rebuild hash table)
	void			UpdateQueues();

public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	bool IsLimited(CString& strText) const;

	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeAgentList();
	afx_msg void OnEditChangeAgentList();
	afx_msg void OnAgentAdd();
	afx_msg void OnAgentRemove();
	afx_msg void OnQueueNew();
	afx_msg void OnQueueEdit();
	afx_msg void OnQueueDelete();
	afx_msg void OnItemChangedQueues(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkQueues(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnQueueDrop(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	DECLARE_MESSAGE_MAP()
};
