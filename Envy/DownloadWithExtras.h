//
// DownloadWithExtras.h
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

#include "DownloadWithSearch.h"

class CDownloadMonitorDlg;
class CFilePreviewDlg;


// CDownloadReview stores a review of a download. Can be a G2 review, copied from the search, or an ed2k review received during download.
class CDownloadReview
{
public:
	CDownloadReview();
	CDownloadReview(in_addr *pIP, int nUserPicture, int nRating, LPCTSTR pszUserName, LPCTSTR pszComment);
	~CDownloadReview();

public:
	in_addr			m_pUserIP;			// To prevent duplicate reviews
	CString			m_sUserName;		// User who made comments
	int				m_nUserPicture;		// Picture to display. 2 = G2, 3 = ED2K
	int				m_nFileRating;		// 0 = Unrated, 1 = Fake, 1-6 = Num Stars
	CString			m_sFileComments;	// The review/comments

	CDownloadReview* m_pNext;			// Next review in list (or NULL for the last)
	CDownloadReview* m_pPrev;			// PRevious review in list (or NULL for the first)

	void Serialize(CArchive& ar, int nVersion);

	friend class		CDownloadWithExtras;
};


class CDownloadWithExtras : public CDownloadWithSearch
{
protected:
	CDownloadWithExtras();
	virtual ~CDownloadWithExtras();

private:
	CList< CString > m_pPreviews;
	CFilePreviewDlg* m_pPreviewWnd;
	CDownloadMonitorDlg* m_pMonitorWnd;

protected:
	CDownloadReview* m_pReviewFirst;
	CDownloadReview* m_pReviewLast;
	DWORD			 m_nReviewCount;

public:
	BOOL			m_bRemotePreviewCapable;
	BOOL			m_bWaitingPreview;	// The remote preview request was sent

public:
	BOOL			GotPreview() const;
	BOOL			CanPreview(DWORD nIndex);
	BOOL			PreviewFile(DWORD nIndex, CSingleLock* pLock);
	BOOL			IsPreviewVisible() const;
	BOOL			IsMonitorVisible() const;
	void			ShowMonitor(CSingleLock* pLock = NULL);
	void			AddPreviewName(LPCTSTR pszFile);
	void			DeletePreviews();
	BOOL			AddReview(IN_ADDR* pIP, int nUserPicture, int nRating, LPCTSTR pszUserName, LPCTSTR pszComment);
	BOOL			AddReview(CDownloadReview* pReview);
	void			DeleteReviews();
	void			DeleteReview(CDownloadReview* pReview);
	int				GetReviewCount() const { return m_nReviewCount; }
	int				GetReviewAverage() const;

protected:
	virtual void	Serialize(CArchive& ar, int nVersion);

public:
	CDownloadReview* GetFirstReview() const { return m_pReviewFirst; }
private:
	CDownloadReview* FindReview(IN_ADDR* pIP) const;
	CDownloadReview* FindReview(LPCTSTR pszUserName) const;
	CDownloadReview* FindReview(int nRating, LPCTSTR pszName, LPCTSTR pszComment) const;

public:
	friend class CDownloadMonitorDlg;
	friend class CFilePreviewDlg;
};
