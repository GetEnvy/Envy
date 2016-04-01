//
// CtrlIRCFrame.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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

#include "RichViewCtrl.h"
#include "RichDocument.h"
#include "CtrlCoolBar.h"
#include "CtrlIRCPanel.h"


// IRC Window Dimensions
// Settings.Skin.HeaderbarHeight		64
#define TABBAR_HEIGHT					22
#define SEPARATOR_HEIGHT				3
#define SMALLHEADER_HEIGHT				20
#define EDITBOX_HEIGHT					22
// Settings.Skin.ToolbarHeight			28
#define STATUSBOX_WIDTH					330

#define MAX_CHANNELS					12

#define ID_MESSAGE_SERVER_DISCONNECT	209
#define ID_MESSAGE_SERVER_PING			210
#define ID_MESSAGE_SERVER_NOTICE		211
#define ID_MESSAGE_SERVER_ERROR			212
#define ID_MESSAGE_SERVER_CONNECTED		213
#define ID_MESSAGE_SERVER_MSG			214

#define ID_MESSAGE_CLIENT_JOIN_USERLIST	215
#define ID_MESSAGE_CLIENT_JOIN_ENDNAMES	216
#define ID_MESSAGE_CLIENT_JOIN			217
#define ID_MESSAGE_CLIENT_NOTICE		218
#define ID_MESSAGE_CLIENT_INVITE		219
#define ID_MESSAGE_CLIENT_WHOWAS		220
#define ID_MESSAGE_CLIENT_WHOIS			221

#define ID_MESSAGE_CHANNEL_TOPICSETBY	222
#define ID_MESSAGE_CHANNEL_TOPICSHOW	223
#define ID_MESSAGE_CHANNEL_PART			224
#define ID_MESSAGE_CHANNEL_QUIT			225
#define ID_MESSAGE_CHANNEL_JOIN			226
#define ID_MESSAGE_CHANNEL_SETMODE		227
#define ID_MESSAGE_CHANNEL_NOTICE		228
#define ID_MESSAGE_CHANNEL_MESSAGE		229
#define ID_MESSAGE_CHANNEL_LIST			230
#define ID_MESSAGE_CHANNEL_ME			231
#define ID_MESSAGE_CHANNEL_LISTEND		232
#define ID_MESSAGE_CHANNEL_PART_FORCED	244

#define ID_MESSAGE_USER_MESSAGE			233
#define ID_MESSAGE_USER_ME				236
#define ID_MESSAGE_USER_AWAY			237
#define ID_MESSAGE_USER_INVITE			238
#define ID_MESSAGE_USER_KICK			239
#define ID_MESSAGE_USER_CTCPTIME		234
#define	ID_MESSAGE_USER_CTCPVERSION		235
#define ID_MESSAGE_USER_CTCPBROWSE		245

#define ID_MESSAGE_NICK					240
#define ID_MESSAGE_IGNORE				241
#define ID_MESSAGE_STOPAWAY				242
#define ID_MESSAGE_SETAWAY				243

#define ID_COLOR_CHATWINDOW				0
#define ID_COLOR_TEXT					1
#define ID_COLOR_TEXTLOCAL				2
#define ID_COLOR_CHANNELACTION			3
#define ID_COLOR_ME						4
#define ID_COLOR_MSG					5
#define ID_COLOR_NEWMSG					6
#define ID_COLOR_SERVERMSG				7
#define	ID_COLOR_TOPIC					8
#define ID_COLOR_NOTICE					9
#define ID_COLOR_SERVERERROR			10
#define	ID_COLOR_TABS					11

#define ID_KIND_CLIENT					51
#define ID_KIND_PRIVATEMSG				52
#define ID_KIND_CHANNEL					53

#define IDC_CHAT_TEXT					100
#define IDC_CHAT_EDIT					101
#define IDC_CHAT_TABS					102
#define IDC_CHAT_TEXTSTATUS				100

#define IDC_IRC_DBLCLKCHANNELS			200
#define IDC_IRC_DBLCLKUSERS				201
#define IDC_IRC_MENUUSERS				202
#define IDC_IRC_CHANNELS				122

#define IDC_IRC_FRAME					400

#define WM_REMOVECHANNEL				20933
#define WM_ADDCHANNEL					20934


class CIRCNewMessage
{
protected:
	class CIRCMessage
	{
	public:
		CIRCMessage(LPCTSTR szMessage = L"", LPCTSTR szTargetName = L"", int nColor = 0)
			: sMessage		( szMessage )
			, sTargetName	( szTargetName )
			, nColorID		( nColor )
		{
		}

		CIRCMessage(const CIRCMessage& msg)
			: sMessage		( msg.sMessage )
			, sTargetName	( msg.sTargetName )
			, nColorID		( msg.nColorID )
		{
		}

