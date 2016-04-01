//
// DlgQueueProperties.h
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

class CUploadQueue;


class CQueuePropertiesDlg : public CSkinDialog
{
public:
	CQueuePropertiesDlg(CUploadQueue* pQueue, BOOL bEnable, CWnd* pParent = NULL);

	enum { IDD = IDD_QUEUE_PROPERTIES };

public:
	CButton 		m_wndPartialOnly;
	CButton 		m_wndLibraryOnly;
	CButton			m_wndBoth;
	CEdit			m_wndMatch;
	CEdit			m_wndBandwidthPoints;
	CEdit			m_wndBandwidthValue;
	CSpinButtonCtrl	m_wndTransfersMin;
	CSpinButtonCtrl	m_wndTransfersMax;
	CSpinButtonCtrl	m_wndRotateTimeSpin;
	CEdit			m_wndRotateTime;
	CListCtrl		m_wndProtocols;
	CEdit			m_wndMinSize;
	CEdit			m_wndMaxSize;
	CComboBox		m_wndMarked;
	CSpinButtonCtrl	m_wndCapacity;
	CSliderCtrl		m_wndBandwidthSlider;
	INT_PTR			m_nCapacity;
	BOOL			m_bMaxSize;
	CString			m_sMaxSize;
	BOOL			m_bMinSize;
	CString			m_sMinSize;
	BOOL			m_bMarked;
	CString			m_sName;
	DWORD			m_nFileStatusFlag;
	BOOL			m_bProtocols;
	BOOL			m_bRotate;
	BOOL			m_bReward;
	int				m_nRotateTime;
	INT_PTR			m_nTransfersMax;
	INT_PTR			m_nTransfersMin;
	BOOL			m_bMatch;
	CString			m_sMatch;
	CString			m_sMarked;
	BOOL			m_bEnable;
	BOOL			m_bEnableOverride;

	CImageList		m_gdiProtocols;
	CUploadQueue*	m_pQueue;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

protected:
	afx_msg void OnMinimumCheck();
	afx_msg void OnMaximumCheck();
	afx_msg void OnProtocolsCheck();
	afx_msg void OnMatchCheck();
	afx_msg void OnMarkedCheck();
	afx_msg void OnRotateEnable();
	afx_msg void OnChangeTransfersMax();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg void OnPartialClicked();
	afx_msg void OnLibraryClicked();
	afx_msg void OnBothClicked();

	CButton* GetProtocolCheckbox()
	{
		return ((CButton*)GetDlgItem(IDC_PROTOCOLS_CHECK));
	}

	DECLARE_MESSAGE_MAP()
};
