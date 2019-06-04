//
// PageFileMetadata.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2010
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

#include "DlgFilePropertiesPage.h"
#include "CtrlSchema.h"
#include "CtrlSchemaCombo.h"

class CFileMetadataPage : public CFilePropertiesPage
{
	DECLARE_DYNCREATE(CFileMetadataPage)

public:
	CFileMetadataPage();

	enum { IDD = IDD_FILE_METADATA };

public:
	CSchemaCombo	m_wndSchemas;
	CSchemaCtrl		m_wndData;
	CXMLElement*	m_pXML;

protected:
	CAutoPtr< CXMLElement > m_pSchemaContainer;

	DWORD UpdateFileData(BOOL bRealSave = TRUE);
	void AddCrossAttributes(CXMLElement* pXML, LPCTSTR pszTargetURI);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnCloseUpSchemas();

	DECLARE_MESSAGE_MAP()
};

#define IDC_METADATA	100
