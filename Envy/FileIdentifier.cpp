//
// FileIdentifier.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2020
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2016
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
#include "FileIdentifier.h"
#include "EDPacket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CFileIdentifier construction

CFileIdentifier::CFileIdentifier()
	: m_nSize( 0 )
{
}

CFileIdentifier::CFileIdentifier(const Hashes::Ed2kHash& oHash, QWORD nSize)
	: m_oHash( oHash )
	, m_nSize( nSize )
{
}

CFileIdentifier::~CFileIdentifier()
{
}

//////////////////////////////////////////////////////////////////////
// CFileIdentifier parse from packet

bool CFileIdentifier::Parse(CEDPacket* pPacket)
{
	if ( ! pPacket )
		return false;

	// FileIdentifier format: <Hash 16><Size 8>
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + sizeof(QWORD) )
		return false;

	pPacket->Read( m_oHash );
	m_nSize = pPacket->ReadInt64();

	return true;
}

//////////////////////////////////////////////////////////////////////
// CFileIdentifier write to packet

void CFileIdentifier::Write(CEDPacket* pPacket) const
{
	if ( ! pPacket )
		return;

	// FileIdentifier format: <Hash 16><Size 8>
	pPacket->Write( m_oHash );
	pPacket->WriteInt64( m_nSize );
}

//////////////////////////////////////////////////////////////////////
// CFileIdentifier getters

const Hashes::Ed2kHash& CFileIdentifier::GetHash() const
{
	return m_oHash;
}

QWORD CFileIdentifier::GetSize() const
{
	return m_nSize;
}
