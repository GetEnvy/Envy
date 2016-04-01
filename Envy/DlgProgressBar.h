//
// DlgProgressBar.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2009-2014 and Shareaza 2009
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


class CProgressBarDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CProgressBarDlg)

public:
	CProgressBarDlg(CWnd* pParent = NULL);
	virtual ~CProgressBarDlg();

	enum { IDD = IDD_PROGRESSBAR };

private:
	CString			m_sAction;
	CString			m_sEvent;
	CProgressCtrl	m_oEventProgress;
	CString			m_sSubAction;
	CString			m_sSubEvent;
	CProgressCtrl	m_oSubEventProgress;

public:
	void	SetActionText(const CString& strText);
	void	SetEventText(const CString& strText);
	void	SetEventRange(int nLower, int nUpper);
	void	SetEventPos(int nPos);
	void	SetEventStep(int nStep);
	void	SetSubActionText(const CString& strText);
	void	SetSubEventText(const CString& strText);
	void	SetSubEventRange(int nLower, int nUpper);
	void	SetSubEventPos(int nPos);
	void	SetSubEventStep(int nStep);
	void	StepEvent();
	void	StepEvent(int nPos);
	void	StepSubEvent();
	void	StepSubEvent(int nPos);

protected:
	virtual void	DoDataExchange(CDataExchange* pDX);
	virtual BOOL	OnInitDialog();
	virtual void	OnCancel();
	virtual void	OnOK();

	//DECLARE_MESSAGE_MAP()
};
