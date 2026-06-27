//
// DlgSearchMonitorFilter.h
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

struct SearchFilterCriteria;

class CDlgSearchMonitorFilter : public CSkinDialog
{
public:
	CDlgSearchMonitorFilter(CWnd* pParent = NULL, SearchFilterCriteria* pCriteria = NULL);

	enum { IDD = IDD_SEARCHMONITOR_FILTER };

public:
	CString m_sTextFilter;
	CString m_sIPFilter;
	CString m_sSchemaFilter;
	CString m_sMinSize;
	CString m_sMaxSize;
	BOOL m_bProtocolG2;
	BOOL m_bProtocolG1;
	BOOL m_bProtocolED2K;
	BOOL m_bProtocolDC;
	BOOL m_bFilterEnabled;

	SearchFilterCriteria* m_pCriteria;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
