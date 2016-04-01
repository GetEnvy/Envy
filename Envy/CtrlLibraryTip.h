//
// CtrlLibraryTip.h
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

#include "EnvyFile.h"
#include "ThreadImpl.h"
#include "CtrlCoolTip.h"
#include "MetaList.h"


class CLibraryTipCtrl : public CCoolTipCtrl, public CThreadImpl
{
	DECLARE_DYNAMIC(CLibraryTipCtrl)

public:
	CLibraryTipCtrl();
	virtual ~CLibraryTipCtrl();

public:
	void Show(DWORD pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_nFileIndex );
		m_nFileIndex = pContext;
		m_pFile = NULL;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

	void Show(CEnvyFile* pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_pFile );
		m_nFileIndex = 0;
		m_pFile = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	CEnvyFile*	m_pFile;
	DWORD				m_nFileIndex;
	CString 			m_sName;
	CString 			m_sPath;
	CString 			m_sFolder;
	CString 			m_sSize;
	CString 			m_sType;
	CString 			m_sSHA1;
	CString 			m_sTTH;
	CString 			m_sED2K;
	CString 			m_sBTH;
	CString 			m_sMD5;
	int 				m_nIcon;
	int 				m_nKeyWidth;
	CMetaList			m_pMetadata;
	CCriticalSection	m_pSection;
	CBitmap				m_bmThumb;
	DWORD				m_tHidden;

	void		OnRun();
	void		StopThread();

	virtual BOOL OnPrepare();
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnPaint(CDC* pDC);

protected:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
