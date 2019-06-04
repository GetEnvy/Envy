//
// CtrlWizard.h
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

class CXMLElement;
class CAlbumFolder;
class CLibraryFile;


class CWizardCtrl : public CWnd
{
	DECLARE_DYNAMIC(CWizardCtrl)

public:
	CWizardCtrl();
	virtual ~CWizardCtrl();

public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPCTSTR pszXMLPath, const CAlbumFolder* pFolder);

public:
	CString m_sMainFilePath;
	CString m_sEvenFilePath;
	CString m_sOddFilePath;

	CArray< CString >	m_pFileDocs;	// All documents for each file
	CArray< CString >	m_pFilePaths;	// All file paths
	CArray< CString >	m_pImagePaths;
	CArray< CString >	m_pTemplatePaths;
	CStringIMap			m_pItems;

protected:
	int		m_nCaptionWidth;
	int		m_nItemHeight;
	BOOL	m_bShowBorder;
	BOOL	m_bValid;
	CString	m_sEvenFile;
	CString	m_sOddFile;
	int		m_nScroll;

	CArray< CWnd* >   m_pControls;	// Holds all controls
	CArray< CString > m_pCaptions;	// All label texts

public:
	size_t	GetSize() const { return m_pControls.GetSize(); }
	BOOL	IsValid() const { return m_bValid; }
	BOOL	OnTab();

protected:
	void	Layout();
	void	Clear();
	void	ScrollBy(int nDelta);
	void	SetFocusTo(CWnd* pCtrl);
	BOOL	CollectFiles(CXMLElement* pBase);
	BOOL	CollectImages(CXMLElement* pBase);
	BOOL	MakeControls(const CString& sXMLPath, CXMLElement* pBase, std::vector< const CLibraryFile* > pList);
	void	MakeAll(const CString& sXMLPath, const CAlbumFolder* pFolder);
	BOOL	PrepareDoc(CLibraryFile* pFile, LPCTSTR pszTemplate = L"" );

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBtnPress();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};

#define IDC_WIZARD_CONTROL		100
