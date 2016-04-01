//
// MetaPanel.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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

#include "MetaList.h"


class CMetaPanel : public CMetaList
{
public:
	CMetaPanel();
	virtual ~CMetaPanel();

public:
	int		Layout(CDC* pDC, int nWidth);
	void	Paint(CDC* pDC, const CRect* prcArea);
	BOOL	OnClick(const CPoint& point);

	virtual BOOL IsWorking() const { return FALSE; }
	virtual void Start() {}
	virtual void Stop() {}
	virtual void Clear() { CMetaList::Clear(); }

	CMetaPanel*		m_pChild;
	CString			m_sThumbnailURL;

public:
	int		m_nHeight;

protected:
	CBitmap m_bmMusicBrainz;
};
