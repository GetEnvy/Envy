//
// DlgHitColumns.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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

#include "DlgSkinDialog.h"
#include "CtrlSchemaCombo.h"
#include "Schema.h"


class CSchemaColumnsDlg : public CSkinDialog
{
public:
	CSchemaColumnsDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SCHEMA_COLUMNS };

public:
	CListCtrl	m_wndColumns;
	CSchemaCombo	m_wndSchemas;

public:
	CSchemaPtr	m_pSchema;
	CList< CSchemaMember* >	m_pColumns;

public:
	static CMenu*	BuildColumnMenu(CSchemaPtr pSchema, CList< CSchemaMember* >* pColumns = NULL);
	static BOOL		LoadColumns(CSchemaPtr pSchema, CList< CSchemaMember* >* pColumns);
	static BOOL		SaveColumns(CSchemaPtr pSchema, CList< CSchemaMember* >* pColumns);
	static BOOL		ToggleColumnHelper(CSchemaPtr pSchema, CList< CSchemaMember* >* pSource, CList< CSchemaMember* >* pTarget, UINT nToggleID, BOOL bSave = FALSE);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeSchemas();

	DECLARE_MESSAGE_MAP()
};
