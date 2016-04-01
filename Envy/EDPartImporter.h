//
// EDPartImporter.h
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

#include "ThreadImpl.h"


class CEDPartImporter : public CThreadImpl
{
public:
	CEDPartImporter();
	virtual ~CEDPartImporter();

public:
	void	AddFolder(LPCTSTR pszFolder);
	void	Start(CEdit* pCtrl);
	void	Stop();

protected:
	CList< CString > m_pFolders;
	CEdit*	m_pTextCtrl;
	int 	m_nCount;

	void	ImportFolder(LPCTSTR pszPath);
	BOOL	ImportFile(LPCTSTR pszPath, LPCTSTR pszFile);
	void	Message(UINT nMessageID, ...);
	void	OnRun();
};
