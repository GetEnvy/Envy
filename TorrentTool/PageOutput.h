//
// PageOutput.h
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

class COutputPage : public CWizardPage
{
// Construction
public:
	COutputPage();

	DECLARE_DYNCREATE(COutputPage)

// Dialog Data
public:
	enum { IDD = IDD_OUTPUT_PAGE };
	CEdit		m_wndName;
	CComboBox	m_wndFolders;
	CString 	m_sFolder;
	CString 	m_sName;
	CComboBox	m_wndPieceSize;
	int			m_nPieceIndex;
	BOOL		m_bAutoPieces;
	BOOL		m_bSHA1;
	BOOL		m_bED2K;
	BOOL		m_bMD5;

// Overrides
protected:
	virtual void OnReset();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnClearFolders();
	afx_msg void OnBrowseFolder();
	afx_msg void OnClickedAutoPieceSize();
	afx_msg void OnCloseupPieceSize();
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);

	DECLARE_MESSAGE_MAP()
};