		CIRCMessage& operator=(const CIRCMessage& msg)
		{
			sMessage = msg.sMessage;
			sTargetName = msg.sTargetName;
			nColorID = msg.nColorID;
			return *this;
		}

		CString	sMessage;
		CString	sTargetName;
		int		nColorID;
	};

public:
	inline void Add(LPCTSTR szMessage, LPCTSTR szTargetName, int nColor)
	{
		CIRCMessage msg( szMessage, szTargetName, nColor );
		m_pMessages.Add( msg );
	}

	CArray< CIRCMessage > m_pMessages;
};


class CIRCTabCtrl : public CTabCtrl
{
	enum
	{
		paintNone = 0,
		paintBody = 0x1,
		paintSelected = 0x2,
		paintHotTrack = 0x4
	};

public:
	CIRCTabCtrl();
	virtual ~CIRCTabCtrl();

protected:
	HANDLE	m_hTheme;
public:
	int		m_nHoverTab;

	void	DrawTabControl(CDC* pDC);
	void	DrawTabThemed(HDC dc, int nItem, const RECT& rcItem, UINT flags);
	void	DrawTabItem(HDC dc, int nItem, const RECT& rcItem, UINT flags);
	HRESULT DrawThemesPart(HDC dc, int nPartID, int nStateID, LPRECT prcBox);

//	void			SetTabColor(int nItem, COLORREF cRGB);
//	COLORREF		GetTabColor(int nItem);

	virtual BOOL	PreTranslateMessage(MSG* pMsg);
//	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
//	virtual BOOL	OnEraseBkgnd(CDC* pDC);

	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnPaint();

	DECLARE_MESSAGE_MAP()
};


class CIRCChannelList
{
public:
	CIRCChannelList();

	void			AddChannel(LPCTSTR strDisplayName, LPCTSTR strName, BOOL bUserDefined = FALSE );
	void			RemoveChannel(const CString& strDisplayName);
	void			RemoveAll(int nType = -1);
	int				GetCount(int nType = -1) const;
	BOOL			GetType(const CString& strDisplayName) const;
	int				GetIndexOfDisplay(const CString& strDisplayName) const;
	int				GetIndexOfName(const CString& strName) const;
	CString			GetDisplayOfIndex(int nIndex) const;
	CString			GetNameOfIndex(int nIndex) const;
protected:
	int				m_nCount;
	int				m_nCountUserDefined;
	CStringArray	m_sChannelName;
	CStringArray	m_sChannelDisplayName;
	CArray<BOOL>	m_bUserDefined;
};


class CIRCFrame : public CWnd
{
	DECLARE_DYNAMIC(CIRCFrame)

public:
	CIRCFrame();
	virtual ~CIRCFrame();

	static CIRCFrame*	g_pIrcFrame;

public:
	virtual BOOL	Create(CWnd* pParentWnd);
	virtual BOOL	PreTranslateMessage(MSG* pMsg);

	void	OnSkinChange();
	void	OnUpdateCmdUI();

protected:
	BOOL			m_bConnected;
	CString			m_sStatus;
	int				m_nSelectedTab;
	int				m_nMsgsInSec;
	int				m_nTimerVal;
	int				m_nSelectedTabType;
	int				m_nRSelectedTab;
	BOOL			m_bFloodProtectionRunning;
	int				m_nFloodLimit;
	int				m_nFloodingDelay;
	int				m_nUpdateFrequency;
//	int				m_nUpdateChanListFreq;

	CString			m_sFile;
	CString			m_sNickname;
	CStringArray	m_pIrcBuffer[ MAX_CHANNELS ];
	CStringArray	m_pIrcUsersBuffer[ MAX_CHANNELS ];
	CStringArray	m_pLastLineBuffer[ MAX_CHANNELS ];
	int				m_nCurrentPosLineBuffer[ MAX_CHANNELS ];
	int				m_nBufferCount;
	CIRCChannelList	m_pChanList;

	// Header
	int				m_nHeaderIcon;
	CBitmap			m_bmWatermark;
	CBitmap			m_bmBuffer;
	HBITMAP			m_hBuffer;
	CDC				m_dcBuffer;
	CIRCPanel		m_wndPanel;

	CRichDocument	m_pContent;
	CRichDocument	m_pContentStatus;
	CRichViewCtrl	m_wndView;
	CEdit			m_wndEdit;
	CIRCTabCtrl		m_wndTab;
	CCoolBarCtrl	m_wndMainBar;

	CString			m_sTemp;
	CString			m_sCurrent;
//	CPoint			m_ptCursor;
//	int				m_nListWidth;
	int				m_nLocalTextLimit;
	int				m_nLocalLinesLimit;
	SOCKET			m_nSocket;
	CEvent			m_pWakeup;
	CStringA		m_sWsaBuffer;

