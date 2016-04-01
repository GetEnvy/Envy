//
// CtrlSearchPanel.h
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

#include "AutocompleteEdit.h"
#include "CtrlTaskPanel.h"
#include "CtrlSchemaCombo.h"
#include "CtrlSchema.h"
#include "CtrlIconButton.h"
#include "ManagedSearch.h"


class CSearchInputBox : public CTaskBox
{
	DECLARE_DYNAMIC(CSearchInputBox)

public:
	CSearchInputBox();
	virtual ~CSearchInputBox();

public:
	CAutocompleteEdit m_wndSearch;
	CSchemaCombo	m_wndSchemas;
	CIconButtonCtrl	m_wndStart;
	CIconButtonCtrl	m_wndStop;
	CIconButtonCtrl	m_wndPrefix;

public:
	void	OnSkinChange();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnCloseUpSchemas();
	afx_msg void OnSearchStart();
	afx_msg void OnSearchStop();
	afx_msg void OnSearchPrefix();
	afx_msg void OnSearchPrefixSHA1();
	afx_msg void OnSearchPrefixTiger();
	afx_msg void OnSearchPrefixSHA1Tiger();
	afx_msg void OnSearchPrefixED2K();
	afx_msg void OnSearchPrefixBTH();
	afx_msg void OnSearchPrefixMD5();

	DECLARE_MESSAGE_MAP()
};


class CSearchAdvancedBox : public CTaskBox
{
	DECLARE_DYNAMIC(CSearchAdvancedBox)

public:
	CSearchAdvancedBox();
	virtual ~CSearchAdvancedBox();

public:
	CStatic		m_wndSizeMinMax;
	CComboBox	m_wndSizeMin;
	CComboBox	m_wndSizeMax;
	CButton		m_wndCheckBoxG1;
	CButton		m_wndCheckBoxG2;
	CButton		m_wndCheckBoxED2K;
	CButton		m_wndCheckBoxDC;
	CBrush		m_brBack;
	COLORREF	m_crBack;
	CImageList	m_gdiProtocols;

public:
	void		OnSkinChange();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnG2Clicked();
	afx_msg void OnG1Clicked();
	afx_msg void OnED2KClicked();
	afx_msg void OnDCClicked();
	afx_msg LRESULT OnCtlColorStatic(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};


class CSearchSchemaBox : public CTaskBox
{
	DECLARE_DYNAMIC(CSearchSchemaBox)

public:
	CSearchSchemaBox();
	virtual ~CSearchSchemaBox();

public:
	CSchemaCtrl	m_wndSchema;

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};


class CSearchResultsBox : public CTaskBox
{
	DECLARE_DYNAMIC(CSearchResultsBox)

public:
	CSearchResultsBox();
	virtual ~CSearchResultsBox();

public:
	BOOL	m_bActive;
	DWORD	m_nHubs;
	DWORD	m_nLeaves;
	DWORD	m_nFiles;
	DWORD	m_nHits;
	DWORD	m_nBadHits;

public:
	void	Update(BOOL bSearching, DWORD nHubs, DWORD nLeaves, DWORD nFiles, DWORD nHits, DWORD nBadHits);

protected:
	static void  DrawText(CDC* pDC, int nX, int nY, UINT nFlags, LPCTSTR pszText);

	virtual void OnExpanded(BOOL bOpen);

protected:
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
};


class CSearchPanel : public CTaskPanel
{
	DECLARE_DYNAMIC(CSearchPanel)

public:
	CSearchPanel();

public:
	BOOL				m_bSendSearch;
	CSearchInputBox		m_boxSearch;
	CSearchAdvancedBox	m_boxAdvanced;
	CSearchSchemaBox	m_boxSchema;
	CSearchResultsBox	m_boxResults;
	BOOL				m_bAdvanced;

public:
	CSearchPtr	GetSearch();
	void		SetSearchFocus();
	void		ShowSearch(const CManagedSearch* pSearch);
	void		ShowStatus(BOOL bStarted, BOOL bSearching, DWORD nHubs, DWORD nLeaves, DWORD nFiles, DWORD nHits, DWORD nBadHits);
	void		OnSchemaChange();
	void		OnSkinChange();
	void		ExecuteSearch();
	void		Enable();
	void		Disable();

public:
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#define IDC_SEARCH_SIZEMIN				108
#define IDC_SEARCH_SIZEMAX				109

#define IDC_SEARCH_PREFIX				120
#define IDC_SEARCH_PREFIX_SHA1			121
#define IDC_SEARCH_PREFIX_TIGER			122
#define IDC_SEARCH_PREFIX_SHA1_TIGER	123
#define IDC_SEARCH_PREFIX_ED2K			124
#define IDC_SEARCH_PREFIX_BTH			125
#define IDC_SEARCH_PREFIX_MD5			126

#define IDC_SEARCH_GNUTELLA1			277
#define IDC_SEARCH_GNUTELLA2			278
#define IDC_SEARCH_EDONKEY				279
#define IDC_SEARCH_DC					280
