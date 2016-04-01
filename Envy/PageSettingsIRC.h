//
// PageSettingsIRC.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2005-2007
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
// Original Author: peer_l_@hotmail.com
//

#pragma once

#include "WndSettingsPage.h"
#include "CtrlFontCombo.h"

class CIRCSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CIRCSettingsPage)

public:
	CIRCSettingsPage();
//	virtual ~CIRCSettingsPage();

	enum { IDD = IDD_SETTINGS_IRC };

public:
	CButton		m_wndColorBg;
	CButton		m_wndColorText;
	CButton		m_wndColorTextLocal;
	CButton		m_wndColorUserAction;
	CButton		m_wndColorAction;
	CButton		m_wndColorServer;
	CButton		m_wndColorNotice;
	CButton		m_wndColorTopic;
	BOOL		m_bShow;
	BOOL		m_bFloodEnable;
	BOOL		m_bTimestamp;
	CString		m_sNick;
	CString		m_sAlternate;
	CString		m_sServerName;
	CString		m_sUserName;
	CString		m_sRealName;
	CString		m_sOnConnect;
	CComboBox	m_wndServerName;
	int 		m_nServerPort;
	int 		m_nFloodLimit;
	int 		m_nFontSize;
	CString		m_sScreenFont;
	CFontCombo	m_wndFonts;
	CSpinButtonCtrl	m_wndFontSizeSpin;
	CSpinButtonCtrl	m_wndFloodLimitSpin;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct);

public:
	virtual void OnOK();
	virtual BOOL OnApply();

protected:
	afx_msg void OnClickIrcColorBg();
	afx_msg void OnClickIrcColorText();
	afx_msg void OnClickIrcColorTextLocal();
	afx_msg void OnClickIrcColorUserAction();
	afx_msg void OnClickIrcColorAction();
	afx_msg void OnClickIrcColorServer();
	afx_msg void OnClickIrcColorNotice();
	afx_msg void OnClickIrcColorTopic();

	DECLARE_MESSAGE_MAP()
};

#define	ID_COLOR_CHATWINDOW				0
#define ID_COLOR_TEXT					1
#define ID_COLOR_TEXTLOCAL				2
#define	ID_COLOR_CHANNELACTION			3
#define	ID_COLOR_ME						4
#define	ID_COLOR_SERVERMSG				7
#define	ID_COLOR_TOPIC					8
#define	ID_COLOR_NOTICE					9
