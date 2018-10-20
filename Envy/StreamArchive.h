//
// StreamArchive.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2006 and PeerProject 2008-2010
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

class CStreamArchive : public CArchive
{
public:
	CStreamArchive(UINT nMode, int nBufSize = 4096, void* lpBuf = NULL) throw() :
		CArchive( ( ( m_pStreamFile = new COleStreamFile ),
			( m_pStreamFile ? m_pStreamFile->CreateMemoryStream() : NULL ), m_pStreamFile ),
			nMode, nBufSize, lpBuf )
	{
	}

	CStreamArchive(IStream* pIStream, UINT nMode, int nBufSize = 4096, void* lpBuf = NULL) throw() :
		CArchive( ( ( m_pStreamFile = new COleStreamFile ),
			( m_pStreamFile ? m_pStreamFile->Attach( pIStream ) : NULL ), m_pStreamFile ),
			nMode, nBufSize, lpBuf )
	{
	}

	virtual ~CStreamArchive() throw()
	{
		if ( m_pFile )
		{
			Close();
		}

		delete m_pStreamFile;
	}

	inline operator LPSTREAM() throw()
	{
		return m_pStreamFile ? m_pStreamFile->m_lpStream : NULL;
	}

	inline LPSTREAM Detach() throw()
	{
		if ( m_pFile )
		{
			Close();
		}

		LPSTREAM pStream = m_pStreamFile ? m_pStreamFile->Detach() : NULL;

		delete m_pStreamFile;
		m_pStreamFile = NULL;

		return pStream;
	}

	inline bool IsValid() const throw()
	{
		return ( m_pStreamFile && ( m_pStreamFile->m_lpStream != NULL ) );
	}

protected:
	COleStreamFile*	m_pStreamFile;
};
