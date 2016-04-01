//
// DlgScheduleItem.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2010,2014 and Shareaza 2010
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
//#include <afxdtctl.h>		// MFC date/time controls

class CScheduleTask;

//////////////////////////////////////////////////////////////////////
// CScheduleTaskDlg class: Schedule Item Dialog
//

class CScheduleTaskDlg : public CSkinDialog
{
public:
	CScheduleTaskDlg(CWnd* pParent = NULL, CScheduleTask* pSchTask = NULL);
	virtual ~CScheduleTaskDlg();

	enum { IDD = IDD_SCHEDULE_TASK };

public:
	CScheduleTask		*m_pScheduleTask;
	unsigned int		m_nAction;
	unsigned int		m_nDays;
	CString				m_sDescription;
	CTime				m_tDateAndTime;
	bool				m_bSpecificDays;
	bool				m_bActive;
	bool				m_bNew;
	BOOL				m_bLimitedNetworks;
	int					m_nLimitDown;
	int					m_nLimitUp;

	CComboBox			m_wndTypeSel;
	CButton				m_wndLimitedCheck;
	CEdit				m_wndLimitedEditDown;
	CEdit				m_wndLimitedEditUp;
	CStatic				m_wndLimitedStaticDown;
	CStatic				m_wndLimitedStaticUp;
	CSpinButtonCtrl		m_wndSpinDown;
	CSpinButtonCtrl		m_wndSpinUp;
	CDateTimeCtrl		m_wndDate;
	CDateTimeCtrl		m_wndTime;
	CButton				m_wndActiveCheck;
	CButton				m_wndRadioOnce;
	CButton				m_wndRadioEveryDay;
	CButton				m_wndChkDayMon;
	CButton				m_wndChkDayTues;
	CButton				m_wndChkDayWed;
	CButton				m_wndChkDayThu;
	CButton				m_wndChkDayFri;
	CButton				m_wndChkDaySat;
	CButton				m_wndChkDaySun;
	CButton				m_wndBtnAllDays;

protected:
	void EnableDaysOfWeek(bool bEnable);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnCbnSelchangeEventType();
	afx_msg void OnDtnDateTimeChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOnlyonce();
	afx_msg void OnBnClickedEveryday();
	afx_msg void OnBnClickedButtonAllDays();

	DECLARE_MESSAGE_MAP()
};
