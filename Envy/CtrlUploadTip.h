//
// CtrlUploadTip.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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

#include "CtrlCoolTip.h"
#include "BTTrackerRequest.h"

class CUpload;
class CUploadFile;
class CLineGraph;
class CGraphItem;


class CUploadTipCtrl : public CCoolTipCtrl, public CTrackerEvent /* Scrape */
{
	DECLARE_DYNAMIC(CUploadTipCtrl)

public:
	CUploadTipCtrl();
	virtual ~CUploadTipCtrl();

public:
	void Show(CUploadFile* pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_pUploadFile );
		m_pUploadFile = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	CUploadFile*		m_pUploadFile;
	CLineGraph*			m_pGraph;
	CGraphItem*			m_pItem;
	CString				m_sAddress;
	CString				m_sSeedsPeers;
	int					m_nHeaderWidth;
	CArray< CString >	m_pHeaderName;
	CArray< CString >	m_pHeaderValue;

protected:
	void DrawProgressBar(CDC* pDC, CPoint* pPoint, CUploadFile* pFile);

protected:
	virtual void OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip, CBTTrackerRequest* pEvent);

	virtual BOOL OnPrepare();
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnPaint(CDC* pDC);

protected:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
