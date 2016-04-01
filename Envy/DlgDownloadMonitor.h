//
// DlgDownloadMonitor.h
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

#include "DlgSkinDialog.h"

class CDownload;
class CLineGraph;
class CGraphItem;


class CDownloadMonitorDlg : public CSkinDialog
{
public:
	CDownloadMonitorDlg(CDownload* pDownload);
	virtual ~CDownloadMonitorDlg();

	enum { IDD = IDD_DOWNLOAD_MONITOR };

public:
	CDownload*		m_pDownload;
	CString			m_sName;
	CLineGraph*		m_pGraph;
	CGraphItem*		m_pItem;
	NOTIFYICONDATA	m_pTray;
	BOOL			m_bTray;
	BOOL			m_bCompleted;

	static CList< CDownloadMonitorDlg* >	m_pWindows;

#ifdef __ITaskbarList3_INTERFACE_DEFINED__	// VS2010+
	CComPtr< ITaskbarList3 > m_pTaskbar;	// Windows task bar
#endif

// Operatons
protected:
	BOOL	CreateReal(UINT nID);
	void	Update(CWnd* pWnd, LPCTSTR pszText);
	void	Update(CWnd* pWnd, BOOL bEnabled);
	void	DoPaint(CDC& dc);
	void	DrawProgressBar(CDC* pDC, CRect* pRect);
public:
	static void OnSkinChange(BOOL bSet);
	static void CloseAll();

public:
	CStatic	m_wndVolume;
	CStatic m_wndVolumeLabel;
	CButton	m_wndAutoClose;
	CButton	m_wndAction;
	CButton	m_wndClose;
	CButton	m_wndShow;
	CStatic	m_wndProgress;
	CStatic	m_wndTime;
	CStatic	m_wndTimeLabel;
	CStatic	m_wndStatus;
	CStatic	m_wndSpeed;
	CStatic	m_wndSpeedLabel;
	CStatic	m_wndSources;
	CStatic	m_wndSourcesLabel;
	CButton	m_wndLaunch;
	CStatic	m_wndIcon;
	CStatic	m_wndGraph;
	CStatic	m_wndFile;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void PostNcDestroy();

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDownloadShow();
	afx_msg void OnDownloadAction();
	afx_msg void OnDownloadClose();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg LRESULT OnTray(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnNeedText(UINT nID, NMHDR* pTTT, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
