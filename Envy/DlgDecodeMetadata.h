//
// DlgDecodeMetadata.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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
#include "SharedFile.h"

class CDecodeMetadataDlg : public CSkinDialog
{
public:
	CDecodeMetadataDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_CODEPAGES };

public:
	CButton		m_wndOK;
	CComboBox	m_wndCodepages;
	CString		m_sOriginalWords;
	CString		m_sPreview1;
	CString		m_sPreview2;

	CList<DWORD> m_pFiles;

protected:
	int 		m_nMethod;

protected:
	static const unsigned codePages[];
	virtual void DoDataExchange(CDataExchange* pDX);

public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void AddFile(CLibraryFile* pFile);
	void GetEncodedText(CString& strText, int nMethod = 0) const;

protected:
	afx_msg void OnClickedMethod1();
	afx_msg void OnClickedMethod2();
	afx_msg void OnCloseupCodepages();
	afx_msg void OnSelchangeCodepages();

	DECLARE_MESSAGE_MAP()
};