	CFont			m_fntEdit;
	CStringArray	m_pWords;
	CString 		m_sUser;

	void			ConnectIrc();
	void			SetFonts();
	BOOL			OnNewMessage(const CString& strMessage);
	void			StatusMessage(LPCTSTR pszText, int nFlags = ID_COLOR_NOTICE);
	void			SendString(const CString& strMessage);
	int				FindParsedItem(LPCTSTR szMessage, int nFirst = 0);
	void			LoadBufferForWindow(int nTab);
	void			ParseString(const CString& strMessage, CIRCNewMessage& oNewMessage);
	CString			TrimString(CString strMessage) const;
	CString			GetStringAfterParsedItem(int nItem) const;
//	CString			GetTargetName(CString strReceiverName, int nReceiverType, CString strSenderName, int nSenderType) const;
	CString			GetTabText(int nTabIndex = -1) const;
	int				GetTabIndex(const CString& strTabName) const;
	void			HighlightTab(int nTab, BOOL bHighlight = TRUE);
	int				AddTab(const CString& TabName, int nKindOfTab);
	void			TabClick();
	void			ReloadViewText();
	int				FindInList(CString strName, int nList=0, int nTab=0);
	//void			SortUserList();

	int				ParseMessageID();
	void			ActivateMessageByID(CIRCNewMessage& oNewMessage, int nMessageID);
	CString			GetTextFromRichPoint() const;
	CString			RemoveModeOfNick(CString strNick) const;
	void			UserListDblClick();
	void			ChanListDblClick();
	void			FillChanList();
	void			FillChanListCount(const CString& strUserCount, const CString& strChannelName);
	void			ClearChanListCount();
	int				IsUserInList(CString strUser) const;
	void			PaintListHeader(CDC& dc, CRect& rcBar, CString strText);
	void			PaintHeader(CRect rcHeader, CDC &dc);
	void			DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText);

	inline CString GetSelectedUser() const
	{
		CString strUser;
		int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
		if ( nItem >= 0 )
			m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strUser );
		return strUser;
	}

	inline void SetSelectedUser(int nIndex)
	{
		m_wndPanel.m_boxUsers.m_wndUserList.SetCurSel( nIndex );
	}

	inline void ClearUserList()
	{
		m_wndPanel.m_boxUsers.m_wndUserList.ResetContent();
	}

	inline void AddUser(LPCTSTR szUser)
	{
		m_wndPanel.m_boxUsers.m_wndUserList.AddString( szUser );
	}

	inline void DeleteUser(int nIndex)
	{
		m_wndPanel.m_boxUsers.m_wndUserList.DeleteString( nIndex );
	}

	inline int GetUserCount() const
	{
		return m_wndPanel.m_boxUsers.m_wndUserList.GetCount();
	}

	inline CString GetUser(int nIndex) const
	{
		CString strUser;
		m_wndPanel.m_boxUsers.m_wndUserList.GetText( nIndex, strUser );
		return strUser;
	}

protected:
	virtual void OnLocalText(LPCTSTR pszText);
	virtual void OnStatusMessage(LPCTSTR pszText, int nFlags);

	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRichCursorMove(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRichClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRichDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnIrcShowSettings();
	afx_msg void OnIrcChanCmdOpen();

	afx_msg void OnUpdateIrcUserCmd(CCmdUI* pCmdUI);
	afx_msg void OnIrcUserCmdWhois();
	afx_msg void OnIrcUserCmdTime();
	afx_msg void OnIrcUserCmdVersion();
	afx_msg void OnIrcUserCmdBrowse();
	afx_msg void OnIrcUserCmdIgnore();
	afx_msg void OnIrcUserCmdUnignore();

	afx_msg void OnIrcUserCmdOp();
	afx_msg void OnIrcUserCmdDeop();
	afx_msg void OnIrcUserCmdVoice();
	afx_msg void OnIrcUserCmdDevoice();

	afx_msg void OnIrcUserCmdKick();
	afx_msg void OnIrcUserCmdUnban();
	afx_msg void OnIrcUserCmdBan();
	afx_msg void OnIrcUserCmdBanKick();
	afx_msg void OnIrcUserCmdBanKickWhy();
	afx_msg void OnIrcUserCmdKickWhy();

	afx_msg void OnIrcChanCmdSave();

	afx_msg void OnUpdateIrcConnect(CCmdUI* pCmdUI);
	afx_msg void OnIrcConnect();
	afx_msg void OnUpdateIrcDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnIrcDisconnect();
	afx_msg void OnUpdateIrcCloseTab(CCmdUI* pCmdUI);
	afx_msg void OnIrcCloseTab();
	afx_msg void OnUpdateIrcSendText(CCmdUI* pCmdUI);
	afx_msg void OnIrcSendText();

	DECLARE_MESSAGE_MAP()
};
