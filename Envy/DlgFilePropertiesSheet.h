//
// DlgFilePropertiesSheet.h
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

#include "PagePropertyAdv.h"
#include "LibraryList.h"


class CFilePropertiesSheet : public CPropertySheetAdv
{
	DECLARE_DYNAMIC(CFilePropertiesSheet)

public:
	CFilePropertiesSheet(CLibraryListItem oObject = CLibraryListItem());

	CLibraryListPtr	m_pList;

	void	Add(CLibraryListItem oObject);
	void	Add(const CLibraryList* pList);

	virtual INT_PTR DoModal(int nPage = -1);

protected:
	CString		m_sGeneralTitle;
	CString		m_sMetadataTitle;
	CString		m_sCommentsTitle;
	CString		m_sSharingTitle;
	CString		m_sSourcesTitle;

	virtual BOOL OnInitDialog();

//	DECLARE_MESSAGE_MAP()
};
