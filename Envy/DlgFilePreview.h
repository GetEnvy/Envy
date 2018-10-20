//
// DlgFilePreview.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2012
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
#include "DownloadWithExtras.h"

//class CDownloadWithExtras;
class CFilePreviewDlg;


typedef CList< CFilePreviewDlg* > CPreviewList;


class CFilePreviewDlg :
	public CSkinDialog,
	public CThreadImpl
{
	DECLARE_DYNAMIC(CFilePreviewDlg)

public:
	CFilePreviewDlg(CDownloadWithExtras* pDownload, DWORD nIndex, CWnd* pParent = NULL);
	virtual ~CFilePreviewDlg();

	enum { IDD = IDD_FILE_PREVIEW };

public:
	static void	OnSkinChange(BOOL bSet);
	static void	CloseAll();

protected:
	CDownloadWithExtras*	m_pDownload;
	CString					m_sDisplayName;
	CString					m_sSourceName;
	CString					m_sTargetName;
	QWORD					m_nRange;
	QWORD					m_nPosition;
	DWORD					m_nScaled;
	DWORD					m_nOldScaled;
	CString					m_sStatus;
	CString					m_sOldStatus;
	CArray< QWORD > 		m_pRanges;

	CString					m_sExecute;
	CButton					m_wndCancel;
	CProgressCtrl			m_wndProgress;
	CStatic					m_wndStatus;
	CStatic					m_wndName;

	CComQIPtr< IDownloadPreviewPlugin2 > m_pPlugin;
	static CCriticalSection	m_pSection;
	static CPreviewList		m_pWindows;

	void		OnRun();
	BOOL		RunPlugin();
	BOOL		RunManual();
	BOOL		QueueDeleteFile(LPCTSTR pszFile);
	BOOL		ExecuteFile(LPCTSTR pszFile);
	void		UpdateProgress(BOOL bRange, QWORD nRange, BOOL bPosition, QWORD nPosition);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void PostNcDestroy();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()

// IDownloadPreviewSite
	BEGIN_INTERFACE_PART(DownloadPreviewSite, IDownloadPreviewSite)
		STDMETHOD(GetSuggestedFilename)(BSTR FAR* psFile);
		STDMETHOD(GetAvailableRanges)(SAFEARRAY FAR* FAR* ppArray);
		STDMETHOD(SetProgressRange)(DWORD nRange);
		STDMETHOD(SetProgressPosition)(DWORD nPosition);
		STDMETHOD(SetProgressMessage)(BSTR sMessage);
		STDMETHOD(QueueDeleteFile)(BSTR sTempFile);
		STDMETHOD(ExecuteFile)(BSTR sFile);
	END_INTERFACE_PART(DownloadPreviewSite)

	DECLARE_INTERFACE_MAP()
};
