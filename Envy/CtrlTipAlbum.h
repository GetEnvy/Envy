//
// CtrlTipAlbum.h
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

#include "CtrlCoolTip.h"
#include "MetaList.h"


class CAlbumTipCtrl : public CCoolTipCtrl
{
	DECLARE_DYNAMIC(CAlbumTipCtrl)

public:
	CAlbumTipCtrl();
	virtual ~CAlbumTipCtrl();

public:
	void Show(CAlbumFolder* pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_pAlbumFolder );
		m_pAlbumFolder = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	CAlbumFolder*	m_pAlbumFolder;
	CString			m_sName;
	CString			m_sType;
	int				m_nIcon32;
	int				m_nIcon48;
	BOOL			m_bCollection;
	CMetaList		m_pMetadata;
	int				m_nKeyWidth;
	COLORREF		m_crLight;

protected:
	void			DrawThumb(CDC* pDC, CRect& rcThumb);

	virtual BOOL	OnPrepare();
	virtual void	OnCalcSize(CDC* pDC);
	virtual void	OnPaint(CDC* pDC);

	//DECLARE_MESSAGE_MAP()
};
