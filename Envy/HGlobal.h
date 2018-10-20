//
// HGlobal.h
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

template < typename T > class CHGlobal
{
public:
	CHGlobal( SIZE_T dwBytes  = sizeof( T ) ) throw() :
		m_phGlobal( GlobalAlloc( GHND, dwBytes ) ),
		m_pCached( NULL )
	{
	}

	CHGlobal( HGLOBAL hglobIn ) throw() :
		m_phGlobal( NULL ),
		m_pCached( NULL )
	{
		if ( hglobIn )
		{
			SIZE_T cb = GlobalSize( hglobIn );
			LPVOID pvIn = GlobalLock( hglobIn );
			if ( pvIn )
			{
				m_phGlobal = GlobalAlloc( GHND, cb );
				if ( m_phGlobal )
				{
					LPVOID pvOut = GlobalLock( m_phGlobal );
					if ( pvOut )
					{
						CopyMemory( pvOut, pvIn, cb );
					}
					GlobalUnlock( m_phGlobal );
				}
				GlobalUnlock( hglobIn );
			}
		}
	}

	virtual ~CHGlobal() throw()
	{
		Clean();
	}

	inline void Clean() throw()
	{
		if ( m_phGlobal )
		{
			if ( m_pCached )
			{
				GlobalUnlock( m_phGlobal );
				m_pCached = NULL;
			}
			GlobalFree( m_phGlobal );
			m_phGlobal = NULL;
		}
	}

	inline operator T*() throw()
	{
		if ( m_phGlobal )
		{
			if ( ! m_pCached )
			{
				m_pCached = static_cast < T* > ( GlobalLock( m_phGlobal ) );
			}
			return m_pCached;
		}
		return NULL;
	}

	inline T* operator ->() throw()
	{
		return operator T*();
	}

	inline bool IsValid() const throw()
	{
		return ( m_phGlobal != NULL );
	}

	inline operator HGLOBAL() throw()
	{
		return m_phGlobal;
	}

	inline HGLOBAL Detach() throw()
	{
		if ( m_phGlobal )
		{
			if ( m_pCached )
			{
				GlobalUnlock( m_phGlobal );
				m_pCached = NULL;
			}
		}
		HGLOBAL hGlobal = m_phGlobal;
		m_phGlobal = NULL;
		return hGlobal;
	}

	inline SIZE_T Size() const throw()
	{
		if ( m_phGlobal )
		{
			return GlobalSize( m_phGlobal );
		}
		return 0;
	}

protected:
	HGLOBAL	m_phGlobal;
	T*		m_pCached;
};
