//
// DlgURLCopy.h
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

#include "DlgSkinDialog.h"
#include "EnvyFile.h"


class CURLCopyDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CURLCopyDlg)

public:
	CURLCopyDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_URL_COPY };

protected:
	CEnvyFile m_pFile;
	CButton		m_wndIncludeSelf;
	CStatic		m_wndMessage;
//	CString		m_sGnutella;
	CString		m_sMagnet;
	CString		m_sED2K;
	CString		m_sHost;
	CString		m_sHash;

public:
	void		Add(const CEnvyFile* pFile);

	// Gather more information about file (including trackers list)
	static void Resolve(CEnvyFile& pFile, CString& sTracker);
	static CString CreateMagnet(CEnvyFile& pFile);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnStnClickedUrlMagnet();
	afx_msg void OnStnClickedUrlEd2k();
	afx_msg void OnStnClickedUrlHost();
	afx_msg void OnStnClickedUrlHash();
	afx_msg void OnIncludeSelf();

	DECLARE_MESSAGE_MAP()
};
