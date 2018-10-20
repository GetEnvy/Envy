//
// DlgCollectionExport.h
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
#include "CtrlWizard.h"

class CAlbumFolder;


class CCollectionExportDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CCollectionExportDlg)

public:
	CCollectionExportDlg(const CAlbumFolder* pFolder, CWnd* pParent = NULL);
	virtual ~CCollectionExportDlg();

	enum { IDD = IDD_COLLECTION_EXPORT };

protected:
	CButton		m_wndOK;
	CStatic		m_wndExplain;
	CStatic		m_wndLblAuthor;
	CStatic		m_wndLblName;
	CStatic		m_wndLblDesc;
	CStatic		m_wndGroupBox;
	CButton		m_wndDelete;
	CEdit		m_wndDesc;
	CStatic		m_wndName;
	CStatic		m_wndAuthor;
	CListCtrl	m_wndList;

protected:
	const CAlbumFolder* m_pFolder;
	CImageList	m_gdiImageList;
	BOOL		m_bThumbnails;
	CString		m_sXMLPath;
	CString		m_sNewFilename;
	int			m_nSelected;
	int			m_nStep;

	CWizardCtrl	m_wndWizard;
	CString		m_sBtnBack;
	CString		m_sBtnDelete;
	CString		m_sBtnExport;
	CString		m_sBtnNext;
	CString		m_sLblExplain1;
	CString		m_sLblExplain2;

protected:
	void		EnumerateTemplates(LPCTSTR pszPath = NULL);
	BOOL		AddTemplate(LPCTSTR pszPath, LPCTSTR pszName);
	CString		DirFromPath(LPCTSTR szPath);
	BOOL		ErrorMessage(LPCTSTR pszError, LPCTSTR pszTarget = NULL);
	BOOL		Step1();	// First wizard screen
	BOOL		Step2();	// Second wizard screen

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

protected:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnItemChangedTemplates(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTemplatesDeleteOrBack();

	DECLARE_MESSAGE_MAP()
};

#define IDC_WIZARD		99
