//
// DlgDownloadSheet.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2002-2006
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

#include "PagePropertyAdv.h"
#include "Download.h"


class CDownloadSheet : public CPropertySheetAdv
{
	DECLARE_DYNAMIC(CDownloadSheet)

public:
	CDownloadSheet(CSingleLock& pLock, CDownload* pDownload);

	CDownload*		GetDownload() const;

	virtual INT_PTR DoModal();

protected:
	CSingleLock&	m_pLock;		// Transfers.m_pSection
	CDownload*		m_pDownload;

	CString			m_sFilesTitle;
	CString			m_sTrackersTitle;
	CString			m_sGeneralTitle;
	CString			m_sDownloadTitle;
	CString			m_sActionsTitle;

	virtual BOOL	OnInitDialog();

//	afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
//	afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};
