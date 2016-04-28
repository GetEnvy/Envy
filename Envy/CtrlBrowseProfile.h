//
// CtrlBrowseProfile.h
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

#include "RichViewCtrl.h"
#include "ImageServices.h"
#include "ImageFile.h"

#define IDC_BROWSE_PROFILE	124

class CHostBrowser;
class CGProfile;
class CG2Packet;


class CBrowseProfileCtrl : public CWnd
{
public:
	CBrowseProfileCtrl();
	virtual ~CBrowseProfileCtrl();

protected:
	CCriticalSection	m_pSection;
	CRichViewCtrl		m_wndDoc1;
	CRichViewCtrl		m_wndDoc2;
	CRichDocument*		m_pDocumentLeft;
	CRichDocument*		m_pDocumentRight;
	CRichElement*		m_pdNick;
	CRichElement*		m_pdFullName;
	CRichElement*		m_pdFullLocation;
	CRichElement*		m_pdGenderMale;
	CRichElement*		m_pdGenderFemale;
	CRichElement*		m_pdAge;
	CRichElement*		m_pdBioText;
	CRichElement*		m_pdInterests;
	CRichElement*		m_pdBookmarks;
	CRichElement*		m_pdContactEmail;
	CRichElement*		m_pdContactSkype;
	CRichElement*		m_pdContactYahoo;
	CRichElement*		m_pdContactICQ;
	CRichElement*		m_pdContactAOL;
	CRichElement*		m_pdContactJabber;
	CRichElement*		m_pdContactTwitter;
	CRichElement*		m_pdContactFacebook;
	CRichElement*		m_pdContactGetEnvy;
	CRichElement*		m_pdVendor;
	CRichElement*		m_pdAddress;

	CImageFile			m_imgHead;

public:
	void	Serialize(CArchive& ar, int nVersion);	// BROWSER_SER_VERSION
	void	OnSkinChange();
	void	OnHeadPacket(CG2Packet* pPacket);
	void	Update(CHostBrowser* pBrowser);
protected:
	void	UpdateDocumentLeft(CHostBrowser* pBrowser, CGProfile* pProfile);
	void	UpdateDocumentRight(CHostBrowser* pBrowser, CGProfile* pProfile);
	void	LoadDefaultHead();

public:
	virtual BOOL Create(CWnd* pParentWnd);

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClickView(NMHDR* pNotify, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};
