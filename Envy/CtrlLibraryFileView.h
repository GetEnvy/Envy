//
// CtrlLibraryFileView.h
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

#include "CtrlLibraryView.h"

class CLibraryFile;
class CMetaList;


class CLibraryFileView : public CLibraryView
{
	DECLARE_DYNAMIC(CLibraryFileView)

public:
	CLibraryFileView();
	virtual ~CLibraryFileView();

protected:
	BOOL				m_bEditing;
	BOOL				m_bRequestingService;
	INT_PTR				m_nCurrentPage;
	CList<CMetaList*>	m_pServiceDataPages;
	BOOL				m_bServiceFailed;

protected:
	void				CheckDynamicBar();
	void				ClearServicePages();

	virtual void		SelectAll() = 0;
	virtual BOOL		CheckAvailable(CLibraryTreeItem* pSel);

public:
	virtual BOOL		PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateLibraryLaunch(CCmdUI* pCmdUI);
	afx_msg void OnLibraryLaunch();
	afx_msg void OnUpdateLibraryEnqueue(CCmdUI* pCmdUI);
	afx_msg void OnLibraryEnqueue();
	afx_msg void OnUpdateLibraryLaunchFolder(CCmdUI* pCmdUI);
	afx_msg void OnLibraryLaunchFolder();
	afx_msg void OnUpdateLibraryURL(CCmdUI* pCmdUI);
	afx_msg void OnLibraryURL();
	afx_msg void OnUpdateLibraryMove(CCmdUI* pCmdUI);
	afx_msg void OnLibraryMove();
	afx_msg void OnUpdateLibraryCopy(CCmdUI* pCmdUI);
	afx_msg void OnLibraryCopy();
	afx_msg void OnUpdateLibraryDelete(CCmdUI* pCmdUI);
	afx_msg void OnLibraryDelete();
	afx_msg void OnUpdateLibraryRefreshMetadata(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRefreshMetadata();
	afx_msg void OnUpdateLibraryShared(CCmdUI* pCmdUI);
	afx_msg void OnLibraryShared();
	afx_msg void OnUpdateLibraryProperties(CCmdUI* pCmdUI);
	afx_msg void OnLibraryProperties();
	afx_msg void OnUpdateLibraryUnlink(CCmdUI* pCmdUI);
	afx_msg void OnLibraryUnlink();
	afx_msg void OnUpdateSearchForThis(CCmdUI* pCmdUI);
	afx_msg void OnSearchForThis();
	afx_msg void OnUpdateSearchForSimilar(CCmdUI* pCmdUI);
	afx_msg void OnSearchForSimilar();
	afx_msg void OnUpdateSearchForArtist(CCmdUI* pCmdUI);
	afx_msg void OnSearchForArtist();
	afx_msg void OnUpdateSearchForAlbum(CCmdUI* pCmdUI);
	afx_msg void OnSearchForAlbum();
	afx_msg void OnUpdateSearchForSeries(CCmdUI* pCmdUI);
	afx_msg void OnSearchForSeries();
	afx_msg void OnUpdateLibraryCreateTorrent(CCmdUI* pCmdUI);
	afx_msg void OnLibraryCreateTorrent();
	afx_msg void OnUpdateLibraryRebuildAnsi(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRebuildAnsi();
	afx_msg void OnUpdateLibraryRebuild(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRebuild();

	// WebServices:
	afx_msg void OnUpdateLibraryBitprintsWeb(CCmdUI* pCmdUI);
	afx_msg void OnLibraryBitprintsWeb();
	afx_msg void OnUpdateLibraryBitprintsDownload(CCmdUI* pCmdUI);
	afx_msg void OnLibraryBitprintsDownload();
	afx_msg void OnUpdateMusicBrainzLookup(CCmdUI* pCmdUI);
	afx_msg void OnMusicBrainzLookup();
	afx_msg void OnUpdateMusicBrainzMatches(CCmdUI* pCmdUI);
	afx_msg void OnMusicBrainzMatches();
	afx_msg void OnUpdateMusicBrainzAlbums(CCmdUI* pCmdUI);
	afx_msg void OnMusicBrainzAlbums();
	// Legacy Sharemonkey:
	//afx_msg void OnUpdateShareMonkeyLookup(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeyLookup();
	//afx_msg void OnUpdateShareMonkeyDownload(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeyDownload();
	//afx_msg void OnUpdateShareMonkeySave(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeySave();
	//afx_msg void OnUpdateShareMonkeySaveOption(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeySaveOption();
	//afx_msg void OnUpdateShareMonkeyPrevious(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeyPrevious();
	//afx_msg void OnUpdateShareMonkeyNext(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeyNext();
	//afx_msg void OnUpdateShareMonkeyPrices(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeyPrices();
	//afx_msg void OnUpdateShareMonkeyCompare(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeyCompare();
	//afx_msg void OnUpdateShareMonkeyBuy(CCmdUI* pCmdUI);
	//afx_msg void OnShareMonkeyBuy();

	afx_msg LRESULT OnServiceDone(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
