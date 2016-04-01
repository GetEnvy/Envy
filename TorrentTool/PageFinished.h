//
// PageFinished.h
//
// This file is part of Envy Torrent Tool (getenvy.com) © 2016
// Portions copyright PeerProject 2008 and Shareaza 2007
//
// Envy Torrent Tool is free software; you can redistribute it
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Torrent Tool is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "WizardSheet.h"

class CTorrentBuilder;


class CFinishedPage : public CWizardPage
{
// Construction
public:
	CFinishedPage();
	virtual ~CFinishedPage();

	DECLARE_DYNCREATE(CFinishedPage)

// Dialog Data
public:
	//{{AFX_DATA(CFinishedPage)
	enum { IDD = IDD_FINISHED_PAGE };
	CButton	m_wndAbort;
	CEdit	m_wndTorrentName;
	CButton	m_wndTorrentCopy;
	CButton	m_wndTorrentOpen;
	CButton	m_wndTorrentSeed;
	CStatic	m_wndSpeedMessage;
	CSliderCtrl	m_wndSpeed;
	CStatic	m_wndSpeedSlow;
	CStatic	m_wndSpeedFast;
	CProgressCtrl	m_wndProgress;
	CStatic	m_wndFileName;
	CStatic	m_wndDone2;
	CStatic	m_wndDone1;
	//}}AFX_DATA

// Attributes
protected:
	CTorrentBuilder*	m_pBuilder;

// Operations
protected:
	void	Start();

// Overrides
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

// Implementation
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnAbort();
	afx_msg void OnTorrentCopy();
	afx_msg void OnTorrentOpen();
	afx_msg void OnTorrentSeed();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};
