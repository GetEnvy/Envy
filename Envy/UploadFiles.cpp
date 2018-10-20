//
// UploadFiles.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2010
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

#include "StdAfx.h"
#include "Envy.h"
#include "UploadFiles.h"
#include "UploadFile.h"
#include "UploadTransfer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CUploadFiles UploadFiles;


//////////////////////////////////////////////////////////////////////
// CUploadFiles construction

CUploadFiles::CUploadFiles()
{
}

CUploadFiles::~CUploadFiles()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CUploadFiles clear

void CUploadFiles::Clear()
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		GetNext( pos )->Remove();
	}

	ASSERT( GetCount() == 0 );
}

//////////////////////////////////////////////////////////////////////
// CUploadFiles file allocation

CUploadFile* CUploadFiles::GetFile(CUploadTransfer* pUpload, const Hashes::Sha1Hash& oSHA1, LPCTSTR pszName, LPCTSTR pszPath, QWORD nSize)
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		CUploadFile* pFile = GetNext( pos );

		if ( pFile->m_pAddress.S_un.S_addr == pUpload->m_pHost.sin_addr.S_un.S_addr )
		{
			if ( pFile->m_sPath == pszPath )
			{
				pFile->Add( pUpload );
				return pFile;
			}
		}
	}

	CUploadFile* pFile = new CUploadFile( pUpload, oSHA1, pszName, pszPath, nSize );
	m_pList.AddTail( pFile );

	return pFile;
}

//////////////////////////////////////////////////////////////////////
// CUploadFiles remove an upload trasnfer

void CUploadFiles::Remove(CUploadTransfer* pTransfer)
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posRemove = pos;
		CUploadFile* pFile = GetNext( pos );

		if ( pFile->Remove( pTransfer ) )
		{
			delete pFile;
			m_pList.RemoveAt( posRemove );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadFiles move a file with this transfer to the head/tail. (Cheap BT sorting)

void CUploadFiles::MoveToHead(CUploadTransfer* pTransfer)
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posThis = pos;
		CUploadFile* pFile = GetNext( pos );

		if ( pFile->GetActive() == pTransfer )
		{
			m_pList.RemoveAt( posThis );
			m_pList.AddHead( pFile );
			return;
		}
	}
}

void CUploadFiles::MoveToTail(CUploadTransfer* pTransfer)
{
	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posThis = pos;
		CUploadFile* pFile = GetNext( pos );

		if ( pFile->GetActive() == pTransfer )
		{
			m_pList.RemoveAt( posThis );
			m_pList.AddTail( pFile );
			return;
		}
	}
}
