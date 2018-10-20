//
// CtrlMatchTip.h
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

#include "CtrlCoolTip.h"
#include "MetaList.h"

class CMatchFile;
class CQueryHit;


class CMatchTipCtrl : public CCoolTipCtrl
{
	DECLARE_DYNAMIC(CMatchTipCtrl)

public:
	CMatchTipCtrl();
//	virtual ~CMatchTipCtrl();

public:
	void		Show(CMatchFile* pFile, CQueryHit* pHit);

protected:
	CMatchFile*	m_pFile;
	CQueryHit*	m_pHit;

	CString		m_sName;
	CString		m_sUser;
	CString		m_sCountryCode;
	CString		m_sCountry;
	CString		m_sSHA1;
	CString		m_sTiger;
	CString		m_sED2K;
	CString		m_sBTH;
	CString		m_sMD5;
	CString		m_sType;
	CString		m_sSize;
	CString		m_sBusy;		// Busy status message
	CString		m_sPush;		// Firewalled status message
	CString		m_sUnstable;	// Unstable status message
	int 		m_nIcon;
	CString		m_sStatus;
	COLORREF	m_crStatus;
	CString		m_sPartial;
	CString		m_sQueue;
	CSchemaPtr	m_pSchema;
	CMetaList	m_pMetadata;
	int 		m_nKeyWidth;
	int 		m_nRating;

protected:
	void		LoadFromFile();
	void		LoadFromHit();
	BOOL		LoadTypeInfo();

protected:
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);
	virtual void OnCalcSize(CDC* pDC);
	virtual BOOL OnPrepare();

	//DECLARE_MESSAGE_MAP()
};
