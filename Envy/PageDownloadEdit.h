//
// PageDownloadEdit.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
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
#include "WndSettingsPage.h"	// For CEditPath
//#include <afxdtctl.h>			// MFC date & time controls

class CDownload;


class CDownloadEditPage : public CPropertyPageAdv
{
	DECLARE_DYNAMIC(CDownloadEditPage)

public:
	CDownloadEditPage();
	virtual ~CDownloadEditPage();

	enum { IDD = IDD_DOWNLOAD_EDIT };

protected:
	CString	m_sName;
	CString	m_sDiskName;
	CString	m_sFileSize;
	CString	m_sSHA1;
	CString	m_sTiger;
	CString	m_sED2K;
	CString	m_sMD5;
	CString	m_sBTH;
	BOOL	m_bSHA1Trusted;
	BOOL	m_bTigerTrusted;
	BOOL	m_bED2KTrusted;
	BOOL	m_bMD5Trusted;
	BOOL	m_bBTHTrusted;

	CDateTimeCtrl	m_wndDate;
	CEditPath		m_wndPath;	// For Double-click aware Edit box

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	//DECLARE_MESSAGE_MAP()
};
