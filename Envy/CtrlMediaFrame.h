//
// CtrlMediaFrame.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2015 and Shareaza 2002-2007
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

#include "CtrlCoolBar.h"
#include "CtrlMediaList.h"


//class CLazySliderCtrl : public CSliderCtrl
//{
//public:
//	CLazySliderCtrl() : m_nPos( -1 ), m_nMin( -1 ), m_nMax( -1 ) {}
//
//	int GetPos() const
//	{
//		if ( m_nPos == -1 )
//			m_nPos = CSliderCtrl::GetPos();
//		return m_nPos;
//	}
//
//	void SetPos(_In_ int nPos)
//	{
//		if ( m_nPos != nPos )
//		{
//			m_nPos = nPos;
//			CSliderCtrl::SetPos( nPos );
//		}
//	}
//
//	void SetRange(_In_ int nMin, _In_ int nMax, _In_ BOOL bRedraw = FALSE)
//	{
//		if ( m_nMin != nMin || m_nMax != nMax )
//		{
//			m_nMin = nMin;
//			m_nMax = nMax;
//			CSliderCtrl::SetRange( nMin, nMax, bRedraw );
//		}
//	}
//
//private:
//	mutable int m_nPos, m_nMin, m_nMax;
//};


class CMediaFrame : public CWnd
{
	DECLARE_DYNAMIC(CMediaFrame)

public:
	CMediaFrame();
	virtual ~CMediaFrame();

public:
	void	OnSkinChange();
	void	OnUpdateCmdUI();
	BOOL	PlayFile(LPCTSTR pszFile);
	BOOL	EnqueueFile(LPCTSTR pszFile);
	void	OnFileDelete(LPCTSTR pszFile);
	BOOL	IsPlaying();
	float	GetPosition();
	float	GetVolume();
	BOOL	SeekTo(float nPosition);
	BOOL	SetVolume(float nVolume);
	BOOL	PaintStatusMicro(CDC& dc, CRect& rcBar);
	void	UpdateScreenSaverStatus(BOOL bWindowActive);

	CString	GetNowPlaying() const
	{
		return m_sNowPlaying;
	}

	static CMediaFrame* GetMediaFrame();

	inline IMediaPlayer* GetPlayer() { return m_pPlayer; }
	inline MediaState GetState() { return m_pPlayer != NULL ? m_nState : smsNull; }

protected:
	void	SetFullScreen(BOOL bFullScreen);
	void	PaintSplash(CDC& dc, CRect& rcBar);
	void	PaintListHeader(CDC& dc, CRect& rcBar);
	void	PaintStatus(CDC& dc, CRect& rcBar);
	BOOL	DoSizeList();
	BOOL	Prepare();
	BOOL	PrepareVis();
	BOOL	OpenFile(LPCTSTR pszFile);
	void	ZoomTo(MediaZoom nZoom);
	void	AspectTo(double nAspect);
	void	Cleanup();
	void	UpdateState();
	void	DisableScreenSaver();
	void	EnableScreenSaver();
	HRESULT PluginPlay(BSTR bsFileName);

private:
	void	UpdateNowPlaying(BOOL bEmpty = FALSE);
	BOOL	IsDisplayMeta(CMetaItem* pItem);

protected:
	static CMediaFrame*	m_wndMediaFrame;

	IMediaPlayer*	m_pPlayer;		// CComQIPtr< IMediaPlayer > (Shareaza r8522 for .exe)
	MediaState		m_nState;
	LONGLONG		m_nLength;
	LONGLONG		m_nPosition;
	BOOL			m_bMute;
	BOOL			m_bThumbPlay;
	BOOL			m_bRepeat;
	BOOL			m_bLastMedia;
	BOOL			m_bLastNotPlayed;
	BOOL			m_bEnqueue;
	BOOL			m_bStopFlag;
	DWORD			m_tLastPlay;

	CString			m_sFile;
	CMetaList		m_pMetadata;
	DWORD			m_tMetadata;

