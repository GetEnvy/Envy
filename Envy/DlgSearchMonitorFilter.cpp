//
// DlgSearchMonitorFilter.cpp
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "DlgSearchMonitorFilter.h"
#include "WndSearchMonitor.h"
#include "CoolInterface.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

BEGIN_MESSAGE_MAP(CDlgSearchMonitorFilter, CSkinDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSearchMonitorFilter dialog

CDlgSearchMonitorFilter::CDlgSearchMonitorFilter(CWnd* pParent, SearchFilterCriteria* pCriteria)
	: CSkinDialog(CDlgSearchMonitorFilter::IDD, pParent)
	, m_pCriteria(pCriteria)
	, m_bProtocolG2(FALSE)
	, m_bProtocolG1(FALSE)
	, m_bProtocolED2K(FALSE)
	, m_bProtocolDC(FALSE)
	, m_bFilterEnabled(FALSE)
{
	if (m_pCriteria)
	{
		m_sTextFilter = m_pCriteria->sTextFilter;
		m_sIPFilter = m_pCriteria->sIPFilter;
		m_sSchemaFilter = m_pCriteria->sSchemaFilter;
		m_sMinSize = m_pCriteria->nMinSize > 0 ? Settings.SmartVolume(m_pCriteria->nMinSize) : L"";
		m_sMaxSize = m_pCriteria->nMaxSize > 0 ? Settings.SmartVolume(m_pCriteria->nMaxSize) : L"";
		m_bFilterEnabled = m_pCriteria->bFilterEnabled;

		// Check which protocols are selected
		for (int i = 0; i < m_pCriteria->aProtocols.GetSize(); i++)
		{
			if (m_pCriteria->aProtocols[i] == L"G2")
				m_bProtocolG2 = TRUE;
			else if (m_pCriteria->aProtocols[i] == L"G1")
				m_bProtocolG1 = TRUE;
			else if (m_pCriteria->aProtocols[i] == L"ED2K")
				m_bProtocolED2K = TRUE;
			else if (m_pCriteria->aProtocols[i] == L"DC++")
				m_bProtocolDC = TRUE;
		}
	}
}

void CDlgSearchMonitorFilter::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FILTER_TEXT, m_sTextFilter);
	DDX_Text(pDX, IDC_FILTER_IP, m_sIPFilter);
	DDX_Text(pDX, IDC_FILTER_SCHEMA, m_sSchemaFilter);
	DDX_Text(pDX, IDC_FILTER_MIN_SIZE, m_sMinSize);
	DDX_Text(pDX, IDC_FILTER_MAX_SIZE, m_sMaxSize);
	DDX_Check(pDX, IDC_FILTER_PROTOCOL_G2, m_bProtocolG2);
	DDX_Check(pDX, IDC_FILTER_PROTOCOL_G1, m_bProtocolG1);
	DDX_Check(pDX, IDC_FILTER_PROTOCOL_ED2K, m_bProtocolED2K);
	DDX_Check(pDX, IDC_FILTER_PROTOCOL_DC, m_bProtocolDC);
	DDX_Check(pDX, IDC_FILTER_ENABLED, m_bFilterEnabled);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSearchMonitorFilter message handlers

BOOL CDlgSearchMonitorFilter::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe(L"CDlgSearchMonitorFilter", IDR_SEARCHMONITORFRAME);

	UpdateData(FALSE);

	return TRUE;
}

void CDlgSearchMonitorFilter::OnOK()
{
	UpdateData(TRUE);

	if (m_pCriteria)
	{
		m_pCriteria->sTextFilter = m_sTextFilter;
		m_pCriteria->sIPFilter = m_sIPFilter;
		m_pCriteria->sSchemaFilter = m_sSchemaFilter;
		m_pCriteria->nMinSize = Settings.ParseVolume(m_sMinSize);
		m_pCriteria->nMaxSize = Settings.ParseVolume(m_sMaxSize);
		m_pCriteria->bFilterEnabled = m_bFilterEnabled == TRUE;

		// Update protocol list
		m_pCriteria->aProtocols.RemoveAll();
		if (m_bProtocolG2)
			m_pCriteria->aProtocols.Add(L"G2");
		if (m_bProtocolG1)
			m_pCriteria->aProtocols.Add(L"G1");
		if (m_bProtocolED2K)
			m_pCriteria->aProtocols.Add(L"ED2K");
		if (m_bProtocolDC)
			m_pCriteria->aProtocols.Add(L"DC++");
	}

	CSkinDialog::OnOK();
}
