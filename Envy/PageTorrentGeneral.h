//
// PageTorrentGeneral.h
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


class CTorrentGeneralPage : public CPropertyPageAdv
{
	DECLARE_DYNCREATE(CTorrentGeneralPage)

public:
	CTorrentGeneralPage();
	virtual ~CTorrentGeneralPage();

	enum { IDD = IDD_TORRENT_GENERAL };

protected:
	CString			m_sName;
	CString			m_sComment;
	CString			m_sCreatedBy;
	CString			m_sCreationDate;
	CString			m_sTorrentOther;
	CComboBox		m_wndStartDownloads;
	CString			m_sUploadTotal;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL	OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
};