	CMediaListCtrl	m_wndList;
	CCoolBarCtrl	m_wndListBar;
	CCoolBarCtrl	m_wndToolBar;
	CSliderCtrl 	m_wndPosition;	// CLazySliderCtrl above
	CSliderCtrl 	m_wndSpeed; 	// CLazySliderCtrl above
	CSliderCtrl 	m_wndVolume;	// CLazySliderCtrl above

	BOOL			m_bFullScreen;
	DWORD			m_tBarTime;
	CPoint			m_ptCursor;
	BOOL			m_bListVisible;
	BOOL			m_bListWasVisible;
	int				m_nListSize;
	BOOL			m_bStatusVisible;

	CRect			m_rcVideo;
	CRect			m_rcStatus;
	CBitmap			m_bmLogo;
	CImageList		m_pIcons;
	CFont			m_pFontDefault;
	CFont			m_pFontKey;
	CFont			m_pFontValue;

	BOOL			m_bScreenSaverEnabled;
	ULONG			m_nVidAC, m_nVidDC;
	UINT			m_nPowerSchemeId, m_nScreenSaverTime;
	GLOBAL_POWER_POLICY m_CurrentGP;	// Current Global Power Policy
	POWER_POLICY	m_CurrentPP;		// Current Power Policy

private:
	CString			m_sNowPlaying;

public:
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar = NULL);
	afx_msg void OnClose();

	afx_msg void OnUpdateMediaClose(CCmdUI* pCmdUI);
	afx_msg void OnMediaClose();
	afx_msg void OnUpdateMediaPlay(CCmdUI* pCmdUI);
	afx_msg void OnMediaPlay();
	afx_msg void OnUpdateMediaPause(CCmdUI* pCmdUI);
	afx_msg void OnMediaPause();
	afx_msg void OnUpdateMediaStop(CCmdUI* pCmdUI);
	afx_msg void OnMediaStop();
	afx_msg void OnMediaZoom();
	afx_msg void OnUpdateMediaSizeFill(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeFill();
	afx_msg void OnUpdateMediaSizeDistort(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeDistort();
	afx_msg void OnUpdateMediaSizeOne(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeOne();
	afx_msg void OnUpdateMediaSizeTwo(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeTwo();
	afx_msg void OnUpdateMediaSizeHalf(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeHalf();
	afx_msg void OnUpdateMediaAspectDefault(CCmdUI* pCmdUI);
	afx_msg void OnMediaAspectDefault();
	afx_msg void OnUpdateMediaAspect43(CCmdUI* pCmdUI);
	afx_msg void OnMediaAspect43();
	afx_msg void OnUpdateMediaAspect169(CCmdUI* pCmdUI);
	afx_msg void OnMediaAspect169();
	afx_msg void OnUpdateMediaFullScreen(CCmdUI* pCmdUI);
	afx_msg void OnMediaFullScreen();
	afx_msg void OnUpdateMediaPlaylist(CCmdUI* pCmdUI);
	afx_msg void OnMediaPlaylist();
	afx_msg void OnMediaSettings();
	afx_msg void OnUpdateMediaSettings(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMediaVis(CCmdUI* pCmdUI);
	afx_msg void OnMediaVis();
	afx_msg void OnUpdateMediaStatus(CCmdUI* pCmdUI);
	afx_msg void OnMediaStatus();
	afx_msg void OnUpdateMediaMute(CCmdUI* pCmdUI);
	afx_msg void OnMediaMute();

	afx_msg void OnNewCurrent(NMHDR* pNotify, LRESULT* pResult);
	afx_msg LRESULT OnMediaKey(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

#define IDC_MEDIA_PLAYLIST		120

#define VK_MEDIA_NEXT_TRACK		0xB0
#define VK_MEDIA_PREV_TRACK		0xB1
#define VK_MEDIA_STOP			0xB2
#define VK_MEDIA_PLAY_PAUSE		0xB3
