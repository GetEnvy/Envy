//
// CtrlLibraryCollectionView.h
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

#include "CtrlLibraryFileView.h"

class CCollectionFile;
class CLibraryCollectionView;
class CLibraryFile;
class CWebCtrl;


class CHtmlCollection : public CComObject
{
	DECLARE_DYNAMIC(CHtmlCollection)

public:
	CHtmlCollection();
	virtual ~CHtmlCollection();

protected:
	CLibraryCollectionView*	m_pView;

	BEGIN_INTERFACE_PART(View, ICollectionHtmlView)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication **ppApplication);
		STDMETHOD(Detect)(BSTR sURN, BSTR *psState);
		STDMETHOD(Hover)(BSTR sURN);
		STDMETHOD(Open)(BSTR sURN, VARIANT_BOOL *pbResult);
		STDMETHOD(Enqueue)(BSTR sURN, VARIANT_BOOL *pbResult);
		STDMETHOD(Download)(BSTR sURN, VARIANT_BOOL *pbResult);
		STDMETHOD(DownloadAll)();
		STDMETHOD(get_MissingCount)(LONG *pnCount);
	END_INTERFACE_PART(View)

	DECLARE_INTERFACE_MAP()

	friend class CLibraryCollectionView;
};


class CLibraryCollectionView : public CLibraryFileView
{
	DECLARE_DYNAMIC(CLibraryCollectionView)

public:
	CLibraryCollectionView();
	virtual ~CLibraryCollectionView();

protected:
	CWebCtrl*			m_pWebCtrl;
	DWORD				m_nWebIndex;
	CCollectionFile*	m_pCollection;
	Hashes::Sha1Hash	m_oSHA1;
	CHtmlCollection		m_xExternal;
	TRISTATE			m_bTrusted;
	BOOL				m_bLockdown;

public:
	virtual BOOL	CheckAvailable(CLibraryTreeItem* pSel);
	virtual void	SelectAll() {}
	virtual void	Update();
protected:
	BOOL			ShowCollection(CLibraryFile* pFile);
	virtual DWORD_PTR HitTestIndex(const CPoint& /*point*/) const { return 0; };
	virtual BOOL Create(CWnd* pParentWnd);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWebContextMenu(NMHDR* pNMHDR, LPARAM* pResult);
	afx_msg void OnUpdateLibraryFolderDownload(CCmdUI *pCmdUI);
	afx_msg void OnLibraryFolderDownload();
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()

	friend class CHtmlCollection;
};
